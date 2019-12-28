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

#ifndef __ROM_SKILLS_H
#define __ROM_SKILLS_H

#include "merc.h"

/* Function prototypes. */
void char_set_default_skills (CHAR_T *ch);
int char_get_skill (const CHAR_T *ch, int sn);
int char_get_mobile_skill (const CHAR_T *ch, int sn);
void char_list_skills_and_groups (CHAR_T *ch, bool chosen);
void char_try_skill_improve (CHAR_T *ch, int sn, bool success, int multiplier);
void char_add_skill (CHAR_T *ch, int sn, bool deduct);
void char_remove_skill (CHAR_T *ch, int sn, bool refund);
void char_add_skill_group (CHAR_T *ch, int gn, bool deduct);
void char_remove_skill_group (CHAR_T *ch, int gn, bool refund);
void char_add_skill_or_group (CHAR_T *ch, const char *name, bool deduct);
void char_remove_skill_or_group (CHAR_T *ch, const char *name, bool refund);
void skill_clear_mapping (void);
void skill_init_mapping (void);

#endif
