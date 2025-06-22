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
 * Tests of mocks with matchers accepting any type of value.
 */

#include "mocito.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Create the default function to manage the mocking-related errors. */
void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }

long lfun1(char c) {
	return moc_get_l(moc_act(MOC_FN(lfun1), moc_type_l(),
			moc_values_1(moc_c(c))));
}

long lfun2(char c, short s) {
	return moc_get_l(moc_act(MOC_FN(lfun2), moc_type_l(),
			moc_values_2(moc_c(c), moc_s(s))));
}

long lfun3(char c, short s, int i) {
	return moc_get_l(moc_act(MOC_FN(lfun3), moc_type_l(),
			moc_values_3(moc_c(c), moc_s(s), moc_i(i))));
}

long lfun4(char c, short s, int i, long l) {
	return moc_get_l(moc_act(MOC_FN(lfun4), moc_type_l(),
			moc_values_4(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l))));
}

long lfun5(char c, short s, int i, long l, float f) {
	return moc_get_l(moc_act(MOC_FN(lfun5), moc_type_l(),
			moc_values_5(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f))));
}

long lfun6(char c, short s, int i, long l, float f, double d) {
	return moc_get_l(moc_act(MOC_FN(lfun6), moc_type_l(),
			moc_values_6(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f), moc_d(d))));
}

long lfun7(char c, short s, int i, long l, float f, double d, void *p) {
	return moc_get_l(moc_act(MOC_FN(lfun7), moc_type_l(),
			moc_values_7(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f), moc_d(d),
					moc_p(p))));
}

const char *strfun1(signed char sc) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun1), moc_type_cp_c(),
			moc_values_1(moc_sc(sc))));
}

const char *strfun2(signed char sc, unsigned char uc) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun2), moc_type_cp_c(),
			moc_values_2(moc_sc(sc), moc_uc(uc))));
}

const char *strfun3(signed char sc, unsigned char uc,
		unsigned short us) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun3), moc_type_cp_c(),
			moc_values_3(moc_sc(sc), moc_uc(uc),
					moc_us(us))));
}

const char *strfun4(signed char sc, unsigned char uc,
		unsigned short us, unsigned int ui) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun4), moc_type_cp_c(),
			moc_values_4(moc_sc(sc), moc_uc(uc),
					moc_us(us), moc_ui(ui))));
}

const char *strfun5(signed char sc, unsigned char uc,
		unsigned short us, unsigned int ui, unsigned long ul) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun5), moc_type_cp_c(),
			moc_values_5(moc_sc(sc), moc_uc(uc),
					moc_us(us), moc_ui(ui),
					moc_ul(ul))));
}

const char *strfun6(signed char sc, unsigned char uc,
		unsigned short us, unsigned int ui, unsigned long ul,
		int (*cmp)(const void *, const void *)) {
	return moc_get_cp_c(moc_act(MOC_FN(strfun6), moc_type_cp_c(),
			moc_values_6(moc_sc(sc), moc_uc(uc),
					moc_us(us), moc_ui(ui),
					moc_ul(ul),
					moc_fn(MOC_FP(cmp)))));
}

void test_any(void) {
	int n, m;
	char mem[5000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(lfun1),
			moc_match_1(moc_any()),
			moc_respond_1(moc_return(moc_l(1L))));
	moc_given(MOC_FN(lfun2),
			moc_match_2(moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(2L))));
	moc_given(MOC_FN(lfun3),
			moc_match_3(moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(3L))));
	moc_given(MOC_FN(lfun4),
			moc_match_4(moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(4L))));
	moc_given(MOC_FN(lfun5),
			moc_match_5(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(5L))));
	moc_given(MOC_FN(lfun6),
			moc_match_6(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(6L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(7L))));

	assert(1L == lfun1('a'));
	assert(1L == lfun1('b'));
	assert(2L == lfun2('c',11));
	assert(2L == lfun2('d',12));
	assert(3L == lfun3('e',13,-100));
	assert(3L == lfun3('f',14,-110));
	assert(4L == lfun4('g',15,-120,1111L));
	assert(4L == lfun4('h',16,-130,2222L));
	assert(5L == lfun5('i',17,-140,3333L,1.2));
	assert(5L == lfun5('j',18,-150,4444L,2.3));
	assert(6L == lfun6('k',19,-160,5555L,3.4,-100.001));
	assert(6L == lfun6('l',20,-170,6666L,4.5,-200.002));
	assert(7L == lfun7('m',21,-180,7777L,5.6,-300.003,&n));
	assert(7L == lfun7('n',22,-190,8888L,6.7,-400.004,&m));
}

int cmpint(const void *pi1, const void *pi2) {
	return (*(int *)pi1) - (*(int *)pi2);
}

int cmplong(const void *pl1, const void *pl2) {
	return (*(long *)pl1) - (*(long *)pl2);
}

void test_any_extended(void) {
	char mem[5000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(strfun1),
			moc_match_1(moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X1"))));
	moc_given(MOC_FN(strfun2),
			moc_match_2(moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X2"))));
	moc_given(MOC_FN(strfun3),
			moc_match_3(moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X3"))));
	moc_given(MOC_FN(strfun4),
			moc_match_4(moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X4"))));
	moc_given(MOC_FN(strfun5),
			moc_match_5(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X5"))));
	moc_given(MOC_FN(strfun6),
			moc_match_6(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_cp_c("X6"))));

	assert(strcmp(strfun1(-10), "X1") == 0);
	assert(strcmp(strfun1(-11), "X1") == 0);
	assert(strcmp(strfun2(-12,0), "X2") == 0);
	assert(strcmp(strfun2(-13,1), "X2") == 0);
	assert(strcmp(strfun3(-14,2,11), "X3") == 0);
	assert(strcmp(strfun3(-15,3,22), "X3") == 0);
	assert(strcmp(strfun4(-16,4,33,111), "X4") == 0);
	assert(strcmp(strfun4(-17,5,44,222), "X4") == 0);
	assert(strcmp(strfun5(-18,6,55,333,1111L), "X5") == 0);
	assert(strcmp(strfun5(-19,7,66,444,2222L), "X5") == 0);
	assert(strcmp(strfun6(-20,8,77,555,3333L,cmpint), "X6") == 0);
	assert(strcmp(strfun6(-21,9,88,666,4444L,cmplong), "X6") == 0);
}

void test_any_type(void) {
	int n, m;
	char mem[5000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(lfun1),
			moc_match_1(moc_any_c()),
			moc_respond_1(moc_return(moc_l(-1L))));
	moc_given(MOC_FN(lfun2),
			moc_match_2(moc_any_c(),
				moc_any_s()),
			moc_respond_1(moc_return(moc_l(-2L))));
	moc_given(MOC_FN(lfun3),
			moc_match_3(moc_any_c(),
				moc_any_s(),
				moc_any_i()),
			moc_respond_1(moc_return(moc_l(-3L))));
	moc_given(MOC_FN(lfun4),
			moc_match_4(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_any_l()),
			moc_respond_1(moc_return(moc_l(-4L))));
	moc_given(MOC_FN(lfun5),
			moc_match_5(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_any_l(),
				moc_any_f()),
			moc_respond_1(moc_return(moc_l(-5L))));
	moc_given(MOC_FN(lfun6),
			moc_match_6(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_any_l(),
				moc_any_f(),
				moc_any_d()),
			moc_respond_1(moc_return(moc_l(-6L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_any_l(),
				moc_any_f(),
				moc_any_d(),
				moc_any_p()),
			moc_respond_1(moc_return(moc_l(-7L))));

	assert(-1L == lfun1('a'));
	assert(-1L == lfun1('b'));
	assert(-2L == lfun2('c',11));
	assert(-2L == lfun2('d',12));
	assert(-3L == lfun3('e',13,-100));
	assert(-3L == lfun3('f',14,-110));
	assert(-4L == lfun4('g',15,-120,1111L));
	assert(-4L == lfun4('h',16,-130,2222L));
	assert(-5L == lfun5('i',17,-140,3333L,1.2));
	assert(-5L == lfun5('j',18,-150,4444L,2.3));
	assert(-6L == lfun6('k',19,-160,5555L,3.4,-100.001));
	assert(-6L == lfun6('l',20,-170,6666L,4.5,-200.002));
	assert(-7L == lfun7('m',21,-180,7777L,5.6,-300.003,&n));
	assert(-7L == lfun7('n',22,-190,8888L,6.7,-400.004,&m));
}

void test_any_type_extended(void) {
	char mem[5000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(strfun1),
			moc_match_1(moc_any_sc()),
			moc_respond_1(moc_return(moc_cp_c("Y1"))));
	moc_given(MOC_FN(strfun2),
			moc_match_2(moc_any_sc(),
				moc_any_uc()),
			moc_respond_1(moc_return(moc_cp_c("Y2"))));
	moc_given(MOC_FN(strfun3),
			moc_match_3(moc_any_sc(),
				moc_any_uc(),
				moc_any_us()),
			moc_respond_1(moc_return(moc_cp_c("Y3"))));
	moc_given(MOC_FN(strfun4),
			moc_match_4(moc_any_sc(),
				moc_any_uc(),
				moc_any_us(),
				moc_any_ui()),
			moc_respond_1(moc_return(moc_cp_c("Y4"))));
	moc_given(MOC_FN(strfun5),
			moc_match_5(moc_any_sc(),
				moc_any_uc(),
				moc_any_us(),
				moc_any_ui(),
				moc_any_ul()),
			moc_respond_1(moc_return(moc_cp_c("Y5"))));
	moc_given(MOC_FN(strfun6),
			moc_match_6(moc_any_sc(),
				moc_any_uc(),
				moc_any_us(),
				moc_any_ui(),
				moc_any_ul(),
				moc_any_fn()),
			moc_respond_1(moc_return(moc_cp_c("Y6"))));

	assert(strcmp(strfun1(-10), "Y1") == 0);
	assert(strcmp(strfun1(-11), "Y1") == 0);
	assert(strcmp(strfun2(-12,0), "Y2") == 0);
	assert(strcmp(strfun2(-13,1), "Y2") == 0);
	assert(strcmp(strfun3(-14,2,11), "Y3") == 0);
	assert(strcmp(strfun3(-15,3,22), "Y3") == 0);
	assert(strcmp(strfun4(-16,4,33,111), "Y4") == 0);
	assert(strcmp(strfun4(-17,5,44,222), "Y4") == 0);
	assert(strcmp(strfun5(-18,6,55,333,1111L), "Y5") == 0);
	assert(strcmp(strfun5(-19,7,66,444,2222L), "Y5") == 0);
	assert(strcmp(strfun6(-20,8,77,555,3333L,cmpint), "Y6") == 0);
	assert(strcmp(strfun6(-21,9,88,666,4444L,cmplong), "Y6") == 0);
}

void test_any_cascade(void) {
	char c = 'A';
	short s = 100;
	int i = 9999;
	long l = 10000L;
	float f = 12.345;
	double d = -66.6666;
	int n, m;
	char mem[10000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d)),
				moc_eq(moc_p(&n))),
			moc_respond_1(moc_return(moc_l(0L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any(),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d)),
				moc_any()),
			moc_respond_1(moc_return(moc_l(11L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any(),
				moc_any(),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(22L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any(),
				moc_any(),
				moc_any(),
				moc_eq(moc_l(l)),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(33L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any(),
				moc_any()),
			moc_respond_1(moc_return(moc_l(44L))));

	assert(0L == lfun7(c,s,i,l,f,d,&n));
	assert(0L == lfun7(c,s,i,l,f,d,&n));
	assert(11L == lfun7('B',s,i,l,f,d,&n));
	assert(11L == lfun7(c,s,i,l,f,d,&m));
	assert(11L == lfun7('B',s,i,l,f,d,&m));
	assert(22L == lfun7(c,200,i,l,f,d,&n));
	assert(22L == lfun7(c,s,i,l,f,13.579,&n));
	assert(22L == lfun7(c,200,i,l,f,13.579,&n));
	assert(33L == lfun7(c,s,8888,l,f,d,&n));
	assert(33L == lfun7(c,s,i,l,-2.46,d,&n));
	assert(33L == lfun7(c,s,8888,l,-2.46,d,&n));
	assert(44L == lfun7(c,s,i,20000L,f,d,&n));
	assert(44L == lfun7(c,s,i,30000L,f,d,&n));
}

void test_any_type_cascade(void) {
	char c = 'A';
	short s = 100;
	int i = 9999;
	long l = 10000L;
	float f = 12.345;
	double d = -66.6666;
	int n, m;
	char mem[10000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d)),
				moc_eq(moc_p(&n))),
			moc_respond_1(moc_return(moc_l(0L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any_c(),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d)),
				moc_any_p()),
			moc_respond_1(moc_return(moc_l(11L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any_c(),
				moc_any_s(),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_any_d(),
				moc_any_p()),
			moc_respond_1(moc_return(moc_l(22L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_eq(moc_l(l)),
				moc_any_f(),
				moc_any_d(),
				moc_any_p()),
			moc_respond_1(moc_return(moc_l(33L))));
	moc_given(MOC_FN(lfun7),
			moc_match_7(moc_any_c(),
				moc_any_s(),
				moc_any_i(),
				moc_any_l(),
				moc_any_f(),
				moc_any_d(),
				moc_any_p()),
			moc_respond_1(moc_return(moc_l(44L))));

	assert(0L == lfun7(c,s,i,l,f,d,&n));
	assert(0L == lfun7(c,s,i,l,f,d,&n));
	assert(11L == lfun7('B',s,i,l,f,d,&n));
	assert(11L == lfun7(c,s,i,l,f,d,&m));
	assert(11L == lfun7('B',s,i,l,f,d,&m));
	assert(22L == lfun7(c,200,i,l,f,d,&n));
	assert(22L == lfun7(c,s,i,l,f,13.579,&n));
	assert(22L == lfun7(c,200,i,l,f,13.579,&n));
	assert(33L == lfun7(c,s,8888,l,f,d,&n));
	assert(33L == lfun7(c,s,i,l,-2.46,d,&n));
	assert(33L == lfun7(c,s,8888,l,-2.46,d,&n));
	assert(44L == lfun7(c,s,i,20000L,f,d,&n));
	assert(44L == lfun7(c,s,i,30000L,f,d,&n));
}

int main(void) {
	test_any();
	test_any_extended();
	test_any_type();
	test_any_cascade();
	test_any_type_cascade();
	return 0;
}

