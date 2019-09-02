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

#include "db.h"
#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "magic.h"
#include "fight.h"
#include "lookup.h"
#include "skills.h"
#include "recycle.h"
#include "act_comm.h"
#include "chars.h"
#include "find.h"

#include "act_skills.h"

void do_skills_or_spells (CHAR_DATA * ch, char *argument, int spells) {
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[MAX_LEVEL + 1][MAX_STRING_LENGTH];
    char skill_columns[MAX_LEVEL + 1];
    int top_level = UMAX(LEVEL_HERO, ch->level);
    int sn, level, min_lev = 1, max_lev = top_level;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH], *prefix = NULL, *type_str;

    if (IS_NPC(ch))
        return;

    type_str = (spells == TRUE)  ? "spells"
             : (spells == FALSE) ? "skills"
             :                     "abilities";

    /* Parameter options. */
    if (argument[0] == '\0')
        max_lev = URANGE(1, ch->level, top_level);
    else if (!str_cmp (argument, "all"))
        ;
    else if (argument[0] >= '1' && argument[0] <= '9') {
        argument = one_argument (argument, arg);
        BAIL_IF (!is_number (arg),
            "Please specify a valid number for level ranges.\n\r", ch);
        max_lev = atoi (arg);
        if (max_lev < 1 || max_lev > top_level) {
            printf_to_char (ch, "Levels must be between 1 and %d.\n\r",
                top_level);
            return;
        }

        if (argument[0] != '\0') {
            argument = one_argument (argument, arg);
            BAIL_IF (!is_number (arg),
                "Please specify a valid number for level ranges.\n\r", ch);
            min_lev = max_lev;
            max_lev = atoi (arg);

            if (max_lev < 1 || max_lev > top_level) {
                printf_to_char (ch, "Levels must be between 1 and %d.\n\r",
                    top_level);
                return;
            }
            BAIL_IF (min_lev > max_lev,
                "That would be silly.\n\r", ch);
        }
    }
    else
        prefix = argument;

    /* Show the skill/spell/abilities we're looking for. */
    if (min_lev == 1 && max_lev == top_level)
        printf_to_char (ch, "Showing all %s", type_str);
    else if (min_lev == max_lev)
        printf_to_char (ch, "Showing %s for level %d", type_str,
            min_lev);
    else {
        printf_to_char (ch, "Showing %s between levels %d and %d",
            type_str, min_lev, max_lev);
    }
    if (prefix)
        printf_to_char (ch, " that begin with '%s'", prefix);
    send_to_char (":\n\r", ch);

    /* initialize data */
    for (level = min_lev; level <= max_lev; level++) {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }

    for (sn = 0; sn < SKILL_MAX; sn++) {
        int is_spell;
        if (skill_table[sn].name == NULL)
            break;

        level = skill_table[sn].skill_level[ch->class];
        is_spell = skill_table[sn].spell_fun != spell_null;
        if ((spells == FALSE && is_spell) || (spells == TRUE && !is_spell))
            continue;
        if (level < min_lev || level > max_lev)
            continue;
        if (ch->pcdata->learned[sn] < 1 && !IS_IMMORTAL(ch))
            continue;
        if (prefix != NULL && str_prefix(prefix, skill_table[sn].name))
            continue;

        if (skill_list[level][0] == '\0')
            sprintf (skill_list[level], "Level %2d: ", level);
        else if (++skill_columns[level] % 2 == 0)
            strcat (skill_list[level], "\n\r          ");

        found = TRUE;
        if (ch->level < level)
            sprintf (buf, "%-18s n/a        ", skill_table[sn].name);
        else if (!is_spell) {
            sprintf (buf, "%-18s %3d%%       ", skill_table[sn].name,
                     ch->pcdata->learned[sn]);
        }
        else if (is_spell) {
            int mana = UMAX (skill_table[sn].min_mana,
                         100 / (2 + ch->level - level));
            sprintf (buf, "%-18s %3d mana   ", skill_table[sn].name,
                     mana);
        }
        strcat (skill_list[level], buf);
    }

    /* return results */
    if (!found) {
        printf_to_char (ch, "No %s found.\n\r", type_str);
        return;
    }

    buffer = buf_new ();
    for (level = min_lev; level <= max_lev; level++) {
        if (skill_list[level][0] != '\0') {
            add_buf (buffer, skill_list[level]);
            add_buf (buffer, "\n\r");
        }
    }
    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}

/* used to get new skills */
void do_gain (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC (ch))
        return;
    BAIL_IF ((trainer = char_get_trainer_room (ch)) == NULL,
        "You can't do that here.\n\r", ch);
    BAIL_IF (!char_can_see_in_room (ch, trainer),
        "You can't do that here.\n\r", ch);

    one_argument (argument, arg);
    BAIL_IF_ACT (arg[0] == '\0',
        "$N tells you 'Pardon me?'", ch, NULL, trainer);

    if (!str_prefix (arg, "list")) {
        int col;
        col = 0;
        printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
                 "group", "cost", "group", "cost", "group", "cost");

        for (gn = 0; gn < GROUP_MAX && group_table[gn].name; gn++) {
            if (ch->pcdata->group_known[gn])
                continue;
            if (group_table[gn].rating[ch->class] <= 0)
                continue;

            printf_to_char (ch, "%-18s %-5d ", group_table[gn].name,
                group_table[gn].rating[ch->class]);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 3 != 0)
            send_to_char ("\n\r", ch);

        send_to_char ("\n\r", ch);
        col = 0;

        printf_to_char (ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
                 "skill", "cost", "skill", "cost", "skill", "cost");

        for (sn = 0; sn < SKILL_MAX && skill_table[sn].name; sn++) {
            if (ch->pcdata->learned[sn])
                continue;
            if (skill_table[sn].rating[ch->class] <= 0)
                continue;
            if (skill_table[sn].spell_fun != spell_null)
                continue;

            printf_to_char (ch, "%-18s %-5d ", skill_table[sn].name,
                skill_table[sn].rating[ch->class]);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 3 != 0)
            send_to_char ("\n\r", ch);
        return;
    }

    if (!str_prefix (arg, "convert")) {
        BAIL_IF_ACT (ch->practice < 10,
            "$N tells you 'You are not yet ready.'", ch, NULL, trainer);

        act ("$N helps you apply your practice to training.",
             ch, NULL, trainer, TO_CHAR);
        ch->practice -= 10;
        ch->train += 1;
        return;
    }

    if (!str_prefix (arg, "points")) {
        BAIL_IF_ACT (ch->train < 2,
            "$N tells you 'You are not yet ready.'", ch, NULL, trainer);
        BAIL_IF_ACT (ch->pcdata->points <= 40,
            "$N tells you 'There would be no point in that.'", ch, NULL, trainer);

        act ("$N trains you, and you feel more at ease with your skills.",
             ch, NULL, trainer, TO_CHAR);
        ch->train -= 2;
        ch->pcdata->points -= 1;
        ch->exp = exp_per_level (ch, ch->pcdata->points) * ch->level;
        return;
    }

    /* else add a group/skill */
    gn = group_lookup (argument);
    if (gn > 0) {
        BAIL_IF_ACT (ch->pcdata->group_known[gn],
            "$N tells you 'You already know that group!'", ch, NULL, trainer);
        BAIL_IF_ACT (group_table[gn].rating[ch->class] <= 0,
            "$N tells you 'That group is beyond your powers.'", ch, NULL, trainer);
        BAIL_IF_ACT (ch->train < group_table[gn].rating[ch->class],
            "$N tells you 'You are not yet ready for that group.'", ch, NULL, trainer);

        /* add the group */
        gn_add (ch, gn);
        act ("$N trains you in the art of $t.",
             ch, group_table[gn].name, trainer, TO_CHAR);
        ch->train -= group_table[gn].rating[ch->class];
        return;
    }

    sn = skill_lookup (argument);
    if (sn > -1) {
        BAIL_IF_ACT (skill_table[sn].spell_fun != spell_null,
            "$N tells you 'You must learn the full group.'", ch, NULL, trainer);
        BAIL_IF_ACT (ch->pcdata->learned[sn],
            "$N tells you 'You already know that skill!'", ch, NULL, trainer);
        BAIL_IF_ACT (skill_table[sn].rating[ch->class] <= 0,
            "$N tells you 'That skill is beyond your powers.'", ch, NULL, trainer);
        BAIL_IF_ACT (ch->train < skill_table[sn].rating[ch->class],
            "$N tells you 'You are not yet ready for that skill.'", ch, NULL, trainer);

        /* add the skill */
        ch->pcdata->learned[sn] = 1;
        act ("$N trains you in the art of $t.",
             ch, skill_table[sn].name, trainer, TO_CHAR);
        ch->train -= skill_table[sn].rating[ch->class];
        return;
    }

    act ("$N tells you 'I do not understand...'", ch, NULL, trainer, TO_CHAR);
}

void do_skills (CHAR_DATA * ch, char *argument)
    { do_skills_or_spells (ch, argument, FALSE); }
void do_spells (CHAR_DATA * ch, char *argument)
    { do_skills_or_spells (ch, argument, TRUE); }
void do_abilities (CHAR_DATA * ch, char *argument)
    { do_skills_or_spells (ch, argument, -1); }

/* shows all groups, or the sub-members of a group */
void do_groups (CHAR_DATA * ch, char *argument) {
    int gn, sn, col;
    if (IS_NPC (ch))
        return;

    /* show all groups */
    if (argument[0] == '\0') {
        col = 0;
        for (gn = 0; gn < GROUP_MAX && group_table[gn].name != NULL; gn++) {
            if (!ch->pcdata->group_known[gn])
                continue;
            printf_to_char (ch, "%-20s ", group_table[gn].name);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 3 != 0)
            send_to_char ("\n\r", ch);
        printf_to_char (ch, "Creation points: %d\n\r", ch->pcdata->points);
        return;
    }

    /* show all groups */
    if (!str_cmp (argument, "all")) {
        col = 0;
        for (gn = 0; gn < GROUP_MAX && group_table[gn].name != NULL; gn++) {
            printf_to_char (ch, "%-20s ", group_table[gn].name);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }
        if (col % 3 != 0)
            send_to_char ("\n\r", ch);
        return;
    }

    /* show the sub-members of a group */
    gn = group_lookup (argument);
    if (gn == -1) {
        send_to_char (
            "No group of that name exist.\n\r"
            "Type 'groups all' or 'info all' for a full listing.\n\r", ch);
        return;
    }

    col = 0;
    for (sn = 0; sn < MAX_IN_GROUP && group_table[gn].spells[gn] != NULL; sn++) {
        printf_to_char (ch, "%-20s ", group_table[gn].spells[sn]);
        if (++col % 3 == 0)
            send_to_char ("\n\r", ch);
    }
    if (col % 3 != 0)
        send_to_char ("\n\r", ch);
}

void do_train (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = -1;
    char *pOutput = NULL;
    int cost;

    if (IS_NPC (ch))
        return;

    /* Check for trainer. */
    BAIL_IF ((mob = char_get_trainer_room (ch)) == NULL,
        "You can't do that here.\n\r", ch);
    if (argument[0] == '\0') {
        sprintf (buf, "You have %d training sessions.\n\r", ch->train);
        send_to_char (buf, ch);
        argument = "foo";
    }

    cost = 1;
    if (!str_cmp (argument, "str")) {
        if (class_table[ch->class].attr_prime == STAT_STR)
            cost = 1;
        stat = STAT_STR;
        pOutput = "strength";
    }
    else if (!str_cmp (argument, "int")) {
        if (class_table[ch->class].attr_prime == STAT_INT)
            cost = 1;
        stat = STAT_INT;
        pOutput = "intelligence";
    }
    else if (!str_cmp (argument, "wis")) {
        if (class_table[ch->class].attr_prime == STAT_WIS)
            cost = 1;
        stat = STAT_WIS;
        pOutput = "wisdom";
    }
    else if (!str_cmp (argument, "dex")) {
        if (class_table[ch->class].attr_prime == STAT_DEX)
            cost = 1;
        stat = STAT_DEX;
        pOutput = "dexterity";
    }
    else if (!str_cmp (argument, "con")) {
        if (class_table[ch->class].attr_prime == STAT_CON)
            cost = 1;
        stat = STAT_CON;
        pOutput = "constitution";
    }
    else if (!str_cmp (argument, "hp"))
        cost = 1;
    else if (!str_cmp (argument, "mana"))
        cost = 1;
    else {
        strcpy (buf, "You can train:");
        if (ch->perm_stat[STAT_STR] < char_get_max_train (ch, STAT_STR))
            strcat (buf, " str");
        if (ch->perm_stat[STAT_INT] < char_get_max_train (ch, STAT_INT))
            strcat (buf, " int");
        if (ch->perm_stat[STAT_WIS] < char_get_max_train (ch, STAT_WIS))
            strcat (buf, " wis");
        if (ch->perm_stat[STAT_DEX] < char_get_max_train (ch, STAT_DEX))
            strcat (buf, " dex");
        if (ch->perm_stat[STAT_CON] < char_get_max_train (ch, STAT_CON))
            strcat (buf, " con");
        strcat (buf, " hp mana");

        if (buf[strlen (buf) - 1] != ':') {
            strcat (buf, ".\n\r");
            send_to_char (buf, ch);
        }
        else {
            /* This message dedicated to Jordan ... you big stud! */
            act ("You have nothing left to train, you $T!",
                 ch, NULL,
                 (ch->sex == SEX_MALE) ? "big stud" :
                 (ch->sex == SEX_FEMALE) ? "hot babe" : "wild thing", TO_CHAR);
        }
        return;
    }

    BAIL_IF (cost > ch->train,
        "You don't have enough training sessions.\n\r", ch);

    if (!str_cmp ("hp", argument)) {
        ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit += 10;
        act ("Your durability increases!", ch, NULL, NULL, TO_CHAR);
        act ("$n's durability increases!", ch, NULL, NULL, TO_NOTCHAR);
        return;
    }
    if (!str_cmp ("mana", argument)) {
        ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
        act ("Your power increases!", ch, NULL, NULL, TO_CHAR);
        act ("$n's power increases!", ch, NULL, NULL, TO_NOTCHAR);
        return;
    }

    if (ch->perm_stat[stat] >= char_get_max_train (ch, stat)) {
        act ("Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR);
        return;
    }

    ch->train -= cost;
    ch->perm_stat[stat] += 1;
    act ("Your $T increases!", ch, NULL, pOutput, TO_CHAR);
    act ("$n's $T increases!", ch, NULL, pOutput, TO_NOTCHAR);
}

void do_practice (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    int sn, level, col, rating;
    CHAR_DATA *mob;
    int adept, top_level = UMAX(LEVEL_HERO, ch->level);

    if (IS_NPC (ch))
        return;

    if (argument[0] == '\0') {
        col = 0;
        for (sn = 0; sn < SKILL_MAX; sn++) {
            if (skill_table[sn].name == NULL)
                break;

            level = skill_table[sn].skill_level[ch->class];
            if (level < 1 || level > top_level)
                continue;
            if (!IS_IMMORTAL(ch) && ch->level < skill_table[sn].skill_level[ch->class])
                continue;
            if (!IS_IMMORTAL(ch) && ch->pcdata->learned[sn] < 1)
                continue;

            sprintf (buf, "%-18s %3d%%  ",
                     skill_table[sn].name, ch->pcdata->learned[sn]);
            send_to_char (buf, ch);
            if (++col % 3 == 0)
                send_to_char ("\n\r", ch);
        }

        if (col % 3 != 0)
            send_to_char ("\n\r", ch);

        sprintf (buf, "You have %d practice sessions left.\n\r",
                 ch->practice);
        send_to_char (buf, ch);
        return;
    }

    BAIL_IF (!IS_AWAKE (ch),
        "In your dreams, or what?\n\r", ch);
    BAIL_IF ((mob = char_get_practicer_room (ch)) == NULL,
        "You can't do that here.\n\r", ch);

    sn = find_spell (ch, argument);
    BAIL_IF (sn < 0 || sn >= SKILL_MAX,
        "Practice what now?\n\r", ch);

    level = skill_table[sn].skill_level[ch->class];
    BAIL_IF (level < 1 || level > top_level,
        "Practice what now?\n\r", ch);

    if (!IS_IMMORTAL (ch)) {
        BAIL_IF ((ch->pcdata->learned[sn] < 1 ||
                skill_table[sn].rating[ch->class] == 0),
            "Practice what now?\n\r", ch);
        BAIL_IF (ch->level < skill_table[sn].skill_level[ch->class],
            "You can't practice that yet.\n\r", ch);
    }

    adept = IS_NPC (ch) ? 100 : class_table[ch->class].skill_adept;
    if (ch->pcdata->learned[sn] >= adept) {
        printf_to_char (ch, "You are already learned at %s.\n\r",
            skill_table[sn].name);
        return;
    }

    BAIL_IF (ch->practice <= 0,
        "You have no practice sessions left.\n\r", ch);

    ch->practice--;
    rating = skill_table[sn].rating[ch->class];
    ch->pcdata->learned[sn] +=
        int_app[char_get_curr_stat (ch, STAT_INT)].learn /
        (rating < 1 ? 1 : rating);
    if (ch->pcdata->learned[sn] < adept) {
        act2 ("You practice $T.", "$n practices $T.",
            ch, NULL, skill_table[sn].name, 0, POS_RESTING);
    }
    else {
        ch->pcdata->learned[sn] = adept;
        act2 ("You are now learned at $T.", "$n is now learned at $T.",
            ch, NULL, skill_table[sn].name, 0, POS_RESTING);
    }
}

void do_cast (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    /* Switched NPC's can cast spells, but others can't. */
    if (IS_NPC (ch) && ch->desc == NULL)
        return;

    target_name = one_argument (argument, arg1);
    if (arg1[0] == '\0') {
        send_to_char ("Cast which what where?\n\r", ch);
        return;
    }
    one_argument (target_name, arg2);

    BAIL_IF ((sn = find_spell (ch, arg1)) < 1,
        "You don't know any spells of that name.\n\r", ch);
    BAIL_IF (skill_table[sn].spell_fun == spell_null,
        "You don't know any spells of that name.\n\r", ch);
    BAIL_IF (!IS_NPC (ch) && !IS_IMMORTAL(ch) && (
            ch->level < skill_table[sn].skill_level[ch->class] ||
            ch-> pcdata->learned[sn] == 0),
        "You don't know any spells of that name.\n\r", ch);

    BAIL_IF (ch->position < skill_table[sn].minimum_position,
        "You can't concentrate enough.\n\r", ch);

    if (IS_NPC (ch))
        mana = 25;
    else if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
        mana = 50;
    else
        mana = UMAX (skill_table[sn].min_mana, 100 / (2 + ch->level -
                        skill_table[sn].skill_level[ch->class]));

    /* Locate targets. */
    victim = NULL;
    obj = NULL;
    vo = NULL;
    target = TARGET_NONE;

    switch (skill_table[sn].target) {
        default:
            bug ("do_cast: bad target for sn %d.", sn);
            return;

        case TAR_IGNORE:
            break;

        case TAR_CHAR_OFFENSIVE:
            if (arg2[0] == '\0') {
                BAIL_IF ((victim = ch->fighting) == NULL,
                    "Cast the spell on whom?\n\r", ch);
            }
            else {
                BAIL_IF ((victim = find_char_same_room (ch, target_name)) == NULL,
                    "They aren't here.\n\r", ch);
            }
            if (!IS_NPC (ch)) {
                BAIL_IF (is_safe (ch, victim) && victim != ch,
                    "Not on that target.\n\r", ch);
                check_killer (ch, victim);
            }
            BAIL_IF (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
                "You can't do that on your own follower.\n\r", ch);
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_DEFENSIVE:
            if (arg2[0] == '\0')
                victim = ch;
            else {
                BAIL_IF ((victim = find_char_same_room (ch, target_name)) == NULL,
                    "They aren't here.\n\r", ch);
            }
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_SELF:
            BAIL_IF (arg2[0] != '\0' && !is_name (target_name, ch->name),
                "You cannot cast this spell on another.\n\r", ch);
            vo = (void *) ch;
            target = TARGET_CHAR;
            break;

        case TAR_OBJ_INV:
            BAIL_IF (arg2[0] == '\0',
                "What should the spell be cast upon?\n\r", ch);
            BAIL_IF ((obj = find_obj_own_inventory (ch, target_name)) == NULL,
                "You are not carrying that.\n\r", ch);
            vo = (void *) obj;
            target = TARGET_OBJ;
            break;

        case TAR_OBJ_CHAR_OFF:
            if (arg2[0] == '\0') {
                BAIL_IF ((victim = ch->fighting) == NULL,
                    "Cast the spell on whom or what?\n\r", ch);
                target = TARGET_CHAR;
            }
            else if ((victim = find_char_same_room (ch, target_name)) != NULL)
                target = TARGET_CHAR;

            /* check the sanity of the attack */
            if (target == TARGET_CHAR) {
                BAIL_IF (is_safe_spell (ch, victim, FALSE) && victim != ch,
                    "Not on that target.\n\r", ch);
                BAIL_IF (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
                    "You can't do that on your own follower.\n\r", ch);
                if (!IS_NPC (ch))
                    check_killer (ch, victim);
                vo = (void *) victim;
            }
            else if ((obj = find_obj_here (ch, target_name)) != NULL) {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else {
                send_to_char ("You don't see that here.\n\r", ch);
                return;
            }
            break;

        case TAR_OBJ_CHAR_DEF:
            if (arg2[0] == '\0') {
                victim = ch;
                vo = (void *) ch;
                target = TARGET_CHAR;
            }
            else if ((victim = find_char_same_room (ch, target_name)) != NULL) {
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else if ((obj = find_obj_own_inventory (ch, target_name)) != NULL) {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            else {
                send_to_char ("You don't see that here.\n\r", ch);
                return;
            }
            break;
    }

    BAIL_IF (!IS_NPC (ch) && ch->mana < mana,
        "You don't have enough mana.\n\r", ch);

    if (str_cmp (skill_table[sn].name, "ventriloquate"))
        say_spell (ch, sn, IS_NPC (ch) ? CLASS_MAGE : ch->class);

    WAIT_STATE (ch, skill_table[sn].beats);

    if (number_percent () > get_skill (ch, sn)) {
        send_to_char ("You lost your concentration.\n\r", ch);
        check_improve (ch, sn, FALSE, 1);
        ch->mana -= mana / 2;
    }
    else {
        ch->mana -= mana;
        if (IS_NPC (ch) || class_table[ch->class].fMana)
            /* class has spells */
            (*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
        else
            (*skill_table[sn].spell_fun) (sn, ch->level * 3 / 4, ch, vo, target);
        check_improve (ch, sn, TRUE, 1);
    }

    spell_fight_back_if_possible (ch, victim, sn, target);
}
