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
#include "olc.h"

#include "olc_oedit.h"

/* Object Editor Functions. */
void show_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * obj) {
    char buf[MAX_STRING_LENGTH];

    switch (obj->item_type) {
        default: /* No values. */
            break;

        case ITEM_LIGHT:
            if (obj->value[2] == -1 || obj->value[2] == 999)    /* ROM OLC */
                sprintf (buf, "[v2] Light:  Infinite[-1]\n\r");
            else
                sprintf (buf, "[v2] Light:  [%d]\n\r", obj->value[2]);
            send_to_char (buf, ch);
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf (buf,
                     "[v0] Level:          [%d]\n\r"
                     "[v1] Charges Total:  [%d]\n\r"
                     "[v2] Charges Left:   [%d]\n\r"
                     "[v3] Spell:          %s\n\r",
                     obj->value[0],
                     obj->value[1],
                     obj->value[2],
                     obj->value[3] != -1 ? skill_table[obj->value[3]].name
                     : "none");
            send_to_char (buf, ch);
            break;

        case ITEM_PORTAL:
            sprintf (buf,
                     "[v0] Charges:        [%d]\n\r"
                     "[v1] Exit Flags:     %s\n\r"
                     "[v2] Portal Flags:   %s\n\r"
                     "[v3] Goes to (vnum): [%d]\n\r",
                     obj->value[0],
                     flag_string (exit_flags, obj->value[1]),
                     flag_string (portal_flags, obj->value[2]),
                     obj->value[3]);
            send_to_char (buf, ch);
            break;

        case ITEM_FURNITURE:
            sprintf (buf,
                     "[v0] Max people:      [%d]\n\r"
                     "[v1] Max weight:      [%d]\n\r"
                     "[v2] Furniture Flags: %s\n\r"
                     "[v3] Heal bonus:      [%d]\n\r"
                     "[v4] Mana bonus:      [%d]\n\r",
                     obj->value[0],
                     obj->value[1],
                     flag_string (furniture_flags, obj->value[2]),
                     obj->value[3], obj->value[4]);
            send_to_char (buf, ch);
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            sprintf (buf,
                     "[v0] Level:  [%d]\n\r"
                     "[v1] Spell:  %s\n\r"
                     "[v2] Spell:  %s\n\r"
                     "[v3] Spell:  %s\n\r"
                     "[v4] Spell:  %s\n\r",
                     obj->value[0],
                     obj->value[1] != -1 ? skill_table[obj->value[1]].name
                     : "none",
                     obj->value[2] != -1 ? skill_table[obj->value[2]].name
                     : "none",
                     obj->value[3] != -1 ? skill_table[obj->value[3]].name
                     : "none",
                     obj->value[4] != -1 ? skill_table[obj->value[4]].name
                     : "none");
            send_to_char (buf, ch);
            break;

        /* ARMOR for ROM */
        case ITEM_ARMOR:
            sprintf (buf,
                     "[v0] Ac pierce       [%d]\n\r"
                     "[v1] Ac bash         [%d]\n\r"
                     "[v2] Ac slash        [%d]\n\r"
                     "[v3] Ac exotic       [%d]\n\r",
                     obj->value[0], obj->value[1], obj->value[2],
                     obj->value[3]);
            send_to_char (buf, ch);
            break;

        /* WEAPON changed in ROM: */
        /* I had to split the output here, I have no idea why, but it helped -- Hugin */
        /* It somehow fixed a bug in showing scroll/pill/potions too ?! */
        case ITEM_WEAPON:
            sprintf (buf, "[v0] Weapon class:   %s\n\r",
                     flag_string (weapon_types, obj->value[0]));
            send_to_char (buf, ch);
            sprintf (buf, "[v1] Number of dice: [%d]\n\r", obj->value[1]);
            send_to_char (buf, ch);
            sprintf (buf, "[v2] Type of dice:   [%d]\n\r", obj->value[2]);
            send_to_char (buf, ch);
            sprintf (buf, "[v3] Type:           %s\n\r",
                     attack_table[obj->value[3]].name);
            send_to_char (buf, ch);
            sprintf (buf, "[v4] Special type:   %s\n\r",
                     flag_string (weapon_flags, obj->value[4]));
            send_to_char (buf, ch);
            break;

        case ITEM_CONTAINER:
            sprintf (buf,
                     "[v0] Weight:     [%d kg]\n\r"
                     "[v1] Flags:      [%s]\n\r"
                     "[v2] Key:     %s [%d]\n\r"
                     "[v3] Capacity    [%d]\n\r"
                     "[v4] Weight Mult [%d]\n\r",
                     obj->value[0],
                     flag_string (container_flags, obj->value[1]),
                     get_obj_index (obj->value[2])
                     ? get_obj_index (obj->value[2])->short_descr
                     : "none", obj->value[2], obj->value[3], obj->value[4]);
            send_to_char (buf, ch);
            break;

        case ITEM_DRINK_CON:
            sprintf (buf,
                     "[v0] Liquid Total: [%d]\n\r"
                     "[v1] Liquid Left:  [%d]\n\r"
                     "[v2] Liquid:       %s\n\r"
                     "[v3] Poisoned:     %s\n\r",
                     obj->value[0],
                     obj->value[1],
                     liq_table[obj->value[2]].name,
                     obj->value[3] != 0 ? "Yes" : "No");
            send_to_char (buf, ch);
            break;

        case ITEM_FOUNTAIN:
            sprintf (buf,
                     "[v0] Liquid Total: [%d]\n\r"
                     "[v1] Liquid Left:  [%d]\n\r"
                     "[v2] Liquid:        %s\n\r",
                     obj->value[0],
                     obj->value[1], liq_table[obj->value[2]].name);
            send_to_char (buf, ch);
            break;

        case ITEM_FOOD:
            sprintf (buf,
                     "[v0] Food hours: [%d]\n\r"
                     "[v1] Full hours: [%d]\n\r"
                     "[v3] Poisoned:   %s\n\r",
                     obj->value[0],
                     obj->value[1], obj->value[3] != 0 ? "Yes" : "No");
            send_to_char (buf, ch);
            break;

        case ITEM_MONEY:
            sprintf (buf, "[v0] Gold:   [%d]\n\r", obj->value[0]);
            send_to_char (buf, ch);
            break;
    }
}

bool set_obj_values (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, int value_num,
                     char *argument)
{
    switch (pObj->item_type) {
        default:
            break;

        case ITEM_LIGHT:
            switch (value_num) {
                default:
                    do_help (ch, "ITEM_LIGHT");
                    return FALSE;
                case 2:
                    send_to_char ("HOURS OF LIGHT SET.\n\r\n\r", ch);
                    pObj->value[2] = atoi (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("SPELL TYPE SET.\n\r", ch);
                    pObj->value[3] = skill_lookup (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("SPELL TYPE 1 SET.\n\r\n\r", ch);
                    pObj->value[1] = skill_lookup (argument);
                    break;
                case 2:
                    send_to_char ("SPELL TYPE 2 SET.\n\r\n\r", ch);
                    pObj->value[2] = skill_lookup (argument);
                    break;
                case 3:
                    send_to_char ("SPELL TYPE 3 SET.\n\r\n\r", ch);
                    pObj->value[3] = skill_lookup (argument);
                    break;
                case 4:
                    send_to_char ("SPELL TYPE 4 SET.\n\r\n\r", ch);
                    pObj->value[4] = skill_lookup (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("AC BASH SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("AC SLASH SET.\n\r\n\r", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("AC EXOTIC SET.\n\r\n\r", ch);
                    pObj->value[3] = atoi (argument);
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
                    ALT_FLAGVALUE_SET (pObj->value[0], weapon_types,
                                       argument);
                    break;
                case 1:
                    send_to_char ("NUMBER OF DICE SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("TYPE OF DICE SET.\n\r\n\r", ch);
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("WEAPON TYPE SET.\n\r\n\r", ch);
                    pObj->value[3] = attack_lookup (argument);
                    break;
                case 4:
                    send_to_char ("SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch);
                    ALT_FLAGVALUE_TOGGLE (pObj->value[4], weapon_flags,
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("EXIT FLAGS SET.\n\r\n\r", ch);
                    ALT_FLAGVALUE_SET (pObj->value[1], exit_flags, argument);
                    break;
                case 2:
                    send_to_char ("PORTAL FLAGS SET.\n\r\n\r", ch);
                    ALT_FLAGVALUE_SET (pObj->value[2], portal_flags, argument);
                    break;
                case 3:
                    send_to_char ("EXIT VNUM SET.\n\r\n\r", ch);
                    pObj->value[3] = atoi (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("MAX WEIGHT SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
                    ALT_FLAGVALUE_TOGGLE (pObj->value[2], furniture_flags, argument);
                    break;
                case 3:
                    send_to_char ("HEAL BONUS SET.\n\r\n\r", ch);
                    pObj->value[3] = atoi (argument);
                    break;
                case 4:
                    send_to_char ("MANA BONUS SET.\n\r\n\r", ch);
                    pObj->value[4] = atoi (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    if ((value = flag_value (container_flags, argument)) !=
                        NO_FLAG)
                        TOGGLE_BIT (pObj->value[1], value);
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
                    pObj->value[2] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("CONTAINER MAX WEIGHT SET.\n\r", ch);
                    pObj->value[3] = atoi (argument);
                    break;
                case 4:
                    send_to_char ("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
                    pObj->value[4] = atoi (argument);
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
                    pObj->value[2] = (liq_lookup (argument) >= 0 ?
                                      liq_lookup (argument) : 0);
                    break;
                case 3:
                    send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
                    pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 2:
                    send_to_char ("LIQUID TYPE SET.\n\r\n\r", ch);
                    pObj->value[2] = (liq_lookup (argument) >= 0 ?
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("HOURS OF FULL SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
                case 3:
                    send_to_char ("POISON VALUE TOGGLED.\n\r\n\r", ch);
                    pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
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
                    pObj->value[0] = atoi (argument);
                    break;
                case 1:
                    send_to_char ("SILVER AMOUNT SET.\n\r\n\r", ch);
                    pObj->value[1] = atoi (argument);
                    break;
            }
            break;
    }
    show_obj_values (ch, pObj);
    return TRUE;
}

bool set_value (CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, char *argument,
    int value)
{
    if (argument[0] == '\0') {
        set_obj_values (ch, pObj, -1, "");    /* '\0' changed to "" -- Hugin */
        return FALSE;
    }
    if (set_obj_values (ch, pObj, value, argument))
        return TRUE;
    return FALSE;
}

/*****************************************************************************
 Name:        oedit_values
 Purpose:    Finds the object and sets its value.
 Called by:    The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values (CHAR_DATA * ch, char *argument, int value) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);
    if (set_value (ch, pObj, argument, value))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_show) {
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int cnt;

    EDIT_OBJ (ch, pObj);

    sprintf (buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
             pObj->name,
             !pObj->area ? -1 : pObj->area->vnum,
             !pObj->area ? "No Area" : pObj->area->title);
    send_to_char (buf, ch);

    sprintf (buf, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
             pObj->vnum, item_get_name (pObj->item_type));
    send_to_char (buf, ch);

    sprintf (buf, "Level:       [%5d]\n\r", pObj->level);
    send_to_char (buf, ch);

    sprintf (buf, "Wear flags:  [%s]\n\r",
             flag_string (wear_flags, pObj->wear_flags));
    send_to_char (buf, ch);

    sprintf (buf, "Extra flags: [%s]\n\r",
             flag_string (extra_flags, pObj->extra_flags));
    send_to_char (buf, ch);

    sprintf (buf, "Material:    [%s]\n\r",    /* ROM */
             material_get_name (pObj->material));
    send_to_char (buf, ch);

    sprintf (buf, "Condition:   [%5d]\n\r",    /* ROM */
             pObj->condition);
    send_to_char (buf, ch);

    sprintf (buf, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
             pObj->weight, pObj->cost);
    send_to_char (buf, ch);

    if (pObj->extra_descr) {
        EXTRA_DESCR_DATA *ed;
        send_to_char ("Ex desc kwd: ", ch);

        for (ed = pObj->extra_descr; ed; ed = ed->next) {
            send_to_char ("[", ch);
            send_to_char (ed->keyword, ch);
            send_to_char ("]", ch);
        }
        send_to_char ("\n\r", ch);
    }

    sprintf (buf, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
             pObj->short_descr, pObj->description);
    send_to_char (buf, ch);

    for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
        if (cnt == 0) {
            send_to_char ("Number Modifier Bits, Apply\n\r", ch);
            send_to_char ("------ -------- -----------------\n\r", ch);
        }
        sprintf (buf, "[%4d] %-8d %8s, %8s\n\r", cnt,
                paf->modifier,
                flag_string (affect_bit_types,   paf->bit_type),
                flag_string (affect_apply_types, paf->apply));
        send_to_char (buf, ch);
        cnt++;
    }

    show_obj_values (ch, pObj);

    return FALSE;
}


/* Need to issue warning if flag isn't valid. -- does so now -- Hugin.  */
OEDIT (oedit_addaffect) {
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, loc);
    one_argument (argument, mod);

    if (loc[0] == '\0' || mod[0] == '\0' || !is_number (mod)) {
        send_to_char ("Syntax:  addaffect [location] [#xmod]\n\r", ch);
        return FALSE;
    }
    if ((value = flag_value (affect_apply_types, loc)) == NO_FLAG) { /* Hugin */
        send_to_char ("Valid affects are:\n\r", ch);
        show_help (ch, "apply");
        return FALSE;
    }

    pAf = affect_new ();
    affect_init (pAf, TO_OBJECT, -1, pObj->level, -1, value, atoi (mod), 0);
    LIST_FRONT (pAf, next, pObj->affected);

    send_to_char ("Affect added.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_addapply) {
    int to, app;
    flag_t bv;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char bit_buf[MAX_STRING_LENGTH];
    char app_buf[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, bit_buf);
    argument = one_argument (argument, app_buf);
    argument = one_argument (argument, mod);
    one_argument (argument, bvector);

    if (bit_buf[0] == '\0' || (to = flag_value (affect_bit_types, bit_buf)) == NO_FLAG) {
        send_to_char ("Invalid bit type. Valid bit types are:\n\r", ch);
        show_help (ch, "affect_bit_types");
        return FALSE;
    }

    if (app_buf[0] == '\0' || (app = flag_value (affect_apply_types, app_buf)) == NO_FLAG) {
        send_to_char ("Invalid apply type. Valid apply types are:\n\r", ch);
        show_help (ch, "affect_apply_types");
        return FALSE;
    }

    /* TODO: look this up... */
#if 0
    if (bvector[0] == '\0' || (bv = flag_value (bitvector_type[typ].table, bvector)) == NO_FLAG) {
        send_to_char ("Invalid bitvector type.\n\r", ch);
        send_to_char ("Valid bitvector types are:\n\r", ch);
        //show_help (ch, bitvector_type[typ].help);
        return FALSE;
    }
#else
    bv = 0;
#endif

    if (mod[0] == '\0' || !is_number (mod)) {
        send_to_char ("Syntax:  addapply [to] [apply] [#xmod] [bitvector]\n\r", ch);
        return FALSE;
    }

    pAf = affect_new ();
    affect_init (pAf, to, -1, pObj->level, -1, app, atoi (mod), bv);
    LIST_FRONT (pAf, next, pObj->affected);

    send_to_char ("Apply added.\n\r", ch);
    return TRUE;
}

/* My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers. */
OEDIT (oedit_delaffect) {
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf, *pAf_prev;
    char affect[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_OBJ (ch, pObj);
    one_argument (argument, affect);

    if (!is_number (affect) || affect[0] == '\0') {
        send_to_char ("Syntax:  delaffect [#xaffect]\n\r", ch);
        return FALSE;
    }
    value = atoi (affect);
    if (value < 0) {
        send_to_char ("Only non-negative affect-numbers allowed.\n\r", ch);
        return FALSE;
    }

    /* Find the affect and its previous link in the list. */
    LIST_FIND_WITH_PREV (value >= cnt++, next, pObj->affected,
        pAf, pAf_prev);
    if (!pAf) {
        send_to_char ("OEdit:  Non-existant affect.\n\r", ch);
        return FALSE;
    }

    LIST_REMOVE_WITH_PREV (pAf, pAf_prev, next, pObj->affected);
    affect_free (pAf);

    send_to_char ("Affect removed.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_name) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  name [string]\n\r", ch);
        return FALSE;
    }

    str_free (pObj->name);
    pObj->name = str_dup (argument);

    send_to_char ("Name set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_short) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  short [string]\n\r", ch);
        return FALSE;
    }

    str_free (pObj->short_descr);
    pObj->short_descr = str_dup (argument);
    pObj->short_descr[0] = LOWER (pObj->short_descr[0]);

    send_to_char ("Short description set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_long) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  long [string]\n\r", ch);
        return FALSE;
    }

    str_free (pObj->description);
    pObj->description = str_dup (argument);
    pObj->description[0] = UPPER (pObj->description[0]);

    send_to_char ("Long description set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_value0) {
    if (oedit_values (ch, argument, 0))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_value1) {
    if (oedit_values (ch, argument, 1))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_value2) {
    if (oedit_values (ch, argument, 2))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_value3) {
    if (oedit_values (ch, argument, 3))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_value4) {
    if (oedit_values (ch, argument, 4))
        return TRUE;
    return FALSE;
}

OEDIT (oedit_weight) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  weight [number]\n\r", ch);
        return FALSE;
    }
    pObj->weight = atoi (argument);

    send_to_char ("Weight set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_cost) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  cost [number]\n\r", ch);
        return FALSE;
    }
    pObj->cost = atoi (argument);

    send_to_char ("Cost set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_create) {
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int value;
    int iHash;

    value = atoi (argument);
    if (argument[0] == '\0' || value == 0) {
        send_to_char ("Syntax:  oedit create [vnum]\n\r", ch);
        return FALSE;
    }

    pArea = area_get_by_inner_vnum (value);
    if (!pArea) {
        send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("OEdit:  Vnum in an area you cannot build in.\n\r", ch);
        return FALSE;
    }
    if (get_obj_index (value)) {
        send_to_char ("OEdit:  Object vnum already exists.\n\r", ch);
        return FALSE;
    }

    pObj = obj_index_new ();
    pObj->area = pArea;
    pObj->vnum = value;
    pObj->anum = value - pArea->min_vnum;

    if (value > top_vnum_obj)
        top_vnum_obj = value;

    iHash = value % MAX_KEY_HASH;
    LIST_FRONT (pObj, next, obj_index_hash[iHash]);
    ch->desc->pEdit = (void *) pObj;

    send_to_char ("Object Created.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_ed) {
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ (ch, pObj);

    argument = one_argument (argument, command);
    one_argument (argument, keyword);

    if (command[0] == '\0') {
        send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
        send_to_char ("         ed delete [keyword]\n\r", ch);
        send_to_char ("         ed edit [keyword]\n\r", ch);
        send_to_char ("         ed format [keyword]\n\r", ch);
        return FALSE;
    }
    if (!str_cmp (command, "add")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed add [keyword]\n\r", ch);
            return FALSE;
        }
        ed = extra_descr_new ();
        ed->keyword = str_dup (keyword);
        LIST_FRONT (ed, next, pObj->extra_descr);

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "edit")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed edit [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (is_name (keyword, ed->keyword), next,
            pObj->extra_descr, ed);
        if (!ed) {
            send_to_char ("OEdit:  Extra description keyword not found.\n\r", ch);
            return FALSE;
        }

        string_append (ch, &ed->description);
        return TRUE;
    }

    if (!str_cmp (command, "delete")) {
        EXTRA_DESCR_DATA *ped;
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed delete [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND_WITH_PREV (is_name (keyword, ed->keyword),
            next, pObj->extra_descr, ed, ped);
        if (!ed) {
            send_to_char ("OEdit:  Extra description keyword not found.\n\r", ch);
            return FALSE;
        }
        LIST_REMOVE_WITH_PREV (ed, ped, next, pObj->extra_descr);

        extra_descr_free (ed);
        send_to_char ("Extra description deleted.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "format")) {
        if (keyword[0] == '\0') {
            send_to_char ("Syntax:  ed format [keyword]\n\r", ch);
            return FALSE;
        }
        LIST_FIND (is_name (keyword, ed->keyword), next,
            pObj->extra_descr, ed);
        if (!ed) {
            send_to_char ("OEdit:  Extra description keyword not found.\n\r", ch);
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
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, pObj);
        if ((value = flag_value (extra_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pObj->extra_flags, value);
            send_to_char ("Extra flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax:  extra [flag]\n\r"
                  "Type '? extra' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of oedit() due to naming conflicts -- Hugin */
OEDIT (oedit_wear) {
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, pObj);
        if ((value = flag_value (wear_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pObj->wear_flags, value);
            send_to_char ("Wear flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax:  wear [flag]\n\r"
                  "Type '? wear' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of oedit() due to naming conflicts -- Hugin */
OEDIT (oedit_type) {
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, pObj);
        if ((value = item_lookup (argument)) >= 0) {
            pObj->item_type = value;
            send_to_char ("Type set.\n\r", ch);
            pObj->value[0] = 0;
            pObj->value[1] = 0;
            pObj->value[2] = 0;
            pObj->value[3] = 0;
            pObj->value[4] = 0; /* ROM */
            return TRUE;
        }
    }

    send_to_char ("Syntax:  type [flag]\n\r"
                  "Type '? type' for a list of flags.\n\r", ch);
    return FALSE;
}

OEDIT (oedit_material) {
    OBJ_INDEX_DATA *pObj;
    const MATERIAL_TYPE *mat;

    if (argument[0] != '\0') {
        EDIT_OBJ (ch, pObj);
        if ((mat = material_get_by_name (argument)) != NULL) {
            str_replace_dup (&(pObj->material_str), mat->name);
            pObj->material = mat->type;
            send_to_char ("Material type changed.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax:  material [type]\n\r", ch);
    return FALSE;
}

OEDIT (oedit_level) {
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ (ch, pObj);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  level [number]\n\r", ch);
        return FALSE;
    }
    pObj->level = atoi (argument);

    send_to_char ("Level set.\n\r", ch);
    return TRUE;
}

OEDIT (oedit_condition) {
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0'
        && (value = atoi (argument)) >= 0 && (value <= 100))
    {
        EDIT_OBJ (ch, pObj);
        pObj->condition = value;
        send_to_char ("Condition set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("Syntax:  condition [number]\n\r"
        "Where number can range from 0 (ruined) to 100 (perfect).\n\r", ch);
    return FALSE;
}
