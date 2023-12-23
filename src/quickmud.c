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

#include "quickmud.h"

#include "fread.h"
#include "utils.h"

void qmconfig_read (void) {
    FILE *fp;
    bool match, reading;
    char *word;
    extern int mud_ansiprompt, mud_ansicolor, mud_telnetga;

    log_f ("Loading configuration settings from %s.", QMCONFIG_FILE);
    fp = fopen (QMCONFIG_FILE, "r");
    if (!fp) {
        log_f ("%s not found. Using compiled-in defaults.", QMCONFIG_FILE);
        return;
    }

    reading = TRUE;
    while (reading) {
        word = feof (fp) ? "END" : fread_word_static (fp);
        match = FALSE;

        switch (UPPER(word[0])) {
            case '#':
                /* This is a comment line! */
                match = TRUE;
                fread_to_eol (fp);
                break;

            case '*':
                match = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                KEY ("Ansicolor",  mud_ansicolor,  fread_number(fp));
                KEY ("Ansiprompt", mud_ansiprompt, fread_number(fp));
                break;

            case 'E':
                if (!str_cmp (word, "END")) {
                    reading = FALSE;
                    match = TRUE;
                }
                break;

            case 'T':
                KEY ("Telnetga", mud_telnetga, fread_number(fp));
                break;
        }
        if (!match) {
            log_f ("qmconfig_read: no match for %s!", word);
            fread_to_eol(fp);
        }
    }

 // log_f ("Settings have been read from %s", QMCONFIG_FILE);
    fclose (fp);
}
