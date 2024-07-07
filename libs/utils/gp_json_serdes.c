// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <string.h>

#include <core/gp_debug.h>

#include <utils/gp_json_reader.h>
#include <utils/gp_json_writer.h>
#include <utils/gp_json_serdes.h>

static size_t count_and_check_descs(const gp_json_struct *desc)
{
	const gp_json_struct *i, *prev = NULL;
	size_t cnt = 0;

	for (i = desc; i->id; i++) {
		if (prev && strcmp(prev->id, i->id) >= 0) {
			GP_WARN("Struct desc not sorted alphabetically '%s', '%s', ...",
			        prev->id, i->id);
			return 0;
		}

		switch (GP_JSON_SERDES_TYPE(i->type)) {
		case GP_JSON_SERDES_INT:
		case GP_JSON_SERDES_UINT:
			if (i->type_size != 1 && i->type_size != 2 &&
			    i->type_size != 4 && i->type_size != 8) {
				GP_WARN("Invalid (u)int size %zu", i->type_size);
				return 0;
			}
		break;
		case GP_JSON_SERDES_FLOAT:
			if (i->type_size != sizeof(float) &&
			    i->type_size != sizeof(double)) {
				GP_WARN("Invalid float/double size %zu", i->type_size);
				return 0;
			}
		break;
		case GP_JSON_SERDES_STR:
		break;
		}

		prev = i;
		cnt++;
	}

	return cnt;
}

static int str_cpy(const gp_json_struct *desc, gp_json_val *val, void *baseptr)
{
	char *dsc = baseptr + desc->offset;

	strncpy(dsc, val->val_str, desc->type_size);

	dsc[desc->type_size - 1] = 0;

	return 0;
}

static int str_dup(const gp_json_struct *desc, gp_json_val *val, void *baseptr)
{
	char **dsc = baseptr + desc->offset;

	*dsc = strndup(val->val_str, desc->str_max_size);

	return *dsc == NULL;
}

static int null_store(const gp_json_struct *desc, void *baseptr)
{
	void **dsc = baseptr + desc->offset;

	*dsc = NULL;

	return 0;
}

static int str_store(const gp_json_struct *desc, gp_json_val *val, void *baseptr)
{
	if (desc->type_size)
		return str_cpy(desc, val, baseptr);

	if (val->type == GP_JSON_STR)
		return str_dup(desc, val, baseptr);
	else
		return null_store(desc, baseptr);
}

static int int8_store(gp_json_reader *json, const gp_json_struct *desc,
                      gp_json_val *val, void *baseptr)
{
	if (val->val_int < INT8_MIN) {
		gp_json_warn(json, "Int8 value out of range");
		return 1;
	}

	if (val->val_int > INT8_MAX) {
		gp_json_warn(json, "Int8 value out of range");
		return 1;
	}

	*(int8_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int int16_store(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val, void *baseptr)
{
	if (val->val_int < INT16_MIN) {
		gp_json_warn(json, "Int16 value out of range");
		return 1;
	}

	if (val->val_int > INT16_MAX) {
		gp_json_warn(json, "Int16 value out of range");
		return 1;
	}

	*(int16_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int int32_store(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val, void *baseptr)
{
	if (val->val_int < INT32_MIN) {
		gp_json_warn(json, "Int32 value out of range");
		return 1;
	}

	if (val->val_int > INT32_MAX) {
		gp_json_warn(json, "Int32 value out of range");
		return 1;
	}

	*(int32_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int int64_store(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val, void *baseptr)
{
	if (val->val_int < INT64_MIN) {
		gp_json_warn(json, "Int64 value out of range");
		return 1;
	}

	if (val->val_int > INT64_MAX) {
		gp_json_warn(json, "Int64 value out of range");
		return 1;
	}

	*(int64_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int int_store(gp_json_reader *json, const gp_json_struct *desc,
                     gp_json_val *val, void *baseptr)
{
	switch (desc->type_size) {
	case 1:
		return int8_store(json, desc, val, baseptr);
	case 2:
		return int16_store(json, desc, val, baseptr);
	case 4:
		return int32_store(json, desc, val, baseptr);
	case 8:
		return int64_store(json, desc, val, baseptr);
	}

	return 1;
}

static int uint8_store(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val, void *baseptr)
{
	if (val->val_int < 0) {
		gp_json_warn(json, "Uint8 value out of range");
		return 1;
	}

	if (val->val_int > UINT8_MAX) {
		gp_json_warn(json, "Uint8 value out of range");
		return 1;
	}

	*(uint8_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int uint16_store(gp_json_reader *json, const gp_json_struct *desc,
                        gp_json_val *val, void *baseptr)
{
	if (val->val_int < 0) {
		gp_json_warn(json, "Uint16 value out of range");
		return 1;
	}

	if (val->val_int > UINT16_MAX) {
		gp_json_warn(json, "Uint16 value out of range");
		return 1;
	}

	*(uint16_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int uint32_store(gp_json_reader *json, const gp_json_struct *desc,
                        gp_json_val *val, void *baseptr)
{
	if (val->val_int < 0) {
		gp_json_warn(json, "Uint32 value out of range");
		return 1;
	}

	if (val->val_int > UINT16_MAX) {
		gp_json_warn(json, "Uint32 value out of range");
		return 1;
	}

	*(uint32_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int uint64_store(gp_json_reader *json, const gp_json_struct *desc,
                        gp_json_val *val, void *baseptr)
{
	if (val->val_int < 0) {
		gp_json_warn(json, "Uint64 value out of range");
		return 1;
	}

	if ((uint64_t)val->val_int > UINT64_MAX) {
		gp_json_warn(json, "Uint64 value out of range");
		return 1;
	}

	*(uint64_t*)(baseptr+desc->offset) = val->val_int;

	return 0;
}

static int uint_store(gp_json_reader *json, const gp_json_struct *desc,
                      gp_json_val *val, void *baseptr)
{
	switch (desc->type_size) {
	case 1:
		return uint8_store(json, desc, val, baseptr);
	case 2:
		return uint16_store(json, desc, val, baseptr);
	case 4:
		return uint32_store(json, desc, val, baseptr);
	case 8:
		return uint64_store(json, desc, val, baseptr);
	}

	return 1;
}

static int float_store(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val, void *baseptr)
{
	(void) json;

	*(float*)(baseptr+desc->offset) = val->val_float;

	return 0;
}

static int double_store(gp_json_reader *json, const gp_json_struct *desc,
                        gp_json_val *val, void *baseptr)
{
	(void) json;

	*(double*)(baseptr+desc->offset) = val->val_float;

	return 0;
}

static int float_double_store(gp_json_reader *json, const gp_json_struct *desc,
                              gp_json_val *val, void *baseptr)
{
	switch (desc->type_size) {
	case sizeof(float):
		return float_store(json, desc, val, baseptr);
	case sizeof(double):
		return double_store(json, desc, val, baseptr);
	}

	return 0;
}

static int int_check(gp_json_reader *json, const gp_json_struct *desc,
                     gp_json_val *val)
{

	if (val->val_int < desc->lim_int.min) {
		gp_json_warn(json, "Int value out of range %lli < %lli",
		             val->val_int, (long long int) desc->lim_int.min);
		return 1;
	}

	if (val->val_int > 0 && (uint64_t)val->val_int > desc->lim_int.max) {
		gp_json_warn(json, "Int value out of range %lli > %lli",
		             val->val_int, (long long int) desc->lim_int.max);
		return 1;
	}

	return 0;
}

static int float_check(gp_json_reader *json, const gp_json_struct *desc,
                       gp_json_val *val)
{
	if (val->val_float < desc->lim_float.min) {
		gp_json_warn(json, "Float value out of range %f < %lf",
		             val->val_float, desc->lim_float.min);
		return 1;
	}

	if (val->val_float > desc->lim_float.max) {
		gp_json_warn(json, "Float value out of range %f > %lf",
		             val->val_float, desc->lim_float.max);
		return 1;
	}

	return 0;
}

static int memb_store(gp_json_reader *json, const gp_json_struct *desc,
                      gp_json_val *val, void *baseptr)
{
	enum json_serdes_type type = GP_JSON_SERDES_TYPE(desc->type);

	switch (val->type) {
	case GP_JSON_ARR:
		gp_json_warn(json, "Array deserialization is not supported.");
		return 1;
	case GP_JSON_OBJ:
		gp_json_warn(json, "Object deserialization is unsupported.");
		return 1;
	case GP_JSON_VOID:
		return 0;
	case GP_JSON_BOOL:
		gp_json_warn(json, "Bool deserialization is unsupported.");
		return 1;
	case GP_JSON_INT:
		if (type != GP_JSON_SERDES_INT && type != GP_JSON_SERDES_UINT && type != GP_JSON_SERDES_FLOAT) {
			gp_json_warn(json, "Invalid value type expected number");
			return 1;
		}
	break;
	case GP_JSON_FLOAT:
		if (type != GP_JSON_SERDES_FLOAT) {
			gp_json_warn(json, "Invalid value type expected float");
			return 1;
		}
	break;
	case GP_JSON_STR:
		if (type != GP_JSON_SERDES_STR) {
			gp_json_warn(json, "Invalid value type expected string");
			return 1;
		}
	break;
	case GP_JSON_NULL:
		if (type != GP_JSON_SERDES_STR || desc->type_size) {
			gp_json_warn(json, "NULL can be only stored into a pointer");
			return 1;
		}
	break;
	}

	switch (GP_JSON_SERDES_TYPE(desc->type)) {
	case GP_JSON_SERDES_STR:
		return str_store(desc, val, baseptr);
	case GP_JSON_SERDES_INT:
		if (int_check(json, desc, val))
			return 1;
		return int_store(json, desc, val, baseptr);
	case GP_JSON_SERDES_UINT:
		if (int_check(json, desc, val))
			return 1;
		return uint_store(json, desc, val, baseptr);
	case GP_JSON_SERDES_FLOAT:
		if (float_check(json, desc, val))
			return 1;
		return float_double_store(json, desc, val, baseptr);
	}

	return 1;
}

static size_t memb_lookup(const char *id, const gp_json_struct *desc, size_t desc_cnt)
{
	return gp_json_lookup(desc, sizeof(*desc), desc_cnt, id);
}

int gp_json_read_struct(gp_json_reader *json, gp_json_val *val,
                        const gp_json_struct *desc, void *baseptr)
{
	size_t desc_cnt, i;
	int err = 0;

	if (gp_json_next_type(json) != GP_JSON_OBJ) {
		gp_json_warn(json, "Expected JSON object");
		return 1;
	}

	desc_cnt = count_and_check_descs(desc);
	if (!desc_cnt) {
		gp_json_obj_skip(json);
		return 1;
	}

	int loaded[desc_cnt];

	memset(loaded, 0, sizeof(loaded));

	GP_JSON_OBJ_FOREACH(json, val) {
		size_t idx = memb_lookup(val->id, desc, desc_cnt);

		if (idx == (size_t)-1) {
			gp_json_warn(json, "Unexpected member");
			continue;
		}

		if (loaded[idx])
			gp_json_warn(json, "Parameter %s redefined", val->id);

		err |= memb_store(json, desc + idx, val, baseptr);

		loaded[idx] = 1;
	}

	for (i = 0; i < desc_cnt; i++) {
		if (!loaded[i] && !(desc[i].type & GP_JSON_SERDES_OPTIONAL)) {
			gp_json_warn(json, "Missing %s attribute", desc[i].id);
			err = 1;
		}
	}

	return err;
}

int gp_json_load_struct(const char *path,
                        const gp_json_struct *desc, void *baseptr)
{
	gp_json_val *val;
	gp_json_reader *json;
	int err = 1;

	val = gp_json_val_alloc(0);
	json = gp_json_reader_load(path);

	if (!val || !json)
		goto exit;

	err = gp_json_read_struct(json, val, desc, baseptr);
	if (err)
		goto exit;

	gp_json_reader_finish(json);
exit:
	gp_json_val_free(val);
	gp_json_reader_free(json);
	return err;
}

static inline const char *str_ptr(const gp_json_struct *desc, void *baseptr)
{
	if (desc->type_size)
		return baseptr + desc->offset;

	return *((char**)(baseptr + desc->offset));
}

static int64_t int_val(const gp_json_struct *desc, void *baseptr)
{
	switch (desc->type_size) {
	case 1:
		return *(int8_t*)(baseptr + desc->offset);
	case 2:
		return *(int16_t*)(baseptr + desc->offset);
	case 4:
		return *(int32_t*)(baseptr + desc->offset);
	case 8:
		return *(int64_t*)(baseptr + desc->offset);
	}

	return 0;
}

static uint64_t uint_val(const gp_json_struct *desc, void *baseptr)
{
	switch (desc->type_size) {
	case 1:
		return *(uint8_t*)(baseptr + desc->offset);
	case 2:
		return *(uint16_t*)(baseptr + desc->offset);
	case 4:
		return *(uint32_t*)(baseptr + desc->offset);
	case 8:
		return *(uint64_t*)(baseptr + desc->offset);
	}

	return 0;
}

static float float_val(const gp_json_struct *desc, void *baseptr)
{
	switch (desc->type_size) {
	case sizeof(float):
		return *(float*)(baseptr + desc->offset);
	case sizeof(double):
		return *(double*)(baseptr + desc->offset);
	}

	return 0;
}

int gp_json_write_struct(gp_json_writer *json, const gp_json_struct *desc,
                         const char *id, void *baseptr)
{
	const gp_json_struct *i;
	int err = 0;
	const char *str;

	err |= gp_json_obj_start(json, id);

	for (i = desc; i->id; i++) {
		switch (GP_JSON_SERDES_TYPE(i->type)) {
		case GP_JSON_SERDES_INT:
			gp_json_int_add(json, i->id, int_val(i, baseptr));
		break;
		case GP_JSON_SERDES_UINT:
			gp_json_int_add(json, i->id, uint_val(i, baseptr));
		break;
		case GP_JSON_SERDES_FLOAT:
			gp_json_float_add(json, i->id, float_val(i, baseptr));
		break;
		case GP_JSON_SERDES_STR:
			str = str_ptr(i, baseptr);
			if (str)
				gp_json_str_add(json, i->id, str);
			else
				gp_json_null_add(json, i->id);
		break;
		}
	}

	err |= gp_json_obj_finish(json);

	return err;
}

int gp_json_save_struct(const char *path,
                        const gp_json_struct *desc, void *baseptr)
{
	gp_json_writer *json = gp_json_writer_file_open(path);

	if (!json)
		return 1;

	gp_json_write_struct(json, desc, NULL, baseptr);

	if (!gp_json_writer_file_close(json))
		return 1;

	return 0;
}
