// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#define _GNU_SOURCE
#include <link.h>
#include <stddef.h>
#include <string.h>
#include <utils/gp_elf_note.h>

struct note_callback {
	void (*callback)(uint32_t, const void *, void *);
	void *priv;
};

#define ALIGN(val, align) (((val) + (align) - 1) & ~((align) - 1))

static int gfxprim_hdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
	unsigned int i;
	struct note_callback *note_callback = data;

	(void) size;

        for (i = 0; i < info->dlpi_phnum; i++) {
                if (info->dlpi_phdr[i].p_type != PT_NOTE)
                        continue;

                struct gp_elf_note_hdr *note = (void *)(info->dlpi_addr +
                                               info->dlpi_phdr[i].p_vaddr);

                ptrdiff_t len = info->dlpi_phdr[i].p_filesz;

                while (len >= (ptrdiff_t)sizeof(struct gp_elf_note_hdr)) {
                        if (note->namesz == sizeof(GP_ELF_NOTE_VENDOR) &&
                            !strcmp(note->name, GP_ELF_NOTE_VENDOR)) {
                                note_callback->callback(note->type, note->desc_data, note_callback->priv);
                        }

                        size_t offset = sizeof(ElfW(Nhdr)) +
                                    ALIGN(note->namesz, 4) +
                                    ALIGN(note->descsz, 4);
                        note = (struct gp_elf_note_hdr *)((char *)note + offset);
                        len -= offset;
                }
        }

        return 0;
}

void gp_elf_notes_process(void (*callback)(uint32_t note_type, const void *note_desc, void *priv),
                          void *priv)
{
	struct note_callback note_callback = {
		.callback = callback,
		.priv = priv,
	};

        dl_iterate_phdr(gfxprim_hdr_callback, &note_callback);
}
