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
 * Tests of mocks with ordinary matchers.
 */

#include "mocito.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

/* Create the default function to manage the mocking-related errors. */
void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }

int ifun0(void) {
	return moc_get_i(moc_act(MOC_FN(ifun0), moc_type_i(),
			moc_values_0()));
}

int ifun1(char c) {
	return moc_get_i(moc_act(MOC_FN(ifun1), moc_type_i(),
			moc_values_1(moc_c(c))));
}

int ifun2(char c, short s) {
	return moc_get_i(moc_act(MOC_FN(ifun2), moc_type_i(),
			moc_values_2(moc_c(c), moc_s(s))));
}

int ifun3(char c, short s, int i) {
	return moc_get_i(moc_act(MOC_FN(ifun3), moc_type_i(),
			moc_values_3(moc_c(c), moc_s(s), moc_i(i))));
}

int ifun4(char c, short s, int i, long l) {
	return moc_get_i(moc_act(MOC_FN(ifun4), moc_type_i(),
			moc_values_4(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l))));
}

int ifun5(char c, short s, int i, long l, float f) {
	return moc_get_i(moc_act(MOC_FN(ifun5), moc_type_i(),
			moc_values_5(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f))));
}

int ifun6(char c, short s, int i, long l, float f, double d) {
	return moc_get_i(moc_act(MOC_FN(ifun6), moc_type_i(),
			moc_values_6(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f), moc_d(d))));
}

int ifun7(char c, short s, int i, long l, float f, double d, void *p) {
	return moc_get_i(moc_act(MOC_FN(ifun7), moc_type_i(),
			moc_values_7(moc_c(c), moc_s(s), moc_i(i),
					moc_l(l), moc_f(f), moc_d(d),
					moc_p(p))));
}

double dfun7(void *p, double d, float f, long l, int i, short s, char c) {
	return moc_get_d(moc_act(MOC_FN(dfun7), moc_type_d(),
			moc_values_7(moc_p(p), moc_d(d),
					moc_f(f), moc_l(l), moc_i(i),
					moc_s(s), moc_c(c))));
}

void test_num_params(void) {
	char c = 'a';
	short s = 3;
	int i = 100;
	long l = 999L;
	float f = 1.23;
	double d = 45.6789;
	int n;
	char mem[5000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(ifun0),
			moc_match_0(),
			moc_respond_1(moc_return(moc_i(0))));
	moc_given(MOC_FN(ifun1),
			moc_match_1(moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_i(1))));
	moc_given(MOC_FN(ifun2),
			moc_match_2(moc_eq(moc_c(c)),
				moc_eq(moc_s(s))),
			moc_respond_1(moc_return(moc_i(2))));
	moc_given(MOC_FN(ifun3),
			moc_match_3(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i))),
			moc_respond_1(moc_return(moc_i(3))));
	moc_given(MOC_FN(ifun4),
			moc_match_4(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l))),
			moc_respond_1(moc_return(moc_i(4))));
	moc_given(MOC_FN(ifun5),
			moc_match_5(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f))),
			moc_respond_1(moc_return(moc_i(5))));
	moc_given(MOC_FN(ifun6),
			moc_match_6(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d))),
			moc_respond_1(moc_return(moc_i(6))));
	moc_given(MOC_FN(ifun7),
			moc_match_7(moc_eq(moc_c(c)),
				moc_eq(moc_s(s)),
				moc_eq(moc_i(i)),
				moc_eq(moc_l(l)),
				moc_eq(moc_f(f)),
				moc_eq(moc_d(d)),
				moc_eq(moc_p(&n))),
			moc_respond_1(moc_return(moc_i(7))));

	assert(0 == ifun0());
	assert(1 == ifun1(c));
	assert(2 == ifun2(c,s));
	assert(3 == ifun3(c,s,i));
	assert(4 == ifun4(c,s,i,l));
	assert(5 == ifun5(c,s,i,l,f));
	assert(6 == ifun6(c,s,i,l,f,d));
	assert(7 == ifun7(c,s,i,l,f,d,&n));
}

void test_value_params(void) {
	char c = 'a';
	short s = 3;
	int i = 100;
	long l = 999L;
	float f = 1.23;
	double d = 45.6789;
	int n, m;
	char mem[11000];

	moc_init(mem, sizeof(mem));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.0))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&m)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.1))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(-45.6789)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.2))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(-1.23)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.3))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(-999L)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.4))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(-100)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.5))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(-3)),
				moc_eq(moc_c(c))),
			moc_respond_1(moc_return(moc_d(7.6))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_eq(moc_p(&n)),
				moc_eq(moc_d(d)),
				moc_eq(moc_f(f)),
				moc_eq(moc_l(l)),
				moc_eq(moc_i(i)),
				moc_eq(moc_s(s)),
				moc_eq(moc_c('A'))),
			moc_respond_1(moc_return(moc_d(7.7))));
	moc_given(MOC_FN(dfun7),
			moc_match_7(moc_any_p(),
				moc_any_d(),
				moc_any_f(),
				moc_any_l(),
				moc_any_i(),
				moc_any_s(),
				moc_any_c()),
			moc_respond_1(moc_return(moc_d(-7.0))));

	assert(7.0 == dfun7(&n,d,f,l,i,s,c));
	assert(7.0 == dfun7(&n,d,f,l,i,s,c));
	assert(7.1 == dfun7(&m,d,f,l,i,s,c));
	assert(7.1 == dfun7(&m,d,f,l,i,s,c));
	assert(7.2 == dfun7(&n,-45.6789,f,l,i,s,c));
	assert(7.2 == dfun7(&n,-45.6789,f,l,i,s,c));
	assert(7.3 == dfun7(&n,d,-1.23,l,i,s,c));
	assert(7.3 == dfun7(&n,d,-1.23,l,i,s,c));
	assert(7.4 == dfun7(&n,d,f,-999L,i,s,c));
	assert(7.4 == dfun7(&n,d,f,-999L,i,s,c));
	assert(7.5 == dfun7(&n,d,f,l,-100,s,c));
	assert(7.5 == dfun7(&n,d,f,l,-100,s,c));
	assert(7.6 == dfun7(&n,d,f,l,i,-3,c));
	assert(7.6 == dfun7(&n,d,f,l,i,-3,c));
	assert(7.7 == dfun7(&n,d,f,l,i,s,'A'));
	assert(7.7 == dfun7(&n,d,f,l,i,s,'A'));
	assert(-7.0 == dfun7(&n,d,f,l,i,s,'b'));
	assert(-7.0 == dfun7(&n,d,f,l,i,4,c));
	assert(-7.0 == dfun7(&n,d,f,l,101,s,c));
	assert(-7.0 == dfun7(&n,d,f,1000L,i,s,c));
	assert(-7.0 == dfun7(&n,d,2.23,l,i,s,c));
	assert(-7.0 == dfun7(&n,46.6789,f,l,i,s,c));
	assert(-7.0 == dfun7(mem,d,f,l,i,s,c));
}

int main(void) {
	test_num_params();
	test_value_params();
	return 0;
}

