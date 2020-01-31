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

#include <string.h>

#include "merc.h"

#include "interp.h"
#include "lookup.h"
#include "utils.h"

#include "flags.h"

flag_t flag_lookup (const FLAG_T *flag_table, const char *name)
    { SIMPLE_LOOKUP_PROP (flag_table, bit, name, FLAG_NONE, 0); }
flag_t flag_lookup_exact (const FLAG_T *flag_table, const char *name)
    { SIMPLE_LOOKUP_PROP_EXACT (flag_table, bit, name, FLAG_NONE, 0); }
const FLAG_T *flag_get_by_name (const FLAG_T *flag_table, const char *name)
    { SIMPLE_GET_BY_NAME (flag_table, name, 0); }
const FLAG_T *flag_get_by_name_exact (const FLAG_T *flag_table, char const *name)
    { SIMPLE_GET_BY_NAME_EXACT (flag_table, name, 0); }
const FLAG_T *flag_get (const FLAG_T *flag_table, flag_t bit)
    { SIMPLE_GET (flag_table, bit, name, NULL, 0); }
const char *flag_get_name (const FLAG_T *flag_table, flag_t bit)
    { SIMPLE_GET_NAME_FROM_ELEMENT (FLAG_T, flag_get (flag_table, bit), name); }

flag_t flags_from_string (const FLAG_T *flag_table, const char *name)
    { return flags_from_string_real (flag_table, name, FALSE); }
flag_t flags_from_string_exact (const FLAG_T *flag_table, const char *name)
    { return flags_from_string_real (flag_table, name, TRUE); }

flag_t flags_from_string_real (const FLAG_T *flag_table, const char *name,
    bool exact)
{
    const FLAG_T *flag;
    char word[MAX_INPUT_LENGTH];
    flag_t marked;

    /* Accept multiple flags. */
    marked = 0;
    while (1) {
        name = one_argument (name, word);
        if (word[0] == '\0')
            break;
        flag = exact
            ? flag_get_by_name_exact (flag_table, word)
            : flag_get_by_name (flag_table, word);
        if (flag != NULL)
            SET_BIT (marked, flag->bit);
    }
    return (marked != 0) ? marked : FLAG_NONE;
}

/* Increased buffers from 2 to 16! That should give us the illusion
 * of stability. -- Synival */
const char *flags_to_string (const FLAG_T *flag_table, flag_t bits)
    { return flags_to_string_real (flag_table, bits, "none"); }

const char *flags_to_string_real (const FLAG_T *flag_table, flag_t bits,
    const char *none_str)
{
    static char buf[16][512];
    static int cnt = 0;
    int i;

    if (++cnt >= 16)
        cnt = 0;

    buf[cnt][0] = '\0';
    for (i = 0; flag_table[i].name != NULL; i++) {
        if (IS_SET (bits, flag_table[i].bit)) {
            if (buf[cnt][0] != '\0')
                strcat (buf[cnt], " ");
            strcat (buf[cnt], flag_table[i].name);
        }
    }
    return (buf[cnt][0] == '\0') ? none_str : buf[cnt];
}

const FLAG_T plr_flags[] = {
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

const FLAG_T affect_flags[] = {
    {"blind",         AFF_BLIND,         TRUE},
    {"invisible",     AFF_INVISIBLE,     TRUE},
    {"detect_evil",   AFF_DETECT_EVIL,   TRUE},
    {"detect_invis",  AFF_DETECT_INVIS,  TRUE},
    {"detect_magic",  AFF_DETECT_MAGIC,  TRUE},
    {"detect_hidden", AFF_DETECT_HIDDEN, TRUE},
    {"detect_good",   AFF_DETECT_GOOD,   TRUE},
    {"sanctuary",     AFF_SANCTUARY,     TRUE},
    {"faerie_fire",   AFF_FAERIE_FIRE,   TRUE},
    {"infrared",      AFF_INFRARED,      TRUE},
    {"curse",         AFF_CURSE,         TRUE},
    {"unused_aff_1",  AFF_UNUSED_FLAG_1, FALSE},
    {"poison",        AFF_POISON,        TRUE},
    {"protect_evil",  AFF_PROTECT_EVIL,  TRUE},
    {"protect_good",  AFF_PROTECT_GOOD,  TRUE},
    {"sneak",         AFF_SNEAK,         TRUE},
    {"hide",          AFF_HIDE,          TRUE},
    {"sleep",         AFF_SLEEP,         TRUE},
    {"charm",         AFF_CHARM,         TRUE},
    {"flying",        AFF_FLYING,        TRUE},
    {"pass_door",     AFF_PASS_DOOR,     TRUE},
    {"haste",         AFF_HASTE,         TRUE},
    {"calm",          AFF_CALM,          TRUE},
    {"plague",        AFF_PLAGUE,        TRUE},
    {"weaken",        AFF_WEAKEN,        TRUE},
    {"dark_vision",   AFF_DARK_VISION,   TRUE},
    {"berserk",       AFF_BERSERK,       TRUE},
    {"swim",          AFF_SWIM,          TRUE},
    {"regeneration",  AFF_REGENERATION,  TRUE},
    {"slow",          AFF_SLOW,          TRUE},
    {0}
};

const FLAG_T off_flags[] = {
    {"area_attack",    OFF_AREA_ATTACK, TRUE},
    {"backstab",       OFF_BACKSTAB,    TRUE},
    {"bash",           OFF_BASH,        TRUE},
    {"berserk",        OFF_BERSERK,     TRUE},
    {"disarm",         OFF_DISARM,      TRUE},
    {"dodge",          OFF_DODGE,       TRUE},
    {"fade",           OFF_FADE,        TRUE},
    {"fast",           OFF_FAST,        TRUE},
    {"kick",           OFF_KICK,        TRUE},
    {"dirt_kick",      OFF_KICK_DIRT,   TRUE},
    {"parry",          OFF_PARRY,       TRUE},
    {"rescue",         OFF_RESCUE,      TRUE},
    {"tail",           OFF_TAIL,        TRUE},
    {"trip",           OFF_TRIP,        TRUE},
    {"crush",          OFF_CRUSH,       TRUE},
    {"assist_all",     ASSIST_ALL,      TRUE},
    {"assist_align",   ASSIST_ALIGN,    TRUE},
    {"assist_race",    ASSIST_RACE,     TRUE},
    {"assist_players", ASSIST_PLAYERS,  TRUE},
    {"assist_guard",   ASSIST_GUARD,    TRUE},
    {"assist_vnum",    ASSIST_VNUM,     TRUE},
    {0}
};

const FLAG_T form_flags[] = {
    {"edible",        FORM_EDIBLE,        TRUE},
    {"poison",        FORM_POISON,        TRUE},
    {"magical",       FORM_MAGICAL,       TRUE},
    {"instant_decay", FORM_INSTANT_DECAY, TRUE},
    {"other",         FORM_OTHER,         TRUE},
    {"unused_form_1", FORM_UNUSED_FLAG_1, FALSE},
    {"animal",        FORM_ANIMAL,        TRUE},
    {"sentient",      FORM_SENTIENT,      TRUE},
    {"undead",        FORM_UNDEAD,        TRUE},
    {"construct",     FORM_CONSTRUCT,     TRUE},
    {"mist",          FORM_MIST,          TRUE},
    {"intangible",    FORM_INTANGIBLE,    TRUE},
    {"biped",         FORM_BIPED,         TRUE},
    {"centaur",       FORM_CENTAUR,       TRUE},
    {"insect",        FORM_INSECT,        TRUE},
    {"spider",        FORM_SPIDER,        TRUE},
    {"crustacean",    FORM_CRUSTACEAN,    TRUE},
    {"worm",          FORM_WORM,          TRUE},
    {"blob",          FORM_BLOB,          TRUE},
    {"unused_form_2", FORM_UNUSED_FLAG_2, FALSE},
    {"unused_form_3", FORM_UNUSED_FLAG_3, FALSE},
    {"mammal",        FORM_MAMMAL,        TRUE},
    {"bird",          FORM_BIRD,          TRUE},
    {"reptile",       FORM_REPTILE,       TRUE},
    {"snake",         FORM_SNAKE,         TRUE},
    {"dragon",        FORM_DRAGON,        TRUE},
    {"amphibian",     FORM_AMPHIBIAN,     TRUE},
    {"fish",          FORM_FISH,          TRUE},
    {"cold_blood",    FORM_COLD_BLOOD,    TRUE},
    {0}
};

const FLAG_T part_flags[] = {
    {"head",          PART_HEAD,          TRUE},
    {"arms",          PART_ARMS,          TRUE},
    {"legs",          PART_LEGS,          TRUE},
    {"heart",         PART_HEART,         TRUE},
    {"brains",        PART_BRAINS,        TRUE},
    {"guts",          PART_GUTS,          TRUE},
    {"hands",         PART_HANDS,         TRUE},
    {"feet",          PART_FEET,          TRUE},
    {"fingers",       PART_FINGERS,       TRUE},
    {"ear",           PART_EAR,           TRUE},
    {"eye",           PART_EYE,           TRUE},
    {"long_tongue",   PART_LONG_TONGUE,   TRUE},
    {"eyestalks",     PART_EYESTALKS,     TRUE},
    {"tentacles",     PART_TENTACLES,     TRUE},
    {"fins",          PART_FINS,          TRUE},
    {"wings",         PART_WINGS,         TRUE},
    {"tail",          PART_TAIL,          TRUE},
    {"unused_part_1", PART_UNUSED_FLAG_1, FALSE},
    {"unused_part_2", PART_UNUSED_FLAG_2, FALSE},
    {"unused_part_3", PART_UNUSED_FLAG_3, FALSE},
    {"claws",         PART_CLAWS,         TRUE},
    {"fangs",         PART_FANGS,         TRUE},
    {"horns",         PART_HORNS,         TRUE},
    {"scales",        PART_SCALES,        TRUE},
    {"tusks",         PART_TUSKS,         TRUE},
    {0}
};

const FLAG_T comm_flags[] = {
    {"quiet",         COMM_QUIET,         TRUE},
    {"deaf",          COMM_DEAF,          TRUE},
    {"nowiz",         COMM_NOWIZ,         TRUE},
    {"noclangossip",  COMM_NOAUCTION,     TRUE},
    {"nogossip",      COMM_NOGOSSIP,      TRUE},
    {"noquestion",    COMM_NOQUESTION,    TRUE},
    {"nomusic",       COMM_NOMUSIC,       TRUE},
    {"noclan",        COMM_NOCLAN,        TRUE},
    {"noquote",       COMM_NOQUOTE,       TRUE},
    {"shoutsoff",     COMM_SHOUTSOFF,     TRUE},
    {"unused_comm_1", COMM_UNUSED_FLAG_1, FALSE},
    {"compact",       COMM_COMPACT,       TRUE},
    {"brief",         COMM_BRIEF,         TRUE},
    {"prompt",        COMM_PROMPT,        TRUE},
    {"combine",       COMM_COMBINE,       TRUE},
    {"telnet_ga",     COMM_TELNET_GA,     TRUE},
    {"show_affects",  COMM_SHOW_AFFECTS,  TRUE},
    {"nograts",       COMM_NOGRATS,       TRUE},
    {"unused_comm_2", COMM_UNUSED_FLAG_2, FALSE},
    {"noemote",       COMM_NOEMOTE,       FALSE},
    {"noshout",       COMM_NOSHOUT,       FALSE},
    {"notell",        COMM_NOTELL,        FALSE},
    {"nochannels",    COMM_NOCHANNELS,    FALSE},
    {"unused_comm_3", COMM_UNUSED_FLAG_3, FALSE},
    {"snoop_proof",   COMM_SNOOP_PROOF,   FALSE},
    {"afk",           COMM_AFK,           TRUE},
    {0}
};

const FLAG_T mprog_flags[] = {
    {"act",    TRIG_ACT,    TRUE},
    {"bribe",  TRIG_BRIBE,  TRUE},
    {"death",  TRIG_DEATH,  TRUE},
    {"entry",  TRIG_ENTRY,  TRUE},
    {"fight",  TRIG_FIGHT,  TRUE},
    {"give",   TRIG_GIVE,   TRUE},
    {"greet",  TRIG_GREET,  TRUE},
    {"grall",  TRIG_GRALL,  TRUE},
    {"kill",   TRIG_KILL,   TRUE},
    {"hpcnt",  TRIG_HPCNT,  TRUE},
    {"random", TRIG_RANDOM, TRUE},
    {"speech", TRIG_SPEECH, TRUE},
    {"exit",   TRIG_EXIT,   TRUE},
    {"exall",  TRIG_EXALL,  TRUE},
    {"delay",  TRIG_DELAY,  TRUE},
    {"surrender", TRIG_SURR, TRUE},
    {0}
};

const FLAG_T area_flags[] = {
    {"changed", AREA_CHANGED, TRUE},
    {"added",   AREA_ADDED,   TRUE},
    {"loading", AREA_LOADING, FALSE},
    {0}
};

const FLAG_T exit_flags[] = {
    {"door",          EX_ISDOOR,        TRUE},
    {"closed",        EX_CLOSED,        TRUE},
    {"locked",        EX_LOCKED,        TRUE},
    {"unused_exit_1", EX_UNUSED_FLAG_1, FALSE},
    {"unused_exit_2", EX_UNUSED_FLAG_2, FALSE},
    {"pickproof",     EX_PICKPROOF,     TRUE},
    {"nopass",        EX_NOPASS,        TRUE},
    {"easy",          EX_EASY,          TRUE},
    {"hard",          EX_HARD,          TRUE},
    {"infuriating",   EX_INFURIATING,   TRUE},
    {"noclose",       EX_NOCLOSE,       TRUE},
    {"nolock",        EX_NOLOCK,        TRUE},
    {0}
};

const FLAG_T room_flags[] = {
    {"dark",          ROOM_DARK,          TRUE},
    {"unused_room_1", ROOM_UNUSED_FLAG_1, FALSE},
    {"no_mob",        ROOM_NO_MOB,        TRUE},
    {"indoors",       ROOM_INDOORS,       TRUE},
    {"unused_room_2", ROOM_UNUSED_FLAG_2, FALSE},
    {"unused_room_3", ROOM_UNUSED_FLAG_3, FALSE},
    {"unused_room_4", ROOM_UNUSED_FLAG_4, FALSE},
    {"unused_room_5", ROOM_UNUSED_FLAG_5, FALSE},
    {"unused_room_6", ROOM_UNUSED_FLAG_6, FALSE},
    {"private",       ROOM_PRIVATE,       TRUE},
    {"safe",          ROOM_SAFE,          TRUE},
    {"solitary",      ROOM_SOLITARY,      TRUE},
    {"pet_shop",      ROOM_PET_SHOP,      TRUE},
    {"no_recall",     ROOM_NO_RECALL,     TRUE},
    {"imp_only",      ROOM_IMP_ONLY,      TRUE},
    {"gods_only",     ROOM_GODS_ONLY,     TRUE},
    {"heroes_only",   ROOM_HEROES_ONLY,   TRUE},
    {"newbies_only",  ROOM_NEWBIES_ONLY,  TRUE},
    {"law",           ROOM_LAW,           TRUE},
    {"nowhere",       ROOM_NOWHERE,       TRUE},
    {0}
};

const FLAG_T extra_flags[] = {
    {"glow",          ITEM_GLOW,          TRUE},
    {"hum",           ITEM_HUM,           TRUE},
    {"dark",          ITEM_DARK,          TRUE},
    {"lock",          ITEM_LOCK,          TRUE},
    {"evil",          ITEM_EVIL,          TRUE},
    {"invis",         ITEM_INVIS,         TRUE},
    {"magic",         ITEM_MAGIC,         TRUE},
    {"nodrop",        ITEM_NODROP,        TRUE},
    {"bless",         ITEM_BLESS,         TRUE},
    {"antigood",      ITEM_ANTI_GOOD,     TRUE},
    {"antievil",      ITEM_ANTI_EVIL,     TRUE},
    {"antineutral",   ITEM_ANTI_NEUTRAL,  TRUE},
    {"noremove",      ITEM_NOREMOVE,      TRUE},
    {"inventory",     ITEM_INVENTORY,     TRUE},
    {"nopurge",       ITEM_NOPURGE,       TRUE},
    {"rotdeath",      ITEM_ROT_DEATH,     TRUE},
    {"visdeath",      ITEM_VIS_DEATH,     TRUE},
    {"unused_extra_1",ITEM_UNUSED_FLAG_1, FALSE},
    {"nonmetal",      ITEM_NONMETAL,      TRUE},
    {"nolocate",      ITEM_NOLOCATE,      TRUE},
    {"meltdrop",      ITEM_MELT_DROP,     TRUE},
    {"hadtimer",      ITEM_HAD_TIMER,     TRUE},
    {"sellextract",   ITEM_SELL_EXTRACT,  TRUE},
    {"unused_extra_2",ITEM_UNUSED_FLAG_2, FALSE},
    {"burnproof",     ITEM_BURN_PROOF,    TRUE},
    {"nouncurse",     ITEM_NOUNCURSE,     TRUE},
    {"corroded",      ITEM_CORRODED,      TRUE},
    {0}
};

const FLAG_T wear_flags[] = {
    {"take",      ITEM_TAKE,        TRUE},
    {"finger",    ITEM_WEAR_FINGER, TRUE},
    {"neck",      ITEM_WEAR_NECK,   TRUE},
    {"body",      ITEM_WEAR_BODY,   TRUE},
    {"head",      ITEM_WEAR_HEAD,   TRUE},
    {"legs",      ITEM_WEAR_LEGS,   TRUE},
    {"feet",      ITEM_WEAR_FEET,   TRUE},
    {"hands",     ITEM_WEAR_HANDS,  TRUE},
    {"arms",      ITEM_WEAR_ARMS,   TRUE},
    {"shield",    ITEM_WEAR_SHIELD, TRUE},
    {"about",     ITEM_WEAR_ABOUT,  TRUE},
    {"waist",     ITEM_WEAR_WAIST,  TRUE},
    {"wrist",     ITEM_WEAR_WRIST,  TRUE},
    {"wield",     ITEM_WIELD,       TRUE},
    {"hold",      ITEM_HOLD,        TRUE},
    {"nosac",     ITEM_NO_SAC,      TRUE},
    {"wearfloat", ITEM_WEAR_FLOAT,  TRUE},
    {0}
};

const FLAG_T container_flags[] = {
    {"closeable", CONT_CLOSEABLE, TRUE},
    {"pickproof", CONT_PICKPROOF, TRUE},
    {"closed",    CONT_CLOSED,    TRUE},
    {"locked",    CONT_LOCKED,    TRUE},
    {"puton",     CONT_PUT_ON,    TRUE},
    {0}
};

const FLAG_T weapon_flags[] = {
    {"flaming",  WEAPON_FLAMING,   TRUE},
    {"frost",    WEAPON_FROST,     TRUE},
    {"vampiric", WEAPON_VAMPIRIC,  TRUE},
    {"sharp",    WEAPON_SHARP,     TRUE},
    {"vorpal",   WEAPON_VORPAL,    TRUE},
    {"twohands", WEAPON_TWO_HANDS, TRUE},
    {"shocking", WEAPON_SHOCKING,  TRUE},
    {"poison",   WEAPON_POISON,    TRUE},
    {0}
};

const FLAG_T res_flags[] = {
    {"summon",        RES_SUMMON,        TRUE},
    {"charm",         RES_CHARM,         TRUE},
    {"magic",         RES_MAGIC,         TRUE},
    {"weapon",        RES_WEAPON,        TRUE},
    {"bash",          RES_BASH,          TRUE},
    {"pierce",        RES_PIERCE,        TRUE},
    {"slash",         RES_SLASH,         TRUE},
    {"fire",          RES_FIRE,          TRUE},
    {"cold",          RES_COLD,          TRUE},
    {"lightning",     RES_LIGHTNING,     TRUE},
    {"acid",          RES_ACID,          TRUE},
    {"poison",        RES_POISON,        TRUE},
    {"negative",      RES_NEGATIVE,      TRUE},
    {"holy",          RES_HOLY,          TRUE},
    {"energy",        RES_ENERGY,        TRUE},
    {"mental",        RES_MENTAL,        TRUE},
    {"disease",       RES_DISEASE,       TRUE},
    {"drowning",      RES_DROWNING,      TRUE},
    {"light",         RES_LIGHT,         TRUE},
    {"sound",         RES_SOUND,         TRUE},
    {"unused_res_1",  RES_UNUSED_FLAG_1, FALSE},
    {"unused_res_2",  RES_UNUSED_FLAG_2, FALSE},
    {"unused_res_3",  RES_UNUSED_FLAG_3, FALSE},
    {"wood",          RES_WOOD,          TRUE},
    {"silver",        RES_SILVER,        TRUE},
    {"iron",          RES_IRON,          TRUE},
    {0}
};

const FLAG_T gate_flags[] = {
    {"normal_exit", GATE_NORMAL_EXIT, TRUE},
    {"no_curse",    GATE_NOCURSE,     TRUE},
    {"go_with",     GATE_GOWITH,      TRUE},
    {"buggy",       GATE_BUGGY,       TRUE},
    {"random",      GATE_RANDOM,      TRUE},
    {0}
};

const FLAG_T furniture_flags[] = {
    {"stand_at",   STAND_AT,   TRUE},
    {"stand_on",   STAND_ON,   TRUE},
    {"stand_in",   STAND_IN,   TRUE},
    {"sit_at",     SIT_AT,     TRUE},
    {"sit_on",     SIT_ON,     TRUE},
    {"sit_in",     SIT_IN,     TRUE},
    {"rest_at",    REST_AT,    TRUE},
    {"rest_on",    REST_ON,    TRUE},
    {"rest_in",    REST_IN,    TRUE},
    {"sleep_at",   SLEEP_AT,   TRUE},
    {"sleep_on",   SLEEP_ON,   TRUE},
    {"sleep_in",   SLEEP_IN,   TRUE},
    {"put_at",     PUT_AT,     TRUE},
    {"put_on",     PUT_ON,     TRUE},
    {"put_in",     PUT_IN,     TRUE},
    {"put_inside", PUT_INSIDE, TRUE},
    {0}
};

const FLAG_T dam_flags[] = {
    {"magical", DAM_MAGICAL, TRUE},
    {0}
};
