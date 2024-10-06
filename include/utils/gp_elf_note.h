// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_elf_note.h
 * @brief An ELF note implementation.
 */

#ifndef GP_ELF_NOTE_H
#define GP_ELF_NOTE_H

#include <core/gp_common.h>
#include <stdint.h>

#define GP_ELF_NOTE_VENDOR "gfxprim"

/**
 * @brief ELF note header as defined in ELF ABI.
 *
 * The note allows to store descsz bytes at the end of the note.
 */
struct gp_elf_note_hdr {
        /** @brief 4 bytes of note name size. */
	uint32_t namesz;
	/** @brief 4 bytes of note description size. */
        uint32_t descsz;
	/** @brief 4 bytes of note type. */
        uint32_t type;
	/** @brief A namesz long null terminated string of the entry owner, i.e. "gfxprim". */
	char name[sizeof(GP_ELF_NOTE_VENDOR)];
	/** @brief A descsz bytes of payload data. */
	char desc_data[];
};

/**
 * @brief Declares a gfxprim ELF note.
 *
 * This macro is supposed to be used as:
 *
 * @code
 * struct payload_type1 {
 *	int val;
 *	char str[64];
 * };
 *
 * enum payload_types {
 *	...
 *	PAYLOAD_TYPE1,
 *	...
 * };
 *
 * GP_ELF_NOTE("namespace", PAYLOAD_TYPE1, struct payload desc,
 *             sizeof(struct payload), {.val = 1, .str = "test"});
 *
 * @endcode
 *
 * @param note_namespace A subsection to store the note to. The section is
 *                       constructed as ".note.vendor.namespace". The namespace
 *                       is stored in ELF headers however it's not accessible
 *                       to the program at runtime.
 *
 * @param note_type This is usually an enum of all note types defined by the
 *                  vendor and identifies what is stored in the note payload.
 *
 * @param note_c_type A C type that is used to store the payload to.
 *
 * @param note_c_type_size A C type size, usually sizeof(note_c_type), but can
 *                         be larger in the case of a flexible array.
 *
 * @param ... An initializer for note_c_type, e.g. {.foo = bar, ...}.
 */
#define GP_ELF_NOTE(note_namespace, note_type, note_c_type, note_c_type_size, ...) \
	__attribute__((used, section(".note." GP_ELF_NOTE_VENDOR "." note_namespace), aligned(4))) \
	static const struct { \
		const struct gp_elf_note_hdr hdr; \
		const note_c_type; \
	} GP_UNIQUE_ID(gp_elf_note_) = { \
		.hdr = { \
			.namesz = sizeof(GP_ELF_NOTE_VENDOR), \
			.descsz = note_c_type_size, \
			.type = note_type, \
			.name = GP_ELF_NOTE_VENDOR, \
		}, \
		.desc = __VA_ARGS__, \
	};

/**
 * @brief Declares a gfxprim string note.
 *
 * This is a simplified interface for the GP_ELF_NOTE().
 *
 * @param note_namespace A subsection to store the note to. The section is
 *                       constructed as ".note.vendor.namespace". The namespace
 *                       is stored in ELF headers however it's not accessible
 *                       to the program at runtime.
 *
 * @param note_type This is usually an enum of all note types defined by the
 *                  vendor and identifies what is stored in the note payload.
 *
 * @param note_str A string that is stored in the note, i.e. the note payload.
 */
#define GP_ELF_NOTE_STR(note_namespace, note_type, note_str) \
	GP_ELF_NOTE(note_namespace, note_type, char desc[sizeof(note_str)], sizeof(note_str), note_str)

/**
 * @brief Runs a callback on each gfxprim ELF note.
 *
 * This function is build on a top of a dl_iterate_phdr() and filters out
 * everything that does not have vendor set to 'gfxprim'.
 *
 * @callback A callback to be called for each note.
 * @priv A priv pointer to be passed to the callback.
 */
void gp_elf_notes_process(void (*callback)(uint32_t note_type, const void *note_desc, void *priv),
                          void *priv);

#endif /* GP_ELF_NOTE_H */
