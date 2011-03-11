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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  This interface is modeled after linux input event system.
  
 */

#ifndef GP_EVENT_H
#define GP_EVENT_H

/* events */

typedef enum GP_EventType {
	GP_EVENT_KEY = 0x01, /* key up/down event */
	GP_EVENT_REL = 0x02, /* relative XY event */
	GP_EVENT_ABS = 0x03, /* absolute XY value */
} GP_EventType;

/* subevents */

typedef enum GP_EventKeyType {
	GP_KEY_UP   = 0x00,
	GP_KEY_DOWN = 0x01,
} GP_EventKeyType;

typedef enum GP_EventRelType {
	GP_REL_MOVE     = 0x00,
	GP_REL_BTN_UP   = 0x01,
	GP_REL_BTN_DOWN = 0x02,
	GP_REL_WHEEL    = 0x03,
} GP_EventRelType;

typedef enum GP_EventAbsType {
	GP_ABS_MOVE     = 0x00,
	GP_ABS_PEN_UP   = 0x01,
	GP_ABS_PEN_DOWN = 0x02,
} GP_EventAbsType;

/* data structures */

typedef struct GP_EventAbs {
	/* event type */
	GP_EventType type;
	/* device id */
	int dev_id;
	/* possition */
	int x;
	int y;
	/* size */
	int w;
	int h;
} GP_EventAbs;

typedef struct GP_EventRel {
	GP_EventType type;
	/* device id */
	int dev_id;
	/* possition */
	int x;
	int y;
	/* relative movement */
	int rx;
	int ry;
	int rwheel;
	/* size */
	int w;
	int h;
} GP_EventRel;

typedef struct GP_EventKey {
	/* event type */
	GP_EventType type;
	/* device id */
	int dev_id;
	/* key */
	int key;
} GP_EventKey;

typedef union GP_EventPtr {
	GP_EventAbs *abs;
	GP_EventRel *rel;
	GP_EventKey *key;
} GP_EventPtr;

typedef struct GP_Time {
	long sec;
	long usec;
} GP_Time;

typedef struct GP_Event {
	/* GP_EventType */
	int type;
	/* GP_Event???Type */
	int stype;
	/* time */
	GP_Time time;
	/* event data */
	GP_EventPtr data;
} GP_Event;

#endif /* GP_EVENT_H */
