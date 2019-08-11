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
#include "olc.h"
#include "mob_cmds.h"

#include "olc_medit.h"

MEDIT (medit_show) {
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    MPROG_LIST *list;

    EDIT_MOB (ch, pMob);

    sprintf (buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
             pMob->name, !pMob->area ? -1 : pMob->area->vnum,
             !pMob->area ? "No Area" : pMob->area->title);
    send_to_char (buf, ch);

    printf_to_char (ch, "Mob:         [%s]\n\r",
        flag_string (mob_flags, pMob->mob_orig));

    sprintf (buf, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n\r",
             pMob->vnum,
             pMob->sex == SEX_MALE ? "male   " :
             pMob->sex == SEX_FEMALE ? "female " :
             pMob->sex == 3 ? "random " : "neutral",
             race_table[pMob->race].name);
    send_to_char (buf, ch);

    sprintf (buf,
             "Level:       [%2d]    Align: [%4d]      Hitroll: [%2d] Dam Type:    [%s]\n\r",
             pMob->level, pMob->alignment,
             pMob->hitroll, attack_table[pMob->dam_type].name);
    send_to_char (buf, ch);

    if (pMob->group) {
        sprintf (buf, "Group:       [%5d]\n\r", pMob->group);
        send_to_char (buf, ch);
    }

    sprintf (buf, "Hit dice:    [%2dd%-3d+%4d] ",
             pMob->hit[DICE_NUMBER],
             pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS]);
    send_to_char (buf, ch);

    sprintf (buf, "Damage dice: [%2dd%-3d+%4d] ",
             pMob->damage[DICE_NUMBER],
             pMob->damage[DICE_TYPE], pMob->damage[DICE_BONUS]);
    send_to_char (buf, ch);

    sprintf (buf, "Mana dice:   [%2dd%-3d+%4d]\n\r",
             pMob->mana[DICE_NUMBER],
             pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS]);
    send_to_char (buf, ch);

    /* ROM values end */
    sprintf (buf, "Affected by: [%s]\n\r",
             flag_string (affect_flags, pMob->affected_by_orig));
    send_to_char (buf, ch);

    /* ROM values: */
    sprintf (buf,
             "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
             pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
             pMob->ac[AC_EXOTIC]);
    send_to_char (buf, ch);

    printf_to_char (ch, "Form:        [%s]\n\r", flag_string (form_flags, pMob->form_orig));
    printf_to_char (ch, "Parts:       [%s]\n\r", flag_string (part_flags, pMob->parts_orig));
    printf_to_char (ch, "Imm:         [%s]\n\r", flag_string (res_flags, pMob->imm_flags_orig));
    printf_to_char (ch, "Res:         [%s]\n\r", flag_string (res_flags, pMob->res_flags_orig));
    printf_to_char (ch, "Vuln:        [%s]\n\r", flag_string (res_flags, pMob->vuln_flags_orig));
    printf_to_char (ch, "Off:         [%s]\n\r", flag_string (off_flags, pMob->off_flags_orig));
    printf_to_char (ch, "Size:        [%s]\n\r", flag_string (size_types, pMob->size));
    printf_to_char (ch, "Material:    [%s]\n\r", material_get_name (pMob->material));
    printf_to_char (ch, "Start pos.   [%s]\n\r", flag_string (position_types, pMob->start_pos));
    printf_to_char (ch, "Default pos  [%s]\n\r", flag_string (position_types, pMob->default_pos));
    printf_to_char (ch, "Wealth:      [%5ld]\n\r", pMob->wealth);

    /* ROM values end */
    if (pMob->spec_fun) {
        sprintf (buf, "Spec fun:    [%s]\n\r",
            spec_function_name (pMob->spec_fun));
        send_to_char (buf, ch);
    }

    sprintf (buf, "Short descr: %s\n\rLong descr:\n\r%s",
             pMob->short_descr, pMob->long_descr);
    send_to_char (buf, ch);

    sprintf (buf, "Description:\n\r%s", pMob->description);
    send_to_char (buf, ch);

    if (pMob->pShop) {
        SHOP_DATA *pShop;
        int iTrade;

        pShop = pMob->pShop;
        sprintf (buf,
                 "Shop data for [%5d]:\n\r"
                 "  Markup for purchaser: %d%%\n\r"
                 "  Markdown for seller:  %d%%\n\r",
                 pShop->keeper, pShop->profit_buy, pShop->profit_sell);
        send_to_char (buf, ch);
        sprintf (buf, "  Hours: %d to %d.\n\r",
                 pShop->open_hour, pShop->close_hour);
        send_to_char (buf, ch);

        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
            if (pShop->buy_type[iTrade] == 0)
                continue;
            if (iTrade == 0) {
                send_to_char ("  Number Trades Type\n\r", ch);
                send_to_char ("  ------ -----------\n\r", ch);
            }
            sprintf (buf, "  [%4d] %s\n\r", iTrade,
                     item_get_name (pShop->buy_type[iTrade]));
            send_to_char (buf, ch);
        }
    }

    if (pMob->mprogs) {
        int cnt;

        sprintf (buf, "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);
        send_to_char (buf, ch);

        for (cnt = 0, list = pMob->mprogs; list; list = list->next) {
            if (cnt == 0) {
                send_to_char (" Number Vnum Trigger Phrase\n\r", ch);
                send_to_char (" ------ ---- ------- ------\n\r", ch);
            }

            sprintf (buf, "[%5d] %4d %7s %s\n\r", cnt,
                     list->vnum, mprog_type_to_name (list->trig_type),
                     list->trig_phrase);
            send_to_char (buf, ch);
            cnt++;
        }
    }

    return FALSE;
}

MEDIT (medit_create) {
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    int iHash;

    value = atoi (argument);
    if (argument[0] == '\0' || value == 0) {
        send_to_char ("Syntax:  medit create [vnum]\n\r", ch);
        return FALSE;
    }

    pArea = area_get_by_inner_vnum (value);
    if (!pArea) {
        send_to_char ("MEdit:  That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("MEdit:  Vnum in an area you cannot build in.\n\r", ch);
        return FALSE;
    }
    if (get_mob_index (value)) {
        send_to_char ("MEdit:  Mobile vnum already exists.\n\r", ch);
        return FALSE;
    }

    pMob = mob_index_new ();
    pMob->area = pArea;
    pMob->vnum = value;
    pMob->anum = value - pArea->min_vnum;

    if (value > top_vnum_mob)
        top_vnum_mob = value;

    pMob->mob = MOB_IS_NPC;
    iHash = value % MAX_KEY_HASH;
    LIST_FRONT (pMob, next, mob_index_hash[iHash]);
    ch->desc->pEdit = (void *) pMob;

    send_to_char ("Mobile Created.\n\r", ch);
    return TRUE;
}

MEDIT (medit_spec) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  spec [special function]\n\r", ch);
        return FALSE;
    }
    if (!str_cmp (argument, "none")) {
        pMob->spec_fun = NULL;
        send_to_char ("Spec removed.\n\r", ch);
        return TRUE;
    }
    if (spec_lookup (argument) >= 0) {
        pMob->spec_fun = spec_lookup_function (argument);
        send_to_char ("Spec set.\n\r", ch);
        return TRUE;
    }

    send_to_char ("MEdit: No such special function.\n\r", ch);
    return FALSE;
}

MEDIT (medit_damtype) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  damtype [damage message]\n\r", ch);
        send_to_char
            ("For a list of damage types, type '? weapon'.\n\r",
             ch);
        return FALSE;
    }

    pMob->dam_type = attack_lookup (argument);
    send_to_char ("Damage type set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_align) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  alignment [number]\n\r", ch);
        return FALSE;
    }

    pMob->alignment = atoi (argument);
    send_to_char ("Alignment set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_level) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  level [number]\n\r", ch);
        return FALSE;
    }

    pMob->level = atoi (argument);
    send_to_char ("Level set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_desc) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        string_append (ch, &pMob->description);
        return TRUE;
    }
    send_to_char ("Syntax:  desc    - line edit\n\r", ch);
    return FALSE;
}

MEDIT (medit_long) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  long [string]\n\r", ch);
        return FALSE;
    }

    str_free (pMob->long_descr);
    strcat (argument, "\n\r");
    pMob->long_descr = str_dup (argument);
    pMob->long_descr[0] = UPPER (pMob->long_descr[0]);

    send_to_char ("Long description set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_short) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  short [string]\n\r", ch);
        return FALSE;
    }
    str_free (pMob->short_descr);
    pMob->short_descr = str_dup (argument);

    send_to_char ("Short description set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_name) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax:  name [string]\n\r", ch);
        return FALSE;
    }

    str_replace_dup (&pMob->name, argument);
    send_to_char ("Name set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_shop) {
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument (argument, command);
    argument = one_argument (argument, arg1);

    EDIT_MOB (ch, pMob);

    if (command[0] == '\0') {
        send_to_char ("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
        send_to_char ("         shop profit [#xbuying%] [#xselling%]\n\r", ch);
        send_to_char ("         shop type [#x0-4] [item type]\n\r", ch);
        send_to_char ("         shop assign\n\r", ch);
        send_to_char ("         shop remove\n\r", ch);
        return FALSE;
    }

    if (!str_cmp (command, "hours")) {
        if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0' || !is_number (argument)) {
            send_to_char ("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
            return FALSE;
        }
        if (!pMob->pShop) {
            send_to_char ("MEdit:  You must create the shop first (shop assign).\n\r", ch);
            return FALSE;
        }

        pMob->pShop->open_hour = atoi (arg1);
        pMob->pShop->close_hour = atoi (argument);

        send_to_char ("Shop hours set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "profit")) {
        if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0' || !is_number (argument)) {
            send_to_char ("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch);
            return FALSE;
        }
        if (!pMob->pShop) {
            send_to_char ("MEdit:  You must create the shop first (shop assign).\n\r", ch);
            return FALSE;
        }

        pMob->pShop->profit_buy = atoi (arg1);
        pMob->pShop->profit_sell = atoi (argument);

        send_to_char ("Shop profit set.\n\r", ch);
        return TRUE;
    }

    if (!str_cmp (command, "type")) {
        char buf[MAX_INPUT_LENGTH];
        int value;

        if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0') {
            send_to_char ("Syntax:  shop type [#x0-4] [item type]\n\r", ch);
            return FALSE;
        }
        if (atoi (arg1) >= MAX_TRADE) {
            sprintf (buf, "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
            send_to_char (buf, ch);
            return FALSE;
        }
        if (!pMob->pShop) {
            send_to_char ("MEdit:  You must create the shop first (shop assign).\n\r", ch);
            return FALSE;
        }
        if ((value = item_lookup (argument)) < 0) {
            send_to_char ("MEdit:  That type of item is not known.\n\r", ch);
            return FALSE;
        }
        pMob->pShop->buy_type[atoi (arg1)] = value;

        send_to_char ("Shop type set.\n\r", ch);
        return TRUE;
    }

    /* shop assign && shop delete by Phoenix */
    if (!str_prefix (command, "assign")) {
        if (pMob->pShop) {
            send_to_char ("Mob already has a shop assigned to it.\n\r", ch);
            return FALSE;
        }

        pMob->pShop = shop_new ();
        LISTB_BACK (pMob->pShop, next, shop_first, shop_last);
        pMob->pShop->keeper = pMob->vnum;

        send_to_char ("New shop assigned to mobile.\n\r", ch);
        return TRUE;
    }

    if (!str_prefix (command, "remove")) {
        LISTB_REMOVE (pMob->pShop, next, shop_first, shop_last,
            SHOP_DATA, NO_FAIL);
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
    MOB_INDEX_DATA *pMob;
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
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (mob_flags, argument)) != NO_FLAG) {
            pMob->mob_orig ^= value;
            SET_BIT (pMob->mob_orig, MOB_IS_NPC);

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
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (affect_flags, argument)) != NO_FLAG) {
            TOGGLE_BIT (pMob->affected_by_orig, value);
            send_to_char ("Affect flag toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: affect [flag]\n\r"
                  "Type '? affect' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_ac) {
    MOB_INDEX_DATA *pMob;
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
        pMob->ac[AC_BASH] = bash;
        pMob->ac[AC_SLASH] = slash;
        pMob->ac[AC_EXOTIC] = exotic;

        send_to_char ("Ac set.\n\r", ch);
        return TRUE;
    }
    while (FALSE); /* Just do it once.. */

    send_to_char
        ("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
         "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
    return FALSE;
}

MEDIT (medit_form) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (form_flags, argument)) != NO_FLAG) {
            pMob->form_orig ^= value;
            send_to_char ("Form toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: form [flags]\n\r"
                  "Type '? form' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_part) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (part_flags, argument)) != NO_FLAG) {
            pMob->parts_orig ^= value;
            send_to_char ("Parts toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: part [flags]\n\r"
                  "Type '? part' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_imm) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            pMob->imm_flags_orig ^= value;
            send_to_char ("Immunity toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: imm [flags]\n\r"
                  "Type '? imm' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_res) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            pMob->res_flags_orig ^= value;
            send_to_char ("Resistance toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char ("Syntax: res [flags]\n\r"
                  "Type '? res' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_vuln) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (res_flags, argument)) != NO_FLAG) {
            pMob->vuln_flags_orig ^= value;
            send_to_char ("Vulnerability toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: vuln [flags]\n\r"
                  "Type '? vuln' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_material) {
    MOB_INDEX_DATA *pMob;
    const MATERIAL_TYPE *mat;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((mat = material_get_by_name (argument)) != NULL) {
            str_replace_dup (&(pMob->material_str), mat->name);
            pMob->material = mat->type;
            send_to_char ("Material type changed.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax:  material [type]\n\r", ch);
    return FALSE;
}

MEDIT (medit_off) {
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0') {
        EDIT_MOB (ch, pMob);
        if ((value = flag_value (off_flags, argument)) != NO_FLAG) {
            pMob->off_flags ^= value;
            send_to_char ("Offensive behaviour toggled.\n\r", ch);
            return TRUE;
        }
    }
    send_to_char ("Syntax: off [flags]\n\r"
                  "Type '? off' for a list of flags.\n\r", ch);
    return FALSE;
}

MEDIT (medit_size) {
    MOB_INDEX_DATA *pMob;
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
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);
    if (argument[0] == '\0') {
        send_to_char (syntax, ch);
        return FALSE;
    }

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

    if ((!is_number (num) || atoi (num) < 1)
        || (!is_number (type) || atoi (type) < 1)
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        send_to_char (syntax, ch);
        return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi (num);
    pMob->hit[DICE_TYPE] = atoi (type);
    pMob->hit[DICE_BONUS] = atoi (bonus);

    send_to_char ("Hitdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_manadice) {
    static char syntax[] =
        "Syntax:  manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);
    if (argument[0] == '\0') {
        send_to_char (syntax, ch);
        return FALSE;
    }

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

    if (!(is_number (num) && is_number (type) && is_number (bonus))) {
        send_to_char (syntax, ch);
        return FALSE;
    }

    if ((!is_number (num) || atoi (num) < 1)
        || (!is_number (type) || atoi (type) < 1)
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        send_to_char (syntax, ch);
        return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi (num);
    pMob->mana[DICE_TYPE] = atoi (type);
    pMob->mana[DICE_BONUS] = atoi (bonus);

    send_to_char ("Manadice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_damdice) {
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB (ch, pMob);
    if (argument[0] == '\0') {
        send_to_char (syntax, ch);
        return FALSE;
    }

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

    if (!(is_number (num) && is_number (type) && is_number (bonus))) {
        send_to_char (syntax, ch);
        return FALSE;
    }

    if ((!is_number (num) || atoi (num) < 1)
        || (!is_number (type) || atoi (type) < 1)
        || (!is_number (bonus) || atoi (bonus) < 0))
    {
        send_to_char (syntax, ch);
        return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi (num);
    pMob->damage[DICE_TYPE] = atoi (type);
    pMob->damage[DICE_BONUS] = atoi (bonus);

    send_to_char ("Damdice set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_race) {
    MOB_INDEX_DATA *pMob;
    int race;

    if (argument[0] != '\0' && (race = race_lookup (argument)) >= 0) {
        EDIT_MOB (ch, pMob);
        pMob->race = race;

        pMob->mob         = pMob->mob_orig         | race_table[race].mob;
        pMob->affected_by = pMob->affected_by_orig | race_table[race].aff;
        pMob->off_flags   = pMob->off_flags_orig   | race_table[race].off;
        pMob->imm_flags   = pMob->imm_flags_orig   | race_table[race].imm;
        pMob->res_flags   = pMob->res_flags_orig   | race_table[race].res;
        pMob->vuln_flags  = pMob->vuln_flags_orig  | race_table[race].vuln;
        pMob->form        = pMob->form_orig        | race_table[race].form;
        pMob->parts       = pMob->parts_orig       | race_table[race].parts;

        send_to_char ("Race set.\n\r", ch);
        return TRUE;
    }

    if (argument[0] == '?') {
        char buf[MAX_STRING_LENGTH];
        send_to_char ("Available races are:", ch);

        for (race = 0; race_table[race].name != NULL; race++) {
            if ((race % 3) == 0)
                send_to_char ("\n\r", ch);
            sprintf (buf, " %-15s", race_table[race].name);
            send_to_char (buf, ch);
        }

        send_to_char ("\n\r", ch);
        return FALSE;
    }
    send_to_char ("Syntax:  race [race]\n\r"
                  "Type 'race ?' for a list of races.\n\r", ch);
    return FALSE;
}

MEDIT (medit_position) {
    MOB_INDEX_DATA *pMob;
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

    send_to_char ("Syntax:  position [start/default] [position]\n\r"
                  "Type '? position' for a list of positions.\n\r", ch);
    return FALSE;
}

MEDIT (medit_gold) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  wealth [number]\n\r", ch);
        return FALSE;
    }

    pMob->wealth = atoi (argument);
    send_to_char ("Wealth set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_hitroll) {
    MOB_INDEX_DATA *pMob;
    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0' || !is_number (argument)) {
        send_to_char ("Syntax:  hitroll [number]\n\r", ch);
        return FALSE;
    }

    pMob->hitroll = atoi (argument);
    send_to_char ("Hitroll set.\n\r", ch);
    return TRUE;
}

MEDIT (medit_group) {
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER *buffer;
    bool found = FALSE;

    EDIT_MOB (ch, pMob);

    if (argument[0] == '\0') {
        send_to_char ("Syntax: group [number]\n\r", ch);
        send_to_char ("        group show [number]\n\r", ch);
        return FALSE;
    }
    if (is_number (argument)) {
        pMob->group = atoi (argument);
        send_to_char ("Group set.\n\r", ch);
        return TRUE;
    }

    argument = one_argument (argument, arg);
    if (!strcmp (arg, "show") && is_number (argument)) {
        if (atoi (argument) == 0) {
            send_to_char ("Are you crazy?\n\r", ch);
            return FALSE;
        }

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
    MOB_INDEX_DATA *pMob;
    MPROG_LIST *list;
    MPROG_CODE *code;
    AREA_DATA *area;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_MOB (ch, pMob);
    argument = one_argument (argument, num);
    argument = one_argument (argument, trigger);
    argument = one_argument (argument, phrase);

    if (!is_number (num) || trigger[0] == '\0' || phrase[0] == '\0') {
        send_to_char ("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r", ch);
        return FALSE;
    }

    vnum = atoi (num);
    area = area_get_by_inner_vnum (vnum);
    if (!area) {
        send_to_char ("MEdit:  That vnum is not assigned an area.\n\r", ch);
        return FALSE;
    }
    if ((value = flag_value (mprog_flags, trigger)) == NO_FLAG) {
        send_to_char ("Valid flags are:\n\r", ch);
        show_help (ch, "mprog");
        return FALSE;
    }
    if ((code = get_mprog_index (atoi (num))) == NULL) {
        send_to_char ("No such MOBProgram.\n\r", ch);
        return FALSE;
    }

    list = mprog_new ();
    list->area = area;
    list->vnum = vnum;
    list->anum = vnum - area->min_vnum;
    list->trig_type = value;
    list->trig_phrase = str_dup (phrase);
    list->code = code->code;
    SET_BIT (pMob->mprog_flags, value);
    LIST_FRONT (list, next, pMob->mprogs);

    send_to_char ("Mprog Added.\n\r", ch);
    return TRUE;
}

MEDIT (medit_delmprog) {
    MOB_INDEX_DATA *pMob;
    MPROG_LIST *pList, *pList_prev;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB (ch, pMob);
    one_argument (argument, mprog);

    if (!is_number (mprog) || mprog[0] == '\0') {
        send_to_char ("Syntax:  delmprog [#mprog]\n\r", ch);
        return FALSE;
    }

    value = atoi (mprog);
    if (value < 0) {
        send_to_char ("Only non-negative mprog-numbers allowed.\n\r", ch);
        return FALSE;
    }

    /* Find the affect and its previous link in the list. */
    LIST_FIND_WITH_PREV (value >= cnt++, next, pMob->mprogs,
        pList, pList_prev);
    if (!pList) {
        send_to_char ("MEdit:  Non-existant mprog.\n\r", ch);
        return FALSE;
    }

    LIST_REMOVE_WITH_PREV (pList, pList_prev, next, pMob->mprogs);
    REMOVE_BIT (pMob->mprog_flags, pList->trig_type);
    mprog_free (pList);

    send_to_char ("Mprog removed.\n\r", ch);
    return TRUE;
}
