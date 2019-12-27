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

/* Globals. */
sh_int gsn_backstab;
sh_int gsn_dodge;
sh_int gsn_envenom;
sh_int gsn_hide;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_sneak;
sh_int gsn_steal;

sh_int gsn_disarm;
sh_int gsn_enhanced_damage;
sh_int gsn_kick;
sh_int gsn_parry;
sh_int gsn_rescue;
sh_int gsn_second_attack;
sh_int gsn_third_attack;

sh_int gsn_blindness;
sh_int gsn_charm_person;
sh_int gsn_curse;
sh_int gsn_invis;
sh_int gsn_mass_invis;
sh_int gsn_poison;
sh_int gsn_plague;
sh_int gsn_sleep;
sh_int gsn_sanctuary;
sh_int gsn_fly;

sh_int gsn_axe;
sh_int gsn_dagger;
sh_int gsn_flail;
sh_int gsn_mace;
sh_int gsn_polearm;
sh_int gsn_shield_block;
sh_int gsn_spear;
sh_int gsn_sword;
sh_int gsn_whip;

sh_int gsn_bash;
sh_int gsn_berserk;
sh_int gsn_dirt;
sh_int gsn_hand_to_hand;
sh_int gsn_trip;

sh_int gsn_fast_healing;
sh_int gsn_haggle;
sh_int gsn_lore;
sh_int gsn_meditation;

sh_int gsn_scrolls;
sh_int gsn_staves;
sh_int gsn_wands;
sh_int gsn_recall;
sh_int gsn_frenzy;

/* for returning skill information */
int char_get_skill (const CHAR_T *ch, int sn) {
    int skill;

    /* ignore invalid skills. */
    if (sn < -1 || sn >= SKILL_MAX) {
        bug ("Bad sn %d in char_get_skill.", sn);
        return 0;
    }

    /* shorthand for level based skills */
    if (sn == -1)
        skill = ch->level * 5 / 2;
    /* players */
    else if (!IS_NPC (ch)) {
        if (ch->level < skill_table[sn].classes[ch->class].level)
            skill = 0;
        else
            skill = ch->pcdata->learned[sn];
    }
    /* mobiles */
    else
        skill = char_get_mobile_skill (ch, sn);

    /* dazed characters have bad skills. */
    if (ch->daze > 0) {
        if (skill_table[sn].spell_fun == spell_null)
            skill = 2 * skill / 3;
        else
            skill /= 2;
    }

    /* drunks are bad at things. */
    if (IS_DRUNK (ch))
        skill = 9 * skill / 10;

    return URANGE (0, skill, 100);
}

int char_get_mobile_skill (const CHAR_T *ch, int sn) {
    /* TODO: this should probably be a table of some sort... */
    if (skill_table[sn].spell_fun != spell_null)
        return 40 + 2 * ch->level;
    else if (sn == gsn_sneak || sn == gsn_hide)
        return ch->level * 2 + 20;
    else if ((sn == gsn_dodge && IS_SET (ch->off_flags, OFF_DODGE))
             || (sn == gsn_parry && IS_SET (ch->off_flags, OFF_PARRY)))
        return ch->level * 2;
    else if (sn == gsn_shield_block)
        return 10 + 2 * ch->level;
    else if (sn == gsn_second_attack && (IS_SET (ch->mob, MOB_WARRIOR)
                                         || IS_SET (ch->mob, MOB_THIEF)))
        return 10 + 3 * ch->level;
    else if (sn == gsn_third_attack && IS_SET (ch->mob, MOB_WARRIOR))
        return 4 * ch->level - 40;
    else if (sn == gsn_hand_to_hand)
        return 40 + 2 * ch->level;
    else if (sn == gsn_trip && IS_SET (ch->off_flags, OFF_TRIP))
        return 10 + 3 * ch->level;
    else if (sn == gsn_bash && IS_SET (ch->off_flags, OFF_BASH))
        return 10 + 3 * ch->level;
    else if (sn == gsn_disarm && (IS_SET (ch->off_flags, OFF_DISARM)
                                  || IS_SET (ch->mob, MOB_WARRIOR)
                                  || IS_SET (ch->mob, MOB_THIEF)))
        return 20 + 3 * ch->level;
    else if (sn == gsn_berserk && IS_SET (ch->off_flags, OFF_BERSERK))
        return 3 * ch->level;
    else if (sn == gsn_kick)
        return 10 + 3 * ch->level;
    else if (sn == gsn_backstab && IS_SET (ch->mob, MOB_THIEF))
        return 20 + 2 * ch->level;
    else if (sn == gsn_rescue)
        return 40 + ch->level;
    else if (sn == gsn_recall)
        return 40 + ch->level;
    else if (sn == gsn_sword || sn == gsn_dagger || sn == gsn_spear ||
             sn == gsn_mace  || sn == gsn_axe    || sn == gsn_flail ||
             sn == gsn_whip  || sn == gsn_polearm)
        return 40 + 5 * ch->level / 2;
    else
        return 0;
}

void char_list_skills_and_groups (CHAR_T *ch, bool chosen) {
    const SKILL_GROUP_T *group;
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
        "group", "cp", "group", "cp", "group", "cp");

    col = 0;
    for (gn = 0; gn < SKILL_GROUP_MAX; gn++) {
        if ((group = skill_group_get (gn)) == NULL)
            break;
        if (group->classes[ch->class].cost <= 0)
            continue;
        if (ch->pcdata->group_known[gn] != chosen)
            continue;
        if (ch->gen_data && ch->gen_data->group_chosen[gn] != chosen)
            continue;

        if (!!ch->pcdata->group_known[gn] == !!chosen) {
            printf_to_char (ch, "%-18s %-5d ", group->name,
                group->classes[ch->class].cost);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
    }
    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    col = 0;
    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
        "skill", "cp", "skill", "cp", "skill", "cp");

    for (sn = 0; sn < SKILL_MAX; sn++) {
        if (skill_table[sn].name == NULL)
            break;
        if (skill_table[sn].classes[ch->class].effort <= 0)
            continue;
        if (skill_table[sn].spell_fun != spell_null)
            continue;
        if (((ch->pcdata->learned[sn] > 0) ? TRUE : FALSE) != chosen)
            continue;
        if (ch->gen_data->skill_chosen[sn] != chosen)
            continue;

        if (!!(ch->pcdata->learned[sn] != 0) == !!chosen) {
            printf_to_char (ch, "%-18s %-5d ", skill_table[sn].name,
                skill_table[sn].classes[ch->class].effort);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
    }

    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    printf_to_char (ch, "Creation points: %d\n\r", ch->gen_data->points_chosen);
    printf_to_char (ch, "Experience per level: %d\n\r",
        exp_per_level (ch, ch->pcdata->points));
}

/* this procedure handles the input parsing for the skill generator */
bool char_parse_gen_groups (CHAR_T *ch, char *argument) {
    const SKILL_T *skill;
    const SKILL_GROUP_T *group;
    char arg[MAX_INPUT_LENGTH];
    int num;

    if (argument[0] == '\0')
        return FALSE;

    argument = one_argument (argument, arg);
    if (!str_prefix (arg, "help")) {
        if (argument[0] == '\0') {
            do_function (ch, &do_help, "group help");
            return TRUE;
        }

        do_function (ch, &do_help, argument);
        return TRUE;
    }

    if (!str_prefix (arg, "add")) {
        if (argument[0] == '\0') {
            send_to_char ("You must provide a skill name.\n\r", ch);
            return TRUE;
        }

        num = skill_group_lookup (argument);
        if (num != -1) {
            group = skill_group_get (num);
            if (ch->gen_data->group_chosen[num] || ch->pcdata->group_known[num]) {
                send_to_char ("You already know that group!\n\r", ch);
                return TRUE;
            }
            if (group->classes[ch->class].cost < 1) {
                send_to_char ("That group is not available.\n\r", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->gen_data->points_chosen + group->classes[ch->class].cost > 300) {
                send_to_char ("You cannot take more than 300 creation points.\n\r", ch);
                return TRUE;
            }

            printf_to_char (ch, "Group '%s' added.\n\r", group->name);
            ch->gen_data->group_chosen[num] = TRUE;
            ch->gen_data->points_chosen += group->classes[ch->class].cost;
            char_add_skill_group (ch, num, TRUE);
            return TRUE;
        }

        num = skill_lookup (argument);
        if (num != -1) {
            skill = skill_get (num);
            if (ch->gen_data->skill_chosen[num] || ch->pcdata->learned[num] != 0) {
                send_to_char ("You already know that skill!\n\r", ch);
                return TRUE;
            }
            if (skill->classes[ch->class].effort < 1 || skill->spell_fun != spell_null) {
                send_to_char ("That skill is not available.\n\r", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->pcdata->points + skill->classes[ch->class].effort > 300) {
                send_to_char ("You cannot take more than 300 creation points.\n\r", ch);
                return TRUE;
            }

            printf_to_char (ch, "Skill '%s' added.\n\r", skill->name);
            ch->gen_data->skill_chosen[num] = TRUE;
            ch->gen_data->points_chosen += skill->classes[ch->class].effort;
            char_add_skill (ch, num, TRUE);
            return TRUE;
        }

        send_to_char ("No skills or groups by that name...\n\r", ch);
        return TRUE;
    }

    if (!strcmp (arg, "drop")) {
        if (argument[0] == '\0') {
            send_to_char ("You must provide a skill to drop.\n\r", ch);
            return TRUE;
        }

        num = skill_group_lookup (argument);
        if (num != -1 && ch->gen_data->group_chosen[num]) {
            group = skill_group_get (num);
            printf_to_char (ch, "Group '%s' dropped.\n\r", group->name);
            ch->gen_data->group_chosen[num] = FALSE;
            ch->gen_data->points_chosen -= group->classes[ch->class].cost;
            char_remove_skill_group (ch, num, TRUE);
            return TRUE;
        }

        num = skill_lookup (argument);
        if (num != -1 && ch->gen_data->skill_chosen[num]) {
            skill = skill_get (num);
            printf_to_char (ch, "Skill '%s' dropped.\n\r", skill->name);
            ch->gen_data->skill_chosen[num] = FALSE;
            ch->gen_data->points_chosen -= skill->classes[ch->class].effort;
            char_remove_skill (ch, num, TRUE);
            return TRUE;
        }

        send_to_char ("You haven't bought any such skill or group.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (arg, "premise")) {
        do_function (ch, &do_help, "premise");
        return TRUE;
    }
    if (!str_prefix (arg, "list")) {
        char_list_skills_and_groups (ch, FALSE);
        return TRUE;
    }
    if (!str_prefix (arg, "learned")) {
        char_list_skills_and_groups (ch, TRUE);
        return TRUE;
    }
    if (!str_prefix (arg, "abilities")) {
        do_function (ch, &do_abilities, "all");
        return TRUE;
    }
    if (!str_prefix (arg, "info")) {
        do_function (ch, &do_groups, argument);
        return TRUE;
    }

    return FALSE;
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

    if (ch->pcdata->learned[sn] == 0) { /* i.e. not known */
        ch->pcdata->learned[sn] = 1;
        if (deduct)
            ch->pcdata->points += skill->classes[ch->class].effort;
    }
}

void char_remove_skill (CHAR_T *ch, int sn, bool refund) {
    const SKILL_T *skill;
    if ((skill = skill_get (sn)) == NULL) {
        bugf ("char_remove_skill: Unknown skill number %d", sn);
        return;
    }

    if (ch->pcdata->learned[sn] != 0) {
        ch->pcdata->learned[sn] = 0;
        if (refund)
            ch->pcdata->points -= skill->classes[ch->class].effort;
    }
}

/* recursively adds a group given its number -- uses skill_group_add */
void char_add_skill_group (CHAR_T *ch, int gn, bool deduct) {
    const SKILL_GROUP_T *group;
    int i;

    if ((group = skill_group_get (gn)) == NULL) {
        bugf ("char_add_skill_group: Unknown group number %d", gn);
        return;
    }

    ch->pcdata->group_known[gn] = TRUE;
    if (ch->pcdata->group_known[gn] == FALSE) {
        ch->pcdata->group_known[gn] = TRUE;
        if (deduct)
            ch->pcdata->points += group->classes[ch->class].cost;
    }

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

    if (ch->pcdata->group_known[gn] == TRUE) {
        ch->pcdata->group_known[gn] = FALSE;
        if (refund)
            ch->pcdata->points -= group->classes[ch->class].cost;
    }

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
