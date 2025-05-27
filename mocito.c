/*
   Mocito is a mocking library for writing unit tests of C code.
   Copyright (C) 2025 Carlos Rica Espinosa <jasampler@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

/*
 * Implementation of the Mocito C mocking library for writing tests.
 */

#include "mocito.h"

/* Remove this definition to run tests of internal functions. */
#define MOC_NOTESTS

#define MOC_ERR_NFUNLIMIT 1  /* insufficient memory for functions */
#define MOC_ERR_NMAPLIMIT 2  /* insufficient memory for mappings */
#define MOC_ERR_NMTCLIMIT 3  /* insufficient memory for matchers */
#define MOC_ERR_NRSPLIMIT 4  /* insufficient memory for responders */
#define MOC_ERR_NNODLIMIT 5  /* insufficient memory for list nodes */
#define MOC_ERR_PARAMTYPE 6  /* unexpected parameter type */
#define MOC_ERR_RETURTYPE 7  /* unexpected return type */
#define MOC_ERR_INVALTYPE 8  /* invalid parameter type */
#define MOC_ERR_INVALOPER 9  /* invalid comparison operator */
#define MOC_ERR_FUNNOTFND 10 /* function not found in mappings */
#define MOC_ERR_MAPNOTFND 11 /* no mappings matched for call */
#define MOC_ERR_INVALMTCH 12 /* invalid place for matcher */
#define MOC_ERR_INVNPARAM 13 /* invalid parameter number */

static const char *moc_gerrdesc[] = {
	/* (UNUSED) */      "",
	/* MOC_ERR_NFUNLIMIT */ "insufficient memory for functions",
	/* MOC_ERR_NMAPLIMIT */ "insufficient memory for mappings",
	/* MOC_ERR_NMTCLIMIT */ "insufficient memory for matchers",
	/* MOC_ERR_NRSPLIMIT */ "insufficient memory for responders",
	/* MOC_ERR_NNODLIMIT */ "insufficient memory for list nodes",
	/* MOC_ERR_PARAMTYPE */ "unexpected parameter type",
	/* MOC_ERR_RETURTYPE */ "unexpected return type",
	/* MOC_ERR_INVALTYPE */ "invalid parameter type",
	/* MOC_ERR_INVALOPER */ "invalid comparison operator",
	/* MOC_ERR_FUNNOTFND */ "function not found in mappings",
	/* MOC_ERR_MAPNOTFND */ "no mappings matched for call",
	/* MOC_ERR_INVALMTCH */ "invalid place for matcher",
	/* MOC_ERR_INVNPARAM */ "invalid parameter number"
};

static const char *moc_gtypenames[] = {
	"void",
	"char",
	"short",
	"int",
	"long",
	"float",
	"double",
	"signed char",
	"unsigned char",
	"unsigned short",
	"unsigned int",
	"unsigned long",
	"function"
};

typedef unsigned char MOC_NUM_T; /* small count of items */
typedef unsigned short MOC_SIZE_T; /* medium count of items */
typedef unsigned char MOC_OPTS_T; /* matcher/responder options */

/* Data of a mocking-related error for generating an error message. */
struct moc_error_t {
	const char *funcname;
	MOC_SIZE_T pos;
	moc_type acttype, exptype;
	unsigned char errnum;
	char errmsg[200];
};

/* Single-linked list structure with a pointer to the last member. */
struct moc_list {
	struct moc_listnode *first;
	struct moc_listnode *last;
};

/* Element of the single-linked list structure that includes an
 * additional number of items stored from the given item pointer. */
struct moc_listnode {
	void *item;
	struct moc_listnode *next;
	MOC_NUM_T nitems;
};

/* Internal value structure. */
struct moc_ivalue {
	double data;
	moc_type type;
};

/* Internal matcher structure. */
struct moc_imatcher {
	struct moc_value mval;
	union moc_mtcfn_union {
		moc_mtcfn_param_t prm;
		moc_mtcfn_call_t cll;
	} mtcfn;
	MOC_OPTS_T mopts;
};

/* Internal responder structure. */
struct moc_iresponder {
	struct moc_value rval;
	union moc_rspfn_union {
		moc_rspfn_param_t prm;
		moc_rspfn_call_t cll;
	} rspfn;
	MOC_OPTS_T ropts;
};

/* Mapping structure that stores a list of arrays of responders and an
 * array of matchers having a number of matchers equal to the number of
 * parameters of the function plus the number of extra matchers. */
struct moc_mapping {
	struct moc_list lresps;
	struct moc_matcher *matchers;
	MOC_NUM_T nxmatchers;
};

/* Function structure that stores its name, its number of parameters
 * and a list of mappings that connect its matchers and responders. */
struct moc_function {
	struct moc_list lmaps;
	const char *name;
	MOC_NUM_T nparams;
};

/* Defines a memory position to mark where a list of nodes ends. */
static struct moc_listnode moc_nullnode;
#define MOC_NULL ((void *) &moc_nullnode)
#define MOC_NULLNODE ((struct moc_listnode *) &moc_nullnode)
#define MOC_AUXMAX 7

/* The context groups all the global variables used by Mocito. */
struct moc_context {
	struct moc_error_t lasterr;
	moc_errfn_t errfn;
	struct moc_matcher auxmatcs[MOC_AUXMAX];
	struct moc_matcher auxxmatcs[MOC_AUXMAX];
	struct moc_responder auxresps[MOC_AUXMAX];
	struct moc_value auxvals[MOC_AUXMAX];
	struct moc_function *funcs;
	struct moc_mapping *maps;
	struct moc_matcher *matcs;
	struct moc_responder *resps;
	struct moc_listnode *lnods;
	MOC_SIZE_T maxfuncs, nfuncs;
	MOC_SIZE_T maxmaps, nmaps;
	MOC_SIZE_T maxmatcs, nmatcs;
	MOC_SIZE_T maxresps, nresps;
	MOC_SIZE_T maxlnods, nlnods;
};

static struct moc_context moc_ctx;

/* The type of a value is defined by a standard (and a pointer) type. */
enum moc_stdtype {
	MOC_VOID, MOC_CHR, MOC_SHR, MOC_INT, MOC_LNG, MOC_FLT, MOC_DBL,
	MOC_SCHR, MOC_UCHR, MOC_USHR, MOC_UINT, MOC_ULNG, MOC_FUN
};

/* The type of a value is defined by a pointer (and a standard) type. */
enum moc_ptrtype { MOC_NOPTR, MOC_PTR, MOC_CPTR };

static struct moc_value moc_emptyval; /* empty value */
static struct moc_matcher moc_emptymtc; /* empty matcher */
static struct moc_responder moc_emptyrsp; /* empty responder */

/* Type of a comparison operation. */
enum moc_op { MOC_EQ, MOC_NE, MOC_LT, MOC_LE, MOC_GT, MOC_GE };

/* Copies the digits of an integer in a string and returns the string. */
static char *moc_itostr(char *dest, int orig) {
	int i, j, neg, d;
	if (orig == 0)
	{
		dest[0] = '0';
		dest[1] = '\0';
	} else {
		neg = (orig < 0);
		i = 0;
		do {
			d = orig % 10;
			dest[i++] = (d < 0 ? -d : d) + '0';
			orig /= 10;
		} while (orig != 0);
		if (neg) {
			dest[i++] = '-';
		}
		dest[i--] = '\0';
		j = 0;
		while (j < i) {
			d = dest[i];
			dest[i--] = dest[j];
			dest[j++] = (char) d;
		}
	}
	return dest;
}

/* Returns the lenght of the given string. */
static unsigned long moc_strlen(const char *str) {
	unsigned long n = 0;
	while (*str++ != '\0') {
		n++;
	}
	return n;
}

/* Copies at most n-1 characters from orig string to the dest string. */
static char *moc_strncpy(char *dest, const char *orig, unsigned long n) {
	unsigned long i;
	char *r;
	r = dest;
	for (i = 1; i < n && *orig != '\0'; i++) {
		*dest++ = *orig++;
	}
	if (n > 0) {
		*dest = '\0';
	}
	return r;
}

/* Returns an integer that is less than, equal to or greater than 0, if
 * the first string is less than, equal to or greater than the second. */
static int moc_strcmp(const char *str1, const char *str2) {
	while (*str1 == *str2 && *str1 != '\0') {
		str1++;
		str2++;
	}
	return ((int) *str2) - *str1;
}

/*
 * Returns the first index of the given substring in the given string.
 * If not found (or both are empty), then returns the index of the end.
 */
static unsigned long moc_substridx(const char *str, const char *substr) {
	unsigned long i, j;
	if (substr[0] == '\0') {
		return 0;
	}
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == substr[0]) {
			for (j = 1; str[i + j] != '\0'
					&& substr[j] != '\0'; j++) {
				if (str[i + j] != substr[j]) {
					break;
				}
			}
			if (substr[j] == '\0') {
				break;
			}
		}
	}
	return i;
}

/* Initializes the pointed error structure with the given data. */
static void moc_init_error_t(struct moc_error_t *e, unsigned char errnum,
		const char *funcname, MOC_SIZE_T pos,
		moc_type acttype, moc_type exptype) {
	e->errnum = errnum;
	e->funcname = funcname;
	e->pos = pos;
	e->exptype = exptype;
	e->acttype = acttype;
	e->errmsg[0] = '\0';
}

/* Saves the given last error data and calls to the error function. */
static void moc_send_error(unsigned char errnum,
		const char *funcname, MOC_SIZE_T pos,
		moc_type acttype, moc_type exptype) {
	moc_init_error_t(&(moc_ctx.lasterr), errnum, funcname, pos,
			acttype, exptype);
	moc_ctx.errfn();
}

#ifndef MOC_NOTESTS
#include <assert.h>
static void moc_test_size(void) {
	assert(sizeof(struct moc_value)
			<= sizeof(struct moc_ivalue));
	assert(sizeof(struct moc_matcher)
			<= sizeof(struct moc_imatcher));
	assert(sizeof(struct moc_responder)
			<= sizeof(struct moc_iresponder));
}

static void moc_test_itostr(void) {
	char buf[20];
	assert(moc_strcmp("0", moc_itostr(buf,0)) == 0);
	assert(moc_strcmp("1", moc_itostr(buf,1)) == 0);
	assert(moc_strcmp("9", moc_itostr(buf,9)) == 0);
	assert(moc_strcmp("10", moc_itostr(buf,10)) == 0);
	assert(moc_strcmp("22", moc_itostr(buf,22)) == 0);
	assert(moc_strcmp("300", moc_itostr(buf,300)) == 0);
	assert(moc_strcmp("4605", moc_itostr(buf,4605)) == 0);
	assert(moc_strcmp("-1", moc_itostr(buf,-1)) == 0);
	assert(moc_strcmp("-9", moc_itostr(buf,-9)) == 0);
	assert(moc_strcmp("-10", moc_itostr(buf,-10)) == 0);
	assert(moc_strcmp("-22", moc_itostr(buf,-22)) == 0);
	assert(moc_strcmp("-300", moc_itostr(buf,-300)) == 0);
	assert(moc_strcmp("-4605", moc_itostr(buf,-4605)) == 0);
}

static void moc_test_strncpy(void) {
	char buf[100];
	moc_strncpy(buf, "NO", 3);
	assert(moc_strcmp(moc_strncpy(buf, "", 0), "NO") == 0);
	moc_strncpy(buf, "NO", 3);
	assert(moc_strcmp(moc_strncpy(buf, "", 1), "") == 0);
	moc_strncpy(buf, "NO", 3);
	assert(moc_strcmp(moc_strncpy(buf, "", 2), "") == 0);
	moc_strncpy(buf, "NO", 3);
	assert(moc_strcmp(moc_strncpy(buf, "abc", 0), "NO") == 0);
	assert(moc_strcmp(moc_strncpy(buf, "def", 1), "") == 0);
	assert(moc_strcmp(moc_strncpy(buf, "ghi", 2), "g") == 0);
	assert(moc_strcmp(moc_strncpy(buf, "jkl", 3), "jk") == 0);
	assert(moc_strcmp(moc_strncpy(buf, "mno", 4), "mno") == 0);
	assert(moc_strcmp(moc_strncpy(buf, "pqr", 5), "pqr") == 0);
}

static void moc_test_substridx(void) {
	assert(moc_substridx("", "") == 0);
	assert(moc_substridx("", "aa") == 0);
	assert(moc_substridx("bbb", "") == 0);
	assert(moc_substridx("", "d") == 0);
	assert(moc_substridx("a", "d") == 1);
	assert(moc_substridx("ab", "d") == 2);
	assert(moc_substridx("abc", "d") == 3);
	assert(moc_substridx("eeff", "e") == 0);
	assert(moc_substridx("eeff", "ee") == 0);
	assert(moc_substridx("eeff", "eef") == 0);
	assert(moc_substridx("eeff", "eeff") == 0);
	assert(moc_substridx("eeff", "ef") == 1);
	assert(moc_substridx("eeff", "eff") == 1);
	assert(moc_substridx("eeff", "f") == 2);
	assert(moc_substridx("eeff", "ff") == 2);
	assert(moc_substridx("eeff", "g") == 4);
	assert(moc_substridx("eeff", "eg") == 4);
	assert(moc_substridx("eeff", "eeg") == 4);
	assert(moc_substridx("eeff", "eefg") == 4);
	assert(moc_substridx("eeff", "eeffg") == 4);
	assert(moc_substridx("eeff", "efg") == 4);
	assert(moc_substridx("eeff", "effg") == 4);
	assert(moc_substridx("eeff", "fg") == 4);
	assert(moc_substridx("eeff", "ffg") == 4);
}

static void moc_test_errmsg(void) {
	moc_init_error_t(&(moc_ctx.lasterr), MOC_ERR_NFUNLIMIT, "f1", 0,
			0, 0);
	assert(0 == moc_strcmp(moc_errmsg(),
			"insufficient memory for functions: f1"));
	moc_init_error_t(&(moc_ctx.lasterr), MOC_ERR_FUNNOTFND, "f2", 2,
			0, 0);
	assert(0 == moc_strcmp(moc_errmsg(),
			"function not found in mappings: f2 (2)"));
	moc_init_error_t(&(moc_ctx.lasterr), MOC_ERR_INVALTYPE, "f3", 3,
			moc_type_cp_c(), moc_type_cp_c());
	assert(0 == moc_strcmp(moc_errmsg(),
			"invalid parameter type: f3 (3): (const char *)"));
	moc_init_error_t(&(moc_ctx.lasterr), MOC_ERR_PARAMTYPE, "f4", 4,
			moc_type_p(), moc_type_p_i());
	assert(0 == moc_strcmp(moc_errmsg(),
		"unexpected parameter type: f4 (4): (void *)<>(int *)"));
	moc_init_error_t(&(moc_ctx.lasterr), MOC_ERR_RETURTYPE, "f5", 0,
				moc_type_l(), moc_type_ul());
	assert(0 == moc_strcmp(moc_errmsg(),
		"unexpected return type: f5: (long)<>(unsigned long)"));
}
#endif

void moc_init(char *mem, unsigned long size) {
	char *restmem;
	moc_ctx.errfn = moc_error;
	moc_ctx.maxfuncs = size / (5 * sizeof(struct moc_function));
	moc_ctx.maxmaps = size / (5 * sizeof(struct moc_mapping));
	moc_ctx.maxmatcs = size / (5 * sizeof(struct moc_matcher));
	moc_ctx.maxresps = size / (5 * sizeof(struct moc_responder));
	moc_ctx.maxlnods = size / (5 * sizeof(struct moc_listnode));
	moc_ctx.nfuncs = moc_ctx.nmaps = moc_ctx.nmatcs =
		moc_ctx.nresps = moc_ctx.nlnods = 0;
	restmem = mem;
	moc_ctx.funcs = (struct moc_function *) restmem;
	restmem += moc_ctx.maxfuncs * sizeof(struct moc_function);
	moc_ctx.maps = (struct moc_mapping *) restmem;
	restmem += moc_ctx.maxmaps * sizeof(struct moc_mapping);
	moc_ctx.matcs = (struct moc_matcher *) restmem;
	restmem += moc_ctx.maxmatcs * sizeof(struct moc_matcher);
	moc_ctx.resps = (struct moc_responder *) restmem;
	restmem += moc_ctx.maxresps * sizeof(struct moc_responder);
	moc_ctx.lnods = (struct moc_listnode *) restmem;
#ifndef MOC_NOTESTS
	moc_test_size();
	moc_test_itostr();
	moc_test_strncpy();
	moc_test_substridx();
	moc_test_errmsg();
#endif
}

void moc_set_errfn(moc_errfn_t errfn) {
	moc_ctx.errfn = errfn;
}

#define MOC_IVAL(pvalue)       ((struct moc_ivalue *) (pvalue))
#define MOC_VALBYTE(value)    (MOC_IVAL(&(value))->type)
#define MOC_STDTYPE(byte)     ((enum moc_stdtype) (((int) (byte)) / 4))
#define MOC_PTRTYPE(byte)     ((enum moc_ptrtype) (((int) (byte)) & 3))
#define MOC_VALSTDTYPE(value) MOC_STDTYPE(MOC_VALBYTE(value))
#define MOC_VALPTRTYPE(value) MOC_PTRTYPE(MOC_VALBYTE(value))
#define MOC_VALDATA(value)    (&(MOC_IVAL(&(value))->data))
#define MOC_EMPTYVAL(value)   ((value) = moc_emptyval)
#define MOC_TYPES2BYTE(stdtype, ptrtype) \
	((unsigned short) ((((int) (stdtype)) * 4) | (ptrtype)))
#define MOC_IMTC(pmatcher)     ((struct moc_imatcher *) (pmatcher))
#define MOC_IRSP(presponder)   ((struct moc_iresponder *) (presponder))

static void moc_type2str(char *dest, unsigned short type) {
	unsigned long n = 0;
	int stdtype, ptrtype;
	stdtype = MOC_STDTYPE(type);
	ptrtype = MOC_PTRTYPE(type);
	if (stdtype <= MOC_FUN) {
		moc_strncpy(dest + n, "(", 2);
		n++;
		if (ptrtype == MOC_CPTR) {
			moc_strncpy(dest + n, "const ", 7);
			n += 6;
		}
		moc_strncpy(dest + n, moc_gtypenames[stdtype], 14);
		n += moc_strlen(dest + n);
		if (ptrtype == MOC_PTR || ptrtype == MOC_CPTR) {
			moc_strncpy(dest + n, " *", 3);
			n += 2;
		}
		moc_strncpy(dest + n, ")", 2);
		n++;
	} else {
		moc_itostr(dest + n, stdtype);
		n += moc_strlen(dest + n);
	}
}

/* "err_desc: func_name (n_param): (actual_type)<>(expected_type)" */
const char *moc_errmsg(void) {
	unsigned long n;
	struct moc_error_t *e;
	e = &(moc_ctx.lasterr);
	if(e->errmsg[0] == '\0') {
		n = 0;
		if (e->errnum <= MOC_ERR_INVNPARAM) {
			moc_strncpy(e->errmsg + n,
					moc_gerrdesc[e->errnum], 35);
		}
		n += moc_strlen(e->errmsg + n);
		moc_strncpy(e->errmsg + n, ": ", 3);
		n += 2;
		moc_strncpy(e->errmsg + n, e->funcname, 65);
		n += moc_strlen(e->funcname);
		if (e->pos > 0) {
			moc_strncpy(e->errmsg + n, " (", 3);
			n += 2;
			moc_itostr(e->errmsg + n, e->pos);
			n += moc_strlen(e->errmsg + n);
			moc_strncpy(e->errmsg + n, ")", 2);
			n++;
		}
		if (e->exptype || e->exptype != e->acttype) {
			moc_strncpy(e->errmsg + n, ": ", 3);
			n += 2;
			moc_type2str(e->errmsg + n, e->acttype);
			n += moc_strlen(e->errmsg + n);
			if (e->exptype != e->acttype) {
				moc_strncpy(e->errmsg + n, "<>", 3);
				n += 2;
				moc_type2str(e->errmsg + n, e->exptype);
				n += moc_strlen(e->errmsg + n);
			}
		}
	}
	return e->errmsg;
}

moc_type moc_type_void(void) {
	return MOC_TYPES2BYTE(MOC_VOID, MOC_NOPTR);
}

moc_type moc_type_c(void) {
	return MOC_TYPES2BYTE(MOC_CHR, MOC_NOPTR);
}

moc_type moc_type_s(void) {
	return MOC_TYPES2BYTE(MOC_SHR, MOC_NOPTR);
}

moc_type moc_type_i(void) {
	return MOC_TYPES2BYTE(MOC_INT, MOC_NOPTR);
}

moc_type moc_type_l(void) {
	return MOC_TYPES2BYTE(MOC_LNG, MOC_NOPTR);
}

moc_type moc_type_f(void) {
	return MOC_TYPES2BYTE(MOC_FLT, MOC_NOPTR);
}

moc_type moc_type_d(void) {
	return MOC_TYPES2BYTE(MOC_DBL, MOC_NOPTR);
}

moc_type moc_type_sc(void) {
	return MOC_TYPES2BYTE(MOC_SCHR, MOC_NOPTR);
}

moc_type moc_type_uc(void) {
	return MOC_TYPES2BYTE(MOC_UCHR, MOC_NOPTR);
}

moc_type moc_type_us(void) {
	return MOC_TYPES2BYTE(MOC_USHR, MOC_NOPTR);
}

moc_type moc_type_ui(void) {
	return MOC_TYPES2BYTE(MOC_UINT, MOC_NOPTR);
}

moc_type moc_type_ul(void) {
	return MOC_TYPES2BYTE(MOC_ULNG, MOC_NOPTR);
}

moc_type moc_type_fn(void) {
	return MOC_TYPES2BYTE(MOC_FUN, MOC_NOPTR);
}

moc_type moc_type_p(void) {
	return MOC_TYPES2BYTE(MOC_VOID, MOC_PTR);
}

moc_type moc_type_p_c(void) {
	return MOC_TYPES2BYTE(MOC_CHR, MOC_PTR);
}

moc_type moc_type_p_s(void) {
	return MOC_TYPES2BYTE(MOC_SHR, MOC_PTR);
}

moc_type moc_type_p_i(void) {
	return MOC_TYPES2BYTE(MOC_INT, MOC_PTR);
}

moc_type moc_type_p_l(void) {
	return MOC_TYPES2BYTE(MOC_LNG, MOC_PTR);
}

moc_type moc_type_p_f(void) {
	return MOC_TYPES2BYTE(MOC_FLT, MOC_PTR);
}

moc_type moc_type_p_d(void) {
	return MOC_TYPES2BYTE(MOC_DBL, MOC_PTR);
}

moc_type moc_type_p_sc(void) {
	return MOC_TYPES2BYTE(MOC_SCHR, MOC_PTR);
}

moc_type moc_type_p_uc(void) {
	return MOC_TYPES2BYTE(MOC_UCHR, MOC_PTR);
}

moc_type moc_type_p_us(void) {
	return MOC_TYPES2BYTE(MOC_USHR, MOC_PTR);
}

moc_type moc_type_p_ui(void) {
	return MOC_TYPES2BYTE(MOC_UINT, MOC_PTR);
}

moc_type moc_type_p_ul(void) {
	return MOC_TYPES2BYTE(MOC_ULNG, MOC_PTR);
}

moc_type moc_type_p_fn(void) {
	return MOC_TYPES2BYTE(MOC_FUN, MOC_PTR);
}

moc_type moc_type_cp(void) {
	return MOC_TYPES2BYTE(MOC_VOID, MOC_CPTR);
}

moc_type moc_type_cp_c(void) {
	return MOC_TYPES2BYTE(MOC_CHR, MOC_CPTR);
}

moc_type moc_type_cp_s(void) {
	return MOC_TYPES2BYTE(MOC_SHR, MOC_CPTR);
}

moc_type moc_type_cp_i(void) {
	return MOC_TYPES2BYTE(MOC_INT, MOC_CPTR);
}

moc_type moc_type_cp_l(void) {
	return MOC_TYPES2BYTE(MOC_LNG, MOC_CPTR);
}

moc_type moc_type_cp_f(void) {
	return MOC_TYPES2BYTE(MOC_FLT, MOC_CPTR);
}

moc_type moc_type_cp_d(void) {
	return MOC_TYPES2BYTE(MOC_DBL, MOC_CPTR);
}

moc_type moc_type_cp_sc(void) {
	return MOC_TYPES2BYTE(MOC_SCHR, MOC_CPTR);
}

moc_type moc_type_cp_uc(void) {
	return MOC_TYPES2BYTE(MOC_UCHR, MOC_CPTR);
}

moc_type moc_type_cp_us(void) {
	return MOC_TYPES2BYTE(MOC_USHR, MOC_CPTR);
}

moc_type moc_type_cp_ui(void) {
	return MOC_TYPES2BYTE(MOC_UINT, MOC_CPTR);
}

moc_type moc_type_cp_ul(void) {
	return MOC_TYPES2BYTE(MOC_ULNG, MOC_CPTR);
}

moc_type moc_type_cp_fn(void) {
	return MOC_TYPES2BYTE(MOC_FUN, MOC_CPTR);
}

struct moc_value moc_void(void) {
	return moc_emptyval;
}

struct moc_value moc_c(char val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((char *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_CHR, MOC_NOPTR);
	return value;
}

struct moc_value moc_s(short val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((short *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SHR, MOC_NOPTR);
	return value;
}

struct moc_value moc_i(int val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((int *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_INT, MOC_NOPTR);
	return value;
}

struct moc_value moc_l(long val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((long *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_LNG, MOC_NOPTR);
	return value;
}

struct moc_value moc_f(float val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((float *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FLT, MOC_NOPTR);
	return value;
}

struct moc_value moc_d(double val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((double *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_DBL, MOC_NOPTR);
	return value;
}

struct moc_value moc_sc(signed char val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((signed char *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SCHR, MOC_NOPTR);
	return value;
}

struct moc_value moc_uc(unsigned char val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned char *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UCHR, MOC_NOPTR);
	return value;
}

struct moc_value moc_us(unsigned short val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned short *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_USHR, MOC_NOPTR);
	return value;
}

struct moc_value moc_ui(unsigned int val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned int *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UINT, MOC_NOPTR);
	return value;
}

struct moc_value moc_ul(unsigned long val) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned long *) MOC_VALDATA(value)) = val;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_ULNG, MOC_NOPTR);
	return value;
}

struct moc_value moc_fn(moc_fnptr fn) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((moc_fnptr *) MOC_VALDATA(value)) = fn;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FUN, MOC_NOPTR);
	return value;
}

struct moc_value moc_p(void *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((void **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_VOID, MOC_PTR);
	return value;
}

struct moc_value moc_p_c(char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_CHR, MOC_PTR);
	return value;
}

struct moc_value moc_p_s(short *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((short **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SHR, MOC_PTR);
	return value;
}

struct moc_value moc_p_i(int *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((int **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_INT, MOC_PTR);
	return value;
}

struct moc_value moc_p_l(long *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((long **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_LNG, MOC_PTR);
	return value;
}

struct moc_value moc_p_f(float *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((float **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FLT, MOC_PTR);
	return value;
}

struct moc_value moc_p_d(double *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((double **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_DBL, MOC_PTR);
	return value;
}

struct moc_value moc_p_sc(signed char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((signed char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SCHR, MOC_PTR);
	return value;
}

struct moc_value moc_p_uc(unsigned char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UCHR, MOC_PTR);
	return value;
}

struct moc_value moc_p_us(unsigned short *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned short **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_USHR, MOC_PTR);
	return value;
}

struct moc_value moc_p_ui(unsigned int *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned int **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UINT, MOC_PTR);
	return value;
}

struct moc_value moc_p_ul(unsigned long *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((unsigned long **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_ULNG, MOC_PTR);
	return value;
}

struct moc_value moc_p_fn(moc_fnptr *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((moc_fnptr **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FUN, MOC_PTR);
	return value;
}

struct moc_value moc_cp(const void *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const void **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_VOID, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_c(const char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_CHR, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_s(const short *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const short **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SHR, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_i(const int *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const int **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_INT, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_l(const long *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const long **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_LNG, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_f(const float *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const float **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FLT, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_d(const double *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const double **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_DBL, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_sc(const signed char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const signed char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_SCHR, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_uc(const unsigned char *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const unsigned char **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UCHR, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_us(const unsigned short *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const unsigned short **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_USHR, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_ui(const unsigned int *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const unsigned int **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_UINT, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_ul(const unsigned long *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const unsigned long **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_ULNG, MOC_CPTR);
	return value;
}

struct moc_value moc_cp_fn(const moc_fnptr *p) {
	struct moc_value value;
	MOC_EMPTYVAL(value);
	*((const moc_fnptr **) MOC_VALDATA(value)) = p;
	MOC_VALBYTE(value) = MOC_TYPES2BYTE(MOC_FUN, MOC_CPTR);
	return value;
}

/* Functions to extract a primitive value from a Mocito value: */

char moc_get_c(struct moc_value value) {
	return *((char *) MOC_VALDATA(value));
}

short moc_get_s(struct moc_value value) {
	return *((short *) MOC_VALDATA(value));
}

int moc_get_i(struct moc_value value) {
	return *((int *) MOC_VALDATA(value));
}

long moc_get_l(struct moc_value value) {
	return *((long *) MOC_VALDATA(value));
}

float moc_get_f(struct moc_value value) {
	return *((float *) MOC_VALDATA(value));
}

double moc_get_d(struct moc_value value) {
	return *((double *) MOC_VALDATA(value));
}

signed char moc_get_sc(struct moc_value value) {
	return *((signed char *) MOC_VALDATA(value));
}

unsigned char moc_get_uc(struct moc_value value) {
	return *((unsigned char *) MOC_VALDATA(value));
}

unsigned short moc_get_us(struct moc_value value) {
	return *((unsigned short *) MOC_VALDATA(value));
}

unsigned int moc_get_ui(struct moc_value value) {
	return *((unsigned int *) MOC_VALDATA(value));
}

unsigned long moc_get_ul(struct moc_value value) {
	return *((unsigned long *) MOC_VALDATA(value));
}

moc_fnptr moc_get_fn(struct moc_value value) {
	return *((moc_fnptr *) MOC_VALDATA(value));
}

void *moc_get_p(struct moc_value value) {
	return *((void **) MOC_VALDATA(value));
}

char   *moc_get_p_c(struct moc_value value) {
	return *((char **) MOC_VALDATA(value));
}

short  *moc_get_p_s(struct moc_value value) {
	return *((short **) MOC_VALDATA(value));
}

int    *moc_get_p_i(struct moc_value value) {
	return *((int **) MOC_VALDATA(value));
}

long   *moc_get_p_l(struct moc_value value) {
	return *((long **) MOC_VALDATA(value));
}

float  *moc_get_p_f(struct moc_value value) {
	return *((float **) MOC_VALDATA(value));
}

double *moc_get_p_d(struct moc_value value) {
	return *((double **) MOC_VALDATA(value));
}

signed char    *moc_get_p_sc(struct moc_value value) {
	return *((signed char **) MOC_VALDATA(value));
}

unsigned char  *moc_get_p_uc(struct moc_value value) {
	return *((unsigned char **) MOC_VALDATA(value));
}

unsigned short *moc_get_p_us(struct moc_value value) {
	return *((unsigned short **) MOC_VALDATA(value));
}

unsigned int   *moc_get_p_ui(struct moc_value value) {
	return *((unsigned int **) MOC_VALDATA(value));
}

unsigned long  *moc_get_p_ul(struct moc_value value) {
	return *((unsigned long **) MOC_VALDATA(value));
}

moc_fnptr *moc_get_p_fn(struct moc_value value) {
	return *((moc_fnptr **) MOC_VALDATA(value));
}

const void *moc_get_cp(struct moc_value value) {
	return *((const void **) MOC_VALDATA(value));
}

const char   *moc_get_cp_c(struct moc_value value) {
	return *((const char **) MOC_VALDATA(value));
}

const short  *moc_get_cp_s(struct moc_value value) {
	return *((const short **) MOC_VALDATA(value));
}

const int    *moc_get_cp_i(struct moc_value value) {
	return *((const int **) MOC_VALDATA(value));
}

const long   *moc_get_cp_l(struct moc_value value) {
	return *((const long **) MOC_VALDATA(value));
}

const float  *moc_get_cp_f(struct moc_value value) {
	return *((const float **) MOC_VALDATA(value));
}

const double *moc_get_cp_d(struct moc_value value) {
	return *((const double **) MOC_VALDATA(value));
}

const signed char    *moc_get_cp_sc(struct moc_value value) {
	return *((const signed char **) MOC_VALDATA(value));
}

const unsigned char  *moc_get_cp_uc(struct moc_value value) {
	return *((const unsigned char **) MOC_VALDATA(value));
}

const unsigned short *moc_get_cp_us(struct moc_value value) {
	return *((const unsigned short **) MOC_VALDATA(value));
}

const unsigned int   *moc_get_cp_ui(struct moc_value value) {
	return *((const unsigned int **) MOC_VALDATA(value));
}

const unsigned long  *moc_get_cp_ul(struct moc_value value) {
	return *((const unsigned long **) MOC_VALDATA(value));
}

const moc_fnptr *moc_get_cp_fn(struct moc_value value) {
	return *((const moc_fnptr **) MOC_VALDATA(value));
}

#define MOC_GET_C(value)  (*(char *)   MOC_VALDATA(value))
#define MOC_GET_S(value)  (*(short *)  MOC_VALDATA(value))
#define MOC_GET_I(value)  (*(int *)    MOC_VALDATA(value))
#define MOC_GET_L(value)  (*(long *)   MOC_VALDATA(value))
#define MOC_GET_F(value)  (*(float *)  MOC_VALDATA(value))
#define MOC_GET_D(value)  (*(double *) MOC_VALDATA(value))
#define MOC_GET_SC(value) (*(signed char *)    MOC_VALDATA(value))
#define MOC_GET_UC(value) (*(unsigned char *)  MOC_VALDATA(value))
#define MOC_GET_US(value) (*(unsigned short *) MOC_VALDATA(value))
#define MOC_GET_UI(value) (*(unsigned int *)   MOC_VALDATA(value))
#define MOC_GET_UL(value) (*(unsigned long *)  MOC_VALDATA(value))
#define MOC_GET_FN(value) (*(moc_fnptr *)   MOC_VALDATA(value))
#define MOC_GET_P(value)  (*(void **)       MOC_VALDATA(value))
#define MOC_GET_CP(value) (*(const void **) MOC_VALDATA(value))

/* Returns moc_true if the given values have equal type and value. */
static moc_bool moc_values_eq(struct moc_value val1,
		struct moc_value val2) {
	int ptrtype;
	if (MOC_VALBYTE(val1) == MOC_VALBYTE(val2)) {
		ptrtype = MOC_VALPTRTYPE(val1);
		if (ptrtype != MOC_NOPTR) {
			return (moc_get_p(val1) == moc_get_p(val2));
		}
		switch (MOC_VALSTDTYPE(val1)) {
			case MOC_CHR: return (moc_get_c(val1)
					== moc_get_c(val2));
			case MOC_SHR: return (moc_get_s(val1)
					== moc_get_s(val2));
			case MOC_INT: return (moc_get_i(val1)
					== moc_get_i(val2));
			case MOC_LNG: return (moc_get_l(val1)
					== moc_get_l(val2));
			case MOC_FLT: return (moc_get_f(val1)
					== moc_get_f(val2));
			case MOC_DBL: return (moc_get_d(val1)
					== moc_get_d(val2));
			case MOC_SCHR: return (moc_get_sc(val1)
					== moc_get_sc(val2));
			case MOC_UCHR: return (moc_get_uc(val1)
					== moc_get_uc(val2));
			case MOC_USHR: return (moc_get_us(val1)
					== moc_get_us(val2));
			case MOC_UINT: return (moc_get_ui(val1)
					== moc_get_ui(val2));
			case MOC_ULNG: return (moc_get_ul(val1)
					== moc_get_ul(val2));
			case MOC_FUN: return (*moc_get_fn(val1)
					== moc_get_fn(val2));
			default: return moc_false;
		}
	}
	return moc_false;
}

/* Matcher function that returns always true. */
static moc_bool moc_mtctrue(struct moc_value val1,
		struct moc_value val2) {
	if (sizeof(val1)) {} /* unused warning */
	if (sizeof(val2)) {} /* unused warning */
	return moc_true;
}

#define MOC_SWITCHOP(v1, v2, op) \
	switch (op) { \
		case MOC_EQ: return (v1) == (v2); \
		case MOC_NE: return (v1) != (v2); \
		case MOC_LT: return (v1) <  (v2); \
		case MOC_LE: return (v1) <= (v2); \
		case MOC_GT: return (v1) >  (v2); \
		case MOC_GE: return (v1) >= (v2); \
		default: return moc_false; \
	}

static moc_bool moc_cmpc(char v1, char v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmps(short v1, short v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpi(int v1, int v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpl(long v1, long v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpf(float v1, float v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpd(double v1, double v2, enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpsc(signed char v1, signed char v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpuc(unsigned char v1, unsigned char v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpus(unsigned short v1, unsigned short v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpui(unsigned int v1, unsigned int v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpul(unsigned long v1, unsigned long v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpp(const void *v1, const void *v2,
		enum moc_op op) {
	MOC_SWITCHOP(v1, v2, op);
}

static moc_bool moc_cmpfn(moc_fnptr v1, moc_fnptr v2, enum moc_op op) {
	switch (op) {
		case MOC_EQ: return (v1) == (v2);
		case MOC_NE: return (v1) != (v2);
		default: return moc_false; /* must not arrive here */
	}
}

static moc_bool moc_cmpop(struct moc_value val1, struct moc_value val2,
		enum moc_op op) {
	enum moc_ptrtype ptrtype;
	ptrtype = MOC_VALPTRTYPE(val1);
	if (ptrtype != MOC_NOPTR) {
		if(ptrtype == MOC_PTR || ptrtype == MOC_CPTR) {
			return moc_cmpp(MOC_GET_CP(val1),
					MOC_GET_CP(val2), op);
		}
		return moc_false; /* must not arrive here */
	}
	switch (MOC_VALSTDTYPE(val1)) {
		case MOC_CHR: return moc_cmpc(MOC_GET_C(val1),
						MOC_GET_C(val2), op);
		case MOC_SHR: return moc_cmps(MOC_GET_S(val1),
						MOC_GET_S(val2), op);
		case MOC_INT: return moc_cmpi(MOC_GET_I(val1),
						MOC_GET_I(val2), op);
		case MOC_LNG: return moc_cmpl(MOC_GET_L(val1),
						MOC_GET_L(val2), op);
		case MOC_FLT: return moc_cmpf(MOC_GET_F(val1),
						MOC_GET_F(val2), op);
		case MOC_DBL: return moc_cmpd(MOC_GET_D(val1),
						MOC_GET_D(val2), op);
		case MOC_SCHR: return moc_cmpsc(MOC_GET_SC(val1),
						MOC_GET_SC(val2), op);
		case MOC_UCHR: return moc_cmpuc(MOC_GET_UC(val1),
						MOC_GET_UC(val2), op);
		case MOC_USHR: return moc_cmpus(MOC_GET_US(val1),
						MOC_GET_US(val2), op);
		case MOC_UINT: return moc_cmpui(MOC_GET_UI(val1),
						MOC_GET_UI(val2), op);
		case MOC_ULNG: return moc_cmpul(MOC_GET_UL(val1),
						MOC_GET_UL(val2), op);
		case MOC_FUN: return moc_cmpfn(MOC_GET_FN(val1),
						MOC_GET_FN(val2), op);
		default: return moc_false; /* must not arrive here */
	}
}

static moc_bool moc_cmpeq(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_EQ);
}

static moc_bool moc_cmpne(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_NE);
}

static moc_bool moc_cmplt(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_LT);
}

static moc_bool moc_cmple(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_LE);
}

static moc_bool moc_cmpgt(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_GT);
}

static moc_bool moc_cmpge(struct moc_value val1, struct moc_value val2) {
	return moc_cmpop(val1, val2, MOC_GE);
}

/* Maximum number of parameters, used to mark out of range errors.
 * If opts is greater than 127, then type checking is disabled. */
#define MOC_MAXPARAMS 127

struct moc_matcher moc_mparam(moc_mtcfn_param_t mtcfn,
		struct moc_value value) {
	struct moc_matcher m = moc_emptymtc;
	MOC_IMTC(&m)->mval = value;
	MOC_IMTC(&m)->mtcfn.prm = mtcfn;
	MOC_IMTC(&m)->mopts = 0;
	return m;
}

struct moc_matcher moc_mparam_nochk(moc_mtcfn_param_t mtcfn,
		struct moc_value value) {
	struct moc_matcher m = moc_emptymtc;
	MOC_IMTC(&m)->mval = value;
	MOC_IMTC(&m)->mtcfn.prm = mtcfn;
	MOC_IMTC(&m)->mopts = 128;
	return m;
}

struct moc_matcher moc_xparam(int nparam,
		moc_mtcfn_param_t mtcfn, struct moc_value value) {
	struct moc_matcher m = moc_emptymtc;
	MOC_IMTC(&m)->mval = value;
	MOC_IMTC(&m)->mtcfn.prm = mtcfn;
	MOC_IMTC(&m)->mopts = (MOC_OPTS_T) (nparam <= 0
			|| nparam >= MOC_MAXPARAMS ?
				MOC_MAXPARAMS : nparam);
	return m;
}

struct moc_matcher moc_xparam_nochk(int nparam,
		moc_mtcfn_param_t mtcfn, struct moc_value value) {
	struct moc_matcher m = moc_emptymtc;
	MOC_IMTC(&m)->mval = value;
	MOC_IMTC(&m)->mtcfn.prm = mtcfn;
	MOC_IMTC(&m)->mopts = (MOC_OPTS_T) (nparam <= 0
			|| nparam >= MOC_MAXPARAMS ?
				MOC_MAXPARAMS : 128 + nparam);
	return m;
}

struct moc_matcher moc_xcall(moc_mtcfn_call_t mtcfn,
		struct moc_value value) {
	struct moc_matcher m = moc_emptymtc;
	MOC_IMTC(&m)->mval = value;
	MOC_IMTC(&m)->mtcfn.cll = mtcfn;
	MOC_IMTC(&m)->mopts = 128 + MOC_MAXPARAMS;
	return m;
}

static moc_bool moc_cmpeqstr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) == 0;
}

static moc_bool moc_cmpnestr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) != 0;
}

static moc_bool moc_cmpltstr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) < 0;
}

static moc_bool moc_cmplestr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) <= 0;
}

static moc_bool moc_cmpgtstr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) > 0;
}

static moc_bool moc_cmpgestr(struct moc_value val1,
		struct moc_value val2) {
	return moc_strcmp(moc_get_cp_c(val1), moc_get_cp_c(val2)) >= 0;
}

static moc_bool moc_hassubstr(struct moc_value val1,
		struct moc_value val2) {
	unsigned long idx;
	const char *str1, *str2;
	str1 = moc_get_cp_c(val1);
	str2 = moc_get_cp_c(val2);
	idx = moc_substridx(str1, str2);
	return (str2[0] == '\0' || str1[idx] != '\0');
}

struct moc_matcher moc_eq(struct moc_value value) {
	return moc_mparam(moc_cmpeq, value);
}

struct moc_matcher moc_ne(struct moc_value value) {
	return moc_mparam(moc_cmpne, value);
}

struct moc_matcher moc_lt(struct moc_value value) {
	return moc_mparam(moc_cmplt, value);
}

struct moc_matcher moc_le(struct moc_value value) {
	return moc_mparam(moc_cmple, value);
}

struct moc_matcher moc_gt(struct moc_value value) {
	return moc_mparam(moc_cmpgt, value);
}

struct moc_matcher moc_ge(struct moc_value value) {
	return moc_mparam(moc_cmpge, value);
}

struct moc_matcher moc_eq_str(char *str) {
	return moc_mparam(moc_cmpeqstr, moc_p_c(str));
}

struct moc_matcher moc_ne_str(char *str) {
	return moc_mparam(moc_cmpnestr, moc_p_c(str));
}

struct moc_matcher moc_lt_str(char *str) {
	return moc_mparam(moc_cmpltstr, moc_p_c(str));
}

struct moc_matcher moc_le_str(char *str) {
	return moc_mparam(moc_cmplestr, moc_p_c(str));
}

struct moc_matcher moc_gt_str(char *str) {
	return moc_mparam(moc_cmpgtstr, moc_p_c(str));
}

struct moc_matcher moc_ge_str(char *str) {
	return moc_mparam(moc_cmpgestr, moc_p_c(str));
}

struct moc_matcher moc_eq_cstr(const char *str) {
	return moc_mparam(moc_cmpeqstr, moc_cp_c(str));
}

struct moc_matcher moc_ne_cstr(const char *str) {
	return moc_mparam(moc_cmpnestr, moc_cp_c(str));
}

struct moc_matcher moc_lt_cstr(const char *str) {
	return moc_mparam(moc_cmpltstr, moc_cp_c(str));
}

struct moc_matcher moc_le_cstr(const char *str) {
	return moc_mparam(moc_cmplestr, moc_cp_c(str));
}

struct moc_matcher moc_gt_cstr(const char *str) {
	return moc_mparam(moc_cmpgtstr, moc_cp_c(str));
}

struct moc_matcher moc_ge_cstr(const char *str) {
	return moc_mparam(moc_cmpgestr, moc_cp_c(str));
}

struct moc_matcher moc_substr(char *str) {
	return moc_mparam(moc_hassubstr, moc_p_c(str));
}

struct moc_matcher moc_csubstr(const char *str) {
	return moc_mparam(moc_hassubstr, moc_cp_c(str));
}

struct moc_matcher moc_any(void) {
	return moc_mparam_nochk(moc_mtctrue, moc_emptyval);
}

struct moc_matcher moc_any_c(void) {
	return moc_mparam(moc_mtctrue, moc_c(0));
}

struct moc_matcher moc_any_s(void) {
	return moc_mparam(moc_mtctrue, moc_s(0));
}

struct moc_matcher moc_any_i(void) {
	return moc_mparam(moc_mtctrue, moc_i(0));
}

struct moc_matcher moc_any_l(void) {
	return moc_mparam(moc_mtctrue, moc_l(0));
}

struct moc_matcher moc_any_f(void) {
	return moc_mparam(moc_mtctrue, moc_f(0));
}

struct moc_matcher moc_any_d(void) {
	return moc_mparam(moc_mtctrue, moc_d(0));
}

struct moc_matcher moc_any_sc(void) {
	return moc_mparam(moc_mtctrue, moc_sc(0));
}

struct moc_matcher moc_any_uc(void) {
	return moc_mparam(moc_mtctrue, moc_uc(0));
}

struct moc_matcher moc_any_us(void) {
	return moc_mparam(moc_mtctrue, moc_us(0));
}

struct moc_matcher moc_any_ui(void) {
	return moc_mparam(moc_mtctrue, moc_ui(0));
}

struct moc_matcher moc_any_ul(void) {
	return moc_mparam(moc_mtctrue, moc_ul(0));
}

struct moc_matcher moc_any_fn(void) {
	return moc_mparam(moc_mtctrue, moc_fn((moc_fnptr) moc_mtctrue));
}

struct moc_matcher moc_any_p(void) {
	return moc_mparam(moc_mtctrue, moc_p(MOC_NULL));
}

struct moc_matcher moc_any_p_c(void) {
	return moc_mparam(moc_mtctrue, moc_p_c((char *) MOC_NULL));
}

struct moc_matcher moc_any_p_s(void) {
	return moc_mparam(moc_mtctrue, moc_p_s((short *) MOC_NULL));
}

struct moc_matcher moc_any_p_i(void) {
	return moc_mparam(moc_mtctrue, moc_p_i((int *) MOC_NULL));
}

struct moc_matcher moc_any_p_l(void) {
	return moc_mparam(moc_mtctrue, moc_p_l((long *) MOC_NULL));
}

struct moc_matcher moc_any_p_f(void) {
	return moc_mparam(moc_mtctrue, moc_p_f((float *) MOC_NULL));
}

struct moc_matcher moc_any_p_d(void) {
	return moc_mparam(moc_mtctrue, moc_p_d((double *) MOC_NULL));
}

struct moc_matcher moc_any_p_sc(void) {
	return moc_mparam(moc_mtctrue, moc_p_sc((signed char *)
				MOC_NULL));
}

struct moc_matcher moc_any_p_uc(void) {
	return moc_mparam(moc_mtctrue, moc_p_uc((unsigned char *)
				MOC_NULL));
}

struct moc_matcher moc_any_p_us(void) {
	return moc_mparam(moc_mtctrue, moc_p_us((unsigned short *)
				MOC_NULL));
}

struct moc_matcher moc_any_p_ui(void) {
	return moc_mparam(moc_mtctrue, moc_p_ui((unsigned int *)
				MOC_NULL));
}

struct moc_matcher moc_any_p_ul(void) {
	return moc_mparam(moc_mtctrue, moc_p_ul((unsigned long *)
				MOC_NULL));
}

struct moc_matcher moc_any_p_fn(void) {
	return moc_mparam(moc_mtctrue, moc_p_fn((moc_fnptr *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp(void) {
	return moc_mparam(moc_mtctrue, moc_cp(MOC_NULL));
}

struct moc_matcher moc_any_cp_c(void) {
	return moc_mparam(moc_mtctrue, moc_cp_c((char *) MOC_NULL));
}

struct moc_matcher moc_any_cp_s(void) {
	return moc_mparam(moc_mtctrue, moc_cp_s((short *) MOC_NULL));
}

struct moc_matcher moc_any_cp_i(void) {
	return moc_mparam(moc_mtctrue, moc_cp_i((int *) MOC_NULL));
}

struct moc_matcher moc_any_cp_l(void) {
	return moc_mparam(moc_mtctrue, moc_cp_l((long *) MOC_NULL));
}

struct moc_matcher moc_any_cp_f(void) {
	return moc_mparam(moc_mtctrue, moc_cp_f((float *) MOC_NULL));
}

struct moc_matcher moc_any_cp_d(void) {
	return moc_mparam(moc_mtctrue, moc_cp_d((double *) MOC_NULL));
}

struct moc_matcher moc_any_cp_sc(void) {
	return moc_mparam(moc_mtctrue, moc_cp_sc((signed char *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp_uc(void) {
	return moc_mparam(moc_mtctrue, moc_cp_uc((unsigned char *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp_us(void) {
	return moc_mparam(moc_mtctrue, moc_cp_us((unsigned short *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp_ui(void) {
	return moc_mparam(moc_mtctrue, moc_cp_ui((unsigned int *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp_ul(void) {
	return moc_mparam(moc_mtctrue, moc_cp_ul((unsigned long *)
				MOC_NULL));
}

struct moc_matcher moc_any_cp_fn(void) {
	return moc_mparam(moc_mtctrue, moc_cp_fn((moc_fnptr *)
				MOC_NULL));
}

struct moc_responder moc_rparam(int nparam,
		moc_rspfn_param_t rspfn, struct moc_value val) {
	struct moc_responder r = moc_emptyrsp;
	MOC_IRSP(&r)->rspfn.prm = rspfn;
	MOC_IRSP(&r)->rval = val;
	MOC_IRSP(&r)->ropts = (MOC_OPTS_T) (nparam <= 0
			|| nparam >= MOC_MAXPARAMS ?
				MOC_MAXPARAMS : nparam);
	return r;
}

struct moc_responder moc_rparam_nochk(int nparam,
		moc_rspfn_param_t rspfn, struct moc_value val) {
	struct moc_responder r = moc_emptyrsp;
	MOC_IRSP(&r)->rspfn.prm = rspfn;
	MOC_IRSP(&r)->rval = val;
	MOC_IRSP(&r)->ropts = (MOC_OPTS_T) (nparam <= 0
			|| nparam >= MOC_MAXPARAMS ?
				MOC_MAXPARAMS : 128 + nparam);
	return r;
}

struct moc_responder moc_rcall(moc_rspfn_call_t rspfn,
		struct moc_value val) {
	struct moc_responder r = moc_emptyrsp;
	MOC_IRSP(&r)->rspfn.cll = rspfn;
	MOC_IRSP(&r)->rval = val;
	MOC_IRSP(&r)->ropts = 128 + MOC_MAXPARAMS;
	return r;
}

static struct moc_value moc_rsprt(struct moc_call *call,
		struct moc_value val) {
	if (sizeof(call)) {} /* unused warning */
	return val;
}

struct moc_responder moc_return(struct moc_value val) {
	return moc_rcall(&moc_rsprt, val);
}

static struct moc_value moc_rspinc(struct moc_call *call,
		struct moc_value val) {
	if (sizeof(call)) {} /* unused warning */
	if (MOC_VALPTRTYPE(val) == MOC_PTR) {
		switch (MOC_VALSTDTYPE(val)) {
			case MOC_CHR: (*moc_get_p_c(val))++; break;
			case MOC_SHR: (*moc_get_p_s(val))++; break;
			case MOC_INT: (*moc_get_p_i(val))++; break;
			case MOC_LNG: (*moc_get_p_l(val))++; break;
			case MOC_FLT: (*moc_get_p_f(val))++; break;
			case MOC_DBL: (*moc_get_p_d(val))++; break;
			case MOC_SCHR: (*moc_get_p_sc(val))++; break;
			case MOC_UCHR: (*moc_get_p_uc(val))++; break;
			case MOC_USHR: (*moc_get_p_us(val))++; break;
			case MOC_UINT: (*moc_get_p_ui(val))++; break;
			case MOC_ULNG: (*moc_get_p_ul(val))++; break;
			default: break;
		}
	}
	return moc_emptyval;
}

struct moc_responder moc_count(struct moc_value ptr) {
	return moc_rcall(&moc_rspinc, ptr);
}

struct moc_matchers_grp moc_init_matchers_grp(unsigned char nelems,
		struct moc_matcher *elems) {
	struct moc_matchers_grp v;
	v.nelems = nelems;
	v.elems = elems;
	return v;
}

struct moc_matchers_grp moc_match_0(void) {
	return moc_init_matchers_grp(0, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_1(struct moc_matcher mtc1) {
	moc_ctx.auxmatcs[0] = mtc1;
	return moc_init_matchers_grp(1, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_2(struct moc_matcher mtc1,
		struct moc_matcher mtc2) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	return moc_init_matchers_grp(2, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_3(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	moc_ctx.auxmatcs[2] = mtc3;
	return moc_init_matchers_grp(3, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_4(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	moc_ctx.auxmatcs[2] = mtc3;
	moc_ctx.auxmatcs[3] = mtc4;
	return moc_init_matchers_grp(4, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_5(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	moc_ctx.auxmatcs[2] = mtc3;
	moc_ctx.auxmatcs[3] = mtc4;
	moc_ctx.auxmatcs[4] = mtc5;
	return moc_init_matchers_grp(5, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_6(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	moc_ctx.auxmatcs[2] = mtc3;
	moc_ctx.auxmatcs[3] = mtc4;
	moc_ctx.auxmatcs[4] = mtc5;
	moc_ctx.auxmatcs[5] = mtc6;
	return moc_init_matchers_grp(6, moc_ctx.auxmatcs);
}

struct moc_matchers_grp moc_match_7(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6, struct moc_matcher mtc7) {
	moc_ctx.auxmatcs[0] = mtc1;
	moc_ctx.auxmatcs[1] = mtc2;
	moc_ctx.auxmatcs[2] = mtc3;
	moc_ctx.auxmatcs[3] = mtc4;
	moc_ctx.auxmatcs[4] = mtc5;
	moc_ctx.auxmatcs[5] = mtc6;
	moc_ctx.auxmatcs[6] = mtc7;
	return moc_init_matchers_grp(7, moc_ctx.auxmatcs);
}

struct moc_xmatchers_grp moc_init_xmatchers_grp(unsigned char nelems,
		struct moc_matcher *elems) {
	struct moc_xmatchers_grp v;
	v.nelems = nelems;
	v.elems = elems;
	return v;
}

struct moc_xmatchers_grp moc_xmatch_0(void) {
	return moc_init_xmatchers_grp(0, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_1(struct moc_matcher mtc1) {
	moc_ctx.auxxmatcs[0] = mtc1;
	return moc_init_xmatchers_grp(1, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_2(struct moc_matcher mtc1,
		struct moc_matcher mtc2) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	return moc_init_xmatchers_grp(2, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_3(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	moc_ctx.auxxmatcs[2] = mtc3;
	return moc_init_xmatchers_grp(3, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_4(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	moc_ctx.auxxmatcs[2] = mtc3;
	moc_ctx.auxxmatcs[3] = mtc4;
	return moc_init_xmatchers_grp(4, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_5(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	moc_ctx.auxxmatcs[2] = mtc3;
	moc_ctx.auxxmatcs[3] = mtc4;
	moc_ctx.auxxmatcs[4] = mtc5;
	return moc_init_xmatchers_grp(5, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_6(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	moc_ctx.auxxmatcs[2] = mtc3;
	moc_ctx.auxxmatcs[3] = mtc4;
	moc_ctx.auxxmatcs[4] = mtc5;
	moc_ctx.auxxmatcs[5] = mtc6;
	return moc_init_xmatchers_grp(6, moc_ctx.auxxmatcs);
}

struct moc_xmatchers_grp moc_xmatch_7(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6, struct moc_matcher mtc7) {
	moc_ctx.auxxmatcs[0] = mtc1;
	moc_ctx.auxxmatcs[1] = mtc2;
	moc_ctx.auxxmatcs[2] = mtc3;
	moc_ctx.auxxmatcs[3] = mtc4;
	moc_ctx.auxxmatcs[4] = mtc5;
	moc_ctx.auxxmatcs[5] = mtc6;
	moc_ctx.auxxmatcs[6] = mtc7;
	return moc_init_xmatchers_grp(7, moc_ctx.auxxmatcs);
}

struct moc_responders_grp moc_init_responders_grp(unsigned char nelems,
		struct moc_responder *elems) {
	struct moc_responders_grp v;
	v.nelems = nelems;
	v.elems = elems;
	return v;
}

struct moc_responders_grp moc_respond_0(void) {
	return moc_init_responders_grp(0, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_1(struct moc_responder rsp1) {
	moc_ctx.auxresps[0] = rsp1;
	return moc_init_responders_grp(1, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_2(struct moc_responder rsp1,
		struct moc_responder rsp2) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	return moc_init_responders_grp(2, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_3(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	moc_ctx.auxresps[2] = rsp3;
	return moc_init_responders_grp(3, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_4(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	moc_ctx.auxresps[2] = rsp3;
	moc_ctx.auxresps[3] = rsp4;
	return moc_init_responders_grp(4, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_5(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	moc_ctx.auxresps[2] = rsp3;
	moc_ctx.auxresps[3] = rsp4;
	moc_ctx.auxresps[4] = rsp5;
	return moc_init_responders_grp(5, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_6(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5,
		struct moc_responder rsp6) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	moc_ctx.auxresps[2] = rsp3;
	moc_ctx.auxresps[3] = rsp4;
	moc_ctx.auxresps[4] = rsp5;
	moc_ctx.auxresps[5] = rsp6;
	return moc_init_responders_grp(6, moc_ctx.auxresps);
}

struct moc_responders_grp moc_respond_7(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5,
		struct moc_responder rsp6, struct moc_responder rsp7) {
	moc_ctx.auxresps[0] = rsp1;
	moc_ctx.auxresps[1] = rsp2;
	moc_ctx.auxresps[2] = rsp3;
	moc_ctx.auxresps[3] = rsp4;
	moc_ctx.auxresps[4] = rsp5;
	moc_ctx.auxresps[5] = rsp6;
	moc_ctx.auxresps[6] = rsp7;
	return moc_init_responders_grp(7, moc_ctx.auxresps);
}

struct moc_values_grp moc_init_values_grp(unsigned char nelems,
		struct moc_value *elems) {
	struct moc_values_grp v;
	v.nelems = nelems;
	v.elems = elems;
	return v;
}

struct moc_values_grp moc_values_0(void) {
	return moc_init_values_grp(0, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_1(struct moc_value val1) {
	moc_ctx.auxvals[0] = val1;
	return moc_init_values_grp(1, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_2(struct moc_value val1,
		struct moc_value val2) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	return moc_init_values_grp(2, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_3(struct moc_value val1,
		struct moc_value val2, struct moc_value val3) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	moc_ctx.auxvals[2] = val3;
	return moc_init_values_grp(3, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_4(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	moc_ctx.auxvals[2] = val3;
	moc_ctx.auxvals[3] = val4;
	return moc_init_values_grp(4, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_5(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	moc_ctx.auxvals[2] = val3;
	moc_ctx.auxvals[3] = val4;
	moc_ctx.auxvals[4] = val5;
	return moc_init_values_grp(5, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_6(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5,
		struct moc_value val6) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	moc_ctx.auxvals[2] = val3;
	moc_ctx.auxvals[3] = val4;
	moc_ctx.auxvals[4] = val5;
	moc_ctx.auxvals[5] = val6;
	return moc_init_values_grp(6, moc_ctx.auxvals);
}

struct moc_values_grp moc_values_7(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5,
		struct moc_value val6, struct moc_value val7) {
	moc_ctx.auxvals[0] = val1;
	moc_ctx.auxvals[1] = val2;
	moc_ctx.auxvals[2] = val3;
	moc_ctx.auxvals[3] = val4;
	moc_ctx.auxvals[4] = val5;
	moc_ctx.auxvals[5] = val6;
	moc_ctx.auxvals[6] = val7;
	return moc_init_values_grp(7, moc_ctx.auxvals);
}

/* Functions to use single-linked lists with a pointer to the end: */

static void moc_inilist(struct moc_list *lst) {
	lst->first = lst->last = MOC_NULLNODE;
}

static void moc_inilistnode(struct moc_listnode *node, void *item,
		MOC_NUM_T nitems) {
	node->item = item;
	node->next = MOC_NULLNODE;
	node->nitems = nitems;
}

static void moc_inslastlistnode(struct moc_list *lst,
		struct moc_listnode *node) {
	if (lst->first == MOC_NULLNODE) {
		lst->first = lst->last = node;
	} else {
		lst->last->next = node;
		lst->last = node;
	}
}

static struct moc_listnode *moc_delfirstlistnode(struct moc_list *lst) {
	struct moc_listnode *node;
	node = lst->first;
	lst->first = node->next;
	if (lst->first == MOC_NULLNODE) {
		lst->last = MOC_NULLNODE;
	} else {
		node->next = MOC_NULLNODE;
	}
	return node;
}

/*
 * Current internal structure of the mocking-related data:
 *
 * Functions'array|Mappings'list|Matchers'array|Responders'arrays'list
 *  __________     _________
 * |"f1"/2/ *-|-->|_*_/1/_*-|-->...
 * |     ^    |    |    _________
 * |  nparams |    '-->|_*_/_*_/1|
 * |__________|          |   |    _______ _______ ________
 * |"f2"/3/ *-|-->...    |   '-->|mtc1/v1|mtc2/v2|xmtc1/v3|
 * |__________|          |                    _________
 * |...       |          '------------------>|_*_/2/_*-|-->...
 *                                             |    _______ _______
 *                                             '-->|rsp1/v1|rsp2/v2|
 */

/* Returns true if the given value is in the valid range. */
static moc_bool moc_isvalidtype(moc_type type) {
	int stdtype, ptrtype;
	stdtype = MOC_STDTYPE(type);
	ptrtype = MOC_PTRTYPE(type);
	return (stdtype >= MOC_VOID && stdtype <= MOC_FUN
			&& ptrtype >= MOC_NOPTR && ptrtype <= MOC_CPTR);
}

static moc_bool moc_matchers_eq(struct moc_matcher *m1,
			struct moc_matcher *m2) {
	struct moc_imatcher *im1, *im2;
	im1 = MOC_IMTC(m1);
	im2 = MOC_IMTC(m2);
	if (im1->mopts != im2->mopts) {
		return moc_false;
	}
	if (im1->mopts) {
		if (im1->mtcfn.prm != im2->mtcfn.prm) {
			return moc_false;
		}
	} else if (im1->mtcfn.cll != im2->mtcfn.cll) {
		return moc_false;
	}
	if (! moc_values_eq(im1->mval, im2->mval)) {
		return moc_false;
	}
	return moc_true;
}

static void moc_given_nnn(const char *funcname, MOC_NUM_T nmatchers,
		struct moc_matcher *matchers, MOC_NUM_T nxmatchers,
		struct moc_matcher *xmatchers, MOC_NUM_T nresponders,
		struct moc_responder *responders) {
	struct moc_function *func;
	struct moc_mapping *map;
	struct moc_responder *resps;
	struct moc_listnode *mnode, *rnode;
	MOC_SIZE_T m, r;
	moc_type type;
	MOC_SIZE_T nf, f, nfuncsinc = 0, pos;
	/* Searches the function by name and nparams: */
	nf = moc_ctx.nfuncs;
	for (f = 0; f < nf; f++) {
		if (moc_ctx.funcs[f].nparams == nmatchers
				&& moc_strcmp(moc_ctx.funcs[f].name,
					funcname) == 0) {
			break; /* function found */
		}
	}
	if (f < nf) {
		/* Searches the mapping node with equal matchers: */
		func = moc_ctx.funcs + f;
		mnode = func->lmaps.first;
		while (mnode != MOC_NULLNODE) {
			map = (struct moc_mapping *) mnode->item;
			if (nxmatchers != map->nxmatchers) {
				mnode = mnode->next;
				continue;
			}
			for (m = 0; m < nmatchers + nxmatchers; m++) {
				if (moc_matchers_eq(map->matchers + m,
						matchers + m)) {
					break; /* mapping node found */
				}
			}
			if (m < nmatchers + nxmatchers) {
				break; /* mapping node found */
			}
			mnode = mnode->next;
		}
	} else {
		/* Adds name and nparams to the first free function: */
		if (nf == moc_ctx.maxfuncs) {
			moc_send_error(MOC_ERR_NFUNLIMIT, funcname, 0,
					0, 0);
			return;
		}
		func = moc_ctx.funcs + nf;
		func->name = funcname;
		func->nparams = nmatchers;
		moc_inilist(&(func->lmaps));
		mnode = MOC_NULLNODE;
		nfuncsinc++; /* to remember increasing it */
	}
	/* Checks if there is enough memory to add the mapping: */
	if (mnode == MOC_NULLNODE && moc_ctx.nmaps == moc_ctx.maxmaps) {
		moc_send_error(MOC_ERR_NMAPLIMIT, funcname, 0, 0, 0);
		return;
	}
	if (mnode == MOC_NULLNODE && moc_ctx.maxmatcs - moc_ctx.nmatcs
			< nmatchers + nxmatchers) {
		moc_send_error(MOC_ERR_NMTCLIMIT, funcname, 0, 0, 0);
		return;
	}
	if (moc_ctx.maxlnods - moc_ctx.nlnods
			< (mnode == MOC_NULLNODE ? 1 : 0) + 1) {
		moc_send_error(MOC_ERR_NNODLIMIT, funcname, 0, 0, 0);
		return;
	}
	if (moc_ctx.maxresps - moc_ctx.nresps < nresponders) {
		moc_send_error(MOC_ERR_NRSPLIMIT, funcname, 0, 0, 0);
		return;
	}
	pos = 1;
	for (m = 0; m < nmatchers; m++, pos++) {
		type = MOC_VALBYTE(MOC_IMTC(matchers + m)->mval);
		if (! moc_isvalidtype(type)) {
			moc_send_error(MOC_ERR_INVALTYPE, funcname, pos,
					type, type);
			return;
		}
	}
	for (m = 0; m < nxmatchers; m++, pos++) {
		type = MOC_VALBYTE(MOC_IMTC(xmatchers + m)->mval);
		if (! moc_isvalidtype(type)) {
			moc_send_error(MOC_ERR_INVALTYPE, funcname, pos,
					type, type);
			return;
		}
	}
	for (r = 0; r < nresponders; r++, pos++) {
		type = MOC_VALBYTE(MOC_IRSP(responders + r)->rval);
		if (! moc_isvalidtype(type)) {
			moc_send_error(MOC_ERR_INVALTYPE, funcname, pos,
					type, type);
			return;
		}
	}
	moc_ctx.nfuncs += nfuncsinc;
	if (mnode == MOC_NULLNODE) {
		/* Adds matchers to a new mapping inserted the last: */
		map = moc_ctx.maps + moc_ctx.nmaps;
		moc_ctx.nmaps++;
		mnode = moc_ctx.lnods + moc_ctx.nlnods;
		moc_ctx.nlnods++;
		moc_inilistnode(mnode, map, 1);
		moc_inslastlistnode(&(func->lmaps), mnode);
		map->matchers = moc_ctx.matcs + moc_ctx.nmatcs;
		map->nxmatchers = nxmatchers;
		moc_ctx.nmatcs += nmatchers + nxmatchers;
		for (m = 0; m < nmatchers; m++) {
			map->matchers[m] = matchers[m];
		}
		for (m = 0; m < nxmatchers; m++) {
			map->matchers[m + nmatchers] = xmatchers[m];
		}
		moc_inilist(&(map->lresps));
	} else {
		map = (struct moc_mapping *) mnode->item;
	}
	/* Adds the responders to the end of the list of nodes: */
	resps = moc_ctx.resps + moc_ctx.nresps;
	moc_ctx.nresps += nresponders;
	rnode = moc_ctx.lnods + moc_ctx.nlnods;
	moc_ctx.nlnods++;
	moc_inilistnode(rnode, resps, nresponders);
	moc_inslastlistnode(&(map->lresps), rnode);
	for (r = 0; r < nresponders; r++) {
		resps[r] = responders[r];
	}
}

static moc_bool moc_iscmpfn(struct moc_matcher *mtc) {
	moc_mtcfn_param_t fn;
	fn = MOC_IMTC(mtc)->mtcfn.prm;
	return (fn == moc_cmplt || fn == moc_cmple
		|| fn == moc_cmpgt || (fn == moc_cmpge
			? moc_true : moc_false));
}

static moc_bool moc_chkmtc(struct moc_matcher *pm, int pos,
		const char *funcname, unsigned char nparams,
		struct moc_value *params) {
	MOC_OPTS_T opts;
	moc_type type;
	opts = MOC_IMTC(pm)->mopts;
	if ((pos <= nparams && opts != 0 && opts != 128)
	|| (pos > nparams && (opts == 0 || opts == 128))) {
		moc_send_error(MOC_ERR_INVALMTCH, funcname, pos, 0, 0);
		return moc_false;
	}
	if (pos > nparams && (opts == MOC_MAXPARAMS
			|| (opts < MOC_MAXPARAMS && opts > nparams)
			|| (opts > 128 && opts < 128 + MOC_MAXPARAMS
					&& opts - 128 > nparams))) {
		moc_send_error(MOC_ERR_INVNPARAM, funcname, pos, 0, 0);
		return moc_false;
	}
	type = MOC_VALBYTE(MOC_IMTC(pm)->mval);
	if (! moc_isvalidtype(type)) {
		moc_send_error(MOC_ERR_INVALTYPE, funcname, pos,
				type, type);
		return moc_false;
	}
	if (type == MOC_TYPES2BYTE(MOC_FUN, MOC_NOPTR)
			&& moc_iscmpfn(pm)) {
		moc_send_error(MOC_ERR_INVALOPER, funcname, pos, 0, 0);
		return moc_false;
	}
	if (pos <= nparams && opts < MOC_MAXPARAMS
			&& type != MOC_VALBYTE(params[pos - 1])) {
		moc_send_error(MOC_ERR_PARAMTYPE, funcname, pos,
				MOC_VALBYTE(params[pos - 1]), type);
		return moc_false;
	}
	if (pos > nparams && opts < MOC_MAXPARAMS
			&& type != MOC_VALBYTE(params[opts - 1])) {
		moc_send_error(MOC_ERR_PARAMTYPE, funcname, pos,
				MOC_VALBYTE(params[opts - 1]), type);
		return moc_false;
	}
	return moc_true;
}

static moc_bool moc_chkrsp(struct moc_responder *pr, int pos,
		const char *funcname, unsigned char nparams,
		struct moc_value *params) {
	MOC_OPTS_T opts;
	moc_type type;
	opts = MOC_IRSP(pr)->ropts;
	if (opts == 0 || opts == MOC_MAXPARAMS
			|| (opts < MOC_MAXPARAMS && opts > nparams)
			|| (opts > 128 && opts < 128 + MOC_MAXPARAMS
					&& opts - 128 > nparams)) {
		moc_send_error(MOC_ERR_INVNPARAM, funcname, pos, 0, 0);
		return moc_false;
	}
	type = MOC_VALBYTE(MOC_IRSP(pr)->rval);
	if (! moc_isvalidtype(type)) {
		moc_send_error(MOC_ERR_INVALTYPE, funcname, pos,
				type, type);
		return moc_false;
	}
	if (opts < MOC_MAXPARAMS
			&& type != MOC_VALBYTE(params[opts - 1])) {
		moc_send_error(MOC_ERR_PARAMTYPE, funcname, pos,
				MOC_VALBYTE(params[opts - 1]), type);
		return moc_false;
	}
	return moc_true;
}

static struct moc_value moc_act_n(const char *funcname, moc_type rettype,
		unsigned char nparams, struct moc_value *params) {
	struct moc_listnode *mnode, *rnode;
	struct moc_matcher *pm;
	struct moc_responder *pr, *responders;
	struct moc_mapping *map;
	struct moc_value retval;
	struct moc_call call;
	MOC_SIZE_T nf, f, m, r;
	MOC_OPTS_T opts;
	int i;
	/* Searches the function by name and nparams: */
	nf = moc_ctx.nfuncs;
	for (f = 0; f < nf; f++) {
		if (moc_ctx.funcs[f].nparams == nparams
				&& moc_strcmp(moc_ctx.funcs[f].name,
					funcname) == 0) {
			break;
		}
	}
	if (f == nf) {
		moc_send_error(MOC_ERR_FUNNOTFND, funcname, nparams,
				0, 0);
		return moc_emptyval;
	}
	/* Searches a mapping node that matches all the matchers: */
	mnode = moc_ctx.funcs[f].lmaps.first;
	while (mnode != MOC_NULLNODE) {
		map = (struct moc_mapping *) mnode->item;
		for (m = 0; m < nparams + map->nxmatchers; m++) {
			pm = map->matchers + m;
			if (! moc_chkmtc(pm, m + 1, funcname,
					nparams, params)) {
				return moc_emptyval;
			}
			opts = MOC_IMTC(pm)->mopts;
			if (opts == 128 + MOC_MAXPARAMS) {
				call.funcname = funcname;
				call.nparams = nparams;
				call.params = params;
				if (! MOC_IMTC(pm)->mtcfn.cll(&call,
						MOC_IMTC(pm)->mval)) {
					break; /* matchers don't match */
				}
			} else {
				if (m < nparams) {
					i = m;
				} else {
					i = opts > 128 ? opts - 128 - 1
						: opts - 1;
				}
				if (! MOC_IMTC(pm)->mtcfn.prm(params[i],
						MOC_IMTC(pm)->mval)) {
					break; /* matchers don't match */
				}
			}
		}
		if (m == nparams + map->nxmatchers) {
			break; /* matchers matched */
		}
		mnode = mnode->next;
	}
	if (mnode == MOC_NULLNODE) {
		moc_send_error(MOC_ERR_MAPNOTFND, funcname, nparams,
				0, 0);
		return moc_emptyval;
	}
	/* Checks the data of the responders before executing them: */
	rnode = map->lresps.first;
	responders = (struct moc_responder *) rnode->item;
	for (r = 0; r < rnode->nitems; r++) {
		if (! moc_chkrsp(responders + r, 1 + r + m, funcname,
					nparams, params)) {
			return moc_emptyval;
		}

	}
	/* Executes the first responders of the mapping: */
	retval = moc_emptyval; /* default value */
	for (r = 0; r < rnode->nitems; r++) {
		pr = responders + r;
		opts = MOC_IRSP(pr)->ropts;
		if (opts == 128 + MOC_MAXPARAMS) {
			call.funcname = funcname;
			call.nparams = nparams;
			call.params = params;
			retval = MOC_IRSP(pr)->rspfn.cll(&call,
						MOC_IRSP(pr)->rval);
		} else {
			i = opts > 128 ? opts - 128 - 1 : opts - 1;
			retval = MOC_IRSP(pr)->rspfn.prm(params[i],
						MOC_IRSP(pr)->rval);
		}
	}
	if (MOC_VALBYTE(retval) != rettype) {
		moc_send_error(MOC_ERR_RETURTYPE, funcname, 0,
				MOC_VALBYTE(retval), rettype);
	}
	/* Moves the first responders to the end of the list: */
	if(rnode->next != MOC_NULLNODE) {
		moc_inslastlistnode(&(map->lresps),
				moc_delfirstlistnode(&(map->lresps)));
	}
	return retval;
}

void moc_given(const char *funcname, struct moc_matchers_grp mgrp,
		struct moc_responders_grp rgrp) {
	moc_given_nnn(funcname, mgrp.nelems, mgrp.elems, 0, mgrp.elems,
			rgrp.nelems, rgrp.elems);
}

void moc_given_extra(const char *funcname, struct moc_matchers_grp mgrp,
		struct moc_xmatchers_grp mxgrp,
		struct moc_responders_grp rgrp) {
	moc_given_nnn(funcname, mgrp.nelems, mgrp.elems,
			mxgrp.nelems, mxgrp.elems,
			rgrp.nelems, rgrp.elems);
}

struct moc_value moc_act(const char *funcname, moc_type rettype,
		struct moc_values_grp pgrp) {
	return moc_act_n(funcname, rettype, pgrp.nelems, pgrp.elems);
}

