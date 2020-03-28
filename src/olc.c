/***************************************************************************
 *  File: olc.c                                                            *
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

#include "utils.h"
#include "interp.h"
#include "comm.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include "magic.h"
#include "chars.h"
#include "memory.h"

#include "act_olc.h"
#include "olc_aedit.h"
#include "olc_hedit.h"
#include "olc_medit.h"
#include "olc_mpedit.h"
#include "olc_oedit.h"
#include "olc_redit.h"

#include "olc.h"

const EDITOR_CMD_T editor_table[] = {
    {"area",   do_aedit},
    {"room",   do_redit},
    {"object", do_oedit},
    {"mobile", do_medit},
    {"mpcode", do_mpedit},
    {"hedit",  do_hedit},
    {0}
};

const OLC_CMD_T aedit_table[] = {
    {"age",      aedit_age},
    {"builder",  aedit_builder}, /* s removed -- Hugin */
    {"commands", show_commands},
    {"create",   aedit_create},
    {"filename", aedit_file},
    {"name",     aedit_title},
/*  {"recall",   aedit_recall}, ROM OLC */
    {"reset",    aedit_reset},
    {"security", aedit_security},
    {"show",     aedit_show},
    {"vnum",     aedit_vnum},
    {"lvnum",    aedit_lvnum},
    {"uvnum",    aedit_uvnum},
    {"credits",  aedit_credits},
    {"?",        show_help},
    {"version",  show_version},
    {0}
};

const OLC_CMD_T redit_table[] = {
    /* We want single-letter navigation to always be possible. */
    {"n",        redit_north},
    {"s",        redit_south},
    {"e",        redit_east},
    {"w",        redit_west},
    {"u",        redit_up},
    {"d",        redit_down},

    {"commands", show_commands},
    {"create",   redit_create},
    {"desc",     redit_desc},
    {"ed",       redit_ed},
    {"format",   redit_format},
    {"name",     redit_name},
    {"show",     redit_show},
    {"heal",     redit_heal},
    {"mana",     redit_mana},
    {"clan",     redit_clan},
    {"portal",   redit_portal},

    {"north",    redit_north},
    {"south",    redit_south},
    {"east",     redit_east},
    {"west",     redit_west},
    {"up",       redit_up},
    {"down",     redit_down},

    /* New reset commands. */
    {"mreset",   redit_mreset},
    {"oreset",   redit_oreset},
    {"mlist",    redit_mlist},
    {"rlist",    redit_rlist},
    {"olist",    redit_olist},
    {"mshow",    redit_mshow},
    {"oshow",    redit_oshow},
    {"owner",    redit_owner},
    {"room",     redit_room},
    {"sector",   redit_sector},

    {"?",        show_help},
    {"version",  show_version},
    {0},
};

const OLC_CMD_T oedit_table[] = {
    {"addaffect", oedit_addaffect},
    {"addapply",  oedit_addapply},
    {"commands",  show_commands},
    {"cost",      oedit_cost},
    {"create",    oedit_create},
    {"delaffect", oedit_delaffect},
    {"ed",        oedit_ed},
    {"long",      oedit_long},
    {"name",      oedit_name},
    {"short",     oedit_short},
    {"show",      oedit_show},
    {"v0",        oedit_value0},
    {"v1",        oedit_value1},
    {"v2",        oedit_value2},
    {"v3",        oedit_value3},
    {"v4",        oedit_value4},    /* ROM */
    {"weight",    oedit_weight},

    {"extra",     oedit_extra},     /* ROM */
    {"wear",      oedit_wear},      /* ROM */
    {"type",      oedit_type},      /* ROM */
    {"material",  oedit_material},  /* ROM */
    {"level",     oedit_level},     /* ROM */
    {"condition", oedit_condition}, /* ROM */

    {"?",         show_help},
    {"version",   show_version},
    {0},
};

const OLC_CMD_T medit_table[] = {
    {"alignment", medit_align},
    {"commands",  show_commands},
    {"create",    medit_create},
    {"desc",      medit_desc},
    {"level",     medit_level},
    {"long",      medit_long},
    {"name",      medit_name},
    {"shop",      medit_shop},
    {"short",     medit_short},
    {"show",      medit_show},
    {"spec",      medit_spec},

    {"sex",       medit_sex},      /* ROM */
    {"act",       medit_act},      /* ROM */
    {"affect",    medit_affect},   /* ROM */
    {"armor",     medit_ac},       /* ROM */
    {"form",      medit_form},     /* ROM */
    {"part",      medit_part},     /* ROM */
    {"imm",       medit_imm},      /* ROM */
    {"res",       medit_res},      /* ROM */
    {"vuln",      medit_vuln},     /* ROM */
    {"material",  medit_material}, /* ROM */
    {"off",       medit_off},      /* ROM */
    {"size",      medit_size},     /* ROM */
    {"hitdice",   medit_hitdice},  /* ROM */
    {"manadice",  medit_manadice}, /* ROM */
    {"damdice",   medit_damdice},  /* ROM */
    {"race",      medit_race},     /* ROM */
    {"position",  medit_position}, /* ROM */
    {"wealth",    medit_gold},     /* ROM */
    {"hitroll",   medit_hitroll},  /* ROM */
    {"damtype",   medit_damtype},  /* ROM */
    {"group",     medit_group},    /* ROM */
    {"addmprog",  medit_addmprog}, /* ROM */
    {"delmprog",  medit_delmprog}, /* ROM */

    {"?",         show_help},
    {"version",   show_version},
    {0}
};

const OLC_CMD_T mpedit_table[] = {
    {"commands", show_commands},
    {"create",   mpedit_create},
    {"code",     mpedit_code},
    {"show",     mpedit_show},
    {"list",     mpedit_list},
    {"?",        show_help},
    {0}
};

const OLC_CMD_T hedit_table[] = {
    {"keyword",  hedit_keyword},
    {"text",     hedit_text},
    {"new",      hedit_new},
    {"level",    hedit_level},
    {"commands", show_commands},
    {"delete",   hedit_delete},
    {"list",     hedit_list},
    {"show",     hedit_show},
    {"?",        show_help},
    {0}
};

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor (DESCRIPTOR_T *d) {
    switch (d->editor) {
        case ED_AREA:   aedit  (d->character, d->incomm); break;
        case ED_ROOM:   redit  (d->character, d->incomm); break;
        case ED_OBJECT: oedit  (d->character, d->incomm); break;
        case ED_MOBILE: medit  (d->character, d->incomm); break;
        case ED_MPCODE: mpedit (d->character, d->incomm); break;
        case ED_HELP:   hedit  (d->character, d->incomm); break;
        default:
            return FALSE;
    }
    return TRUE;
}

const char *olc_ed_name (CHAR_T *ch) {
    switch (ch->desc->editor) {
        case ED_AREA:   return "AEdit";  break;
        case ED_ROOM:   return "REdit";  break;
        case ED_OBJECT: return "OEdit";  break;
        case ED_MOBILE: return "MEdit";  break;
        case ED_MPCODE: return "MPEdit"; break;
        case ED_HELP:   return "HEdit";  break;
        default:        return " ";      break;
    }
}

char *olc_ed_vnum (CHAR_T *ch) {
    AREA_T *area;
    ROOM_INDEX_T *room;
    OBJ_INDEX_T *obj;
    MOB_INDEX_T *mob;
    MPROG_CODE_T *mprog;
    HELP_T *help;
    static char buf[MIL];

    buf[0] = '\0';
    switch (ch->desc->editor) {
        case ED_AREA:
            area = (AREA_T *) ch->desc->olc_edit;
            sprintf (buf, "%d", area ? area->vnum : 0);
            break;
        case ED_ROOM:
            room = ch->in_room;
            sprintf (buf, "%d", room ? room->vnum : 0);
            break;
        case ED_OBJECT:
            obj = (OBJ_INDEX_T *) ch->desc->olc_edit;
            sprintf (buf, "%d", obj ? obj->vnum : 0);
            break;
        case ED_MOBILE:
            mob = (MOB_INDEX_T *) ch->desc->olc_edit;
            sprintf (buf, "%d", mob ? mob->vnum : 0);
            break;
        case ED_MPCODE:
            mprog = (MPROG_CODE_T *) ch->desc->olc_edit;
            sprintf (buf, "%d", mprog ? mprog->vnum : 0);
            break;
        case ED_HELP:
            help = (HELP_T *) ch->desc->olc_edit;
            sprintf (buf, "%s", help ? help->keyword : "");
            break;
        default:
            sprintf (buf, " ");
            break;
    }
    return buf;
}

/*****************************************************************************
 Name:       show_olc_cmds
 Purpose:    Format up the commands from given table.
 Called by:  show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds (CHAR_T *ch, const struct olc_cmd_type *olc_table) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name != NULL; cmd++) {
        sprintf (buf, "%-15.15s", olc_table[cmd].name);
        strcat (buf1, buf);
        if (++col % 5 == 0)
            strcat (buf1, "\n\r");
    }
    if (col % 5 != 0)
        strcat (buf1, "\n\r");

    send_to_char (buf1, ch);
}

/*****************************************************************************
 Name:       show_commands
 Purpose:    Display all olc commands.
 Called by:  olc interpreters.
 ****************************************************************************/
bool show_commands (CHAR_T *ch, char *argument) {
    switch (ch->desc->editor) {
        case ED_AREA:   show_olc_cmds (ch, aedit_table);  break;
        case ED_ROOM:   show_olc_cmds (ch, redit_table);  break;
        case ED_OBJECT: show_olc_cmds (ch, oedit_table);  break;
        case ED_MOBILE: show_olc_cmds (ch, medit_table);  break;
        case ED_MPCODE: show_olc_cmds (ch, mpedit_table); break;
        case ED_HELP:   show_olc_cmds (ch, hedit_table);  break;
    }
    return FALSE;
}

/*****************************************************************************
 Name:        edit_done
 Purpose:    Resets builder information on completion.
 Called by:    aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done (CHAR_T *ch) {
    printf_to_char (ch, "Exiting OLC.\n\r");
    ch->desc->olc_edit = NULL;
    ch->desc->editor = 0;
    return FALSE;
}

/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/

/* Area Interpreter, called by do_aedit. */
void aedit (CHAR_T *ch, char *argument) {
    AREA_T *area;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;
    int value;

    EDIT_AREA (ch, area);
    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, area)) {
        printf_to_char (ch, "AEdit:  Insufficient security to modify area.\n\r");
        edit_done (ch);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }
    if (command[0] == '\0') {
        aedit_show (ch, argument);
        return;
    }
    if ((value = flags_from_string_exact (area_flags, command)) != FLAG_NONE) {
        TOGGLE_BIT (area->area_flags, value);
        printf_to_char (ch, "Flag toggled.\n\r");
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; aedit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, aedit_table[cmd].name)) {
            if ((*aedit_table[cmd].olc_fun) (ch, argument)) {
                SET_BIT (area->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
}

/* Room Interpreter, called by do_redit. */
void redit (CHAR_T *ch, char *argument) {
    AREA_T *area;
    ROOM_INDEX_T *room;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    EDIT_ROOM (ch, room);
    area = room->area;

    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, area)) {
        printf_to_char (ch, "REdit:  Insufficient security to modify room.\n\r");
        edit_done (ch);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }
    if (command[0] == '\0') {
        redit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; redit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, redit_table[cmd].name)) {
            if ((*redit_table[cmd].olc_fun) (ch, argument)) {
                SET_BIT (area->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
}

/* Object Interpreter, called by do_oedit. */
void oedit (CHAR_T *ch, char *argument) {
    AREA_T *area;
    OBJ_INDEX_T *obj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_OBJ (ch, obj);
    area = obj->area;

    if (!IS_BUILDER (ch, area)) {
        printf_to_char (ch, "OEdit: Insufficient security to modify area.\n\r");
        edit_done (ch);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }
    if (command[0] == '\0') {
        oedit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; oedit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, oedit_table[cmd].name)) {
            if ((*oedit_table[cmd].olc_fun) (ch, argument)) {
                SET_BIT (area->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
}

/* Mobile Interpreter, called by do_medit. */
void medit (CHAR_T *ch, char *argument) {
    AREA_T *area;
    MOB_INDEX_T *mob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd;

    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_MOB (ch, mob);
    area = mob->area;

    if (!IS_BUILDER (ch, area)) {
        printf_to_char (ch, "MEdit: Insufficient security to modify area.\n\r");
        edit_done (ch);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }
    if (command[0] == '\0') {
        medit_show (ch, argument);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; medit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, medit_table[cmd].name)) {
            if ((*medit_table[cmd].olc_fun) (ch, argument)) {
                SET_BIT (area->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
}

void mpedit (CHAR_T *ch, char *argument) {
    MPROG_CODE_T *mcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_T *ad;

    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_MPCODE (ch, mcode);

    if (mcode) {
        ad = area_get_by_inner_vnum (mcode->vnum);
        if (ad == NULL) { /* ??? */
            edit_done (ch);
            return;
        }
        if (!IS_BUILDER (ch, ad)) {
            printf_to_char (ch, "MPEdit: Insufficient security to modify code.\n\r");
            edit_done (ch);
            return;
        }
    }

    if (command[0] == '\0') {
        mpedit_show (ch, argument);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }
    for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, mpedit_table[cmd].name)) {
            if ((*mpedit_table[cmd].olc_fun) (ch, argument) && mcode)
                if ((ad = area_get_by_inner_vnum (mcode->vnum)) != NULL)
                    SET_BIT (ad->area_flags, AREA_CHANGED);
            return;
        }
    }

    interpret (ch, arg);
}

void hedit (CHAR_T *ch, char *argument) {
    HELP_T *help;
    HELP_AREA_T *had;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    str_smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_HELP (ch, help);

    had = help_area_get_by_help (help);
    if (had == NULL) {
        bugf ("hedit : had for help %s NULL", help->keyword);
        edit_done (ch);
        return;
    }
    if (ch->pcdata->security < 9) {
        printf_to_char (ch, "HEdit: Insufficient security to edit helps.\n\r");
        edit_done (ch);
        return;
    }
    if (command[0] == '\0') {
        hedit_show (ch, argument);
        return;
    }
    if (!str_cmp (command, "done")) {
        edit_done (ch);
        return;
    }

    for (cmd = 0; hedit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, hedit_table[cmd].name)) {
            if ((*hedit_table[cmd].olc_fun) (ch, argument))
                had->changed = TRUE;
            return;
        }
    }

    interpret (ch, arg);
}

bool show_version (CHAR_T *ch, char *argument) {
    send_to_char (OLC_VERSION, ch);
    printf_to_char (ch, "\n\r");
    send_to_char (OLC_AUTHOR, ch);
    printf_to_char (ch, "\n\r");
    send_to_char (OLC_DATE, ch);
    printf_to_char (ch, "\n\r");
    send_to_char (OLC_CREDITS, ch);
    printf_to_char (ch, "\n\r");
    return FALSE;
}

void show_liqlist (CHAR_T *ch) {
    int liq;
    BUFFER_T *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = buf_new ();

    for (liq = 0; liq_table[liq].name != NULL; liq++) {
        if ((liq % 21) == 0)
            buf_cat (buffer,
                     "Name                 Color          Proof Full Thirst Food Ssize\n\r");

        sprintf (buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
            liq_table[liq].name, liq_table[liq].color,
            liq_table[liq].cond[COND_DRUNK],  liq_table[liq].cond[COND_FULL],
            liq_table[liq].cond[COND_THIRST], liq_table[liq].cond[COND_HUNGER],
            liq_table[liq].serving_size);
        buf_cat (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}

void show_damlist (CHAR_T *ch) {
    int att;
    BUFFER_T *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = buf_new ();

    for (att = 0; attack_table[att].name != NULL; att++) {
        if ((att % 21) == 0)
            buf_cat (buffer, "Name                 Noun\n\r");
        sprintf (buf, "%-20s %-20s\n\r",
                 attack_table[att].name, attack_table[att].noun);
        buf_cat (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}

/*****************************************************************************
 Name:       show_flag_cmds
 Purpose:    Displays settable flags and stats.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds (CHAR_T *ch, const FLAG_T *flag_table) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int i, col;

    buf1[0] = '\0';
    col = 0;
    for (i = 0; flag_table[i].name != NULL; i++) {
        if (flag_table[i].settable) {
            sprintf (buf, "%-19.18s", flag_table[i].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\n\r");
        }
    }
    if (col % 4 != 0)
        strcat (buf1, "\n\r");

    send_to_char (buf1, ch);
}

void show_type_cmds (CHAR_T *ch, const TYPE_T *type_table) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int i, col;

    buf1[0] = '\0';
    col = 0;
    for (i = 0; type_table[i].name != NULL; i++) {
        if (type_table[i].settable) {
            sprintf (buf, "%-19.18s", type_table[i].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\n\r");
        }
    }
    if (col % 4 != 0)
        strcat (buf1, "\n\r");

    send_to_char (buf1, ch);
}

/*****************************************************************************
 Name:        show_skill_cmds
 Purpose:    Displays all skill functions.
         Does remove those damn immortal commands from the list.
         Could be improved by:
         (1) Adding a check for a particular class.
         (2) Adding a check for a level range.
 Called by:    show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds (CHAR_T *ch, int tar) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH * 2];
    int sn;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < SKILL_MAX && skill_table[sn].name; sn++) {
        if (!str_cmp (skill_table[sn].name, "reserved")
            || skill_table[sn].spell_fun == spell_null)
            continue;

        if (tar == -1 || skill_table[sn].target == tar) {
            sprintf (buf, "%-19.18s", skill_table[sn].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\n\r");
        }
    }

    if (col % 4 != 0)
        strcat (buf1, "\n\r");
    send_to_char (buf1, ch);
}

/*****************************************************************************
 Name:       show_spec_cmds
 Purpose:    Displays settable special functions.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds (CHAR_T *ch) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int spec;
    int col;

    buf1[0] = '\0';
    col = 0;
    printf_to_char (ch, "Preceed special functions with 'spec_'\n\r\n\r");
    for (spec = 0; spec_table[spec].function != NULL; spec++) {
        sprintf (buf, "%-19.18s", &spec_table[spec].name[5]);
        strcat (buf1, buf);
        if (++col % 4 == 0)
            strcat (buf1, "\n\r");
    }

    if (col % 4 != 0)
        strcat (buf1, "\n\r");

    send_to_char (buf1, ch);
}

/*****************************************************************************
 Name:       show_help
 Purpose:    Displays help for many tables used in OLC.
 Called by:  olc interpreters.
 ****************************************************************************/
bool show_help (CHAR_T *ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument (argument, arg);
    one_argument (argument, spell);

    /* Display syntax.  */
    if (arg[0] == '\0') {
        printf_to_char (ch, "Syntax:  ? [command]\n\r\n\r");
        printf_to_char (ch, "[command]          [description]\n\r");
        for (cnt = 0; master_table[cnt].table != NULL; cnt++)
            printf_to_char (ch, "%-18.18s -%s\n\r",
                master_table[cnt].name, master_table[cnt].description);
        return FALSE;
    }

    /* Find the command, show changeable data.
     * --------------------------------------- */
    for (cnt = 0; master_table[cnt].name != NULL; cnt++) {
        if (!(arg[0] == master_table[cnt].name[0] &&
              !str_prefix (arg, master_table[cnt].name)))
            continue;

        if (master_table[cnt].table == spec_table) {
            show_spec_cmds (ch);
            return FALSE;
        }
        else if (master_table[cnt].table == liq_table) {
            show_liqlist (ch);
            return FALSE;
        }
        else if (master_table[cnt].table == attack_table) {
            show_damlist (ch);
            return FALSE;
        }
        else if (master_table[cnt].table == skill_table) {
            if (spell[0] == '\0') {
                send_to_char (
                    "Syntax:  ? spells "
                    "[ignore/attack/defend/self/object/all]\n\r", ch);
                return FALSE;
            }
            if (!str_prefix (spell, "all"))
                show_skill_cmds (ch, -1);
            else if (!str_prefix (spell, "ignore"))
                show_skill_cmds (ch, SKILL_TARGET_IGNORE);
            else if (!str_prefix (spell, "attack"))
                show_skill_cmds (ch, SKILL_TARGET_CHAR_OFFENSIVE);
            else if (!str_prefix (spell, "defend"))
                show_skill_cmds (ch, SKILL_TARGET_CHAR_DEFENSIVE);
            else if (!str_prefix (spell, "self"))
                show_skill_cmds (ch, SKILL_TARGET_CHAR_SELF);
            else if (!str_prefix (spell, "object"))
                show_skill_cmds (ch, SKILL_TARGET_OBJ_INV);
            else {
                send_to_char (
                    "Syntax:  ? spell "
                    "[ignore/attack/defend/self/object/all]\n\r", ch);
            }
            return FALSE;
        }
        else if (master_table[cnt].type == TABLE_FLAGS) {
            show_flag_cmds (ch, master_table[cnt].table);
            return FALSE;
        }
        else if (master_table[cnt].type == TABLE_TYPES) {
            show_type_cmds (ch, master_table[cnt].table);
            return FALSE;
        }
        else {
            printf_to_char (ch, "No help for table '%s'.\n\r",
                master_table[cnt].name);
            return FALSE;
        }
    }

    show_help (ch, "");
    return FALSE;
}

bool olc_str_replace_dup (CHAR_T *ch, char **old_str, char *new_str,
    char *syntax_msg, char *success_msg)
{
    RETURN_IF (new_str == NULL || new_str[0] == '\0',
        syntax_msg, ch, FALSE);
    str_replace_dup (old_str, new_str);
    send_to_char (success_msg, ch);
    return TRUE;
}

bool olc_int_replace (CHAR_T *ch, int *old_val, char *new_val,
    char *syntax_msg, char *success_msg)
{
    char buf[MAX_STRING_LENGTH];
    one_argument (new_val, buf);

    RETURN_IF (buf == NULL || buf[0] == '\0' || !is_number (buf),
        syntax_msg, ch, FALSE);
    *old_val = atoi (buf);
    send_to_char (success_msg, ch);
    return TRUE;
}

bool olc_sh_int_replace (CHAR_T *ch, sh_int *old_val, char *new_val,
    char *syntax_msg, char *success_msg)
{
    char buf[MAX_STRING_LENGTH];
    one_argument (new_val, buf);

    RETURN_IF (buf == NULL || buf[0] == '\0' || !is_number (buf),
        syntax_msg, ch, FALSE);
    *old_val = atoi (buf);
    send_to_char (success_msg, ch);
    return TRUE;
}

bool olc_long_int_replace (CHAR_T *ch, long int *old_val, char *new_val,
    char *syntax_msg, char *success_msg)
{
    char buf[MAX_STRING_LENGTH];
    one_argument (new_val, buf);

    RETURN_IF (buf == NULL || buf[0] == '\0' || !is_number (buf),
        syntax_msg, ch, FALSE);
    *old_val = atol (buf);
    send_to_char (success_msg, ch);
    return TRUE;
}
