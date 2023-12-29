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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

#ifndef __ROM_DESCS_H
#define __ROM_DESCS_H

#include "merc.h"

/* OS-dependent local functions. */
#if defined(NOSERVER)
    void init_descriptor args ((void));
#else
    int init_socket args ((int port));
    void init_descriptor args ((int control));
#endif

bool read_from_descriptor args ((DESCRIPTOR_T *d));
bool write_to_descriptor args ((int desc, char *txt, int length));
void close_socket (DESCRIPTOR_T *dclose);
void read_from_buffer (DESCRIPTOR_T *d);
bool process_output (DESCRIPTOR_T *d, bool prompt);
bool desc_flush_output (DESCRIPTOR_T *d);
void write_to_buffer (DESCRIPTOR_T *d, const char *txt, int length);
bool check_reconnect (DESCRIPTOR_T *d, char *name, bool conn);
bool check_playing (DESCRIPTOR_T *d, char *name);
void send_to_desc (const char *txt, DESCRIPTOR_T *d);
void clear_page (DESCRIPTOR_T *d);
void append_to_page (DESCRIPTOR_T *d, const char *txt);
int show_page (DESCRIPTOR_T *d);
void printf_to_desc (DESCRIPTOR_T *d, char *fmt, ...);
void desc_substitute_alias (DESCRIPTOR_T *d, char *argument);

#endif
