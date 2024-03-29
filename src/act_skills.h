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

#ifndef __ROM_ACT_SKILLS_H
#define __ROM_ACT_SKILLS_H

#include "merc.h"

/* Sub-routines and filters. */
void do_skills_or_spells (CHAR_T *ch, char *argument, int spells);

/* Commands. */
DECLARE_DO_FUN (do_gain);
DECLARE_DO_FUN (do_skills);
DECLARE_DO_FUN (do_spells);
DECLARE_DO_FUN (do_abilities);
DECLARE_DO_FUN (do_groups);
DECLARE_DO_FUN (do_train);
DECLARE_DO_FUN (do_practice);
DECLARE_DO_FUN (do_cast);

/* Types of stats gainable via 'train'. */
DECLARE_TRAIN_STAT_FUN (train_stat_cost_stat);
DECLARE_TRAIN_STAT_FUN (train_stat_can_stat);
DECLARE_TRAIN_STAT_FUN (train_stat_do_stat);
DECLARE_TRAIN_STAT_FUN (train_stat_do_hp_mana);

#endif
