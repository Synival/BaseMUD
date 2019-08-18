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

#include "interp.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "lookup.h"
#include "recycle.h"
#include "do_sub.h"
#include "mob_cmds.h"
#include "act_info.h"
#include "chars.h"
#include "objs.h"
#include "rooms.h"
#include "find.h"
#include "affects.h"

#include "wiz_im.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: employ tables whenever possible */
/* TODO: do_stat() and its derivatives are GIGANTIC. deflate somehow? */
/* TODO: merge do_invis() and do_incognito() */
/* TODO: possibly use a table for do_stat() */

void do_wizhelp (CHAR_DATA * ch, char *argument) {
    int cmd, col;

    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (cmd_table[cmd].level >= LEVEL_HERO &&
            cmd_table[cmd].level <= char_get_trust (ch) && cmd_table[cmd].show)
        {
            printf_to_char (ch, "%-12s", cmd_table[cmd].name);
            if (++col % 6 == 0)
                send_to_char ("\n\r", ch);
        }
    }
    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
}

void do_holylight (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_HOLYLIGHT,
        "Holy light mode off.\n\r",
        "Holy light mode on.\n\r");
}

void do_incognito (CHAR_DATA * ch, char *argument) {
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument (argument, arg);

    /* take the default path */
    if (arg[0] == '\0') {
        if (ch->incog_level) {
            ch->incog_level = 0;
            send_to_char ("You are no longer cloaked.\n\r", ch);
            act ("$n is no longer cloaked.", ch, NULL, NULL, TO_NOTCHAR);
        }
        else {
            ch->incog_level = char_get_trust (ch);
            send_to_char ("You cloak your presence.\n\r", ch);
            act ("$n cloaks $s presence.", ch, NULL, NULL, TO_NOTCHAR);
        }
        return;
    }

    /* do the level thing */
    level = atoi (arg);
    BAIL_IF (level < 2 || level > char_get_trust (ch),
        "Incog level must be between 2 and your level.\n\r", ch);

    ch->reply = NULL;
    ch->incog_level = level;
    send_to_char ("You cloak your presence.\n\r", ch);
    act ("$n cloaks $s presence.", ch, NULL, NULL, TO_NOTCHAR);
}

/* New routines by Dionysos. */
void do_invis (CHAR_DATA * ch, char *argument) {
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument (argument, arg);

    if (arg[0] == '\0') {
        /* take the default path */
        if (ch->invis_level) {
            ch->invis_level = 0;
            send_to_char ("You slowly fade back into existence.\n\r", ch);
            act ("$n slowly fades into existence.", ch, NULL, NULL, TO_NOTCHAR);
        }
        else {
            ch->invis_level = char_get_trust (ch);
            send_to_char ("You slowly vanish into thin air.\n\r", ch);
            act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_NOTCHAR);
        }
        return;
    }

    /* do the level thing */
    level = atoi (arg);
    BAIL_IF (level < 2 || level > char_get_trust (ch),
        "Invis level must be between 2 and your level.\n\r", ch);

    ch->reply = NULL;
    ch->invis_level = level;
    send_to_char ("You slowly vanish into thin air.\n\r", ch);
    act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_NOTCHAR);
}

void do_memory (CHAR_DATA * ch, char *argument) {
    char *buf = memory_dump ("\n\r");
    send_to_char (buf, ch);
}

void do_mwhere (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if (argument[0] == '\0') {
        DESCRIPTOR_DATA *d;

        /* show characters logged */
        buffer = buf_new ();
        for (d = descriptor_list; d != NULL; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;
            if ((victim = d->character) == NULL)
                continue;
            if (victim->in_room == NULL)
                continue;
            if (!char_can_see_anywhere (ch, victim))
                continue;
            if (!char_can_see_room (ch, victim->in_room))
                continue;

            count++;
            if (d->original != NULL)
                sprintf (buf, "%3d) %s (in the body of %s) is in %s [%d]\n\r",
                    count, d->original->name, victim->short_descr,
                    victim->in_room->name, victim->in_room->vnum);
            else
                sprintf (buf, "%3d) %s is in %s [%d]\n\r", count,
                    victim->name, victim->in_room->name,
                    victim->in_room->vnum);

            add_buf (buffer, buf);
        }

        page_to_char (buf_string (buffer), ch);
        buf_free (buffer);
        return;
    }

    found = FALSE;
    buffer = buf_new ();
    for (victim = char_list; victim != NULL; victim = victim->next) {
        if (victim->in_room == NULL)
            continue;
        if (!is_name (argument, victim->name))
            continue;

        found = TRUE;
        count++;
        sprintf (buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                 IS_NPC (victim) ? victim->pIndexData->vnum : 0,
                 IS_NPC (victim) ? victim->short_descr : victim->name,
                 victim->in_room->vnum, victim->in_room->name);
        add_buf (buffer, buf);
    }

    if (!found)
        act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
    else
        page_to_char (buf_string (buffer), ch);

    buf_free (buffer);
}

void do_owhere (CHAR_DATA * ch, char *argument) {
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = buf_new ();
    BAIL_IF (argument[0] == '\0',
        "Find what?\n\r", ch);

    for (obj = object_list; obj != NULL; obj = obj->next) {
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (argument, obj->name))
            continue;
        if (ch->level < obj->level)
            continue;

        found = TRUE;
        number++;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
            ; /* empty */
        if (in_obj->carried_by != NULL && char_can_see_anywhere (ch, in_obj->carried_by)
            && in_obj->carried_by->in_room != NULL)
            sprintf (buf, "%3d) %s is carried by %s [Room %d]\n\r",
                     number, obj->short_descr, PERS_AW (in_obj->carried_by, ch),
                     in_obj->carried_by->in_room->vnum);
        else if (in_obj->in_room != NULL && char_can_see_room (ch, in_obj->in_room))
            sprintf (buf, "%3d) %s is in %s [Room %d]\n\r",
                     number, obj->short_descr, in_obj->in_room->name,
                     in_obj->in_room->vnum);
        else
            sprintf (buf, "%3d) %s is somewhere\n\r", number,
                     obj->short_descr);

        buf[0] = UPPER (buf[0]);
        add_buf (buffer, buf);

        if (number >= max_found)
            break;
    }

    if (!found)
        send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
    else
        page_to_char (buf_string (buffer), ch);

    buf_free (buffer);
}

/* RT to replace the 3 stat commands */
void do_stat (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  stat <name>\n\r", ch);
        send_to_char ("  stat obj <name>\n\r", ch);
        send_to_char ("  stat mob <name>\n\r", ch);
        send_to_char ("  stat room <number>\n\r", ch);
        return;
    }

    /* first, check for explicit types. */
    BAIL_IF_EXPR (!str_cmp (arg, "room"),
        do_function (ch, &do_rstat, string));
    BAIL_IF_EXPR (!str_cmp (arg, "obj"),
        do_function (ch, &do_ostat, string));
    BAIL_IF_EXPR (!str_cmp (arg, "char") || !str_cmp (arg, "mob"),
        do_function (ch, &do_mstat, string));

    /* do it the old way */
    BAIL_IF_EXPR ((obj = find_obj_world (ch, argument)) != NULL,
        do_function (ch, &do_ostat, argument));
    BAIL_IF_EXPR ((victim = find_char_world (ch, argument)) != NULL,
        do_function (ch, &do_mstat, argument));
    BAIL_IF_EXPR ((location = find_location (ch, argument)) != NULL,
        do_function (ch, &do_rstat, argument));

    send_to_char ("Nothing by that name found anywhere.\n\r", ch);
}

void do_rstat (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    location = (argument[0] == '\0')
        ? ch->in_room : find_location (ch, argument);
    BAIL_IF (location == NULL,
        "No such location.\n\r", ch);

    BAIL_IF (!room_is_owner (location, ch) && ch->in_room != location &&
            room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR),
        "That room is private right now.\n\r", ch);

    printf_to_char (ch, "Name: '%s'\n\rArea: '%s'\n\r",
        location->name, location->area->title);

    printf_to_char (ch,
        "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
        location->vnum, location->sector_type, location->light,
        location->heal_rate, location->mana_rate);

    printf_to_char (ch, "Room flags: %ld.\n\rDescription:\n\r%s",
        location->room_flags, location->description);

    if (location->extra_descr != NULL) {
        EXTRA_DESCR_DATA *ed;

        send_to_char ("Extra description keywords: '", ch);
        for (ed = location->extra_descr; ed; ed = ed->next) {
            send_to_char (ed->keyword, ch);
            if (ed->next != NULL)
                send_to_char (" ", ch);
        }
        send_to_char ("'.\n\r", ch);
    }

    send_to_char ("Characters:", ch);
    for (rch = location->people; rch; rch = rch->next_in_room) {
        if (!char_can_see_anywhere (ch, rch))
            continue;
        one_argument (rch->name, buf);
        printf_to_char (ch, " %s", buf);
    }
    send_to_char (".\n\r", ch);

    send_to_char ("Objects:   ", ch);
    for (obj = location->contents; obj; obj = obj->next_content) {
        one_argument (obj->name, buf);
        printf_to_char (ch, " %s", buf);
    }
    send_to_char (".\n\r", ch);

    for (door = 0; door <= 5; door++) {
        EXIT_DATA *pexit;
        if ((pexit = location->exit[door]) == NULL)
            continue;

        printf_to_char (ch,
            "Door: %d.  To: %d.  Key: %d.  Exit flags: %ld.\n\r"
            "Keyword: '%s'.  Description: %s",
            door,
            (pexit->to_room == NULL ? -1 : pexit->to_room->vnum),
            pexit->key, pexit->exit_flags, pexit->keyword,
            pexit->description[0] != '\0' ? pexit->description : "(none).\n\r"
        );
    }
}

void do_ostat (CHAR_DATA * ch, char *argument) {
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    BAIL_IF (argument[0] == '\0',
        "Stat what?\n\r", ch);
    BAIL_IF ((obj = find_obj_world (ch, argument)) == NULL,
        "Nothing like that in hell, earth, or heaven.\n\r", ch);

    printf_to_char (ch, "Name(s): %s\n\r", obj->name);

    printf_to_char (ch, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
        obj->pIndexData->vnum,
        obj->pIndexData->new_format ? "new" : "old",
        item_get_name (obj->item_type), obj->pIndexData->reset_num);

    printf_to_char (ch, "Short description: %s\n\rLong description: %s\n\r",
        obj->short_descr, obj->description);

    printf_to_char (ch, "Wear bits: %s\n\rExtra bits: %s\n\r",
        wear_bit_name (obj->wear_flags), extra_bit_name (obj->extra_flags));

    printf_to_char (ch,
        "Number: %d/%d  Weight: %d/%d/%d (10th pounds)  Material: %s\n\r",
        1, obj_get_carry_number (obj), obj->weight, obj_get_weight (obj),
        obj_get_true_weight (obj),
        if_null_str ((char *) material_get_name (obj->material), "unknown"));

    printf_to_char (ch, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
        obj->level, obj->cost, obj->condition, obj->timer);

    printf_to_char (ch,
        "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
        obj->in_room == NULL ? 0 : obj->in_room->vnum,
        obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
        obj->carried_by == NULL ? "(none)" : PERS_AW(obj->carried_by, ch),
        obj->wear_loc);

    printf_to_char (ch, "Values: %d %d %d %d %d\n\r",
        obj->value[0], obj->value[1], obj->value[2], obj->value[3],
        obj->value[4]);

    /* now give out vital statistics as per identify */
    switch (obj->item_type) {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            printf_to_char (ch, "Level %d spells of:", obj->value[0]);
            if (obj->value[1] >= 0 && obj->value[1] < SKILL_MAX)
                printf_to_char (ch, " '%s'", skill_table[obj->value[1]].name);
            if (obj->value[2] >= 0 && obj->value[2] < SKILL_MAX)
                printf_to_char (ch, " '%s'", skill_table[obj->value[2]].name);
            if (obj->value[3] >= 0 && obj->value[3] < SKILL_MAX)
                printf_to_char (ch, " '%s'", skill_table[obj->value[3]].name);
            if (obj->value[4] >= 0 && obj->value[4] < SKILL_MAX)
                printf_to_char (ch, " '%s'", skill_table[obj->value[4]].name);
            send_to_char (".\n\r", ch);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            printf_to_char (ch, "Has %d(%d) charges of level %d",
                obj->value[1], obj->value[2], obj->value[0]);
            if (obj->value[3] >= 0 && obj->value[3] < SKILL_MAX)
                printf_to_char (ch, " '%s'", skill_table[obj->value[3]].name);
            send_to_char (".\n\r", ch);
            break;

        case ITEM_DRINK_CON:
            printf_to_char (ch, "It holds %s-colored %s.\n\r",
                liq_table[obj->value[2]].color,
                liq_table[obj->value[2]].name);
            break;

        case ITEM_WEAPON:
            printf_to_char (ch, "Weapon type is %s\n\r",
                if_null_str (weapon_get_name (obj->value[0]), "unknown"));

            if (obj->pIndexData->new_format)
                printf_to_char (ch, "Damage is %dd%d (average %d)\n\r",
                    obj->value[1], obj->value[2],
                    (1 + obj->value[2]) * obj->value[1] / 2);
            else
                printf_to_char (ch, "Damage is %d to %d (average %d)\n\r",
                    obj->value[1], obj->value[2],
                    (obj->value[1] + obj->value[2]) / 2);

            printf_to_char (ch, "Damage noun is %s.\n\r",
                (obj->value[3] > 0 && obj->value[3] < ATTACK_MAX)
                    ? attack_table[obj->value[3]].noun : "undefined");

            if (obj->value[4] > 0)
                printf_to_char (ch, "Weapons flags: %s\n\r",
                    weapon_bit_name (obj->value[4]));
            break;

        case ITEM_ARMOR:
            printf_to_char (ch,
                "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
            break;

        case ITEM_CONTAINER:
            printf_to_char (ch,
                "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name (obj->value[1]));

            if (obj->value[4] != 100)
                printf_to_char (ch, "Weight multiplier: %d%%\n\r",
                    obj->value[4]);
            break;
    }

    if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL) {
        EXTRA_DESCR_DATA *ed;

        send_to_char ("Extra description keywords: '", ch);
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
            send_to_char (ed->keyword, ch);
            if (ed->next != NULL)
                send_to_char (" ", ch);
        }
        for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next) {
            send_to_char (ed->keyword, ch);
            if (ed->next != NULL)
                send_to_char (" ", ch);
        }
        send_to_char ("'\n\r", ch);
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        printf_to_char (ch, "Affects %s by %d, level %d",
            affect_apply_name (paf->apply), paf->modifier, paf->level);

        if (paf->duration > -1)
            printf_to_char (ch, ", %d hours.\n\r", paf->duration);
        else
            send_to_char (".\n\r", ch);

        if (paf->bits)
            send_to_char (affect_bit_message (paf->bit_type, paf->bits), ch);
    }

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
            printf_to_char (ch, "Affects %s by %d, level %d.\n\r",
                affect_apply_name (paf->apply), paf->modifier, paf->level);
            if (paf->bits)
                send_to_char (affect_bit_message (paf->bit_type, paf->bits), ch);
        }
    }
}

void do_mstat (CHAR_DATA * ch, char *argument) {
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    BAIL_IF (argument[0] == '\0',
        "Stat whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, argument)) == NULL,
        "They aren't here.\n\r", ch);

    printf_to_char (ch, "Name: %s\n\r", victim->name);

    printf_to_char (ch,
        "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
        IS_NPC (victim) ? victim->pIndexData->vnum : 0,
        IS_NPC (victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
        race_table[victim->race].name,
        IS_NPC (victim) ? victim->group : 0, sex_table[victim->sex].name,
        victim->in_room == NULL ? 0 : victim->in_room->vnum);

    if (IS_NPC (victim))
        printf_to_char (ch, "Count: %d  Killed: %d\n\r",
            victim->pIndexData->count, victim->pIndexData->killed);

    printf_to_char (ch,
        "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
        victim->perm_stat[STAT_STR], char_get_curr_stat (victim, STAT_STR),
        victim->perm_stat[STAT_INT], char_get_curr_stat (victim, STAT_INT),
        victim->perm_stat[STAT_WIS], char_get_curr_stat (victim, STAT_WIS),
        victim->perm_stat[STAT_DEX], char_get_curr_stat (victim, STAT_DEX),
        victim->perm_stat[STAT_CON], char_get_curr_stat (victim, STAT_CON));

    printf_to_char (ch,
        "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
        victim->hit, victim->max_hit, victim->mana, victim->max_mana,
        victim->move, victim->max_move, IS_NPC (ch) ? 0 : victim->practice);

    printf_to_char (ch,
        "Lv: %d  Class: %s  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
        victim->level,
        IS_NPC (victim) ? "mobile" : class_table[victim->class].name,
        victim->alignment, victim->gold, victim->silver, victim->exp);

    printf_to_char (ch,
        "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
        GET_AC (victim, AC_PIERCE), GET_AC (victim, AC_BASH),
        GET_AC (victim, AC_SLASH),  GET_AC (victim, AC_EXOTIC));

    printf_to_char (ch,
        "Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
        GET_HITROLL (victim), GET_DAMROLL (victim), victim->saving_throw,
        size_table[victim->size].name,
        position_table[victim->position].long_name, victim->wimpy);

    if (IS_NPC (victim) && victim->pIndexData->new_format) {
        printf_to_char (ch, "Damage: %dd%d  Message:  %s\n\r",
            victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
            attack_table[victim->dam_type].noun);
    }

    printf_to_char (ch, "Fighting: %s\n\r",
        victim->fighting ? victim->fighting->name : "(none)");

    if (!IS_NPC (victim)) {
        printf_to_char (ch,
            "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
            victim->pcdata->condition[COND_THIRST],
            victim->pcdata->condition[COND_HUNGER],
            victim->pcdata->condition[COND_FULL],
            victim->pcdata->condition[COND_DRUNK]);
    }

    printf_to_char (ch,
        "Carry number: %d  Carry weight: %ld  Material: %s\n\r",
        victim->carry_number, char_get_carry_weight (victim) / 10,
        if_null_str ((char *) material_get_name (victim->material), "unknown"));

    if (!IS_NPC (victim)) {
        printf_to_char (ch,
            "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
            char_get_age (victim),
            (int) (victim->played + current_time - victim->logon) / 3600,
            victim->pcdata->last_level, victim->timer);
    }

    printf_to_char (ch, "Mob: %s\n\r", mob_bit_name (victim->mob));
    printf_to_char (ch, "Plr: %s\n\r", plr_bit_name (victim->plr));

    if (victim->comm)
        printf_to_char (ch, "Comm: %s\n\r",
            comm_bit_name (victim->comm));
    if (IS_NPC (victim) && victim->off_flags)
        printf_to_char (ch, "Offense: %s\n\r",
            off_bit_name (victim->off_flags));
    if (victim->affected_by)
        printf_to_char (ch, "Affected by: %s\n\r",
            affect_bit_name (victim->affected_by));
    if (victim->imm_flags)
        printf_to_char (ch, "Immune to: %s\n\r",
            res_bit_name (victim->imm_flags));
    if (victim->res_flags)
        printf_to_char (ch, "Resist to: %s\n\r",
            res_bit_name (victim->res_flags));
    if (victim->vuln_flags)
        printf_to_char (ch, "Vulnerable to: %s\n\r",
            res_bit_name (victim->vuln_flags));

    printf_to_char (ch, "Form: %s\n\rParts: %s\n\r",
        form_bit_name (victim->form), part_bit_name (victim->parts));

    printf_to_char (ch, "Master: %s  Leader: %s  Pet: %s\n\r",
        victim->master ? victim->master->name : "(none)",
        victim->leader ? victim->leader->name : "(none)",
        victim->pet ? victim->pet->name : "(none)");

    /* OLC */
    if (!IS_NPC (victim))
        printf_to_char (ch, "Security: %d.\n\r", victim->pcdata->security);

    printf_to_char (ch, "Short description: %s\n\rLong  description: %s",
        victim->short_descr,
        victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r");

    if (IS_NPC (victim) && victim->spec_fun != 0)
        printf_to_char (ch, "Mobile has special procedure %s.\n\r",
            spec_function_name (victim->spec_fun));

    for (paf = victim->affected; paf != NULL; paf = paf->next) {
        printf_to_char (ch,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
            skill_table[(int) paf->type].name, affect_apply_name (paf->apply),
            paf->modifier, paf->duration, affect_bit_name (paf->bits),
            paf->level);
    }
}

void do_wiznet (CHAR_DATA * ch, char *argument) {
    const WIZNET_TYPE *flag;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (argument[0] == '\0') {
        do_flag_toggle (ch, FALSE, &(ch->wiznet), WIZ_ON,
            "Signing off of Wiznet.\n\r",
            "Welcome to Wiznet!\n\r");
        return;
    }
    if (!str_prefix (argument, "on")) {
        send_to_char ("Welcome to Wiznet!\n\r", ch);
        SET_BIT (ch->wiznet, WIZ_ON);
        return;
    }
    if (!str_prefix (argument, "off")) {
        send_to_char ("Signing off of Wiznet.\n\r", ch);
        REMOVE_BIT (ch->wiznet, WIZ_ON);
        return;
    }

    /* show wiznet status */
    if (!str_prefix (argument, "status")) {
        buf[0] = '\0';

        if (!IS_SET (ch->wiznet, WIZ_ON))
            strcat (buf, "off ");

        for (i = 0; wiznet_table[i].name != NULL; i++) {
            if (IS_SET (ch->wiznet, wiznet_table[i].bit)) {
                strcat (buf, wiznet_table[i].name);
                strcat (buf, " ");
            }
        }
        strcat (buf, "\n\r");

        send_to_char ("Wiznet status:\n\r", ch);
        send_to_char (buf, ch);
        return;
    }

    if (!str_prefix (argument, "show"))
        /* list of all wiznet options */
    {
        buf[0] = '\0';
        for (i = 0; wiznet_table[i].name != NULL; i++) {
            if (wiznet_table[i].level <= char_get_trust (ch)) {
                strcat (buf, wiznet_table[i].name);
                strcat (buf, " ");
            }
        }

        strcat (buf, "\n\r");

        send_to_char ("Wiznet options available to you are:\n\r", ch);
        send_to_char (buf, ch);
        return;
    }

    flag = wiznet_get_by_name (argument);
    BAIL_IF (flag == NULL || char_get_trust (ch) < flag->level,
        "No such option.\n\r", ch);

    if (IS_SET (ch->wiznet, flag->bit)) {
        printf_to_char (ch, "You will no longer see %s on wiznet.\n\r",
            flag->name);
        REMOVE_BIT (ch->wiznet, flag->bit);
        return;
    }
    else {
        printf_to_char (ch, "You will now see %s on wiznet.\n\r", flag->name);
        SET_BIT (ch->wiznet, flag->bit);
        return;
    }
}

void do_immtalk (CHAR_DATA * ch, char *argument) {
    DESCRIPTOR_DATA *d;

    if (do_comm_toggle_channel_if_blank (ch, argument, COMM_NOWIZ,
            "{iImmortal channel is now ON{k\n\r",
            "{iImmortal channel is now OFF{k\n\r"))
        return;
    REMOVE_BIT (ch->comm, COMM_NOWIZ);

    act_new ("{i[{I$n{i]: $t{x", ch, argument, NULL, TO_CHAR, POS_DEAD);
    for (d = descriptor_list; d != NULL; d = d->next) {
        CHAR_DATA *victim = CH(d);
        if (victim == ch || d->connected != CON_PLAYING)
            continue;
        if (IS_SET (victim->comm, COMM_NOWIZ))
            continue;
        if (!IS_IMMORTAL (d->character))
            continue;
        act_new ("{i[{I$n{i]: $t{x", ch, argument, victim, TO_VICT, POS_DEAD);
    }
}

/* (from act_info.c): RT Commands to replace news, motd, imotd, etc from ROM */
void do_imotd (CHAR_DATA * ch, char *argument)
    { do_function (ch, &do_help, "imotd"); }

void do_smote (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    BAIL_IF (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE),
        "You can't show your emotions.\n\r", ch);
    BAIL_IF (argument[0] == '\0',
        "Emote what?\n\r", ch);
    BAIL_IF (strstr (argument, ch->name) == NULL,
        "You must include your name in an smote.\n\r", ch);

    send_to_char (argument, ch);
    send_to_char ("\n\r", ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
        if (vch->desc == NULL || vch == ch)
            continue;

        if ((letter = strstr (argument, vch->name)) == NULL) {
            send_to_char (argument, vch);
            send_to_char ("\n\r", vch);
            continue;
        }

        strcpy (temp, argument);
        temp[strlen (argument) - strlen (letter)] = '\0';
        last[0] = '\0';
        name = vch->name;

        for (; *letter != '\0'; letter++) {
            if (*letter == '\'' && matches == strlen (vch->name)) {
                strcat (temp, "r");
                continue;
            }
            if (*letter == 's' && matches == strlen (vch->name)) {
                matches = 0;
                continue;
            }
            if (matches == strlen (vch->name))
                matches = 0;

            if (*letter == *name) {
                matches++;
                name++;
                if (matches == strlen (vch->name)) {
                    strcat (temp, "you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat (last, letter, 1);
                continue;
            }

            matches = 0;
            strcat (temp, last);
            strncat (temp, letter, 1);
            last[0] = '\0';
            name = vch->name;
        }

        send_to_char (temp, vch);
        send_to_char ("\n\r", vch);
    }
}

/* prefix command: it will put the string typed on each line typed */
void do_prefi (CHAR_DATA * ch, char *argument) {
    send_to_char ("You cannot abbreviate the prefix command.\r\n", ch);
}

void do_prefix (CHAR_DATA * ch, char *argument) {
    if (argument[0] == '\0') {
        BAIL_IF (ch->prefix == NULL || ch->prefix[0] == '\0',
            "You have no prefix to clear.\r\n", ch);
        send_to_char ("Prefix removed.\r\n", ch);
        str_replace_dup (&(ch->prefix), "");
        return;
    }

    if (ch->prefix && ch->prefix[0] != '\0')
        printf_to_char (ch, "Prefix changed to %s.\r\n", argument);
    else
        printf_to_char (ch, "Prefix set to %s.\r\n", argument);
    str_replace_dup (&(ch->prefix), argument);
}

/* Displays the source code of a given MOBprogram
 * Syntax: mpdump [vnum] */
void do_mpdump (CHAR_DATA * ch, char *argument) {
    char buf[MAX_INPUT_LENGTH];
    MPROG_CODE *mprg;

    one_argument (argument, buf);
    if ((mprg = get_mprog_index (atoi (buf))) == NULL) {
        send_to_char ("No such MOBprogram.\n\r", ch);
        return;
    }
    page_to_char (mprg->code, ch);
}

/* Displays MOBprogram triggers of a mobile
 * Syntax: mpstat [name] */
void do_mpstat (CHAR_DATA * ch, char *argument) {
    MPROG_LIST *mprg;
    CHAR_DATA *victim;
    int i;

    BAIL_IF (argument[0] == '\0',
        "Mpstat whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, argument)) == NULL,
        "No such creature.\n\r", ch);
    BAIL_IF (!IS_NPC (victim),
        "That is not a mobile.\n\r", ch);

    printf_to_char (ch, "Mobile #%-6d [%s]\n\r",
        victim->pIndexData->vnum, victim->short_descr);

    printf_to_char (ch, "Delay   %-6d [%s]\n\r",
        victim->mprog_delay,
        victim->mprog_target == NULL
            ? "No target" : victim->mprog_target->name);

    BAIL_IF (!victim->pIndexData->mprog_flags,
        "[No programs set]\n\r", ch);

    for (i = 1, mprg = victim->pIndexData->mprogs; mprg != NULL;
         mprg = mprg->next, i++)
    {
        printf_to_char (ch,
            "[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n\r",
            i, mprog_type_to_name (mprg->trig_type),
            mprg->vnum, mprg->trig_phrase);
    }
}
