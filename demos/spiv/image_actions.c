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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Image Actions.

   Image action is an arbitrary command with printf-like syntax for image name,
   format and path.

   %f - image path
   %F - shell escaped path

   %e - file extension

   %n - image name (without extension)
   %N - escaped image name (without extension)

  */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ACTION_MAX 10u

struct action_param {
	char type;
	int pos;
};

struct action {
	char *cmd;
	unsigned int param_cnt;
	struct action_param params[];
};

static struct action *actions[ACTION_MAX];

static int find_modifiers(const char *cmd, struct action_param *params)
{
	int i, flag = 0, mod_cnt = 0;

	/* validate action command and count modifiers */
	for (i = 0; cmd[i]; i++) {
		if (flag) {
			switch (cmd[i]) {
			case 'f':
			case 'F':
			case 'n':
			case 'N':
			case 'e':
				if (params) {
					params[mod_cnt].type = cmd[i];
					params[mod_cnt].pos = i;
				}
				mod_cnt++;
			break;
			case '%':
			break;
			default:
				fprintf(stderr, "Invalid action modifier %%%c",
				        cmd[i]);
				return -1;
			}
			flag = 0;
		} else {
			if (cmd[i] == '%')
				flag = 1;
		}
	}

	return mod_cnt;
}

int image_action_set(unsigned int action, const char *cmd)
{
	char *cmd_dup;
	int mod_cnt;

	if (action > ACTION_MAX) {
		fprintf(stderr, "Invalid action slot %u, ACTION_MAX = %u\n",
		        action, ACTION_MAX);
		return 1;
	}

	mod_cnt = find_modifiers(cmd, NULL);

	if (mod_cnt < 0)
		return 1;

	if (actions[action]) {
		free(actions[action]->cmd);
		free(actions[action]);
	}

	action[actions] = malloc(sizeof(struct action) +
	                         sizeof(struct action_param) * mod_cnt);
	cmd_dup = strdup(cmd);

	if (!actions[action] || !cmd_dup) {
		free(actions[action]);
		free(cmd_dup);
		actions[action] = 0;
		fprintf(stderr, "Failed to strdup() command\n");
		return 1;
	}

	action[actions]->cmd = cmd_dup;
	action[actions]->param_cnt = mod_cnt;
	find_modifiers(cmd_dup, action[actions]->params);

	return 0;
}

static size_t get_name_ext(const char *path,
                           const char **name, const char **extension)
{
	int i, len = strlen(path);
	size_t name_len = 0;

	*name = NULL;
	*extension = NULL;

	for (i = len; i > 0; i--) {

		if (*extension)
			name_len++;

		switch (path[i]) {
		case '.':
			if (!*extension)
				*extension = &path[i+1];
		break;
		case '/':
			if (!*name) {
				*name = &path[i+1];
				return name_len - 1;
			}
		break;
		}
	}

	*name = path;

	if (*extension)
		return name_len - 1;

	return name_len;
}

static int escape(const char *name, size_t name_len,
                  char *buf, size_t buf_len)
{
	unsigned int i, j = 0;

	buf[0] = '\'';

	for (i = 1; j < name_len && i + 2 < buf_len; j++) {
		switch (name[j]) {
		case '\'':
			if (i + 5 < buf_len)
				return -1;

			buf[i++] = '\'';
			buf[i++] = '"';
			buf[i++] = '\'';
			buf[i++] = '"';
			buf[i++] = '\'';
		break;
		default:
			buf[i++] = name[j];
		}
	}

	buf[i] = '\'';
	buf[++i] = '\0';

	return i;
}

/* Global cmd buffer */
static char *cmd = NULL;
static size_t cmd_size;
static size_t cmd_pos;

static int cmd_append(const char *str, size_t len)
{
	if (len == 0)
		len = strlen(str);

	/* Initial allocation size */
	if (!cmd) {
		cmd = malloc(1024);
		cmd_size = 1024;
	}

	if (!cmd) {
		fprintf(stderr, "Failed to allocated command buffer\n");
		return 1;
	}

	if (cmd_size - cmd_pos <= len) {
		char *new_cmd = realloc(cmd, cmd_size + 1024);

		if (new_cmd == NULL) {
			fprintf(stderr, "Failed to allocated command buffer\n");
			return 1;
		}
	}

	memcpy(cmd + cmd_pos, str, len);
	cmd_pos += len;
	cmd[cmd_pos] = '\0';

	return 0;
}

static int cmd_append_escape(const char *str, size_t len)
{
	int ret;

	if (len == 0)
		len = strlen(str);

	while ((ret = escape(str, len, cmd + cmd_pos, cmd_size - cmd_pos)) < 0) {
		char *new_cmd = realloc(cmd, cmd_size + 1024);

		if (new_cmd == NULL) {
			fprintf(stderr, "Failed to allocated command buffer\n");
			return 1;
		}
	}

	cmd_pos += ret;

	return 0;
}

static void cmd_reset(void)
{
	cmd_pos = 0;
}

static int prepare_cmd(unsigned int action, const char *img_path)
{
	const char *img_name;
	const char *img_extension;
	unsigned int i, name_len;

	name_len = get_name_ext(img_path, &img_name, &img_extension);

	cmd_reset();

	char *prev = actions[action]->cmd;
	size_t len;

	for (i = 0; i < actions[action]->param_cnt; i++) {
		len = actions[action]->params[i].pos;

		if (i > 0)
			len -= actions[action]->params[i - 1].pos + 1;

		if (cmd_append(prev, len - 1))
			return 1;

		switch (actions[action]->params[i].type) {
		case 'f':
			cmd_append(img_path, 0);
		break;
		case 'F':
			cmd_append_escape(img_path, 0);
		break;
		case 'e':
			cmd_append(img_extension, 0);
		break;
		case 'n':
			cmd_append(img_name, name_len);
		break;
		case 'N':
			cmd_append_escape(img_name, name_len);
		break;
		}

		prev += len + 1;
	}

	cmd_append(prev, 0);

	return 0;
}

int image_action_run(unsigned int action, const char *img_path)
{
	if (!actions[action]) {
		fprintf(stderr, "Undefined action %u\n", action);
		return 1;
	}

	if (prepare_cmd(action, img_path))
		return 1;

	printf("Executing cmd \"%s\"\n", cmd);

	if (system(cmd)) {
		fprintf(stderr, "Failed to execute cmd '%s': %s", cmd, strerror(errno));
		return 1;
	}

	return 0;
}
