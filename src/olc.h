/***************************************************************************
 *  File: olc.h                                                            *
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

#ifndef __ROM_OLC_H
#define __ROM_OLC_H

#include "merc.h"

/* NOTE
 * ----
 * Much of the OLC functions have been moved around to different files to
 * facilitate their clean-up. This header now only represents variables
 * and functions in this file only, with the exception of OLC-wide structures
 * and definitions.
 *     -- Synival */

/* Original comments below:
 * ------------------------ */

/* This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel */

/* The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel */

#define OLC_VERSION "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
                    "     Port a ROM 2.4 v1.8\n\r"
#define OLC_AUTHOR  "     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                    "     Modified for use with ROM 2.3\n\r"        \
                    "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                    "     Modificado para uso en ROM 2.4b6\n\r"    \
                    "     Por Ivan Toledo (itoledo@ctcreuna.cl)\n\r"
#define OLC_DATE    "     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
                    "     (Port a ROM 2.4 - Nov 2, 1996)\n\r" \
                    "     Version actual : 1.8 - Sep 8, 1998\n\r"
#define OLC_CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

/* Connected states for editor. */
#define ED_NONE     0
#define ED_AREA     1
#define ED_ROOM     2
#define ED_OBJECT   3
#define ED_MOBILE   4
#define ED_MPCODE   5
#define ED_HELP     6

/* OLC Constants */
#define MAX_MOB     1 /* Default maximum number for resetting mobs */

/* New typedefs. */
typedef bool OLC_FUN (CHAR_DATA *ch, char *argument);
#define DECLARE_OLC_FUN(fun) OLC_FUN fun

/* Structure for an OLC editor command. */
struct olc_cmd_type {
    char *const name;
    OLC_FUN *olc_fun;
};

/* Structure for an OLC editor startup command. */
struct editor_cmd_type {
    char *const name;
    DO_FUN *do_fun;
};

/* Global variables. */
extern int top_vnum_room;
extern int top_vnum_mob;
extern int top_vnum_obj;

/* Interpreter Table Prototypes */
extern const struct editor_cmd_type editor_table[];
extern const struct olc_cmd_type aedit_table[];
extern const struct olc_cmd_type redit_table[];
extern const struct olc_cmd_type oedit_table[];
extern const struct olc_cmd_type medit_table[];
extern const struct olc_cmd_type mpedit_table[];
extern const struct olc_cmd_type hedit_table[];

/* General Functions */
bool run_olc_editor (DESCRIPTOR_DATA * d);
char *olc_ed_name (CHAR_DATA * ch);
char *olc_ed_vnum (CHAR_DATA * ch);
void show_olc_cmds (CHAR_DATA * ch, const struct olc_cmd_type *olc_table);
bool show_commands (CHAR_DATA * ch, char *argument);
bool edit_done (CHAR_DATA * ch);
bool show_version (CHAR_DATA * ch, char *argument);
void show_liqlist (CHAR_DATA * ch);
void show_damlist (CHAR_DATA * ch);
void show_flag_cmds (CHAR_DATA * ch, const FLAG_TYPE *flag_table);
void show_skill_cmds (CHAR_DATA * ch, int tar);
void show_spec_cmds (CHAR_DATA * ch);
bool show_help (CHAR_DATA * ch, char *argument);

/* Modification helper functions. */
bool olc_str_replace_dup (CHAR_DATA *ch, char **old_str, char *new_str,
    char *syntax_msg, char *success_msg);
bool olc_int_replace (CHAR_DATA *ch, int *old_val, char *new_val,
    char *syntax_msg, char *success_msg);
bool olc_sh_int_replace (CHAR_DATA *ch, sh_int *old_val, char *new_val,
    char *syntax_msg, char *success_msg);
bool olc_long_int_replace (CHAR_DATA *ch, long int *old_val, char *new_val,
    char *syntax_msg, char *success_msg);

/* Interpreter Prototypes */
void aedit  (CHAR_DATA *ch, char *argument);
void redit  (CHAR_DATA *ch, char *argument);
void medit  (CHAR_DATA *ch, char *argument);
void oedit  (CHAR_DATA *ch, char *argument);
void mpedit (CHAR_DATA *ch, char *argument);
void hedit  (CHAR_DATA *ch, char *argument);

#endif
