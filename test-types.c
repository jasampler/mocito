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
 * Tests of packing and unpacking basic types and values.
 */

#include "mocito.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

/* Create the default function to manage the mocking-related errors. */
void moc_error(void) { fprintf(stderr, "%s\n", moc_errmsg()); exit(1); }

void test_get_type(void) {
	char c = 11;
	short s = 111;
	int i = 200;
	long l = 444;
	float f = 3.5;
	double d = 8.9;
	signed char sc = 22;
	unsigned char uc = 33;
	unsigned short us = 44;
	unsigned int ui = 55;
	unsigned long ul = 66;
	moc_fnptr fn = MOC_FP(moc_fn);
	void *p = &c;

	assert(moc_get_type(moc_void()) == moc_type_void());
	assert(moc_get_type(moc_c(c)) == moc_type_c());
	assert(moc_get_type(moc_s(s)) == moc_type_s());
	assert(moc_get_type(moc_i(i)) == moc_type_i());
	assert(moc_get_type(moc_l(l)) == moc_type_l());
	assert(moc_get_type(moc_f(f)) == moc_type_f());
	assert(moc_get_type(moc_d(d)) == moc_type_d());
	assert(moc_get_type(moc_sc(sc)) == moc_type_sc());
	assert(moc_get_type(moc_uc(uc)) == moc_type_uc());
	assert(moc_get_type(moc_us(us)) == moc_type_us());
	assert(moc_get_type(moc_ui(ui)) == moc_type_ui());
	assert(moc_get_type(moc_ul(ul)) == moc_type_ul());
	assert(moc_get_type(moc_fn(MOC_FP(moc_fn))) == moc_type_fn());

	assert(moc_get_type(moc_p(&p)) == moc_type_p());
	assert(moc_get_type(moc_p_c(&c)) == moc_type_p_c());
	assert(moc_get_type(moc_p_s(&s)) == moc_type_p_s());
	assert(moc_get_type(moc_p_i(&i)) == moc_type_p_i());
	assert(moc_get_type(moc_p_l(&l)) == moc_type_p_l());
	assert(moc_get_type(moc_p_f(&f)) == moc_type_p_f());
	assert(moc_get_type(moc_p_d(&d)) == moc_type_p_d());
	assert(moc_get_type(moc_p_sc(&sc)) == moc_type_p_sc());
	assert(moc_get_type(moc_p_uc(&uc)) == moc_type_p_uc());
	assert(moc_get_type(moc_p_us(&us)) == moc_type_p_us());
	assert(moc_get_type(moc_p_ui(&ui)) == moc_type_p_ui());
	assert(moc_get_type(moc_p_ul(&ul)) == moc_type_p_ul());
	assert(moc_get_type(moc_p_fn(&fn)) == moc_type_p_fn());

	assert(moc_get_type(moc_cp(&p)) == moc_type_cp());
	assert(moc_get_type(moc_cp_c(&c)) == moc_type_cp_c());
	assert(moc_get_type(moc_cp_s(&s)) == moc_type_cp_s());
	assert(moc_get_type(moc_cp_i(&i)) == moc_type_cp_i());
	assert(moc_get_type(moc_cp_l(&l)) == moc_type_cp_l());
	assert(moc_get_type(moc_cp_f(&f)) == moc_type_cp_f());
	assert(moc_get_type(moc_cp_d(&d)) == moc_type_cp_d());
	assert(moc_get_type(moc_cp_sc(&sc)) == moc_type_cp_sc());
	assert(moc_get_type(moc_cp_uc(&uc)) == moc_type_cp_uc());
	assert(moc_get_type(moc_cp_us(&us)) == moc_type_cp_us());
	assert(moc_get_type(moc_cp_ui(&ui)) == moc_type_cp_ui());
	assert(moc_get_type(moc_cp_ul(&ul)) == moc_type_cp_ul());
	assert(moc_get_type(moc_cp_fn(&fn)) == moc_type_cp_fn());
}

void test_get_value(void) {
	char c = 11;
	short s = 111;
	int i = 200;
	long l = 444;
	float f = 3.5;
	double d = 8.9;
	signed char sc = 22;
	unsigned char uc = 33;
	unsigned short us = 44;
	unsigned int ui = 55;
	unsigned long ul = 66;
	moc_fnptr fn = MOC_FP(moc_fn);
	void *p = &c;

	assert(moc_get_c(moc_c(c)) == c);
	assert(moc_get_s(moc_s(s)) == s);
	assert(moc_get_i(moc_i(i)) == i);
	assert(moc_get_l(moc_l(l)) == l);
	assert(moc_get_f(moc_f(f)) == f);
	assert(moc_get_d(moc_d(d)) == d);
	assert(moc_get_sc(moc_sc(sc)) == sc);
	assert(moc_get_uc(moc_uc(uc)) == uc);
	assert(moc_get_us(moc_us(us)) == us);
	assert(moc_get_ui(moc_ui(ui)) == ui);
	assert(moc_get_ul(moc_ul(ul)) == ul);
	assert(moc_get_fn(moc_fn(MOC_FP(moc_fn))) == fn);

	assert(moc_get_p(moc_p(&p)) == &p);
	assert(moc_get_p_c(moc_p_c(&c)) == &c);
	assert(moc_get_p_s(moc_p_s(&s)) == &s);
	assert(moc_get_p_i(moc_p_i(&i)) == &i);
	assert(moc_get_p_l(moc_p_l(&l)) == &l);
	assert(moc_get_p_f(moc_p_f(&f)) == &f);
	assert(moc_get_p_d(moc_p_d(&d)) == &d);
	assert(moc_get_p_sc(moc_p_sc(&sc)) == &sc);
	assert(moc_get_p_uc(moc_p_uc(&uc)) == &uc);
	assert(moc_get_p_us(moc_p_us(&us)) == &us);
	assert(moc_get_p_ui(moc_p_ui(&ui)) == &ui);
	assert(moc_get_p_ul(moc_p_ul(&ul)) == &ul);
	assert(moc_get_p_fn(moc_p_fn(&fn)) == &fn);

	assert(moc_get_cp(moc_cp(&p)) == &p);
	assert(moc_get_cp_c(moc_cp_c(&c)) == &c);
	assert(moc_get_cp_s(moc_cp_s(&s)) == &s);
	assert(moc_get_cp_i(moc_cp_i(&i)) == &i);
	assert(moc_get_cp_l(moc_cp_l(&l)) == &l);
	assert(moc_get_cp_f(moc_cp_f(&f)) == &f);
	assert(moc_get_cp_d(moc_cp_d(&d)) == &d);
	assert(moc_get_cp_sc(moc_cp_sc(&sc)) == &sc);
	assert(moc_get_cp_uc(moc_cp_uc(&uc)) == &uc);
	assert(moc_get_cp_us(moc_cp_us(&us)) == &us);
	assert(moc_get_cp_ui(moc_cp_ui(&ui)) == &ui);
	assert(moc_get_cp_ul(moc_cp_ul(&ul)) == &ul);
	assert(moc_get_cp_fn(moc_cp_fn(&fn)) == &fn);
}

void test_get_value_min(void) {
	char c = CHAR_MIN;
	short s = SHRT_MIN;
	int i = INT_MIN;
	long l = LONG_MIN;
	float f = FLT_MAX;
	double d = DBL_MAX;
	signed char sc = SCHAR_MIN;
	unsigned char uc = 0;
	unsigned short us = 0;
	unsigned int ui = 0;
	unsigned long ul = 0;

	assert(moc_get_c(moc_c(c)) == c);
	assert(moc_get_s(moc_s(s)) == s);
	assert(moc_get_i(moc_i(i)) == i);
	assert(moc_get_l(moc_l(l)) == l);
	assert(moc_get_f(moc_f(f)) == f);
	assert(moc_get_d(moc_d(d)) == d);
	assert(moc_get_sc(moc_sc(sc)) == sc);
	assert(moc_get_uc(moc_uc(uc)) == uc);
	assert(moc_get_us(moc_us(us)) == us);
	assert(moc_get_ui(moc_ui(ui)) == ui);
	assert(moc_get_ul(moc_ul(ul)) == ul);
}

void test_get_value_max(void) {
	char c = CHAR_MAX;
	short s = SHRT_MAX;
	int i = INT_MAX;
	long l = LONG_MAX;
	float f = FLT_MIN;
	double d = DBL_MIN;
	signed char sc = SCHAR_MAX;
	unsigned char uc = UCHAR_MAX;
	unsigned short us = USHRT_MAX;
	unsigned int ui = UINT_MAX;
	unsigned long ul = ULONG_MAX;

	assert(moc_get_c(moc_c(c)) == c);
	assert(moc_get_s(moc_s(s)) == s);
	assert(moc_get_i(moc_i(i)) == i);
	assert(moc_get_l(moc_l(l)) == l);
	assert(moc_get_f(moc_f(f)) == f);
	assert(moc_get_d(moc_d(d)) == d);
	assert(moc_get_sc(moc_sc(sc)) == sc);
	assert(moc_get_uc(moc_uc(uc)) == uc);
	assert(moc_get_us(moc_us(us)) == us);
	assert(moc_get_ui(moc_ui(ui)) == ui);
	assert(moc_get_ul(moc_ul(ul)) == ul);
}

int main(void) {
	test_get_type();
	test_get_value();
	test_get_value_min();
	test_get_value_max();
	return 0;
}

