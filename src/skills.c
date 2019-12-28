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

#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"
#include "affects.h"
#include "comm.h"
#include "utils.h"
#include "fight.h"
#include "act_info.h"
#include "act_skills.h"
#include "chars.h"

#include "skills.h"

void char_set_default_skills (CHAR_T *ch) {
    sh_int *learned = ch->pcdata->learned;

    if (learned[SN(RECALL)] < 50)
        learned[SN(RECALL)] = 50;
}

/* for returning skill information */
int char_get_skill (const CHAR_T *ch, int sn) {
    const SKILL_T *skill;
    int ch_skill;

    /* shorthand for level based skills */
    if (sn == -1) {
        skill = NULL;
        ch_skill = ch->level * 5 / 2;
    }
    /* ignore invalid skills. */
    else if (sn < -1 || sn >= SKILL_MAX || (skill = skill_get (sn)) == NULL) {
        bug ("char_get_skill: Bad sn %d.", sn);
        return 0;
    }

    if (skill != NULL) {
        /* players */
        if (!IS_NPC (ch)) {
            if (ch->level < skill->classes[ch->class].level)
                ch_skill = 0;
            else
                ch_skill = ch->pcdata->learned[sn];
        }
        /* mobiles */
        else
            ch_skill = char_get_mobile_skill (ch, sn);
    }

    /* dazed characters have bad skills. */
    if (ch->daze > 0) {
        if (skill == NULL || skill->spell_fun == spell_null)
            ch_skill = 2 * ch_skill / 3;
        else
            ch_skill /= 2;
    }

    /* drunks are bad at things. */
    if (IS_DRUNK (ch))
        ch_skill = 9 * ch_skill / 10;

    return URANGE (0, ch_skill, 100);
}

int char_get_mobile_skill (const CHAR_T *ch, int sn) {
    /* TODO: this should probably be a table of some sort... */
    if (skill_table[sn].spell_fun != spell_null)
        return 40 + 2 * ch->level;
    else if (sn == SN(SNEAK) || sn == SN(HIDE))
        return ch->level * 2 + 20;
    else if ((sn == SN(DODGE) && IS_SET (ch->off_flags, OFF_DODGE)) ||
             (sn == SN(PARRY) && IS_SET (ch->off_flags, OFF_PARRY)))
        return ch->level * 2;
    else if (sn == SN(SHIELD_BLOCK))
        return 10 + 2 * ch->level;
    else if (sn == SN(SECOND_ATTACK) && (IS_SET (ch->mob, MOB_WARRIOR) ||
                                         IS_SET (ch->mob, MOB_THIEF)))
        return 10 + 3 * ch->level;
    else if (sn == SN(THIRD_ATTACK) && IS_SET (ch->mob, MOB_WARRIOR))
        return 4 * ch->level - 40;
    else if (sn == SN(HAND_TO_HAND))
        return 40 + 2 * ch->level;
    else if (sn == SN(TRIP) && IS_SET (ch->off_flags, OFF_TRIP))
        return 10 + 3 * ch->level;
    else if (sn == SN(BASH) && IS_SET (ch->off_flags, OFF_BASH))
        return 10 + 3 * ch->level;
    else if (sn == SN(DISARM) && (IS_SET (ch->off_flags, OFF_DISARM) ||
                                  IS_SET (ch->mob, MOB_WARRIOR)      ||
                                  IS_SET (ch->mob, MOB_THIEF)))
        return 20 + 3 * ch->level;
    else if (sn == SN(BERSERK) && IS_SET (ch->off_flags, OFF_BERSERK))
        return 3 * ch->level;
    else if (sn == SN(KICK))
        return 10 + 3 * ch->level;
    else if (sn == SN(BACKSTAB) && IS_SET (ch->mob, MOB_THIEF))
        return 20 + 2 * ch->level;
    else if (sn == SN(RESCUE))
        return 40 + ch->level;
    else if (sn == SN(RECALL))
        return 40 + ch->level;
    else if (sn == SN(SWORD) || sn == SN(DAGGER) || sn == SN(SPEAR) ||
             sn == SN(MACE)  || sn == SN(AXE)    || sn == SN(FLAIL) ||
             sn == SN(WHIP)  || sn == SN(POLEARM))
        return 40 + 5 * ch->level / 2;
    else
        return 0;
}

void char_list_skills_and_groups (CHAR_T *ch, bool chosen) {
    const SKILL_GROUP_T *group;
    int num, col;

    if (IS_NPC (ch))
        return;

    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
        "group", "cp", "group", "cp", "group", "cp");

    col = 0;
    chosen = (!!chosen);
    for (num = 0; num < SKILL_GROUP_MAX; num++) {
        if ((group = skill_group_get (num)) == NULL || group->name == NULL)
            break;
        if (group->classes[ch->class].cost <= 0)
            continue;
        if (!!ch->pcdata->group_known[num] != chosen)
            continue;
        if (ch->gen_data && !!ch->gen_data->group_chosen[num] != chosen)
            continue;

        printf_to_char (ch, "%-18s %-5d ", group->name,
            group->classes[ch->class].cost);
        if (++col % 3 == 0)
            send_to_char ("\n\r", ch);
    }
    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    col = 0;
    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
        "skill", "cp", "skill", "cp", "skill", "cp");

    for (num = 0; num < SKILL_MAX && skill_table[num].name != NULL; num++) {
        if (skill_table[num].classes[ch->class].effort <= 0)
            continue;
        if (skill_table[num].spell_fun != spell_null)
            continue;
        if (!!ch->pcdata->skill_known[num] != chosen)
            continue;
        if (ch->gen_data && !!ch->gen_data->skill_chosen[num] != chosen)
            continue;
        if ((ch->pcdata->learned[num] > 0) != chosen)
            continue;

        printf_to_char (ch, "%-18s %-5d ", skill_table[num].name,
            skill_table[num].classes[ch->class].effort);
        if (++col % 3 == 0)
            send_to_char ("\n\r", ch);
    }

    if (col % 3 != 0)
        send_to_char ("\n\r", ch);

    /* only show generation points during character generation. */
    if (ch->gen_data) {
        send_to_char ("\n\r", ch);
        printf_to_char (ch, "Creation points: %d\n\r",
            ch->pcdata->creation_points);
        printf_to_char (ch, "Experience per level: %d\n\r", exp_per_level (ch));
    }
}

/* checks for skill improvement */
void char_try_skill_improve (CHAR_T *ch, int sn, bool success, int multiplier) {
    int chance;

    if (IS_NPC (ch))
        return;

    /* skill is not known or already mastered */
    if (ch->level < skill_table[sn].classes[ch->class].level ||
        skill_table[sn].classes[ch->class].effort == 0 ||
        ch->pcdata->learned[sn] == 0 ||
        ch->pcdata->learned[sn] == 100)
        return;

    /* check to see if the character has a chance to learn */
    chance = 10 * char_int_learn_rate (ch);
    chance /= (multiplier * skill_table[sn].classes[ch->class].effort * 4);
    chance += ch->level;

    if (number_range (1, 1000) > chance)
        return;

    /* now that the character has a CHANCE to learn, see if they really have */

    if (success) {
        chance = URANGE (5, 100 - ch->pcdata->learned[sn], 95);
        if (number_percent () < chance)
        {
            printf_to_char (ch, "{5You have become better at %s!{x\n\r",
                skill_table[sn].name);
            ch->pcdata->learned[sn]++;
            gain_exp (ch, 2 * skill_table[sn].classes[ch->class].effort);
        }
    }
    else {
        chance = URANGE (5, ch->pcdata->learned[sn] / 2, 30);
        if (number_percent () < chance) {
            printf_to_char (ch,
                "{5You learn from your mistakes, and your %s skill improves.{x\n\r",
                skill_table[sn].name);
            ch->pcdata->learned[sn] += number_range (1, 3);
            ch->pcdata->learned[sn] = UMIN (ch->pcdata->learned[sn], 100);
            gain_exp (ch, 2 * skill_table[sn].classes[ch->class].effort);
        }
    }
}

void char_add_skill (CHAR_T *ch, int sn, bool deduct) {
    const SKILL_T *skill;
    if ((skill = skill_get (sn)) == NULL) {
        bugf ("char_add_skill: Unknown skill number %d", sn);
        return;
    }

    if (!ch->pcdata->skill_known[sn] && deduct)
        ch->pcdata->creation_points += skill->classes[ch->class].effort;
    ch->pcdata->skill_known[sn]++;
    if (ch->pcdata->learned[sn] == 0)
        ch->pcdata->learned[sn] = 1;
}

void char_remove_skill (CHAR_T *ch, int sn, bool refund) {
    const SKILL_T *skill;
    if ((skill = skill_get (sn)) == NULL) {
        bugf ("char_remove_skill: Unknown skill number %d", sn);
        return;
    }

    if (ch->pcdata->skill_known[sn] <= 0)
        return;
    if (refund)
        ch->pcdata->creation_points -= skill->classes[ch->class].effort;
    ch->pcdata->skill_known[sn]--;
    if (ch->pcdata->skill_known[sn] == 0)
        ch->pcdata->learned[sn] = 0;
}

/* recursively adds a group given its number -- uses skill_group_add */
void char_add_skill_group (CHAR_T *ch, int gn, bool deduct) {
    const SKILL_GROUP_T *group;
    int i;

    if ((group = skill_group_get (gn)) == NULL) {
        bugf ("char_add_skill_group: Unknown group number %d", gn);
        return;
    }

    if (!ch->pcdata->group_known[gn] && deduct)
        ch->pcdata->creation_points += group->classes[ch->class].cost;
    ch->pcdata->group_known[gn]++;

    for (i = 0; i < MAX_IN_GROUP; i++) {
        if (group->spells[i] == NULL)
            break;
        char_add_skill_or_group (ch, group->spells[i], FALSE);
    }
}

/* recusively removes a group given its number -- uses skill_group_remove */
void char_remove_skill_group (CHAR_T *ch, int gn, bool refund) {
    const SKILL_GROUP_T *group;
    int i;

    if ((group = skill_group_get (gn)) == NULL) {
        bugf ("char_remove_skill_group: Unknown group number %d", gn);
        return;
    }

    if (ch->pcdata->group_known[gn] <= 0)
        return;
    if (refund)
        ch->pcdata->creation_points -= group->classes[ch->class].cost;
    ch->pcdata->group_known[gn]--;

    for (i = 0; i < MAX_IN_GROUP; i++) {
        if (group->spells[i] == NULL)
            break;
        char_remove_skill_or_group (ch, group->spells[i], FALSE);
    }
}

/* use for processing a skill or group for addition  */
void char_add_skill_or_group (CHAR_T *ch, const char *name, bool deduct) {
    int num;

    if (IS_NPC (ch)) /* NPCs do not have skills */
        return;

    /* first, check for skills. */
    num = skill_lookup_exact (name);
    if (num != -1) {
        char_add_skill (ch, num, deduct);
        return;
    }

    num = skill_group_lookup_exact (name);
    if (num != -1) {
        char_add_skill_group (ch, num, deduct);
        return;
    }

    bugf ("char_add_skill_or_group: Unknown skill or group '%s'", name);
}

/* used for processing a skill or group for deletion */
void char_remove_skill_or_group (CHAR_T *ch, const char *name, bool refund) {
    int num;

    /* first, check for skills. */
    num = skill_lookup_exact (name);
    if (num != -1) {
        char_remove_skill (ch, num, refund);
        return;
    }

    /* now check groups */
    num = skill_group_lookup_exact (name);
    if (num != -1) {
        char_remove_skill_group (ch, num, refund);
        return;
    }

    bugf ("char_remove_skill_or_group: Unknown skill or group '%s'", name);
}

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
