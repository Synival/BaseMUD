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

#ifndef __ROM_UPDATE_H
#define __ROM_UPDATE_H

/* Function prototypes. */
int recovery_in_position (int gain, int position);
int hit_gain (CHAR_T *ch, bool apply_learning);
int mana_gain (CHAR_T *ch, bool apply_learning);
int move_gain (CHAR_T *ch, bool apply_learning);
void gain_condition (CHAR_T *ch, int cond, int value);
void area_update (void);
void mobile_update (void);
void weather_update (void);
void health_update (void);
void health_update_ch (CHAR_T *ch);
void health_update_ch_stat (CHAR_T *ch, sh_int *cur, sh_int *max,
    sh_int *rem, int (*func) (CHAR_T *, bool));
void char_update (void);
void obj_update (void);
void aggr_update (void);
void violence_update (void);
void pulse_update (void);
void update_handler (void);

#endif
