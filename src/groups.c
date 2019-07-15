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

#include "utils.h"
#include "affects.h"
#include "comm.h"
#include "skills.h"
#include "db.h"
#include "chars.h"

#include "groups.h"

/* TODO: is_friend() and should_assist_group() probably have some overlap. */
/* TODO: any other functions from other files that should be dumped here? */

/* It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C */
bool is_same_group (CHAR_DATA * ach, CHAR_DATA * bch) {
    if (ach == NULL || bch == NULL)
        return FALSE;

    if (ach->leader != NULL)
        ach = ach->leader;
    if (bch->leader != NULL)
        bch = bch->leader;
    return ach == bch;
}

void add_follower (CHAR_DATA * ch, CHAR_DATA * master) {
    if (ch->master != NULL) {
        bug ("add_follower: non-null master.", 0);
        return;
    }

    ch->master = master;
    ch->leader = NULL;

    if (char_can_see_in_room (master, ch))
        act ("$n now follows you.", ch, NULL, master, TO_VICT);
    act ("You now follow $N.", ch, NULL, master, TO_CHAR);
}

void stop_follower (CHAR_DATA * ch) {
    if (ch->master == NULL) {
        bug ("stop_follower: null master.", 0);
        return;
    }

    if (IS_AFFECTED (ch, AFF_CHARM)) {
        REMOVE_BIT (ch->affected_by, AFF_CHARM);
        affect_strip (ch, gsn_charm_person);
    }

    if (char_can_see_in_room (ch->master, ch) && ch->in_room != NULL)
        act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
    act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    if (ch->master->pet == ch)
        ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
}

/* nukes charmed monsters and pets */
void nuke_pets (CHAR_DATA * ch) {
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL) {
        stop_follower (pet);
        if (pet->in_room != NULL)
            act ("$N slowly fades away.", ch, NULL, pet, TO_OTHERS);
        char_extract (pet, TRUE);
    }
    ch->pet = NULL;
}

void die_follower (CHAR_DATA * ch) {
    CHAR_DATA *fch;

    if (ch->master != NULL) {
        if (ch->master->pet == ch)
            ch->master->pet = NULL;
        stop_follower (ch);
    }

    ch->leader = NULL;
    for (fch = char_list; fch != NULL; fch = fch->next) {
        if (fch->master == ch)
            stop_follower (fch);
        if (fch->leader == ch)
            fch->leader = fch;
    }
}
