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
int get_skill (CHAR_T *ch, int sn) {
    int skill;

    /* shorthand for level based skills */
    if (sn == -1)
        skill = ch->level * 5 / 2;
    else if (sn < -1 || sn > SKILL_MAX) {
        bug ("Bad sn %d in get_skill.", sn);
        skill = 0;
    }
    else if (!IS_NPC (ch)) {
        if (ch->level < skill_table[sn].skill_level[ch->class])
            skill = 0;
        else
            skill = ch->pcdata->learned[sn];
    }
    /* mobiles */
    else {
        if (skill_table[sn].spell_fun != spell_null)
            skill = 40 + 2 * ch->level;
        else if (sn == gsn_sneak || sn == gsn_hide)
            skill = ch->level * 2 + 20;
        else if ((sn == gsn_dodge && IS_SET (ch->off_flags, OFF_DODGE))
                 || (sn == gsn_parry && IS_SET (ch->off_flags, OFF_PARRY)))
            skill = ch->level * 2;
        else if (sn == gsn_shield_block)
            skill = 10 + 2 * ch->level;
        else if (sn == gsn_second_attack && (IS_SET (ch->mob, MOB_WARRIOR)
                                             || IS_SET (ch->mob, MOB_THIEF)))
            skill = 10 + 3 * ch->level;
        else if (sn == gsn_third_attack && IS_SET (ch->mob, MOB_WARRIOR))
            skill = 4 * ch->level - 40;
        else if (sn == gsn_hand_to_hand)
            skill = 40 + 2 * ch->level;
        else if (sn == gsn_trip && IS_SET (ch->off_flags, OFF_TRIP))
            skill = 10 + 3 * ch->level;
        else if (sn == gsn_bash && IS_SET (ch->off_flags, OFF_BASH))
            skill = 10 + 3 * ch->level;
        else if (sn == gsn_disarm && (IS_SET (ch->off_flags, OFF_DISARM)
                                      || IS_SET (ch->mob, MOB_WARRIOR)
                                      || IS_SET (ch->mob, MOB_THIEF)))
            skill = 20 + 3 * ch->level;
        else if (sn == gsn_berserk && IS_SET (ch->off_flags, OFF_BERSERK))
            skill = 3 * ch->level;
        else if (sn == gsn_kick)
            skill = 10 + 3 * ch->level;
        else if (sn == gsn_backstab && IS_SET (ch->mob, MOB_THIEF))
            skill = 20 + 2 * ch->level;
        else if (sn == gsn_rescue)
            skill = 40 + ch->level;
        else if (sn == gsn_recall)
            skill = 40 + ch->level;
        else if (sn == gsn_sword
                 || sn == gsn_dagger
                 || sn == gsn_spear
                 || sn == gsn_mace
                 || sn == gsn_axe
                 || sn == gsn_flail || sn == gsn_whip || sn == gsn_polearm)
            skill = 40 + 5 * ch->level / 2;
        else
            skill = 0;
    }
    if (ch->daze > 0) {
        if (skill_table[sn].spell_fun != spell_null)
            skill /= 2;
        else
            skill = 2 * skill / 3;
    }
    if (IS_DRUNK (ch))
        skill = 9 * skill / 10;

    return URANGE (0, skill, 100);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs (CHAR_T *ch) {
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
        "group", "cp", "group", "cp", "group", "cp");

    col = 0;
    for (gn = 0; gn < GROUP_MAX; gn++) {
        if (group_table[gn].name == NULL)
            break;

        if (!ch->gen_data->group_chosen[gn]
            && !ch->pcdata->group_known[gn]
            && group_table[gn].rating[ch->class] > 0)
        {
            printf_to_char (ch, "%-18s %-5d ", group_table[gn].name,
                group_table[gn].rating[ch->class]);
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

        if (!ch->gen_data->skill_chosen[sn]
            && ch->pcdata->learned[sn] == 0
            && skill_table[sn].spell_fun == spell_null
            && skill_table[sn].rating[ch->class] > 0)
        {
            printf_to_char (ch, "%-18s %-5d ", skill_table[sn].name,
                skill_table[sn].rating[ch->class]);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
    }
    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    printf_to_char (ch, "Creation points: %d\n\r", ch->pcdata->points);
    printf_to_char (ch, "Experience per level: %d\n\r",
        exp_per_level (ch, ch->gen_data->points_chosen));
}

void list_group_chosen (CHAR_T *ch) {
    int gn, sn, col;

    if (IS_NPC (ch))
        return;

    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s",
        "group", "cp", "group", "cp", "group", "cp\n\r");

    col = 0;
    for (gn = 0; gn < GROUP_MAX; gn++) {
        if (group_table[gn].name == NULL)
            break;

        if (ch->gen_data->group_chosen[gn]
            && group_table[gn].rating[ch->class] > 0)
        {
            printf_to_char (ch, "%-18s %-5d ", group_table[gn].name,
                group_table[gn].rating[ch->class]);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
    }
    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    col = 0;
    printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s",
        "skill", "cp", "skill", "cp", "skill", "cp\n\r");

    for (sn = 0; sn < SKILL_MAX; sn++) {
        if (skill_table[sn].name == NULL)
            break;

        if (ch->gen_data->skill_chosen[sn]
            && skill_table[sn].rating[ch->class] > 0)
        {
            printf_to_char (ch, "%-18s %-5d ", skill_table[sn].name,
                skill_table[sn].rating[ch->class]);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
    }

    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
    send_to_char ("\n\r", ch);

    printf_to_char (ch, "Creation points: %d\n\r", ch->gen_data->points_chosen);
    printf_to_char (ch, "Experience per level: %d\n\r",
        exp_per_level (ch, ch->gen_data->points_chosen));
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups (CHAR_T *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    int gn, sn, i;

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

        gn = group_lookup (argument);
        if (gn != -1) {
            if (ch->gen_data->group_chosen[gn] || ch->pcdata->group_known[gn]) {
                send_to_char ("You already know that group!\n\r", ch);
                return TRUE;
            }

            if (group_table[gn].rating[ch->class] < 1) {
                send_to_char ("That group is not available.\n\r", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->gen_data->points_chosen +
                group_table[gn].rating[ch->class] > 300)
            {
                send_to_char ("You cannot take more than 300 creation points.\n\r", ch);
                return TRUE;
            }

            printf_to_char (ch, "%s group added\n\r", group_table[gn].name);
            ch->gen_data->group_chosen[gn] = TRUE;
            ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
            gn_add (ch, gn);
            ch->pcdata->points += group_table[gn].rating[ch->class];
            return TRUE;
        }

        sn = skill_lookup (argument);
        if (sn != -1) {
            if (ch->gen_data->skill_chosen[sn] || ch->pcdata->learned[sn] > 0) {
                send_to_char ("You already know that skill!\n\r", ch);
                return TRUE;
            }

            if (skill_table[sn].rating[ch->class] < 1
                || skill_table[sn].spell_fun != spell_null)
            {
                send_to_char ("That skill is not available.\n\r", ch);
                return TRUE;
            }

            /* Close security hole */
            if (ch->gen_data->points_chosen +
                skill_table[sn].rating[ch->class] > 300)
            {
                send_to_char ("You cannot take more than 300 creation points.\n\r", ch);
                return TRUE;
            }

            printf_to_char (ch, "%s skill added\n\r", skill_table[sn].name);
            ch->gen_data->skill_chosen[sn] = TRUE;
            ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
            ch->pcdata->learned[sn] = 1;
            ch->pcdata->points += skill_table[sn].rating[ch->class];
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

        gn = group_lookup (argument);
        if (gn != -1 && ch->gen_data->group_chosen[gn]) {
            send_to_char ("Group dropped.\n\r", ch);
            ch->gen_data->group_chosen[gn] = FALSE;
            ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
            gn_remove (ch, gn);
            for (i = 0; i < GROUP_MAX; i++) {
                if (ch->gen_data->group_chosen[gn])
                    gn_add (ch, gn);
            }
            ch->pcdata->points -= group_table[gn].rating[ch->class];
            return TRUE;
        }

        sn = skill_lookup (argument);
        if (sn != -1 && ch->gen_data->skill_chosen[sn]) {
            send_to_char ("Skill dropped.\n\r", ch);
            ch->gen_data->skill_chosen[sn] = FALSE;
            ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
            ch->pcdata->learned[sn] = 0;
            ch->pcdata->points -= skill_table[sn].rating[ch->class];
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
        list_group_costs (ch);
        return TRUE;
    }
    if (!str_prefix (arg, "learned")) {
        list_group_chosen (ch);
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
void check_improve (CHAR_T *ch, int sn, bool success, int multiplier) {
    int chance;

    if (IS_NPC (ch))
        return;

    /* skill is not known or already mastered */
    if (ch->level < skill_table[sn].skill_level[ch->class]
        || skill_table[sn].rating[ch->class] == 0
        || ch->pcdata->learned[sn] == 0 || ch->pcdata->learned[sn] == 100)
        return;

    /* check to see if the character has a chance to learn */
    chance = 10 * char_int_learn_rate (ch);
    chance /= (multiplier * skill_table[sn].rating[ch->class] * 4);
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
            gain_exp (ch, 2 * skill_table[sn].rating[ch->class]);
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
            gain_exp (ch, 2 * skill_table[sn].rating[ch->class]);
        }
    }
}

/* recursively adds a group given its number -- uses group_add */
void gn_add (CHAR_T *ch, int gn) {
    int i;
    ch->pcdata->group_known[gn] = TRUE;
    for (i = 0; i < MAX_IN_GROUP; i++) {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add (ch, group_table[gn].spells[i], FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove (CHAR_T *ch, int gn) {
    int i;
    ch->pcdata->group_known[gn] = FALSE;
    for (i = 0; i < MAX_IN_GROUP; i++) {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_remove (ch, group_table[gn].spells[i]);
    }
}

/* use for processing a skill or group for addition  */
void group_add (CHAR_T *ch, const char *name, bool deduct) {
    int sn, gn;
    if (IS_NPC (ch)) /* NPCs do not have skills */
        return;

    sn = skill_lookup (name);
    if (sn != -1) {
        if (ch->pcdata->learned[sn] == 0) { /* i.e. not known */
            ch->pcdata->learned[sn] = 1;
            if (deduct)
                ch->pcdata->points += skill_table[sn].rating[ch->class];
        }
        return;
    }

    /* now check groups */
    gn = group_lookup (name);
    if (gn != -1) {
        if (ch->pcdata->group_known[gn] == FALSE) {
            ch->pcdata->group_known[gn] = TRUE;
            if (deduct)
                ch->pcdata->points += group_table[gn].rating[ch->class];
        }
        gn_add (ch, gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */
void group_remove (CHAR_T *ch, const char *name) {
    int sn, gn;
    sn = skill_lookup (name);

    if (sn != -1) {
        ch->pcdata->learned[sn] = 0;
        return;
    }

    /* now check groups */
    gn = group_lookup (name);
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE) {
        ch->pcdata->group_known[gn] = FALSE;
        gn_remove (ch, gn); /* be sure to call gn_add on all remaining groups */
    }
}
