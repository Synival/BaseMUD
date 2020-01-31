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

#include <stdarg.h>
#include <string.h>

#include "merc.h"

#include "interp.h"
#include "lookup.h"
#include "utils.h"

#include "ext_flags.h"

/* various flag tables */
const EXT_FLAG_DEF_T mob_flags[] = {
    {"npc",           MOB_IS_NPC,        FALSE},
    {"sentinel",      MOB_SENTINEL,      TRUE},
    {"scavenger",     MOB_SCAVENGER,     TRUE},
    {"unused_act_1",  MOB_UNUSED_FLAG_1, FALSE},
    {"unused_act_2",  MOB_UNUSED_FLAG_2, FALSE},
    {"aggressive",    MOB_AGGRESSIVE,    TRUE},
    {"stay_area",     MOB_STAY_AREA,     TRUE},
    {"wimpy",         MOB_WIMPY,         TRUE},
    {"pet",           MOB_PET,           TRUE},
    {"train",         MOB_TRAIN,         TRUE},
    {"practice",      MOB_PRACTICE,      TRUE},
    {"unused_act_3",  MOB_UNUSED_FLAG_3, FALSE},
    {"unused_act_4",  MOB_UNUSED_FLAG_4, FALSE},
    {"unused_act_5",  MOB_UNUSED_FLAG_5, FALSE},
    {"undead",        MOB_UNDEAD,        TRUE},
    {"unused_act_6",  MOB_UNUSED_FLAG_6, FALSE},
    {"cleric",        MOB_CLERIC,        TRUE},
    {"mage",          MOB_MAGE,          TRUE},
    {"thief",         MOB_THIEF,         TRUE},
    {"warrior",       MOB_WARRIOR,       TRUE},
    {"noalign",       MOB_NOALIGN,       TRUE},
    {"nopurge",       MOB_NOPURGE,       TRUE},
    {"outdoors",      MOB_OUTDOORS,      TRUE},
    {"unused_act_7",  MOB_UNUSED_FLAG_7, FALSE},
    {"indoors",       MOB_INDOORS,       TRUE},
    {"unused_act_8",  MOB_UNUSED_FLAG_8, FALSE},
    {"healer",        MOB_IS_HEALER,     TRUE},
    {"gain",          MOB_GAIN,          TRUE},
    {"update_always", MOB_UPDATE_ALWAYS, TRUE},
    {"changer",       MOB_IS_CHANGER,    TRUE},
    {0}
};

const EXT_FLAG_DEF_T plr_flags[] = {
    {"!npc!",         PLR_IS_NPC,        FALSE},
    {"unused_plr_1",  PLR_UNUSED_FLAG_1, FALSE},
    {"autoassist",    PLR_AUTOASSIST,    FALSE},
    {"autoexit",      PLR_AUTOEXIT,      FALSE},
    {"autoloot",      PLR_AUTOLOOT,      FALSE},
    {"autosac",       PLR_AUTOSAC,       FALSE},
    {"autogold",      PLR_AUTOGOLD,      FALSE},
    {"autosplit",     PLR_AUTOSPLIT,     FALSE},
    {"unused_plr_2",  PLR_UNUSED_FLAG_2, FALSE},
    {"unused_plr_3",  PLR_UNUSED_FLAG_3, FALSE},
    {"unused_plr_4",  PLR_UNUSED_FLAG_4, FALSE},
    {"unused_plr_5",  PLR_UNUSED_FLAG_5, FALSE},
    {"unused_plr_6",  PLR_UNUSED_FLAG_6, FALSE},
    {"holylight",     PLR_HOLYLIGHT,     FALSE},
    {"unused_plr_7",  PLR_UNUSED_FLAG_7, FALSE},
    {"can_loot",      PLR_CANLOOT,       FALSE},
    {"nosummon",      PLR_NOSUMMON,      FALSE},
    {"nofollow",      PLR_NOFOLLOW,      FALSE},
    {"unused_plr_8",  PLR_UNUSED_FLAG_8, FALSE},
    {"colour",        PLR_COLOUR,        FALSE},
    {"permit",        PLR_PERMIT,        TRUE},
    {"unused_plr_9",  PLR_UNUSED_FLAG_9, FALSE},
    {"log",           PLR_LOG,           FALSE},
    {"deny",          PLR_DENY,          FALSE},
    {"freeze",        PLR_FREEZE,        FALSE},
    {"thief",         PLR_THIEF,         FALSE},
    {"killer",        PLR_KILLER,        FALSE},
    {0}
};

#define EF_INDEX(_flag) ((flag) / 8)
#define EF_BIT(_flag)   (1 << ((flag) % 8))

EXT_FLAGS_T ext_flags_build (int first, ...) {
    EXT_FLAGS_T flags = EXT_ZERO;
    va_list ap;
    int flag;

    va_start (ap, first);
    flag = first;
    while (flag >= 0) {
        EXT_SET (flags, flag);
        flag = va_arg(ap, int);
    }
    va_end (ap);

    return flags;
}

EXT_FLAGS_T ext_flags_from_init (const EXT_INIT_FLAGS_T *flags) {
    EXT_FLAGS_T new_flags = EXT_ZERO;
    int i;
    for (i = 0; flags->bits[i] >= 0; i++)
        EXT_SET (new_flags, flags->bits[i]);
    return new_flags;
}

bool ext_flags_is_set (EXT_FLAGS_T var, int flag)
    { return (var.bits[EF_INDEX(flag)] & EF_BIT(flag)) ? TRUE : FALSE; }

void ext_flags_set (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] |= EF_BIT(flag); }
void ext_flags_set_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_set) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        var->bits[i] |= to_set.bits[i];
}

void ext_flags_unset (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] &= ~EF_BIT(flag); }
void ext_flags_unset_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_remove) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        var->bits[i] &= ~to_remove.bits[i];
}

void ext_flags_toggle (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] ^= EF_BIT(flag); }
void ext_flags_toggle_many (EXT_FLAGS_T *var, EXT_FLAGS_T to_set) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        var->bits[i] ^= to_set.bits[i];
}

EXT_FLAGS_T ext_flags_from_flag_t (flag_t flags) {
    EXT_FLAGS_T bits = EXT_ZERO;
    bits.bits[0] = (flags >>  0) & 0xff;
    bits.bits[1] = (flags >>  8) & 0xff;
    bits.bits[2] = (flags >> 16) & 0xff;
    bits.bits[3] = (flags >> 24) & 0x7f; /* no bit 32 */
    return bits;
}

flag_t ext_flags_to_flag_t (EXT_FLAGS_T bits) {
    return
        (bits.bits[0]         <<  0) |
        (bits.bits[1]         <<  8) |
        (bits.bits[2]         << 16) |
       ((bits.bits[3] & 0x7f) << 24); /* no bit 32 */
}

bool ext_flags_is_zero (EXT_FLAGS_T bits) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        if (bits.bits[i] != 0)
            return FALSE;
    return TRUE;
}

bool ext_flags_equals (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        if (bits1.bits[i] != bits2.bits[i])
            return FALSE;
    return TRUE;
}

EXT_FLAGS_T ext_flags_with (EXT_FLAGS_T bits, int flag) {
    bits.bits[EF_INDEX(flag)] |= EF_BIT(flag);
    return bits;
}

EXT_FLAGS_T ext_flags_without (EXT_FLAGS_T bits, int flag) {
    bits.bits[EF_INDEX(flag)] &= ~EF_BIT(flag);
    return bits;
}

EXT_FLAGS_T ext_flags_with_many (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        bits1.bits[i] |= bits2.bits[i];
    return bits1;
}

EXT_FLAGS_T ext_flags_without_many (EXT_FLAGS_T bits1, EXT_FLAGS_T bits2) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        bits1.bits[i] &= ~bits2.bits[i];
    return bits1;
}

EXT_FLAGS_T ext_flags_inverted (EXT_FLAGS_T bits) {
    int i;
    for (i = 0; i < EXT_FLAGS_ARRAY_LENGTH; i++)
        bits.bits[i] = ~(bits.bits[i]);
    return bits;
}

int ext_flag_lookup (const EXT_FLAG_DEF_T *flag_table, const char *name)
    { SIMPLE_LOOKUP_PROP (flag_table, bit, name, EXT_FLAG_NONE, 0); }
int ext_flag_lookup_exact (const EXT_FLAG_DEF_T *flag_table, const char *name)
    { SIMPLE_LOOKUP_PROP_EXACT (flag_table, bit, name, EXT_FLAG_NONE, 0); }
const EXT_FLAG_DEF_T *ext_flag_get_by_name (const EXT_FLAG_DEF_T *flag_table, const char *name)
    { SIMPLE_GET_BY_NAME (flag_table, name, 0); }
const EXT_FLAG_DEF_T *ext_flag_get_by_name_exact (const EXT_FLAG_DEF_T *flag_table, char const *name)
    { SIMPLE_GET_BY_NAME_EXACT (flag_table, name, 0); }
const EXT_FLAG_DEF_T *ext_flag_get (const EXT_FLAG_DEF_T *flag_table, flag_t bit)
    { SIMPLE_GET (flag_table, bit, name, NULL, 0); }
const char *ext_flag_get_name (const EXT_FLAG_DEF_T *flag_table, flag_t bit)
    { SIMPLE_GET_NAME_FROM_ELEMENT (EXT_FLAG_DEF_T, ext_flag_get (flag_table, bit), name); }

EXT_FLAGS_T ext_flags_from_string (const EXT_FLAG_DEF_T *flag_table,
    const char *name)
{
    return ext_flags_from_string_real (flag_table, name, FALSE);
}

EXT_FLAGS_T ext_flags_from_string_exact (const EXT_FLAG_DEF_T *flag_table,
    const char *name)
{
    return ext_flags_from_string_real (flag_table, name, TRUE);
}

EXT_FLAGS_T ext_flags_from_string_real (const EXT_FLAG_DEF_T *flag_table,
    const char *name, bool exact)
{
    const EXT_FLAG_DEF_T *flag;
    char word[MAX_INPUT_LENGTH];
    EXT_FLAGS_T marked = EXT_ZERO;

    /* Accept multiple flags. */
    while (1) {
        name = one_argument (name, word);
        if (word[0] == '\0')
            break;
        flag = exact
            ? ext_flag_get_by_name_exact (flag_table, word)
            : ext_flag_get_by_name (flag_table, word);
        if (flag != NULL)
            EXT_SET (marked, flag->bit);
    }
    return marked;
}

const char *ext_flags_to_string (const EXT_FLAG_DEF_T *flag_table,
    EXT_FLAGS_T bits)
{
    return ext_flags_to_string_real (flag_table, bits, "none");
}

const char *ext_flags_to_string_real (const EXT_FLAG_DEF_T *flag_table,
    EXT_FLAGS_T bits, const char *none_str)
{
    static char buf[16][512];
    static int cnt = 0;
    int i;

    if (++cnt >= 16)
        cnt = 0;

    buf[cnt][0] = '\0';
    for (i = 0; flag_table[i].name != NULL; i++) {
        if (EXT_IS_SET (bits, flag_table[i].bit)) {
            if (buf[cnt][0] != '\0')
                strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[i].name);
        }
    }
    return (buf[cnt][0] == '\0') ? none_str : buf[cnt];
}
