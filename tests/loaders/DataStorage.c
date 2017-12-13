/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <loaders/GP_DataStorage.h>

#include "tst_test.h"

static int create_destroy(void)
{
	gp_storage *storage;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	gp_storage_destroy(storage);

	return TST_SUCCESS;
}

static int create_get_destroy(void)
{
	gp_storage *storage;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	if (gp_storage_get(storage, NULL, "nonexistent")) {
		tst_msg("DataStorageGet() returned nonexistent record");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	gp_storage_destroy(storage);

	return TST_SUCCESS;
}

static int records_are_equal(const gp_data_node *a, const gp_data_node *b)
{
	if (strcmp(a->id, b->id)) {
		tst_msg("Data id's do not match ('%s', '%s')", a->id, b->id);
		return 0;
	}

	if (a->type != b->type) {
		tst_msg("Data types do no match (%s, %s)",
		        gp_data_type_name(a->type), gp_data_type_name(b->type));
		return 0;
	}

	switch (a->type) {
	case GP_DATA_INT:
		return a->value.i == b->value.i;
	case GP_DATA_STRING:
		return !strcmp(a->value.str, b->value.str);
	case GP_DATA_DOUBLE:
		return a->value.d == b->value.d;
	case GP_DATA_RATIONAL:
		return a->value.rat.num == b->value.rat.num &&
		       a->value.rat.den == b->value.rat.den;
	}

	tst_msg("Unknown data type %i", a->type);
	return 0;
}

static int create_add_get_destroy(void)
{
	gp_storage *storage;
	const gp_data_node *ret;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	gp_data_node data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = gp_storage_add(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	if (!records_are_equal(ret, &data)) {
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	ret = gp_storage_get(storage, NULL, "string");
	if (!ret) {
		tst_msg("DataStorageGet() failed for newly added data");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	if (!records_are_equal(ret, &data)) {
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	gp_storage_destroy(storage);

	return TST_SUCCESS;
}

static int duplicit_id_add(void)
{
	gp_storage *storage;
	gp_data_node *ret;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	gp_data_node data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = gp_storage_add(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	ret = gp_storage_add(storage, NULL, &data);
	if (ret) {
		tst_msg("DataStorageAdd() added data with duplicit id");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	gp_storage_destroy(storage);
	return TST_SUCCESS;
}

static int wrong_type_add(void)
{
	gp_storage *storage;
	gp_data_node *ret;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	ret = gp_storage_add_int(storage, NULL, "not-a-dict", 0);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	ret = gp_storage_add_int(storage, ret, "int", 0);
	if (ret) {
		tst_msg("DataStorageAdd() added data into Integer Node");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	gp_storage_destroy(storage);
	return TST_SUCCESS;
}

static int get_by_path(void)
{
	gp_storage *storage;
	gp_data_node *ret, *res;
	int fail = 0;

	storage = gp_storage_create();

	if (!storage)
		return TST_FAILED;

	gp_data_node data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = gp_storage_add(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		gp_storage_destroy(storage);
		return TST_FAILED;
	}

	/* Global path */
	res = gp_storage_get_by_path(storage, NULL, "/string");

	if (res != ret) {
		tst_msg("gp_storage_get_by_path(storage, NULL, '/string')");
		fail++;
	}

	/* Local path */
	res = gp_storage_get_by_path(NULL, gp_storage_root(storage), "string");

	if (res != ret) {
		tst_msg("gp_storage_get_by_path(NULL, root, 'string')");
		fail++;
	}

	/* Non existing in global path */
	res = gp_storage_get_by_path(storage, NULL, "/does-not-exist");

	if (res) {
		tst_msg("gp_storage_get_by_path(storage, NULL, '/does-not-exist')");
		fail++;
	}

	/* Non existing in local path */
	res = gp_storage_get_by_path(NULL, gp_storage_root(storage), "does-not-exist");

	if (res) {
		tst_msg("gp_storage_get_by_path(NULL, root, 'does-not-exist')");
		fail++;
	}

	/* Empty dict for local path */
	res = gp_storage_get_by_path(NULL, NULL, "does-not-exist");

	if (res) {
		tst_msg("gp_storage_get_by_path(NULL, NULL, 'does-not-exist')");
		fail++;
	}

	/* Empty storage for global path */
	res = gp_storage_get_by_path(NULL, NULL, "/does-not-exist");

	if (res) {
		tst_msg("gp_storage_get_by_path(NULL, NULL, '/does-not-exist')");
		fail++;
	}

	gp_storage_destroy(storage);

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Data Storage",
	.tests = {
		{.name = "Create Destroy",
		 .tst_fn = create_destroy,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Create Get Destroy",
		 .tst_fn = create_get_destroy,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Create Add Get Destroy",
		 .tst_fn = create_add_get_destroy,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Duplicit ID Add",
		 .tst_fn = duplicit_id_add,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Wrong type Add",
		 .tst_fn = wrong_type_add,
		 .flags = TST_CHECK_MALLOC},

		{.name = "gp_storage_get_by_path()",
		 .tst_fn = get_by_path,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
