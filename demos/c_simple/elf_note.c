// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <utils/gp_elf_note.h>

enum note_type {
	NOTE_STR,
	NOTE_PAYLOAD,
};

GP_ELF_NOTE_STR("namespace1", NOTE_STR, "NOTE 2");

struct note_payload {
	unsigned int val;
	char desc[32];
};

GP_ELF_NOTE("namespace2", NOTE_PAYLOAD, struct note_payload desc, sizeof(struct note_payload), {.desc = "test", .val = 66});

static void print_notes(uint32_t note_type, const void *note_desc, void *priv)
{
	(void) priv;
	const struct note_payload *payload = note_desc;

	printf("Note type = %04u:\n", (unsigned int)note_type);

	switch (note_type) {
	case NOTE_STR:
		printf(" desc=%-32s\n", (const char *)note_desc);
	break;
	case NOTE_PAYLOAD:
		printf(" val=%u desc=%-32s\n", payload->val, payload->desc);
	break;
	}
}

int main(void)
{
	gp_elf_notes_process(print_notes, NULL);

	return 0;
}
