/***************************************************************************
 *  File: olc_medit.c                                                      *
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
#include <ctype.h>

#include "comm.h"
#include "lookup.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "string.h"
#include "mob_cmds.h"
#include "chars.h"
#include "globals.h"
#include "olc.h"
#include "memory.h"

#include "olc_medit.h"

MEDIT (medit_show) {
    MOB_INDEX_T *pMob;
    MPROG_LIST_T *list;
    EDIT_MOB (ch, pMob);

    printf_to_char (ch, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
        pMob->name, !pMob->area ? -1 : pMob->area->vnum,
        !pMob->area ? "No Area" : pMob->area->title);

    printf_to_char (ch, "Mob:         [%s]\n\r",
        flag_string (mob_flags, pMob->mob_final));

    printf_to_char (ch, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n\r",
         pMob->vnum,
         pMob->sex == SEX_MALE   ? "male   " :
         pMob->sex == SEX_FEMALE ? "female " :
         pMob->sex == 3          ? "random " : "neutral",
         race_table[pMob->race].name);

    printf_to_char (ch, "Level:       [%2d]    Align: [%4d]      "
                        "Hitroll: [%2d] Dam Type:    [%s]\n\r",
        pMob->level, pMob->alignment, pMob->hitroll,
        attack_table[pMob->dam_type].name);

    if (pMob->group)
        printf_to_char (ch, "Group:       [%5d]\n\r", pMob->group);

    printf_to_char (ch, "Hit dice:    [%2dd%-3d+%4d] ",
        pMob->hit.number, pMob->hit.size, pMob->hit.bonus);

    printf_to_char (ch, "Damage dice: [%2dd%-3d+%4d] ",
        pMob->damage.number, pMob->damage.size, pMob->damage.bonus);

    printf_to_char (ch, "Mana dice:   [%2dd%-3d+%4d]\n\r",
        pMob->mana.number, pMob->mana.size, pMob->mana.bonus);

    /* ROM values end */
    printf_to_char (ch, "Affected by: [%s]\n\r",
        flag_string (affect_flags, pMob->affected_by_final));

    /* ROM values: */
    printf_to_char (ch, "Armor:       [pierce: %d  bash: %d  "
                        "slash: %d  magic: %d]\n\r",
        pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
        pMob->ac[AC_EXOTIC]);

    printf_to_char (ch, "Form:        [%s]\n\r", flag_string (form_flags, pMob->form_final));
    printf_to_char (ch, "Parts:       [%s]\n\r", flag_string (part_flags, pMob->parts_final));
    printf_to_char (ch, "Imm:         [%s]\n\r", flag_string (res_flags, pMob->imm_flags_final));
    printf_to_char (ch, "Res:         [%s]\n\r", flag_string (res_flags, pMob->res_flags_final));
    printf_to_char (ch, "Vuln:        [%s]\n\r", flag_string (res_flags, pMob->vuln_flags_final));
    printf_to_char (ch, "Off:         [%s]\n\r", flag_string (off_flags, pMob->off_flags_final));
    printf_to_char (ch, "Size:        [%s]\n\r", flag_string (size_types, pMob->size));
    printf_to_char (ch, "Material:    [%s]\n\r", material_get_name (pMob->material));
    printf_to_char (ch, "Start pos.   [%s]\n\r", flag_string (position_types, pMob->start_pos));
    printf_to_char (ch, "Default pos  [%s]\n\r", flag_string (position_types, pMob->default_pos));
    printf_to_char (ch, "Wealth:      [%5ld]\n\r", pMob->wealth);

    /* ROM values end */
    if (pMob->spec_fun)
        printf_to_char (ch, "Spec fun:    [%s]\n\r",
            spec_function_name (pMob->spec_fun));

    printf_to_char (ch, "Short descr: %s\n\rLong descr:\n\r%s",
        pMob->short_descr, pMob->long_descr);

    printf_to_char (ch, "Description:\n\r%s", pMob->description);

    if (pMob->pShop) {
        SHOP_T *pShop;
        int iTrade;

        pShop = pMob->pShop;
        printf_to_char (ch, "Shop data for [%5d]:\n\r"
                            "  Markup for purchaser: %d%%\n\r"
                            "  Markdown for seller:  %d%%\n\r",
            pShop->keeper, pShop->profit_buy, pShop->profit_sell);

        printf_to_char (ch, "  Hours: %d to %d.\n\r",
            pShop->open_hour, pShop->close_hour);

        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
            if (pShop->buy_type[iTrade] == 0)
                continue;
            if (iTrade == 0) {
                send_to_char ("  Number Trades Type\n\r", ch);
                send_to_char ("  ------ -----------\n\r", ch);
            }
            printf_to_char (ch, "  [%4d] %s\n\r", iTrade,
                item_get_name (pShop->buy_type[iTrade]));
        }
    }

    if (pMob->mprogs) {
        int cnt;

        printf_to_char (ch, "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);
        for (cnt = 0, list = pMob->mprogs; list; list = list->next) {
            if (cnt == 0) {
                send_to_char (" Number Vnum Trigger Phrase\n\r", ch);
                send_to_char (" ------ ---- ------- ------\n\r", ch);
            }

            printf_to_char (ch, "[%5d] %4d %7s %s\n\r", cnt,
                 list->vnum, mprog_type_to_name (list->trig_type),
                 list->trig_phrase);
            cnt++;
        }
    }
    return FALSE;
}

MEDIT (medit_create) {
    MOB_INDEX_T *pMob;
    AREA_T *pArea;
    int value;
    int iHash;

    value = atoi (argument);
    RETURN_IF (argument[0] == '\0' || value == 0,
        "Syntax: medit create [vnum]\n\r", ch, FALSE);

    pArea = area_get_by_inner_vnum (value);
    RETURN_IF (!pArea,
        "MEdit: That vnum is not assigned an area.\n\r", ch, FALSE);
    RETURN_IF (!IS_BUILDER (ch, pArea),
        "MEdit: Vnum in an area you cannot build in.\n\r", ch, FALSE);
    RETURN_IF (get_mob_index (value),
        "MEdit: Mobile vnum already exists.\n\r", ch, FALSE);

    pMob = mob_index_new ();
    pMob->area = pArea;
    pMob->vnum = value;
    pMob->anum = value - pArea->min_vnum;

    if (value > top_vnum_mob)
        top_vnum_mob = value;

    pMob->mob_plus = MOB_IS_NPC;
    iHash = value % MAX_KEY_HASH;
    LIST_FRONT (pMob, next, mob_index_hash[iHash]);
    ch->desc->pEdit = (void *) pMob;

    db_finalize_mob (pMob);

    send_to_char ("Mobile created.\n\r", ch);
    return TRUE;
}

MEDIT (medit_spec) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: spec [special function]\n\r", ch, FALSE);

    if (!str_cmp (argument, "none")) {
        pMob->spec_fun = NULL;
        send_to_char ("Spec removed.\n\r", ch);
        return TRUE;
    }
    else if (spec_lookup (argument) >= 0) {
        pMob->spec_fun = spec_lookup_function (argument);
        send_to_char ("Spec set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("MEdit: No such special function.\n\r", ch);
    return FALSE;
}

MEDIT (medit_damtype) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: damtype [damage message]\n\r"
        "For a list of damage types, type '? weapon'.\n\r", ch, FALSE);

    pMob->dam_type = attack_lookup (argument);
    send_to_char ("Damage type set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_align) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_sh_int_replace (ch, &(pMob->alignment), argument,
        "Syntax: alignment [number]\n\r",
        "Alignment set.\n\r");
}

MEDIT (medit_level) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_sh_int_replace (ch, &(pMob->level), argument,
        "Syntax: level [number]\n\r",
        "Level set.\n\r");
}

MEDIT (medit_desc) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        string_append (ch, &pMob->description);
        return TRUE;
    }
    send_to_char ("Syntax: desc    - line edit\n\r", ch);
    return FALSE;
}

MEDIT (medit_long) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: long [string]\n\r", ch, FALSE);

    strcat (argument, "\n\r");
    argument[0] = UPPER (argument[0]);
    str_replace_dup (&(pMob->long_descr), argument);

    send_to_char ("Long description set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_short) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_str_replace_dup (ch, &(pMob->short_descr), argument,
        "Syntax: short [string]\n\r",
        "Short description set.\n\r");
}

MEDIT (medit_name) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_str_replace_dup (ch, &(pMob->name), argument,
        "Syntax: name [string]\n\r",
        "Name set.\n\r");
}

MEDIT (medit_shop) {
    MOB_INDEX_T *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    argument = one_argument (argument, arg1);

    EDIT_MOB (ch, pMob);

    RETURN_IF (command[0] == '\0',
        "Syntax: shop hours [#xopening] [#xclosing]\n\r"
        "        shop profit [#xbuying%] [#xselling%]\n\r"
        "        shop type [#x0-4] [item type]\n\r"
        "        shop assign\n\r"
        "        shop remove\n\r", ch, FALSE);

    if (!str_cmp (command, "hours")) {
        RETURN_IF (arg1[0] == '\0' || !is_number (arg1) ||
                   argument[0] == '\0' || !is_number (argument),
            "Syntax: shop hours [#xopening] [#xclosing]\n\r", ch, FALSE);
        RETURN_IF (!pMob->pShop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);

        pMob->pShop->open_hour  = atoi (arg1);
        pMob->pShop->close_hour = atoi (argument);

        send_to_char ("Shop hours set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "profit")) {
        RETURN_IF (arg1[0] == '\0' || !is_number (arg1) ||
                   argument[0] == '\0' || !is_number (argument),
            "Syntax: shop profit [#xbuying%] [#xselling%]\n\r", ch, FALSE);
        RETURN_IF (!pMob->pShop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);

        pMob->pShop->profit_buy = atoi (arg1);
        pMob->pShop->profit_sell = atoi (argument);

        send_to_char ("Shop profit set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "type")) {
        int value;

        RETURN_IF (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0',
            "Syntax: shop type [#x0-4] [item type]\n\r", ch, FALSE);
        if (atoi (arg1) >= MAX_TRADE) {
            printf_to_char (ch, "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
            return FALSE;
        }
        RETURN_IF (!pMob->pShop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);
        RETURN_IF ((value = item_lookup (argument)) < 0,
            "MEdit: That type of item is not known.\n\r", ch, FALSE);
        pMob->pShop->buy_type[atoi (arg1)] = value;

        send_to_char ("Shop type set.\n\r", ch);
        return TRUE;
    }

    /* shop assign && shop delete by Phoenix */
    if (!str_prefix (command, "assign")) {
        RETURN_IF (pMob->pShop,
            "Mob already has a shop assigned to it.\n\r", ch, FALSE);

        pMob->pShop = shop_new ();
        LISTB_BACK (pMob->pShop, next, shop_first, shop_last);
        pMob->pShop->keeper = pMob->vnum;

        send_to_char ("New shop assigned to mobile.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (command, "remove")) {
        LISTB_REMOVE (pMob->pShop, next, shop_first, shop_last,
            SHOP_T, NO_FAIL);
        shop_free (pMob->pShop);
        pMob->pShop = NULL;

        send_to_char ("Mobile is no longer a shopkeeper.\n\r", ch);
        return TRUE;
    }

    medit_shop (ch, "");
    return FALSE;
}

/* ROM medit functions: */
/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT (medit_sex) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (sex_types, argument)) != NO_FLAG) {
            pMob->sex = value;
            send_to_char ("Sex set.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: sex [sex]\n\r"
                  "Type '? sex' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT (medit_act) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (mob_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->mob_final, value);
            SET_BIT (pMob->mob_final, MOB_IS_NPC);
            pMob->mob_plus  = MISSING_BITS (pMob->mob_final, race_table[pMob->race].mob);
            pMob->mob_minus = MISSING_BITS (race_table[pMob->race].mob, pMob->mob_final);

            send_to_char ("Mob flag toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: act [flag]\n\r"
                  "Type '? act' for a list of flags.\n\r", ch);
    return FALSE;
}

/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT (medit_affect) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (affect_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->affected_by_final, value);
            pMob->affected_by_plus  = MISSING_BITS (pMob->affected_by_final, race_table[pMob->race].aff);
            pMob->affected_by_minus = MISSING_BITS (race_table[pMob->race].aff, pMob->affected_by_final);

            send_to_char ("Affect flag toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: affect [flag]\n\r"
                  "Type '? affect' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_ac) {
    MOB_INDEX_T *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    /* So that I can use break and send the syntax in one place */
    do {
        if (argument[0] == '\0')
            break;
        EDIT_MOB (ch, pMob);

        argument = one_argument (argument, arg);
        if (!is_number (arg))
            break;
        pierce = atoi (arg);

        argument = one_argument (argument, arg);
        if (arg[0] != '\0') {
            if (!is_number (arg))
                break;
            bash = atoi (arg);
            argument = one_argument (argument, arg);
        }
        else
            bash = pMob->ac[AC_BASH];

        if (arg[0] != '\0') {
            if (!is_number (arg))
                break;
            slash = atoi (arg);
            argument = one_argument (argument, arg);
        }
        else
            slash = pMob->ac[AC_SLASH];

        if (arg[0] != '\0') {
            if (!is_number (arg))
                break;
            exotic = atoi (arg);
        }
        else
            exotic = pMob->ac[AC_EXOTIC];

        pMob->ac[AC_PIERCE] = pierce;
        pMob->ac[AC_BASH]   = bash;
        pMob->ac[AC_SLASH]  = slash;
        pMob->ac[AC_EXOTIC] = exotic;

        send_to_char ("Ac set.\n\r", ch);
        return TRUE;
    }
    while (FALSE); /* Just do it once.. */

    send_to_char (
        "Syntax: ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
        "help MOB_AC gives a list of reasonable ac-values.\n\r", ch);
    return FALSE;
}

MEDIT (medit_form) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (form_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->form_plus, value);
            pMob->form_plus  = MISSING_BITS (pMob->form_final, race_table[pMob->race].form);
            pMob->form_minus = MISSING_BITS (race_table[pMob->race].form, pMob->form_final);

            send_to_char ("Form toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: form [flags]\n\r"
                  "Type '? form' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_part) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (part_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->parts_plus, value);
            pMob->parts_plus  = MISSING_BITS (pMob->parts_final, race_table[pMob->race].parts);
            pMob->parts_minus = MISSING_BITS (race_table[pMob->race].parts, pMob->parts_final);

            send_to_char ("Parts toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: part [flags]\n\r"
                  "Type '? part' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_imm) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->imm_flags_plus, value);
            pMob->imm_flags_plus  = MISSING_BITS (pMob->imm_flags_final, race_table[pMob->race].imm);
            pMob->imm_flags_minus = MISSING_BITS (race_table[pMob->race].imm, pMob->imm_flags_final);

            send_to_char ("Immunity toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: imm [flags]\n\r"
                  "Type '? imm' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_res) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->res_flags_plus, value);
            pMob->res_flags_plus  = MISSING_BITS (pMob->res_flags_final, race_table[pMob->race].res);
            pMob->res_flags_minus = MISSING_BITS (race_table[pMob->race].res, pMob->res_flags_final);

            send_to_char ("Resistance toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: res [flags]\n\r"
                  "Type '? res' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_vuln) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->vuln_flags_plus, value);
            pMob->vuln_flags_plus  = MISSING_BITS (pMob->vuln_flags_final, race_table[pMob->race].vuln);
            pMob->vuln_flags_minus = MISSING_BITS (race_table[pMob->race].vuln, pMob->vuln_flags_final);

            send_to_char ("Vulnerability toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: vuln [flags]\n\r"
                  "Type '? vuln' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_material) {
    MOB_INDEX_T *pMob;
    const MATERIAL_T *mat;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((mat = material_get_by_name (argument)) != NULL) {
            pMob->material = mat->type;
            send_to_char ("Material type changed.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: material [type]\n\r", ch);
    return FALSE;
}

MEDIT (medit_off) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (off_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->off_flags_plus, value);
            pMob->off_flags_plus  = MISSING_BITS (pMob->off_flags_final, race_table[pMob->race].off);
            pMob->off_flags_minus = MISSING_BITS (race_table[pMob->race].off, pMob->off_flags_final);

            send_to_char ("Offensive behaviour toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: off [flags]\n\r"
                  "Type '? off' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_size) {
    MOB_INDEX_T *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (size_types, argument)) != NO_FLAG) {
            pMob->size = value;
            send_to_char ("Size set.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: size [size]\n\r"
                  "Type '? size' for a list of sizes.\n\r", ch);
    return FALSE;
}

MEDIT (medit_hitdice) {
    static const char *syntax =
        "Syntax: hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_T *pMob;

    EDIT_MOB (ch, pMob);
    RETURN_IF (argument[0] == '\0', syntax, ch, FALSE);

    num = cp = argument;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;
    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    RETURN_IF ((!is_number (num)   || atoi (num)   < 1) ||
               (!is_number (type)  || atoi (type)  < 1) ||
               (!is_number (bonus) || atoi (bonus) < 0),
        syntax, ch, FALSE);

    pMob->hit.number = atoi (num);
    pMob->hit.size   = atoi (type);
    pMob->hit.bonus  = atoi (bonus);

    send_to_char ("Hitdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_manadice) {
    static const char *syntax =
        "Syntax: manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_T *pMob;

    EDIT_MOB (ch, pMob);
    RETURN_IF (argument[0] == '\0', syntax, ch, FALSE);

    num = cp = argument;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;
    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    RETURN_IF (!(is_number (num) && is_number (type) && is_number (bonus)),
        syntax, ch, FALSE);

    RETURN_IF ((!is_number (num)   || atoi (num)   < 1) ||
               (!is_number (type)  || atoi (type)  < 1) ||
               (!is_number (bonus) || atoi (bonus) < 0),
        syntax, ch, FALSE);

    pMob->mana.number = atoi (num);
    pMob->mana.size   = atoi (type);
    pMob->mana.bonus  = atoi (bonus);

    send_to_char ("Manadice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_damdice) {
    static const char *syntax =
        "Syntax: damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_T *pMob;

    EDIT_MOB (ch, pMob);
    RETURN_IF (argument[0] == '\0',
        syntax, ch, FALSE);

    num = cp = argument;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    type = cp;
    while (isdigit (*cp))
        ++cp;
    while (*cp != '\0' && !isdigit (*cp))
        *(cp++) = '\0';

    bonus = cp;
    while (isdigit (*cp))
        ++cp;
    if (*cp != '\0')
        *cp = '\0';

    RETURN_IF (!(is_number (num) && is_number (type) && is_number (bonus)),
        syntax, ch, FALSE);

    RETURN_IF ((!is_number (num)   || atoi (num)   < 1) ||
               (!is_number (type)  || atoi (type)  < 1) ||
               (!is_number (bonus) || atoi (bonus) < 0),
        syntax, ch, FALSE);

    pMob->damage.number = atoi (num);
    pMob->damage.size   = atoi (type);
    pMob->damage.bonus  = atoi (bonus);

    send_to_char ("Damdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_race) {
    MOB_INDEX_T *pMob;
    int race;

    if (argument[0] != '\0' && (race = race_lookup (argument)) >= 0) {
        EDIT_MOB (ch, pMob);
        pMob->race = race;
        db_finalize_mob (pMob);

        send_to_char ("Race set.\n\r", ch);
        return TRUE;
    }

    if (argument[0] == '?') {
        send_to_char ("Available races are:", ch);

        for (race = 0; race_table[race].name != NULL; race++) {
            if ((race % 3) == 0)
                send_to_char ("\n\r", ch);
            printf_to_char (ch, " %-15s", race_table[race].name);
        }

        send_to_char ("\n\r", ch);
        return FALSE;
    }
    send_to_char ("Syntax: race [race]\n\r"
                  "Type 'race ?' for a list of races.\n\r", ch);
    return FALSE;
}

MEDIT (medit_position) {
    MOB_INDEX_T *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument (argument, arg);
    switch (arg[0]) {
        default:
            break;

        case 'S':
        case 's':
            if (str_prefix (arg, "start"))
                break;
            if ((value = flag_value (position_types, argument)) == NO_FLAG)
                break;

            EDIT_MOB (ch, pMob);
            pMob->start_pos = value;
            send_to_char ("Start position set.\n\r", ch);
            return TRUE;

        case 'D':
        case 'd':
            if (str_prefix (arg, "default"))
                break;
            if ((value = flag_value (position_types, argument)) == NO_FLAG)
                break;

            EDIT_MOB (ch, pMob);
            pMob->default_pos = value;
            send_to_char ("Default position set.\n\r", ch);
            return TRUE;
    }

    send_to_char ("Syntax: position [start/default] [position]\n\r"
                  "Type '? position' for a list of positions.\n\r", ch);
    return FALSE;
}

MEDIT (medit_gold) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_long_int_replace (ch, &(pMob->wealth), argument,
        "Syntax: wealth [number]\n\r",
        "Wealth set.\n\r");
}

MEDIT (medit_hitroll) {
    MOB_INDEX_T *pMob;
    EDIT_MOB (ch, pMob);
    return olc_sh_int_replace (ch, &(pMob->hitroll), argument,
        "Syntax: hitroll [number]\n\r",
        "Hitroll set.\n\r");
}

MEDIT (medit_group) {
    MOB_INDEX_T *pMob;
    MOB_INDEX_T *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER_T *buffer;
    bool found = FALSE;

    EDIT_MOB (ch, pMob);
    RETURN_IF (argument[0] == '\0',
        "Syntax: group [number]\n\r"
        "        group show [number]\n\r", ch, FALSE);

    if (is_number (argument)) {
        pMob->group = atoi (argument);
        send_to_char ("Group set.\n\r", ch);
        return TRUE;
    }

    argument = one_argument (argument, arg);
    if (!strcmp (arg, "show") && is_number (argument)) {
        RETURN_IF (atoi (argument) == 0,
            "Are you crazy?\n\r", ch, FALSE);

        buffer = buf_new ();
        for (temp = 0; temp < 65536; temp++) {
            pMTemp = get_mob_index (temp);
            if (pMTemp && (pMTemp->group == atoi (argument))) {
                found = TRUE;
                sprintf (buf, "[%5d] %s\n\r", pMTemp->vnum, pMTemp->name);
                add_buf (buffer, buf);
            }
        }

        if (found)
            page_to_char (buf_string (buffer), ch);
        else
            send_to_char ("No mobs in that group.\n\r", ch);

        buf_free (buffer);
        return FALSE;
    }
    return FALSE;
}

MEDIT (medit_addmprog) {
    int value, vnum;
    MOB_INDEX_T *pMob;
    MPROG_LIST_T *list;
    MPROG_CODE_T *code;
    AREA_T *area;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_MOB (ch, pMob);
    argument = one_argument (argument, num);
    argument = one_argument (argument, trigger);
    argument = one_argument (argument, phrase);

    RETURN_IF (!is_number (num) || trigger[0] == '\0' || phrase[0] == '\0',
        "Syntax: addmprog [vnum] [trigger] [phrase]\n\r", ch, FALSE);

    vnum = atoi (num);
    area = area_get_by_inner_vnum (vnum);
    RETURN_IF (!area,
        "MEdit: That vnum is not assigned an area.\n\r", ch, FALSE);

    if ((value = flag_value (mprog_flags, trigger)) == NO_FLAG) {
        send_to_char ("Valid flags are:\n\r", ch);
        show_help (ch, "mprog");
        return FALSE;
    }
    RETURN_IF ((code = get_mprog_index (atoi (num))) == NULL,
        "No such mob program.\n\r", ch, FALSE);

    list = mprog_new ();
    list->area = area;
    list->vnum = vnum;
    list->anum = vnum - area->min_vnum;
    list->trig_type = value;
    list->trig_phrase = str_dup (phrase);
    list->code = code->code;
    SET_BIT (pMob->mprog_flags, value);
    LIST_FRONT (list, next, pMob->mprogs);

    send_to_char ("Mob program created.\n\r", ch);
    return TRUE;
}

MEDIT (medit_delmprog) {
    MOB_INDEX_T *pMob;
    MPROG_LIST_T *pList, *pList_prev;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB (ch, pMob);
    one_argument (argument, mprog);

    RETURN_IF (!is_number (mprog) || mprog[0] == '\0',
        "Syntax: delmprog [#mprog]\n\r", ch, FALSE);

    value = atoi (mprog);
    RETURN_IF (value < 0,
        "Only non-negative mprog-numbers allowed.\n\r", ch, FALSE);

    /* Find the affect and its previous link in the list. */
    LIST_FIND_WITH_PREV (value >= cnt++, next, pMob->mprogs,
        pList, pList_prev);
    RETURN_IF (!pList,
        "MEdit: Non-existant mob program.\n\r", ch, FALSE);

    LIST_REMOVE_WITH_PREV (pList, pList_prev, next, pMob->mprogs);
    REMOVE_BIT (pMob->mprog_flags, pList->trig_type);
    mprog_free (pList);

    send_to_char ("Mob program removed.\n\r", ch);
    return TRUE;
}
