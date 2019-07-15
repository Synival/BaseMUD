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

#ifndef __ROM_COLOUR_H
#define __ROM_COLOUR_H

#include "merc.h"

/* Colour Bits */
#define CB_RED          (A) // 0x0001
#define CB_GREEN        (B) // 0x0002
#define CB_BLUE         (C) // 0x0004
#define CB_BRIGHT       (D) // 0x0008
#define CB_BACK_RED     (E) // 0x0010
#define CB_BACK_GREEN   (F) // 0x0020
#define CB_BACK_BLUE    (G) // 0x0040
#define CB_BACK_BRIGHT  (H) // 0x0080
#define CB_BEEP         (I) // 0x0100
#define CB_DEFAULT      (J) // 0x0200
#define CB_BACK_DEFAULT (K) // 0x0400

/* Colour bitMasks */
#define CM_FORECOLOUR   ((A) | (B) | (C) | (D) | (J))
#define CM_BACKCOLOUR   ((E) | (F) | (G) | (H) | (K))
#define CM_BEEP         (I)

/* Colour Codes */
#define CC_CLEAR               (CB_DEFAULT | CB_BACK_DEFAULT)
#define CC_DEFAULT             (CB_DEFAULT)
#define CC_BLACK               (0x00                       )
#define CC_RED                 (CB_RED                     )
#define CC_GREEN               (         CB_GREEN          )
#define CC_YELLOW              (CB_RED | CB_GREEN          )
#define CC_BLUE                (                    CB_BLUE)
#define CC_MAGENTA             (CB_RED            | CB_BLUE)
#define CC_CYAN                (         CB_GREEN | CB_BLUE)
#define CC_WHITE               (CB_RED | CB_GREEN | CB_BLUE)
#define CC_BRIGHT_DEFAULT      (CB_BRIGHT | CC_DEFAULT)
#define CC_DARK_GREY           (CB_BRIGHT | CC_BLACK)
#define CC_BRIGHT_RED          (CB_BRIGHT | CC_RED)
#define CC_BRIGHT_GREEN        (CB_BRIGHT | CC_GREEN)
#define CC_BRIGHT_YELLOW       (CB_BRIGHT | CC_YELLOW)
#define CC_BRIGHT_BLUE         (CB_BRIGHT | CC_BLUE)
#define CC_BRIGHT_MAGENTA      (CB_BRIGHT | CC_MAGENTA)
#define CC_BRIGHT_CYAN         (CB_BRIGHT | CC_CYAN)
#define CC_BRIGHT_WHITE        (CB_BRIGHT | CC_WHITE)
#define CC_BACK_DEFAULT        (CB_BACK_DEFAULT)
#define CC_BACK_BLACK          (CC_BLACK   << 4)
#define CC_BACK_RED            (CC_RED     << 4)
#define CC_BACK_GREEN          (CC_GREEN   << 4)
#define CC_BACK_YELLOW         (CC_YELLOW  << 4)
#define CC_BACK_BLUE           (CC_BLUE    << 4)
#define CC_BACK_MAGENTA        (CC_MAGENTA << 4)
#define CC_BACK_CYAN           (CC_CYAN    << 4)
#define CC_BACK_WHITE          (CC_WHITE   << 4)
#define CC_BACK_BRIGHT_DEFAULT (CC_BACK_BRIGHT | CB_BACK_DEFAULT)
#define CC_BACK_DARK_GREY      (CC_DARK_GREY      << 4)
#define CC_BACK_BRIGHT_RED     (CC_BRIGHT_RED     << 4)
#define CC_BACK_BRIGHT_GREEN   (CC_BRIGHT_GREEN   << 4)
#define CC_BACK_BRIGHT_YELLOW  (CC_BRIGHT_YELLOW  << 4)
#define CC_BACK_BRIGHT_BLUE    (CC_BRIGHT_BLUE    << 4)
#define CC_BACK_BRIGHT_MAGENTA (CC_BRIGHT_MAGENTA << 4)
#define CC_BACK_BRIGHT_CYAN    (CC_BRIGHT_CYAN    << 4)
#define CC_BACK_BRIGHT_WHITE   (CC_BRIGHT_WHITE   << 4)

/* functions */
int colour_to_full_name args ((flag_t colour, char *buf_out, size_t size));
int colour_code_to_ansi args ((CHAR_DATA * ch, int use_colour,
                               char type, char *buf_out, size_t size));
int colour_puts args ((CHAR_DATA *ch, bool use_colour, const char *buf_in,
                       char *buf_out, size_t size));
int colour_to_ansi args ((flag_t colour, char *buf_out, size_t size));
const COLOUR_SETTING_TYPE *colour_setting_get_by_char (char ch);

#endif
