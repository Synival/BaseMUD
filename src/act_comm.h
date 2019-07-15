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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#ifndef __ROM_ACT_COMM_H
#define __ROM_ACT_COMM_H

#include "merc.h"

/* Sub-routines and filters. */
bool do_comm_filter_quiet (CHAR_DATA *ch);
bool do_comm_filter_nochannels (CHAR_DATA *ch);
bool do_comm_filter_quiet_nochannels (CHAR_DATA *ch);
bool do_comm_filter_emote (CHAR_DATA *ch, char *argument);
void do_comm_channel_global (CHAR_DATA *ch, char *argument, flag_t channel,
    char *message_on, char *message_off, char *act_self, char *act_world,
    int max_pos);
bool do_comm_filter_can_tell_or_reply (CHAR_DATA *ch);
void do_comm_tell_to_buffer (CHAR_DATA *ch, CHAR_DATA *victim, char *msg);
bool do_comm_filter_tell_target_can_receive_tells(CHAR_DATA *ch,
    CHAR_DATA *victim);
bool do_comm_filter_tell_not_now (CHAR_DATA *ch, CHAR_DATA *victim, char *msg);
void do_comm_tell_send_message (CHAR_DATA *ch, CHAR_DATA *victim, char *msg);
void do_comm_try_tell (CHAR_DATA *ch, CHAR_DATA *victim, char *msg);

/* Commands. */
DECLARE_DO_FUN (do_socials);
DECLARE_DO_FUN (do_channels);
DECLARE_DO_FUN (do_deaf);
DECLARE_DO_FUN (do_quiet);
DECLARE_DO_FUN (do_afk);
DECLARE_DO_FUN (do_replay);
DECLARE_DO_FUN (do_auction);
DECLARE_DO_FUN (do_gossip);
DECLARE_DO_FUN (do_grats);
DECLARE_DO_FUN (do_quote);
DECLARE_DO_FUN (do_question);
DECLARE_DO_FUN (do_answer);
DECLARE_DO_FUN (do_music);
DECLARE_DO_FUN (do_clantalk);
DECLARE_DO_FUN (do_say);
DECLARE_DO_FUN (do_shout);
DECLARE_DO_FUN (do_tell);
DECLARE_DO_FUN (do_reply);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_emote);
DECLARE_DO_FUN (do_pmote);
DECLARE_DO_FUN (do_pose);
DECLARE_DO_FUN (do_bug);
DECLARE_DO_FUN (do_typo);

#endif
