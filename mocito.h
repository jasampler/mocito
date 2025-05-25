/*
   Mocito is a mocking library for writing unit tests of C code.
   Copyright (C) 2025, Carlos Rica Espinosa <jasampler@gmail.com>

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

/**
 * \file mocito.h
 * Header file of the Mocito C mocking library for writing tests.
 */

#ifndef MOCITO_H
#define MOCITO_H

/**
 * Initializes the given memory block to store the mocking-related data.
 */
void moc_init(char *memblk, unsigned long size);

/**
 * Function that must be defined by the user for managing mocking-related
 * errors and mantain them separated from those of the code to be tested.
 * Can be implemented as: fprintf(stderr, "%s\n", moc_errmsg()); exit(1);
 */
void moc_error(void);

/**
 * Returns a readable message with the information about the last error:
 * error description, function name, position of parameter and type name.
 */
const char *moc_errmsg(void);

/**
 * Type of functions that can manage mocking-related errors.
 */
typedef void (*moc_errfn_t)(void);

/**
 * Allows to replace the function that will be called by default
 * in a test when a mocking-related error is detected in that test.
 */
void moc_set_errfn(moc_errfn_t errfn);

/**
 * Generic function pointer type for storing a pointer to any function.
 */
typedef void (*moc_fnptr)(void);

/**
 * Casts a function pointer to the generic pointer (to avoid warnings).
 */
#define MOC_FP(fn) ((moc_fnptr) (fn))

/**
 * Macro to convert a function name in a string, so you can write
 * MOC_FN(myfn) instead of "myfn" for preserving the IDE navigation.
 */
#define MOC_FN(fn) (#fn)

/**
 * Boolean type used in Mocito, using moc_false and moc_true as values.
 */
typedef int moc_bool;
#define moc_false 0
#define moc_true (!moc_false)

/**
 * Type of the identifiers of the supported data types.
 */
typedef unsigned char moc_type;

/**
 * Structure to store any value of a basic type including its type.
 */
struct moc_value {
	double opaque[2];
};

/* The type functions are used for providing the type of the return value
 * of the mock because it checks that the type of the value is equal: */

moc_type moc_type_void(void);
moc_type moc_type_c(void);
moc_type moc_type_s(void);
moc_type moc_type_i(void);
moc_type moc_type_l(void);
moc_type moc_type_f(void);
moc_type moc_type_d(void);
moc_type moc_type_sc(void);
moc_type moc_type_uc(void);
moc_type moc_type_us(void);
moc_type moc_type_ui(void);
moc_type moc_type_ul(void);
moc_type moc_type_fn(void);

moc_type moc_type_p(void);
moc_type moc_type_p_c(void);
moc_type moc_type_p_s(void);
moc_type moc_type_p_i(void);
moc_type moc_type_p_l(void);
moc_type moc_type_p_f(void);
moc_type moc_type_p_d(void);
moc_type moc_type_p_sc(void);
moc_type moc_type_p_uc(void);
moc_type moc_type_p_us(void);
moc_type moc_type_p_ui(void);
moc_type moc_type_p_ul(void);
moc_type moc_type_p_fn(void);

moc_type moc_type_cp(void);
moc_type moc_type_cp_c(void);
moc_type moc_type_cp_s(void);
moc_type moc_type_cp_i(void);
moc_type moc_type_cp_l(void);
moc_type moc_type_cp_f(void);
moc_type moc_type_cp_d(void);
moc_type moc_type_cp_sc(void);
moc_type moc_type_cp_uc(void);
moc_type moc_type_cp_us(void);
moc_type moc_type_cp_ui(void);
moc_type moc_type_cp_ul(void);
moc_type moc_type_cp_fn(void);

/* Functions to convert primitive values to Mocito values: */

struct moc_value moc_void(void);
struct moc_value moc_c(char val);
struct moc_value moc_s(short val);
struct moc_value moc_i(int val);
struct moc_value moc_l(long val);
struct moc_value moc_f(float val);
struct moc_value moc_d(double val);
struct moc_value moc_sc(signed char val);
struct moc_value moc_uc(unsigned char val);
struct moc_value moc_us(unsigned short val);
struct moc_value moc_ui(unsigned int val);
struct moc_value moc_ul(unsigned long val);
struct moc_value moc_fn(moc_fnptr fn);

struct moc_value moc_p(void *p);
struct moc_value moc_p_c(char *p);
struct moc_value moc_p_s(short *p);
struct moc_value moc_p_i(int *p);
struct moc_value moc_p_l(long *p);
struct moc_value moc_p_f(float *p);
struct moc_value moc_p_d(double *p);
struct moc_value moc_p_sc(signed char *p);
struct moc_value moc_p_uc(unsigned char *p);
struct moc_value moc_p_us(unsigned short *p);
struct moc_value moc_p_ui(unsigned int *p);
struct moc_value moc_p_ul(unsigned long *p);
struct moc_value moc_p_fn(moc_fnptr *p);

struct moc_value moc_cp(const void *p);
struct moc_value moc_cp_c(const char *p);
struct moc_value moc_cp_s(const short *p);
struct moc_value moc_cp_i(const int *p);
struct moc_value moc_cp_l(const long *p);
struct moc_value moc_cp_f(const float *p);
struct moc_value moc_cp_d(const double *p);
struct moc_value moc_cp_sc(const signed char *p);
struct moc_value moc_cp_uc(const unsigned char *p);
struct moc_value moc_cp_us(const unsigned short *p);
struct moc_value moc_cp_ui(const unsigned int *p);
struct moc_value moc_cp_ul(const unsigned long *p);
struct moc_value moc_cp_fn(const moc_fnptr *p);

/* Functions to extract a primitive value from a Mocito value: */

char   moc_get_c(struct moc_value value);
short  moc_get_s(struct moc_value value);
int    moc_get_i(struct moc_value value);
long   moc_get_l(struct moc_value value);
float  moc_get_f(struct moc_value value);
double moc_get_d(struct moc_value value);
signed char    moc_get_sc(struct moc_value value);
unsigned char  moc_get_uc(struct moc_value value);
unsigned short moc_get_us(struct moc_value value);
unsigned int   moc_get_ui(struct moc_value value);
unsigned long  moc_get_ul(struct moc_value value);
moc_fnptr moc_get_fn(struct moc_value value);

void   *moc_get_p(struct moc_value value);
char   *moc_get_p_c(struct moc_value value);
short  *moc_get_p_s(struct moc_value value);
int    *moc_get_p_i(struct moc_value value);
long   *moc_get_p_l(struct moc_value value);
float  *moc_get_p_f(struct moc_value value);
double *moc_get_p_d(struct moc_value value);
signed char    *moc_get_p_sc(struct moc_value value);
unsigned char  *moc_get_p_uc(struct moc_value value);
unsigned short *moc_get_p_us(struct moc_value value);
unsigned int   *moc_get_p_ui(struct moc_value value);
unsigned long  *moc_get_p_ul(struct moc_value value);
moc_fnptr *moc_get_p_fn(struct moc_value value);

const void   *moc_get_cp(struct moc_value value);
const char   *moc_get_cp_c(struct moc_value value);
const short  *moc_get_cp_s(struct moc_value value);
const int    *moc_get_cp_i(struct moc_value value);
const long   *moc_get_cp_l(struct moc_value value);
const float  *moc_get_cp_f(struct moc_value value);
const double *moc_get_cp_d(struct moc_value value);
const signed char    *moc_get_cp_sc(struct moc_value value);
const unsigned char  *moc_get_cp_uc(struct moc_value value);
const unsigned short *moc_get_cp_us(struct moc_value value);
const unsigned int   *moc_get_cp_ui(struct moc_value value);
const unsigned long  *moc_get_cp_ul(struct moc_value value);
const moc_fnptr *moc_get_cp_fn(struct moc_value value);

/**
 * Call-related data to be used optionally by matchers or responders.
 */
struct moc_call {
	const char *funcname;
	struct moc_value *params;
	unsigned char nparams;
};

/**
 * A matcher includes a function and a generic value to be used by it
 * and an additional field of options for different types or parameters.
 */
struct moc_matcher {
	double opaque[4];
};

/**
 * Type of the functions for matchers receiving one of the parameters
 * of the current call and the value saved in the matcher being invoked,
 * returning a boolean value indicating if the parameter matched or not.
 */
typedef moc_bool (*moc_mtcfn_param_t)(struct moc_value param,
		struct moc_value data);

/**
 * Type of the functions for matchers receiving all the parameters
 * of the current call and the value saved in the matcher being invoked,
 * returning a boolean value indicating if the parameters matched or not.
 */
typedef moc_bool (*moc_mtcfn_call_t)(struct moc_call *call,
		struct moc_value data);

/* Predefined functions for creating matchers: */

struct moc_matcher moc_eq(struct moc_value value);
struct moc_matcher moc_ne(struct moc_value value);
struct moc_matcher moc_lt(struct moc_value value);
struct moc_matcher moc_le(struct moc_value value);
struct moc_matcher moc_gt(struct moc_value value);
struct moc_matcher moc_ge(struct moc_value value);

struct moc_matcher moc_str_eq(char *str);
struct moc_matcher moc_str_ne(char *str);
struct moc_matcher moc_str_lt(char *str);
struct moc_matcher moc_str_le(char *str);
struct moc_matcher moc_str_gt(char *str);
struct moc_matcher moc_str_ge(char *str);

struct moc_matcher moc_cstr_eq(const char *str);
struct moc_matcher moc_cstr_ne(const char *str);
struct moc_matcher moc_cstr_lt(const char *str);
struct moc_matcher moc_cstr_le(const char *str);
struct moc_matcher moc_cstr_gt(const char *str);
struct moc_matcher moc_cstr_ge(const char *str);

struct moc_matcher moc_substr(char *str);
struct moc_matcher moc_csubstr(const char *str);

struct moc_matcher moc_any(void);
struct moc_matcher moc_any_c(void);
struct moc_matcher moc_any_s(void);
struct moc_matcher moc_any_i(void);
struct moc_matcher moc_any_l(void);
struct moc_matcher moc_any_f(void);
struct moc_matcher moc_any_d(void);
struct moc_matcher moc_any_sc(void);
struct moc_matcher moc_any_uc(void);
struct moc_matcher moc_any_us(void);
struct moc_matcher moc_any_ui(void);
struct moc_matcher moc_any_ul(void);
struct moc_matcher moc_any_fn(void);

struct moc_matcher moc_any_p(void);
struct moc_matcher moc_any_p_c(void);
struct moc_matcher moc_any_p_s(void);
struct moc_matcher moc_any_p_i(void);
struct moc_matcher moc_any_p_l(void);
struct moc_matcher moc_any_p_f(void);
struct moc_matcher moc_any_p_d(void);
struct moc_matcher moc_any_p_sc(void);
struct moc_matcher moc_any_p_uc(void);
struct moc_matcher moc_any_p_us(void);
struct moc_matcher moc_any_p_ui(void);
struct moc_matcher moc_any_p_ul(void);
struct moc_matcher moc_any_p_fn(void);

struct moc_matcher moc_any_cp(void);
struct moc_matcher moc_any_cp_c(void);
struct moc_matcher moc_any_cp_s(void);
struct moc_matcher moc_any_cp_i(void);
struct moc_matcher moc_any_cp_l(void);
struct moc_matcher moc_any_cp_f(void);
struct moc_matcher moc_any_cp_d(void);
struct moc_matcher moc_any_cp_sc(void);
struct moc_matcher moc_any_cp_uc(void);
struct moc_matcher moc_any_cp_us(void);
struct moc_matcher moc_any_cp_ui(void);
struct moc_matcher moc_any_cp_ul(void);
struct moc_matcher moc_any_cp_fn(void);

/**
 * Composes and returns a matcher that calls to the given function for
 * comparing a parameter with the given value, checking if type is equal.
 *
 * This type of matcher can only be used as ordinary matcher, not extra.
 */
struct moc_matcher moc_mparam(moc_mtcfn_param_t mtcfn,
		struct moc_value value);

/**
 * Composes and returns a matcher that calls to the given function for
 * comparing a parameter with the given value, but without checking type.
 *
 * This type of matcher can only be used as ordinary matcher, not extra.
 */
struct moc_matcher moc_mparam_nochk(moc_mtcfn_param_t mtcfn,
		struct moc_value value);

/**
 * Composes and returns a matcher that calls a given function to compare
 * the given parameter with the given value, checking if type is equal.
 *
 * This type of matcher can only be used as extra matcher, not ordinary.
 */
struct moc_matcher moc_xparam(int nparam,
		moc_mtcfn_param_t mtcfn, struct moc_value value);

/**
 * Composes and returns a matcher that calls a given function to compare
 * the given parameter with the given value, but without checking type.
 *
 * This type of matcher can only be used as extra matcher, not ordinary.
 */
struct moc_matcher moc_xparam_nochk(int nparam,
		moc_mtcfn_param_t mtcfn, struct moc_value value);

/**
 * Composes and returns a matcher that calls to the given function
 * passing to it all the parameters of the call and the given value.
 *
 * This type of matcher can only be used as extra matcher, not ordinary.
 */
struct moc_matcher moc_xcall(moc_mtcfn_call_t mtcfn,
		struct moc_value value);

/**
 * A responder includes a function and a generic value to be used by it
 * and an additional field of options for different types or parameters.
 */
struct moc_responder {
	double opaque[4];
};

/**
 * Type of the functions for responders receiving one of the parameters
 * of the call and the value saved in the responder being invoked, that
 * returns the value to return by the call when it is the last responder.
 */
typedef struct moc_value (*moc_rspfn_param_t)(struct moc_value param,
		struct moc_value data);

/**
 * Type of the functions for responders receiving all the parameters
 * of the call and the value saved in the responder being invoked, that
 * returns the value to return by the call when it is the last responder.
 */
typedef struct moc_value (*moc_rspfn_call_t)(struct moc_call *call,
		struct moc_value val);

/* Predefined functions for creating responders: */

/**
 * Composes and returns a responder that returns the given parameter.
 */
struct moc_responder moc_return(struct moc_value val);

/**
 * Returns a responder that increments the pointed variable on each call.
 */
struct moc_responder moc_count(struct moc_value ptr);

/**
 * Composes and returns a responder that calls the given function sending
 * the indicated parameter and the given value, checking if type is equal.
 */
struct moc_responder moc_rparam(int nparam,
		moc_rspfn_param_t rspfn, struct moc_value val);

/**
 * Composes and returns a responder that calls the given function sending
 * the indicated parameter and the given value, but without checking type.
 */
struct moc_responder moc_rparam_nochk(int nparam,
		moc_rspfn_param_t rspfn, struct moc_value val);

/**
 * Composes and returns a responder that calls to the given function
 * sending to it all the parameters of the call and the given value.
 */
struct moc_responder moc_rcall(moc_rspfn_call_t rspfn,
		struct moc_value val);

/** Structure for grouping a list of ordinary matchers. */
struct moc_matchers_grp {
	struct moc_matcher *elems;
	unsigned char nelems;
};

/** Helper to initialize and return a matchers group struct. */
struct moc_matchers_grp moc_init_matchers_grp(unsigned char nelems,
		struct moc_matcher *elems);

struct moc_matchers_grp moc_match_0(void);
struct moc_matchers_grp moc_match_1(struct moc_matcher mtc1);
struct moc_matchers_grp moc_match_2(struct moc_matcher mtc1,
		struct moc_matcher mtc2);
struct moc_matchers_grp moc_match_3(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3);
struct moc_matchers_grp moc_match_4(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4);
struct moc_matchers_grp moc_match_5(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5);
struct moc_matchers_grp moc_match_6(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6);
struct moc_matchers_grp moc_match_7(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6, struct moc_matcher mtc7);

/** Structure for grouping a list of extra matchers. */
struct moc_xmatchers_grp {
	struct moc_matcher *elems;
	unsigned char nelems;
};

/** Helper to initialize and return an extra matchers group struct. */
struct moc_xmatchers_grp moc_init_xmatchers_grp(unsigned char nelems,
		struct moc_matcher *elems);

struct moc_xmatchers_grp moc_xmatch_0(void);
struct moc_xmatchers_grp moc_xmatch_1(struct moc_matcher mtc1);
struct moc_xmatchers_grp moc_xmatch_2(struct moc_matcher mtc1,
		struct moc_matcher mtc2);
struct moc_xmatchers_grp moc_xmatch_3(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3);
struct moc_xmatchers_grp moc_xmatch_4(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4);
struct moc_xmatchers_grp moc_xmatch_5(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5);
struct moc_xmatchers_grp moc_xmatch_6(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6);
struct moc_xmatchers_grp moc_xmatch_7(struct moc_matcher mtc1,
		struct moc_matcher mtc2, struct moc_matcher mtc3,
		struct moc_matcher mtc4, struct moc_matcher mtc5,
		struct moc_matcher mtc6, struct moc_matcher mtc7);

/** Structure for grouping a list of responders. */
struct moc_responders_grp {
	struct moc_responder *elems;
	unsigned char nelems;
};

/** Helper to initialize and return a responders group struct. */
struct moc_responders_grp moc_init_responders_grp(unsigned char nelems,
		struct moc_responder *elems);

struct moc_responders_grp moc_respond_0(void);
struct moc_responders_grp moc_respond_1(struct moc_responder rsp1);
struct moc_responders_grp moc_respond_2(struct moc_responder rsp1,
		struct moc_responder rsp2);
struct moc_responders_grp moc_respond_3(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3);
struct moc_responders_grp moc_respond_4(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4);
struct moc_responders_grp moc_respond_5(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5);
struct moc_responders_grp moc_respond_6(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5,
		struct moc_responder rsp6);
struct moc_responders_grp moc_respond_7(struct moc_responder rsp1,
		struct moc_responder rsp2, struct moc_responder rsp3,
		struct moc_responder rsp4, struct moc_responder rsp5,
		struct moc_responder rsp6, struct moc_responder rsp7);

/** Structure for grouping a list of values. */
struct moc_values_grp {
	struct moc_value *elems;
	unsigned char nelems;
};

/** Helper to initialize and return a values group struct. */
struct moc_values_grp moc_init_values_grp(unsigned char nelems,
		struct moc_value *elems);

struct moc_values_grp moc_values_0(void);
struct moc_values_grp moc_values_1(struct moc_value val1);
struct moc_values_grp moc_values_2(struct moc_value val1,
		struct moc_value val2);
struct moc_values_grp moc_values_3(struct moc_value val1,
		struct moc_value val2, struct moc_value val3);
struct moc_values_grp moc_values_4(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4);
struct moc_values_grp moc_values_5(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5);
struct moc_values_grp moc_values_6(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5,
		struct moc_value val6);
struct moc_values_grp moc_values_7(struct moc_value val1,
		struct moc_value val2, struct moc_value val3,
		struct moc_value val4, struct moc_value val5,
		struct moc_value val6, struct moc_value val7);

/**
 * Adds a mapping for the given mocked function connecting a group of
 * matchers, one for each function parameter, with a group of responders.
 *
 * When that function is called, all its mappings are checked in order,
 * and if all the matchers of a mapping return true, then the responders
 * of that mapping will be executed, returning the value returned by the
 * last responder without checking the rest of the configured mappings.
 */
void moc_given(const char *funcname, struct moc_matchers_grp mgrp,
		struct moc_responders_grp rgrp);

/**
 * Adds a mapping for the given mocked function connecting a group of
 * matchers, one for each function parameter, with a group of responders,
 * but including extra matchers that will be executed after the others.
 *
 * When that function is called, all its mappings are checked in order,
 * and if all the matchers of a mapping return true, then the responders
 * of that mapping will be executed, returning the value returned by the
 * last responder without checking the rest of the configured mappings.
 */
void moc_given_extra(const char *funcname, struct moc_matchers_grp mgrp,
		struct moc_xmatchers_grp mxgrp,
		struct moc_responders_grp rgrp);

/**
 * Function that must be called from every mocked function for
 * executing the responders configured for the given parameters
 * and returning the value returned by the last of those responders
 * also checking if the type of that value is equal to the given type.
 */
struct moc_value moc_act(const char *funcname, moc_type rettype,
		struct moc_values_grp pgrp);

#endif /* MOCITO_H */
