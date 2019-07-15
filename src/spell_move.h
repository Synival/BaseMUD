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

#ifndef __ROM_SPELL_MOVE_H
#define __ROM_SPELL_MOVE_H

#include "merc.h"

/* Sub-routines and filters. */
bool spell_filter_can_go_to (CHAR_DATA *ch, CHAR_DATA *victim,
    int level, flag_t res_type, flag_t dam_type);
bool spell_filter_use_warp_stone (CHAR_DATA *ch);
OBJ_DATA *spell_sub_create_portal (ROOM_INDEX_DATA *from_room,
    ROOM_INDEX_DATA *to_room, int duration);

/* Spells. */
DECLARE_SPELL_FUN (spell_gate);
DECLARE_SPELL_FUN (spell_summon);
DECLARE_SPELL_FUN (spell_teleport);
DECLARE_SPELL_FUN (spell_word_of_recall);
DECLARE_SPELL_FUN (spell_portal);
DECLARE_SPELL_FUN (spell_nexus);

#endif
