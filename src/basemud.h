/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#ifndef __ROM_BASEMUD_H
#define __ROM_BASEMUD_H

#define BASEMUD_VERSION "0.0.6"

/* Extra information, coloring, or display changes. */
#define BASEMUD_SHOW_DOORS
#define BASEMUD_SHOW_ARRIVAL_DIRECTIONS
#define BASEMUD_SHOW_OLC_IN_PROMPT
#define BASEMUD_SHOW_ABSOLUTE_HIT_DAMAGE
#define BASEMUD_SHOW_ENHANCED_DAMAGE
#define BASEMUD_SHOW_RECOVERY_RATE
#define BASEMUD_SHOW_POSITION_IN_LOOK
#define BASEMUD_COLOR_STATUS_EFFECTS
#define BASEMUD_COLOR_ROOMS_BY_SECTOR
#define BASEMUD_MORE_PRECISE_CONDITIONS
#define BASEMUD_MORE_PRECISE_RELATIVE_DAMAGE
#define BASEMUD_MOBS_SAY_SPELLS

/* Game rule changes. */
#define BASEMUD_NO_RECALL_TO_SAME_ROOM
#define BASEMUD_NO_WORTHLESS_SACRIFICES
#define BASEMUD_GRADUAL_RECOVERY
#define BASEMUD_CAP_JOINED_AFFECTS
#define BASEMUD_DETECT_EXTREME_ALIGNMENTS
#define BASEMUD_ALLOW_STUNNED_MOBS

/* New BaseMUD commands. */
#define BASEMUD_MATERIALS_COMMAND
#define BASEMUD_DISENGAGE_COMMAND
#define BASEMUD_ORDER_ALL_COMMAND
#define BASEMUD_ABILITIES_COMMAND

/* Other BaseMUD features. */
#define BASEMUD_PIXIE_RACE
#define BASEMUD_IMPORT_JSON

#endif
