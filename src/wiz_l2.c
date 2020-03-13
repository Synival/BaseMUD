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
#include "comm.h"
#include "interp.h"
#include "ban.h"
#include "utils.h"
#include "recycle.h"
#include "lookup.h"
#include "chars.h"
#include "rooms.h"
#include "find.h"
#include "globals.h"

#include "wiz_l2.h"

DEFINE_DO_FUN (do_allow) {
    char arg[MAX_INPUT_LENGTH];
    BAN_T *pban, *pban_next;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Remove which site from the ban list?\n\r", ch);

    for (pban = ban_first; pban; pban = pban_next) {
        pban_next = pban->global_next;
        if (str_cmp (arg, pban->name))
            continue;
        BAIL_IF (pban->level > char_get_trust (ch),
            "You are not powerful enough to lift that ban.\n\r", ch);
        ban_free (pban);

        printf_to_char (ch, "Ban on %s lifted.\n\r", arg);
        ban_save_all ();
        return;
    }

    send_to_char ("Site is not banned.\n\r", ch);
}

DEFINE_DO_FUN (do_ban) {
    ban_site (ch, argument, FALSE);
}

/* RT set replaces sset, mset, oset, and rset */
DEFINE_DO_FUN (do_set) {
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

    BAIL_IF_EXPR (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"),
        do_function (ch, &do_mset, argument));
    BAIL_IF_EXPR (!str_prefix (arg, "skill") || !str_prefix (arg, "spell"),
        do_function (ch, &do_sset, argument));
    BAIL_IF_EXPR (!str_prefix (arg, "object"),
        do_function (ch, &do_oset, argument));
    BAIL_IF_EXPR (!str_prefix (arg, "room"),
        do_function (ch, &do_rset, argument));

    /* echo syntax */
    do_function (ch, &do_set, "");
}

DEFINE_DO_FUN (do_sset) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    int value;
    int sn;
    bool all;

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

    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);

    all = !str_cmp (arg2, "all");
    sn = 0;
    BAIL_IF (!all && (sn = skill_lookup (arg2)) < 0,
        "No such skill or spell.\n\r", ch);

    /* Snarf the value. */
    BAIL_IF (!is_number (arg3),
        "Value must be numeric.\n\r", ch);

    value = atoi (arg3);
    BAIL_IF (value < 0 || value > 100,
        "Value range is 0 to 100.\n\r", ch);

    if (all) {
        for (sn = 0; sn < SKILL_MAX; sn++)
            if (skill_table[sn].name != NULL)
                victim->pcdata->learned[sn] = value;
    }
    else
        victim->pcdata->learned[sn] = value;
}

DEFINE_DO_FUN (do_mset) {
    const CLASS_T *class;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    CHAR_T *victim;
    int value;
    int class_n, i;

    str_smash_tilde (argument);
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

    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "They aren't here.\n\r", ch);

    /* clear zones for mobs */
    victim->area = NULL;

    /* Snarf the value (which need not be numeric). */
    value = is_number (arg3) ? atoi (arg3) : -1;

    /* Set something. */
    if (!str_cmp (arg2, "str")) {
        if (value < 3 || value > char_get_max_train (victim, STAT_STR)) {
            printf_to_char (ch, "Strength range is 3 to %d\n\r.",
                char_get_max_train (victim, STAT_STR));
            return;
        }
        victim->perm_stat[STAT_STR] = value;
        return;
    }

    if (!str_cmp (arg2, "security")) { /* OLC */
        BAIL_IF (IS_NPC (ch),
            "NPC's can't set this value.\n\r", ch);
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
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
        BAIL_IF (value < 0 || value > 2,
            "Sex range is 0 to 2.\n\r", ch);
        victim->sex = value;
        if (!IS_NPC (victim))
            victim->pcdata->true_sex = value;
        return;
    }

    if (!str_prefix (arg2, "class")) {
        BAIL_IF (IS_NPC (victim),
            "Mobiles have no class.\n\r", ch);

        class_n = class_lookup (arg3);
        if (class < 0) {
            char buf[MAX_STRING_LENGTH];

            strcpy (buf, "Possible classes are: ");
            for (i = 0; (class = class_get (i)) != NULL; i++) {
                if (i > 0)
                    strcat (buf, " ");
                strcat (buf, class->name);
            }
            strcat (buf, ".\n\r");

            send_to_char (buf, ch);
            return;
        }

        victim->class = class_n;
        return;
    }

    if (!str_prefix (arg2, "level")) {
        BAIL_IF (!IS_NPC (victim),
            "Not on PC's.\n\r", ch);
        if (value < 0 || value > MAX_LEVEL) {
            printf_to_char (ch, "Level range is 0 to %d.\n\r", MAX_LEVEL);
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
        BAIL_IF (value < -10 || value > 30000,
            "Hp range is -10 to 30,000 hit points.\n\r", ch);
        victim->max_hit = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_hit = value;
        return;
    }

    if (!str_prefix (arg2, "mana")) {
        BAIL_IF (value < 0 || value > 30000,
            "Mana range is 0 to 30,000 mana points.\n\r", ch);
        victim->max_mana = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_mana = value;
        return;
    }

    if (!str_prefix (arg2, "move")) {
        BAIL_IF (value < 0 || value > 30000,
            "Move range is 0 to 30,000 move points.\n\r", ch);
        victim->max_move = value;
        if (!IS_NPC (victim))
            victim->pcdata->perm_move = value;
        return;
    }

    if (!str_prefix (arg2, "practice")) {
        BAIL_IF (value < 0 || value > 250,
            "Practice range is 0 to 250 sessions.\n\r", ch);
        victim->practice = value;
        return;
    }

    if (!str_prefix (arg2, "train")) {
        BAIL_IF (value < 0 || value > 50,
            "Training session range is 0 to 50 sessions.\n\r", ch);
        victim->train = value;
        return;
    }

    if (!str_prefix (arg2, "align")) {
        BAIL_IF (value < -1000 || value > 1000,
            "Alignment range is -1000 to 1000.\n\r", ch);
        victim->alignment = value;
        return;
    }

    if (!str_prefix (arg2, "thirst")) {
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
        BAIL_IF (value < -1 || value > 100,
            "Thirst range is -1 to 100.\n\r", ch);
        victim->pcdata->cond_hours[COND_THIRST] = value;
        return;
    }

    if (!str_prefix (arg2, "drunk")) {
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
        BAIL_IF (value < -1 || value > 100,
            "Drunk range is -1 to 100.\n\r", ch);
        victim->pcdata->cond_hours[COND_DRUNK] = value;
        return;
    }

    if (!str_prefix (arg2, "full")) {
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
        BAIL_IF (value < -1 || value > 100,
            "Full range is -1 to 100.\n\r", ch);
        victim->pcdata->cond_hours[COND_FULL] = value;
        return;
    }

    if (!str_prefix (arg2, "hunger")) {
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
        BAIL_IF (value < -1 || value > 100,
            "Full range is -1 to 100.\n\r", ch);
        victim->pcdata->cond_hours[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix (arg2, "race")) {
        int race = race_lookup (arg3);

        BAIL_IF (race <= 0,
            "That is not a valid race.\n\r", ch);
        BAIL_IF (!IS_NPC (victim) && !pc_race_get_by_race (race),
            "That is not a valid player race.\n\r", ch);
        victim->race = race;
        return;
    }

    if (!str_prefix (arg2, "group")) {
        BAIL_IF (!IS_NPC (victim),
            "Only on NPCs.\n\r", ch);
        victim->group = value;
        return;
    }

    if (!str_prefix (arg2, "hours")) {
        BAIL_IF (IS_NPC (victim),
            "Not on NPC's.\n\r", ch);
        BAIL_IF (!is_number (arg3),
            "Value must be numeric.\n\r", ch);

        value = atoi (arg3);
        BAIL_IF (value < 0 || value > 999,
            "Value must be between 0 and 999.\n\r", ch);

        victim->played = value * 3600;
        printf_to_char (ch, "%s's hours set to %d.", victim->name, value);
        return;
    }

    /* Generate usage message. */
    do_function (ch, &do_mset, "");
}

DEFINE_DO_FUN (do_oset) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int value;

    str_smash_tilde (argument);
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
    BAIL_IF ((obj = find_obj_world (ch, arg1)) == NULL,
        "Nothing like that in heaven or earth.\n\r", ch);

    /* Snarf the value (which need not be numeric). */
    value = atoi (arg3);

    /* Set something. */
    BAIL_IF_EXPR (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0"),
        obj->v.value[0] = UMIN (50, value));
    BAIL_IF_EXPR (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1"),
        obj->v.value[1] = value);
    BAIL_IF_EXPR (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2"),
        obj->v.value[2] = value);
    BAIL_IF_EXPR (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3"),
        obj->v.value[3] = value);
    BAIL_IF_EXPR (!str_cmp (arg2, "value4") || !str_cmp (arg2, "v4"),
        obj->v.value[4] = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "extra"),
        obj->extra_flags = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "wear"),
        obj->wear_flags = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "level"),
        obj->level = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "weight"),
        obj->weight = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "cost"),
        obj->cost = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "timer"),
        obj->timer = value);

    /* Generate usage message. */
    do_function (ch, &do_oset, "");
}

DEFINE_DO_FUN (do_rset) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_T *location;
    int value;

    str_smash_tilde (argument);
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
    BAIL_IF ((location = find_location (ch, arg1)) == NULL,
        "No such location.\n\r", ch);
    BAIL_IF (!room_is_owner (location, ch) && ch->in_room != location
            && room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR),
        "That room is private right now.\n\r", ch);

    /* Snarf the value. */
    BAIL_IF (!is_number (arg3),
        "Value must be numeric.\n\r", ch);
    value = atoi (arg3);

    /* Set something. */
    BAIL_IF_EXPR (!str_prefix (arg2, "flags"),
        location->room_flags = value);
    BAIL_IF_EXPR (!str_prefix (arg2, "sector"),
        location->sector_type = value);

    /* Generate usage message. */
    do_function (ch, &do_rset, "");
}

DEFINE_DO_FUN (do_wizlock) {
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
