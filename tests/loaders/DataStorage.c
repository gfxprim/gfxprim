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
	GP_DataStorage *storage;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	GP_DataStorageDestroy(storage);

	return TST_SUCCESS;
}

static int create_get_destroy(void)
{
	GP_DataStorage *storage;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	if (GP_DataStorageGet(storage, NULL, "nonexistent")) {
		tst_msg("DataStorageGet() returned nonexistent record");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	GP_DataStorageDestroy(storage);

	return TST_SUCCESS;
}

static int records_are_equal(const GP_DataNode *a, const GP_DataNode *b)
{
	if (strcmp(a->id, b->id)) {
		tst_msg("Data id's do not match ('%s', '%s')", a->id, b->id);
		return 0;
	}

	if (a->type != b->type) {
		tst_msg("Data types do no match (%s, %s)",
		        GP_DataTypeName(a->type), GP_DataTypeName(b->type));
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
	GP_DataStorage *storage;
	const GP_DataNode *ret;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	GP_DataNode data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = GP_DataStorageAdd(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	if (!records_are_equal(ret, &data)) {
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	ret = GP_DataStorageGet(storage, NULL, "string");
	if (!ret) {
		tst_msg("DataStorageGet() failed for newly added data");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	if (!records_are_equal(ret, &data)) {
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	GP_DataStorageDestroy(storage);

	return TST_SUCCESS;
}

static int duplicit_id_add(void)
{
	GP_DataStorage *storage;
	GP_DataNode *ret;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	GP_DataNode data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = GP_DataStorageAdd(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	ret = GP_DataStorageAdd(storage, NULL, &data);
	if (ret) {
		tst_msg("DataStorageAdd() added data with duplicit id");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	GP_DataStorageDestroy(storage);
	return TST_SUCCESS;
}

static int wrong_type_add(void)
{
	GP_DataStorage *storage;
	GP_DataNode *ret;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	ret = GP_DataStorageAddInt(storage, NULL, "not-a-dict", 0);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	ret = GP_DataStorageAddInt(storage, ret, "int", 0);
	if (ret) {
		tst_msg("DataStorageAdd() added data into Integer Node");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	GP_DataStorageDestroy(storage);
	return TST_SUCCESS;
}

static int get_by_path(void)
{
	GP_DataStorage *storage;
	GP_DataNode *ret, *res;
	int fail = 0;

	storage = GP_DataStorageCreate();

	if (!storage)
		return TST_FAILED;

	GP_DataNode data = {
		.type = GP_DATA_STRING,
		.id = "string",
		.value.str = "test string",
	};

	ret = GP_DataStorageAdd(storage, NULL, &data);

	if (!ret) {
		tst_msg("DataStorageAdd() failed");
		GP_DataStorageDestroy(storage);
		return TST_FAILED;
	}

	/* Global path */
	res = GP_DataStorageGetByPath(storage, NULL, "/string");

	if (res != ret) {
		tst_msg("DataStorageGetByPath(storage, NULL, '/string')");
		fail++;
	}

	/* Local path */
	res = GP_DataStorageGetByPath(NULL, GP_DataStorageRoot(storage), "string");

	if (res != ret) {
		tst_msg("DataStorageGetByPath(NULL, root, 'string')");
		fail++;
	}

	/* Non existing in global path */
	res = GP_DataStorageGetByPath(storage, NULL, "/does-not-exist");

	if (res) {
		tst_msg("DataStorageGetByPath(storage, NULL, '/does-not-exist')");
		fail++;
	}

	/* Non existing in local path */
	res = GP_DataStorageGetByPath(NULL, GP_DataStorageRoot(storage), "does-not-exist");

	if (res) {
		tst_msg("DataStorageGetByPath(NULL, root, 'does-not-exist')");
		fail++;
	}

	/* Empty dict for local path */
	res = GP_DataStorageGetByPath(NULL, NULL, "does-not-exist");

	if (res) {
		tst_msg("DataStorageGetByPath(NULL, NULL, 'does-not-exist')");
		fail++;
	}

	/* Empty storage for global path */
	res = GP_DataStorageGetByPath(NULL, NULL, "/does-not-exist");

	if (res) {
		tst_msg("DataStorageGetByPath(NULL, NULL, '/does-not-exist')");
		fail++;
	}

	GP_DataStorageDestroy(storage);

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

		{.name = "GP_DataStorageGetByPath()",
		 .tst_fn = get_by_path,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
