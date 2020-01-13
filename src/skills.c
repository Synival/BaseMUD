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

#include <string.h>
#include <stdlib.h>

#include "lookup.h"
#include "utils.h"

#include "skills.h"

void skill_clear_mapping (void) {
    int i;

    for (i = 0; i < SKILL_MAP_MAX; i++)
        skill_map_table[i].skill_index = -1;
    for (i = 0; i < SKILL_MAX; i++) {
        skill_table[i].map_index    = -1;
        skill_table[i].weapon_index = -1;
    }
    for (i = 0; i < WEAPON_MAX; i++)
        weapon_table[i].skill_index = -1;
}

void skill_init_mapping (void) {
    SKILL_MAP_T *map;
    SKILL_T *skill;
    WEAPON_T *weapon;
    int i, index;

    /* map skills by name to our internal mapping and vice-versa. */
    for (i = 0; i < SKILL_MAP_MAX; i++) {
        map = &(skill_map_table[i]);
        if (map->name == NULL)
            continue;
        if ((index = skill_lookup_exact (map->name)) < 0) {
            bugf ("skill_init_mapping: Skill '%s' not found", map->name);
            continue;
        }
        skill = &(skill_table[index]);

        map->skill_index = index;
        skill->map_index = i;
    }

    /* map weapons to skills and vice-versa. */
    for (i = 0; i < WEAPON_MAX; i++) {
        weapon = &(weapon_table[i]);
        if (weapon->name == NULL)
            continue;
        if ((index = skill_lookup_exact (weapon->skill)) < 0) {
            bugf ("skill_init_mapping: Weapon skill '%s' not found", weapon->skill);
            continue;
        }
        skill = &(skill_table[index]);

        weapon->skill_index = index;
        skill->weapon_index = i;
    }
}
