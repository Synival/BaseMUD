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

#include "wiz_im.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */
/* TODO: do_stat() and its derivatives are GIGANTIC. deflate somehow? */

void do_wizhelp (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
        if (cmd_table[cmd].level >= LEVEL_HERO &&
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

void do_holylight (CHAR_DATA * ch, char *argument) {
    if (IS_NPC (ch))
        return;
    if (IS_SET (ch->act, PLR_HOLYLIGHT)) {
        REMOVE_BIT (ch->act, PLR_HOLYLIGHT);
        send_to_char ("Holy light mode off.\n\r", ch);
    }
    else {
        SET_BIT (ch->act, PLR_HOLYLIGHT);
        send_to_char ("Holy light mode on.\n\r", ch);
    }
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
    }
    /* do the level thing */
    else {
        level = atoi (arg);
        if (level < 2 || level > char_get_trust (ch)) {
            send_to_char ("Incog level must be between 2 and your level.\n\r", ch);
            return;
        }
        else {
            ch->reply = NULL;
            ch->incog_level = level;
            send_to_char ("You cloak your presence.\n\r", ch);
            act ("$n cloaks $s presence.", ch, NULL, NULL, TO_NOTCHAR);
        }
    }
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
    }
    /* do the level thing */
    else {
        level = atoi (arg);
        if (level < 2 || level > char_get_trust (ch)) {
            send_to_char ("Invis level must be between 2 and your level.\n\r", ch);
            return;
        }
        else {
            ch->reply = NULL;
            ch->invis_level = level;
            send_to_char ("You slowly vanish into thin air.\n\r", ch);
            act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_NOTCHAR);
        }
    }
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
            if (d->character != NULL && d->connected == CON_PLAYING
                && d->character->in_room != NULL
                && char_can_see_anywhere (ch, d->character)
                && char_can_see_room (ch, d->character->in_room))
            {
                victim = d->character;
                count++;
                if (d->original != NULL)
                    sprintf (buf,
                             "%3d) %s (in the body of %s) is in %s [%d]\n\r",
                             count, d->original->name, victim->short_descr,
                             victim->in_room->name, victim->in_room->vnum);
                else
                    sprintf (buf, "%3d) %s is in %s [%d]\n\r", count,
                             victim->name, victim->in_room->name,
                             victim->in_room->vnum);
                add_buf (buffer, buf);
            }
        }

        page_to_char (buf_string (buffer), ch);
        buf_free (buffer);
        return;
    }

    found = FALSE;
    buffer = buf_new ();
    for (victim = char_list; victim != NULL; victim = victim->next) {
        if (victim->in_room != NULL && is_name (argument, victim->name)) {
            found = TRUE;
            count++;
            sprintf (buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                     IS_NPC (victim) ? victim->pIndexData->vnum : 0,
                     IS_NPC (victim) ? victim->short_descr : victim->name,
                     victim->in_room->vnum, victim->in_room->name);
            add_buf (buffer, buf);
        }
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
    if (argument[0] == '\0') {
        send_to_char ("Find what?\n\r", ch);
        return;
    }

    for (obj = object_list; obj != NULL; obj = obj->next) {
        if (!char_can_see_obj (ch, obj) || !is_name (argument, obj->name)
            || ch->level < obj->level)
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
    if (!str_cmp (arg, "room")) {
        do_function (ch, &do_rstat, string);
        return;
    }
    if (!str_cmp (arg, "obj")) {
        do_function (ch, &do_ostat, string);
        return;
    }
    if (!str_cmp (arg, "char") || !str_cmp (arg, "mob")) {
        do_function (ch, &do_mstat, string);
        return;
    }

    /* do it the old way */
    if ((obj = find_obj_world (ch, argument)) != NULL) {
        do_function (ch, &do_ostat, argument);
        return;
    }
    if ((victim = find_char_world (ch, argument)) != NULL) {
        do_function (ch, &do_mstat, argument);
        return;
    }
    if ((location = find_location (ch, argument)) != NULL) {
        do_function (ch, &do_rstat, argument);
        return;
    }
    send_to_char ("Nothing by that name found anywhere.\n\r", ch);
}

void do_rstat (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument (argument, arg);
    location = (arg[0] == '\0') ? ch->in_room : find_location (ch, arg);
    if (location == NULL) {
        send_to_char ("No such location.\n\r", ch);
        return;
    }

    if (!room_is_owner (location, ch) && ch->in_room != location
        && room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        send_to_char ("That room is private right now.\n\r", ch);
        return;
    }

    sprintf (buf, "Name: '%s'\n\rArea: '%s'\n\r",
             location->name, location->area->title);
    send_to_char (buf, ch);

    sprintf (buf,
             "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
             location->vnum,
             location->sector_type,
             location->light, location->heal_rate, location->mana_rate);
    send_to_char (buf, ch);

    sprintf (buf,
             "Room flags: %ld.\n\rDescription:\n\r%s",
             location->room_flags, location->description);
    send_to_char (buf, ch);

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
        send_to_char (" ", ch);
        one_argument (rch->name, buf);
        send_to_char (buf, ch);
    }

    send_to_char (".\n\rObjects:   ", ch);
    for (obj = location->contents; obj; obj = obj->next_content) {
        send_to_char (" ", ch);
        one_argument (obj->name, buf);
        send_to_char (buf, ch);
    }
    send_to_char (".\n\r", ch);

    for (door = 0; door <= 5; door++) {
        EXIT_DATA *pexit;
        if ((pexit = location->exit[door]) != NULL) {
            sprintf (buf,
                     "Door: %d.  To: %d.  Key: %d.  Exit flags: %ld.\n\r"
                     "Keyword: '%s'.  Description: %s",
                     door,
                     (pexit->to_room ==
                      NULL ? -1 : pexit->to_room->vnum), pexit->key,
                     pexit->exit_flags, pexit->keyword,
                     pexit->description[0] !=
                     '\0' ? pexit->description : "(none).\n\r");
            send_to_char (buf, ch);
        }
    }
}

void do_ostat (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Stat what?\n\r", ch);
        return;
    }
    if ((obj = find_obj_world (ch, argument)) == NULL) {
        send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
        return;
    }

    sprintf (buf, "Name(s): %s\n\r", obj->name);
    send_to_char (buf, ch);

    sprintf (buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
             obj->pIndexData->vnum,
             obj->pIndexData->new_format ? "new" : "old",
             item_get_name (obj->item_type), obj->pIndexData->reset_num);
    send_to_char (buf, ch);

    sprintf (buf, "Short description: %s\n\rLong description: %s\n\r",
             obj->short_descr, obj->description);
    send_to_char (buf, ch);

    sprintf (buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
             wear_bit_name (obj->wear_flags),
             extra_bit_name (obj->extra_flags));
    send_to_char (buf, ch);

    sprintf (buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)  Material: %s\n\r",
             1, obj_get_carry_number (obj),
             obj->weight, obj_get_weight (obj), obj_get_true_weight (obj),
             if_null_str ((char *) material_get_name (obj->material), "unknown"));
    send_to_char (buf, ch);

    sprintf (buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
             obj->level, obj->cost, obj->condition, obj->timer);
    send_to_char (buf, ch);

    sprintf (buf,
             "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
             obj->in_room == NULL ? 0 : obj->in_room->vnum,
             obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
             obj->carried_by == NULL ? "(none)" :
             char_can_see_anywhere (ch, obj->carried_by) ? obj->carried_by->name
             : "someone", obj->wear_loc);
    send_to_char (buf, ch);

    sprintf (buf, "Values: %d %d %d %d %d\n\r",
             obj->value[0], obj->value[1], obj->value[2], obj->value[3],
             obj->value[4]);
    send_to_char (buf, ch);

    /* now give out vital statistics as per identify */
    switch (obj->item_type) {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf (buf, "Level %d spells of:", obj->value[0]);
            send_to_char (buf, ch);

            if (obj->value[1] >= 0 && obj->value[1] < SKILL_MAX) {
                send_to_char (" '", ch);
                send_to_char (skill_table[obj->value[1]].name, ch);
                send_to_char ("'", ch);
            }
            if (obj->value[2] >= 0 && obj->value[2] < SKILL_MAX) {
                send_to_char (" '", ch);
                send_to_char (skill_table[obj->value[2]].name, ch);
                send_to_char ("'", ch);
            }
            if (obj->value[3] >= 0 && obj->value[3] < SKILL_MAX) {
                send_to_char (" '", ch);
                send_to_char (skill_table[obj->value[3]].name, ch);
                send_to_char ("'", ch);
            }
            if (obj->value[4] >= 0 && obj->value[4] < SKILL_MAX) {
                send_to_char (" '", ch);
                send_to_char (skill_table[obj->value[4]].name, ch);
                send_to_char ("'", ch);
            }

            send_to_char (".\n\r", ch);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf (buf, "Has %d(%d) charges of level %d",
                     obj->value[1], obj->value[2], obj->value[0]);
            send_to_char (buf, ch);

            if (obj->value[3] >= 0 && obj->value[3] < SKILL_MAX) {
                send_to_char (" '", ch);
                send_to_char (skill_table[obj->value[3]].name, ch);
                send_to_char ("'", ch);
            }
            send_to_char (".\n\r", ch);
            break;

        case ITEM_DRINK_CON:
            sprintf (buf, "It holds %s-colored %s.\n\r",
                     liq_table[obj->value[2]].color,
                     liq_table[obj->value[2]].name);
            send_to_char (buf, ch);
            break;

        case ITEM_WEAPON:
            send_to_char ("Weapon type is ", ch);
            switch (obj->value[0]) {
                case (WEAPON_EXOTIC):
                    send_to_char ("exotic\n\r", ch);
                    break;
                case (WEAPON_SWORD):
                    send_to_char ("sword\n\r", ch);
                    break;
                case (WEAPON_DAGGER):
                    send_to_char ("dagger\n\r", ch);
                    break;
                case (WEAPON_SPEAR):
                    send_to_char ("spear/staff\n\r", ch);
                    break;
                case (WEAPON_MACE):
                    send_to_char ("mace/club\n\r", ch);
                    break;
                case (WEAPON_AXE):
                    send_to_char ("axe\n\r", ch);
                    break;
                case (WEAPON_FLAIL):
                    send_to_char ("flail\n\r", ch);
                    break;
                case (WEAPON_WHIP):
                    send_to_char ("whip\n\r", ch);
                    break;
                case (WEAPON_POLEARM):
                    send_to_char ("polearm\n\r", ch);
                    break;
                default:
                    send_to_char ("unknown\n\r", ch);
                    break;
            }
            if (obj->pIndexData->new_format)
                sprintf (buf, "Damage is %dd%d (average %d)\n\r",
                         obj->value[1], obj->value[2],
                         (1 + obj->value[2]) * obj->value[1] / 2);
            else
                sprintf (buf, "Damage is %d to %d (average %d)\n\r",
                         obj->value[1], obj->value[2],
                         (obj->value[1] + obj->value[2]) / 2);
            send_to_char (buf, ch);

            sprintf (buf, "Damage noun is %s.\n\r",
                     (obj->value[3] > 0
                      && obj->value[3] <
                      ATTACK_MAX) ? attack_table[obj->value[3]].noun :
                     "undefined");
            send_to_char (buf, ch);

            if (obj->value[4]) { /* weapon flags */
                sprintf (buf, "Weapons flags: %s\n\r",
                         weapon_bit_name (obj->value[4]));
                send_to_char (buf, ch);
            }
            break;

        case ITEM_ARMOR:
            sprintf (buf, "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                     obj->value[0], obj->value[1], obj->value[2],
                     obj->value[3]);
            send_to_char (buf, ch);
            break;

        case ITEM_CONTAINER:
            sprintf (buf, "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                     obj->value[0], obj->value[3],
                     cont_bit_name (obj->value[1]));
            send_to_char (buf, ch);
            if (obj->value[4] != 100) {
                sprintf (buf, "Weight multiplier: %d%%\n\r", obj->value[4]);
                send_to_char (buf, ch);
            }
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
        sprintf (buf, "Affects %s by %d, level %d",
                 affect_apply_name (paf->apply), paf->modifier, paf->level);
        send_to_char (buf, ch);
        if (paf->duration > -1)
            sprintf (buf, ", %d hours.\n\r", paf->duration);
        else
            sprintf (buf, ".\n\r");
        send_to_char (buf, ch);
        if (paf->bitvector) {
            switch (paf->bit_type) {
                case TO_AFFECTS:
                    sprintf (buf, "Adds %s affect.\n",
                             affect_bit_name (paf->bitvector));
                    break;
                case TO_WEAPON:
                    sprintf (buf, "Adds %s weapon flags.\n",
                             weapon_bit_name (paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf (buf, "Adds %s object flag.\n",
                             extra_bit_name (paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf (buf, "Adds immunity to %s.\n",
                             res_bit_name (paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf (buf, "Adds resistance to %s.\n\r",
                             res_bit_name (paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf (buf, "Adds vulnerability to %s.\n\r",
                             res_bit_name (paf->bitvector));
                    break;
                default:
                    sprintf (buf, "Unknown bit %d: %ld\n\r",
                             paf->bit_type, paf->bitvector);
                    break;
            }
            send_to_char (buf, ch);
        }
    }

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
            sprintf (buf, "Affects %s by %d, level %d.\n\r",
                     affect_apply_name (paf->apply), paf->modifier,
                     paf->level);
            send_to_char (buf, ch);
            if (paf->bitvector) {
                switch (paf->bit_type) {
                    case TO_AFFECTS:
                        sprintf (buf, "Adds %s affect.\n",
                                 affect_bit_name (paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf (buf, "Adds %s object flag.\n",
                                 extra_bit_name (paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf (buf, "Adds immunity to %s.\n",
                                 res_bit_name (paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf (buf, "Adds resistance to %s.\n\r",
                                 res_bit_name (paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf (buf, "Adds vulnerability to %s.\n\r",
                                 res_bit_name (paf->bitvector));
                        break;
                    default:
                        sprintf (buf, "Unknown bit %d: %ld\n\r",
                                 paf->bit_type, paf->bitvector);
                        break;
                }
                send_to_char (buf, ch);
            }
        }
    }
}

void do_mstat (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Stat whom?\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, argument)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    sprintf (buf, "Name: %s\n\r", victim->name);
    send_to_char (buf, ch);

    sprintf (buf, "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
             IS_NPC (victim) ? victim->pIndexData->vnum : 0,
             IS_NPC (victim) ? victim->
             pIndexData->new_format ? "new" : "old" : "pc",
             race_table[victim->race].name,
             IS_NPC (victim) ? victim->group : 0, sex_table[victim->sex].name,
             victim->in_room == NULL ? 0 : victim->in_room->vnum);
    send_to_char (buf, ch);

    if (IS_NPC (victim)) {
        sprintf (buf, "Count: %d  Killed: %d\n\r",
                 victim->pIndexData->count, victim->pIndexData->killed);
        send_to_char (buf, ch);
    }

    sprintf (buf, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
             victim->perm_stat[STAT_STR],
             char_get_curr_stat (victim, STAT_STR),
             victim->perm_stat[STAT_INT],
             char_get_curr_stat (victim, STAT_INT),
             victim->perm_stat[STAT_WIS],
             char_get_curr_stat (victim, STAT_WIS),
             victim->perm_stat[STAT_DEX],
             char_get_curr_stat (victim, STAT_DEX),
             victim->perm_stat[STAT_CON], char_get_curr_stat (victim, STAT_CON));
    send_to_char (buf, ch);

    sprintf (buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
             victim->hit, victim->max_hit,
             victim->mana, victim->max_mana,
             victim->move, victim->max_move,
             IS_NPC (ch) ? 0 : victim->practice);
    send_to_char (buf, ch);

    sprintf (buf, "Lv: %d  Class: %s  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
             victim->level,
             IS_NPC (victim) ? "mobile" : class_table[victim->class].name,
             victim->alignment, victim->gold, victim->silver, victim->exp);
    send_to_char (buf, ch);

    sprintf (buf, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
             GET_AC (victim, AC_PIERCE), GET_AC (victim, AC_BASH),
             GET_AC (victim, AC_SLASH), GET_AC (victim, AC_EXOTIC));
    send_to_char (buf, ch);

    sprintf (buf, "Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
             GET_HITROLL (victim), GET_DAMROLL (victim), victim->saving_throw,
             size_table[victim->size].name,
             position_table[victim->position].long_name, victim->wimpy);
    send_to_char (buf, ch);

    if (IS_NPC (victim) && victim->pIndexData->new_format) {
        sprintf (buf, "Damage: %dd%d  Message:  %s\n\r",
                 victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
                 attack_table[victim->dam_type].noun);
        send_to_char (buf, ch);
    }
    sprintf (buf, "Fighting: %s\n\r",
             victim->fighting ? victim->fighting->name : "(none)");
    send_to_char (buf, ch);

    if (!IS_NPC (victim)) {
        sprintf (buf, "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
                 victim->pcdata->condition[COND_THIRST],
                 victim->pcdata->condition[COND_HUNGER],
                 victim->pcdata->condition[COND_FULL],
                 victim->pcdata->condition[COND_DRUNK]);
        send_to_char (buf, ch);
    }

    sprintf (buf, "Carry number: %d  Carry weight: %ld  Material: %s\n\r",
             victim->carry_number, char_get_carry_weight (victim) / 10,
             if_null_str ((char *) material_get_name (victim->material),
                "unknown"));
    send_to_char (buf, ch);


    if (!IS_NPC (victim)) {
        sprintf (buf, "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
                 char_get_age (victim),
                 (int) (victim->played + current_time - victim->logon) / 3600,
                 victim->pcdata->last_level, victim->timer);
        send_to_char (buf, ch);
    }

    sprintf (buf, "Act: %s\n\r", act_bit_name (victim->act));
    send_to_char (buf, ch);

    if (victim->comm) {
        sprintf (buf, "Comm: %s\n\r", comm_bit_name (victim->comm));
        send_to_char (buf, ch);
    }
    if (IS_NPC (victim) && victim->off_flags) {
        sprintf (buf, "Offense: %s\n\r", off_bit_name (victim->off_flags));
        send_to_char (buf, ch);
    }
    if (victim->affected_by) {
        sprintf (buf, "Affected by: %s\n\r", affect_bit_name (victim->affected_by));
        send_to_char (buf, ch);
    }
    if (victim->imm_flags) {
        sprintf (buf, "Immune to: %s\n\r", res_bit_name (victim->imm_flags));
        send_to_char (buf, ch);
    }
    if (victim->res_flags) {
        sprintf (buf, "Resist to: %s\n\r", res_bit_name (victim->res_flags));
        send_to_char (buf, ch);
    }
    if (victim->vuln_flags) {
        sprintf (buf, "Vulnerable to: %s\n\r", res_bit_name (victim->vuln_flags));
        send_to_char (buf, ch);
    }

    sprintf (buf, "Form: %s\n\rParts: %s\n\r",
             form_bit_name (victim->form), part_bit_name (victim->parts));
    send_to_char (buf, ch);

    sprintf (buf, "Master: %s  Leader: %s  Pet: %s\n\r",
             victim->master ? victim->master->name : "(none)",
             victim->leader ? victim->leader->name : "(none)",
             victim->pet ? victim->pet->name : "(none)");
    send_to_char (buf, ch);

    if (!IS_NPC (victim)) {
        sprintf (buf, "Security: %d.\n\r", victim->pcdata->security);    /* OLC */
        send_to_char (buf, ch);    /* OLC */
    }

    sprintf (buf, "Short description: %s\n\rLong  description: %s",
             victim->short_descr,
             victim->long_descr[0] !=
             '\0' ? victim->long_descr : "(none)\n\r");
    send_to_char (buf, ch);

    if (IS_NPC (victim) && victim->spec_fun != 0) {
        sprintf (buf, "Mobile has special procedure %s.\n\r",
                 spec_function_name (victim->spec_fun));
        send_to_char (buf, ch);
    }

    for (paf = victim->affected; paf != NULL; paf = paf->next) {
        sprintf (buf, "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
                 skill_table[(int) paf->type].name,
                 affect_apply_name (paf->apply),
                 paf->modifier,
                 paf->duration, affect_bit_name (paf->bitvector), paf->level);
        send_to_char (buf, ch);
    }
}

void do_wiznet (CHAR_DATA * ch, char *argument) {
    const WIZNET_TYPE *flag;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (argument[0] == '\0') {
        do_flag_toggle(ch, FALSE, &(ch->wiznet), WIZ_ON,
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
    if (flag == NULL || char_get_trust (ch) < flag->level) {
        send_to_char ("No such option.\n\r", ch);
        return;
    }
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

    if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE)) {
        send_to_char ("You can't show your emotions.\n\r", ch);
        return;
    }
    if (argument[0] == '\0') {
        send_to_char ("Emote what?\n\r", ch);
        return;
    }
    if (strstr (argument, ch->name) == NULL) {
        send_to_char ("You must include your name in an smote.\n\r", ch);
        return;
    }

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
    char buf[MAX_INPUT_LENGTH];
    if (argument[0] == '\0') {
        if (ch->prefix[0] == '\0') {
            send_to_char ("You have no prefix to clear.\r\n", ch);
            return;
        }
        send_to_char ("Prefix removed.\r\n", ch);
        str_free (ch->prefix);
        ch->prefix = str_dup ("");
        return;
    }
    if (ch->prefix[0] != '\0') {
        sprintf (buf, "Prefix changed to %s.\r\n", argument);
        str_free (ch->prefix);
    }
    else
        sprintf (buf, "Prefix set to %s.\r\n", argument);
    ch->prefix = str_dup (argument);
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
    char arg[MAX_STRING_LENGTH];
    MPROG_LIST *mprg;
    CHAR_DATA *victim;
    int i;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Mpstat whom?\n\r", ch);
        return;
    }

    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("No such creature.\n\r", ch);
        return;
    }
    if (!IS_NPC (victim)) {
        send_to_char ("That is not a mobile.\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("No such creature visible.\n\r", ch);
        return;
    }

    sprintf (arg, "Mobile #%-6d [%s]\n\r",
             victim->pIndexData->vnum, victim->short_descr);
    send_to_char (arg, ch);

    sprintf (arg, "Delay   %-6d [%s]\n\r",
             victim->mprog_delay,
             victim->mprog_target == NULL
             ? "No target" : victim->mprog_target->name);
    send_to_char (arg, ch);

    if (!victim->pIndexData->mprog_flags) {
        send_to_char ("[No programs set]\n\r", ch);
        return;
    }

    for (i = 0, mprg = victim->pIndexData->mprogs; mprg != NULL;
         mprg = mprg->next)
    {
        sprintf (arg, "[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n\r",
                 ++i,
                 mprog_type_to_name (mprg->trig_type),
                 mprg->vnum, mprg->trig_phrase);
        send_to_char (arg, ch);
    }
}
