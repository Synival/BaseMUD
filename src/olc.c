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

#include "act_olc.h"
#include "olc_aedit.h"
#include "olc_hedit.h"
#include "olc_medit.h"
#include "olc_mpedit.h"
#include "olc_oedit.h"
#include "olc_redit.h"

#include "olc.h"

/* TODO: find out exactly how and where these functions are fun. */
/* TODO: test these functions thoroughly to understand what they do! */
/* TODO: do a first-pass clean-up. */
/* TODO: remove or at least trim-down the gargantuan comment blocks... */

/* Global variables. */
int top_vnum_room = 0;
int top_vnum_mob  = 0;
int top_vnum_obj  = 0;

/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct editor_cmd_type editor_table[] = {
    {"area",   do_aedit},
    {"room",   do_redit},
    {"object", do_oedit},
    {"mobile", do_medit},
    {"mpcode", do_mpedit},
    {"hedit",  do_hedit},
    {0}
};

const struct olc_cmd_type aedit_table[] = {
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

const struct olc_cmd_type hedit_table[] = {
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

const struct olc_cmd_type medit_table[] = {
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

const struct olc_cmd_type mpedit_table[] = {
    {"commands", show_commands},
    {"create",   mpedit_create},
    {"code",     mpedit_code},
    {"show",     mpedit_show},
    {"list",     mpedit_list},
    {"?",        show_help},
    {0}
};

const struct olc_cmd_type oedit_table[] = {
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

const struct olc_cmd_type redit_table[] = {
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

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor (DESCRIPTOR_DATA * d) {
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

char *olc_ed_name (CHAR_DATA * ch) {
    static char buf[10];

    buf[0] = '\0';
    switch (ch->desc->editor) {
        case ED_AREA:   sprintf (buf, "AEdit");  break;
        case ED_ROOM:   sprintf (buf, "REdit");  break;
        case ED_OBJECT: sprintf (buf, "OEdit");  break;
        case ED_MOBILE: sprintf (buf, "MEdit");  break;
        case ED_MPCODE: sprintf (buf, "MPEdit"); break;
        case ED_HELP:   sprintf (buf, "HEdit");  break;
        default:        sprintf (buf, " ");      break;
    }
    return buf;
}

char *olc_ed_vnum (CHAR_DATA * ch) {
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    MPROG_CODE *pMprog;
    HELP_DATA *pHelp;
    static char buf[MIL];

    buf[0] = '\0';
    switch (ch->desc->editor) {
        case ED_AREA:
            pArea = (AREA_DATA *) ch->desc->pEdit;
            sprintf (buf, "%d", pArea ? pArea->vnum : 0);
            break;
        case ED_ROOM:
            pRoom = ch->in_room;
            sprintf (buf, "%d", pRoom ? pRoom->vnum : 0);
            break;
        case ED_OBJECT:
            pObj = (OBJ_INDEX_DATA *) ch->desc->pEdit;
            sprintf (buf, "%d", pObj ? pObj->vnum : 0);
            break;
        case ED_MOBILE:
            pMob = (MOB_INDEX_DATA *) ch->desc->pEdit;
            sprintf (buf, "%d", pMob ? pMob->vnum : 0);
            break;
        case ED_MPCODE:
            pMprog = (MPROG_CODE *) ch->desc->pEdit;
            sprintf (buf, "%d", pMprog ? pMprog->vnum : 0);
            break;
        case ED_HELP:
            pHelp = (HELP_DATA *) ch->desc->pEdit;
            sprintf (buf, "%s", pHelp ? pHelp->keyword : "");
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
void show_olc_cmds (CHAR_DATA * ch, const struct olc_cmd_type *olc_table) {
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
bool show_commands (CHAR_DATA * ch, char *argument) {
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
bool edit_done (CHAR_DATA * ch) {
    send_to_char ("Exiting OLC.\n\r", ch);
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;
    return FALSE;
}

/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/

/* Area Interpreter, called by do_aedit. */
void aedit (CHAR_DATA * ch, char *argument) {
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;
    int value;

    EDIT_AREA (ch, pArea);
    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("AEdit:  Insufficient security to modify area.\n\r", ch);
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
    if ((value = flag_value (area_flags, command)) != NO_FLAG) {
        TOGGLE_BIT (pArea->area_flags, value);
        send_to_char ("Flag toggled.\n\r", ch);
        return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; aedit_table[cmd].name != NULL; cmd++) {
        if (!str_prefix (command, aedit_table[cmd].name)) {
            if ((*aedit_table[cmd].olc_fun) (ch, argument)) {
                SET_BIT (pArea->area_flags, AREA_CHANGED);
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
void redit (CHAR_DATA * ch, char *argument) {
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    EDIT_ROOM (ch, pRoom);
    pArea = pRoom->area;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("REdit:  Insufficient security to modify room.\n\r", ch);
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
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
    return;
}

/* Object Interpreter, called by do_oedit. */
void oedit (CHAR_DATA * ch, char *argument) {
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_OBJ (ch, pObj);
    pArea = pObj->area;

    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("OEdit: Insufficient security to modify area.\n\r", ch);
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
                SET_BIT (pArea->area_flags, AREA_CHANGED);
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
void medit (CHAR_DATA * ch, char *argument) {
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_MOB (ch, pMob);
    pArea = pMob->area;

    if (!IS_BUILDER (ch, pArea)) {
        send_to_char ("MEdit: Insufficient security to modify area.\n\r", ch);
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
                SET_BIT (pArea->area_flags, AREA_CHANGED);
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret (ch, arg);
}

void mpedit (CHAR_DATA * ch, char *argument) {
    MPROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_MPCODE (ch, pMcode);

    if (pMcode) {
        ad = area_get_by_inner_vnum (pMcode->vnum);
        if (ad == NULL) { /* ??? */
            edit_done (ch);
            return;
        }
        if (!IS_BUILDER (ch, ad)) {
            send_to_char ("MPEdit: Insufficient security to modify code.\n\r",
                          ch);
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
            if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
                if ((ad = area_get_by_inner_vnum (pMcode->vnum)) != NULL)
                    SET_BIT (ad->area_flags, AREA_CHANGED);
            return;
        }
    }

    interpret (ch, arg);
}

void hedit (CHAR_DATA * ch, char *argument) {
    HELP_DATA *pHelp;
    HELP_AREA *had;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde (argument);
    strcpy (arg, argument);
    argument = one_argument (argument, command);

    EDIT_HELP (ch, pHelp);

    had = help_area_get_by_help (pHelp);
    if (had == NULL) {
        bugf ("hedit : had for help %s NULL", pHelp->keyword);
        edit_done (ch);
        return;
    }
    if (ch->pcdata->security < 9) {
        send_to_char ("HEdit: Insufficient security to edit helps.\n\r", ch);
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

bool show_version (CHAR_DATA * ch, char *argument) {
    send_to_char (OLC_VERSION, ch);
    send_to_char ("\n\r", ch);
    send_to_char (OLC_AUTHOR, ch);
    send_to_char ("\n\r", ch);
    send_to_char (OLC_DATE, ch);
    send_to_char ("\n\r", ch);
    send_to_char (OLC_CREDITS, ch);
    send_to_char ("\n\r", ch);
    return FALSE;
}

void show_liqlist (CHAR_DATA * ch) {
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = buf_new ();

    for (liq = 0; liq_table[liq].name != NULL; liq++) {
        if ((liq % 21) == 0)
            add_buf (buffer,
                     "Name                 Color          Proof Full Thirst Food Ssize\n\r");

        sprintf (buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
                 liq_table[liq].name, liq_table[liq].color,
                 liq_table[liq].affect[0], liq_table[liq].affect[1],
                 liq_table[liq].affect[2], liq_table[liq].affect[3],
                 liq_table[liq].affect[4]);
        add_buf (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}

void show_damlist (CHAR_DATA * ch) {
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = buf_new ();

    for (att = 0; attack_table[att].name != NULL; att++) {
        if ((att % 21) == 0)
            add_buf (buffer, "Name                 Noun\n\r");
        sprintf (buf, "%-20s %-20s\n\r",
                 attack_table[att].name, attack_table[att].noun);
        add_buf (buffer, buf);
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
}

/*****************************************************************************
 Name:        show_flag_cmds
 Purpose:    Displays settable flags and stats.
 Called by:    show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds (CHAR_DATA * ch, const FLAG_TYPE *flag_table) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int flag;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++) {
        if (flag_table[flag].settable) {
            sprintf (buf, "%-19.18s", flag_table[flag].name);
            strcat (buf1, buf);
            if (++col % 4 == 0)
                strcat (buf1, "\n\r");
        }
    }

    if (col % 4 != 0)
        strcat (buf1, "\n\r");

    send_to_char (buf1, ch);
    return;
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
void show_skill_cmds (CHAR_DATA * ch, int tar) {
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
    return;
}

/*****************************************************************************
 Name:       show_spec_cmds
 Purpose:    Displays settable special functions.
 Called by:  show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds (CHAR_DATA * ch) {
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int spec;
    int col;

    buf1[0] = '\0';
    col = 0;
    send_to_char ("Preceed special functions with 'spec_'\n\r\n\r", ch);
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
bool show_help (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument (argument, arg);
    one_argument (argument, spell);

    /* Display syntax.  */
    if (arg[0] == '\0') {
        send_to_char ("Syntax:  ? [command]\n\r\n\r", ch);
        send_to_char ("[command]          [description]\n\r", ch);
        for (cnt = 0; master_table[cnt].table != NULL; cnt++) {
            sprintf (buf, "%-18.18s -%s\n\r",
                     master_table[cnt].name,
                     master_table[cnt].description);
            send_to_char (buf, ch);
        }
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
                show_skill_cmds (ch, TAR_IGNORE);
            else if (!str_prefix (spell, "attack"))
                show_skill_cmds (ch, TAR_CHAR_OFFENSIVE);
            else if (!str_prefix (spell, "defend"))
                show_skill_cmds (ch, TAR_CHAR_DEFENSIVE);
            else if (!str_prefix (spell, "self"))
                show_skill_cmds (ch, TAR_CHAR_SELF);
            else if (!str_prefix (spell, "object"))
                show_skill_cmds (ch, TAR_OBJ_INV);
            else {
                send_to_char (
                    "Syntax:  ? spell "
                    "[ignore/attack/defend/self/object/all]\n\r", ch);
            }
            return FALSE;
        }
        else if (IS_SET (master_table[cnt].flags, TABLE_FLAG_TYPE)) {
            show_flag_cmds (ch, master_table[cnt].table);
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

bool olc_str_replace_dup (CHAR_DATA *ch, char **old_str, char *new_str,
    char *syntax_msg, char *success_msg)
{
    RETURN_IF (new_str == NULL || new_str[0] == '\0',
        syntax_msg, ch, FALSE);
    str_replace_dup (old_str, new_str);
    send_to_char (success_msg, ch);
    return TRUE;
}

bool olc_int_replace (CHAR_DATA *ch, int *old_val, char *new_val,
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

bool olc_sh_int_replace (CHAR_DATA *ch, sh_int *old_val, char *new_val,
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

bool olc_long_int_replace (CHAR_DATA *ch, long int *old_val, char *new_val,
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
