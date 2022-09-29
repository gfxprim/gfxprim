// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_json.h>
#include <utils/gp_utf.h>
#include <input/gp_event.h>
#include <input/gp_keys.h>
#include <input/gp_event_queue.h>
#include <input/gp_keymap.h>

struct gp_keymap_us {
	int (*event_key)(gp_keymap *self, gp_event_queue *queue, gp_event *ev);
	uint8_t lshift_state:1;
	uint8_t rshift_state:1;
	uint8_t caps_state:1;
	uint8_t lctrl_state:1;
	uint8_t rctrl_state:1;
};

static char keys_to_ascii[] = {
	   0x00, 0x00,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
	    '9',  '0',  '-',  '=', 0x08, '\t',  'q',  'w',  'e',  'r',
	    't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n', 0x00,
	    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	   '\'',  '`', 0x00, '\\',  'z',  'x',  'c',  'v',  'b',  'n',
	    'm',  ',',  '.',  '/', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

static char keys_to_ascii_shift[] = {
	   0x00, 0x00,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',
	    '(',  ')',  '_',  '+', 0x08, '\t',  'Q',  'W',  'E',  'R',
	    'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n', 0x00,
	    'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
	    '"',  '~', 0x00,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',
	    'M',  '<',  '>',  '?', 0x00,  '*', 0x00,  ' ', 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	    '2',  '3',  '0',  '.'
};

static int event_key_us(gp_keymap *self, gp_event_queue *queue, gp_event *ev)
{
	unsigned int key = ev->key.key;
	struct gp_keymap_us *keymap_us = (void*)self;
	uint32_t utf;

	switch (key) {
	case GP_KEY_LEFT_SHIFT:
		keymap_us->lshift_state = !!ev->code;
	break;
	case GP_KEY_RIGHT_SHIFT:
		keymap_us->rshift_state = !!ev->code;
	break;
	case GP_KEY_CAPS_LOCK:
		if (ev->code == GP_EV_KEY_DOWN) {
			keymap_us->caps_state = !keymap_us->caps_state;
			gp_ev_feedback_op op = {
				.op = keymap_us->caps_state ? GP_EV_LEDS_ON : GP_EV_LEDS_OFF,
				.val = GP_KBD_LED_CAPS_LOCK,
			};
			gp_ev_queue_feedback_set_all(queue, &op);
		}
	break;
	case GP_KEY_LEFT_CTRL:
		keymap_us->lctrl_state = !!ev->code;
	break;
	case GP_KEY_RIGHT_CTRL:
		keymap_us->rctrl_state = !!ev->code;
	break;
	}

	if (keymap_us->lctrl_state || keymap_us->rctrl_state)
		return 0;

	if (ev->code != GP_EV_KEY_DOWN)
		return 0;

	if (key >= sizeof(keys_to_ascii))
		return 0;

	uint8_t uppecase = keymap_us->lshift_state ||
	                   keymap_us->rshift_state ||
			   keymap_us->caps_state;

	if (uppecase)
		utf = keys_to_ascii_shift[key];
	else
		utf = keys_to_ascii[key];

	if (utf >= 0x20)
		gp_event_queue_push_utf(queue, utf, ev->time);

	return 0;
}

static struct gp_keymap_us keymap_us = {
	.event_key = event_key_us,
};

#define MAP_MOD_STATES_MAX 15

struct map_mods {
	uint32_t cnt;
	uint32_t states[MAP_MOD_STATES_MAX];
};

struct map {
	uint32_t min_key;
	uint32_t max_key;

	struct map_mods mods;

	struct map *next;

	uint32_t map[];
};

enum mod_type {
	MODS_DEAD,
	MODS_LOCK,
	MODS_MOD,
};

struct mod {
	enum mod_type type;
	uint32_t key;
};

struct mods {
	uint32_t state;
	unsigned int cnt;
	struct mod mods[];
};

static int mod_is_set(struct mods *mods, unsigned int i)
{
	return mods->state & (1<<i);
}

static void mod_set(struct mods *mods, unsigned int i)
{
	mods->state |= (1<<i);
}

static void mod_clear(struct mods *mods, unsigned int i)
{
	mods->state &= ~(1<<i);
}

static int mod_idx(struct mods *mods, uint32_t key)
{
	unsigned int i;

	for (i = 0; i < mods->cnt; i++) {
		if (key == mods->mods[i].key)
			return i;
	}

	return -1;
}

static int get_key_char(gp_json_reader *json, gp_json_val *val,
                        int *key, uint32_t *utf_ch)
{
	*key = 0;

	GP_JSON_OBJ_FOREACH(json, val) {
		if (*key) {
			gp_json_err(json, "Double key in map!");
			return 1;
		}

		if (val->type != GP_JSON_STR) {
			gp_json_err(json, "Wrong map entry, key must map to a string!");
			return 1;
		}

		*key = gp_event_key_val(val->id);
		if (*key < 0) {
			gp_json_err(json, "Invalid key name!");
			return 1;
		}

		const char *utf_str = val->val_str;

		*utf_ch = gp_utf8_next(&utf_str);

		if (*utf_str) {
			gp_json_err(json, "Key can map to a single character only!");
			return 1;
		}
	}

	return 0;
}

static struct map *load_map_keys(gp_json_reader *json, gp_json_val *val)
{
	int min_key = -1;
	int max_key = -1;
	int key;
	uint32_t utf_ch;

	gp_json_state map_start = gp_json_state_start(json);

	GP_JSON_ARR_FOREACH(json, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			if (get_key_char(json, val, &key, &utf_ch))
				return NULL;

			if (min_key == -1) {
				min_key = key;
				max_key = key;
			}

			min_key = GP_MIN(min_key, key);
			max_key = GP_MAX(max_key, key);
		break;
		default:
			gp_json_err(json, "Wrong map entry, expected object");
			return NULL;
		}
	}

	if (min_key == -1) {
		gp_json_err(json, "No keys in map!");
		return NULL;
	}

	size_t map_size = sizeof(struct map) + (max_key-min_key+1) * sizeof(uint32_t);
	struct map *ret = malloc(map_size);
	if (!ret) {
		gp_json_err(json, "Malloc failed");
		return NULL;
	}

	memset(ret, 0, map_size);

	gp_json_state_load(json, map_start);

	GP_JSON_ARR_FOREACH(json, val) {
		get_key_char(json, val, &key, &utf_ch);
		ret->map[key - min_key] = utf_ch;

		GP_DEBUG(3, "Mapped %i -> %i", key, utf_ch);
	}

	ret->min_key = min_key;
	ret->max_key = max_key;

	return ret;
}

static struct gp_json_obj_attr map_attrs[] = {
	GP_JSON_OBJ_ATTR("keys", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR("mods", GP_JSON_ARR),
};

static struct gp_json_obj map_obj_filter = {
	.attrs = map_attrs,
	.attr_cnt = GP_ARRAY_SIZE(map_attrs),
};

enum map_keys {
	MAP_KEYS,
	MAP_MODS,
};

static uint32_t parse_mod_state(gp_json_reader *json, gp_json_val *val, struct mods *mods)
{
	uint32_t ret = 0;
	int key, idx;

	GP_JSON_ARR_FOREACH(json, val) {
		if (val->type != GP_JSON_STR) {
			gp_json_err(json, "Expected key name");
			return 0;
		}

		key = gp_event_key_val(val->val_str);
		if (key < 0) {
			gp_json_err(json, "Invalid key name!");
			return 0;
		}

		idx = mod_idx(mods, key);
		if (idx < 0) {
			gp_json_err(json, "Key is not a mod key!");
			return 0;
		}

		ret |= (1<<idx);
	}

	return ret;
}

static void load_map_mods(gp_json_reader *json, gp_json_val *val, struct mods *mods, struct map_mods *map_mods)
{
	uint32_t i = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		if (val->type != GP_JSON_ARR) {
			gp_json_err(json, "Expected array of key names");
			return;
		}

		if (i >= MAP_MOD_STATES_MAX) {
			gp_json_err(json, "Too many mod states!");
			return;
		}

		map_mods->states[i] = parse_mod_state(json, val, mods);
		GP_DEBUG(5, "Mod mask 0x%04x", map_mods->states[i]);
		i++;
	}

	map_mods->cnt = i;
}

static struct map *load_map(gp_json_reader *json, gp_json_val *val, struct mods *mods)
{
	struct map *ret = NULL;
	struct map_mods map_mods = {};

	GP_JSON_OBJ_FILTER(json, val, &map_obj_filter, NULL) {
		switch (val->idx) {
		case MAP_KEYS:
			if (ret) {
				gp_json_err(json, "Only one keys map can be defined!");
				free(ret);
				return NULL;
			}

			ret = load_map_keys(json, val);
		break;
		case MAP_MODS:
			load_map_mods(json, val, mods, &map_mods);
		break;
		}
	}

	if (ret)
		ret->mods = map_mods;

	return ret;
}

static void free_maps(struct map *maps)
{
	struct map *i = maps, *f;

	while (i) {
		f = i;
		i = i->next;
		free(f);
	}
}

static struct gp_json_obj_attr mods_attrs[] = {
	GP_JSON_OBJ_ATTR("dead", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("lock", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("mod", GP_JSON_STR),
};

static struct gp_json_obj mods_obj_filter = {
	.attrs = mods_attrs,
	.attr_cnt = GP_ARRAY_SIZE(mods_attrs),
};

static struct mods *load_mods(gp_json_reader *json, gp_json_val *val)
{
	unsigned int mods_cnt = 0, i = 0;
	struct mods *ret;

	gp_json_state mods_start = gp_json_state_start(json);

	GP_JSON_OBJ_FILTER(json, val, &mods_obj_filter, NULL) {
		switch (val->idx) {
		case MODS_DEAD:
		case MODS_MOD:
		case MODS_LOCK:
			if (gp_event_key_val(val->val_str) < 0) {
				gp_json_err(json, "Invalid key name!");
				return NULL;
			}

			mods_cnt++;
		break;
		}
	}

	if (!mods_cnt) {
		GP_WARN("Empty mods entry");
		return NULL;
	}

	if (mods_cnt > 32) {
		GP_WARN("Keymap supports up to 32 mods, have %u", mods_cnt);
		return NULL;
	}

	ret = malloc(sizeof(struct mods) + mods_cnt * sizeof(struct mod));
	if (!ret) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	gp_json_state_load(json, mods_start);

	ret->cnt = mods_cnt;
	ret->state = 0;

	GP_JSON_OBJ_FILTER(json, val, &mods_obj_filter, NULL) {
		ret->mods[i].type = val->idx;
		ret->mods[i].key = gp_event_key_val(val->val_str);

		GP_DEBUG(3, "Mod type %s key %s", val->id, val->val_str);

		i++;
	}

	return ret;
}

static struct gp_json_obj_attr keymap_attrs[] = {
	GP_JSON_OBJ_ATTR("map", GP_JSON_OBJ),
	GP_JSON_OBJ_ATTR("mods", GP_JSON_OBJ),
};

static struct gp_json_obj keymap_obj_filter = {
	.attrs = keymap_attrs,
	.attr_cnt = GP_ARRAY_SIZE(keymap_attrs),
};

enum keymap_keys {
	MAP,
	MODS,
};

static int load_keymap(gp_json_reader *json, struct map **rmaps, struct mods **rmods)
{
	char buf[128];
	gp_json_val val = {
		.buf = buf,
		.buf_size = sizeof(buf),
	};
	struct map *maps = NULL, *tmp;
	struct mods *mods = NULL;
	int err;

	GP_JSON_OBJ_FILTER(json, &val, &keymap_obj_filter, NULL) {
		switch (val.idx) {
		case MAP:
			if (!mods) {
				gp_json_err(json, "mods must be defined before keymaps");
				return 1;
			}

			GP_DEBUG(2, "Loading keymap map");
			tmp = load_map(json, &val, mods);
			if (tmp) {
				tmp->next = maps;
				maps = tmp;
			}
		break;
		case MODS:
			if (mods) {
				gp_json_err(json, "Duplicit mods entry!");
				free_maps(maps);
				free(mods);
				return 1;
			}
			GP_DEBUG(2, "Loading keymap mods");
			mods = load_mods(json, &val);
		break;
		}
	}

	err = gp_json_reader_err(json);
	if (err)
		gp_json_err_print(json);
	else if (!gp_json_empty(json))
		gp_json_warn(json, "Garbage after JSON string!");

	if (err) {
		free_maps(maps);
		free(mods);
		return 1;
	}

	*rmaps = maps;
	*rmods = mods;

	return 0;
}

static int map_key(struct map *map, struct mods *mods, gp_event_queue *queue, gp_event *ev)
{
	unsigned int i;
	uint32_t utf;

	if (ev->key.key < map->min_key || ev->key.key > map->max_key)
		return 0;

	for (i = 0; i < map->mods.cnt; i++) {
		if (map->mods.states[i] == mods->state) {
			utf = map->map[ev->key.key - map->min_key];
			if (utf) {
				gp_event_queue_push_utf(queue, utf, ev->time);
				GP_DEBUG(5, "Mapping %i to %u", ev->key.key,
				         map->map[ev->key.key - map->min_key]);
				return 0;
			}
		}
	}

	return 0;
}

static void process_mods(struct mods *mods, gp_event *ev)
{
	unsigned int i;

	for (i = 0; i < mods->cnt; i++) {
		if (ev->code == GP_EV_KEY_DOWN &&
		    mods->mods[i].type == MODS_DEAD &&
		    mod_is_set(mods, i)) {
			mod_clear(mods, i);
		}

		if (mods->mods[i].key != ev->key.key)
			continue;

		switch (mods->mods[i].type) {
		case MODS_DEAD:
			if (ev->code == GP_EV_KEY_DOWN)
				mod_set(mods, i);
		break;
		case MODS_MOD:
			switch (ev->code) {
			case GP_EV_KEY_UP:
				mod_clear(mods, i);
			break;
			case GP_EV_KEY_DOWN:
				mod_set(mods, i);
			break;
			}
		break;
		case MODS_LOCK:
			if (ev->code == GP_EV_KEY_DOWN) {
				if (mod_is_set(mods, i))
					mod_clear(mods, i);
				else
					mod_set(mods, i);
			}
		break;
		}
	}

	GP_DEBUG(5, "Mod mask 0x%04x", mods->state);
}

static int keymap_key(gp_keymap *self, gp_event_queue *queue, gp_event *ev)
{
	struct map *i;
	struct mods *mods = self->priv2;

	if (ev->code != GP_EV_KEY_UP) {
		for (i = self->priv; i; i = i->next)
			map_key(i, mods, queue, ev);
	}

	process_mods(mods, ev);

	return 0;
}

static gp_keymap *keymap_json_load(gp_json_reader *json)
{
	gp_keymap *keymap;

	keymap = malloc(sizeof(gp_keymap));
	if (!keymap)
		return NULL;

	if (load_keymap(json, (void*)&keymap->priv, (void*)&keymap->priv2)) {
		free(keymap);
		return NULL;
	}

	keymap->event_key = keymap_key;

	return keymap;
}

gp_keymap *gp_keymap_json_load(const char *json_str)
{
	gp_json_reader json = GP_JSON_READER_INIT(json_str, strlen(json_str));

	return keymap_json_load(&json);
}

void gp_keymap_free(gp_keymap *self)
{
	if (!self)
		return;

	if (self == (gp_keymap*)&keymap_us)
		return;

	free_maps(self->priv);
	free(self->priv2);
	free(self);
}

gp_keymap *gp_keymap_load(const char *name)
{
	GP_DEBUG(1, "Loading '%s' keymap", name ? name : "default");

	if (!name || !strcmp(name, "us"))
		return (gp_keymap*)&keymap_us;

	GP_WARN("Keymap '%s' not found", name);

	return (gp_keymap*)&keymap_us;
}
