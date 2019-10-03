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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_info.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"
#include "utils.h"
#include "comm.h"
#include "skills.h"
#include "groups.h"
#include "db.h"
#include "fight.h"
#include "update.h"
#include "comm.h"
#include "save.h"
#include "do_sub.h"
#include "act_comm.h"
#include "act_obj.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "find.h"
#include "spell_info.h"

#include "act_info.h"

#define SCAN_ALL_DIRS -2

static char *const scan_distance[8] = {
    "right here.",
    "nearby %s.",
    "not far %s.",
    "a bit far %s.",
    "far %s.",
    "very far %s.",
    "very, very far %s.",
    "extremely far %s.",
};

bool do_filter_blind (CHAR_DATA * ch) {
    if (!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT))
        return FALSE;
    FILTER (IS_AFFECTED (ch, AFF_BLIND),
        "You can't see a thing!\n\r", ch);
    return FALSE;
}

void do_scan_list (ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
    sh_int depth, sh_int door)
{
    CHAR_DATA *rch;

    if (scan_room == NULL)
        return;
    for (rch = scan_room->people; rch != NULL; rch = rch->next_in_room) {
        if (rch == ch)
            continue;
        if (!IS_NPC (rch) && rch->invis_level > char_get_trust (ch))
            continue;
        if (!char_can_see_anywhere (ch, rch))
            continue;
        do_scan_char (rch, ch, depth, door);
    }
}

void do_scan_char (CHAR_DATA * victim, CHAR_DATA * ch, sh_int depth,
    sh_int door)
{
    const DOOR_TYPE *door_obj;
    char buf[MAX_INPUT_LENGTH];

    door_obj = door_get (door);
    sprintf (buf, scan_distance[depth], door_obj->to_phrase);

    printf_to_char (ch, "%s, %s\n\r", PERS_AW (victim, ch), buf);
}

void do_scan_real (CHAR_DATA * ch, char *argument, int max_depth) {
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    int min_depth, i;
    ROOM_INDEX_DATA *scan_room;
    EXIT_DATA *pExit;
    sh_int door, depth;

    argument = one_argument (argument, arg1);
    if (arg1[0] == '\0') {
        min_depth = 0;
        door = SCAN_ALL_DIRS;
        send_to_char ("Looking around you see:\n\r", ch);
        act ("$n looks all around.", ch, NULL, NULL, TO_NOTCHAR);
    }
    else if ((door = door_lookup (arg1)) >= 0) {
        min_depth = 1;
        act2 ("You peer intently $T.", "$n peers intently $T.",
            ch, NULL, door_table[door].name, 0, POS_RESTING);
        sprintf (buf, "Looking %s you see:\n\r", door_table[door].name);
        max_depth *= 2;
    }
    else {
        send_to_char ("That's not a valid direction.\n\r", ch);
        return;
    }

    if (min_depth <= 0) {
        do_scan_list (ch->in_room, ch, 0, 0);
        min_depth = 1;
    }
    for (i = 0; i < DIR_MAX; i++) {
        if (door != SCAN_ALL_DIRS && door != i)
            continue;
        scan_room = ch->in_room;
        for (depth = 1; depth <= max_depth; depth++) {
            if (depth < min_depth)
                continue;
            if ((pExit = scan_room->exit[i]) == NULL)
                break;
            if (IS_SET (pExit->exit_flags, EX_CLOSED))
                break;
            if ((scan_room = pExit->to_room) == NULL)
                break;
            if (!char_can_see_room (ch, scan_room))
                break;
            do_scan_list (scan_room, ch, depth, i);
        }
    }
}

void do_look_room (CHAR_DATA * ch, int is_auto) {
    char sect_char = room_colour_char (ch->in_room);
    printf_to_char (ch, "{%c%s{x", sect_char, ch->in_room->name);

    if ((IS_IMMORTAL (ch) && (IS_NPC (ch) || IS_SET (ch->plr, PLR_HOLYLIGHT)))
        || IS_BUILDER (ch, ch->in_room->area))
        printf_to_char (ch, "{r [{RRoom %d{r]{x", ch->in_room->vnum);
    send_to_char ("\n\r", ch);

    if (!is_auto || (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
        printf_to_char(ch, "  {S%s{x", ch->in_room->description);

    if (!IS_NPC (ch) && IS_SET (ch->plr, PLR_AUTOEXIT))
        do_function (ch, &do_exits, "auto");

    obj_list_show_to_char (ch->in_room->contents, ch, FALSE, FALSE);
    char_list_show_to_char (ch->in_room->people, ch);
}

void do_look_in (CHAR_DATA * ch, char *arg) {
    OBJ_DATA *obj;

    /* 'look in' */
    BAIL_IF (arg[0] == '\0',
        "Look in what?\n\r", ch);
    BAIL_IF ((obj = find_obj_here (ch, arg)) == NULL,
        "You do not see that here.\n\r", ch);

    switch (obj->item_type) {
        case ITEM_DRINK_CON:
            if (obj->value[1] <= 0)
                send_to_char ("It is empty.\n\r", ch);
            else if (obj->value[1] >= obj->value[0]) {
                printf_to_char (ch,
                    "It's completely filled with a %s liquid.\n\r",
                    liq_table[obj->value[2]].color);
            }
            else {
                int percent;
                char *fullness;

                percent = (obj->value[1] * 100) / obj->value[0];
                     if (percent >= 66) fullness = "more than half-filled";
                else if (percent >= 33) fullness = "about half-filled";
                else                    fullness = "less than half-filled";

                printf_to_char (ch, "It's %s with a %s liquid.\n\r",
                    fullness, liq_table[obj->value[2]].color);
            }
            break;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if (IS_SET (obj->value[1], CONT_CLOSED))
                send_to_char ("It is closed.\n\r", ch);
            else {
                act ("$p holds:", ch, obj, NULL, TO_CHAR);
                obj_list_show_to_char (obj->contains, ch, TRUE, TRUE);
            }
            break;

        default:
            send_to_char ("That is not a container.\n\r", ch);
            break;
    }
}

void do_look_direction (CHAR_DATA * ch, int door) {
    EXIT_DATA *pexit;

    BAIL_IF ((pexit = ch->in_room->exit[door]) == NULL,
        "Nothing special there.\n\r", ch);

    if (pexit->description != NULL && pexit->description[0] != '\0')
        send_to_char (pexit->description, ch);
    else
        send_to_char ("Nothing special there.\n\r", ch);

    if (pexit->keyword != NULL && pexit->keyword[0] != '\0' &&
        pexit->keyword[0] != ' ')
    {
        if (IS_SET (pexit->exit_flags, EX_CLOSED))
            act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        else if (IS_SET (pexit->exit_flags, EX_ISDOOR))
            act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
    }
}

bool do_filter_description_remove_line (CHAR_DATA *ch) {
    char buf[MAX_STRING_LENGTH];
    int len;
    bool found = FALSE;

    FILTER (ch->description == NULL || ch->description[0] == '\0',
        "No lines left to remove.\n\r", ch);

    strcpy (buf, ch->description);
    for (len = strlen (buf); len > 0; len--) {
        if (buf[len] == '\r') {
            if (!found) { /* back it up */
                if (len > 0)
                    len--;
                found = TRUE;
            }
            else { /* found the second one */
                buf[len + 1] = '\0';
                str_free (ch->description);
                ch->description = str_dup (buf);
                send_to_char ("Your description is:\n\r", ch);
                send_to_char (ch->description ? ch->description :
                              "(None).\n\r", ch);
                return FALSE;
            }
        }
    }

    buf[0] = '\0';
    str_free (ch->description);
    ch->description = str_dup (buf);
    send_to_char ("Description cleared.\n\r", ch);
    return FALSE;
}

bool do_filter_description_append (CHAR_DATA *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if (argument[0] == '+') {
        if (ch->description != NULL)
            strcat (buf, ch->description);
        argument++;
        while (isspace (*argument))
            argument++;
    }
    FILTER (strlen (buf) >= 1024,
        "Description too long.\n\r", ch);

    strcat (buf, argument);
    strcat (buf, "\n\r");

    str_replace_dup (&ch->description, buf);
    return FALSE;
}

bool do_filter_description_alter (CHAR_DATA *ch, char *argument) {
    smash_tilde (argument);
    if (argument[0] == '-')
        return do_filter_description_remove_line (ch);
    else
        return do_filter_description_append (ch, argument);
}

/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_motd (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "motd"); }
void do_rules (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "rules"); }
void do_story (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "story"); }
void do_wizlist (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "wizlist"); }

/* Not-RT(?) commands that are similar */
void do_credits (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "diku"); }

void do_look (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj_list, *obj;
    char *pdesc1, *pdesc2;
    int i, door;
    int number, count;

    if (ch->desc == NULL)
        return;

    BAIL_IF (ch->position < POS_SLEEPING,
        "You can't see anything but stars!\n\r", ch);
    BAIL_IF (ch->position == POS_SLEEPING,
        "You can't see anything, you're sleeping!\n\r", ch);
    if (do_filter_blind (ch))
        return;

    if (!IS_NPC (ch) && !IS_SET (ch->plr, PLR_HOLYLIGHT) &&
        room_is_dark (ch->in_room))
    {
        send_to_char ("{DIt is pitch black ... {x\n\r", ch);
        char_list_show_to_char (ch->in_room->people, ch);
        return;
    }

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    number   = number_argument (arg1, arg3);
    count    = 0;

    /* "Auto" look shows the room. */
    if (arg1[0] == '\0' || !str_cmp (arg1, "auto")) {
        do_look_room (ch, arg1[0] != '\0');
        return;
    }

    /* Looking in something? */
    if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in") || !str_cmp (arg1, "on")) {
        do_look_in (ch, arg2);
        return;
    }

    /* Looking at someone? */
    if ((victim = find_char_same_room (ch, arg1)) != NULL) {
        char_look_at_char (victim, ch);
        return;
    }

    #define CHECK_LOOK(cond, str, with_crlf) \
        if ((cond)) { \
            if (++count == number) { \
                send_to_char ((str), ch); \
                if ((with_crlf)) \
                    send_to_char ("\n\r", ch); \
                return; \
            } \
            continue; \
        }

    /* Looking at any obj extra descriptions? */
    for (i = 0; i < 2; i++) {
        switch (i) {
            case 0:  obj_list = ch->carrying;          break;
            case 1:  obj_list = ch->in_room->contents; break;
            default: obj_list = NULL;
        }
        for (obj = obj_list; obj != NULL; obj = obj->next_content) {
            if (!char_can_see_obj (ch, obj))
                continue;
            pdesc1 = get_extra_descr (arg3, obj->extra_descr);
            pdesc2 = get_extra_descr (arg3, obj->pIndexData->extra_descr);

            CHECK_LOOK (pdesc1 != NULL, pdesc1, FALSE);
            CHECK_LOOK (pdesc2 != NULL, pdesc2, FALSE);
            CHECK_LOOK (is_name (arg3, obj->name), obj->description, TRUE);
        }
    }

    do {
        pdesc1 = get_extra_descr (arg3, ch->in_room->extra_descr);
        CHECK_LOOK (pdesc1 != NULL, pdesc1, FALSE);
    } while (0);

    /* Did we exceed the count? */
    if (count > 0 && count != number) {
        if (count == 1)
            printf_to_char (ch, "You only see one %s here.\n\r", arg3);
        else
            printf_to_char (ch, "You only see %d of those here.\n\r", count);
        return;
    }

    /* Check for a specific direction. */
    if ((door = door_lookup (arg1)) >= 0) {
        do_look_direction (ch, door);
        return;
    }

    /* All options failed. */
    send_to_char ("You do not see that here.\n\r", ch);
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_look, argument); }

void do_examine (CHAR_DATA * ch, char *argument) {
    char *full_arg;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    full_arg = argument;
    DO_REQUIRE_ARG (arg, "Examine what?\n\r");

    do_function (ch, &do_look, arg);
    if ((obj = find_obj_here (ch, arg)) == NULL)
        return;

    switch (obj->item_type) {
        case ITEM_JUKEBOX:
            do_function (ch, &do_play, "list");
            break;

        case ITEM_MONEY:
            if (obj->value[0] == 0) {
                if (obj->value[1] == 0)
                    sprintf (buf, "Odd...there's no coins in the pile.\n\r");
                else if (obj->value[1] == 1)
                    sprintf (buf, "Wow. One gold coin.\n\r");
                else
                    sprintf (buf, "There are %d gold coins in the pile.\n\r", obj->value[1]);
            }
            else if (obj->value[1] == 0) {
                if (obj->value[0] == 1)
                    sprintf (buf, "Wow. One silver coin.\n\r");
                else
                    sprintf (buf, "There are %d silver coins in the pile.\n\r", obj->value[0]);
            }
            else
                sprintf (buf, "There are %d gold and %d silver coins in the pile.\n\r",
                    obj->value[1], obj->value[0]);
            send_to_char (buf, ch);
            break;

        case ITEM_DRINK_CON:
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            sprintf (buf, "in %s", full_arg);
            do_function (ch, &do_look, buf);
    }
}

void do_lore (CHAR_DATA * ch, char *arg) {
    OBJ_DATA *obj;
    int skill = get_skill (ch, gsn_lore);

    BAIL_IF (skill == 0,
        "You haven't studied any lore.\n\r", ch);
    BAIL_IF (arg[0] == '\0',
        "Check the lore on what?\n\r", ch);
    BAIL_IF ((obj = find_obj_here (ch, arg)) == NULL,
        "You can't find that here.\n\r", ch);
    spell_identify_perform (ch, obj, skill);
}

/* Thanks to Zrin for auto-exit part. */
void do_exits (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    bool fAuto;
    int mode;

    fAuto = !str_cmp (argument, "auto");
    if (do_filter_blind (ch))
        return;

    if (fAuto)
        sprintf (buf, "{o[Exits: ");
    else if (IS_IMMORTAL (ch))
        sprintf (buf, "Obvious exits from room %d:\n\r", ch->in_room->vnum);
    else
        sprintf (buf, "Obvious exits:\n\r");
    send_to_char (buf, ch);

    mode = fAuto ? EXITS_AUTO : EXITS_LONG;
    char_exit_string (ch, ch->in_room, mode, buf, sizeof (buf));
    if (fAuto)
        printf_to_char (ch, "%s]{x\n\r", buf);
    else
        printf_to_char (ch, "%s", buf);
}

void do_worth (CHAR_DATA * ch, char *argument) {
    if (IS_NPC (ch)) {
        printf_to_char (ch, "You have %ld gold and %ld silver.\n\r",
            ch->gold, ch->silver);
        return;
    }
    printf_to_char (ch,
        "You have %ld gold, %ld silver, and %d experience (%d exp to level).\n\r",
        ch->gold, ch->silver, ch->exp, get_exp_to_level(ch));
}

void do_score (CHAR_DATA * ch, char *argument) {
    int i;

    printf_to_char (ch, "You are %s%s, level %d, %d years old (%d hours).\n\r",
        ch->name, IS_NPC (ch) ? "" : ch->pcdata->title, ch->level,
        char_get_age (ch), (ch->played + (int) (current_time - ch->logon)) / 3600);

    if (char_get_trust (ch) != ch->level)
        printf_to_char (ch, "You are trusted at level %d.\n\r",
            char_get_trust (ch));

    printf_to_char (ch, "Race: %s  Sex: %s  Class: %s\n\r",
        race_table[ch->race].name, get_sex_name(ch->sex), get_ch_class_name(ch));

    printf_to_char (ch, "You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
        ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);

#ifndef VANILLA
    printf_to_char (ch, "Your recovery rates are %+d hit, %+d mana, %+d movement.\n\r",
        hit_gain(ch, FALSE), mana_gain(ch, FALSE), move_gain(ch, FALSE));
#endif

    printf_to_char (ch, "You have %d practices and %d training sessions.\n\r",
        ch->practice, ch->train);

    printf_to_char (ch, "You are carrying %d/%d items with weight %ld/%d pounds.\n\r",
        ch->carry_number, char_get_max_carry_count (ch),
        char_get_carry_weight (ch) / 10, char_get_max_carry_weight (ch) / 10);

    printf_to_char (ch, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
        ch->perm_stat[STAT_STR], char_get_curr_stat (ch, STAT_STR),
        ch->perm_stat[STAT_INT], char_get_curr_stat (ch, STAT_INT),
        ch->perm_stat[STAT_WIS], char_get_curr_stat (ch, STAT_WIS),
        ch->perm_stat[STAT_DEX], char_get_curr_stat (ch, STAT_DEX),
        ch->perm_stat[STAT_CON], char_get_curr_stat (ch, STAT_CON));

    printf_to_char (ch, "You have scored %d exp, and have %ld gold and %ld silver coins.\n\r",
        ch->exp, ch->gold, ch->silver);

    /* RT shows exp to level */
    if (!IS_NPC (ch) && ch->level < LEVEL_HERO)
        printf_to_char (ch, "You need %d exp to level.\n\r",
            get_exp_to_level(ch));

    printf_to_char (ch, "Wimpy set to %d hit points.\n\r", ch->wimpy);

    if (IS_DRUNK (ch))
        send_to_char ("You are drunk.\n\r", ch);
    if (IS_THIRSTY (ch))
        send_to_char ("You are thirsty.\n\r", ch);
    if (IS_HUNGRY (ch))
        send_to_char ("You are hungry.\n\r", ch);

    printf_to_char (ch, "You are %s\r\n",
        get_character_position_str (ch, ch->position, ch->on, TRUE));

    /* print attack information */
    if (ch->level >= 15)
        printf_to_char (ch, "Hitroll: %d  Damroll: %d.\n\r",
            GET_HITROLL (ch), GET_DAMROLL (ch));

    /* print AC values */
    if (ch->level >= 25)
        printf_to_char (ch, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
            GET_AC (ch, AC_PIERCE), GET_AC (ch, AC_BASH),
            GET_AC (ch, AC_SLASH), GET_AC (ch, AC_EXOTIC));

    for (i = 0; i < 4; i++)
        printf_to_char (ch, "You are %s %s.\n\r",
            get_ac_rating_phrase (GET_AC (ch, i)), get_ac_type_name(i));

    if (ch->level >= 10)
        printf_to_char (ch, "Alignment: %d.  ", ch->alignment);
    printf_to_char (ch, "You are %s.\n\r", get_align_name (ch->alignment));

    /* wizinvis and holy light */
    if (IS_IMMORTAL (ch)) {
        printf_to_char (ch, "Holy Light: %s",
            IS_SET (ch->plr, PLR_HOLYLIGHT) ? "ON" : "OFF");
        if (ch->invis_level)
            printf_to_char (ch, "  Invisible: level %d", ch->invis_level);
        if (ch->incog_level)
            printf_to_char (ch, "  Incognito: level %d", ch->incog_level);
        send_to_char ("\n\r", ch);
    }

    if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
        do_function (ch, &do_affects, "");
}

void do_affects (CHAR_DATA * ch, char *argument) {
    AFFECT_DATA *paf, *paf_last = NULL;

    BAIL_IF (ch->affected == NULL,
        "You are not affected by any spells.\n\r", ch);

    send_to_char ("You are affected by the following spells:\n\r", ch);
    for (paf = ch->affected; paf != NULL; paf = paf->next) {
        if (paf_last == NULL || paf->type != paf_last->type)
            printf_to_char (ch, "Spell: %-15s\n\r", skill_table[paf->type].name);
        else if (ch->level < 20)
            continue;

        if (ch->level >= 20) {
            if (paf->apply == APPLY_NONE)
                printf_to_char (ch, "   lasts ");
            else
                printf_to_char (ch, "   modifies %s by %d ",
                    affect_apply_name (paf->apply), paf->modifier);
            if (paf->duration == -1)
                send_to_char ("permanently\n\r", ch);
            else
                printf_to_char (ch, "for %d hours\n\r", paf->duration);
        }
        paf_last = paf;
    }
}

void do_time (CHAR_DATA * ch, char *argument) {
    const DAY_TYPE *day_obj;
    const MONTH_TYPE *month_obj;
    extern char str_boot_time[];
    char *suf;
    int day = time_info.day + 1;

    /* Determine suffix */
         if (day > 4 && day < 20) suf = "th";
    else if (day % 10 == 1)       suf = "st";
    else if (day % 10 == 2)       suf = "nd";
    else if (day % 10 == 3)       suf = "rd";
    else                          suf = "th";

    day_obj   = day_get_current();
    month_obj = month_get_current();

    printf_to_char (ch,
        "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
        (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
        time_info.hour >= 12 ? "pm" : "am",
        day_obj->name, day, suf, month_obj->name);

    printf_to_char (ch, "ROM started up at %s\n\rThe system time is %s.\n\r",
        str_boot_time, (char *) ctime (&current_time));
}

void do_weather (CHAR_DATA * ch, char *argument) {
    const SKY_TYPE *sky;
    char *change;

    BAIL_IF (!IS_OUTSIDE (ch),
        "You can't see the weather indoors.\n\r", ch);

    sky = sky_get_current ();
    change = weather_info.change >= 0
        ? "a warm southerly breeze blows"
        : "a cold northern gust blows";

    printf_to_char (ch, "The sky is %s and %s.\n\r", sky->description, change);
}

void do_help (CHAR_DATA * ch, char *argument) {
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    int level;

    output = buf_new ();
    if (argument[0] == '\0')
        argument = "summary";

    /* This parts handles "help a b" so that it returns "help 'a b'" */
    argall[0] = '\0';
    while (argument[0] != '\0') {
        argument = one_argument (argument, argone);
        if (argall[0] != '\0')
            strcat (argall, " ");
        strcat (argall, argone);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next) {
        level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;
        if (level > char_get_trust (ch))
            continue;

        if (is_name (argall, pHelp->keyword)) {
            /* add seperator if found */
            if (found)
                add_buf (output,
                    "\n\r============================================================\n\r\n\r");
            if (pHelp->level >= 0 && str_cmp (argall, "imotd")) {
                add_buf (output, pHelp->keyword);
                add_buf (output, "\n\r");
            }

            /* Strip leading '.' to allow initial blanks. */
            if (pHelp->text[0] == '.')
                add_buf (output, pHelp->text + 1);
            else
                add_buf (output, pHelp->text);
            found = TRUE;

            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING
                    && ch->desc->connected != CON_GEN_GROUPS)
                break;
        }
    }

    if (!found) {
        send_to_char ("No help on that word.\n\r", ch);
        /* Let's log unmet help requests so studious IMP's can improve their help files ;-)
         * But to avoid idiots, we will check the length of the help request, and trim to
         * a reasonable length (set it by redefining MAX_CMD_LEN in merc.h).  -- JR */
        if (strlen(argall) > MAX_CMD_LEN) {
            argall[MAX_CMD_LEN - 1] = '\0';
            log_f ("Excessive command length: %s requested %s.", ch->name, argall);
            send_to_char ("That was rude!\n\r", ch);
        }
        /* OHELPS_FILE is the "orphaned helps" files. Defined in merc.h -- JR */
        else
            append_file (ch, OHELPS_FILE, argall);
    }
    else
        page_to_char (buf_string (output), ch);

    buf_free (output);
}

/* whois command */
void do_whois (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    DO_REQUIRE_ARG (arg, "You must provide a name.\n\r");

    output = buf_new ();
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_DATA *wch = CH(d);
        if (d->connected != CON_PLAYING)
            continue;
        if (!char_can_see_anywhere (ch, d->character))
            continue;
        if (!char_can_see_anywhere (ch, wch))
            continue;
        if (str_prefix (arg, wch->name))
            continue;

        found = TRUE;
        char_get_who_string (ch, wch, buf, sizeof(buf));
        add_buf (output, buf);
    }

    BAIL_IF (!found,
        "No one of that name is playing.\n\r", ch);

    page_to_char (buf_string (output), ch);
    buf_free (output);
}

/* New 'who' command originally by Alander of Rivers of Mud. */
void do_who (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass, iRace, iClan, iLevelLower, iLevelUpper;
    int nNumber, nMatch;
    bool rgfClass[CLASS_MAX];
    bool rgfRace[PC_RACE_MAX];
    bool rgfClan[CLAN_MAX];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;

    /* Set default arguments. */
    iLevelLower = 0;
    iLevelUpper = MAX_LEVEL;
    for (iClass = 0; iClass < CLASS_MAX; iClass++)
        rgfClass[iClass] = FALSE;
    for (iRace = 0; iRace < PC_RACE_MAX; iRace++)
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < CLAN_MAX; iClan++)
        rgfClan[iClan] = FALSE;

    /* Parse arguments. */
    nNumber = 0;
    while (1) {
        char arg[MAX_STRING_LENGTH];
        argument = one_argument (argument, arg);
        if (arg[0] == '\0')
            break;

        /* Check for level arguments. */
        if (is_number (arg)) {
            switch (++nNumber) {
                case 1: iLevelLower = atoi (arg); break;
                case 2: iLevelUpper = atoi (arg); break;
                default:
                    send_to_char ("Only two level numbers allowed.\n\r", ch);
                    return;
            }
            continue;
        }

        /* Look for classes to turn on. */
        if (!str_prefix (arg, "immortals")) {
            fImmortalOnly = TRUE;
            continue;
        }

        /* Check for explicit classes. */
        iClass = class_lookup (arg);
        if (iClass >= 0) {
            fClassRestrict = TRUE;
            rgfClass[iClass] = TRUE;
            continue;
        }

        /* Check for explicit races. */
        iRace = race_lookup (arg);
        if (iRace > 0 && iRace < PC_RACE_MAX) {
            fRaceRestrict = TRUE;
            rgfRace[iRace] = TRUE;
            continue;
        }

        /* Check for anyone with a clan. */
        if (!str_prefix (arg, "clan")) {
            fClan = TRUE;
            continue;
        }

        /* Check for specific clans. */
        iClan = clan_lookup (arg);
        if (iClan) {
            fClanRestrict = TRUE;
            rgfClan[iClan] = TRUE;
            continue;
        }

        /* Unknown argument. */
        send_to_char ("That's not a valid race, class, or clan.\n\r", ch);
        return;
    }

    /* Now show matching chars.  */
    nMatch = 0;
    buf[0] = '\0';
    output = buf_new ();
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_DATA *wch = CH(d);

        /* Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.  */
        if (d->connected != CON_PLAYING)
            continue;
        if (!char_can_see_anywhere (ch, d->character))
            continue;
        if (!char_can_see_anywhere (ch, wch))
            continue;
        if (wch->level < iLevelLower
            || wch->level > iLevelUpper
            || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
            || (fClassRestrict && !rgfClass[wch->class])
            || (fRaceRestrict && !rgfRace[wch->race])
            || (fClan && !char_has_clan (wch))
            || (fClanRestrict && !rgfClan[wch->clan]))
            continue;

        nMatch++;
        char_get_who_string (ch, wch, buf, sizeof(buf));
        add_buf (output, buf);
    }

    sprintf (buf2, "\n\rPlayers found: %d\n\r", nMatch);
    add_buf (output, buf2);
    page_to_char (buf_string (output), ch);
    buf_free (output);
}

/* for keeping track of the player count */
static int max_on = 0;
void do_count (CHAR_DATA * ch, char *argument) {
    int count;
    DESCRIPTOR_DATA *d;

    count = 0;
    for (d = descriptor_list; d != NULL; d = d->next)
        if (d->connected == CON_PLAYING && char_can_see_anywhere (ch, d->character))
            count++;
    max_on = UMAX (count, max_on);

    if (max_on == count) {
        printf_to_char (ch,
            "There are %d characters on, the most so far today.\n\r",
            count);
    }
    else {
        printf_to_char (ch,
            "There are %d characters on, the most on today was %d.\n\r",
            count, max_on);
    }
}

void do_inventory (CHAR_DATA * ch, char *argument) {
    send_to_char ("You are carrying:\n\r", ch);
    obj_list_show_to_char (ch->carrying, ch, TRUE, TRUE);
}

void do_equipment (CHAR_DATA * ch, char *argument) {
    const WEAR_TYPE *wear;
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char ("You are using:\n\r", ch);
    found = FALSE;
    for (iWear = 0; iWear < WEAR_MAX; iWear++) {
        if ((obj = char_get_eq_by_wear (ch, iWear)) == NULL)
            continue;
        if ((wear = wear_get (iWear)) == NULL)
            continue;

        send_to_char (wear->look_msg, ch);
        if (char_can_see_obj (ch, obj)) {
            send_to_char (obj_format_to_char (obj, ch, TRUE), ch);
            send_to_char ("\n\r", ch);
        }
        else
            send_to_char ("something.\n\r", ch);
        found = TRUE;
    }

    if (!found)
        send_to_char ("Nothing.\n\r", ch);
}

void do_compare (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument (argument, arg1);
    BAIL_IF (arg1[0] == '\0',
        "Compare what to what?\n\r", ch);
    BAIL_IF ((obj1 = find_obj_own_inventory (ch, arg1)) == NULL,
        "You do not have that item.\n\r", ch);

    argument = one_argument (argument, arg2);
    if (arg2[0] == '\0') {
        for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content) {
            if (obj2->wear_loc == WEAR_NONE)
                continue;
            if (!char_can_see_obj (ch, obj2))
                continue;
            if (obj1->item_type != obj2->item_type)
                continue;
            if ((obj1->wear_flags & (obj2->wear_flags & ~ITEM_TAKE)) == 0)
                continue;
            break;
        }
        BAIL_IF (obj2 == NULL,
            "You aren't wearing anything comparable.\n\r", ch);
    }
    else if ((obj2 = find_obj_own_inventory (ch, arg2)) == NULL) {
        send_to_char ("You do not have that item.\n\r", ch);
        return;
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2)
        msg = "You compare $p to itself.  It looks about the same.";
    else if (obj1->item_type != obj2->item_type)
        msg = "You can't compare $p and $P.";
    else {
        switch (obj1->item_type) {
            default:
                msg = "You can't compare $p and $P.";
                break;

            case ITEM_ARMOR:
                value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
                value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
                break;

            case ITEM_WEAPON:
                if (obj1->pIndexData->new_format) {
                    value1 = (1 + obj1->value[2]) * obj1->value[1];
                    value2 = (1 + obj2->value[2]) * obj2->value[1];
                }
                else {
                    value1 = obj1->value[1] + obj1->value[2];
                    value2 = obj2->value[1] + obj2->value[2];
                }
                break;
        }
    }

    if (msg == NULL) {
             if (value1 == value2) msg = "$p and $P look about the same.";
        else if (value1  > value2) msg = "$p looks better than $P.";
        else                       msg = "$p looks worse than $P.";
    }

    act (msg, ch, obj1, obj2, TO_CHAR);
}

void do_where (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument (argument, arg);

    if (arg[0] == '\0') {
        send_to_char ("Players near you:\n\r", ch);
        found = FALSE;
        for (d = descriptor_list; d; d = d->next) {
            if (d->connected == CON_PLAYING
                && (victim = d->character) != NULL && !IS_NPC (victim)
                && victim->in_room != NULL
                && !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
                && (room_is_owner (victim->in_room, ch)
                    || !room_is_private (victim->in_room))
                && victim->in_room->area == ch->in_room->area
                && char_can_see_anywhere (ch, victim))
            {
                found = TRUE;
                sprintf (buf, "%-28s %s\n\r",
                         victim->name, victim->in_room->name);
                send_to_char (buf, ch);
            }
        }
        if (!found)
            send_to_char ("None\n\r", ch);
    }
    else {
        found = FALSE;
        for (victim = char_list; victim != NULL; victim = victim->next) {
            if (victim->in_room != NULL
                && victim->in_room->area == ch->in_room->area
                && !IS_AFFECTED (victim, AFF_HIDE)
                && !IS_AFFECTED (victim, AFF_SNEAK)
                && char_can_see_anywhere (ch, victim) && is_name (arg, victim->name))
            {
                found = TRUE;
                sprintf (buf, "%-28s %s\n\r",
                         PERS_AW (victim, ch), victim->in_room->name);
                send_to_char (buf, ch);
                break;
            }
        }
        if (!found)
            act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
    }
}

void do_title (CHAR_DATA * ch, char *argument) {
    int i;

    if (IS_NPC (ch))
        return;

    /* Changed this around a bit to do some sanitization first   *
     * before checking length of the title. Need to come up with *
     * a centralized user input sanitization scheme. FIXME!      *
     * JR -- 10/15/00                                            */

    if (strlen (argument) > 45)
        argument[45] = '\0';

    i = strlen(argument);
    if (argument[i-1] == '{' && argument[i-2] != '{')
        argument[i-1] = '\0';

    BAIL_IF (argument[0] == '\0',
        "Change your title to what?\n\r", ch);

    smash_tilde (argument);
    char_set_title (ch, argument);
    send_to_char ("Ok.\n\r", ch);
}

void do_description (CHAR_DATA * ch, char *argument) {
    if (argument[0] != '\0')
        if (do_filter_description_alter (ch, argument))
            return;

    send_to_char ("Your description is:\n\r", ch);
    send_to_char (ch->description ? ch->description : "(None).\n\r", ch);
}

void do_report (CHAR_DATA * ch, char *argument) {
    char buf[MAX_INPUT_LENGTH];
    sprintf (buf, "I have %d/%d hp %d/%d mana %d/%d mv %d xp.",
        ch->hit, ch->max_hit, ch->mana, ch->max_mana,
        ch->move, ch->max_move, ch->exp);
    do_say (ch, buf);
}

/* Contributed by Alander. */
void do_commands (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (cmd_table[cmd].level < LEVEL_HERO &&
            cmd_table[cmd].level <= char_get_trust (ch) && cmd_table[cmd].show)
        {
            sprintf (buf, "%-12s", cmd_table[cmd].name);
            send_to_char (buf, ch);
            if (++col % 6 == 0)
                send_to_char ("\n\r", ch);
        }
    }

    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
    return;
}

void do_areas (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    BAIL_IF (argument[0] != '\0',
        "No argument is used with this command.\n\r", ch);

    iAreaHalf = (TOP(RECYCLE_AREA_DATA) + 1) / 2;
    pArea1 = area_first;
    pArea2 = area_first;
    for (iArea = 0; iArea < iAreaHalf; iArea++)
        pArea2 = pArea2->next;

    for (iArea = 0; iArea < iAreaHalf; iArea++) {
        sprintf (buf, "%-39s%-39s\n\r",
                 pArea1->credits, (pArea2 != NULL) ? pArea2->credits : "");
        send_to_char_bw (buf, ch);
        pArea1 = pArea1->next;
        if (pArea2 != NULL)
            pArea2 = pArea2->next;
    }
}

void do_scan_short (CHAR_DATA * ch, char *argument)
    { do_scan_real (ch, argument, 1); }
void do_scan_far (CHAR_DATA * ch, char *argument)
    { do_scan_real (ch, argument, 3); }
