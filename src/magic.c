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

#include <stdlib.h>
#include <string.h>

#include "lookup.h"
#include "affects.h"
#include "comm.h"
#include "skills.h"
#include "fight.h"
#include "utils.h"
#include "chars.h"

#include "magic.h"

/* The kludgy global is for spells who want more stuff from command line. */
char *target_name;

int find_spell (CHAR_DATA *ch, const char *name) {
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC (ch))
        return skill_lookup (name);

    for (sn = 0; sn < SKILL_MAX; sn++) {
        if (skill_table[sn].name == NULL)
            break;
        if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
            && !str_prefix (name, skill_table[sn].name))
        {
            if (found == -1)
                found = sn;
            if (ch->level >= skill_table[sn].skill_level[ch->class]
                && ch->pcdata->learned[sn] > 0)
                return sn;
        }
    }
    return found;
}

/* Utter mystical words for an sn. */
void say_spell (CHAR_DATA *ch, int sn, int class) {
    say_spell_name (ch, skill_table[sn].name, class);
}

void say_spell_name (CHAR_DATA *ch, const char *name, int class) {
    char words[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    const char *pName, *plural;
    int iSyl;
    int length;

    struct syl_type {
        char *old;
        char *new;
    };

    static const struct syl_type syl_table[] = {
        {" ",     " "},
        {"ar",    "abra"},
        {"au",    "kada"},
        {"bless", "fido"},
        {"blind", "nose"},
        {"bur",   "mosa"},
        {"cu",    "judi"},
        {"de",    "oculo"},
        {"en",    "unso"},
        {"light", "dies"},
        {"lo",    "hi"},
        {"mor",   "zak"},
        {"move",  "sido"},
        {"ness",  "lacri"},
        {"ning",  "illa"},
        {"per",   "duda"},
        {"ra",    "gru"},
        {"fresh", "ima"},
        {"re",    "candus"},
        {"son",   "sabru"},
        {"tect",  "infra"},
        {"tri",   "cula"},
        {"ven",   "nofo"},
        {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
        {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
        {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
        {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
        {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
        {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
        {"y", "l"}, {"z", "k"},
        {"", ""}
    };

    words[0] = '\0';
    for (pName = name; *pName != '\0'; pName += length) {
        for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++) {
            if (!str_prefix (syl_table[iSyl].old, pName)) {
                strcat (words, syl_table[iSyl].new);
                break;
            }
        }

        if (length == 0)
            length = 1;
    }

    plural = (strchr (name, ' ')) ? "s" : "";
    printf_to_char (ch, "{5You utter the word%s, '%s'.{x\n\r", plural, name);
    sprintf (buf1, "{5$n utters the word%s, '%s'.{x", plural, name);

    plural = (strchr (words, ' ')) ? "s" : "";
    sprintf (buf2, "{5$n utters the word%s, '%s'.{x", plural, words);

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != ch)
            act ((!IS_NPC (rch) && rch->class == class) ? buf1 : buf2,
                 ch, NULL, rch, TO_VICT);
}

/* Compute a saving throw.
 * Negative apply's make saving throw better. */
bool saves_spell (int level, CHAR_DATA *victim, int dam_type) {
    int save;

    save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED (victim, AFF_BERSERK))
        save += victim->level / 2;

    switch (check_immune (victim, dam_type)) {
        case IS_IMMUNE:     return TRUE;
        case IS_RESISTANT:  save += 2; break;
        case IS_VULNERABLE: save -= 2; break;
    }

    if (!IS_NPC (victim) && class_table[victim->class].fMana)
        save = 9 * save / 10;
    save = URANGE (5, save, 95);

    return number_percent () < save;
}

/* RT save for dispels */
bool saves_dispel (int dis_level, int spell_level, int duration) {
    int save;

    if (duration == -1)
        spell_level += 5;

    /* very hard to dispel permanent effects */
    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE (5, save, 95);
    return number_percent () < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel_act (int dis_level, CHAR_DATA *victim, int sn,
    char *act_to_room)
{
    AFFECT_DATA *af;

    if (!is_affected (victim, sn))
        return FALSE;
    for (af = victim->affected; af != NULL; af = af->next) {
        if (af->type != sn)
            continue;
        if (!saves_dispel (dis_level, af->level, af->duration)) {
            affect_strip (victim, sn);
            if (skill_table[sn].msg_off)
                printf_to_char (victim, "%s\n\r", skill_table[sn].msg_off);
            if (act_to_room)
                act (act_to_room, victim, NULL, NULL, TO_NOTCHAR);
            return TRUE;
        }
        else
            af->level--;
    }
    return FALSE;
}

bool check_dispel (int dis_level, CHAR_DATA *victim, int sn) {
    return check_dispel_act (dis_level, victim, sn, NULL);
}

bool check_dispel_quick (int dis_level, CHAR_DATA *victim, char *skill,
    char *act_to_room)
{
    return check_dispel_act (dis_level, victim, skill_lookup(skill),
        act_to_room);
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level) {
    if (ch->level + 2 == level)
        return 1000;
    return UMAX (min_mana, (100 / (2 + ch->level - level)));
}

bool spell_fight_back_if_possible (CHAR_DATA *ch, CHAR_DATA *victim,
    int sn, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (victim == ch || victim == NULL || victim->master == ch)
        return FALSE;
    if (!( skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
          (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR)))
        return FALSE;

    for (vch = ch->in_room->people; vch; vch = vch_next) {
        vch_next = vch->next_in_room;
        if (victim == vch && victim->fighting == NULL) {
            check_killer (victim, ch);
            multi_hit (victim, ch, TYPE_UNDEFINED);
            return TRUE;
        }
    }
    return FALSE;
}

/* Cast spells at targets using a magical object. */
void obj_cast_spell (int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim,
                     OBJ_DATA *obj)
{
    void *vo;
    int target = TARGET_NONE;

    if (sn <= 0)
        return;
    BAIL_IF_BUG (sn >= SKILL_MAX || skill_table[sn].spell_fun == 0,
        "obj_cast_spell: bad sn %d.", sn);

    switch (skill_table[sn].target) {
        case TAR_IGNORE:
            vo = NULL;
            break;

        case TAR_CHAR_OFFENSIVE:
            if (victim == NULL)
                victim = ch->fighting;
            BAIL_IF (victim == NULL,
                "You can't do that.\n\r", ch);
            if (ch != victim && do_filter_can_attack (ch, victim))
                return;
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_CHAR_DEFENSIVE:
        case TAR_CHAR_SELF:
            if (victim == NULL)
                victim = ch;
            vo = (void *) victim;
            target = TARGET_CHAR;
            break;

        case TAR_OBJ_INV:
            BAIL_IF (obj == NULL,
                "You can't do that.\n\r", ch);
            vo = (void *) obj;
            target = TARGET_OBJ;
            break;

        case TAR_OBJ_CHAR_OFF:
            if (victim == NULL && obj == NULL) {
                BAIL_IF (ch->fighting == NULL,
                    "You can't do that.\n\r", ch);
                victim = ch->fighting;
            }
            if (victim != NULL) {
                if (ch != victim && do_filter_can_attack_spell (
                        ch, victim, FALSE))
                    return;
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            break;

        case TAR_OBJ_CHAR_DEF:
            if (victim == NULL && obj == NULL) {
                vo = (void *) ch;
                target = TARGET_CHAR;
            }
            else if (victim != NULL) {
                vo = (void *) victim;
                target = TARGET_CHAR;
            }
            else {
                vo = (void *) obj;
                target = TARGET_OBJ;
            }
            break;

        default:
            bug ("obj_cast_spell: bad target for sn %d.", sn);
            return;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);
    spell_fight_back_if_possible (ch, victim, sn, target);
}

int is_affected_with_act (CHAR_DATA *victim, int sn, flag_t flag,
    CHAR_DATA * ch, char *to_self, char *to_victim)
{
    if ((sn   >= 0 && is_affected (victim, sn)) ||
        (flag >  0 && IS_AFFECTED (victim, flag)))
    {
        act ((victim == ch) ? to_self : to_victim, ch, NULL, victim, TO_CHAR);
        return 1;
    }
    return 0;
}

int isnt_affected_with_act (CHAR_DATA * victim, int sn, flag_t flag,
    CHAR_DATA * ch, char *to_self, char *to_victim)
{
    if (!((sn   >= 0 && is_affected (victim, sn)) ||
          (flag >  0 && IS_AFFECTED (victim, flag))))
    {
        act ((victim == ch) ? to_self : to_victim, ch, NULL, victim, TO_CHAR);
        return 1;
    }
    return 0;
}

DEFINE_SPELL_FUN (spell_null) {
    send_to_char ("That's not a spell!\n\r", ch);
}
