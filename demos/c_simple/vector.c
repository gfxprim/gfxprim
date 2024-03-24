//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Richard Palethorpe <richiejp@f-m.fm>
  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

#include <stdio.h>
#include <utils/gp_vec.h>

static void print_int_vec(int *payload)
{
	size_t i;
	gp_vec *vec = GP_VEC(payload);

	printf("gp_vec { .unit = %zu, .capacity = %zu, .length = %zu, .payload = [\n ",
	       vec->unit, vec->capacity, vec->length);

	for (i = 0; i < vec->length; i++)
		printf("\t%zu\t= %d\n", i, payload[i]);

	if (vec->length < vec->capacity)
		printf("\t... +%zu unused capacity\n", vec->capacity - vec->length);

	printf("]}\n");
}

int main(void)
{
	size_t i;
	int *ints = gp_vec_new(10, sizeof(int));
	char **strs;

	for (i = 0; i < gp_vec_len(ints); i++)
		ints[i] = i + 1;

	printf("Initial range (1,10): ");
	print_int_vec(ints);

	ints = gp_vec_ins(ints, 0, 1);
	ints[0] = 0;

	printf("\nInsert 0: ");
	print_int_vec(ints);

	ints = GP_VEC_APPEND(ints, 11);

	printf("\nPush 11: ");
	print_int_vec(ints);

	ints = gp_vec_ins(ints, 4, 1);
	ints[4] = -1;

	printf("\nInsert -1 at 4: ");
	print_int_vec(ints);

	ints = gp_vec_ins(ints, 5, 3);

	printf("\nInsert 3 of 0 at 5: ");
	print_int_vec(ints);

	ints = gp_vec_del(ints, 5, 3);
	printf("\nDelete 3 of 0 at 5: ");
	print_int_vec(ints);

	ints = gp_vec_del(ints, 4, 1);
	printf("\nDelete -1 at 4: ");
	print_int_vec(ints);

	ints = gp_vec_del(ints, 11, 1);
	printf("\nDelete 11 at 11: ");
	print_int_vec(ints);

	ints = gp_vec_del(ints, 0, 1);
	printf("\nDelete 0 at 0: ");
	print_int_vec(ints);

	printf("\nNow print the int vector with FOREACH macro:\n");
	GP_VEC_FOREACH(ints, int, val)
		printf("%i\n", *val);

	gp_vec_free(ints);

	strs = gp_vec_new(10, sizeof(char*));

	for (i = 0; i < gp_vec_len(strs); i++) {
		strs[i] = gp_vec_new(10, sizeof(char));
		snprintf(strs[i], gp_vec_len(strs[i]), "Line %zu", i);
	}

	printf("\nVec of string vecs: \n");
	GP_VEC_FOREACH(strs, char*, str)
		printf("%s\n", *str);

	GP_VEC_FOREACH(strs, char*, str)
		gp_vec_free(*str);

	gp_vec_free(strs);

	return 0;
}
