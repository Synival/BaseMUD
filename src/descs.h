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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#ifndef __ROM_DESCS_H
#define __ROM_DESCS_H

#include "merc.h"

/* OS-dependent local functions. */
#if defined(macintosh) || defined(MSDOS)
    bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
    bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

#if defined(unix)
    int init_socket args ((int port));
    void init_descriptor args ((int control));
    bool read_from_descriptor args ((DESCRIPTOR_DATA * d));
    bool write_to_descriptor args ((int desc, char *txt, int length));
#endif

void close_socket (DESCRIPTOR_DATA * dclose);
void read_from_buffer (DESCRIPTOR_DATA * d);
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt);
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length);
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn);
bool check_playing (DESCRIPTOR_DATA * d, char *name);
void send_to_desc (const char *txt, DESCRIPTOR_DATA * d);
void clear_page (DESCRIPTOR_DATA *d);
void append_to_page (DESCRIPTOR_DATA *d, const char *txt);
int show_page (DESCRIPTOR_DATA *d);
void printf_to_desc (DESCRIPTOR_DATA * d, char *fmt, ...);
void desc_substitute_alias (DESCRIPTOR_DATA * d, char *argument);

#endif
