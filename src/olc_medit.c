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

#include "olc_medit.h"

#include "chars.h"
#include "comm.h"
#include "db.h"
#include "globals.h"
#include "interp.h"
#include "lookup.h"
#include "memory.h"
#include "mob_cmds.h"
#include "mob_prog.h"
#include "mobiles.h"
#include "olc.h"
#include "recycle.h"
#include "string.h"
#include "tables.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

MEDIT (medit_show) {
    MOB_INDEX_T *mob;
    MPROG_LIST_T *list;
    EDIT_MOB (ch, mob);

    printf_to_char (ch, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
        mob->name, !mob->area ? -1 : mob->area->vnum,
        !mob->area ? "No Area" : mob->area->title);

    printf_to_char (ch, "Mob:         [%s]\n\r",
        ext_flags_to_string (mob_flags, mob->ext_mob_final));

    printf_to_char (ch, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n\r",
         mob->vnum,
         mob->sex == SEX_MALE   ? "male   " :
         mob->sex == SEX_FEMALE ? "female " :
         mob->sex == 3          ? "random " : "neutral",
         race_get_name (mob->race));

    printf_to_char (ch, "Level:       [%2d]    Align: [%4d]      "
                        "Hitroll: [%2d] Dam Type:    [%s]\n\r",
        mob->level, mob->alignment, mob->hitroll,
        attack_table[mob->attack_type].name);

    if (mob->group)
        printf_to_char (ch, "Group:       [%5d]\n\r", mob->group);

    printf_to_char (ch, "Hit dice:    [%2dd%-3d+%4d] ",
        mob->hit.number, mob->hit.size, mob->hit.bonus);

    printf_to_char (ch, "Damage dice: [%2dd%-3d+%4d] ",
        mob->damage.number, mob->damage.size, mob->damage.bonus);

    printf_to_char (ch, "Mana dice:   [%2dd%-3d+%4d]\n\r",
        mob->mana.number, mob->mana.size, mob->mana.bonus);

    /* ROM values end */
    printf_to_char (ch, "Affected by: [%s]\n\r",
        flags_to_string (affect_flags, mob->affected_by_final));

    /* ROM values: */
    printf_to_char (ch, "Armor:       [pierce: %d  bash: %d  "
                        "slash: %d  magic: %d]\n\r",
        mob->ac[AC_PIERCE], mob->ac[AC_BASH], mob->ac[AC_SLASH],
        mob->ac[AC_EXOTIC]);

    printf_to_char (ch, "Form:        [%s]\n\r", flags_to_string (form_flags, mob->form_final));
    printf_to_char (ch, "Parts:       [%s]\n\r", flags_to_string (part_flags, mob->parts_final));
    printf_to_char (ch, "Imm:         [%s]\n\r", flags_to_string (res_flags, mob->imm_flags_final));
    printf_to_char (ch, "Res:         [%s]\n\r", flags_to_string (res_flags, mob->res_flags_final));
    printf_to_char (ch, "Vuln:        [%s]\n\r", flags_to_string (res_flags, mob->vuln_flags_final));
    printf_to_char (ch, "Off:         [%s]\n\r", flags_to_string (off_flags, mob->off_flags_final));
    printf_to_char (ch, "Size:        [%s]\n\r", type_get_name (size_types, mob->size));
    printf_to_char (ch, "Material:    [%s]\n\r", material_get_name (mob->material));
    printf_to_char (ch, "Start pos.   [%s]\n\r", type_get_name (position_types, mob->start_pos));
    printf_to_char (ch, "Default pos  [%s]\n\r", type_get_name (position_types, mob->default_pos));
    printf_to_char (ch, "Wealth:      [%5ld]\n\r", mob->wealth);

    /* ROM values end */
    if (mob->spec_fun)
        printf_to_char (ch, "Spec fun:    [%s]\n\r",
            spec_function_name (mob->spec_fun));

    printf_to_char (ch, "Short descr: %s\n\rLong descr:\n\r%s",
        mob->short_descr, mob->long_descr);

    printf_to_char (ch, "Description:\n\r%s", mob->description);

    if (mob->shop) {
        SHOP_T *shop;
        int trade;

        shop = mob->shop;
        printf_to_char (ch, "Shop data for [%5d]:\n\r"
                            "  Markup for purchaser: %d%%\n\r"
                            "  Markdown for seller:  %d%%\n\r",
            shop->keeper, shop->profit_buy, shop->profit_sell);

        printf_to_char (ch, "  Hours: %d to %d.\n\r",
            shop->open_hour, shop->close_hour);

        for (trade = 0; trade < MAX_TRADE; trade++) {
            if (shop->buy_type[trade] == 0)
                continue;
            if (trade == 0) {
                send_to_char ("  Number Trades Type\n\r", ch);
                send_to_char ("  ------ -----------\n\r", ch);
            }
            printf_to_char (ch, "  [%4d] %s\n\r", trade,
                item_get_name (shop->buy_type[trade]));
        }
    }

    if (mob->mprog_first) {
        int cnt;

        printf_to_char (ch, "\n\rMOBPrograms for [%5d]:\n\r", mob->vnum);
        for (cnt = 0, list = mob->mprog_first; list; list = list->mob_next) {
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
    MOB_INDEX_T *mob;
    AREA_T *area;
    int value;

    value = atoi (argument);
    RETURN_IF (argument[0] == '\0' || value == 0,
        "Syntax: medit create [vnum]\n\r", ch, FALSE);

    area = area_get_by_inner_vnum (value);
    RETURN_IF (!area,
        "MEdit: That vnum is not assigned an area.\n\r", ch, FALSE);
    RETURN_IF (!IS_BUILDER (ch, area),
        "MEdit: Vnum in an area you cannot build in.\n\r", ch, FALSE);
    RETURN_IF (mobile_get_index (value),
        "MEdit: Mobile vnum already exists.\n\r", ch, FALSE);

    mob = mob_index_new ();
    mob_index_to_area (mob, area);
    mob->vnum = value;
    mob->anum = value - area->min_vnum;
    mob->ext_mob_plus = EXT_BITS (MOB_IS_NPC);

    if (value > top_vnum_mob)
        top_vnum_mob = value;

    mob_index_to_hash (mob);
    ch->desc->olc_edit = (void *) mob;

    db_finalize_mob (mob);

    send_to_char ("Mobile created.\n\r", ch);
    return TRUE;
}

MEDIT (medit_spec) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: spec [special function]\n\r", ch, FALSE);

    if (!str_cmp (argument, "none")) {
        mob->spec_fun = NULL;
        send_to_char ("Spec removed.\n\r", ch);
        return TRUE;
    }
    else if (spec_lookup (argument) >= 0) {
        mob->spec_fun = spec_lookup_function (argument);
        send_to_char ("Spec set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("MEdit: No such special function.\n\r", ch);
    return FALSE;
}

MEDIT (medit_damtype) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: damtype [damage message]\n\r"
        "For a list of damage types, type '? weapon'.\n\r", ch, FALSE);

    mob->attack_type = attack_lookup (argument);
    send_to_char ("Damage type set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_align) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_sh_int_replace (ch, &(mob->alignment), argument,
        "Syntax: alignment [number]\n\r",
        "Alignment set.\n\r");
}

MEDIT (medit_level) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_sh_int_replace (ch, &(mob->level), argument,
        "Syntax: level [number]\n\r",
        "Level set.\n\r");
}

MEDIT (medit_desc) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);

    if (argument[0] == '\0') {
        string_append (ch, &mob->description);
        return TRUE;
    }
    send_to_char ("Syntax: desc    - line edit\n\r", ch);
    return FALSE;
}

MEDIT (medit_long) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);

    RETURN_IF (argument[0] == '\0',
        "Syntax: long [string]\n\r", ch, FALSE);

    strcat (argument, "\n\r");
    argument[0] = UPPER (argument[0]);
    str_replace_dup (&(mob->long_descr), argument);

    send_to_char ("Long description set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_short) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_str_replace_dup (ch, &(mob->short_descr), argument,
        "Syntax: short [string]\n\r",
        "Short description set.\n\r");
}

MEDIT (medit_name) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_str_replace_dup (ch, &(mob->name), argument,
        "Syntax: name [string]\n\r",
        "Name set.\n\r");
}

MEDIT (medit_shop) {
    MOB_INDEX_T *mob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    argument = one_argument (argument, arg1);

    EDIT_MOB (ch, mob);

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
        RETURN_IF (!mob->shop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);

        mob->shop->open_hour  = atoi (arg1);
        mob->shop->close_hour = atoi (argument);

        send_to_char ("Shop hours set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "profit")) {
        RETURN_IF (arg1[0] == '\0' || !is_number (arg1) ||
                   argument[0] == '\0' || !is_number (argument),
            "Syntax: shop profit [#xbuying%] [#xselling%]\n\r", ch, FALSE);
        RETURN_IF (!mob->shop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);

        mob->shop->profit_buy = atoi (arg1);
        mob->shop->profit_sell = atoi (argument);

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
        RETURN_IF (!mob->shop,
            "MEdit: You must create the shop first (shop assign).\n\r", ch, FALSE);
        RETURN_IF ((value = item_lookup (argument)) < 0,
            "MEdit: That type of item is not known.\n\r", ch, FALSE);
        mob->shop->buy_type[atoi (arg1)] = value;

        send_to_char ("Shop type set.\n\r", ch);
        return TRUE;
    }

    /* shop assign && shop delete by Phoenix */
    if (!str_prefix (command, "assign")) {
        RETURN_IF (mob->shop,
            "Mob already has a shop assigned to it.\n\r", ch, FALSE);

        mob->shop = shop_new ();
        LIST2_BACK (mob->shop, global_prev, global_next, shop_first, shop_last);
        mob->shop->keeper = mob->vnum;

        send_to_char ("New shop assigned to mobile.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (command, "remove")) {
        LIST2_REMOVE (mob->shop, global_prev, global_next, shop_first, shop_last);
        shop_free (mob->shop);
        mob->shop = NULL;

        send_to_char ("Mobile is no longer a shopkeeper.\n\r", ch);
        return TRUE;
    }

    medit_shop (ch, "");
    return FALSE;
}

/* ROM medit functions: */
/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT (medit_sex) {
    MOB_INDEX_T *mob;
    type_t value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = type_lookup (sex_types, argument)) != TYPE_NONE) {
            mob->sex = value;
            send_to_char ("Sex set.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: sex [sex]\n\r"
                  "Type '? sex' for a list of flags.\n\r", ch);
    return FALSE;
}

static void medit_update_flags (flag_t final, flag_t race_flags, flag_t *plus,
    flag_t *minus)
{
    *plus  = final & ~race_flags;
    *minus = race_flags & ~final;
}

static void medit_update_ext_flags (EXT_FLAGS_T final, EXT_FLAGS_T race_flags,
    EXT_FLAGS_T *plus, EXT_FLAGS_T *minus)
{
    *plus  = EXT_WITHOUT_MANY (final, race_flags);
    *minus = EXT_WITHOUT_MANY (race_flags, final);
}

#define UPDATE_FLAGS(m, f, rf) \
    medit_update_flags ((m)->f ## _final, \
        race_get ((m)->race)->rf, \
        &((m)->f ## _plus), \
        &((m)->f ## _minus))

#define UPDATE_EXT_FLAGS(m, f, rf) \
    medit_update_ext_flags ((m)->f ## _final, \
        race_get ((m)->race)->rf, \
        &((m)->f ## _plus), \
        &((m)->f ## _minus))

/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT (medit_act) {
    MOB_INDEX_T *mob;
    EXT_FLAGS_T flags;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        flags = ext_flags_from_string (mob_flags, argument);
        if (EXT_IS_NONZERO (flags)) {
            EXT_TOGGLE_MANY (mob->ext_mob_final, flags);
            EXT_SET (mob->ext_mob_final, MOB_IS_NPC);
            UPDATE_EXT_FLAGS (mob, ext_mob, ext_mob);

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
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (affect_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->affected_by_final, value);
            UPDATE_FLAGS (mob, affected_by, aff);

            send_to_char ("Affect flag toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: affect [flag]\n\r"
                  "Type '? affect' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_ac) {
    MOB_INDEX_T *mob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    /* So that I can use break and send the syntax in one place */
    do {
        if (argument[0] == '\0')
            break;
        EDIT_MOB (ch, mob);

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
            bash = mob->ac[AC_BASH];

        if (arg[0] != '\0') {
            if (!is_number (arg))
                break;
            slash = atoi (arg);
            argument = one_argument (argument, arg);
        }
        else
            slash = mob->ac[AC_SLASH];

        if (arg[0] != '\0') {
            if (!is_number (arg))
                break;
            exotic = atoi (arg);
        }
        else
            exotic = mob->ac[AC_EXOTIC];

        mob->ac[AC_PIERCE] = pierce;
        mob->ac[AC_BASH]   = bash;
        mob->ac[AC_SLASH]  = slash;
        mob->ac[AC_EXOTIC] = exotic;

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
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (form_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->form_plus, value);
            UPDATE_FLAGS (mob, form, form);

            send_to_char ("Form toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: form [flags]\n\r"
                  "Type '? form' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_part) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (part_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->parts_plus, value);
            UPDATE_FLAGS (mob, parts, parts);

            send_to_char ("Parts toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: part [flags]\n\r"
                  "Type '? part' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_imm) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (res_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->imm_flags_plus, value);
            UPDATE_FLAGS (mob, imm_flags, imm);

            send_to_char ("Immunity toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: imm [flags]\n\r"
                  "Type '? imm' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_res) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (res_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->res_flags_plus, value);
            UPDATE_FLAGS (mob, res_flags, res);

            send_to_char ("Resistance toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: res [flags]\n\r"
                  "Type '? res' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_vuln) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (res_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->vuln_flags_plus, value);
            UPDATE_FLAGS (mob, vuln_flags, vuln);

            send_to_char ("Vulnerability toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: vuln [flags]\n\r"
                  "Type '? vuln' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_material) {
    MOB_INDEX_T *mob;
    const MATERIAL_T *mat;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((mat = material_get_by_name (argument)) != NULL) {
            mob->material = mat->type;
            send_to_char ("Material type changed.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: material [type]\n\r", ch);
    return FALSE;
}

MEDIT (medit_off) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = flags_from_string (off_flags, argument)) != FLAG_NONE) {
            TOGGLE_BIT (mob->off_flags_plus, value);
            UPDATE_FLAGS (mob, off_flags, off);

            send_to_char ("Offensive behaviour toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: off [flags]\n\r"
                  "Type '? off' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_size) {
    MOB_INDEX_T *mob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, mob);
        if ((value = type_lookup (size_types, argument)) != TYPE_NONE) {
            mob->size = value;
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
    MOB_INDEX_T *mob;

    EDIT_MOB (ch, mob);
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

    mob->hit.number = atoi (num);
    mob->hit.size   = atoi (type);
    mob->hit.bonus  = atoi (bonus);

    send_to_char ("Hitdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_manadice) {
    static const char *syntax =
        "Syntax: manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_T *mob;

    EDIT_MOB (ch, mob);
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

    mob->mana.number = atoi (num);
    mob->mana.size   = atoi (type);
    mob->mana.bonus  = atoi (bonus);

    send_to_char ("Manadice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_damdice) {
    static const char *syntax =
        "Syntax: damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_T *mob;

    EDIT_MOB (ch, mob);
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

    mob->damage.number = atoi (num);
    mob->damage.size   = atoi (type);
    mob->damage.bonus  = atoi (bonus);

    send_to_char ("Damdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_race) {
    const RACE_T *race;
    MOB_INDEX_T *mob;
    int i;

    if (argument[0] != '\0' && (i = race_lookup (argument)) >= 0) {
        EDIT_MOB (ch, mob);
        mob->race = i;
        db_finalize_mob (mob);

        send_to_char ("Race set.\n\r", ch);
        return TRUE;
    }

    if (argument[0] == '?') {
        send_to_char ("Available races are:", ch);

        for (i = 0; i < RACE_MAX; i++) {
            if ((race = race_get (i)) == NULL)
                break;
            if ((i % 3) == 0)
                send_to_char ("\n\r", ch);
            printf_to_char (ch, " %-15s", race->name);
        }

        send_to_char ("\n\r", ch);
        return FALSE;
    }
    send_to_char ("Syntax: race [race]\n\r"
                  "Type 'race ?' for a list of races.\n\r", ch);
    return FALSE;
}

MEDIT (medit_position) {
    MOB_INDEX_T *mob;
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
            if ((value = type_lookup (position_types, argument)) == TYPE_NONE)
                break;

            EDIT_MOB (ch, mob);
            mob->start_pos = value;
            send_to_char ("Start position set.\n\r", ch);
            return TRUE;

        case 'D':
        case 'd':
            if (str_prefix (arg, "default"))
                break;
            if ((value = type_lookup (position_types, argument)) == TYPE_NONE)
                break;

            EDIT_MOB (ch, mob);
            mob->default_pos = value;
            send_to_char ("Default position set.\n\r", ch);
            return TRUE;
    }

    send_to_char ("Syntax: position [start/default] [position]\n\r"
                  "Type '? position' for a list of positions.\n\r", ch);
    return FALSE;
}

MEDIT (medit_gold) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_long_int_replace (ch, &(mob->wealth), argument,
        "Syntax: wealth [number]\n\r",
        "Wealth set.\n\r");
}

MEDIT (medit_hitroll) {
    MOB_INDEX_T *mob;
    EDIT_MOB (ch, mob);
    return olc_sh_int_replace (ch, &(mob->hitroll), argument,
        "Syntax: hitroll [number]\n\r",
        "Hitroll set.\n\r");
}

MEDIT (medit_group) {
    MOB_INDEX_T *mob;
    MOB_INDEX_T *mob_temp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER_T *buffer;
    bool found = FALSE;

    EDIT_MOB (ch, mob);
    RETURN_IF (argument[0] == '\0',
        "Syntax: group [number]\n\r"
        "        group show [number]\n\r", ch, FALSE);

    if (is_number (argument)) {
        mob->group = atoi (argument);
        send_to_char ("Group set.\n\r", ch);
        return TRUE;
    }

    argument = one_argument (argument, arg);
    if (!strcmp (arg, "show") && is_number (argument)) {
        RETURN_IF (atoi (argument) == 0,
            "Are you crazy?\n\r", ch, FALSE);

        buffer = buf_new ();
        for (temp = 0; temp < 65536; temp++) {
            mob_temp = mobile_get_index (temp);
            if (mob_temp && (mob_temp->group == atoi (argument))) {
                found = TRUE;
                sprintf (buf, "[%5d] %s\n\r", mob_temp->vnum, mob_temp->name);
                buf_cat (buffer, buf);
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
    MOB_INDEX_T *mob;
    MPROG_LIST_T *list;
    MPROG_CODE_T *code;
    AREA_T *area;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_MOB (ch, mob);
    argument = one_argument (argument, num);
    argument = one_argument (argument, trigger);
    argument = one_argument (argument, phrase);

    RETURN_IF (!is_number (num) || trigger[0] == '\0' || phrase[0] == '\0',
        "Syntax: addmprog [vnum] [trigger] [phrase]\n\r", ch, FALSE);

    vnum = atoi (num);
    area = area_get_by_inner_vnum (vnum);
    RETURN_IF (!area,
        "MEdit: That vnum is not assigned an area.\n\r", ch, FALSE);

    if ((value = flags_from_string (mprog_flags, trigger)) == FLAG_NONE) {
        send_to_char ("Valid flags are:\n\r", ch);
        show_help (ch, "mprog");
        return FALSE;
    }
    RETURN_IF ((code = mpcode_get_index (atoi (num))) == NULL,
        "No such mob program.\n\r", ch, FALSE);

    list = mprog_new ();
    mprog_to_area (list, area);
    list->vnum = vnum;
    list->anum = vnum - area->min_vnum;
    list->trig_type = value;
    list->trig_phrase = str_dup (phrase);
    list->code = code->code;
    SET_BIT (mob->mprog_flags, value);
    LIST2_FRONT (list, mob_prev, mob_next, mob->mprog_first, mob->mprog_last);

    send_to_char ("Mob program created.\n\r", ch);
    return TRUE;
}

MEDIT (medit_delmprog) {
    MOB_INDEX_T *mob;
    MPROG_LIST_T *mplist;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB (ch, mob);
    one_argument (argument, mprog);

    RETURN_IF (!is_number (mprog) || mprog[0] == '\0',
        "Syntax: delmprog [#mprog]\n\r", ch, FALSE);

    value = atoi (mprog);
    RETURN_IF (value < 0,
        "Only non-negative mprog-numbers allowed.\n\r", ch, FALSE);

    /* Find the affect and its previous link in the list. */
    LIST_FIND (value >= cnt++, mob_next, mob->mprog_first, mplist);
    RETURN_IF (!mplist,
        "MEdit: Non-existant mob program.\n\r", ch, FALSE);

    LIST2_REMOVE (mplist, mob_prev, mob_next,
        mob->mprog_first, mob->mprog_last);
    REMOVE_BIT (mob->mprog_flags, mplist->trig_type);
    mprog_free (mplist);

    send_to_char ("Mob program removed.\n\r", ch);
    return TRUE;
}
