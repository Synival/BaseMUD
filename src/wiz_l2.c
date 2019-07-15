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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "ban.h"
#include "utils.h"
#include "recycle.h"
#include "lookup.h"
#include "chars.h"
#include "rooms.h"
#include "find.h"

#include "wiz_l2.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */

void do_allow (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *pban, *prev, *pban_next;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Remove which site from the ban list?\n\r", ch);
        return;
    }

    prev = NULL;
    for (pban = ban_first; pban; prev = pban, pban = pban_next) {
        pban_next = pban->next;
        if (str_cmp (arg, pban->name))
            continue;
        if (pban->level > char_get_trust (ch)) {
            send_to_char ("You are not powerful enough to lift that ban.\n\r", ch);
            return;
        }

        LISTB_REMOVE_WITH_PREV (pban, prev, next, ban_first, ban_last);
        ban_free (pban);

        sprintf (buf, "Ban on %s lifted.\n\r", arg);
        send_to_char (buf, ch);
        save_bans ();
        return;
    }

    send_to_char ("Site is not banned.\n\r", ch);
}

void do_ban (CHAR_DATA * ch, char *argument) {
    ban_site (ch, argument, FALSE);
}

/* RT set replaces sset, mset, oset, and rset */
void do_set (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);

    if (arg[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  set mob       <name> <field> <value>\n\r", ch);
        send_to_char ("  set character <name> <field> <value>\n\r", ch);
        send_to_char ("  set obj       <name> <field> <value>\n\r", ch);
        send_to_char ("  set room      <room> <field> <value>\n\r", ch);
        send_to_char ("  set skill     <name> <spell or skill> <value>\n\r", ch);
        return;
    }
    if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character")) {
        do_function (ch, &do_mset, argument);
        return;
    }
    if (!str_prefix (arg, "skill") || !str_prefix (arg, "spell")) {
        do_function (ch, &do_sset, argument);
        return;
    }
    if (!str_prefix (arg, "object")) {
        do_function (ch, &do_oset, argument);
        return;
    }
    if (!str_prefix (arg, "room")) {
        do_function (ch, &do_rset, argument);
        return;
    }
    /* echo syntax */
    do_function (ch, &do_set, "");
}

void do_sset (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  set skill <name> <spell or skill> <value>\n\r", ch);
        send_to_char ("  set skill <name> all <value>\n\r", ch);
        send_to_char ("   (use the name of the skill, not the number)\n\r", ch);
        return;
    }

    if ((victim = find_char_world (ch, arg1)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }
    if (IS_NPC (victim)) {
        send_to_char ("Not on NPC's.\n\r", ch);
        return;
    }

    fAll = !str_cmp (arg2, "all");
    sn = 0;
    if (!fAll && (sn = skill_lookup (arg2)) < 0) {
        send_to_char ("No such skill or spell.\n\r", ch);
        return;
    }

    /* Snarf the value. */
    if (!is_number (arg3)) {
        send_to_char ("Value must be numeric.\n\r", ch);
        return;
    }

    value = atoi (arg3);
    if (value < 0 || value > 100) {
        send_to_char ("Value range is 0 to 100.\n\r", ch);
        return;
    }

    if (fAll) {
        for (sn = 0; sn < SKILL_MAX; sn++)
            if (skill_table[sn].name != NULL)
                victim->pcdata->learned[sn] = value;
    }
    else
        victim->pcdata->learned[sn] = value;
}

void do_mset (CHAR_DATA * ch, char *argument) {
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  set char <name> <field> <value>\n\r", ch);
        send_to_char ("  Field being one of:\n\r", ch);
        send_to_char ("    str int wis dex con sex class level\n\r", ch);
        send_to_char ("    race group gold silver hp mana move prac\n\r", ch);
        send_to_char ("    align train thirst hunger drunk full\n\r", ch);
        send_to_char ("    security hours\n\r", ch);
        return;
    }

    if ((victim = find_char_world (ch, arg1)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /* Snarf the value (which need not be numeric). */
    value = is_number (arg3) ? atoi (arg3) : -1;

    /* Set something. */
    if (!str_cmp (arg2, "str")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_STR)) {
            sprintf (buf,
                     "Strength range is 3 to %d\n\r.",
                     char_get_max_train (victim, STAT_STR));
            send_to_char (buf, ch);
            return;
        }
        victim->perm_stat[STAT_STR] = value;
        return;
    }

    if (!str_cmp (arg2, "security")) { /* OLC */
        if (IS_NPC (ch)) {
            send_to_char ("NPC's can't set this value.\n\r", ch);
            return;
        }
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (value > ch->pcdata->security || value < 0) {
            if (ch->pcdata->security != 0)
                printf_to_char (ch, "Valid security is 0-%d.\n\r", ch->pcdata->security);
            else
                send_to_char ("Valid security is 0 only.\n\r", ch);
            return;
        }
        victim->pcdata->security = value;
        return;
    }

    if (!str_cmp (arg2, "int")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_INT)) {
            printf_to_char (ch, "Intelligence range is 3 to %d.\n\r",
                            char_get_max_train (victim, STAT_INT));
            return;
        }
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if (!str_cmp (arg2, "wis")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_WIS)) {
            printf_to_char (ch, "Wisdom range is 3 to %d.\n\r",
                            char_get_max_train (victim, STAT_WIS));
            return;
        }
        victim->perm_stat[STAT_WIS] = value;
        return;
    }

    if (!str_cmp (arg2, "dex")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_DEX)) {
            printf_to_char (ch, "Dexterity range is 3 to %d.\n\r",
                            char_get_max_train (victim, STAT_DEX));
            return;
        }
        victim->perm_stat[STAT_DEX] = value;
        return;
    }

    if (!str_cmp (arg2, "con")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_CON)) {
            printf_to_char (ch, "Constitution range is 3 to %d.\n\r",
                            char_get_max_train (victim, STAT_CON));
            return;
        }
        victim->perm_stat[STAT_CON] = value;
        return;
    }

    if (!str_prefix (arg2, "sex")) {
        if (value < 0 || value > 2) {
            send_to_char ("Sex range is 0 to 2.\n\r", ch);
            return;
        }
        victim->sex = value;
        if (!IS_NPC (victim))
            victim->pcdata->true_sex = value;
        return;
    }

    if (!str_prefix (arg2, "class")) {
        int class;
        if (IS_NPC (victim)) {
            send_to_char ("Mobiles have no class.\n\r", ch);
            return;
        }

        class = class_lookup (arg3);
        if (class == -1) {
            char buf[MAX_STRING_LENGTH];

            strcpy (buf, "Possible classes are: ");
            for (class = 0; class < CLASS_MAX; class++) {
                if (class > 0)
                    strcat (buf, " ");
                strcat (buf, class_table[class].name);
            }
            strcat (buf, ".\n\r");

            send_to_char (buf, ch);
            return;
        }

        victim->class = class;
        return;
    }

    if (!str_prefix (arg2, "level")) {
        if (!IS_NPC (victim)) {
            send_to_char ("Not on PC's.\n\r", ch);
            return;
        }
        if (value < 0 || value > MAX_LEVEL) {
            sprintf (buf, "Level range is 0 to %d.\n\r", MAX_LEVEL);
            send_to_char (buf, ch);
            return;
        }
        victim->level = value;
        return;
    }

    if (!str_prefix (arg2, "gold")) {
        victim->gold = value;
        return;
    }
    if (!str_prefix (arg2, "silver")) {
        victim->silver = value;
        return;
    }
    if (!str_prefix (arg2, "hp")) {
        if (value < -10 || value > 30000) {
            send_to_char ("Hp range is -10 to 30,000 hit points.\n\r", ch);
            return;
        }
        victim->max_hit = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_hit = value;
        return;
    }

    if (!str_prefix (arg2, "mana")) {
        if (value < 0 || value > 30000) {
            send_to_char ("Mana range is 0 to 30,000 mana points.\n\r", ch);
            return;
        }
        victim->max_mana = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_mana = value;
        return;
    }

    if (!str_prefix (arg2, "move")) {
        if (value < 0 || value > 30000) {
            send_to_char ("Move range is 0 to 30,000 move points.\n\r", ch);
            return;
        }
        victim->max_move = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_move = value;
        return;
    }

    if (!str_prefix (arg2, "practice")) {
        if (value < 0 || value > 250) {
            send_to_char ("Practice range is 0 to 250 sessions.\n\r", ch);
            return;
        }
        victim->practice = value;
        return;
    }

    if (!str_prefix (arg2, "train")) {
        if (value < 0 || value > 50) {
            send_to_char ("Training session range is 0 to 50 sessions.\n\r", ch);
            return;
        }
        victim->train = value;
        return;
    }

    if (!str_prefix (arg2, "align")) {
        if (value < -1000 || value > 1000) {
            send_to_char ("Alignment range is -1000 to 1000.\n\r", ch);
            return;
        }
        victim->alignment = value;
        return;
    }

    if (!str_prefix (arg2, "thirst")) {
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (value < -1 || value > 100) {
            send_to_char ("Thirst range is -1 to 100.\n\r", ch);
            return;
        }
        victim->pcdata->condition[COND_THIRST] = value;
        return;
    }

    if (!str_prefix (arg2, "drunk")) {
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (value < -1 || value > 100) {
            send_to_char ("Drunk range is -1 to 100.\n\r", ch);
            return;
        }
        victim->pcdata->condition[COND_DRUNK] = value;
        return;
    }

    if (!str_prefix (arg2, "full")) {
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (value < -1 || value > 100) {
            send_to_char ("Full range is -1 to 100.\n\r", ch);
            return;
        }
        victim->pcdata->condition[COND_FULL] = value;
        return;
    }

    if (!str_prefix (arg2, "hunger")) {
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (value < -1 || value > 100) {
            send_to_char ("Full range is -1 to 100.\n\r", ch);
            return;
        }
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix (arg2, "race")) {
        int race = race_lookup (arg3);
        if (race <= 0) {
            send_to_char ("That is not a valid race.\n\r", ch);
            return;
        }
        if (!IS_NPC (victim) && !race_table[race].pc_race) {
            send_to_char ("That is not a valid player race.\n\r", ch);
            return;
        }
        victim->race = race;
        return;
    }

    if (!str_prefix (arg2, "group")) {
        if (!IS_NPC (victim)) {
            send_to_char ("Only on NPCs.\n\r", ch);
            return;
        }
        victim->group = value;
        return;
    }

    if (!str_prefix (arg2, "hours")) {
        if (IS_NPC (victim)) {
            send_to_char ("Not on NPC's.\n\r", ch);
            return;
        }
        if (!is_number (arg3)) {
            send_to_char ("Value must be numeric.\n\r", ch);
            return;
        }

        value = atoi (arg3);
        if (value < 0 || value > 999) {
            send_to_char ("Value must be between 0 and 999.\n\r", ch);
            return;
        }

        victim->played = ( value * 3600 );
        printf_to_char(ch, "%s's hours set to %d.", victim->name, value);
        return;
    }

    /* Generate usage message. */
    do_function (ch, &do_mset, "");
}

void do_oset (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  set obj <object> <field> <value>\n\r", ch);
        send_to_char ("  Field being one of:\n\r", ch);
        send_to_char ("    value0 value1 value2 value3 value4 (v1-v4)\n\r", ch);
        send_to_char ("    extra wear level weight cost timer\n\r", ch);
        return;
    }
    if ((obj = find_obj_world (ch, arg1)) == NULL) {
        send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
        return;
    }

    /* Snarf the value (which need not be numeric). */
    value = atoi (arg3);

    /* Set something. */
    if (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0")) {
        obj->value[0] = UMIN (50, value);
        return;
    }
    if (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1")) {
        obj->value[1] = value;
        return;
    }
    if (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2")) {
        obj->value[2] = value;
        return;
    }
    if (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3")) {
        obj->value[3] = value;
        return;
    }
    if (!str_cmp (arg2, "value4") || !str_cmp (arg2, "v4")) {
        obj->value[4] = value;
        return;
    }
    if (!str_prefix (arg2, "extra")) {
        obj->extra_flags = value;
        return;
    }
    if (!str_prefix (arg2, "wear")) {
        obj->wear_flags = value;
        return;
    }
    if (!str_prefix (arg2, "level")) {
        obj->level = value;
        return;
    }
    if (!str_prefix (arg2, "weight")) {
        obj->weight = value;
        return;
    }
    if (!str_prefix (arg2, "cost")) {
        obj->cost = value;
        return;
    }
    if (!str_prefix (arg2, "timer")) {
        obj->timer = value;
        return;
    }

    /* Generate usage message. */
    do_function (ch, &do_oset, "");
}

void do_rset (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde (argument);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  set room <location> <field> <value>\n\r", ch);
        send_to_char ("  Field being one of:\n\r", ch);
        send_to_char ("    flags sector\n\r", ch);
        return;
    }
    if ((location = find_location (ch, arg1)) == NULL) {
        send_to_char ("No such location.\n\r", ch);
        return;
    }
    if (!room_is_owner (location, ch) && ch->in_room != location
        && room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        send_to_char ("That room is private right now.\n\r", ch);
        return;
    }

    /* Snarf the value. */
    if (!is_number (arg3)) {
        send_to_char ("Value must be numeric.\n\r", ch);
        return;
    }
    value = atoi (arg3);

    /* Set something. */
    if (!str_prefix (arg2, "flags")) {
        location->room_flags = value;
        return;
    }
    if (!str_prefix (arg2, "sector")) {
        location->sector_type = value;
        return;
    }

    /* Generate usage message. */
    do_function (ch, &do_rset, "");
}

void do_wizlock (CHAR_DATA * ch, char *argument) {
    extern bool wizlock;
    wizlock = !wizlock;

    if (wizlock) {
        wiznet ("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
        send_to_char ("Game wizlocked.\n\r", ch);
    }
    else {
        wiznet ("$N removes wizlock.", ch, NULL, 0, 0, 0);
        send_to_char ("Game un-wizlocked.\n\r", ch);
    }
}
