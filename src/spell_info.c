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

#include <stdlib.h>

#include "magic.h"
#include "comm.h"
#include "utils.h"
#include "lookup.h"
#include "chars.h"
#include "recycle.h"
#include "db.h"
#include "interp.h"
#include "act_info.h"
#include "affects.h"

#include "spell_info.h"

/* TODO: move lore messages in spell_identify_know_message() to table */
/* TODO: merge a lot of spell_identity() with do_ostat(). */

DEFINE_SPELL_FUN (spell_detect_poison) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    BAIL_IF (!(obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD),
        "It doesn't look poisoned.\n\r", ch);
    send_to_char ((obj->value[3] != 0)
        ? "You smell poisonous fumes.\n\r"
        : "It looks delicious.\n\r", ch);
}

DEFINE_SPELL_FUN (spell_identify) {
    spell_identify_perform (ch, (OBJ_DATA *) vo, 101);
}

long int spell_identify_seed (CHAR_DATA * ch, OBJ_DATA * obj) {
    long int next_seed = random();
    unsigned long hash;
    char *name1 = ch->name;
    char *name2 = obj->name;
    int c;

    /* Simple code to generate a random seed based on a string, courtesy of
     * stack overflow. There's probably a better way to do this... */
    hash = 5381;
    while ((c = *name1++) != '\0')
        hash = ((hash << 5) + hash) + c;
    while ((c = *name2++) != '\0')
        hash = ((hash << 5) + hash) + c;
    if (obj->pIndexData)
        hash += obj->pIndexData->vnum;

    srandom (hash);
    return next_seed;
}

int spell_identify_know_check (CHAR_DATA * ch, OBJ_DATA * obj, int pos,
    int skill, int *know_count)
{
    int chance, success;

    chance = number_percent();
    success = (skill >= 100) ? 1 : ((skill >= chance) ? 1 : 0);
    if (know_count)
        *know_count += success;

    return success;
}

void spell_identify_perform (CHAR_DATA * ch, OBJ_DATA * obj, int power) {
    long int next_seed = spell_identify_seed (ch, obj);
    spell_identify_perform_seeded (ch, obj, power);
    srandom (next_seed);
}

const char *spell_identify_know_message (int percent) {
    #define KN(p, m) \
        if (percent < p) return m

         KN (  1, "You know absolutely nothing about this object.\n\r");
    else KN ( 10, "You know next to nothing about this object.\n\r");
    else KN ( 20, "You've barely even heard about this object.\n\r");
    else KN ( 30, "You're very unfamiliar with this object.\n\r");
    else KN ( 40, "You're mostly unfamiliar with this object.\n\r");
    else KN ( 50, "You're a bit unfamiliar with the object.\n\r");
    else KN ( 60, "You know a little bit about this object.\n\r");
    else KN ( 70, "You know a few things about this object.\n\r");
    else KN ( 80, "You know a lot about this object.\n\r");
    else KN ( 90, "You know quite a lot about this object.\n\r");
    else KN (100, "You know nearly everything about this object.\n\r");
    else return   "You know absolutely everything about this object.\n\r";
}

void spell_identify_perform_seeded (CHAR_DATA * ch, OBJ_DATA * obj, int power) {
    int know_pos = 0;
    int know_count = 0;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    #define KNOW_CHECK() \
        (spell_identify_know_check(ch, obj, know_pos++, power, &know_count))

    printf_to_char (ch, "Object '%s' (%s):\n\r", obj->short_descr, obj->name);
    if (KNOW_CHECK())
        printf_to_char(ch, "Type: %s\n\r", item_get_name (obj->item_type));
    if (KNOW_CHECK())
        printf_to_char(ch, "Extra flags: %s\n\r", extra_bit_name (obj->extra_flags));
    if (KNOW_CHECK())
        printf_to_char(ch, "Weight: %d\n\r", obj->weight / 10);
    if (KNOW_CHECK())
        printf_to_char(ch, "Value: %d\n\r", obj->cost);
    if (KNOW_CHECK())
        printf_to_char(ch, "Level: %d\n\r", obj->level);

    switch (obj->item_type) {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL: {
            int i;
            char level_str[8];

            snprintf (level_str, sizeof(level_str),
                KNOW_CHECK() ? "%d" : "???", obj->value[0]);
            printf_to_char (ch, "Level %s spells of:", level_str);

            for (i = 1; i <= 4; i++) {
                if (!KNOW_CHECK())
                    continue;
                if (obj->value[i] >= 0 && obj->value[i] < SKILL_MAX)
                    printf_to_char (ch, " '%s'",
                        skill_table[obj->value[i]].name);
            }
            send_to_char (".\n\r", ch);
            break;
        }

        case ITEM_WAND:
        case ITEM_STAFF: {
            int know_spell;
            char charges_str[8], level_str[8];
            snprintf (charges_str, sizeof(charges_str),
                KNOW_CHECK() ? "%d" : "???", obj->value[2]);
            printf_to_char (ch, "Has %s charges of", charges_str);

            know_spell = (KNOW_CHECK()
                && obj->value[3] >= 0 && obj->value[3] < SKILL_MAX);
            snprintf (level_str, sizeof(level_str),
                KNOW_CHECK() ? "%d" : "???", obj->value[0]);
            if (know_spell)
                printf_to_char (ch, " level %s '%s'", level_str,
                    skill_table[obj->value[3]].name);
            else
                printf_to_char (ch, " a level %s ability", level_str);

            send_to_char (".\n\r", ch);
            break;
        }

        case ITEM_DRINK_CON:
            if (!KNOW_CHECK()) {
                printf_to_char (ch, "It holds a %s-colored liquid.\n\r",
                    liq_table[obj->value[2]].color);
            }
            else {
                printf_to_char (ch, "It holds %s-colored %s.\n\r",
                    liq_table[obj->value[2]].color,
                    liq_table[obj->value[2]].name);
            }
            break;

        case ITEM_CONTAINER:
            if (KNOW_CHECK())
                printf_to_char (ch, "Capacity: %d\n\r", obj->value[0]);
            if (KNOW_CHECK())
                printf_to_char (ch, "Maximum weight: %d\n\r", obj->value[3]);
            if (KNOW_CHECK())
                printf_to_char (ch, "Flags: %s\n\r", cont_bit_name(obj->value[1]));
            if (KNOW_CHECK() && obj->value[4] != 100) {
                sprintf (buf, "Weight multiplier: %d%%\n\r", obj->value[4]);
                send_to_char (buf, ch);
            }
            break;

        case ITEM_WEAPON:
            if (KNOW_CHECK()) {
                const FLAG_TYPE *wtype = flag_get(obj->value[0], weapon_types);
                printf_to_char (ch, "Weapon type is %s.\n\r",
                    (wtype == NULL) ? "unknown" : wtype->name);
            }
            if (KNOW_CHECK()) {
                if (obj->pIndexData->new_format)
                    sprintf (buf, "Damage is %dd%d (average %d).\n\r",
                             obj->value[1], obj->value[2],
                             (1 + obj->value[2]) * obj->value[1] / 2);
                else
                    sprintf (buf, "Damage is %d to %d (average %d).\n\r",
                             obj->value[1], obj->value[2],
                             (obj->value[1] + obj->value[2]) / 2);
                send_to_char (buf, ch);
            }
            if (KNOW_CHECK() && obj->value[4]) { /* weapon flags */
                sprintf (buf, "Weapons flags: %s\n\r",
                         weapon_bit_name (obj->value[4]));
                send_to_char (buf, ch);
            }
            break;

        case ITEM_ARMOR:
            if (KNOW_CHECK())
                printf_to_char (ch, "Armor class vs pierce: %d\n\r", obj->value[0]);
            if (KNOW_CHECK())
                printf_to_char (ch, "Armor class vs bash: %d\n\r", obj->value[1]);
            if (KNOW_CHECK())
                printf_to_char (ch, "Armor class vs slash: %d\n\r", obj->value[2]);
            if (KNOW_CHECK())
                printf_to_char (ch, "Armor class vs magic: %d\n\r", obj->value[3]);
            break;
    }

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
            if (!KNOW_CHECK())
                continue;
            if (paf->apply == APPLY_NONE || paf->modifier == 0)
                continue;
            printf_to_char (ch, "Affects %s by %d.\n\r",
                affect_apply_name (paf->apply), paf->modifier);
            if (paf->bits)
                send_to_char (affect_bit_message (paf->bit_type, paf->bits), ch);
        }
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (!KNOW_CHECK())
            continue;
        if (paf->apply != APPLY_NONE && paf->modifier != 0) {
            printf_to_char (ch, "Affects %s by %d",
                affect_apply_name (paf->apply), paf->modifier);
            if (paf->duration > -1)
                printf_to_char (ch, ", %d hours.\n\r", paf->duration);
            else
                send_to_char (".\n\r", ch);
            if (paf->bits)
                send_to_char (affect_bit_message (paf->bit_type, paf->bits), ch);
        }
    }

    if (power <= 100) {
        int percent = (know_pos == 0) ? 100 : (know_count * 100 / know_pos);
        send_to_char (spell_identify_know_message (percent), ch);
    }
}

DEFINE_SPELL_FUN (spell_know_alignment) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;
         if (ap >  700) msg = "$N has a pure and good aura.";
    else if (ap >  350) msg = "$N is of excellent moral character.";
    else if (ap >  100) msg = "$N is often kind and thoughtful.";
    else if (ap > -100) msg = "$N doesn't have a firm moral commitment.";
    else if (ap > -350) msg = "$N lies to $S friends.";
    else if (ap > -700) msg = "$N is a black-hearted murderer.";
    else                msg = "$N is the embodiment of pure evil!.";

    act (msg, ch, NULL, victim, TO_CHAR);
}

DEFINE_SPELL_FUN (spell_locate_object) {
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL (ch) ? 200 : 2 * level;

    buffer = buf_new ();
    for (obj = object_list; obj != NULL; obj = obj->next) {
        if (!char_can_see_obj (ch, obj) || !is_name (target_name, obj->name)
            || IS_OBJ_STAT (obj, ITEM_NOLOCATE)
            || number_percent () > 2 * level || ch->level < obj->level)
            continue;

        found = TRUE;
        number++;

        for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

        if (in_obj->carried_by != NULL &&
            char_can_see_anywhere (ch, in_obj->carried_by))
        {
            sprintf (buf, "one is carried by %s\n\r",
                PERS_AW (in_obj->carried_by, ch));
        }
        else {
            if (IS_IMMORTAL (ch) && in_obj->in_room != NULL)
                sprintf (buf, "one is in %s [Room %d]\n\r",
                         in_obj->in_room->name, in_obj->in_room->vnum);
            else
                sprintf (buf, "one is in %s\n\r", in_obj->in_room == NULL
                         ? "somewhere" : in_obj->in_room->name);
        }

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

DEFINE_SPELL_FUN (spell_farsight) {
    BAIL_IF (IS_AFFECTED (ch, AFF_BLIND),
        "Maybe it would help if you could see?\n\r", ch);
    do_function (ch, &do_scan_far, target_name);
}
