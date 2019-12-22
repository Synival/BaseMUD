/***************************************************************************
 *  File: olc_oedit.c                                                      *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "comm.h"
#include "lookup.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "string.h"
#include "affects.h"
#include "act_info.h"
#include "chars.h"
#include "globals.h"
#include "olc.h"
#include "memory.h"

#include "olc_oedit.h"

/* Object Editor Functions. */
void oedit_show_obj_values (CHAR_T *ch, OBJ_INDEX_T *obj) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            if (obj->v.light.duration == -1 ||
                obj->v.light.duration == 999)
            {
                send_to_char ("[v2] Light:  Infinite[-1]\n\r", ch);
            }
            else
                printf_to_char (ch, "[v2] Light:  [%ld]\n\r",
                    obj->v.light.duration);
            break;

        case ITEM_WAND:
            printf_to_char (ch,
                "[v0] Level:         [%ld]\n\r"
                "[v1] Charges Total: [%ld]\n\r"
                "[v2] Charges Left:  [%ld]\n\r"
                "[v3] Spell:         %s\n\r",
                 obj->v.wand.level,
                 obj->v.wand.recharge,
                 obj->v.wand.charges,
                 obj->v.wand.skill > 0
                    ? skill_table[obj->v.wand.skill].name : "none");
            break;

        case ITEM_STAFF:
            printf_to_char (ch,
                "[v0] Level:         [%ld]\n\r"
                "[v1] Charges Total: [%ld]\n\r"
                "[v2] Charges Left:  [%ld]\n\r"
                "[v3] Spell:         %s\n\r",
                 obj->v.staff.level,
                 obj->v.staff.recharge,
                 obj->v.staff.charges,
                 obj->v.staff.skill > 0
                    ? skill_table[obj->v.staff.skill].name : "none");
            break;

        case ITEM_PORTAL:
            printf_to_char (ch,
                "[v0] Charges:        [%ld]\n\r"
                "[v1] Exit Flags:     %s\n\r"
                "[v2] Portal Flags:   %s\n\r"
                "[v3] Goes to (vnum): [%ld]\n\r",
                obj->v.portal.charges,
                flag_string (exit_flags, obj->v.portal.exit_flags),
                flag_string (gate_flags, obj->v.portal.gate_flags),
                obj->v.portal.to_vnum);
            break;

        case ITEM_FURNITURE:
            printf_to_char (ch,
                "[v0] Max People:      [%ld]\n\r"
                "[v1] Max Weight:      [%ld]\n\r"
                "[v2] Furniture Flags: %s\n\r"
                "[v3] Heal bonus:      [%ld]\n\r"
                "[v4] Mana bonus:      [%ld]\n\r",
                obj->v.furniture.max_people,
                obj->v.furniture.max_weight,
                flag_string (furniture_flags, obj->v.furniture.flags),
                obj->v.furniture.heal_rate,
                obj->v.furniture.mana_rate);
            break;

        case ITEM_SCROLL: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.scroll.level);
            for (i = 0; i < SCROLL_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.scroll.skill[i] > 0
                        ? skill_table[obj->v.scroll.skill[i]].name : "none");
            }
            break;
        }

        case ITEM_POTION: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.potion.level);
            for (i = 0; i < POTION_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.potion.skill[i] > 0
                        ? skill_table[obj->v.potion.skill[i]].name : "none");
            }
            break;
        }

        case ITEM_PILL: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.pill.level);
            for (i = 0; i < PILL_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.pill.skill[i] > 0
                        ? skill_table[obj->v.pill.skill[i]].name : "none");
            }
            break;
        }

        /* ARMOR for ROM */
        case ITEM_ARMOR:
            printf_to_char (ch,
                "[v0] Ac Pierce: [%ld]\n\r"
                "[v1] Ac Bash:   [%ld]\n\r"
                "[v2] Ac Slash:  [%ld]\n\r"
                "[v3] Ac Magic:  [%ld]\n\r",
                obj->v.armor.vs_pierce,
                obj->v.armor.vs_bash,
                obj->v.armor.vs_slash,
                obj->v.armor.vs_magic);
            break;

        /* WEAPON changed in ROM: */
        /* I had to split the output here, I have no idea why, but it helped -- Hugin */
        /* It somehow fixed a bug in showing scroll/pill/potions too ?! */

        /* ^^^ flag_string() uses a static char[], which must be copied to at
         *     least one separate buffer. -- Synival */

        case ITEM_WEAPON: {
            char wtype[MAX_STRING_LENGTH];
            char wflags[MAX_STRING_LENGTH];
            strcpy (wtype,  flag_string (weapon_types, obj->v.weapon.weapon_type));
            strcpy (wflags, flag_string (weapon_flags, obj->v.weapon.flags));

            printf_to_char (ch,
                "[v0] Weapon Class:   %s\n\r"
                "[v1] Number of Dice: [%ld]\n\r"
                "[v2] Type of Dice:   [%ld]\n\r"
                "[v3] Type:           %s\n\r"
                "[v4] Special Type:   %s\n\r",
                wtype,
                obj->v.weapon.dice_num,
                obj->v.weapon.dice_size,
                attack_table[obj->v.weapon.attack_type].name,
                wflags);
            break;
        }

        case ITEM_CONTAINER: {
            OBJ_INDEX_T *key = get_obj_index (obj->v.container.key);
            printf_to_char (ch,
                "[v0] Weight:     [%ld kg]\n\r"
                "[v1] Flags:      [%s]\n\r"
                "[v2] Key:        %s [%ld]\n\r"
                "[v3] Capacity    [%ld]\n\r"
                "[v4] Weight Mult [%ld]\n\r",
                obj->v.container.capacity,
                flag_string (container_flags, obj->v.container.flags),
                key ? key->short_descr : "none",
                obj->v.container.key,
                obj->v.container.max_weight,
                obj->v.container.weight_mult);
            break;
        }

        case ITEM_DRINK_CON:
            printf_to_char (ch,
                "[v0] Liquid Total: [%ld]\n\r"
                "[v1] Liquid Left:  [%ld]\n\r"
                "[v2] Liquid:       %s\n\r"
                "[v3] Poisoned:     %s\n\r",
                obj->v.drink_con.capacity,
                obj->v.drink_con.filled,
                liq_table[obj->v.drink_con.liquid].name,
                obj->v.drink_con.poisoned != 0 ? "Yes" : "No");
            break;

        case ITEM_FOUNTAIN:
            printf_to_char (ch,
                "[v0] Liquid Total: [%ld]\n\r"
                "[v1] Liquid Left:  [%ld]\n\r"
                "[v2] Liquid:        %s\n\r"
                "[v3] Poisoned:      %s\n\r",
                obj->v.fountain.capacity,
                obj->v.fountain.filled,
                liq_table[obj->v.fountain.liquid].name,
                obj->v.fountain.poisoned != 0 ? "Yes" : "No");
            break;

        case ITEM_FOOD:
            printf_to_char (ch,
                "[v0] Food hours: [%ld]\n\r"
                "[v1] Full hours: [%ld]\n\r"
                "[v3] Poisoned:   %s\n\r",
                obj->v.food.hunger,
                obj->v.food.fullness,
                obj->v.food.poisoned != 0 ? "Yes" : "No");
            break;

        case ITEM_MONEY:
            printf_to_char (ch,
                "[v0] Silver: [%ld]\n\r",
                "[v1] Gold:   [%ld]\n\r",
                obj->v.money.silver,
                obj->v.money.gold);
            break;
    }
}

bool oedit_set_obj_values (CHAR_T *ch, OBJ_INDEX_T *obj,
    int value_num, char *argument)
{
    switch (obj->item_type) {
        default:
            break;

        case ITEM_LIGHT:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_LIGHT");
                    return FALSE;
                case 2:
                    send_to_char ("HOURS OF LIGHT SET.\n\r\n\r", ch);
                    obj->v.value[2] = atoi (argument);
                    break;
            }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_STAFF_WAND");
                    return FALSE;
                case 0:
                    send_to_char ("SPELL LEVEL SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch);
                    obj->v.value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("SPELL TYPE SET.\n\r", ch);
                    obj->v.value[3] = skill_lookup (argument);
                    break;
            }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_SCROLL_POTION_PILL");
                    return FALSE;
                case 0:
                    send_to_char ("SPELL LEVEL SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("SPELL TYPE 1 SET.\n\r\n\r", ch);
                    obj->v.value[1] = skill_lookup (argument);
                    break;
                case 2:
                    send_to_char ("SPELL TYPE 2 SET.\n\r\n\r", ch);
                    obj->v.value[2] = skill_lookup (argument);
                    break;
                case 3:
                    send_to_char ("SPELL TYPE 3 SET.\n\r\n\r", ch);
                    obj->v.value[3] = skill_lookup (argument);
                    break;
                case 4:
                    send_to_char ("SPELL TYPE 4 SET.\n\r\n\r", ch);
                    obj->v.value[4] = skill_lookup (argument);
                    break;
            }
            break;

        /* ARMOR for ROM: */
        case ITEM_ARMOR:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_ARMOR");
                    return FALSE;
                case 0:
                    send_to_char ("AC PIERCE SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("AC BASH SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("AC SLASH SET.\n\r\n\r", ch);
                    obj->v.value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("AC EXOTIC SET.\n\r\n\r", ch);
                    obj->v.value[3] = atoi (argument);
                    break;
            }
            break;

        /* WEAPONS changed in ROM */
        case ITEM_WEAPON:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_WEAPON");
                    return FALSE;
                case 0:
                    send_to_char ("WEAPON CLASS SET.\n\r\n\r", ch);
                    ALT_FLAGVALUE_SET (obj->v.value[0], weapon_types,
                                       argument);
                    break;
                case 1:
                    send_to_char ("NUMBER OF DICE SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("TYPE OF DICE SET.\n\r\n\r", ch);
                    obj->v.value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("WEAPON TYPE SET.\n\r\n\r", ch);
                    obj->v.value[3] = attack_lookup (argument);
                    break;
                case 4:
                    send_to_char ("SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch);
                    ALT_FLAGVALUE_TOGGLE (obj->v.value[4], weapon_flags,
                                          argument);
                    break;
            }
            break;

        case ITEM_PORTAL:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_PORTAL");
                    return FALSE;
                case 0:
                    send_to_char ("CHARGES SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("EXIT FLAGS SET.\n\r\n\r", ch);
                    ALT_FLAGVALUE_SET (obj->v.value[1], exit_flags, argument);
                    break;
                case 2:
                    send_to_char ("PORTAL FLAGS SET.\n\r\n\r", ch);
                    ALT_FLAGVALUE_SET (obj->v.value[2], gate_flags, argument);
                    break;
                case 3:
                    send_to_char ("EXIT VNUM SET.\n\r\n\r", ch);
                    obj->v.value[3] = atoi (argument);
                    break;
            }
            break;

        case ITEM_FURNITURE:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_FURNITURE");
                    return FALSE;
                case 0:
                    send_to_char ("NUMBER OF PEOPLE SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("MAX WEIGHT SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
                    ALT_FLAGVALUE_TOGGLE (obj->v.value[2], furniture_flags, argument);
                    break;
                case 3:
                    send_to_char ("HEAL BONUS SET.\n\r\n\r", ch);
                    obj->v.value[3] = atoi (argument);
                    break;
                case 4:
                    send_to_char ("MANA BONUS SET.\n\r\n\r", ch);
                    obj->v.value[4] = atoi (argument);
                    break;
            }
            break;

        case ITEM_CONTAINER:
            switch (value_num) {
                int value;
                default:
                    do_help (ch, "ITEM_CONTAINER");
                    return FALSE;
                case 0:
                    send_to_char ("WEIGHT CAPACITY SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    if ((value = flag_value (container_flags, argument)) !=
                        NO_FLAG)
                        TOGGLE_BIT (obj->v.value[1], value);
                    else {
                        do_help (ch, "ITEM_CONTAINER");
                        return FALSE;
                    }
                    send_to_char ("CONTAINER TYPE SET.\n\r\n\r", ch);
                    break;
                case 2:
                    if (atoi (argument) != 0) {
                        if (!get_obj_index (atoi (argument))) {
                            send_to_char ("THERE IS NO SUCH ITEM.\n\r\n\r", ch);
                            return FALSE;
                        }
                        if (get_obj_index (atoi (argument))->item_type != ITEM_KEY) {
                            send_to_char ("THAT ITEM IS NOT A KEY.\n\r\n\r", ch);
                            return FALSE;
                        }
                    }
                    send_to_char ("CONTAINER KEY SET.\n\r\n\r", ch);
                    obj->v.value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("CONTAINER MAX WEIGHT SET.\n\r\n\r", ch);
                    obj->v.value[3] = atoi (argument);
                    break;
                case 4:
                    send_to_char ("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
                    obj->v.value[4] = atoi (argument);
                    break;
            }
            break;

        case ITEM_DRINK_CON:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_DRINK");
/* OLC              do_help (ch, "liquids"); */
                    return FALSE;
                case 0:
                    send_to_char ("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
                    obj->v.value[2] = (liq_lookup (argument) >= 0 ?
                                      liq_lookup (argument) : 0);
                    break;
                case 3:
                    send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
                    obj->v.value[3] = (obj->v.value[3] == 0) ? 1 : 0;
                    break;
            }
            break;

        case ITEM_FOUNTAIN:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_FOUNTAIN");
/* OLC              do_help (ch, "liquids"); */
                    return FALSE;
                case 0:
                    send_to_char ("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
                    obj->v.value[2] = (liq_lookup (argument) >= 0 ?
                                        liq_lookup (argument) : 0);
                    break;
            }
            break;

        case ITEM_FOOD:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_FOOD");
                    return FALSE;
                case 0:
                    send_to_char ("HOURS OF FOOD SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("HOURS OF FULL SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
                    obj->v.value[3] = (obj->v.value[3] == 0) ? 1 : 0;
                    break;
            }
            break;

        case ITEM_MONEY:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_MONEY");
                    return FALSE;
                case 0:
                    send_to_char ("GOLD AMOUNT SET.\n\r\n\r", ch);
                    obj->v.value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("SILVER AMOUNT SET.\n\r\n\r", ch);
                    obj->v.value[1] = atoi (argument);
                    break;
            }
            break;
    }
    oedit_show_obj_values (ch, obj);
    return TRUE;
}

bool oedit_set_value (CHAR_T *ch, OBJ_INDEX_T *obj, char *argument,
    int value)
{
    if (argument[0] == '\0') {
        /* '\0' changed to "" -- Hugin */
        oedit_set_obj_values (ch, obj, -1, "");
        return FALSE;
    }
    if (oedit_set_obj_values (ch, obj, value, argument))
        return TRUE;
    return FALSE;
}

/*****************************************************************************
 Name:       oedit_values
 Purpose:    Finds the object and sets its value.
 Called by:  The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values (CHAR_T *ch, char *argument, int value) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);
    if (oedit_set_value (ch, obj, argument, value))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_show) {
    OBJ_INDEX_T *obj;
    AFFECT_T *paf;
    int cnt;

    EDIT_OBJ (ch, obj);

    printf_to_char (ch, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
        obj->name,
        !obj->area ? -1 : obj->area->vnum,
        !obj->area ? "No Area" : obj->area->title);

    printf_to_char (ch, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
        obj->vnum, item_get_name (obj->item_type));
    printf_to_char (ch, "Level:       [%5d]\n\r",
        obj->level);
    printf_to_char (ch, "Wear flags:  [%s]\n\r",
        flag_string (wear_flags, obj->wear_flags));
    printf_to_char (ch, "Extra flags: [%s]\n\r",
        flag_string (extra_flags, obj->extra_flags));
    printf_to_char (ch, "Material:    [%s]\n\r",    /* ROM */
        material_get_name (obj->material));
    printf_to_char (ch, "Condition:   [%5d]\n\r",    /* ROM */
        obj->condition);
    printf_to_char (ch, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
        obj->weight, obj->cost);

    if (obj->extra_descr) {
        EXTRA_DESCR_T *ed;
        send_to_char ("Ex desc kwd:", ch);

        for (ed = obj->extra_descr; ed; ed = ed->next)
            printf_to_char (ch, " [%s]", ed->keyword);
        send_to_char ("\n\r", ch);
    }

    printf_to_char (ch, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
        obj->short_descr, obj->description);

    for (cnt = 0, paf = obj->affected; paf; paf = paf->next) {
        if (cnt == 0) {
            send_to_char ("Number Modifier Bits, Apply\n\r"
                          "------ -------- -----------------\n\r", ch);
        }
        printf_to_char (ch, "[%4d] %-8d %8s, %8s\n\r",
            cnt, paf->modifier,
            affect_bit_get_name (paf->bit_type),
            flag_string (affect_apply_types, paf->apply));
        cnt++;
    }

    oedit_show_obj_values (ch, obj);
    return FALSE;
}

/* Need to issue warning if flag isn't valid. -- does so now -- Hugin.  */
OEDIT (oedit_addaffect) {
    int value;
    OBJ_INDEX_T *obj;
    AFFECT_T *aff;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, obj);

    argument = one_argument (argument, loc);
    one_argument (argument, mod);

    RETURN_IF (loc[0] == '\0' || mod[0] == '\0' || !is_number (mod),
        "Syntax: addaffect [location] [#xmod]\n\r", ch, FALSE);

    if ((value = flag_value (affect_apply_types, loc)) == NO_FLAG) { /* Hugin */
        send_to_char ("Valid affects are:\n\r", ch);
        show_help (ch, "apply");
        return FALSE;
    }

    aff = affect_new ();
    affect_init (aff, AFF_TO_OBJECT, -1, obj->level, -1, value, atoi (mod), 0);
    LIST_FRONT (aff, next, obj->affected);

    send_to_char ("Affect added.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_addapply) {
    int bit_type, app;
    flag_t bit;
    const AFFECT_BIT_T *bit_type_obj;
    OBJ_INDEX_T *obj;
    AFFECT_T *aff;
    char bit_type_buf[MAX_STRING_LENGTH];
    char app_buf[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char bit_buf[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, obj);

    argument = one_argument (argument, bit_type_buf);
    argument = one_argument (argument, app_buf);
    argument = one_argument (argument, mod);
    one_argument (argument, bit_buf);

    if (mod[0] == '\0' || !is_number (mod)) {
        send_to_char ("Syntax: addapply [bit_type] [apply] [#xmod] [bit]\n\r", ch);
        return FALSE;
    }

    if (bit_type_buf[0] == '\0' ||
        (bit_type_obj = affect_bit_get_by_name (bit_type_buf)) == NULL)
    {
        send_to_char ("Invalid bit type. Valid bit types are:\n\r", ch);
        show_help (ch, "affect_bit_table");
        return FALSE;
    }
    bit_type = bit_type_obj->type;

    if (app_buf[0] == '\0' ||
        (app = flag_value (affect_apply_types, app_buf)) == NO_FLAG)
    {
        send_to_char ("Invalid apply type. Valid apply types are:\n\r", ch);
        show_help (ch, "affect_apply_types");
        return FALSE;
    }

    if (bit_buf[0] == '\0' ||
        (bit = flag_value (bit_type_obj->flags, bit_buf)) == NO_FLAG)
    {
        send_to_char ("Invalid bit. Valid bits are: \n\r", ch);
        send_to_char ("Valid flag types are:\n\r", ch);
        show_help (ch, bit_type_obj->help);
        return FALSE;
    }

    aff = affect_new ();
    affect_init (aff, bit_type, -1, obj->level, -1, app, atoi (mod), bit);
    LIST_FRONT (aff, next, obj->affected);

    send_to_char ("Apply added.\n\r", ch);
    return TRUE;
}

/* My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers. */
OEDIT (oedit_delaffect) {
    OBJ_INDEX_T *obj;
    AFFECT_T *aff, *aff_prev;
    char affect[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_OBJ (ch, obj);
    one_argument (argument, affect);

    if (!is_number (affect) || affect[0] == '\0') {
        send_to_char ("Syntax: delaffect [#xaffect]\n\r", ch);
        return FALSE;
    }
    value = atoi (affect);
    if (value < 0) {
        send_to_char ("Only non-negative affect-numbers allowed.\n\r", ch);
        return FALSE;
    }

    /* Find the affect and its previous link in the list. */
    LIST_FIND_WITH_PREV (value >= cnt++, next, obj->affected,
        aff, aff_prev);
    if (!aff) {
        send_to_char ("OEdit: Non-existant affect.\n\r", ch);
        return FALSE;
    }

    LIST_REMOVE_WITH_PREV (aff, aff_prev, next, obj->affected);
    affect_free (aff);

    send_to_char ("Affect removed.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_name) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: name [string]\n\r", ch);
        return FALSE;
    }

    str_replace_dup (&(obj->name), argument);

    send_to_char ("Name set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_short) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: short [string]\n\r", ch);
        return FALSE;
    }

    str_replace_dup (&(obj->short_descr), argument);
    obj->short_descr[0] = LOWER (obj->short_descr[0]);

    send_to_char ("Short description set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_long) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: long [string]\n\r", ch);
        return FALSE;
    }

    str_replace_dup (&(obj->description), argument);
    obj->description[0] = UPPER (obj->description[0]);

    send_to_char ("Long description set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_value0)
    { return oedit_values (ch, argument, 0) ? TRUE : FALSE; }
OEDIT (oedit_value1)
    { return oedit_values (ch, argument, 1) ? TRUE : FALSE; }
OEDIT (oedit_value2)
    { return oedit_values (ch, argument, 2) ? TRUE : FALSE; }
OEDIT (oedit_value3)
    { return oedit_values (ch, argument, 3) ? TRUE : FALSE; }
OEDIT (oedit_value4)
    { return oedit_values (ch, argument, 4) ? TRUE : FALSE; }

OEDIT (oedit_weight) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax: weight [number]\n\r", ch);
        return FALSE;
    }
    obj->weight = atoi (argument);

    send_to_char ("Weight set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_cost) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax: cost [number]\n\r", ch);
        return FALSE;
    }
    obj->cost = atoi (argument);

    send_to_char ("Cost set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_create) {
    OBJ_INDEX_T *obj;
    AREA_T *area;
    int value, hash;

    value = atoi (argument);
    if (argument[0] == '\0' || value == 0) {
        send_to_char ("Syntax: oedit create [vnum]\n\r", ch);
        return FALSE;
    }

    area = area_get_by_inner_vnum (value);
    if (!area) {
        send_to_char ("OEdit: That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if (!IS_BUILDER (ch, area)) {
        send_to_char ("OEdit: Vnum in an area you cannot build in.\n\r", ch);
        return FALSE;
    }
    if (get_obj_index (value)) {
        send_to_char ("OEdit: Object vnum already exists.\n\r", ch);
        return FALSE;
    }

    obj = obj_index_new ();
    obj->area = area;
    obj->vnum = value;
    obj->anum = value - area->min_vnum;

    if (value > top_vnum_obj)
        top_vnum_obj = value;

    hash = value % MAX_KEY_HASH;
    LIST_FRONT (obj, next, obj_index_hash[hash]);
    ch->desc->olc_edit = (void *) obj;

    send_to_char ("Object created.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_ed) {
    OBJ_INDEX_T *obj;
    EXTRA_DESCR_T *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ (ch, obj);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0') {
        send_to_char ("Syntax: ed add [keyword]\n\r", ch);
        send_to_char ("        ed delete [keyword]\n\r", ch);
        send_to_char ("        ed edit [keyword]\n\r", ch);
        send_to_char ("        ed format [keyword]\n\r", ch);
        return FALSE;
    }
    if (!str_cmp (command, "add")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed add [keyword]\n\r", ch);
            return FALSE;
        }
        ed = extra_descr_new ();
        ed->keyword = str_dup (keyword);
        LIST_FRONT (ed, next, obj->extra_descr);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "edit")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed edit [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (str_in_namelist (keyword, ed->keyword), next,
            obj->extra_descr, ed);
        if (!ed) {
            send_to_char ("OEdit: Extra description keyword not found.\n\r", ch);
            return FALSE;
        }

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "delete")) {
        EXTRA_DESCR_T *ped;
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed delete [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND_WITH_PREV (str_in_namelist (keyword, ed->keyword),
            next, obj->extra_descr, ed, ped);
        if (!ed) {
            send_to_char ("OEdit: Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        LIST_REMOVE_WITH_PREV (ed, ped, next, obj->extra_descr);

        extra_descr_free (ed);
        send_to_char ("Extra description deleted.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "format")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax: ed format [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (str_in_namelist (keyword, ed->keyword), next,
            obj->extra_descr, ed);
        if (!ed) {
            send_to_char ("OEdit: Extra description keyword not found.\n\r", ch);
            return FALSE;
        }

        ed->description = format_string (ed->description);
        send_to_char ("Extra description formatted.\n\r", ch);
        return TRUE;
    }

    oedit_ed (ch, "");
    return FALSE;
}

/* ROM object functions : */
/* Moved out of oedit() due to naming conflicts -- Hugin */
OEDIT (oedit_extra) {
    OBJ_INDEX_T *obj;
    int value;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, obj);
        if ((value = flag_value (extra_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (obj->extra_flags, value);
            send_to_char ("Extra flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax: extra [flag]\n\r"
                  "Type '? extra' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of oedit() due to naming conflicts -- Hugin */
OEDIT (oedit_wear) {
    OBJ_INDEX_T *obj;
    int value;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, obj);
        if ((value = flag_value (wear_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (obj->wear_flags, value);
            send_to_char ("Wear flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax: wear [flag]\n\r"
                  "Type '? wear' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of oedit() due to naming conflicts -- Hugin */
OEDIT (oedit_type) {
    OBJ_INDEX_T *obj;
    int value, i;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, obj);
        if ((value = item_lookup (argument)) >= 0) {
            obj->item_type = value;
            send_to_char ("Type set.\n\r", ch);
            for (i = 0; i < OBJ_VALUE_MAX; i++)
                obj->v.value[i] = 0;
            return TRUE;
        }
    }

    send_to_char ("Syntax: type [flag]\n\r"
                  "Type '? type' for a list of flags.\n\r", ch);
    return FALSE;
}

OEDIT (oedit_material) {
    OBJ_INDEX_T *obj;
    const MATERIAL_T *mat;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, obj);
        if ((mat = material_get_by_name (argument)) != NULL) {
            obj->material = mat->type;
            send_to_char ("Material type changed.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax: material [type]\n\r", ch);
    return FALSE;
}

OEDIT (oedit_level) {
    OBJ_INDEX_T *obj;
    EDIT_OBJ (ch, obj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax: level [number]\n\r", ch);
        return FALSE;
    }
    obj->level = atoi (argument);

    send_to_char ("Level set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_condition) {
    OBJ_INDEX_T *obj;
    int value;

    if (argument[0] != '\0'
        && (value = atoi (argument)) >= 0 && (value <= 100))
    {
        EDIT_OBJ (ch, obj);
        obj->condition = value;
        send_to_char ("Condition set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax: condition [number]\n\r"
        "Where number can range from 0 (ruined) to 100 (perfect).\n\r", ch);
    return FALSE;
}
