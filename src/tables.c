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

#include <stddef.h>

#include "skills.h"
// #include "magic.h"
#include "nanny.h"
#include "lookup.h"
#include "recycle.h"
#include "colour.h"
#include "board.h"
#include "special.h"
#include "effects.h"
#include "json_tbl.h"
#include "magic.h"

#include "spell_aff.h"
#include "spell_create.h"
#include "spell_cure.h"
#include "spell_info.h"
#include "spell_misc.h"
#include "spell_move.h"
#include "spell_npc.h"
#include "spell_off.h"

/* TODO: move convenience macros to tables.h */
/* TODO: feature - more output functions for JSON objects. */
/* TODO: review table lookups for proper 'not found' entries
 *       (should be -1) */
/* TODO: we should hardly ever have -1 as an index in any of these tables */

#define TFLAGS(table, desc) \
    { table, #table, TABLE_FLAG_TYPE | TABLE_BITS, desc, \
      sizeof (FLAG_TYPE), json_tblw_flag }
#define TTYPES(table, desc) \
    { table, #table, TABLE_FLAG_TYPE, desc, \
      sizeof (FLAG_TYPE), json_tblw_flag }
#define TTABLE(table, desc, jwrite) \
    { table, #table, 0, desc, sizeof(table[0]), jwrite }

const TABLE_TYPE master_table[] = {
    /* from flags.h */
    TFLAGS (mob_flags,        "Mobile flags."),
    TFLAGS (plr_flags,        "Player flags."),
    TFLAGS (affect_flags,     "Mobile affects."),
    TFLAGS (off_flags,        "Mobile offensive behaviour."),
    TFLAGS (form_flags,       "Mobile body form."),
    TFLAGS (part_flags,       "Mobile body parts."),
    TFLAGS (comm_flags,       "Communication channel flags."),
    TFLAGS (mprog_flags,      "MobProgram flags."),
    TFLAGS (area_flags,       "Area attributes."),
    TFLAGS (exit_flags,       "Exit types."),
    TFLAGS (room_flags,       "Room attributes."),
    TFLAGS (extra_flags,      "Object attributes."),
    TFLAGS (wear_flags,       "Where to wear object."),
    TFLAGS (container_flags,  "Container status."),
    TFLAGS (weapon_flags,     "Special weapon type."),
    TFLAGS (res_flags,        "Mobile immunity."),
    TFLAGS (portal_flags,     "Portal types."),
    TFLAGS (furniture_flags,  "Flags for furniture."),

    /* from types.h */
    TTYPES (sex_types,        "Sexes."),
    TTYPES (affect_apply_types, "Affect apply types."),
    TTYPES (wear_loc_phrases, "Phrases for wear locations."),
    TTYPES (wear_loc_types,   "Where mobile wears object."),
    TTYPES (ac_types,         "AC for different attacks."),
    TTYPES (size_types,       "Mobile sizes."),
    TTYPES (weapon_types,     "Weapon classes."),
    TTYPES (position_types,   "Mobile positions."),
    TTYPES (sector_types,     "Sector types, terrain."),
    TTYPES (item_types,       "Types of objects."),
    TTYPES (door_resets,      "Door reset types"),

    /* from tables.h */
    /* TODO: json writing functions for these tables ->->-->->-, */
    TTABLE (clan_table,       "Player clans.",                NULL),
    TTABLE (position_table,   "Character positions.",         NULL),
    TTABLE (sex_table,        "Gender settings.",             NULL),
    TTABLE (size_table,       "Character sizes.",             NULL),
    TTABLE (item_table,       "Item types and properties.",   NULL),
    TTABLE (weapon_table,     "Weapon types and properties.", NULL),
    TTABLE (dam_table,        "Damage types and properties.", NULL),
    TTABLE (attack_table,     "Attack types and properties.", NULL),
    TTABLE (race_table,       "Races and statistics.",        NULL),
    TTABLE (pc_race_table,    "Playable race data.",          NULL),
    TTABLE (class_table,      "Classes and statistics.",      NULL),
    TTABLE (str_app,          "Str apply table.",             NULL),
    TTABLE (int_app,          "Int apply table.",             NULL),
    TTABLE (wis_app,          "Wis apply table.",             NULL),
    TTABLE (dex_app,          "Dex apply table.",             NULL),
    TTABLE (con_app,          "Con apply table.",             NULL),
    TTABLE (liq_table,        "Liquid types.",                NULL),
    TTABLE (skill_table,      "Master skill table.",          NULL),
    TTABLE (group_table,      "Ability group table.",         NULL),
    TTABLE (sector_table,     "Sector/terrain properties.",   NULL),
    TTABLE (nanny_table,      "Descriptor 'Nanny' table.",    NULL),
    TTABLE (door_table,       "Exit names.",                  NULL),
    TTABLE (spec_table,       "Specialized mobile behavior.", NULL),
    TTABLE (furniture_table, "Furniture flags for positions.",NULL),
    TTABLE (wear_table,       "Wearable item table.",         NULL),
    TTABLE (material_table,   "Material properties",          NULL),
    TTABLE (colour_setting_table, "Configurable colours.",    NULL),
    TTABLE (wiznet_table,     "Wiznet channels.",             NULL),
    TTABLE (map_lookup_table, "Types for object mappings.",   NULL),
    TTABLE (map_flags_table,  "Flags for object mappings.",   NULL),
    TTABLE (obj_map_table,    "Obj type-values[] mappings.",  NULL),
    TTABLE (colour_table,     "Colour values.",               NULL),
    TTABLE (recycle_table,    "Recycleable object types.",    NULL),
    TTABLE (board_table,      "Discussion boards.",           NULL),
    TTABLE (affect_bit_table, "Affect bit vector types.",     NULL),
    {0}
};

/* for clans */
const CLAN_TYPE clan_table[CLAN_MAX + 1] = {
    /* name, who entry, death-transfer room, independent */
    /* independent should be FALSE if is a real clan */
    {"",      "",           ROOM_VNUM_ALTAR, TRUE},
    {"loner", "[ Loner ] ", ROOM_VNUM_ALTAR, TRUE},
    {"rom",   "[  ROM  ] ", ROOM_VNUM_ALTAR, FALSE},
    {0},
};

/* for position */
const POSITION_TYPE position_table[POS_MAX + 1] = {
    {POS_DEAD,     "dead",             "dead"},
    {POS_MORTAL,   "mortally wounded", "mort"},
    {POS_INCAP,    "incapacitated",    "incap"},
    {POS_STUNNED,  "stunned",          "stun"},
    {POS_SLEEPING, "sleeping",         "sleep"},
    {POS_RESTING,  "resting",          "rest"},
    {POS_SITTING,  "sitting",          "sit"},
    {POS_FIGHTING, "fighting",         "fight"},
    {POS_STANDING, "standing",         "stand"},
    {0},
};

/* for sex */
const SEX_TYPE sex_table[SEX_MAX + 1] = {
    {SEX_NEUTRAL, "none"},
    {SEX_MALE,    "male"},
    {SEX_FEMALE,  "female"},
    {SEX_EITHER,  "either"},
    {0},
};

/* for sizes */
const SIZE_TYPE size_table[SIZE_MAX_R + 1] = {
    {SIZE_TINY,   "tiny"},
    {SIZE_SMALL,  "small"},
    {SIZE_MEDIUM, "medium"},
    {SIZE_LARGE,  "large"},
    {SIZE_HUGE,   "huge",},
    {SIZE_GIANT,  "giant"},
    {0},
};

/* item type list */
const ITEM_TYPE item_table[ITEM_MAX + 1] = {
    {ITEM_LIGHT,      "light"},
    {ITEM_SCROLL,     "scroll"},
    {ITEM_WAND,       "wand"},
    {ITEM_STAFF,      "staff"},
    {ITEM_WEAPON,     "weapon"},
    {ITEM_TREASURE,   "treasure"},
    {ITEM_ARMOR,      "armor"},
    {ITEM_POTION,     "potion"},
    {ITEM_CLOTHING,   "clothing"},
    {ITEM_FURNITURE,  "furniture"},
    {ITEM_TRASH,      "trash"},
    {ITEM_CONTAINER,  "container"},
    {ITEM_DRINK_CON,  "drink"},
    {ITEM_KEY,        "key"},
    {ITEM_FOOD,       "food"},
    {ITEM_MONEY,      "money"},
    {ITEM_BOAT,       "boat"},
    {ITEM_CORPSE_NPC, "npc_corpse"},
    {ITEM_CORPSE_PC,  "pc_corpse"},
    {ITEM_FOUNTAIN,   "fountain"},
    {ITEM_PILL,       "pill"},
    {ITEM_PROTECT,    "protect"},
    {ITEM_MAP,        "map"},
    {ITEM_PORTAL,     "portal"},
    {ITEM_WARP_STONE, "warp_stone"},
    {ITEM_ROOM_KEY,   "room_key"},
    {ITEM_GEM,        "gem"},
    {ITEM_JEWELRY,    "jewelry"},
    {ITEM_JUKEBOX,    "jukebox"},
    {-1, NULL}
};

/* weapon selection table */
const WEAPON_TYPE weapon_table[WEAPON_MAX + 1] = {
    {WEAPON_SWORD,   "sword",   OBJ_VNUM_SCHOOL_SWORD,   &gsn_sword},
    {WEAPON_MACE,    "mace",    OBJ_VNUM_SCHOOL_MACE,    &gsn_mace},
    {WEAPON_DAGGER,  "dagger",  OBJ_VNUM_SCHOOL_DAGGER,  &gsn_dagger},
    {WEAPON_AXE,     "axe",     OBJ_VNUM_SCHOOL_AXE,     &gsn_axe},
    {WEAPON_SPEAR,   "staff",   OBJ_VNUM_SCHOOL_STAFF,   &gsn_spear},
    {WEAPON_FLAIL,   "flail",   OBJ_VNUM_SCHOOL_FLAIL,   &gsn_flail},
    {WEAPON_WHIP,    "whip",    OBJ_VNUM_SCHOOL_WHIP,    &gsn_whip},
    {WEAPON_POLEARM, "polearm", OBJ_VNUM_SCHOOL_POLEARM, &gsn_polearm},
    {-1, NULL, 0, NULL}
};

const DAM_TYPE dam_table[DAM_MAX + 1] = {
    {DAM_NONE,      "none",      -1,            empty_effect},
    {DAM_BASH,      "bash",      RES_BASH,      empty_effect},
    {DAM_PIERCE,    "pierce",    RES_PIERCE,    empty_effect},
    {DAM_SLASH,     "slash",     RES_SLASH,     empty_effect},
    {DAM_FIRE,      "fire",      RES_FIRE,      fire_effect},
    {DAM_COLD,      "cold",      RES_COLD,      cold_effect},
    {DAM_LIGHTNING, "lightning", RES_LIGHTNING, shock_effect},
    {DAM_ACID,      "acid",      RES_ACID,      acid_effect},
    {DAM_POISON,    "poison",    RES_POISON,    poison_effect},
    {DAM_NEGATIVE,  "negative",  RES_NEGATIVE,  empty_effect},
    {DAM_HOLY,      "holy",      RES_HOLY,      empty_effect},
    {DAM_ENERGY,    "energy",    RES_ENERGY,    empty_effect},
    {DAM_MENTAL,    "mental",    RES_MENTAL,    empty_effect},
    {DAM_DISEASE,   "disease",   RES_DISEASE,   empty_effect},
    {DAM_DROWNING,  "drowning",  RES_DROWNING,  empty_effect},
    {DAM_LIGHT,     "light",     RES_LIGHT,     empty_effect},
    {DAM_OTHER,     "other",     -1,            empty_effect},
    {DAM_HARM,      "harm",      -1,            empty_effect},
    {DAM_CHARM,     "charm",     RES_CHARM,     empty_effect},
    {DAM_SOUND,     "sound",     RES_SOUND,     empty_effect},
    {0}
};

/* attack table  -- not very organized :( */
const ATTACK_TYPE attack_table[ATTACK_MAX + 1] = {
    {"none",      "hit",           -1},          /*  0 */
    {"slice",     "slice",         DAM_SLASH},
    {"stab",      "stab",          DAM_PIERCE},
    {"slash",     "slash",         DAM_SLASH},
    {"whip",      "whip",          DAM_SLASH},
    {"claw",      "claw",          DAM_SLASH},   /*  5 */
    {"blast",     "blast",         DAM_BASH},
    {"pound",     "pound",         DAM_BASH},
    {"crush",     "crush",         DAM_BASH},
    {"grep",      "grep",          DAM_SLASH},
    {"bite",      "bite",          DAM_PIERCE},  /* 10 */
    {"pierce",    "pierce",        DAM_PIERCE},
    {"suction",   "suction",       DAM_BASH},
    {"beating",   "beating",       DAM_BASH},
    {"digestion", "digestion",     DAM_ACID},
    {"charge",    "charge",        DAM_BASH},    /* 15 */
    {"slap",      "slap",          DAM_BASH},
    {"punch",     "punch",         DAM_BASH},
    {"wrath",     "wrath",         DAM_ENERGY},
    {"magic",     "magic",         DAM_ENERGY},
    {"divine",    "divine power",  DAM_HOLY},    /* 20 */
    {"cleave",    "cleave",        DAM_SLASH},
    {"scratch",   "scratch",       DAM_PIERCE},
    {"peck",      "peck",          DAM_PIERCE},
    {"peckb",     "peck",          DAM_BASH},
    {"chop",      "chop",          DAM_SLASH},   /* 25 */
    {"sting",     "sting",         DAM_PIERCE},
    {"smash",     "smash",         DAM_BASH},
    {"shbite",    "shocking bite", DAM_LIGHTNING},
    {"flbite",    "flaming bite",  DAM_FIRE},
    {"frbite",    "freezing bite", DAM_COLD},    /* 30 */
    {"acbite",    "acidic bite",   DAM_ACID},
    {"chomp",     "chomp",         DAM_PIERCE},
    {"drain",     "life drain",    DAM_NEGATIVE},
    {"thrust",    "thrust",        DAM_PIERCE},
    {"slime",     "slime",         DAM_ACID},    /* 35 */
    {"shock",     "shock",         DAM_LIGHTNING},
    {"thwack",    "thwack",        DAM_BASH},
    {"flame",     "flame",         DAM_FIRE},
    {"chill",     "chill",         DAM_COLD},
    {NULL, NULL, -1}
};

#define FORMS_HUMANOID \
    (FORM_EDIBLE | FORM_SENTIENT | FORM_BIPED | FORM_MAMMAL)
#define FORMS_MAMMAL \
    (FORM_EDIBLE | FORM_ANIMAL | FORM_MAMMAL)
#define FORMS_BIRD \
    (FORM_EDIBLE | FORM_ANIMAL | FORM_BIRD)
#define FORMS_BUG \
    (FORM_EDIBLE | FORM_ANIMAL | FORM_INSECT)

#define PARTS_ALIVE \
    (PART_HEART | PART_BRAINS | PART_GUTS)
#define PARTS_QUADRUPED \
    (PART_HEAD | PART_LEGS | PARTS_ALIVE | PART_FEET | PART_EAR | PART_EYE)
#define PARTS_BIPED \
    (PART_HEAD | PART_ARMS | PART_LEGS | PARTS_ALIVE | PART_FEET | PART_EAR | PART_EYE)
#define PARTS_HUMANOID \
    (PARTS_BIPED | PART_HANDS | PART_FINGERS)
#define PARTS_FELINE \
    (PARTS_QUADRUPED | PART_FANGS | PART_TAIL | PART_CLAWS)
#define PARTS_CANINE \
    (PARTS_QUADRUPED | PART_FANGS)
#define PARTS_REPTILE \
    (PARTS_ALIVE | PART_HEAD | PART_EYE | PART_LONG_TONGUE | PART_TAIL | PART_SCALES)
#define PARTS_LIZARD \
    (PARTS_QUADRUPED | PARTS_REPTILE)
#define PARTS_BIRD \
    (PARTS_ALIVE | PART_HEAD | PART_LEGS | PART_FEET | PART_EYE | PART_WINGS)

/* race table */
const RACE_TYPE race_table[RACE_MAX + 1] = {

 /* {name, pc_race?, act bits, aff_by bits, off bits, imm, res, vuln, form, parts} */
    {"unique",         FALSE, 0, 0, 0, 0, 0, 0, 0, 0},
    {"human",          TRUE,  0, 0, 0, 0, 0, 0, FORMS_HUMANOID, PARTS_HUMANOID},
    {"elf",            TRUE,  0, AFF_INFRARED, 0, 0, RES_CHARM, RES_IRON, FORMS_HUMANOID, PARTS_HUMANOID},
    {"dwarf",          TRUE,  0, AFF_INFRARED, 0, 0, RES_POISON | RES_DISEASE, RES_DROWNING, FORMS_HUMANOID, PARTS_HUMANOID},
    {"giant",          TRUE,  0, 0, 0, 0, RES_FIRE | RES_COLD, RES_MENTAL | RES_LIGHTNING, FORMS_HUMANOID, PARTS_HUMANOID},
#ifndef VANILLA
    {"pixie",          TRUE,  0, AFF_FLYING | AFF_DETECT_GOOD | AFF_DETECT_EVIL | AFF_DETECT_MAGIC, 0, 0, 0, 0, FORMS_HUMANOID | FORM_MAGICAL, PARTS_HUMANOID | PART_WINGS},
#else
    {"pixie",          FALSE, 0, AFF_FLYING | AFF_DETECT_GOOD | AFF_DETECT_EVIL | AFF_DETECT_MAGIC, 0, 0, 0, 0, FORMS_HUMANOID | FORM_MAGICAL, PARTS_HUMANOID | PART_WINGS},
#endif
    {"bat",            FALSE, 0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST, 0, 0, RES_LIGHT, FORMS_MAMMAL, PARTS_QUADRUPED | PART_WINGS},
    {"bear",           FALSE, 0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK, 0, RES_BASH | RES_COLD, 0, FORMS_MAMMAL, PARTS_BIPED | PART_CLAWS | PART_FANGS},
    {"cat",            FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, FORMS_MAMMAL, PARTS_FELINE},
    {"centipede",      FALSE, 0, AFF_DARK_VISION, 0, 0, RES_PIERCE | RES_COLD, RES_BASH, FORMS_BUG | FORM_POISON, PART_HEAD | PART_LEGS | PART_EYE},
    {"dog",            FALSE, 0, 0, OFF_FAST, 0, 0, 0, FORMS_MAMMAL, PARTS_CANINE | PART_CLAWS},
    {"doll",           FALSE, 0, 0, 0, RES_COLD | RES_POISON | RES_HOLY | RES_NEGATIVE | RES_MENTAL | RES_DISEASE | RES_DROWNING, RES_BASH | RES_LIGHT, RES_SLASH | RES_FIRE | RES_ACID | RES_LIGHTNING | RES_ENERGY, E|J|M|cc, PARTS_HUMANOID & ~(PARTS_ALIVE | PART_EAR)},
    {"dragon",         FALSE, 0, AFF_INFRARED | AFF_FLYING, 0, 0, RES_FIRE | RES_BASH | RES_CHARM, RES_PIERCE | RES_COLD, A|H|Z, PARTS_LIZARD | PART_FINGERS | PART_CLAWS | PART_FANGS},
    {"fido",           FALSE, 0, 0, OFF_DODGE | ASSIST_RACE, 0, 0, RES_MAGIC, FORMS_MAMMAL | FORM_POISON, PARTS_CANINE | PART_TAIL},
    {"fox",            FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, FORMS_MAMMAL, PARTS_CANINE | PART_TAIL},
    {"goblin",         FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE, RES_MAGIC, FORMS_HUMANOID, PARTS_HUMANOID},
    {"hobgoblin",      FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE | RES_POISON, 0, FORMS_HUMANOID, PARTS_HUMANOID | PART_TUSKS},
    {"kobold",         FALSE, 0, AFF_INFRARED, 0, 0, RES_POISON, RES_MAGIC, FORMS_HUMANOID | FORM_POISON, PARTS_HUMANOID | PART_TAIL},
    {"lizard",         FALSE, 0, 0, 0, 0, RES_POISON, RES_COLD, A|G|X|cc, PARTS_LIZARD},
    {"modron",         FALSE, 0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN, RES_CHARM | RES_DISEASE | RES_MENTAL | RES_HOLY | RES_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID, 0, FORM_SENTIENT, PARTS_HUMANOID & ~(PARTS_ALIVE | PART_FINGERS)},
    {"orc",            FALSE, 0, AFF_INFRARED, 0, 0, RES_DISEASE, RES_LIGHT, FORMS_HUMANOID, PARTS_HUMANOID},
    {"pig",            FALSE, 0, 0, 0, 0, 0, 0, FORMS_MAMMAL, PARTS_QUADRUPED},
    {"rabbit",         FALSE, 0, 0, OFF_DODGE | OFF_FAST, 0, 0, 0, FORMS_MAMMAL, PARTS_QUADRUPED},
    {"school monster", FALSE, MOB_NOALIGN, 0, 0, RES_CHARM | RES_SUMMON, 0, RES_MAGIC, A|M|V, PARTS_BIPED | PART_TAIL | PART_CLAWS},
    {"snake",          FALSE, 0, 0, 0, 0, RES_POISON, RES_COLD, A|G|X|Y|cc, PARTS_REPTILE | PART_FANGS},
    {"song bird",      FALSE, 0, AFF_FLYING, OFF_FAST | OFF_DODGE, 0, 0, 0, FORMS_BIRD, PARTS_BIRD},
    {"troll",          FALSE, 0, AFF_REGENERATION | AFF_INFRARED | AFF_DETECT_HIDDEN, OFF_BERSERK, 0, RES_CHARM | RES_BASH, RES_FIRE | RES_ACID, FORMS_HUMANOID | FORM_POISON, PARTS_HUMANOID | PART_CLAWS | PART_FANGS},
    {"water fowl",     FALSE, 0, AFF_SWIM | AFF_FLYING, 0, 0, RES_DROWNING, 0, FORMS_BIRD, PARTS_BIRD},
    {"wolf",           FALSE, 0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE, 0, 0, 0, FORMS_MAMMAL, PARTS_CANINE | PART_CLAWS | PART_TAIL},
    {"wyvern",         FALSE, 0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN, OFF_BASH | OFF_FAST | OFF_DODGE, RES_POISON, 0, RES_LIGHT, A|B|G|Z, PARTS_LIZARD | PART_FANGS},
    {0}
};

const PC_RACE_TYPE pc_race_table[PC_RACE_MAX + 1] = {
 /* {"race name", short name, points, { class multipliers }, { bonus skills }, { base stats }, { max stats }, size}, */
    {"null race",   "", 0, {100, 100, 100, 100}, {""},                     {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},
    {"human",  "Human", 0, {100, 100, 100, 100}, {""},                     {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, SIZE_MEDIUM},
    {"elf",    " Elf ", 5, {100, 125, 100, 125}, {"sneak", "hide"},        {12, 14, 13, 15, 11}, {16, 20, 18, 21, 15}, SIZE_SMALL},
    {"dwarf",  "Dwarf", 8, {150, 100, 125, 100}, {"berserk"},              {14, 12, 14, 10, 15}, {20, 16, 19, 14, 21}, SIZE_MEDIUM},
    {"giant",  "Giant", 6, {200, 150, 150, 100}, {"bash", "fast healing"}, {16, 11, 13, 11, 14}, {22, 15, 18, 15, 20}, SIZE_LARGE},
#ifndef VANILLA
    {"pixie",  "Pixie", 7, {100, 150, 150, 150}, {"dodge"},                {11, 15, 15, 14, 10}, {15, 21, 20, 20, 14}, SIZE_TINY},
#endif
    {0}
};

/* Class table.  */
const CLASS_TYPE class_table[CLASS_MAX + 1] = {
    {CLASS_MAGE,    "mage",    "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER, {3018, 9618}, 75, 20,   6,  6,  8, TRUE,  "mage basics",    "mage default"},
    {CLASS_CLERIC,  "cleric",  "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,   {3003, 9619}, 75, 20,   2,  7, 10, TRUE,  "cleric basics",  "cleric default"},
    {CLASS_THIEF,   "thief",   "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER, {3028, 9639}, 75, 20,  -4,  8, 13, FALSE, "thief basics",   "thief default"},
    {CLASS_WARRIOR, "warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,  {3022, 9633}, 75, 20, -10, 11, 15, FALSE, "warrior basics", "warrior default"},
    {0}
};

/* Titles.  */
char *const title_table[CLASS_MAX][MAX_LEVEL + 1][2] = {
    {
     {"Man",                      "Woman"},

     {"Apprentice of Magic",      "Apprentice of Magic"},
     {"Spell Student",            "Spell Student"},
     {"Scholar of Magic",         "Scholar of Magic"},
     {"Delver in Spells",         "Delveress in Spells"},
     {"Medium of Magic",          "Medium of Magic"},

     {"Scribe of Magic",          "Scribess of Magic"},
     {"Seer",                     "Seeress"},
     {"Sage",                     "Sage"},
     {"Illusionist",              "Illusionist"},
     {"Abjurer",                  "Abjuress"},

     {"Invoker",                  "Invoker"},
     {"Enchanter",                "Enchantress"},
     {"Conjurer",                 "Conjuress"},
     {"Magician",                 "Witch"},
     {"Creator",                  "Creator"},

     {"Savant",                   "Savant"},
     {"Magus",                    "Craftess"},
     {"Wizard",                   "Wizard"},
     {"Warlock",                  "War Witch"},
     {"Sorcerer",                 "Sorceress"},

     {"Elder Sorcerer",           "Elder Sorceress"},
     {"Grand Sorcerer",           "Grand Sorceress"},
     {"Great Sorcerer",           "Great Sorceress"},
     {"Golem Maker",              "Golem Maker"},
     {"Greater Golem Maker",      "Greater Golem Maker"},

     {"Maker of Stones",          "Maker of Stones",},
     {"Maker of Potions",         "Maker of Potions",},
     {"Maker of Scrolls",         "Maker of Scrolls",},
     {"Maker of Wands",           "Maker of Wands",},
     {"Maker of Staves",          "Maker of Staves",},

     {"Demon Summoner",           "Demon Summoner"},
     {"Greater Demon Summoner",   "Greater Demon Summoner"},
     {"Dragon Charmer",           "Dragon Charmer"},
     {"Greater Dragon Charmer",   "Greater Dragon Charmer"},
     {"Master of all Magic",      "Master of all Magic"},

     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},

     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},

     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},
     {"Master Mage",              "Master Mage"},

     {"Mage Hero",                "Mage Heroine"},
     {"Avatar of Magic",          "Avatar of Magic"},
     {"Angel of Magic",           "Angel of Magic"},
     {"Demigod of Magic",         "Demigoddess of Magic"},
     {"Immortal of Magic",        "Immortal of Magic"},
     {"God of Magic",             "Goddess of Magic"},
     {"Deity of Magic",           "Deity of Magic"},
     {"Supremity of Magic",       "Supremity of Magic"},
     {"Creator",                  "Creator"},
     {"Implementor",              "Implementress"}
     },

    {
     {"Man",                      "Woman"},

     {"Believer",                 "Believer"},
     {"Attendant",                "Attendant"},
     {"Acolyte",                  "Acolyte"},
     {"Novice",                   "Novice"},
     {"Missionary",               "Missionary"},

     {"Adept",                    "Adept"},
     {"Deacon",                   "Deaconess"},
     {"Vicar",                    "Vicaress"},
     {"Priest",                   "Priestess"},
     {"Minister",                 "Lady Minister"},

     {"Canon",                    "Canon"},
     {"Levite",                   "Levitess"},
     {"Curate",                   "Curess"},
     {"Monk",                     "Nun"},
     {"Healer",                   "Healess"},

     {"Chaplain",                 "Chaplain"},
     {"Expositor",                "Expositress"},
     {"Bishop",                   "Bishop"},
     {"Arch Bishop",              "Arch Lady of the Church"},
     {"Patriarch",                "Matriarch"},

     {"Elder Patriarch",          "Elder Matriarch"},
     {"Grand Patriarch",          "Grand Matriarch"},
     {"Great Patriarch",          "Great Matriarch"},
     {"Demon Killer",             "Demon Killer"},
     {"Greater Demon Killer",     "Greater Demon Killer"},

     {"Cardinal of the Sea",      "Cardinal of the Sea"},
     {"Cardinal of the Earth",    "Cardinal of the Earth"},
     {"Cardinal of the Air",      "Cardinal of the Air"},
     {"Cardinal of the Ether",    "Cardinal of the Ether"},
     {"Cardinal of the Heavens",  "Cardinal of the Heavens"},

     {"Avatar of an Immortal",    "Avatar of an Immortal"},
     {"Avatar of a Deity",        "Avatar of a Deity"},
     {"Avatar of a Supremity",    "Avatar of a Supremity"},
     {"Avatar of an Implementor", "Avatar of an Implementor"},
     {"Master of all Divinity",   "Mistress of all Divinity"},

     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},

     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},

     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},
     {"Master Cleric",            "Master Cleric"},

     {"Holy Hero",                "Holy Heroine"},
     {"Holy Avatar",              "Holy Avatar"},
     {"Angel",                    "Angel"},
     {"Demigod",                  "Demigoddess",},
     {"Immortal",                 "Immortal"},
     {"God",                      "Goddess"},
     {"Deity",                    "Deity"},
     {"Supreme Master",           "Supreme Mistress"},
     {"Creator",                  "Creator"},
     {"Implementor",              "Implementress"}
     },

    {
     {"Man",                      "Woman"},

     {"Pilferer",                 "Pilferess"},
     {"Footpad",                  "Footpad"},
     {"Filcher",                  "Filcheress"},
     {"Pick-Pocket",              "Pick-Pocket"},
     {"Sneak",                    "Sneak"},

     {"Pincher",                  "Pincheress"},
     {"Cut-Purse",                "Cut-Purse"},
     {"Snatcher",                 "Snatcheress"},
     {"Sharper",                  "Sharpress"},
     {"Rogue",                    "Rogue"},

     {"Robber",                   "Robber"},
     {"Magsman",                  "Magswoman"},
     {"Highwayman",               "Highwaywoman"},
     {"Burglar",                  "Burglaress"},
     {"Thief",                    "Thief"},

     {"Knifer",                   "Knifer"},
     {"Quick-Blade",              "Quick-Blade"},
     {"Killer",                   "Murderess"},
     {"Brigand",                  "Brigand"},
     {"Cut-Throat",               "Cut-Throat"},

     {"Spy",                      "Spy"},
     {"Grand Spy",                "Grand Spy"},
     {"Master Spy",               "Master Spy"},
     {"Assassin",                 "Assassin"},
     {"Greater Assassin",         "Greater Assassin"},

     {"Master of Vision",         "Mistress of Vision"},
     {"Master of Hearing",        "Mistress of Hearing"},
     {"Master of Smell",          "Mistress of Smell"},
     {"Master of Taste",          "Mistress of Taste"},
     {"Master of Touch",          "Mistress of Touch"},

     {"Crime Lord",               "Crime Mistress"},
     {"Infamous Crime Lord",      "Infamous Crime Mistress"},
     {"Greater Crime Lord",       "Greater Crime Mistress"},
     {"Master Crime Lord",        "Master Crime Mistress"},
     {"Godfather",                "Godmother"},

     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},

     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},

     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},
     {"Master Thief",             "Master Thief"},

     {"Assassin Hero",            "Assassin Heroine"},
     {"Avatar of Death",          "Avatar of Death",},
     {"Angel of Death",           "Angel of Death"},
     {"Demigod of Assassins",     "Demigoddess of Assassins"},
     {"Immortal Assasin",         "Immortal Assassin"},
     {"God of Assassins",         "God of Assassins",},
     {"Deity of Assassins",       "Deity of Assassins"},
     {"Supreme Master",           "Supreme Mistress"},
     {"Creator",                  "Creator"},
     {"Implementor",              "Implementress"}
     },

    {
     {"Man",                      "Woman"},

     {"Swordpupil",               "Swordpupil"},
     {"Recruit",                  "Recruit"},
     {"Sentry",                   "Sentress"},
     {"Fighter",                  "Fighter"},
     {"Soldier",                  "Soldier"},

     {"Warrior",                  "Warrior"},
     {"Veteran",                  "Veteran"},
     {"Swordsman",                "Swordswoman"},
     {"Fencer",                   "Fenceress"},
     {"Combatant",                "Combatess"},

     {"Hero",                     "Heroine"},
     {"Myrmidon",                 "Myrmidon"},
     {"Swashbuckler",             "Swashbuckleress"},
     {"Mercenary",                "Mercenaress"},
     {"Swordmaster",              "Swordmistress"},

     {"Lieutenant",               "Lieutenant"},
     {"Champion",                 "Lady Champion"},
     {"Dragoon",                  "Lady Dragoon"},
     {"Cavalier",                 "Lady Cavalier"},
     {"Knight",                   "Lady Knight"},

     {"Grand Knight",             "Grand Knight"},
     {"Master Knight",            "Master Knight"},
     {"Paladin",                  "Paladin"},
     {"Grand Paladin",            "Grand Paladin"},
     {"Demon Slayer",             "Demon Slayer"},

     {"Greater Demon Slayer",     "Greater Demon Slayer"},
     {"Dragon Slayer",            "Dragon Slayer"},
     {"Greater Dragon Slayer",    "Greater Dragon Slayer"},
     {"Underlord",                "Underlord"},
     {"Overlord",                 "Overlord"},

     {"Baron of Thunder",         "Baroness of Thunder"},
     {"Baron of Storms",          "Baroness of Storms"},
     {"Baron of Tornadoes",       "Baroness of Tornadoes"},
     {"Baron of Hurricanes",      "Baroness of Hurricanes"},
     {"Baron of Meteors",         "Baroness of Meteors"},

     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},

     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},

     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},
     {"Master Warrior",           "Master Warrior"},

     {"Knight Hero",              "Knight Heroine"},
     {"Avatar of War",            "Avatar of War"},
     {"Angel of War",             "Angel of War"},
     {"Demigod of War",           "Demigoddess of War"},
     {"Immortal Warlord",         "Immortal Warlord"},
     {"God of War",               "God of War"},
     {"Deity of War",             "Deity of War"},
     {"Supreme Master of War",    "Supreme Mistress of War"},
     {"Creator",                  "Creator"},
     {"Implementor",              "Implementress"}
    }
};

/* Attribute bonus tables. */
const STR_APP_TYPE str_app[ATTRIBUTE_MAX + 1] = {
    /* stat, tohit, todam, carry, wield */
    { 0, -5, -4,   0,  0},
    { 1, -4, -3,   3,  1},
    { 2, -3, -2,   6,  2},
    { 3, -3, -1,  10,  3},
    { 4, -2, -1,  25,  4},
    { 5, -2, -1,  55,  5},
    { 6, -1,  0,  80,  6},
    { 7, -1,  0,  90,  7},
    { 8,  0,  0, 100,  8},
    { 9,  0,  0, 107,  9},
    {10,  0,  0, 115, 10},
    {11,  0,  0, 123, 11},
    {12,  0,  0, 130, 12},
    {13,  0,  0, 137, 13},
    {14,  0,  1, 143, 14},
    {15,  1,  1, 150, 15},
    {16,  1,  2, 165, 16},
    {17,  2,  3, 180, 22},
    {18,  2,  3, 200, 25},
    {19,  3,  4, 225, 30},
    {20,  3,  5, 250, 35},
    {21,  4,  6, 300, 40},
    {22,  4,  6, 350, 45},
    {23,  5,  7, 400, 50},
    {24,  5,  8, 450, 55},
    {25,  6,  9, 500, 60},

    { -999},
};

const INT_APP_TYPE int_app[ATTRIBUTE_MAX + 1] = {
    /* stat, learn */
    { 0,  3},
    { 1,  5},
    { 2,  7},
    { 3,  8},
    { 4,  9},
    { 5, 10},
    { 6, 11},
    { 7, 12},
    { 8, 13},
    { 9, 15},
    {10, 17},
    {11, 19},
    {12, 22},
    {13, 25},
    {14, 28},
    {15, 31},
    {16, 34},
    {17, 37},
    {18, 40},
    {19, 44},
    {20, 49},
    {21, 55},
    {22, 60},
    {23, 70},
    {24, 80},
    {25, 85},

    {-999}
};

const WIS_APP_TYPE wis_app[ATTRIBUTE_MAX + 1] = {
    /* stat, practice */
    { 0, 0},
    { 1, 0},
    { 2, 0},
    { 3, 0},
    { 4, 0},
    { 5, 1},
    { 6, 1},
    { 7, 1},
    { 8, 1},
    { 9, 1},
    {10, 1},
    {11, 1},
    {12, 1},
    {13, 1},
    {14, 1},
    {15, 2},
    {16, 2},
    {17, 2},
    {18, 3},
    {19, 3},
    {20, 3},
    {21, 3},
    {22, 4},
    {23, 4},
    {24, 4},
    {25, 5},

    {-999}
};

const DEX_APP_TYPE dex_app[ATTRIBUTE_MAX + 1] = {
    /* stat, defensive */
    { 0,  60},
    { 1,  50},
    { 2,  50},
    { 3,  40},
    { 4,  30},
    { 5,  20},
    { 6,  10},
    { 7,   0},
    { 8,   0},
    { 9,   0},
    {10,   0},
    {11,   0},
    {12,   0},
    {13,   0},
    {14,   0},
    {15,  -10},
    {16,  -15},
    {17,  -20},
    {18,  -30},
    {19,  -40},
    {20,  -50},
    {21,  -60},
    {22,  -75},
    {23,  -90},
    {24, -105},
    {25, -120},

    {-999}
};

const CON_APP_TYPE con_app[ATTRIBUTE_MAX + 1] = {
    /* stat, hitp, shock */
    { 0, -4, 20},
    { 1, -3, 25},
    { 2, -2, 30},
    { 3, -2, 35},
    { 4, -1, 40},
    { 5, -1, 45},
    { 6, -1, 50},
    { 7,  0, 55},
    { 8,  0, 60},
    { 9,  0, 65},
    {10,  0, 70},
    {11,  0, 75},
    {12,  0, 80},
    {13,  0, 85},
    {14,  0, 88},
    {15,  1, 90},
    {16,  2, 95},
    {17,  2, 97},
    {18,  3, 99},
    {19,  3, 99},
    {20,  4, 99},
    {21,  4, 99},
    {22,  5, 99},
    {23,  6, 99},
    {24,  7, 99},
    {25,  8, 99},

    { -999},
};

/* Liquid properties. */
const LIQ_TYPE liq_table[LIQ_MAX + 1] = {
  /* name                   color         proof, full, thirst, food, ssize */
    {"water",               "clear",     {0,     1,    10,     0,    16}},
    {"beer",                "amber",     {12,    1,    8,      1,    12}},
    {"red wine",            "burgundy",  {30,    1,    8,      1,    5 }},
    {"ale",                 "brown",     {15,    1,    8,      1,    12}},
    {"dark ale",            "dark",      {16,    1,    8,      1,    12}},
    {"whisky",              "golden",    {120,   1,    5,      0,    2 }},
    {"lemonade",            "pink",      {0,     1,    9,      2,    12}},
    {"firebreather",        "boiling",   {190,   0,    4,      0,    2 }},
    {"local specialty",     "clear",     {151,   1,    3,      0,    2 }},
    {"slime mold juice",    "green",     {0,     2,    -8,     1,    2 }},
    {"milk",                "white",     {0,     2,    9,      3,    12}},
    {"tea",                 "tan",       {0,     1,    8,      0,    6 }},
    {"coffee",              "black",     {0,     1,    8,      0,    6 }},
    {"blood",               "red",       {0,     2,    -1,     2,    6 }},
    {"salt water",          "clear",     {0,     1,    -2,     0,    1 }},
    {"coke",                "brown",     {0,     2,    9,      2,    12}},
    {"root beer",           "brown",     {0,     2,    9,      2,    12}},
    {"elvish wine",         "green",     {35,    2,    8,      1,    5 }},
    {"white wine",          "golden",    {28,    1,    8,      1,    5 }},
    {"champagne",           "golden",    {32,    1,    8,      1,    5 }},
    {"mead",            "honey-colored", {34,    2,    8,      2,    12}},
    {"rose wine",           "pink",      {26,    1,    8,      1,    5 }},
    {"benedictine wine",    "burgundy",  {40,    1,    8,      1,    5 }},
    {"vodka",               "clear",     {130,   1,    5,      0,    2 }},
    {"cranberry juice",     "red",       {0,     1,    9,      2,    12}},
    {"orange juice",        "orange",    {0,     2,    9,      3,    12}},
    {"absinthe",            "green",     {200,   1,    4,      0,    2 }},
    {"brandy",              "golden",    {80,    1,    5,      0,    4 }},
    {"aquavit",             "clear",     {140,   1,    5,      0,    2 }},
    {"schnapps",            "clear",     {90,    1,    5,      0,    2 }},
    {"icewine",             "purple",    {50,    2,    6,      1,    5 }},
    {"amontillado",         "burgundy",  {35,    2,    8,      1,    5 }},
    {"sherry",              "red",       {38,    2,    7,      1,    5 }},
    {"framboise",           "red",       {50,    1,    7,      1,    5 }},
    {"rum",                 "amber",     {151,   1,    4,      0,    2 }},
    {"cordial",             "clear",     {100,   1,    5,      0,    2 }},
    {0}
};

/* The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections. */
#define SLOT(n)    n

#define TI   TAR_IGNORE
#define TCO  TAR_CHAR_OFFENSIVE
#define TCD  TAR_CHAR_DEFENSIVE
#define TCS  TAR_CHAR_SELF
#define TOI  TAR_OBJ_INV
#define TOCD TAR_OBJ_CHAR_DEF
#define TOCO TAR_OBJ_CHAR_OFF

#define PS POS_STANDING
#define PF POS_FIGHTING
#define PR POS_RESTING
#define PP POS_SLEEPING

const SKILL_TYPE skill_table[SKILL_MAX + 1] = {
    /* Magic spells. */
    {"reserved",        {99, 99, 99, 99}, {99, 99, 99, 99}, NULL,                  TI,   PS, NULL,                 SLOT (0),   0,   0,  "",               "", ""},
    {"acid blast",      {28, 53, 35, 32}, {1, 1, 2, 2},     spell_acid_blast,      TCO,  PF, NULL,                 SLOT (70),  20,  12, "acid blast",     "!Acid Blast!", ""},
    {"armor",           {7, 2, 10, 5},    {1, 1, 2, 2},     spell_armor,           TCD,  PS, NULL,                 SLOT (1),   5,   12, "",               "You feel less armored.", ""},
    {"bless",           {53, 7, 53, 8},   {1, 1, 2, 2},     spell_bless,           TOCD, PS, NULL,                 SLOT (3),   5,   12, "",               "You feel less righteous.", "$p's holy aura fades."},
    {"blindness",       {12, 8, 17, 15},  {1, 1, 2, 2},     spell_blindness,       TCO,  PF, &gsn_blindness,       SLOT (4),   5,   12, "",               "You can see again.", ""},
    {"burning hands",   {7, 53, 10, 9},   {1, 1, 2, 2},     spell_burning_hands,   TCO,  PF, NULL,                 SLOT (5),   15,  12, "burning hands",  "!Burning Hands!", ""},
    {"call lightning",  {26, 18, 31, 22}, {1, 1, 2, 2},     spell_call_lightning,  TI,   PF, NULL,                 SLOT (6),   15,  12, "lightning bolt", "!Call Lightning!", ""},
    {"calm",            {48, 16, 50, 20}, {1, 1, 2, 2},     spell_calm,            TI,   PF, NULL,                 SLOT (509), 30,  12, "",               "You have lost your peace of mind.", ""},
    {"cancellation",    {18, 26, 34, 34}, {1, 1, 2, 2},     spell_cancellation,    TCD,  PF, NULL,                 SLOT (507), 20,  12, "",               "!cancellation!",""},
    {"cause critical",  {53, 13, 53, 19}, {1, 1, 2, 2},     spell_cause_critical,  TCO,  PF, NULL,                 SLOT (63),  20,  12, "harmful spell",  "!Cause Critical!", ""},
    {"cause light",     {53, 1, 53, 3},   {1, 1, 2, 2},     spell_cause_light,     TCO,  PF, NULL,                 SLOT (62),  15,  12, "harmful spell",  "!Cause Light!", ""},
    {"cause serious",   {53, 7, 53, 10},  {1, 1, 2, 2},     spell_cause_serious,   TCO,  PF, NULL,                 SLOT (64),  17,  12, "harmful spell",  "!Cause Serious!", ""},
    {"chain lightning", {33, 53, 39, 36}, {1, 1, 2, 2},     spell_chain_lightning, TCO,  PF, NULL,                 SLOT (500), 25,  12, "lightning",      "!Chain Lightning!", ""},
    {"change sex",      {53, 53, 53, 53}, {1, 1, 2, 2},     spell_change_sex,      TCD,  PF, NULL,                 SLOT (82),  15,  12, "",               "Your body feels familiar again.", ""},
    {"charm person",    {20, 53, 25, 53}, {1, 1, 2, 2},     spell_charm_person,    TCO,  PS, &gsn_charm_person,    SLOT (7),   5,   12, "",               "You feel more self-confident.", ""},
    {"chill touch",     {4, 53, 6, 6},    {1, 1, 2, 2},     spell_chill_touch,     TCO,  PF, NULL,                 SLOT (8),   15,  12, "chilling touch", "You feel less cold.", ""},
    {"colour spray",    {16, 53, 22, 20}, {1, 1, 2, 2},     spell_colour_spray,    TCO,  PF, NULL,                 SLOT (10),  15,  12, "colour spray",   "!Colour Spray!", ""},
    {"continual light", {6, 4, 6, 9},     {1, 1, 2, 2},     spell_continual_light, TI,   PS, NULL,                 SLOT (57),  7,   12, "",               "!Continual Light!", ""},
    {"control weather", {15, 19, 28, 22}, {1, 1, 2, 2},     spell_control_weather, TI,   PS, NULL,                 SLOT (11),  25,  12, "",               "!Control Weather!", ""},
    {"create food",     {10, 5, 11, 12},  {1, 1, 2, 2},     spell_create_food,     TI,   PS, NULL,                 SLOT (12),  5,   12, "",               "!Create Food!", ""},
    {"create rose",     {16, 11, 10, 24}, {1, 1, 2, 2},     spell_create_rose,     TI,   PS, NULL,                 SLOT (511), 30,  12, "",               "!Create Rose!", ""},
    {"create spring",   {14, 17, 23, 20}, {1, 1, 2, 2},     spell_create_spring,   TI,   PS, NULL,                 SLOT (80),  20,  12, "",               "!Create Spring!", ""},
    {"create water",    {8, 3, 12, 11},   {1, 1, 2, 2},     spell_create_water,    TOI,  PS, NULL,                 SLOT (13),  5,   12, "",               "!Create Water!", ""},
    {"cure blindness",  {53, 6, 53, 8},   {1, 1, 2, 2},     spell_cure_blindness,  TCD,  PF, NULL,                 SLOT (14),  5,   12, "",               "!Cure Blindness!", ""},
    {"cure critical",   {53, 13, 53, 19}, {1, 1, 2, 2},     spell_cure_critical,   TCD,  PF, NULL,                 SLOT (15),  20,  12, "",               "!Cure Critical!", ""},
    {"cure disease",    {53, 13, 53, 14}, {1, 1, 2, 2},     spell_cure_disease,    TCD,  PS, NULL,                 SLOT (501), 20,  12, "",               "!Cure Disease!", ""},
    {"cure light",      {53, 1, 53, 3},   {1, 1, 2, 2},     spell_cure_light,      TCD,  PF, NULL,                 SLOT (16),  10,  12, "",               "!Cure Light!", ""},
    {"cure poison",     {53, 14, 53, 16}, {1, 1, 2, 2},     spell_cure_poison,     TCD,  PS, NULL,                 SLOT (43),  5,   12, "",               "!Cure Poison!", ""},
    {"cure serious",    {53, 7, 53, 10},  {1, 1, 2, 2},     spell_cure_serious,    TCD,  PF, NULL,                 SLOT (61),  15,  12, "",               "!Cure Serious!", ""},
    {"curse",           {18, 18, 26, 22}, {1, 1, 2, 2},     spell_curse,           TOCO, PF, &gsn_curse,           SLOT (17),  20,  12, "curse",          "The curse wears off.", "$p is no longer impure."},
    {"demonfire",       {53, 34, 53, 45}, {1, 1, 2, 2},     spell_demonfire,       TCO,  PF, NULL,                 SLOT (505), 20,  12, "torments",       "!Demonfire!", ""},
    {"detect evil",     {11, 4, 12, 53},  {1, 1, 2, 2},     spell_detect_evil,     TCS,  PS, NULL,                 SLOT (18),  5,   12, "",               "The red in your vision disappears.", ""},
    {"detect good",     {11, 4, 12, 53},  {1, 1, 2, 2},     spell_detect_good,     TCS,  PS, NULL,                 SLOT (513), 5,   12, "",               "The gold in your vision disappears.", ""},
    {"detect hidden",   {15, 11, 12, 53}, {1, 1, 2, 2},     spell_detect_hidden,   TCS,  PS, NULL,                 SLOT (44),  5,   12, "",               "You feel less aware of your surroundings.", ""},
    {"detect invis",    {3, 8, 6, 53},    {1, 1, 2, 2},     spell_detect_invis,    TCS,  PS, NULL,                 SLOT (19),  5,   12, "",               "You no longer see invisible objects.", ""},
    {"detect magic",    {2, 6, 5, 53},    {1, 1, 2, 2},     spell_detect_magic,    TCS,  PS, NULL,                 SLOT (20),  5,   12, "",               "The detect magic wears off.", ""},
    {"detect poison",   {15, 7, 9, 53},   {1, 1, 2, 2},     spell_detect_poison,   TOI,  PS, NULL,                 SLOT (21),  5,   12, "",               "!Detect Poison!", ""},
    {"dispel evil",     {53, 15, 53, 21}, {1, 1, 2, 2},     spell_dispel_evil,     TCO,  PF, NULL,                 SLOT (22),  15,  12, "dispel evil",    "!Dispel Evil!", ""},
    {"dispel good",     {53, 15, 53, 21}, {1, 1, 2, 2},     spell_dispel_good,     TCO,  PF, NULL,                 SLOT (512), 15,  12, "dispel good",    "!Dispel Good!", ""},
    {"dispel magic",    {16, 24, 30, 30}, {1, 1, 2, 2},     spell_dispel_magic,    TCO,  PF, NULL,                 SLOT (59),  15,  12, "",               "!Dispel Magic!", ""},
    {"earthquake",      {53, 10, 53, 14}, {1, 1, 2, 2},     spell_earthquake,      TI,   PF, NULL,                 SLOT (23),  15,  12, "earthquake",     "!Earthquake!", ""},
    {"enchant armor",   {16, 53, 53, 53}, {2, 2, 4, 4},     spell_enchant_armor,   TOI,  PS, NULL,                 SLOT (510), 100, 24, "",               "!Enchant Armor!", ""},
    {"enchant weapon",  {17, 53, 53, 53}, {2, 2, 4, 4},     spell_enchant_weapon,  TOI,  PS, NULL,                 SLOT (24),  100, 24, "",               "!Enchant Weapon!", ""},
    {"energy drain",    {19, 22, 26, 23}, {1, 1, 2, 2},     spell_energy_drain,    TCO,  PF, NULL,                 SLOT (25),  35,  12, "energy drain",   "!Energy Drain!", ""},
    {"faerie fire",     {6, 3, 5, 8},     {1, 1, 2, 2},     spell_faerie_fire,     TCO,  PF, NULL,                 SLOT (72),  5,   12, "faerie fire",    "The pink aura around you fades away.", ""},
    {"faerie fog",      {14, 21, 16, 24}, {1, 1, 2, 2},     spell_faerie_fog,      TI,   PS, NULL,                 SLOT (73),  12,  12, "faerie fog",     "!Faerie Fog!", ""},
    {"farsight",        {14, 16, 16, 53}, {1, 1, 2, 2},     spell_farsight,        TI,   PS, NULL,                 SLOT (521), 36,  20, "farsight",       "!Farsight!", ""},
    {"fireball",        {22, 53, 30, 26}, {1, 1, 2, 2},     spell_fireball,        TCO,  PF, NULL,                 SLOT (26),  15,  12, "fireball",       "!Fireball!", ""},
    {"fireproof",       {13, 12, 19, 18}, {1, 1, 2, 2},     spell_fireproof,       TOI,  PS, NULL,                 SLOT (523), 10,  12, "",               "", "$p's protective aura fades."},
    {"flamestrike",     {53, 20, 53, 27}, {1, 1, 2, 2},     spell_flamestrike,     TCO,  PF, NULL,                 SLOT (65),  20,  12, "flamestrike",    "!Flamestrike!", ""},
    {"fly",             {10, 18, 20, 22}, {1, 1, 2, 2},     spell_fly,             TCD,  PS, NULL,                 SLOT (56),  10,  18, "",               "You slowly float to the ground.", ""},
    {"floating disc",   {4, 10, 7, 16},   {1, 1, 2, 2},     spell_floating_disc,   TI,   PS, NULL,                 SLOT (522), 40,  24, "",               "!Floating disc!", ""},
    {"frenzy",          {53, 24, 53, 26}, {1, 1, 2, 2},     spell_frenzy,          TCD,  PS, &gsn_frenzy,          SLOT (504), 30,  24, "",               "Your rage ebbs.", ""},
    {"gate",            {27, 17, 32, 28}, {1, 1, 2, 2},     spell_gate,            TI,   PF, NULL,                 SLOT (83),  80,  12, "",               "!Gate!", ""},
    {"giant strength",  {11, 53, 22, 20}, {1, 1, 2, 2},     spell_giant_strength,  TCD,  PS, NULL,                 SLOT (39),  20,  12, "",               "You feel weaker.", ""},
    {"harm",            {53, 23, 53, 28}, {1, 1, 2, 2},     spell_harm,            TCO,  PF, NULL,                 SLOT (27),  35,  12, "harmful spell",  "!Harm!,        " ""},
    {"haste",           {21, 53, 26, 29}, {1, 1, 2, 2},     spell_haste,           TCD,  PF, NULL,                 SLOT (502), 30,  12, "",               "You feel yourself slow down.", ""},
    {"heal",            {53, 21, 33, 30}, {1, 1, 2, 2},     spell_heal,            TCD,  PF, NULL,                 SLOT (28),  50,  12, "",               "!Heal!", ""},
    {"heat metal",      {53, 16, 53, 23}, {1, 1, 2, 2},     spell_heat_metal,      TCO,  PF, NULL,                 SLOT (516), 25,  18, "burn",           "!Heat Metal!", ""},
    {"holy word",       {53, 36, 53, 42}, {2, 2, 4, 4},     spell_holy_word,       TI,   PF, NULL,                 SLOT (506), 200, 24, "divine wrath",   "!Holy Word!", ""},
    {"identify",        {15, 16, 18, 53}, {1, 1, 2, 2},     spell_identify,        TOI,  PS, NULL,                 SLOT (53),  12,  24, "",               "!Identify!", ""},
    {"infravision",     {9, 13, 10, 16},  {1, 1, 2, 2},     spell_infravision,     TCD,  PS, NULL,                 SLOT (77),  5,   18, "",               "You no longer see in the dark.", ""},
    {"invisibility",    {5, 53, 9, 53},   {1, 1, 2, 2},     spell_invis,           TOCD, PS, &gsn_invis,           SLOT (29),  5,   12, "",               "You are no longer invisible.", "$p fades into view."},
    {"know alignment",  {12, 9, 20, 53},  {1, 1, 2, 2},     spell_know_alignment,  TCD,  PF, NULL,                 SLOT (58),  9,   12, "",               "!Know Alignment!", ""},
    {"lightning bolt",  {13, 23, 18, 16}, {1, 1, 2, 2},     spell_lightning_bolt,  TCO,  PF, NULL,                 SLOT (30),  15,  12, "lightning bolt", "!Lightning Bolt!", ""},
    {"locate object",   {9, 15, 11, 53},  {1, 1, 2, 2},     spell_locate_object,   TI,   PS, NULL,                 SLOT (31),  20,  18, "",               "!Locate Object!", ""},
    {"magic missile",   {1, 53, 2, 2},    {1, 1, 2, 2},     spell_magic_missile,   TCO,  PF, NULL,                 SLOT (32),  15,  12, "magic missile",  "!Magic Missile!", ""},
    {"mass healing",    {53, 38, 53, 46}, {2, 2, 4, 4},     spell_mass_healing,    TI,   PS, NULL,                 SLOT (508), 100, 36, "",               "!Mass Healing!", ""},
    {"mass invis",      {22, 25, 31, 53}, {1, 1, 2, 2},     spell_mass_invis,      TI,   PS, &gsn_mass_invis,      SLOT (69),  20,  24, "",               "You are no longer invisible.", ""},
    {"nexus",           {40, 35, 50, 45}, {2, 2, 4, 4},     spell_nexus,           TI,   PS, NULL,                 SLOT (520), 150, 36, "",               "!Nexus!", ""},
    {"pass door",       {24, 32, 25, 37}, {1, 1, 2, 2},     spell_pass_door,       TCS,  PS, NULL,                 SLOT (74),  20,  12, "",               "You feel solid again.", ""},
    {"plague",          {23, 17, 36, 26}, {1, 1, 2, 2},     spell_plague,          TCO,  PF, &gsn_plague,          SLOT (503), 20,  12, "sickness",       "Your sores vanish.", ""},
    {"poison",          {17, 12, 15, 21}, {1, 1, 2, 2},     spell_poison,          TOCO, PF, &gsn_poison,          SLOT (33),  10,  12, "poison",         "You feel less sick.", "The poison on $p dries up."},
    {"portal",          {35, 30, 45, 40}, {2, 2, 4, 4},     spell_portal,          TI,   PS, NULL,                 SLOT (519), 100, 24, "",               "!Portal!", ""},
    {"protection evil", {12, 9, 17, 11},  {1, 1, 2, 2},     spell_protection_evil, TCS,  PS, NULL,                 SLOT (34),  5,   12, "",               "You feel less protected.", ""},
    {"protection good", {12, 9, 17, 11},  {1, 1, 2, 2},     spell_protection_good, TCS,  PS, NULL,                 SLOT (514), 5,   12, "",               "You feel less protected.", ""},
    {"ray of truth",    {53, 35, 53, 47}, {1, 1, 2, 2},     spell_ray_of_truth,    TCO,  PF, NULL,                 SLOT (518), 20,  12, "ray of truth",   "!Ray of Truth!", ""},
    {"recharge",        {9, 53, 53, 53},  {1, 1, 2, 2},     spell_recharge,        TOI,  PS, NULL,                 SLOT (517), 60,  24, "",               "!Recharge!", ""},
    {"refresh",         {8, 5, 12, 9},    {1, 1, 2, 2},     spell_refresh,         TCD,  PS, NULL,                 SLOT (81),  12,  18, "refresh",        "!Refresh!", ""},
    {"remove curse",    {53, 18, 53, 22}, {1, 1, 2, 2},     spell_remove_curse,    TOCD, PS, NULL,                 SLOT (35),  5,   12, "",               "!Remove Curse!", ""},
    {"sanctuary",       {36, 20, 42, 30}, {1, 1, 2, 2},     spell_sanctuary,       TCD,  PS, &gsn_sanctuary,       SLOT (36),  75,  12, "",               "The white aura around your body fades.", ""},
    {"shield",          {20, 35, 35, 40}, {1, 1, 2, 2},     spell_shield,          TCD,  PS, NULL,                 SLOT (67),  12,  18, "",               "Your force shield shimmers then fades away.", ""},
    {"shocking grasp",  {10, 53, 14, 13}, {1, 1, 2, 2},     spell_shocking_grasp,  TCO,  PF, NULL,                 SLOT (53),  15,  12, "shocking grasp", "!Shocking Grasp!", ""},
    {"sleep",           {10, 53, 11, 53}, {1, 1, 2, 2},     spell_sleep,           TCO,  PS, &gsn_sleep,           SLOT (38),  15,  12, "",               "You feel less tired.", ""},
    {"slow",            {23, 30, 29, 32}, {1, 1, 2, 2},     spell_slow,            TCO,  PF, NULL,                 SLOT (515), 30,  12, "",               "You feel yourself speed up.", ""},
    {"stone skin",      {25, 40, 40, 45}, {1, 1, 2, 2},     spell_stone_skin,      TCS,  PS, NULL,                 SLOT (66),  12,  18, "",               "Your skin feels soft again.", ""},
    {"summon",          {24, 12, 29, 22}, {1, 1, 2, 2},     spell_summon,          TI,   PS, NULL,                 SLOT (40),  50,  12, "",               "!Summon!", ""},
    {"teleport",        {13, 22, 25, 36}, {1, 1, 2, 2},     spell_teleport,        TCS,  PF, NULL,                 SLOT (2),   35,  12, "",               "!Teleport!", ""},
    {"ventriloquate",   {1, 53, 2, 53},   {1, 1, 2, 2},     spell_ventriloquate,   TI,   PS, NULL,                 SLOT (41),  5,   12, "",               "!Ventriloquate!", ""},
    {"weaken",          {11, 14, 16, 17}, {1, 1, 2, 2},     spell_weaken,          TCO,  PF, NULL,                 SLOT (68),  20,  12, "spell",          "You feel stronger.", ""},
    {"word of recall",  {32, 28, 40, 30}, {1, 1, 2, 2},     spell_word_of_recall,  TCS,  PR, NULL,                 SLOT (42),  5,   12, "",               "!Word of Recall!", ""},

    /* Dragon breath */
    {"acid breath",     {31, 32, 33, 34}, {1, 1, 2, 2},     spell_acid_breath,     TCO,  PF, NULL,                 SLOT (200), 100, 24, "blast of acid",  "!Acid Breath!", ""},
    {"fire breath",     {40, 45, 50, 51}, {1, 1, 2, 2},     spell_fire_breath,     TCO,  PF, NULL,                 SLOT (201), 200, 24, "blast of flame", "The smoke leaves your eyes.", ""},
    {"frost breath",    {34, 36, 38, 40}, {1, 1, 2, 2},     spell_frost_breath,    TCO,  PF, NULL,                 SLOT (202), 125, 24, "blast of frost", "!Frost Breath!", ""},
    {"gas breath",      {39, 43, 47, 50}, {1, 1, 2, 2},     spell_gas_breath,      TI,   PF, NULL,                 SLOT (203), 175, 24, "blast of gas",   "!Gas Breath!", ""},
    {"lightning breath",{37, 40, 43, 46}, {1, 1, 2, 2},     spell_lightning_breath,TCO,  PF, NULL,                 SLOT (204), 150, 24, "blast of lightning", "!Lightning Breath!", ""},

    /* Spells for mega1.are from Glop/Erkenbrand. */
    {"general purpose", {53, 53, 53, 53}, {0, 0, 0, 0},     spell_general_purpose, TCO,  PF, NULL,                 SLOT (401), 0,   12, "general purpose ammo","!General Purpose Ammo!", ""},
    {"high explosive",  {53, 53, 53, 53}, {0, 0, 0, 0},     spell_high_explosive,  TCO,  PF, NULL,                 SLOT (402), 0,   12, "high explosive ammo", "!High Explosive Ammo!", ""},

    /* combat and weapons skills */
    {"axe",             {1, 1, 1, 1},     {6, 6, 5, 4},     spell_null,            TI,   PF, &gsn_axe,             SLOT (0),   0,   0,  "",               "!Axe!", ""},
    {"dagger",          {1, 1, 1, 1},     {2, 3, 2, 2},     spell_null,            TI,   PF, &gsn_dagger,          SLOT (0),   0,   0,  "",               "!Dagger!", ""},
    {"flail",           {1, 1, 1, 1},     {6, 3, 6, 4},     spell_null,            TI,   PF, &gsn_flail,           SLOT (0),   0,   0,  "",               "!Flail!", ""},
    {"mace",            {1, 1, 1, 1},     {5, 2, 3, 3},     spell_null,            TI,   PF, &gsn_mace,            SLOT (0),   0,   0,  "",               "!Mace!", ""},
    {"polearm",         {1, 1, 1, 1},     {6, 6, 6, 4},     spell_null,            TI,   PF, &gsn_polearm,         SLOT (0),   0,   0,  "",               "!Polearm!", ""},
    {"shield block",    {1, 1, 1, 1},     {6, 4, 6, 2},     spell_null,            TI,   PF, &gsn_shield_block,    SLOT (0),   0,   0,  "",               "!Shield!", ""},
    {"spear",           {1, 1, 1, 1},     {4, 4, 4, 3},     spell_null,            TI,   PF, &gsn_spear,           SLOT (0),   0,   0,  "",               "!Spear!", ""},
    {"sword",           {1, 1, 1, 1},     {5, 6, 3, 2},     spell_null,            TI,   PF, &gsn_sword,           SLOT (0),   0,   0,  "",               "!Sword!", ""},
    {"whip",            {1, 1, 1, 1},     {6, 5, 5, 4},     spell_null,            TI,   PF, &gsn_whip,            SLOT (0),   0,   0,  "",               "!Whip!", ""},
    {"backstab",        {53, 53, 1, 53},  {0, 0, 5, 0},     spell_null,            TI,   PS, &gsn_backstab,        SLOT (0),   0,   24, "backstab",       "!Backstab!", ""},
    {"bash",            {53, 53, 53, 1},  {0, 0, 0, 4},     spell_null,            TI,   PF, &gsn_bash,            SLOT (0),   0,   24, "bash",           "!Bash!", ""},
    {"berserk",         {53, 53, 53, 18}, {0, 0, 0, 5},     spell_null,            TI,   PF, &gsn_berserk,         SLOT (0),   0,   24, "",               "You feel your pulse slow down.", ""},
    {"dirt kicking",    {53, 53, 3, 3},   {0, 0, 4, 4},     spell_null,            TI,   PF, &gsn_dirt,            SLOT (0),   0,   24, "kicked dirt",    "You rub the dirt out of your eyes.", ""},
    {"disarm",          {53, 53, 12, 11}, {0, 0, 6, 4},     spell_null,            TI,   PF, &gsn_disarm,          SLOT (0),   0,   24, "",               "!Disarm!", ""},
    {"dodge",           {20, 22, 1, 13},  {8, 8, 4, 6},     spell_null,            TI,   PF, &gsn_dodge,           SLOT (0),   0,   0,  "",               "!Dodge!", ""},
    {"enhanced damage", {45, 30, 25, 1},  {10, 9, 5, 3},    spell_null,            TI,   PF, &gsn_enhanced_damage, SLOT (0),   0,   0,  "",               "!Enhanced Damage!", ""},
    {"envenom",         {53, 53, 10, 53}, {0, 0, 4, 0},     spell_null,            TI,   PR, &gsn_envenom,         SLOT (0),   0,   36, "",               "!Envenom!", ""},
    {"hand to hand",    {25, 10, 15, 6},  {8, 5, 6, 4},     spell_null,            TI,   PF, &gsn_hand_to_hand,    SLOT (0),   0,   0,  "",               "!Hand to Hand!", ""},
    {"kick",            {53, 12, 14, 8},  {0, 4, 6, 3},     spell_null,            TCO,  PF, &gsn_kick,            SLOT (0),   0,   12, "kick",           "!Kick!", ""},
    {"parry",           {22, 20, 13, 1},  {8, 8, 6, 4},     spell_null,            TI,   PF, &gsn_parry,           SLOT (0),   0,   0,  "",               "!Parry!", ""},
    {"rescue",          {53, 53, 53, 1},  {0, 0, 0, 4},     spell_null,            TI,   PF, &gsn_rescue,          SLOT (0),   0,   12, "",               "!Rescue!", ""},
    {"trip",            {53, 53, 1, 15},  {0, 0, 4, 8},     spell_null,            TI,   PF, &gsn_trip,            SLOT (0),   0,   24, "trip",           "!Trip!", ""},
    {"second attack",   {30, 24, 12, 5},  {10, 8, 5, 3},    spell_null,            TI,   PF, &gsn_second_attack,   SLOT (0),   0,   0,  "",               "!Second Attack!", ""},
    {"third attack",    {53, 53, 24, 12}, {0, 0, 10, 4},    spell_null,            TI,   PF, &gsn_third_attack,    SLOT (0),   0,   0,  "",               "!Third Attack!", ""},

    /* non-combat skills */
    {"fast healing",    {15, 9, 16, 6},   {8, 5, 6, 4},     spell_null,            TI,   PP, &gsn_fast_healing,    SLOT (0),   0,   0,  "",               "!Fast Healing!", ""},
    {"haggle",          {7, 18, 1, 14},   {5, 8, 3, 6},     spell_null,            TI,   PR, &gsn_haggle,          SLOT (0),   0,   0,  "",               "!Haggle!", ""},
    {"hide",            {53, 53, 1, 12},  {0, 0, 4, 6},     spell_null,            TI,   PR, &gsn_hide,            SLOT (0),   0,   12, "",               "!Hide!", ""},
    {"lore",            {5, 6, 7, 8},     {6, 8, 10, 12},   spell_null,            TI,   PR, &gsn_lore,            SLOT (0),   0,   36, "",               "!Lore!", ""},
    {"meditation",      {6, 6, 15, 15},   {5, 5, 8, 8},     spell_null,            TI,   PP, &gsn_meditation,      SLOT (0),   0,   0,  "",               "Meditation", ""},
    {"peek",            {8, 21, 1, 14},   {5, 7, 3, 6},     spell_null,            TI,   PS, &gsn_peek,            SLOT (0),   0,   0,  "",               "!Peek!", ""},
    {"pick lock",       {25, 25, 7, 25},  {8, 8, 4, 8},     spell_null,            TI,   PS, &gsn_pick_lock,       SLOT (0),   0,   12, "",               "!Pick!", ""},
    {"sneak",           {53, 53, 4, 10},  {0, 0, 4, 6},     spell_null,            TI,   PS, &gsn_sneak,           SLOT (0),   0,   12, "",               "You no longer feel stealthy.", ""},
    {"steal",           {53, 53, 5, 53},  {0, 0, 4, 0},     spell_null,            TI,   PS, &gsn_steal,           SLOT (0),   0,   24, "",               "!Steal!", ""},
    {"scrolls",         {1, 1, 1, 1},     {2, 3, 5, 8},     spell_null,            TI,   PS, &gsn_scrolls,         SLOT (0),   0,   24, "",               "!Scrolls!", ""},
    {"staves",          {1, 1, 1, 1},     {2, 3, 5, 8},     spell_null,            TI,   PS, &gsn_staves,          SLOT (0),   0,   12, "",               "!Staves!", ""},
    {"wands",           {1, 1, 1, 1},     {2, 3, 5, 8},     spell_null,            TI,   PS, &gsn_wands,           SLOT (0),   0,   12, "",               "!Wands!", ""},
    {"recall",          {1, 1, 1, 1},     {2, 2, 2, 2},     spell_null,            TI,   PS, &gsn_recall,          SLOT (0),   0,   12, "",               "!Recall!", ""},

    /* done! */
    {0}
};

#undef TI
#undef TCO
#undef TCD
#undef TCS
#undef TOI
#undef TOCD
#undef TOCO

#undef PS
#undef PF
#undef PR
#undef PP

const GROUP_TYPE group_table[GROUP_MAX + 1] = {
    {"rom basics",      { 0,  0,  0,  0}, {"scrolls", "staves", "wands", "recall"}},
    {"mage basics",     { 0, -1, -1, -1}, {"dagger"}},
    {"cleric basics",   {-1,  0, -1, -1}, {"mace"}},
    {"thief basics",    {-1, -1,  0, -1}, {"dagger", "steal"}},
    {"warrior basics",  {-1, -1, -1,  0}, {"sword", "second attack"}},
    {"mage default",    {40, -1, -1, -1}, {"lore", "beguiling", "combat", "detection", "enhancement", "illusion", "maladictions", "protective", "transportation", "weather"}},
    {"cleric default",  {-1, 40, -1, -1}, {"flail", "attack", "creation", "curative", "benedictions", "detection", "healing", "maladictions", "protective", "shield block", "transportation", "weather"}},
    {"thief default",   {-1, -1, 40, -1}, {"mace", "sword", "backstab", "disarm", "dodge", "second attack", "trip", "hide", "peek", "pick lock", "sneak"}},
    {"warrior default", {-1, -1, -1, 40}, {"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", "parry", "rescue", "third attack"}},
    {"weaponsmaster",   {40, 40, 40, 20}, {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip"}},
    {"attack",          {-1,  5, -1,  8}, {"demonfire", "dispel evil", "dispel good", "earthquake", "flamestrike", "heat metal", "ray of truth"}},
    {"beguiling",       { 4, -1,  6, -1}, {"calm", "charm person", "sleep"}},
    {"benedictions",    {-1,  4, -1,  8}, {"bless", "calm", "frenzy", "holy word", "remove curse"}},
    {"combat",          { 6, -1, 10,  9}, {"acid blast", "burning hands", "chain lightning", "chill touch", "colour spray", "fireball", "lightning bolt", "magic missile", "shocking grasp"}},
    {"creation",        { 4,  4,  8,  8}, {"continual light", "create food", "create spring", "create water", "create rose", "floating disc"}},
    {"curative",        {-1,  4, -1,  8}, {"cure blindness", "cure disease", "cure poison"}},
    {"detection",       { 4,  3,  6, -1}, {"detect evil", "detect good", "detect hidden", "detect invis", "detect magic", "detect poison", "farsight", "identify", "know alignment", "locate object"}},
    {"draconian",       { 8, -1, -1, -1}, {"acid breath", "fire breath", "frost breath", "gas breath", "lightning breath"}},
    {"enchantment",     { 6, -1, -1, -1}, {"enchant armor", "enchant weapon", "fireproof", "recharge"}},
    {"enhancement",     { 5, -1,  9,  9}, {"giant strength", "haste", "infravision", "refresh"}},
    {"harmful",         {-1,  3, -1,  6}, {"cause critical", "cause light", "cause serious", "harm"}},
    {"healing",         {-1,  3, -1,  6}, {"cure critical", "cure light", "cure serious", "heal", "mass healing", "refresh"}},
    {"illusion",        { 4, -1,  7, -1}, {"invis", "mass invis", "ventriloquate"}},
    {"maladictions",    { 5,  5,  9,  9}, {"blindness", "change sex", "curse", "energy drain", "plague", "poison", "slow", "weaken"}},
    {"protective",      { 4,  4,  7,  8}, {"armor", "cancellation", "dispel magic", "fireproof", "protection evil", "protection good", "sanctuary", "shield", "stone skin"}},
    {"transportation",  { 4,  4,  8,  9}, {"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport", "word of recall"}},
    {"weather",         { 4,  4,  8,  8}, {"call lightning", "control weather", "faerie fire", "faerie fog", "lightning bolt"}},
    {0}
};

const SECTOR_TYPE sector_table[SECT_MAX + 1] = {
    {SECT_INSIDE,       "inside",      1, 'C'},
    {SECT_CITY,         "city",        2, 'c'},
    {SECT_FIELD,        "field",       2, 'G'},
    {SECT_FOREST,       "forest",      3, 'g'},
    {SECT_HILLS,        "hills",       4, 'y'},
    {SECT_MOUNTAIN,     "mountain",    6, 'R'},
    {SECT_WATER_SWIM,   "swim",        4, 'B'},
    {SECT_WATER_NOSWIM, "noswim",      1, 'b'},
    {SECT_UNUSED,       "unused_sect", 6, 'w'},
    {SECT_AIR,          "air",        10, 'W'},
    {SECT_DESERT,       "desert",      6, 'Y'},
    {0}
};

const NANNY_HANDLER nanny_table[NANNY_MAX + 1] = {
    {CON_ANSI,             "ansi",                 nanny_ansi},
    {CON_GET_NAME,         "get_player_name",      nanny_get_player_name},
    {CON_GET_OLD_PASSWORD, "get_old_password",     nanny_get_old_password},
    {CON_BREAK_CONNECT,    "break_connect",        nanny_break_connect},
    {CON_CONFIRM_NEW_NAME, "confirm_new_name",     nanny_confirm_new_name},
    {CON_GET_NEW_PASSWORD, "get_new_password",     nanny_get_new_password},
    {CON_CONFIRM_PASSWORD, "confirm_new_password", nanny_confirm_new_password},
    {CON_GET_NEW_RACE,     "get_new_race",         nanny_get_new_race},
    {CON_GET_NEW_SEX,      "get_new_sex",          nanny_get_new_sex},
    {CON_GET_NEW_CLASS,    "get_new_class",        nanny_get_new_class},
    {CON_GET_ALIGNMENT,    "get_alignment",        nanny_get_alignment},
    {CON_DEFAULT_CHOICE,   "default_choice",       nanny_default_choice},
    {CON_PICK_WEAPON,      "pick_weapon",          nanny_pick_weapon},
    {CON_GEN_GROUPS,       "gen_groups",           nanny_gen_groups},
    {CON_READ_IMOTD,       "read_imotd",           nanny_read_imotd},
    {CON_READ_MOTD,        "read_motd",            nanny_read_motd},

    /* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
    /* ch MUST be PC here; have nwrite check for PC status! */
    {CON_NOTE_TO,          "note_to",              handle_con_note_to},
    {CON_NOTE_SUBJECT,     "note_subject",         handle_con_note_subject},
    {CON_NOTE_EXPIRE,      "note_expire",          handle_con_note_expire},
    {CON_NOTE_TEXT,        "note_text",            handle_con_note_text},
    {CON_NOTE_FINISH,      "note_finish",          handle_con_note_finish},

    {-1, NULL}
};

const FURNITURE_BITS furniture_table[] = {
    {POS_STANDING, "standing", STAND_AT, STAND_ON, STAND_IN},
    {POS_SITTING,  "sitting",  SIT_AT,   SIT_ON,   SIT_IN},
    {POS_RESTING,  "resting",  REST_AT,  REST_ON,  REST_IN},
    {POS_SLEEPING, "sleeping", SLEEP_AT, SLEEP_ON, SLEEP_IN},
    {-1, NULL, 0, 0}
};

const MAP_LOOKUP_TABLE map_lookup_table[] = {
    {MAP_LOOKUP_WEAPON_TYPE, "weapon_type", NULL},
    {MAP_LOOKUP_ATTACK_TYPE, "attack_type", NULL},
    {MAP_LOOKUP_LIQUID,      "liquid",      NULL},
    {MAP_LOOKUP_SKILL,       "skill",       NULL},
    {-1, NULL, NULL},
};

const MAP_LOOKUP_TABLE map_flags_table[] = {
    {MAP_FLAGS_WEAPON,    "weapon",    weapon_flags},
    {MAP_FLAGS_CONT,      "container", container_flags},
    {MAP_FLAGS_FURNITURE, "furniture", furniture_flags},
    {MAP_FLAGS_EXIT,      "exit",      exit_flags},
    {MAP_FLAGS_PORTAL,    "portal",    portal_flags},
    {-1, NULL, NULL},
};

const OBJ_MAP obj_map_table[] = {
    {ITEM_WEAPON, {
        {0, "weapon_type", MAP_LOOKUP, MAP_LOOKUP_WEAPON_TYPE},
        {1, "dice_num",    MAP_INTEGER, 0},
        {2, "dice_size",   MAP_INTEGER, 0},
        {3, "attack_type", MAP_LOOKUP, MAP_LOOKUP_ATTACK_TYPE},
        {4, "flags",       MAP_FLAGS, MAP_FLAGS_WEAPON},
    }},
    {ITEM_CONTAINER, {
        {0, "capacity",    MAP_INTEGER, 0},
        {1, "flags",       MAP_FLAGS, MAP_FLAGS_CONT},
        {2, "key",         MAP_INTEGER, 0},
        {3, "max_weight",  MAP_INTEGER, 0},
        {4, "weight_mult", MAP_INTEGER, 0}
    }},
    {ITEM_DRINK_CON, {
        {0, "capacity",    MAP_INTEGER, 0},
        {1, "filled",      MAP_INTEGER, 0},
        {2, "liquid",      MAP_LOOKUP, MAP_LOOKUP_LIQUID},
        {3, "poisoned",    MAP_BOOLEAN, 0},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_FOUNTAIN, {
        {0, "capacity",    MAP_INTEGER, 0},
        {1, "filled",      MAP_INTEGER, 0},
        {2, "liquid",      MAP_LOOKUP, MAP_LOOKUP_LIQUID},
        {3, "poisoned",    MAP_BOOLEAN, 0},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_WAND, {
        {0, "level",       MAP_INTEGER, 0},
        {1, "recharge",    MAP_INTEGER, 0},
        {2, "charges",     MAP_INTEGER, 0},
        {3, "skill",       MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_STAFF, {
        {0, "level",       MAP_INTEGER, 0},
        {1, "recharge",    MAP_INTEGER, 0},
        {2, "charges",     MAP_INTEGER, 0},
        {3, "skill",       MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_FOOD, {
        {0, "hunger",      MAP_INTEGER, 0},
        {1, "fullness",    MAP_INTEGER, 0},
        {2, NULL,          MAP_IGNORE, 0},
        {3, "poisoned",    MAP_BOOLEAN, 0},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_MONEY, {
        {0, "silver",      MAP_INTEGER, 0},
        {1, "gold",        MAP_INTEGER, 0},
        {2, NULL,          MAP_IGNORE, 0},
        {3, NULL,          MAP_IGNORE, 0},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_ARMOR, {
        {0, "vs_pierce",   MAP_INTEGER, 0},
        {1, "vs_bash",     MAP_INTEGER, 0},
        {2, "vs_slash",    MAP_INTEGER, 0},
        {3, "vs_magic",    MAP_INTEGER, 0},
        {4, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_POTION, {
        {0, "level",       MAP_INTEGER, 0},
        {1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_PILL, {
        {0, "level",       MAP_INTEGER, 0},
        {1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_SCROLL, {
        {0, "level",       MAP_INTEGER, 0},
        {1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_MAP, {
        {0, "persist",     MAP_BOOLEAN, 0},
        {1, NULL,          MAP_IGNORE, 0},
        {2, NULL,          MAP_IGNORE, 0},
        {3, NULL,          MAP_IGNORE, 0},
        {4, NULL,          MAP_IGNORE, 0},
    }},
    {ITEM_FURNITURE, {
        {0, "max_people",  MAP_INTEGER, 0},
        {1, "max_weight",  MAP_INTEGER, 0},
        {2, "flags",       MAP_FLAGS, MAP_FLAGS_FURNITURE},
        {3, "heal_rate",   MAP_INTEGER, 0},
        {4, "mana_rate",   MAP_INTEGER, 0},
    }},
    {ITEM_LIGHT, {
        {0, NULL,          MAP_IGNORE, 0},
        {1, NULL,          MAP_IGNORE, 0},
        {2, "duration",    MAP_INTEGER, 0},
        {3, NULL,          MAP_IGNORE, 0},
        {4, NULL,          MAP_IGNORE, 0},
    }},
    {ITEM_PORTAL, {
        {0, "charges",     MAP_INTEGER, 0},
        {1, "exit_flags",  MAP_FLAGS, MAP_FLAGS_EXIT},
        {2, "portal_flags",MAP_FLAGS, MAP_FLAGS_PORTAL},
        {3, "to_vnum",     MAP_INTEGER, 0},
        {4, "key",         MAP_INTEGER, 0},
    }},

    #define OBJ_MAP_NO_VALUES(type) \
        { type, { \
            { 0, NULL, MAP_IGNORE, 0 }, { 1, NULL, MAP_IGNORE, 0 }, \
            { 2, NULL, MAP_IGNORE, 0 }, { 3, NULL, MAP_IGNORE, 0 }, \
            { 4, NULL, MAP_IGNORE, 0 } \
        }}

    OBJ_MAP_NO_VALUES(ITEM_TRASH),
    OBJ_MAP_NO_VALUES(ITEM_GEM),
    OBJ_MAP_NO_VALUES(ITEM_TREASURE),
    OBJ_MAP_NO_VALUES(ITEM_KEY),
    OBJ_MAP_NO_VALUES(ITEM_JEWELRY),
    OBJ_MAP_NO_VALUES(ITEM_CLOTHING),
    OBJ_MAP_NO_VALUES(ITEM_BOAT),
    OBJ_MAP_NO_VALUES(ITEM_CORPSE_NPC),
    OBJ_MAP_NO_VALUES(ITEM_CORPSE_PC),
    OBJ_MAP_NO_VALUES(ITEM_JUKEBOX),
    OBJ_MAP_NO_VALUES(ITEM_WARP_STONE),

    OBJ_MAP_NO_VALUES(-1),
};

/* for doors */
const DOOR_TYPE door_table[DIR_MAX + 1] = {
    {DIR_NORTH, "north", "the north", "to the north", DIR_SOUTH},
    {DIR_EAST,  "east",  "the east",  "to the east",  DIR_WEST},
    {DIR_SOUTH, "south", "the south", "to the south", DIR_NORTH},
    {DIR_WEST,  "west",  "the west",  "to the west",  DIR_EAST},
    {DIR_UP,    "up",    "above",     "above you",    DIR_DOWN},
    {DIR_DOWN,  "down",  "below",     "below you",    DIR_UP},
    {0},
};

/* the function table */
const SPEC_TYPE spec_table[SPEC_MAX + 1] = {
    {"spec_breath_any",       spec_breath_any},
    {"spec_breath_acid",      spec_breath_acid},
    {"spec_breath_fire",      spec_breath_fire},
    {"spec_breath_frost",     spec_breath_frost},
    {"spec_breath_gas",       spec_breath_gas},
    {"spec_breath_lightning", spec_breath_lightning},
    {"spec_cast_adept",       spec_cast_adept},
    {"spec_cast_cleric",      spec_cast_cleric},
    {"spec_cast_judge",       spec_cast_judge},
    {"spec_cast_mage",        spec_cast_mage},
    {"spec_cast_undead",      spec_cast_undead},
    {"spec_executioner",      spec_executioner},
    {"spec_fido",             spec_fido},
    {"spec_guard",            spec_guard},
    {"spec_janitor",          spec_janitor},
    {"spec_mayor",            spec_mayor},
    {"spec_poison",           spec_poison},
    {"spec_thief",            spec_thief},
    {"spec_nasty",            spec_nasty},
    {"spec_troll_member",     spec_troll_member},
    {"spec_ogre_member",      spec_ogre_member},
    {"spec_patrolman",        spec_patrolman},
    {0}
};

const COLOUR_SETTING_TYPE colour_setting_table[COLOUR_MAX + 1] = {
    {COLOUR_TEXT,          "text",          't', CC_BACK_DEFAULT | CC_WHITE},
    {COLOUR_AUCTION,       "auction",       'a', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_AUCTION_TEXT,  "auction_text",  'A', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_GOSSIP,        "gossip",        'd', CC_BACK_DEFAULT | CC_MAGENTA},
    {COLOUR_GOSSIP_TEXT,   "gossip_text",   '9', CC_BACK_DEFAULT | CC_BRIGHT_MAGENTA},
    {COLOUR_MUSIC,         "music",         'e', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_MUSIC_TEXT,    "music_text",    'E', CC_BACK_DEFAULT | CC_BRIGHT_RED},
    {COLOUR_QUESTION,      "question",      'q', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_QUESTION_TEXT, "question_text", 'Q', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_ANSWER,        "answer",        'f', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW},
    {COLOUR_ANSWER_TEXT,   "answer_text",   'F', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {COLOUR_QUOTE,         "quote",         'h', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_QUOTE_TEXT,    "quote_text",    'H', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_IMMTALK_TEXT,  "immtalk_text",  'i', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_IMMTALK_TYPE,  "immtalk_type",  'I', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_INFO,          "info",          'j', CC_BACK_DEFAULT | CC_BRIGHT_YELLOW | CB_BEEP},
    {COLOUR_SAY,           "say",           '6', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_SAY_TEXT,      "say_text",      '7', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_TELL,          "tell",          'k', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_TELL_TEXT,     "tell_text",     'K', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_REPLY,         "reply",         'l', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_REPLY_TEXT,    "reply_text",    'L', CC_BACK_DEFAULT | CC_BRIGHT_GREEN},
    {COLOUR_GTELL_TEXT,    "gtell_text",    'n', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_GTELL_TYPE,    "gtell_type",    'N', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_WIZNET,        "wiznet",        'B', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_ROOM_TITLE,    "room_title",    's', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_ROOM_TEXT,     "room_text",     'S', CC_BACK_DEFAULT | CC_WHITE},
    {COLOUR_ROOM_EXITS,    "room_exits",    'o', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_ROOM_THINGS,   "room_things",   'O', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_PROMPT,        "prompt",        'p', CC_BACK_DEFAULT | CC_CYAN},
    {COLOUR_FIGHT_DEATH,   "fight_death",   '1', CC_BACK_DEFAULT | CC_BRIGHT_RED},
    {COLOUR_FIGHT_YHIT,    "fight_yhit",    '2', CC_BACK_DEFAULT | CC_GREEN},
    {COLOUR_FIGHT_OHIT,    "fight_ohit",    '3', CC_BACK_DEFAULT | CC_YELLOW},
    {COLOUR_FIGHT_THIT,    "fight_thit",    '4', CC_BACK_DEFAULT | CC_RED},
    {COLOUR_FIGHT_SKILL,   "fight_skill",   '5', CC_BACK_DEFAULT | CC_BRIGHT_WHITE},
    {0}
};

const COLOUR_TYPE colour_table[] = {
    /* All forecolors */
    {CM_FORECOLOUR, CC_DEFAULT,        "none"},
    {CM_FORECOLOUR, CC_BLACK,          "black"},
    {CM_FORECOLOUR, CC_RED,            "red"},
    {CM_FORECOLOUR, CC_GREEN,          "green"},
    {CM_FORECOLOUR, CC_YELLOW,         "yellow"},
    {CM_FORECOLOUR, CC_BLUE,           "blue"},
    {CM_FORECOLOUR, CC_MAGENTA,        "magenta"},
    {CM_FORECOLOUR, CC_CYAN,           "cyan"},
    {CM_FORECOLOUR, CC_WHITE,          "white"},
    {CM_FORECOLOUR, CC_DARK_GREY,      "grey"},
    {CM_FORECOLOUR, CC_BRIGHT_RED,     "hi-red"},
    {CM_FORECOLOUR, CC_BRIGHT_GREEN,   "hi-green"},
    {CM_FORECOLOUR, CC_BRIGHT_YELLOW,  "hi-yellow"},
    {CM_FORECOLOUR, CC_BRIGHT_BLUE,    "hi-blue"},
    {CM_FORECOLOUR, CC_BRIGHT_MAGENTA, "hi-magenta"},
    {CM_FORECOLOUR, CC_BRIGHT_CYAN,    "hi-cyan"},
    {CM_FORECOLOUR, CC_BRIGHT_WHITE,   "hi-white"},

    /* All backcolors */
    {CM_BACKCOLOUR, CC_BACK_DEFAULT,        "back-none"},
    {CM_BACKCOLOUR, CC_BACK_BLACK,          "back-black"},
    {CM_BACKCOLOUR, CC_BACK_RED,            "back-red"},
    {CM_BACKCOLOUR, CC_BACK_GREEN,          "back-green"},
    {CM_BACKCOLOUR, CC_BACK_YELLOW,         "back-yellow"},
    {CM_BACKCOLOUR, CC_BACK_BLUE,           "back-blue"},
    {CM_BACKCOLOUR, CC_BACK_MAGENTA,        "back-magenta"},
    {CM_BACKCOLOUR, CC_BACK_CYAN,           "back-cyan"},
    {CM_BACKCOLOUR, CC_BACK_WHITE,          "back-white"},

    /* non-standard; let's not support it. */
/*
    {CM_BACKCOLOUR, CC_BACK_DARK_GREY,      "back-grey"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_RED,     "back-hi-red"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_GREEN,   "back-hi-green"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_YELLOW,  "back-hi-yellow"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_BLUE,    "back-hi-blue"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_MAGENTA, "back-hi-magenta"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_CYAN,    "back-hi-cyan"},
    {CM_BACKCOLOUR, CC_BACK_BRIGHT_WHITE,   "back-hi-white"},
*/

    /* Individual bits */
    {CM_BEEP,      CB_BEEP,      "beep"},
    {CM_BEEP,      0x00,         "nobeep"},

    /* End */
    {0}
};

const WEAR_TYPE wear_table[WEAR_MAX + 1] = {
    {WEAR_LIGHT,    "light",        "<used as light>       ", ITEM_LIGHT},
    {WEAR_FINGER_L, "left finger",  "<worn on L-finger>    ", ITEM_WEAR_FINGER},
    {WEAR_FINGER_R, "right finger", "<worn on R-finger>    ", ITEM_WEAR_FINGER},
    {WEAR_NECK_1,   "neck 1",       "<worn around neck 1>  ", ITEM_WEAR_NECK},
    {WEAR_NECK_2,   "neck 2",       "<worn around neck 2>  ", ITEM_WEAR_NECK},
    {WEAR_BODY,     "torso",        "<worn on torso>       ", ITEM_WEAR_BODY},
    {WEAR_HEAD,     "head",         "<worn on head>        ", ITEM_WEAR_HEAD},
    {WEAR_LEGS,     "legs",         "<worn on legs>        ", ITEM_WEAR_LEGS},
    {WEAR_FEET,     "feet",         "<worn on feet>        ", ITEM_WEAR_FEET},
    {WEAR_HANDS,    "hands",        "<worn on hands>       ", ITEM_WEAR_HANDS},
    {WEAR_ARMS,     "arms",         "<worn on arms>        ", ITEM_WEAR_ARMS},
    {WEAR_SHIELD,   "shield",       "<worn as shield>      ", ITEM_WEAR_SHIELD},
    {WEAR_ABOUT,    "body",         "<worn about body>     ", ITEM_WEAR_ABOUT},
    {WEAR_WAIST,    "waist",        "<worn about waist>    ", ITEM_WEAR_WAIST},
    {WEAR_WRIST_L,  "left wrist",   "<worn around L-wrist> ", ITEM_WEAR_WRIST},
    {WEAR_WRIST_R,  "right wrist",  "<worn around R-wrist> ", ITEM_WEAR_WRIST},
    {WEAR_WIELD,    "wield",        "<wielded>             ", ITEM_WIELD},
    {WEAR_HOLD,     "hold",         "<held>                ", ITEM_HOLD},
    {WEAR_FLOAT,    "float",        "<floating nearby>     ", ITEM_WEAR_FLOAT},
    {0}
};

const MATERIAL_TYPE material_table[MATERIAL_MAX + 1] = {
    {MATERIAL_GENERIC,    "generic",    'x'},
    {MATERIAL_ADAMANTITE, "adamantite", 'D'},
    {MATERIAL_ALUMINUM,   "aluminum",   'w'},
    {MATERIAL_BRASS,      "brass",      'y'},
    {MATERIAL_BRONZE,     "bronze",     'y'},
    {MATERIAL_CHINA,      "china",      'W'},
    {MATERIAL_CLAY,       "clay",       'R'},
    {MATERIAL_CLOTH,      "cloth",      'y'},
    {MATERIAL_COPPER,     "copper",     'y'},
    {MATERIAL_CRYSTAL,    "crystal",    'C'},
    {MATERIAL_DIAMOND,    "diamond",    'W'},
    {MATERIAL_ENERGY,     "energy",     'm'},
    {MATERIAL_FLESH,      "flesh",      'r'},
    {MATERIAL_FOOD,       "food",       'y'},
    {MATERIAL_FUR,        "fur",        'y'},
    {MATERIAL_GEM,        "gem",        'M'},
    {MATERIAL_GLASS,      "glass",      'C'},
    {MATERIAL_GOLD,       "gold",       'Y'},
    {MATERIAL_ICE,        "ice",        'C'},
    {MATERIAL_IRON,       "iron",       'D'},
    {MATERIAL_IVORY,      "ivory",      'W'},
    {MATERIAL_LEAD,       "lead",       'D'},
    {MATERIAL_LEATHER,    "leather",    'y'},
    {MATERIAL_MEAT,       "meat",       'R'},
    {MATERIAL_MITHRIL,    "mithril",    'c'},
    {MATERIAL_OBSIDIAN,   "obsidian",   'D'},
    {MATERIAL_PAPER,      "paper",      'w'},
    {MATERIAL_PARCHMENT,  "parchment",  'Y'},
    {MATERIAL_PEARL,      "pearl",      'W'},
    {MATERIAL_PLATINUM,   "platinum",   'W'},
    {MATERIAL_RUBBER,     "rubber",     'x'},
    {MATERIAL_SHADOW,     "shadow",     'D'},
    {MATERIAL_SILVER,     "silver",     'w'},
    {MATERIAL_STEEL,      "steel",      'w'},
    {MATERIAL_TIN,        "tin",        'w'},
    {MATERIAL_VELLUM,     "vellum",     'W'},
    {MATERIAL_WATER,      "water",      'B'},
    {MATERIAL_WOOD,       "wood",       'y'},
    {0}
};

#define RE_NULL

#define RECYCLE_REAL_ENTRY(rtype, rname, vtype, name_off, init, dispose) { \
    rtype, \
    #rname, \
    sizeof (vtype), \
    GET_OFFSET(vtype, rec_data), \
    name_off, \
    init, \
    dispose \
}

#define RECYCLE_ENTRY(rtype, rname, vtype, init, dispose) \
    RECYCLE_REAL_ENTRY(rtype, rname, vtype, -1, init, dispose)

#define RECYCLE_N_ENTRY(rtype, rname, vtype, name, init, dispose) \
    RECYCLE_REAL_ENTRY(rtype, rname, vtype, GET_OFFSET(vtype, name), init, dispose)

#define BLANK -1

RECYCLE_TYPE recycle_table[RECYCLE_MAX + 1] = {
    RECYCLE_N_ENTRY (RECYCLE_BAN_DATA,         ban,         BAN_DATA,         name, ban_init,         ban_dispose),
    RECYCLE_N_ENTRY (RECYCLE_AREA_DATA,        area,        AREA_DATA,        name, area_init,        area_dispose),
    RECYCLE_ENTRY   (RECYCLE_EXTRA_DESCR_DATA, extra_descr, EXTRA_DESCR_DATA,       extra_descr_init, extra_descr_dispose),
    RECYCLE_ENTRY   (RECYCLE_EXIT_DATA,        exit,        EXIT_DATA,              exit_init,        exit_dispose),
    RECYCLE_N_ENTRY (RECYCLE_ROOM_INDEX_DATA,  room_index,  ROOM_INDEX_DATA,  name, room_index_init,  room_index_dispose),
    RECYCLE_N_ENTRY (RECYCLE_OBJ_INDEX_DATA,   obj_index,   OBJ_INDEX_DATA,   name, obj_index_init,   obj_index_dispose),
    RECYCLE_ENTRY   (RECYCLE_SHOP_DATA,        shop,        SHOP_DATA,              shop_init,        NULL),
    RECYCLE_ENTRY   (RECYCLE_MOB_INDEX_DATA,   mob_index,   MOB_INDEX_DATA,         mob_index_init,   mob_index_dispose),
    RECYCLE_ENTRY   (RECYCLE_RESET_DATA,       reset_data,  RESET_DATA,             reset_data_init,  NULL),
    RECYCLE_N_ENTRY (RECYCLE_HELP_DATA,        help,        HELP_DATA,     keyword, NULL,             help_dispose),
    RECYCLE_ENTRY   (RECYCLE_MPROG_CODE,       mpcode,      MPROG_CODE,             mpcode_init,      mpcode_dispose),
    RECYCLE_ENTRY   (RECYCLE_DESCRIPTOR_DATA,  descriptor,  DESCRIPTOR_DATA,        descriptor_init,  descriptor_dispose),
    RECYCLE_ENTRY   (RECYCLE_GEN_DATA,         gen_data,    GEN_DATA,               NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_AFFECT_DATA,      affect,      AFFECT_DATA,            NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_OBJ_DATA,         obj,         OBJ_DATA,               NULL,             obj_dispose),
    RECYCLE_ENTRY   (RECYCLE_CHAR_DATA,        char,        CHAR_DATA,              char_init,        char_dispose),
    RECYCLE_ENTRY   (RECYCLE_PC_DATA,          pcdata,      PC_DATA,                pcdata_init,      pcdata_dispose),
    RECYCLE_ENTRY   (RECYCLE_MEM_DATA,         mem_data,    MEM_DATA,               NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_BUFFER,           buf,         BUFFER,                 buf_init,         buf_dispose),
    RECYCLE_ENTRY   (RECYCLE_MPROG_LIST,       mprog,       MPROG_LIST,             mprog_init,       mprog_dispose),
    RECYCLE_N_ENTRY (RECYCLE_HELP_AREA,        had,         HELP_AREA,    filename, NULL,             had_dispose),
    RECYCLE_ENTRY   (RECYCLE_NOTE_DATA,        note,        NOTE_DATA,              NULL,             note_dispose),
    RECYCLE_N_ENTRY (RECYCLE_SOCIAL_TYPE,      social,      SOCIAL_TYPE,      name, NULL,             social_dispose),
    RECYCLE_N_ENTRY (RECYCLE_PORTAL_EXIT_TYPE, portal_exit, PORTAL_EXIT_TYPE, name, NULL,             portal_exit_dispose),
    RECYCLE_ENTRY   (RECYCLE_PORTAL_TYPE,      portal,      PORTAL_TYPE,            NULL,             portal_dispose),
    {0}
};

/* Technically not const, but this is a good place to have it! */
BOARD_DATA board_table[BOARD_MAX + 1] = {
    {"General",  "General discussion",           0, 2,     "all", DEF_INCLUDE, 21, NULL, FALSE},
    {"Ideas",    "Suggestion for improvement",   0, 2,     "all", DEF_NORMAL,  60, NULL, FALSE},
    {"Announce", "Announcements from Immortals", 0, L_IMM, "all", DEF_NORMAL,  60, NULL, FALSE},
    {"Bugs",     "Typos, bugs, errors",          0, 1,     "imm", DEF_NORMAL,  60, NULL, FALSE},
    {"Personal", "Personal messages",            0, 1,     "all", DEF_EXCLUDE, 28, NULL, FALSE},
    {0}
};

/* wiznet table and prototype for future flag setting */
const WIZNET_TYPE wiznet_table[WIZNET_MAX + 1] = {
    {WIZ_ON,        "on",        IM},
    {WIZ_PREFIX,    "prefix",    IM},
    {WIZ_TICKS,     "ticks",     IM},
    {WIZ_LOGINS,    "logins",    IM},
    {WIZ_SITES,     "sites",     L4},
    {WIZ_LINKS,     "links",     L7},
    {WIZ_NEWBIE,    "newbies",   IM},
    {WIZ_SPAM,      "spam",      L5},
    {WIZ_DEATHS,    "deaths",    IM},
    {WIZ_RESETS,    "resets",    L4},
    {WIZ_MOBDEATHS, "mobdeaths", L4},
    {WIZ_FLAGS,     "flags",     L5},
    {WIZ_PENALTIES, "penalties", L5},
    {WIZ_SACCING,   "saccing",   L5},
    {WIZ_LEVELS,    "levels",    IM},
    {WIZ_LOAD,      "load",      L2},
    {WIZ_RESTORE,   "restore",   L2},
    {WIZ_SNOOPS,    "snoops",    L2},
    {WIZ_SWITCHES,  "switches",  L2},
    {WIZ_SECURE,    "secure",    L1},
    {-1, NULL, 0}
};

const AFFECT_BIT_TYPE affect_bit_table[] = {
    {"affects", TO_AFFECTS, affect_flags, "affect_flags"},
    {"object",  TO_OBJECT,  extra_flags,  "extra_flags"},
    {"immune",  TO_IMMUNE,  res_flags,    "res_flags"},
    {"resist",  TO_RESIST,  res_flags,    "res_flags"},
    {"vuln",    TO_VULN,    res_flags,    "res_flags"},
    {"weapon",  TO_WEAPON,  weapon_flags, "weapon_flags"},
    {NULL,      0,          NULL},
};

const DAY_TYPE day_table[DAY_MAX + 1] = {
    { DAY_MOON,       "the Moon" },
    { DAY_BULL,       "the Bull" },
    { DAY_DECEPTION,  "Deception" },
    { DAY_THUNDER,    "Thunder" },
    { DAY_FREEDOM,    "Freedom" },
    { DAY_GREAT_GODS, "the Great Gods" },
    { DAY_SUN,        "the Sun" },
    { -1, NULL }
};

const MONTH_TYPE month_table[MONTH_MAX + 1] = {
    { MONTH_WINTER,           "Winter" },
    { MONTH_WINTER_WOLF,      "the Winter Wolf" },
    { MONTH_FROST_GIANT,      "the Frost Giant" },
    { MONTH_OLD_FORCES,       "the Old Forces" },
    { MONTH_GRAND_STRUGGLE,   "the Grand Struggle" },
    { MONTH_SPRING,           "the Spring" },
    { MONTH_NATURE,           "Nature" },
    { MONTH_FUTILITY,         "Futility" },
    { MONTH_DRAGON,           "the Dragon" },
    { MONTH_SUN,              "the Sun" },
    { MONTH_HEAT,             "the Heat" },
    { MONTH_BATTLE,           "the Battle" },
    { MONTH_DARK_SHADES,      "the Dark Shades" },
    { MONTH_SHADOWS,          "the Shadows" },
    { MONTH_LONG_SHADOWS,     "the Long Shadows" },
    { MONTH_ANCIENT_DARKNESS, "the Ancient Darkness" },
    { MONTH_GREAT_EVIL,       "the Great Evil" },
    { -1, NULL }
};
