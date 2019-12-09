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

/***************************************************************************
 *                                                                         *
 *  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
 *  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *  This code may be copied and distributed as per the ROM license.        *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROG_MOB_PROG_H
#define __ROG_MOB_PROG_H

#include "merc.h"

/* These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here. */
#define CHK_RAND        (0)
#define CHK_MOBHERE     (1)
#define CHK_OBJHERE     (2)
#define CHK_MOBEXISTS   (3)
#define CHK_OBJEXISTS   (4)
#define CHK_PEOPLE      (5)
#define CHK_PLAYERS     (6)
#define CHK_MOBS        (7)
#define CHK_CLONES      (8)
#define CHK_ORDER       (9)
#define CHK_HOUR        (10)
#define CHK_ISPC        (11)
#define CHK_ISNPC       (12)
#define CHK_ISGOOD      (13)
#define CHK_ISEVIL      (14)
#define CHK_ISNEUTRAL   (15)
#define CHK_ISIMMORT    (16)
#define CHK_ISCHARM     (17)
#define CHK_ISFOLLOW    (18)
#define CHK_ISACTIVE    (19)
#define CHK_ISDELAY     (20)
#define CHK_ISVISIBLE   (21)
#define CHK_HASTARGET   (22)
#define CHK_ISTARGET    (23)
#define CHK_EXISTS      (24)
#define CHK_AFFECTED    (25)
#define CHK_ACT         (26)
#define CHK_OFF         (27)
#define CHK_IMM         (28)
#define CHK_CARRIES     (29)
#define CHK_WEARS       (30)
#define CHK_HAS         (31)
#define CHK_USES        (32)
#define CHK_NAME        (33)
#define CHK_POS         (34)
#define CHK_CLAN        (35)
#define CHK_RACE        (36)
#define CHK_CLASS       (37)
#define CHK_OBJTYPE     (38)
#define CHK_VNUM        (39)
#define CHK_HPCNT       (40)
#define CHK_ROOM        (41)
#define CHK_SEX         (42)
#define CHK_LEVEL       (43)
#define CHK_ALIGN       (44)
#define CHK_MONEY       (45)
#define CHK_OBJVAL0     (46)
#define CHK_OBJVAL1     (47)
#define CHK_OBJVAL2     (48)
#define CHK_OBJVAL3     (49)
#define CHK_OBJVAL4     (50)
#define CHK_GRPSIZE     (51)

/* These defines correspond to the entries in fn_evals[] table. */
#define EVAL_EQ  0
#define EVAL_GE  1
#define EVAL_LE  2
#define EVAL_GT  3
#define EVAL_LT  4
#define EVAL_NE  5

/* Function prototypes. */
int keyword_lookup (const char **table, char *keyword);
int num_eval (int lval, int oper, int rval);
CHAR_T *get_random_char (CHAR_T *mob);
bool count_people_room_check (CHAR_T *mob, CHAR_T *vch, int flag);
int count_people_room (CHAR_T *mob, int flag);
int get_order (CHAR_T *ch);
bool has_item (CHAR_T *ch, sh_int vnum, sh_int item_type, bool wear);
bool get_mob_vnum_room (CHAR_T *ch, sh_int vnum);
bool get_obj_vnum_room (CHAR_T *ch, sh_int vnum);
int cmd_eval (sh_int vnum, char *line, int check, CHAR_T *mob, CHAR_T *ch,
    const void *arg1, const void *arg2, CHAR_T *rch);
void expand_arg (char *buf, const char *format, CHAR_T *mob, CHAR_T *ch,
    const void *arg1, const void *arg2, CHAR_T *rch);
void program_flow (
    sh_int pvnum,    /* For diagnostic purposes */
    char *source,    /* the actual MOBprog code */
    CHAR_T *mob, CHAR_T *ch, const void *arg1, const void *arg2);
bool mp_act_trigger (char *argument, CHAR_T *mob, CHAR_T *ch,
    const void *arg1, const void *arg2, int type);
bool mp_percent_trigger (CHAR_T *mob, CHAR_T *ch, const void *arg1,
    const void *arg2, int type);
bool mp_bribe_trigger (CHAR_T *mob, CHAR_T *ch, int amount);
bool mp_exit_trigger (CHAR_T *ch, int dir);
bool mp_give_trigger (CHAR_T *mob, CHAR_T *ch, OBJ_T *obj);
bool mp_greet_trigger (CHAR_T *ch);
bool mp_hprct_trigger (CHAR_T *mob, CHAR_T *ch);

#endif
