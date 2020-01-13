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

#include <stddef.h>

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

/* Useful macros for defining rows in our master table. */
#define TFLAGS(table, desc) \
    { table, #table, TABLE_FLAGS, desc, \
      sizeof (FLAG_T), NULL, json_tblw_flag }
#define TTYPES(table, desc) \
    { table, #table, TABLE_TYPES, desc, \
      sizeof (TYPE_T), NULL, json_tblw_type }
#define TTABLE(table, name, desc, obj_name, jwrite) \
    { table, name, TABLE_UNIQUE, desc, sizeof(table[0]), obj_name, jwrite }

const TABLE_T master_table[TABLE_MAX + 1] = {
    /* from flags.h */
    TFLAGS (affect_flags,     "Mobile affects."),
    TFLAGS (area_flags,       "Area attributes."),
    TFLAGS (comm_flags,       "Communication channel flags."),
    TFLAGS (container_flags,  "Container status."),
    TFLAGS (dam_flags,        "Attributes for damage types."),
    TFLAGS (exit_flags,       "Exit types."),
    TFLAGS (extra_flags,      "Object attributes."),
    TFLAGS (form_flags,       "Mobile body form."),
    TFLAGS (furniture_flags,  "Flags for furniture."),
    TFLAGS (gate_flags,       "Portal gate flags."),
    TFLAGS (mob_flags,        "Mobile flags."),
    TFLAGS (mprog_flags,      "MobProgram flags."),
    TFLAGS (off_flags,        "Mobile offensive behaviour."),
    TFLAGS (part_flags,       "Mobile body parts."),
    TFLAGS (plr_flags,        "Player flags."),
    TFLAGS (res_flags,        "Mobile immunity."),
    TFLAGS (room_flags,       "Room attributes."),
    TFLAGS (weapon_flags,     "Special weapon type."),
    TFLAGS (wear_flags,       "Types of wear locations."),

    /* from types.h */
    TTYPES (ac_types,         "AC for different attacks."),
    TTYPES (affect_apply_types, "Affect apply types."),
    TTYPES (board_def_types,  "Types of boards."),
    TTYPES (cond_types,       "Conditions like thirst/hunger."),
    TTYPES (door_reset_types, "Door reset types."),
    TTYPES (item_types,       "Types of objects."),
    TTYPES (position_types,   "Mobile positions."),
    TTYPES (sector_types,     "Sector types, terrain."),
    TTYPES (sex_types,        "Sexes."),
    TTYPES (size_types,       "Mobile sizes."),
    TTYPES (skill_target_types, "Targets for skills and spells."),
    TTYPES (stat_types,       "Available stats for characters."),
    TTYPES (weapon_types,     "Weapon classes."),

    /* from tables.h */
    /* TODO: read all of these! */
    TTABLE (attack_table,     "attacks",      "Attack types and properties.", "attack",        json_tblw_attack),
    TTABLE (board_table,      "boards",       "Discussion boards.",           "board",         json_tblw_board),
    TTABLE (clan_table,       "clans",        "Player clans.",                "clan",          json_tblw_clan),
    TTABLE (class_table,      "classes",      "Classes and statistics.",      "class",         json_tblw_class),
    TTABLE (colour_setting_table, "color_settings", "Configurable colours.",  "color_setting", json_tblw_colour_setting),
    TTABLE (colour_table,     "colors",       "Colour values.",               "color",         json_tblw_colour),
    TTABLE (con_app_table,    "con_app",      "Con apply table.",             "con_app",       json_tblw_con_app),
    TTABLE (condition_table,  "conditions",   "Messages based on % of hp.",   "condition",     json_tblw_condition),
    TTABLE (dam_table,        "dam_types",    "Damage types and properties.", "dam_type",      json_tblw_dam),
    TTABLE (day_table,        "days",         "Days of the week.",            "day",           json_tblw_day),
    TTABLE (dex_app_table,    "dex_app",      "Dex apply table.",             "dex_app",       json_tblw_dex_app),
    TTABLE (door_table,       "doors",        "Exit names.",                  "door",          json_tblw_door),
    TTABLE (int_app_table,    "int_app",      "Int apply table.",             "int_app",       json_tblw_int_app),
    TTABLE (item_table,       "items",        "Item types and properties.",   "item",          json_tblw_item),
    TTABLE (liq_table,        "liquids",      "Liquid types.",                "liquid",        json_tblw_liq),
    TTABLE (material_table,   "materials",    "Material properties",          "material",      json_tblw_material),
    TTABLE (month_table,      "months",       "Months of the year.",          "month",         json_tblw_month),
    TTABLE (pc_race_table,    "pc_races",     "Playable race data.",          "player_race",   json_tblw_pc_race),
    TTABLE (pose_table,       "pose",         "Poses based on class and level", "pose",        json_tblw_pose),
    TTABLE (position_table,   "positions",    "Character positions.",         "position",      json_tblw_position),
    TTABLE (race_table,       "races",        "Races and statistics.",        "race",          json_tblw_race),
    TTABLE (sector_table,     "sectors",      "Sector/terrain properties.",   "sector",        json_tblw_sector),
    TTABLE (sex_table,        "sexes",        "Gender settings.",             "sex",           json_tblw_sex),
    TTABLE (size_table,       "sizes",        "Character sizes.",             "size",          json_tblw_size),
    TTABLE (skill_group_table,"skill_groups", "Groups of skills table.",      "skill_group",   json_tblw_skill_group),
    TTABLE (skill_table,      "skills",       "Master skill table.",          "skill",         json_tblw_skill),
    TTABLE (sky_table,        "skies",        "Skies based on the weather.",  "sky",           json_tblw_sky),
    TTABLE (spec_table,       "specs",        "Specialized mobile behavior.", "spec",          json_tblw_spec),
    TTABLE (str_app_table,    "str_app",      "Str apply table.",             "str_app",       json_tblw_str_app),
    TTABLE (sun_table,        "suns",         "Positions of the sun.",        "sun",           json_tblw_sun),
    TTABLE (weapon_table,     "weapons",      "Weapon types and properties.", "weapon",        json_tblw_weapon),
    TTABLE (wear_loc_table,   "wear_locs",    "Wearable item table.",         "wear_loc",      json_tblw_wear_loc),
    TTABLE (wis_app_table,    "wis_app",      "Wis apply table.",             "wis_app",       json_tblw_wis_app),

    /* constant tables that are internal only. */
    TTABLE (affect_bit_table, "affect_bits",  "Affect bit vector types.",       NULL, NULL),
    TTABLE (effect_table,     "effects",      "Damage effects and breaths.",    NULL, NULL),
    TTABLE (furniture_table,  "furnitures",   "Furniture flags for positions.", NULL, NULL),
    TTABLE (map_flags_table,  "map_flags",    "Flags for object mappings.",     NULL, NULL),
    TTABLE (map_lookup_table, "map_lookups",  "Types for object mappings.",     NULL, NULL),
    TTABLE (nanny_table,      "nannies",      "Descriptor 'Nanny' table.",      NULL, NULL),
    TTABLE (obj_map_table,    "obj_maps",     "Obj type-values[] mappings.",    NULL, NULL),
    TTABLE (skill_map_table,  "skill_maps",   "Internal mappings of skills.",   NULL, NULL),
    TTABLE (recycle_table,    "recyclables",  "Recycleable object types.",      NULL, NULL),
    TTABLE (wiznet_table,     "wiznets",      "Wiznet channels.",               NULL, NULL),
    {0}
};

/* for clans */
const CLAN_T clan_table[CLAN_MAX + 1] = {
    /* name, who entry, death-transfer room, independent */
    /* independent should be FALSE if is a real clan */
    {"",      "",           ROOM_VNUM_ALTAR, TRUE},
    {"loner", "[ Loner ] ", ROOM_VNUM_ALTAR, TRUE},
    {"rom",   "[  ROM  ] ", ROOM_VNUM_ALTAR, FALSE},
    {0},
};

const CONDITION_T condition_table[] = {
#ifdef BASEMUD_MORE_PRECISE_CONDITIONS
    {  100, "$1 is in excellent condition." },
    {   90, "$1 has a few scratches." },
    {   80, "$1 has a few bruises." },
    {   70, "$1 has some small wounds and bruises." },
    {   60, "$1 has some large wounds." },
    {   50, "$1 has quite a large few wounds." },
    {   40, "$1 has some big nasty wounds and scratches." },
    {   30, "$1 looks seriously wounded." },
    {   20, "$1 looks pretty hurt." },
    {   10, "$1 is in awful condition." },
    {    1, "$1 is in critical condition." },
#else
    {  100, "$1 is in excellent condition." },
    {   90, "$1 has a few scratches." },
    {   75, "$1 has some small wounds and bruises." },
    {   50, "$1 has quite a few wounds." },
    {   30, "$1 has some big nasty wounds and scratches." },
    {   15, "$1 looks pretty hurt." },
    {    1, "$1 is in awful condition." },
    { -100, "$1 is bleeding to death." },
#endif
    { -999, NULL }
};

/* for position */
const POSITION_T position_table[POS_MAX + 1] = {
    {POS_DEAD,     "dead",             "dead",  "$1 is lying here, DEAD!!",            NULL},
    {POS_MORTAL,   "mortally wounded", "mort",  "$1 is lying here, mortally wounded.", NULL},
    {POS_INCAP,    "incapacitated",    "incap", "$1 is lying here, incapacitated.",    NULL},
    {POS_STUNNED,  "stunned",          "stun",  "$1 is lying here, stunned.",          NULL},
    {POS_SLEEPING, "sleeping",         "sleep", "$1 is sleeping here.",                "$1 is sleeping $2 $3."},
    {POS_RESTING,  "resting",          "rest",  "$1 is resting here.",                 "$1 is resting $2 $3."},
    {POS_SITTING,  "sitting",          "sit",   "$1 is sitting here.",                 "$1 is sitting $2 $3."},
    {POS_FIGHTING, "fighting",         "fight", "$1 is here, fighting $2$3",           NULL},
    {POS_STANDING, "standing",         "stand", "$1 is standing here.",                " is standing $2 $3."},
    {0},
};

/* for sex */
const SEX_T sex_table[SEX_MAX + 1] = {
    {SEX_NEUTRAL, "neutral"},
    {SEX_MALE,    "male"},
    {SEX_FEMALE,  "female"},
    {SEX_EITHER,  "either"},
    {0},
};

/* for sizes */
const SIZE_T size_table[SIZE_MAX_R + 1] = {
    {SIZE_TINY,   "tiny"},
    {SIZE_SMALL,  "small"},
    {SIZE_MEDIUM, "medium"},
    {SIZE_LARGE,  "large"},
    {SIZE_HUGE,   "huge",},
    {SIZE_GIANT,  "giant"},
    {0},
};

/* item type list */
const ITEM_T item_table[ITEM_MAX + 1] = {
    {ITEM_LIGHT,      "light"},
    {ITEM_SCROLL,     "scroll"},
    {ITEM_WAND,       "wand"},
    {ITEM_STAFF,      "staff"},
    {ITEM_WEAPON,     "weapon"},
    {ITEM_UNUSED_1,   "unused_item_1"},
    {ITEM_UNUSED_2,   "unused_item_2"},
    {ITEM_TREASURE,   "treasure"},
    {ITEM_ARMOR,      "armor"},
    {ITEM_POTION,     "potion"},
    {ITEM_CLOTHING,   "clothing"},
    {ITEM_FURNITURE,  "furniture"},
    {ITEM_TRASH,      "trash"},
    {ITEM_UNUSED_3,   "unused_item_3"},
    {ITEM_CONTAINER,  "container"},
    {ITEM_UNUSED_4,   "unused_item_4"},
    {ITEM_DRINK_CON,  "drink"},
    {ITEM_KEY,        "key"},
    {ITEM_FOOD,       "food"},
    {ITEM_MONEY,      "money"},
    {ITEM_UNUSED_5,   "unused_item_5"},
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
    {0}
};

/* weapon selection table */
WEAPON_T weapon_table[WEAPON_MAX + 1] = {
    {WEAPON_SWORD,   "sword",   "sword",   OBJ_VNUM_SCHOOL_SWORD},
    {WEAPON_MACE,    "mace",    "mace",    OBJ_VNUM_SCHOOL_MACE},
    {WEAPON_DAGGER,  "dagger",  "dagger",  OBJ_VNUM_SCHOOL_DAGGER},
    {WEAPON_AXE,     "axe",     "axe",     OBJ_VNUM_SCHOOL_AXE},
    {WEAPON_SPEAR,   "staff",   "staves",  OBJ_VNUM_SCHOOL_STAFF},
    {WEAPON_FLAIL,   "flail",   "flail",   OBJ_VNUM_SCHOOL_FLAIL},
    {WEAPON_WHIP,    "whip",    "whip",    OBJ_VNUM_SCHOOL_WHIP},
    {WEAPON_POLEARM, "polearm", "polearm", OBJ_VNUM_SCHOOL_POLEARM},
    {-1, NULL, 0}
};

const EFFECT_T effect_table[EFFECT_MAX + 1] = {
    {EFFECT_NONE,   "none",   effect_empty},
    {EFFECT_FIRE,   "fire",   effect_fire},
    {EFFECT_COLD,   "cold",   effect_cold},
    {EFFECT_SHOCK,  "shock",  effect_shock},
    {EFFECT_ACID,   "acid",   effect_acid},
    {EFFECT_POISON, "poison", effect_poison},
    {0}
};

const DAM_T dam_table[DAM_MAX + 1] = {
    /* TODO: reference effects by index, not by function directly. */
    {DAM_NONE,      "none",      0,             EFFECT_NONE,   0},
    {DAM_BASH,      "bash",      RES_BASH,      EFFECT_NONE,   0},
    {DAM_PIERCE,    "pierce",    RES_PIERCE,    EFFECT_NONE,   0},
    {DAM_SLASH,     "slash",     RES_SLASH,     EFFECT_NONE,   0},
    {DAM_FIRE,      "fire",      RES_FIRE,      EFFECT_FIRE,   DAM_MAGICAL},
    {DAM_COLD,      "cold",      RES_COLD,      EFFECT_COLD,   DAM_MAGICAL},
    {DAM_LIGHTNING, "lightning", RES_LIGHTNING, EFFECT_SHOCK,  DAM_MAGICAL},
    {DAM_ACID,      "acid",      RES_ACID,      EFFECT_ACID,   DAM_MAGICAL},
    {DAM_POISON,    "poison",    RES_POISON,    EFFECT_POISON, DAM_MAGICAL},
    {DAM_NEGATIVE,  "negative",  RES_NEGATIVE,  EFFECT_NONE,   DAM_MAGICAL},
    {DAM_HOLY,      "holy",      RES_HOLY,      EFFECT_NONE,   DAM_MAGICAL},
    {DAM_ENERGY,    "energy",    RES_ENERGY,    EFFECT_NONE,   DAM_MAGICAL},
    {DAM_MENTAL,    "mental",    RES_MENTAL,    EFFECT_NONE,   DAM_MAGICAL},
    {DAM_DISEASE,   "disease",   RES_DISEASE,   EFFECT_NONE,   DAM_MAGICAL},
    {DAM_DROWNING,  "drowning",  RES_DROWNING,  EFFECT_NONE,   DAM_MAGICAL},
    {DAM_LIGHT,     "light",     RES_LIGHT,     EFFECT_NONE,   DAM_MAGICAL},
    {DAM_OTHER,     "other",     0,             EFFECT_NONE,   DAM_MAGICAL},
    {DAM_HARM,      "harm",      0,             EFFECT_NONE,   DAM_MAGICAL},
    {DAM_CHARM,     "charm",     RES_CHARM,     EFFECT_NONE,   DAM_MAGICAL},
    {DAM_SOUND,     "sound",     RES_SOUND,     EFFECT_NONE,   DAM_MAGICAL},
    {0}
};

/* attack table  -- not very organized :( */
const ATTACK_T attack_table[ATTACK_MAX + 1] = {
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
const RACE_T race_table[RACE_MAX + 1] = {

 /* {name,             act bits,    aff_by bits,                                         off bits,                        imm,                                                    res,                              vuln,                                             form,                          parts} */
    {"unique",         0,           0,                                                   0,                               0,                                                      0,                                0,                                                0,                             0},
    {"bat",            0,           AFF_FLYING | AFF_DARK_VISION,                        OFF_DODGE | OFF_FAST,            0,                                                      0,                                RES_LIGHT,                                        FORMS_MAMMAL,                  PARTS_QUADRUPED | PART_WINGS},
    {"bear",           0,           0,                                              OFF_CRUSH | OFF_DISARM | OFF_BERSERK, 0,                                                      RES_BASH | RES_COLD,              0,                                                FORMS_MAMMAL,                  PARTS_BIPED | PART_CLAWS | PART_FANGS},
    {"cat",            0,           AFF_DARK_VISION,                                     OFF_FAST | OFF_DODGE,            0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_FELINE},
    {"centipede",      0,           AFF_DARK_VISION,                                     0,                               0,                                                      RES_PIERCE | RES_COLD,            RES_BASH,                                         FORMS_BUG | FORM_POISON,       PART_HEAD | PART_LEGS | PART_EYE},
    {"dog",            0,           0,                                                   OFF_FAST,                        0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_CANINE | PART_CLAWS},
    {"doll",           0,           0,                                              0, RES_COLD | RES_POISON | RES_HOLY | RES_NEGATIVE | RES_MENTAL | RES_DISEASE | RES_DROWNING, RES_BASH | RES_LIGHT, RES_SLASH | RES_FIRE | RES_ACID | RES_LIGHTNING | RES_ENERGY, E|J|M|cc,                      PARTS_HUMANOID & ~(PARTS_ALIVE | PART_EAR)},
    {"dragon",         0,           AFF_INFRARED | AFF_FLYING,                           0,                               0,                                                      RES_FIRE | RES_BASH | RES_CHARM,  RES_PIERCE | RES_COLD,                            A|H|Z,                         PARTS_LIZARD | PART_FINGERS | PART_CLAWS | PART_FANGS},
    {"dwarf",          0,           AFF_INFRARED,                                        0,                               0,                                                      RES_POISON | RES_DISEASE,         RES_DROWNING,                                     FORMS_HUMANOID,                PARTS_HUMANOID},
    {"elf",            0,           AFF_INFRARED,                                        0,                               0,                                                      RES_CHARM,                        RES_IRON,                                         FORMS_HUMANOID,                PARTS_HUMANOID},
    {"fido",           0,           0,                                                   OFF_DODGE | ASSIST_RACE,         0,                                                      0,                                RES_MAGIC,                                        FORMS_MAMMAL | FORM_POISON,    PARTS_CANINE | PART_TAIL},
    {"fox",            0,           AFF_DARK_VISION,                                     OFF_FAST | OFF_DODGE,            0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_CANINE | PART_TAIL},
    {"giant",          0,           0,                                                   0,                               0,                                                      RES_FIRE | RES_COLD,              RES_MENTAL | RES_LIGHTNING,                       FORMS_HUMANOID,                PARTS_HUMANOID},
    {"goblin",         0,           AFF_INFRARED,                                        0,                               0,                                                      RES_DISEASE,                      RES_MAGIC,                                        FORMS_HUMANOID,                PARTS_HUMANOID},
    {"hobgoblin",      0,           AFF_INFRARED,                                        0,                               0,                                                      RES_DISEASE | RES_POISON,         0,                                                FORMS_HUMANOID,                PARTS_HUMANOID | PART_TUSKS},
    {"human",          0,           0,                                                   0,                               0,                                                      0,                                0,                                                FORMS_HUMANOID,                PARTS_HUMANOID},
    {"kobold",         0,           AFF_INFRARED,                                        0,                               0,                                                      RES_POISON,                       RES_MAGIC,                                        FORMS_HUMANOID | FORM_POISON,  PARTS_HUMANOID | PART_TAIL},
    {"lizard",         0,           0,                                                   0,                               0,                                                      RES_POISON,                       RES_COLD,                                         A|G|X|cc,                      PARTS_LIZARD},
    {"modron",         0,           AFF_INFRARED,                                     ASSIST_RACE | ASSIST_ALIGN, RES_CHARM | RES_DISEASE | RES_MENTAL | RES_HOLY | RES_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID,   0,                                                FORM_SENTIENT,                 PARTS_HUMANOID & ~(PARTS_ALIVE | PART_FINGERS)},
    {"orc",            0,           AFF_INFRARED,                                        0,                               0,                                                      RES_DISEASE,                      RES_LIGHT,                                        FORMS_HUMANOID,                PARTS_HUMANOID},
    {"pig",            0,           0,                                                   0,                               0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_QUADRUPED},
    {"pixie",          0, AFF_FLYING | AFF_DETECT_GOOD | AFF_DETECT_EVIL | AFF_DETECT_MAGIC, 0,                           0,                                                      0,                                0,                                                FORMS_HUMANOID | FORM_MAGICAL, PARTS_HUMANOID | PART_WINGS},
    {"rabbit",         0,           0,                                                   OFF_DODGE | OFF_FAST,            0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_QUADRUPED},
    {"school monster", MOB_NOALIGN, 0,                                                   0,                               RES_CHARM | RES_SUMMON,                                 0,                                RES_MAGIC,                                        A|M|V,                         PARTS_BIPED | PART_TAIL | PART_CLAWS},
    {"snake",          0,           0,                                                   0,                               0,                                                      RES_POISON,                       RES_COLD,                                         A|G|X|Y|cc,                    PARTS_REPTILE | PART_FANGS},
    {"song bird",      0,           AFF_FLYING,                                          OFF_FAST | OFF_DODGE,            0,                                                      0,                                0,                                                FORMS_BIRD,                    PARTS_BIRD},
    {"troll",          0,           AFF_REGENERATION | AFF_INFRARED | AFF_DETECT_HIDDEN, OFF_BERSERK,                     0,                                                      RES_CHARM | RES_BASH,             RES_FIRE | RES_ACID,                              FORMS_HUMANOID | FORM_POISON,  PARTS_HUMANOID | PART_CLAWS | PART_FANGS},
    {"water fowl",     0,           AFF_SWIM | AFF_FLYING,                               0,                               0,                                                      RES_DROWNING,                     0,                                                FORMS_BIRD,                    PARTS_BIRD},
    {"wolf",           0,           AFF_DARK_VISION,                                     OFF_FAST | OFF_DODGE,            0,                                                      0,                                0,                                                FORMS_MAMMAL,                  PARTS_CANINE | PART_CLAWS | PART_TAIL},
    {"wyvern",         0,           AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,   OFF_BASH | OFF_FAST | OFF_DODGE, RES_POISON,                                             0,                                RES_LIGHT,                                        A|B|G|Z,                       PARTS_LIZARD | PART_FANGS},
    {0}
};

const PC_RACE_T pc_race_table[PC_RACE_MAX + 1] = {
 /* {"race name", short name, points, { class multipliers }, { bonus skills },  { base stats (str,int,wis,dex,con) }, {max}, size}, */
    {"human",  "Human", 0, {100, 100, 100, 100}, {NULL},                         {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, SIZE_MEDIUM, 1},
    {"elf",    " Elf ", 5, {100, 125, 100, 125}, {"sneak", "hide", NULL},        {12, 14, 13, 15, 11}, {16, 20, 18, 21, 15}, SIZE_SMALL,  0},
    {"dwarf",  "Dwarf", 8, {150, 100, 125, 100}, {"berserk", NULL},              {14, 12, 14, 10, 15}, {20, 16, 19, 14, 21}, SIZE_MEDIUM, 0},
    {"giant",  "Giant", 6, {200, 150, 150, 100}, {"bash", "fast healing", NULL}, {16, 11, 13, 11, 14}, {22, 15, 18, 15, 20}, SIZE_LARGE,  0},
#ifdef BASEMUD_RACE_ORC
    {"orc",    " Orc ", 0, {100, 100, 100, 100}, {NULL},                         {13, 13, 13, 13, 13}, {19, 17, 17, 18, 19}, SIZE_MEDIUM, 0},
#endif
#ifdef BASEMUD_RACE_PIXIE
    {"pixie",  "Pixie", 7, {100, 150, 150, 150}, {"dodge", NULL},                {11, 15, 15, 14, 10}, {15, 21, 20, 20, 14}, SIZE_TINY,   0},
#endif
    {0}
};

/* Class table.  */
const CLASS_T class_table[CLASS_MAX + 1] = {
    {"mage",    "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER, {3018, 9618}, 75, 20,   6,  6,  8, TRUE,  "mage basics",    "mage default"},
    {"cleric",  "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,   {3003, 9619}, 75, 20,   2,  7, 10, TRUE,  "cleric basics",  "cleric default"},
    {"thief",   "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER, {3028, 9639}, 75, 20,  -4,  8, 13, FALSE, "thief basics",   "thief default"},
    {"warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,  {3022, 9633}, 75, 20, -10, 11, 15, FALSE, "warrior basics", "warrior default"},
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
const STR_APP_T str_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, tohit, todam, carry, wield */
    { 0, -5, -4,   0,  0}, /* 0 */
    { 1, -4, -3,   3,  1},
    { 2, -3, -2,   6,  2},
    { 3, -3, -1,  10,  3},
    { 4, -2, -1,  25,  4},
    { 5, -2, -1,  55,  5}, /* 5 */
    { 6, -1,  0,  80,  6},
    { 7, -1,  0,  90,  7},
    { 8,  0,  0, 100,  8},
    { 9,  0,  0, 107,  9},
    {10,  0,  0, 115, 10}, /* 10 */
    {11,  0,  0, 123, 11},
    {12,  0,  0, 130, 12},
    {13,  0,  0, 137, 13},
    {14,  0,  1, 143, 14},
    {15,  1,  1, 150, 15}, /* 15 */
    {16,  1,  2, 165, 16},
    {17,  2,  3, 180, 22},
    {18,  2,  3, 200, 25},
    {19,  3,  4, 225, 30},
    {20,  3,  5, 250, 35}, /* 20 */
    {21,  4,  6, 300, 40},
    {22,  4,  6, 350, 45},
    {23,  5,  7, 400, 50},
    {24,  5,  8, 450, 55},
    {25,  6,  9, 500, 60}, /* 25 */

    { -999},
};

const INT_APP_T int_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, learn */
    { 0,  3}, /* 0 */
    { 1,  5},
    { 2,  7},
    { 3,  8},
    { 4,  9},
    { 5, 10}, /* 5 */
    { 6, 11},
    { 7, 12},
    { 8, 13},
    { 9, 15},
    {10, 17}, /* 10 */
    {11, 19},
    {12, 22},
    {13, 25},
    {14, 28},
    {15, 31}, /* 15 */
    {16, 34},
    {17, 37},
    {18, 40},
    {19, 44},
    {20, 49}, /* 20 */
    {21, 55},
    {22, 60},
    {23, 70},
    {24, 80},
    {25, 85}, /* 25 */

    {-999}
};

const WIS_APP_T wis_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, practice */
    { 0, 0}, /* 0 */
    { 1, 0},
    { 2, 0},
    { 3, 0},
    { 4, 0},
    { 5, 1}, /* 5 */
    { 6, 1},
    { 7, 1},
    { 8, 1},
    { 9, 1},
    {10, 1}, /* 10 */
    {11, 1},
    {12, 1},
    {13, 1},
    {14, 1},
    {15, 2}, /* 15 */
    {16, 2},
    {17, 2},
    {18, 3},
    {19, 3},
    {20, 3}, /* 20 */
    {21, 3},
    {22, 4},
    {23, 4},
    {24, 4},
    {25, 5}, /* 25 */

    {-999}
};

const DEX_APP_T dex_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, defensive */
    { 0,  60}, /* 0 */
    { 1,  50},
    { 2,  50},
    { 3,  40},
    { 4,  30},
    { 5,  20}, /* 5 */
    { 6,  10},
    { 7,   0},
    { 8,   0},
    { 9,   0},
    {10,   0}, /* 10 */
    {11,   0},
    {12,   0},
    {13,   0},
    {14,   0},
    {15,  -10}, /* 15 */
    {16,  -15},
    {17,  -20},
    {18,  -30},
    {19,  -40},
    {20,  -50}, /* 20 */
    {21,  -60},
    {22,  -75},
    {23,  -90},
    {24, -105},
    {25, -120}, /* 25 */

    {-999}
};

const CON_APP_T con_app_table[ATTRIBUTE_HIGHEST + 2] = {
    /* stat, hitp, shock */
    { 0, -4, 20}, /* 0 */
    { 1, -3, 25},
    { 2, -2, 30},
    { 3, -2, 35},
    { 4, -1, 40},
    { 5, -1, 45}, /* 5 */
    { 6, -1, 50},
    { 7,  0, 55},
    { 8,  0, 60},
    { 9,  0, 65},
    {10,  0, 70}, /* 10 */
    {11,  0, 75},
    {12,  0, 80},
    {13,  0, 85},
    {14,  0, 88},
    {15,  1, 90}, /* 15 */
    {16,  2, 95},
    {17,  2, 97},
    {18,  3, 99},
    {19,  3, 99},
    {20,  4, 99}, /* 20 */
    {21,  4, 99},
    {22,  5, 99},
    {23,  6, 99},
    {24,  7, 99},
    {25,  8, 99}, /* 25 */

    { -999},
};

/* Liquid properties. */
const LIQ_T liq_table[LIQ_MAX + 1] = {
  /* name                   color         proof, full, thirst, food, serving_size */
    {"water",               "clear",     {0,     1,    10,     0},   16},
    {"beer",                "amber",     {12,    1,    8,      1},   12},
    {"red wine",            "burgundy",  {30,    1,    8,      1},   5 },
    {"ale",                 "brown",     {15,    1,    8,      1},   12},
    {"dark ale",            "dark",      {16,    1,    8,      1},   12},
    {"whisky",              "golden",    {120,   1,    5,      0},   2 },
    {"lemonade",            "pink",      {0,     1,    9,      2},   12},
    {"firebreather",        "boiling",   {190,   0,    4,      0},   2 },
    {"local specialty",     "clear",     {151,   1,    3,      0},   2 },
    {"slime mold juice",    "green",     {0,     2,    -8,     1},   2 },
    {"milk",                "white",     {0,     2,    9,      3},   12},
    {"tea",                 "tan",       {0,     1,    8,      0},   6 },
    {"coffee",              "black",     {0,     1,    8,      0},   6 },
    {"blood",               "red",       {0,     2,    -1,     2},   6 },
    {"salt water",          "clear",     {0,     1,    -2,     0},   1 },
    {"coke",                "brown",     {0,     2,    9,      2},   12},
    {"root beer",           "brown",     {0,     2,    9,      2},   12},
    {"elvish wine",         "green",     {35,    2,    8,      1},   5 },
    {"white wine",          "golden",    {28,    1,    8,      1},   5 },
    {"champagne",           "golden",    {32,    1,    8,      1},   5 },
    {"mead",            "honey-colored", {34,    2,    8,      2},   12},
    {"rose wine",           "pink",      {26,    1,    8,      1},   5 },
    {"benedictine wine",    "burgundy",  {40,    1,    8,      1},   5 },
    {"vodka",               "clear",     {130,   1,    5,      0},   2 },
    {"cranberry juice",     "red",       {0,     1,    9,      2},   12},
    {"orange juice",        "orange",    {0,     2,    9,      3},   12},
    {"absinthe",            "green",     {200,   1,    4,      0},   2 },
    {"brandy",              "golden",    {80,    1,    5,      0},   4 },
    {"aquavit",             "clear",     {140,   1,    5,      0},   2 },
    {"schnapps",            "clear",     {90,    1,    5,      0},   2 },
    {"icewine",             "purple",    {50,    2,    6,      1},   5 },
    {"amontillado",         "burgundy",  {35,    2,    8,      1},   5 },
    {"sherry",              "red",       {38,    2,    7,      1},   5 },
    {"framboise",           "red",       {50,    1,    7,      1},   5 },
    {"rum",                 "amber",     {151,   1,    4,      0},   2 },
    {"cordial",             "clear",     {100,   1,    5,      0},   2 },
    {0}
};

/* The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections. */
#define SLOT(n)    n

#define TI   SKILL_TARGET_IGNORE
#define TCO  SKILL_TARGET_CHAR_OFFENSIVE
#define TCD  SKILL_TARGET_CHAR_DEFENSIVE
#define TCS  SKILL_TARGET_CHAR_SELF
#define TOI  SKILL_TARGET_OBJ_INV
#define TOCD SKILL_TARGET_OBJ_CHAR_DEF
#define TOCO SKILL_TARGET_OBJ_CHAR_OFF

#define PS POS_STANDING
#define PF POS_FIGHTING
#define PR POS_RESTING
#define PP POS_SLEEPING

SKILL_T skill_table[SKILL_MAX + 1] = {
    /* Magic spells. */
    {"reserved",        {{99, 99}, {99, 99}, {99, 99}, {99, 99}}, NULL,                  TI,   PS, SLOT (0),   0,   0,  "",               "", ""},
    {"acid blast",      {{28,  1}, {53,  1}, {35,  2}, {32,  2}}, spell_acid_blast,      TCO,  PF, SLOT (70),  20,  12, "acid blast",     "!Acid Blast!", ""},
    {"armor",           {{ 7,  1}, { 2,  1}, {10,  2}, { 5,  2}}, spell_armor,           TCD,  PS, SLOT (1),   5,   12, "",               "You feel less armored.", ""},
    {"bless",           {{53,  1}, { 7,  1}, {53,  2}, { 8,  2}}, spell_bless,           TOCD, PS, SLOT (3),   5,   12, "",               "You feel less righteous.", "$p's holy aura fades."},
    {"blindness",       {{12,  1}, { 8,  1}, {17,  2}, {15,  2}}, spell_blindness,       TCO,  PF, SLOT (4),   5,   12, "",               "You can see again.", ""},
    {"burning hands",   {{ 7,  1}, {53,  1}, {10,  2}, { 9,  2}}, spell_burning_hands,   TCO,  PF, SLOT (5),   15,  12, "burning hands",  "!Burning Hands!", ""},
    {"call lightning",  {{26,  1}, {18,  1}, {31,  2}, {22,  2}}, spell_call_lightning,  TI,   PF, SLOT (6),   15,  12, "lightning bolt", "!Call Lightning!", ""},
    {"calm",            {{48,  1}, {16,  1}, {50,  2}, {20,  2}}, spell_calm,            TI,   PF, SLOT (509), 30,  12, "",               "You have lost your peace of mind.", ""},
    {"cancellation",    {{18,  1}, {26,  1}, {34,  2}, {34,  2}}, spell_cancellation,    TCD,  PF, SLOT (507), 20,  12, "",               "!cancellation!",""},
    {"cause critical",  {{53,  1}, {13,  1}, {53,  2}, {19,  2}}, spell_cause_critical,  TCO,  PF, SLOT (63),  20,  12, "harmful spell",  "!Cause Critical!", ""},
    {"cause light",     {{53,  1}, { 1,  1}, {53,  2}, { 3,  2}}, spell_cause_light,     TCO,  PF, SLOT (62),  15,  12, "harmful spell",  "!Cause Light!", ""},
    {"cause serious",   {{53,  1}, { 7,  1}, {53,  2}, {10,  2}}, spell_cause_serious,   TCO,  PF, SLOT (64),  17,  12, "harmful spell",  "!Cause Serious!", ""},
    {"chain lightning", {{33,  1}, {53,  1}, {39,  2}, {36,  2}}, spell_chain_lightning, TCO,  PF, SLOT (500), 25,  12, "lightning",      "!Chain Lightning!", ""},
    {"change sex",      {{53,  1}, {53,  1}, {53,  2}, {53,  2}}, spell_change_sex,      TCD,  PF, SLOT (82),  15,  12, "",               "Your body feels familiar again.", ""},
    {"charm person",    {{20,  1}, {53,  1}, {25,  2}, {53,  2}}, spell_charm_person,    TCO,  PS, SLOT (7),   5,   12, "",               "You feel more self-confident.", ""},
    {"chill touch",     {{ 4,  1}, {53,  1}, { 6,  2}, { 6,  2}}, spell_chill_touch,     TCO,  PF, SLOT (8),   15,  12, "chilling touch", "You feel less cold.", ""},
    {"colour spray",    {{16,  1}, {53,  1}, {22,  2}, {20,  2}}, spell_colour_spray,    TCO,  PF, SLOT (10),  15,  12, "colour spray",   "!Colour Spray!", ""},
    {"continual light", {{ 6,  1}, { 4,  1}, { 6,  2}, { 9,  2}}, spell_continual_light, TI,   PS, SLOT (57),  7,   12, "",               "!Continual Light!", ""},
    {"control weather", {{15,  1}, {19,  1}, {28,  2}, {22,  2}}, spell_control_weather, TI,   PS, SLOT (11),  25,  12, "",               "!Control Weather!", ""},
    {"create food",     {{10,  1}, { 5,  1}, {11,  2}, {12,  2}}, spell_create_food,     TI,   PS, SLOT (12),  5,   12, "",               "!Create Food!", ""},
    {"create rose",     {{16,  1}, {11,  1}, {10,  2}, {24,  2}}, spell_create_rose,     TI,   PS, SLOT (511), 30,  12, "",               "!Create Rose!", ""},
    {"create spring",   {{14,  1}, {17,  1}, {23,  2}, {20,  2}}, spell_create_spring,   TI,   PS, SLOT (80),  20,  12, "",               "!Create Spring!", ""},
    {"create water",    {{ 8,  1}, { 3,  1}, {12,  2}, {11,  2}}, spell_create_water,    TOI,  PS, SLOT (13),  5,   12, "",               "!Create Water!", ""},
    {"cure blindness",  {{53,  1}, { 6,  1}, {53,  2}, { 8,  2}}, spell_cure_blindness,  TCD,  PF, SLOT (14),  5,   12, "",               "!Cure Blindness!", ""},
    {"cure critical",   {{53,  1}, {13,  1}, {53,  2}, {19,  2}}, spell_cure_critical,   TCD,  PF, SLOT (15),  20,  12, "",               "!Cure Critical!", ""},
    {"cure disease",    {{53,  1}, {13,  1}, {53,  2}, {14,  2}}, spell_cure_disease,    TCD,  PS, SLOT (501), 20,  12, "",               "!Cure Disease!", ""},
    {"cure light",      {{53,  1}, { 1,  1}, {53,  2}, { 3,  2}}, spell_cure_light,      TCD,  PF, SLOT (16),  10,  12, "",               "!Cure Light!", ""},
    {"cure poison",     {{53,  1}, {14,  1}, {53,  2}, {16,  2}}, spell_cure_poison,     TCD,  PS, SLOT (43),  5,   12, "",               "!Cure Poison!", ""},
    {"cure serious",    {{53,  1}, { 7,  1}, {53,  2}, {10,  2}}, spell_cure_serious,    TCD,  PF, SLOT (61),  15,  12, "",               "!Cure Serious!", ""},
    {"curse",           {{18,  1}, {18,  1}, {26,  2}, {22,  2}}, spell_curse,           TOCO, PF, SLOT (17),  20,  12, "curse",          "The curse wears off.", "$p is no longer impure."},
    {"demonfire",       {{53,  1}, {34,  1}, {53,  2}, {45,  2}}, spell_demonfire,       TCO,  PF, SLOT (505), 20,  12, "torments",       "!Demonfire!", ""},
    {"detect evil",     {{11,  1}, { 4,  1}, {12,  2}, {53,  2}}, spell_detect_evil,     TCS,  PS, SLOT (18),  5,   12, "",               "The red in your vision disappears.", ""},
    {"detect good",     {{11,  1}, { 4,  1}, {12,  2}, {53,  2}}, spell_detect_good,     TCS,  PS, SLOT (513), 5,   12, "",               "The gold in your vision disappears.", ""},
    {"detect hidden",   {{15,  1}, {11,  1}, {12,  2}, {53,  2}}, spell_detect_hidden,   TCS,  PS, SLOT (44),  5,   12, "",               "You feel less aware of your surroundings.", ""},
    {"detect invis",    {{ 3,  1}, { 8,  1}, { 6,  2}, {53,  2}}, spell_detect_invis,    TCS,  PS, SLOT (19),  5,   12, "",               "You no longer see invisible objects.", ""},
    {"detect magic",    {{ 2,  1}, { 6,  1}, { 5,  2}, {53,  2}}, spell_detect_magic,    TCS,  PS, SLOT (20),  5,   12, "",               "The detect magic wears off.", ""},
    {"detect poison",   {{15,  1}, { 7,  1}, { 9,  2}, {53,  2}}, spell_detect_poison,   TOI,  PS, SLOT (21),  5,   12, "",               "!Detect Poison!", ""},
    {"dispel evil",     {{53,  1}, {15,  1}, {53,  2}, {21,  2}}, spell_dispel_evil,     TCO,  PF, SLOT (22),  15,  12, "dispel evil",    "!Dispel Evil!", ""},
    {"dispel good",     {{53,  1}, {15,  1}, {53,  2}, {21,  2}}, spell_dispel_good,     TCO,  PF, SLOT (512), 15,  12, "dispel good",    "!Dispel Good!", ""},
    {"dispel magic",    {{16,  1}, {24,  1}, {30,  2}, {30,  2}}, spell_dispel_magic,    TCO,  PF, SLOT (59),  15,  12, "",               "!Dispel Magic!", ""},
    {"earthquake",      {{53,  1}, {10,  1}, {53,  2}, {14,  2}}, spell_earthquake,      TI,   PF, SLOT (23),  15,  12, "earthquake",     "!Earthquake!", ""},
    {"enchant armor",   {{16,  2}, {53,  2}, {53,  4}, {53,  4}}, spell_enchant_armor,   TOI,  PS, SLOT (510), 100, 24, "",               "!Enchant Armor!", ""},
    {"enchant weapon",  {{17,  2}, {53,  2}, {53,  4}, {53,  4}}, spell_enchant_weapon,  TOI,  PS, SLOT (24),  100, 24, "",               "!Enchant Weapon!", ""},
    {"energy drain",    {{19,  1}, {22,  1}, {26,  2}, {23,  2}}, spell_energy_drain,    TCO,  PF, SLOT (25),  35,  12, "energy drain",   "!Energy Drain!", ""},
    {"faerie fire",     {{ 6,  1}, { 3,  1}, { 5,  2}, { 8,  2}}, spell_faerie_fire,     TCO,  PF, SLOT (72),  5,   12, "faerie fire",    "The pink aura around you fades away.", ""},
    {"faerie fog",      {{14,  1}, {21,  1}, {16,  2}, {24,  2}}, spell_faerie_fog,      TI,   PS, SLOT (73),  12,  12, "faerie fog",     "!Faerie Fog!", ""},
    {"farsight",        {{14,  1}, {16,  1}, {16,  2}, {53,  2}}, spell_farsight,        TI,   PS, SLOT (521), 36,  20, "farsight",       "!Farsight!", ""},
    {"fireball",        {{22,  1}, {53,  1}, {30,  2}, {26,  2}}, spell_fireball,        TCO,  PF, SLOT (26),  15,  12, "fireball",       "!Fireball!", ""},
    {"fireproof",       {{13,  1}, {12,  1}, {19,  2}, {18,  2}}, spell_fireproof,       TOI,  PS, SLOT (523), 10,  12, "",               "", "$p's protective aura fades."},
    {"flamestrike",     {{53,  1}, {20,  1}, {53,  2}, {27,  2}}, spell_flamestrike,     TCO,  PF, SLOT (65),  20,  12, "flamestrike",    "!Flamestrike!", ""},
    {"fly",             {{10,  1}, {18,  1}, {20,  2}, {22,  2}}, spell_fly,             TCD,  PS, SLOT (56),  10,  18, "",               "You slowly float to the ground.", ""},
    {"floating disc",   {{ 4,  1}, {10,  1}, { 7,  2}, {16,  2}}, spell_floating_disc,   TI,   PS, SLOT (522), 40,  24, "",               "!Floating disc!", ""},
    {"frenzy",          {{53,  1}, {24,  1}, {53,  2}, {26,  2}}, spell_frenzy,          TCD,  PS, SLOT (504), 30,  24, "",               "Your rage ebbs.", ""},
    {"gate",            {{27,  1}, {17,  1}, {32,  2}, {28,  2}}, spell_gate,            TI,   PF, SLOT (83),  80,  12, "",               "!Gate!", ""},
    {"giant strength",  {{11,  1}, {53,  1}, {22,  2}, {20,  2}}, spell_giant_strength,  TCD,  PS, SLOT (39),  20,  12, "",               "You feel weaker.", ""},
    {"harm",            {{53,  1}, {23,  1}, {53,  2}, {28,  2}}, spell_harm,            TCO,  PF, SLOT (27),  35,  12, "harmful spell",  "!Harm!,        " ""},
    {"haste",           {{21,  1}, {53,  1}, {26,  2}, {29,  2}}, spell_haste,           TCD,  PF, SLOT (502), 30,  12, "",               "You feel yourself slow down.", ""},
    {"heal",            {{53,  1}, {21,  1}, {33,  2}, {30,  2}}, spell_heal,            TCD,  PF, SLOT (28),  50,  12, "",               "!Heal!", ""},
    {"heat metal",      {{53,  1}, {16,  1}, {53,  2}, {23,  2}}, spell_heat_metal,      TCO,  PF, SLOT (516), 25,  18, "burn",           "!Heat Metal!", ""},
    {"holy word",       {{53,  2}, {36,  2}, {53,  4}, {42,  4}}, spell_holy_word,       TI,   PF, SLOT (506), 200, 24, "divine wrath",   "!Holy Word!", ""},
    {"identify",        {{15,  1}, {16,  1}, {18,  2}, {53,  2}}, spell_identify,        TOI,  PS, SLOT (53),  12,  24, "",               "!Identify!", ""},
    {"infravision",     {{ 9,  1}, {13,  1}, {10,  2}, {16,  2}}, spell_infravision,     TCD,  PS, SLOT (77),  5,   18, "",               "You no longer see in the dark.", ""},
    {"invisibility",    {{ 5,  1}, {53,  1}, { 9,  2}, {53,  2}}, spell_invis,           TOCD, PS, SLOT (29),  5,   12, "",               "You are no longer invisible.", "$p fades into view."},
    {"know alignment",  {{12,  1}, { 9,  1}, {20,  2}, {53,  2}}, spell_know_alignment,  TCD,  PF, SLOT (58),  9,   12, "",               "!Know Alignment!", ""},
    {"lightning bolt",  {{13,  1}, {23,  1}, {18,  2}, {16,  2}}, spell_lightning_bolt,  TCO,  PF, SLOT (30),  15,  12, "lightning bolt", "!Lightning Bolt!", ""},
    {"locate object",   {{ 9,  1}, {15,  1}, {11,  2}, {53,  2}}, spell_locate_object,   TI,   PS, SLOT (31),  20,  18, "",               "!Locate Object!", ""},
    {"magic missile",   {{ 1,  1}, {53,  1}, { 2,  2}, { 2,  2}}, spell_magic_missile,   TCO,  PF, SLOT (32),  15,  12, "magic missile",  "!Magic Missile!", ""},
    {"mass healing",    {{53,  2}, {38,  2}, {53,  4}, {46,  4}}, spell_mass_healing,    TI,   PS, SLOT (508), 100, 36, "",               "!Mass Healing!", ""},
    {"mass invis",      {{22,  1}, {25,  1}, {31,  2}, {53,  2}}, spell_mass_invis,      TI,   PS, SLOT (69),  20,  24, "",               "You are no longer invisible.", ""},
    {"nexus",           {{40,  2}, {35,  2}, {50,  4}, {45,  4}}, spell_nexus,           TI,   PS, SLOT (520), 150, 36, "",               "!Nexus!", ""},
    {"pass door",       {{24,  1}, {32,  1}, {25,  2}, {37,  2}}, spell_pass_door,       TCS,  PS, SLOT (74),  20,  12, "",               "You feel solid again.", ""},
    {"plague",          {{23,  1}, {17,  1}, {36,  2}, {26,  2}}, spell_plague,          TCO,  PF, SLOT (503), 20,  12, "sickness",       "Your sores vanish.", ""},
    {"poison",          {{17,  1}, {12,  1}, {15,  2}, {21,  2}}, spell_poison,          TOCO, PF, SLOT (33),  10,  12, "poison",         "You feel less sick.", "The poison on $p dries up."},
    {"portal",          {{35,  2}, {30,  2}, {45,  4}, {40,  4}}, spell_portal,          TI,   PS, SLOT (519), 100, 24, "",               "!Portal!", ""},
    {"protection evil", {{12,  1}, { 9,  1}, {17,  2}, {11,  2}}, spell_protection_evil, TCS,  PS, SLOT (34),  5,   12, "",               "You feel less protected.", ""},
    {"protection good", {{12,  1}, { 9,  1}, {17,  2}, {11,  2}}, spell_protection_good, TCS,  PS, SLOT (514), 5,   12, "",               "You feel less protected.", ""},
    {"ray of truth",    {{53,  1}, {35,  1}, {53,  2}, {47,  2}}, spell_ray_of_truth,    TCO,  PF, SLOT (518), 20,  12, "ray of truth",   "!Ray of Truth!", ""},
    {"recharge",        {{ 9,  1}, {53,  1}, {53,  2}, {53,  2}}, spell_recharge,        TOI,  PS, SLOT (517), 60,  24, "",               "!Recharge!", ""},
    {"refresh",         {{ 8,  1}, { 5,  1}, {12,  2}, { 9,  2}}, spell_refresh,         TCD,  PS, SLOT (81),  12,  18, "refresh",        "!Refresh!", ""},
    {"remove curse",    {{53,  1}, {18,  1}, {53,  2}, {22,  2}}, spell_remove_curse,    TOCD, PS, SLOT (35),  5,   12, "",               "!Remove Curse!", ""},
    {"sanctuary",       {{36,  1}, {20,  1}, {42,  2}, {30,  2}}, spell_sanctuary,       TCD,  PS, SLOT (36),  75,  12, "",               "The white aura around your body fades.", ""},
    {"shield",          {{20,  1}, {35,  1}, {35,  2}, {40,  2}}, spell_shield,          TCD,  PS, SLOT (67),  12,  18, "",               "Your force shield shimmers then fades away.", ""},
    {"shocking grasp",  {{10,  1}, {53,  1}, {14,  2}, {13,  2}}, spell_shocking_grasp,  TCO,  PF, SLOT (53),  15,  12, "shocking grasp", "!Shocking Grasp!", ""},
    {"sleep",           {{10,  1}, {53,  1}, {11,  2}, {53,  2}}, spell_sleep,           TCO,  PS, SLOT (38),  15,  12, "",               "You feel less tired.", ""},
    {"slow",            {{23,  1}, {30,  1}, {29,  2}, {32,  2}}, spell_slow,            TCO,  PF, SLOT (515), 30,  12, "",               "You feel yourself speed up.", ""},
    {"stone skin",      {{25,  1}, {40,  1}, {40,  2}, {45,  2}}, spell_stone_skin,      TCS,  PS, SLOT (66),  12,  18, "",               "Your skin feels soft again.", ""},
    {"summon",          {{24,  1}, {12,  1}, {29,  2}, {22,  2}}, spell_summon,          TI,   PS, SLOT (40),  50,  12, "",               "!Summon!", ""},
    {"teleport",        {{13,  1}, {22,  1}, {25,  2}, {36,  2}}, spell_teleport,        TCS,  PF, SLOT (2),   35,  12, "",               "!Teleport!", ""},
    {"ventriloquate",   {{ 1,  1}, {53,  1}, { 2,  2}, {53,  2}}, spell_ventriloquate,   TI,   PS, SLOT (41),  5,   12, "",               "!Ventriloquate!", ""},
    {"weaken",          {{11,  1}, {14,  1}, {16,  2}, {17,  2}}, spell_weaken,          TCO,  PF, SLOT (68),  20,  12, "spell",          "You feel stronger.", ""},
    {"word of recall",  {{32,  1}, {28,  1}, {40,  2}, {30,  2}}, spell_word_of_recall,  TCS,  PR, SLOT (42),  5,   12, "",               "!Word of Recall!", ""},

    /* Dragon breath */
    {"acid breath",     {{31,  1}, {32,  1}, {33,  2}, {34,  2}}, spell_acid_breath,     TCO,  PF, SLOT (200), 100, 24, "blast of acid",  "!Acid Breath!", ""},
    {"fire breath",     {{40,  1}, {45,  1}, {50,  2}, {51,  2}}, spell_fire_breath,     TCO,  PF, SLOT (201), 200, 24, "blast of flame", "The smoke leaves your eyes.", ""},
    {"frost breath",    {{34,  1}, {36,  1}, {38,  2}, {40,  2}}, spell_frost_breath,    TCO,  PF, SLOT (202), 125, 24, "blast of frost", "!Frost Breath!", ""},
    {"gas breath",      {{39,  1}, {43,  1}, {47,  2}, {50,  2}}, spell_gas_breath,      TI,   PF, SLOT (203), 175, 24, "blast of gas",   "!Gas Breath!", ""},
    {"lightning breath",{{37,  1}, {40,  1}, {43,  2}, {46,  2}}, spell_lightning_breath,TCO,  PF, SLOT (204), 150, 24, "blast of lightning", "!Lightning Breath!", ""},

    /* Spells for mega1.are from Glop/Erkenbrand. */
    {"general purpose", {{53,  0}, {53,  0}, {53,  0}, {53,  0}}, spell_general_purpose, TCO,  PF, SLOT (401), 0,   12, "general purpose ammo","!General Purpose Ammo!", ""},
    {"high explosive",  {{53,  0}, {53,  0}, {53,  0}, {53,  0}}, spell_high_explosive,  TCO,  PF, SLOT (402), 0,   12, "high explosive ammo", "!High Explosive Ammo!", ""},

    /* combat and weapons skills */
    {"axe",             {{ 1,  6}, { 1,  6}, { 1,  5}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Axe!", ""},
    {"dagger",          {{ 1,  2}, { 1,  3}, { 1,  2}, { 1,  2}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Dagger!", ""},
    {"flail",           {{ 1,  6}, { 1,  3}, { 1,  6}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Flail!", ""},
    {"mace",            {{ 1,  5}, { 1,  2}, { 1,  3}, { 1,  3}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Mace!", ""},
    {"polearm",         {{ 1,  6}, { 1,  6}, { 1,  6}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Polearm!", ""},
    {"shield block",    {{ 1,  6}, { 1,  4}, { 1,  6}, { 1,  2}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Shield!", ""},
    {"spear",           {{ 1,  4}, { 1,  4}, { 1,  4}, { 1,  3}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Spear!", ""},
    {"sword",           {{ 1,  5}, { 1,  6}, { 1,  3}, { 1,  2}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Sword!", ""},
    {"whip",            {{ 1,  6}, { 1,  5}, { 1,  5}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Whip!", ""},
    {"backstab",        {{53,  0}, {53,  0}, { 1,  5}, {53,  0}}, spell_null,            TI,   PS, SLOT (0),   0,   24, "backstab",       "!Backstab!", ""},
    {"bash",            {{53,  0}, {53,  0}, {53,  0}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   24, "bash",           "!Bash!", ""},
    {"berserk",         {{53,  0}, {53,  0}, {53,  0}, {18,  5}}, spell_null,            TI,   PF, SLOT (0),   0,   24, "",               "You feel your pulse slow down.", ""},
    {"dirt kicking",    {{53,  0}, {53,  0}, { 3,  4}, { 3,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   24, "kicked dirt",    "You rub the dirt out of your eyes.", ""},
    {"disarm",          {{53,  0}, {53,  0}, {12,  6}, {11,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   24, "",               "!Disarm!", ""},
    {"dodge",           {{20,  8}, {22,  8}, { 1,  4}, {13,  6}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Dodge!", ""},
    {"enhanced damage", {{45, 10}, {30,  9}, {25,  5}, { 1,  3}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Enhanced Damage!", ""},
    {"envenom",         {{53,  0}, {53,  0}, {10,  4}, {53,  0}}, spell_null,            TI,   PR, SLOT (0),   0,   36, "",               "!Envenom!", ""},
    {"hand to hand",    {{25,  8}, {10,  5}, {15,  6}, { 6,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Hand to Hand!", ""},
    {"kick",            {{53,  0}, {12,  4}, {14,  6}, { 8,  3}}, spell_null,            TCO,  PF, SLOT (0),   0,   12, "kick",           "!Kick!", ""},
    {"parry",           {{22,  8}, {20,  8}, {13,  6}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Parry!", ""},
    {"rescue",          {{53,  0}, {53,  0}, {53,  0}, { 1,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   12, "",               "!Rescue!", ""},
    {"trip",            {{53,  0}, {53,  0}, { 1,  4}, {15,  8}}, spell_null,            TI,   PF, SLOT (0),   0,   24, "trip",           "!Trip!", ""},
    {"second attack",   {{30, 10}, {24,  8}, {12,  5}, { 5,  3}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Second Attack!", ""},
    {"third attack",    {{53,  0}, {53,  0}, {24, 10}, {12,  4}}, spell_null,            TI,   PF, SLOT (0),   0,   0,  "",               "!Third Attack!", ""},

    /* non-combat skills */
    {"fast healing",    {{15,  8}, { 9,  5}, {16,  6}, { 6,  4}}, spell_null,            TI,   PP, SLOT (0),   0,   0,  "",               "!Fast Healing!", ""},
    {"haggle",          {{ 7,  5}, {18,  8}, { 1,  3}, {14,  6}}, spell_null,            TI,   PR, SLOT (0),   0,   0,  "",               "!Haggle!", ""},
    {"hide",            {{53,  0}, {53,  0}, { 1,  4}, {12,  6}}, spell_null,            TI,   PR, SLOT (0),   0,   12, "",               "!Hide!", ""},
    {"lore",            {{ 5,  6}, { 6,  8}, { 7, 10}, { 8, 12}}, spell_null,            TI,   PR, SLOT (0),   0,   36, "",               "!Lore!", ""},
    {"meditation",      {{ 6,  5}, { 6,  5}, {15,  8}, {15,  8}}, spell_null,            TI,   PP, SLOT (0),   0,   0,  "",               "Meditation", ""},
    {"peek",            {{ 8,  5}, {21,  7}, { 1,  3}, {14,  6}}, spell_null,            TI,   PS, SLOT (0),   0,   0,  "",               "!Peek!", ""},
    {"pick lock",       {{25,  8}, {25,  8}, { 7,  4}, {25,  8}}, spell_null,            TI,   PS, SLOT (0),   0,   12, "",               "!Pick!", ""},
    {"sneak",           {{53,  0}, {53,  0}, { 4,  4}, {10,  6}}, spell_null,            TI,   PS, SLOT (0),   0,   12, "",               "You no longer feel stealthy.", ""},
    {"steal",           {{53,  0}, {53,  0}, { 5,  4}, {53,  0}}, spell_null,            TI,   PS, SLOT (0),   0,   24, "",               "!Steal!", ""},
    {"scrolls",         {{ 1,  2}, { 1,  3}, { 1,  5}, { 1,  8}}, spell_null,            TI,   PS, SLOT (0),   0,   24, "",               "!Scrolls!", ""},
    {"staves",          {{ 1,  2}, { 1,  3}, { 1,  5}, { 1,  8}}, spell_null,            TI,   PS, SLOT (0),   0,   12, "",               "!Staves!", ""},
    {"wands",           {{ 1,  2}, { 1,  3}, { 1,  5}, { 1,  8}}, spell_null,            TI,   PS, SLOT (0),   0,   12, "",               "!Wands!", ""},
    {"recall",          {{ 1,  2}, { 1,  2}, { 1,  2}, { 1,  2}}, spell_null,            TI,   PS, SLOT (0),   0,   12, "",               "!Recall!", ""},

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

const SKILL_GROUP_T skill_group_table[SKILL_GROUP_MAX + 1] = {
    {"rom basics",      {{ 0}, { 0}, { 0}, { 0}}, {"scrolls", "staves", "wands", "recall"}},
    {"mage basics",     {{ 0}, {-1}, {-1}, {-1}}, {"dagger"}},
    {"cleric basics",   {{-1}, { 0}, {-1}, {-1}}, {"mace"}},
    {"thief basics",    {{-1}, {-1}, { 0}, {-1}}, {"dagger", "steal"}},
    {"warrior basics",  {{-1}, {-1}, {-1}, { 0}}, {"sword", "second attack"}},
    {"mage default",    {{40}, {-1}, {-1}, {-1}}, {"lore", "beguiling", "combat", "detection", "enhancement", "illusion", "maladictions", "protective", "transportation", "weather"}},
    {"cleric default",  {{-1}, {40}, {-1}, {-1}}, {"flail", "attack", "creation", "curative", "benedictions", "detection", "healing", "maladictions", "protective", "shield block", "transportation", "weather"}},
    {"thief default",   {{-1}, {-1}, {40}, {-1}}, {"mace", "sword", "backstab", "disarm", "dodge", "second attack", "trip", "hide", "peek", "pick lock", "sneak"}},
    {"warrior default", {{-1}, {-1}, {-1}, {40}}, {"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", "parry", "rescue", "third attack"}},
    {"weaponsmaster",   {{40}, {40}, {40}, {20}}, {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip"}},
    {"attack",          {{-1}, { 5}, {-1}, { 8}}, {"demonfire", "dispel evil", "dispel good", "earthquake", "flamestrike", "heat metal", "ray of truth"}},
    {"beguiling",       {{ 4}, {-1}, { 6}, {-1}}, {"calm", "charm person", "sleep"}},
    {"benedictions",    {{-1}, { 4}, {-1}, { 8}}, {"bless", "calm", "frenzy", "holy word", "remove curse"}},
    {"combat",          {{ 6}, {-1}, {10}, { 9}}, {"acid blast", "burning hands", "chain lightning", "chill touch", "colour spray", "fireball", "lightning bolt", "magic missile", "shocking grasp"}},
    {"creation",        {{ 4}, { 4}, { 8}, { 8}}, {"continual light", "create food", "create spring", "create water", "create rose", "floating disc"}},
    {"curative",        {{-1}, { 4}, {-1}, { 8}}, {"cure blindness", "cure disease", "cure poison"}},
    {"detection",       {{ 4}, { 3}, { 6}, {-1}}, {"detect evil", "detect good", "detect hidden", "detect invis", "detect magic", "detect poison", "farsight", "identify", "know alignment", "locate object"}},
    {"draconian",       {{ 8}, {-1}, {-1}, {-1}}, {"acid breath", "fire breath", "frost breath", "gas breath", "lightning breath"}},
    {"enchantment",     {{ 6}, {-1}, {-1}, {-1}}, {"enchant armor", "enchant weapon", "fireproof", "recharge"}},
    {"enhancement",     {{ 5}, {-1}, { 9}, { 9}}, {"giant strength", "haste", "infravision", "refresh"}},
    {"harmful",         {{-1}, { 3}, {-1}, { 6}}, {"cause critical", "cause light", "cause serious", "harm"}},
    {"healing",         {{-1}, { 3}, {-1}, { 6}}, {"cure critical", "cure light", "cure serious", "heal", "mass healing", "refresh"}},
    {"illusion",        {{ 4}, {-1}, { 7}, {-1}}, {"invisibility", "mass invis", "ventriloquate"}},
    {"maladictions",    {{ 5}, { 5}, { 9}, { 9}}, {"blindness", "change sex", "curse", "energy drain", "plague", "poison", "slow", "weaken"}},
    {"protective",      {{ 4}, { 4}, { 7}, { 8}}, {"armor", "cancellation", "dispel magic", "fireproof", "protection evil", "protection good", "sanctuary", "shield", "stone skin"}},
    {"transportation",  {{ 4}, { 4}, { 8}, { 9}}, {"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport", "word of recall"}},
    {"weather",         {{ 4}, { 4}, { 8}, { 8}}, {"call lightning", "control weather", "faerie fire", "faerie fog", "lightning bolt"}},
    {0}
};

/* Globals. */
SKILL_MAP_T skill_map_table[SKILL_MAP_MAX + 1] = {
    {SKILL_MAP_BACKSTAB,        "backstab"},
    {SKILL_MAP_DODGE,           "dodge"},
    {SKILL_MAP_ENVENOM,         "envenom"},
    {SKILL_MAP_HIDE,            "hide"},
    {SKILL_MAP_PEEK,            "peek"},
    {SKILL_MAP_PICK_LOCK,       "pick lock"},
    {SKILL_MAP_SNEAK,           "sneak"},
    {SKILL_MAP_STEAL,           "steal"},

    {SKILL_MAP_DISARM,          "disarm"},
    {SKILL_MAP_ENHANCED_DAMAGE, "enhanced damage"},
    {SKILL_MAP_KICK,            "kick"},
    {SKILL_MAP_PARRY,           "parry"},
    {SKILL_MAP_RESCUE,          "rescue"},
    {SKILL_MAP_SECOND_ATTACK,   "second attack"},
    {SKILL_MAP_THIRD_ATTACK,    "third attack"},

    {SKILL_MAP_BLINDNESS,       "blindness"},
    {SKILL_MAP_CHARM_PERSON,    "charm person"},
    {SKILL_MAP_CURSE,           "curse"},
    {SKILL_MAP_INVIS,           "invisibility"},
    {SKILL_MAP_MASS_INVIS,      "mass invis"},
    {SKILL_MAP_POISON,          "poison"},
    {SKILL_MAP_PLAGUE,          "plague"},
    {SKILL_MAP_SLEEP,           "sleep"},
    {SKILL_MAP_SANCTUARY,       "sanctuary"},
    {SKILL_MAP_FLY,             "fly"},

    {SKILL_MAP_AXE,             "axe"},
    {SKILL_MAP_DAGGER,          "dagger"},
    {SKILL_MAP_FLAIL,           "flail"},
    {SKILL_MAP_MACE,            "mace"},
    {SKILL_MAP_POLEARM,         "polearm"},
    {SKILL_MAP_SHIELD_BLOCK,    "shield block"},
    {SKILL_MAP_SPEAR,           "spear"},
    {SKILL_MAP_SWORD,           "sword"},
    {SKILL_MAP_WHIP,            "whip"},

    {SKILL_MAP_BASH,            "bash"},
    {SKILL_MAP_BERSERK,         "berserk"},
    {SKILL_MAP_DIRT,            "dirt kicking"},
    {SKILL_MAP_HAND_TO_HAND,    "hand to hand"},
    {SKILL_MAP_TRIP,            "trip"},

    {SKILL_MAP_FAST_HEALING,    "fast healing"},
    {SKILL_MAP_HAGGLE,          "haggle"},
    {SKILL_MAP_LORE,            "lore"},
    {SKILL_MAP_MEDITATION,      "meditation"},

    {SKILL_MAP_SCROLLS,         "scrolls"},
    {SKILL_MAP_STAVES,          "staves"},
    {SKILL_MAP_WANDS,           "wands"},
    {SKILL_MAP_RECALL,          "recall"},
    {SKILL_MAP_FRENZY,          "frenzy"},

    {0}
};

const SECTOR_T sector_table[SECT_MAX + 1] = {
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

const NANNY_HANDLER_T nanny_table[NANNY_MAX + 1] = {
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

const FURNITURE_BITS_T furniture_table[] = {
    {POS_STANDING, "standing", STAND_AT, STAND_ON, STAND_IN},
    {POS_SITTING,  "sitting",  SIT_AT,   SIT_ON,   SIT_IN},
    {POS_RESTING,  "resting",  REST_AT,  REST_ON,  REST_IN},
    {POS_SLEEPING, "sleeping", SLEEP_AT, SLEEP_ON, SLEEP_IN},
    {-1, NULL, 0, 0}
};

const MAP_LOOKUP_TABLE_T map_lookup_table[] = {
    {MAP_LOOKUP_WEAPON_TYPE, "weapon_type", NULL},
    {MAP_LOOKUP_ATTACK_TYPE, "attack_type", NULL},
    {MAP_LOOKUP_LIQUID,      "liquid",      NULL},
    {MAP_LOOKUP_SKILL,       "skill",       NULL},
    {-1, NULL, NULL},
};

const MAP_LOOKUP_TABLE_T map_flags_table[] = {
    {MAP_FLAGS_WEAPON,    "weapon",    weapon_flags},
    {MAP_FLAGS_CONT,      "container", container_flags},
    {MAP_FLAGS_FURNITURE, "furniture", furniture_flags},
    {MAP_FLAGS_EXIT,      "exit",      exit_flags},
    {MAP_FLAGS_GATE,      "gate",      gate_flags},
    {-1, NULL, NULL},
};

const OBJ_MAP_T obj_map_table[] = {
    {ITEM_WEAPON, {
        {0, -1, "weapon_type", MAP_LOOKUP, MAP_LOOKUP_WEAPON_TYPE},
        {1,  0, "dice_num",    MAP_INTEGER, 0},
        {2,  0, "dice_size",   MAP_INTEGER, 0},
        {3, -1, "attack_type", MAP_LOOKUP, MAP_LOOKUP_ATTACK_TYPE},
        {4,  0, "flags",       MAP_FLAGS, MAP_FLAGS_WEAPON},
    }},
    {ITEM_CONTAINER, {
        {0,  0, "capacity",    MAP_INTEGER, 0},
        {1,  0, "flags",       MAP_FLAGS, MAP_FLAGS_CONT},
        {2,  0, "key",         MAP_INTEGER, 0},
        {3,  0, "max_weight",  MAP_INTEGER, 0},
        {4,  0, "weight_mult", MAP_INTEGER, 0}
    }},
    {ITEM_DRINK_CON, {
        {0,  0, "capacity",    MAP_INTEGER, 0},
        {1,  0, "filled",      MAP_INTEGER, 0},
        {2, -1, "liquid",      MAP_LOOKUP, MAP_LOOKUP_LIQUID},
        {3,  0, "poisoned",    MAP_BOOLEAN, 0},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_FOUNTAIN, {
        {0,  0, "capacity",    MAP_INTEGER, 0},
        {1,  0, "filled",      MAP_INTEGER, 0},
        {2, -1, "liquid",      MAP_LOOKUP, MAP_LOOKUP_LIQUID},
        {3,  0, "poisoned",    MAP_BOOLEAN, 0},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_WAND, {
        {0,  0, "level",       MAP_INTEGER, 0},
        {1,  0, "recharge",    MAP_INTEGER, 0},
        {2,  0, "charges",     MAP_INTEGER, 0},
        {3, -1, "skill",       MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_STAFF, {
        {0,  0, "level",       MAP_INTEGER, 0},
        {1,  0, "recharge",    MAP_INTEGER, 0},
        {2,  0, "charges",     MAP_INTEGER, 0},
        {3, -1, "skill",       MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_FOOD, {
        {0,  0, "hunger",      MAP_INTEGER, 0},
        {1,  0, "fullness",    MAP_INTEGER, 0},
        {2,  0, NULL,          MAP_IGNORE, 0},
        {3,  0, "poisoned",    MAP_BOOLEAN, 0},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_MONEY, {
        {0,  0, "silver",      MAP_INTEGER, 0},
        {1,  0, "gold",        MAP_INTEGER, 0},
        {2,  0, NULL,          MAP_IGNORE, 0},
        {3,  0, NULL,          MAP_IGNORE, 0},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_ARMOR, {
        {0,  0, "vs_pierce",   MAP_INTEGER, 0},
        {1,  0, "vs_bash",     MAP_INTEGER, 0},
        {2,  0, "vs_slash",    MAP_INTEGER, 0},
        {3,  0, "vs_magic",    MAP_INTEGER, 0},
        {4,  0, NULL,          MAP_IGNORE, 0}
    }},
    {ITEM_POTION, {
        {0,  0, "level",       MAP_INTEGER, 0},
        {1, -1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, -1, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, -1, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, -1, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_PILL, {
        {0,  0, "level",       MAP_INTEGER, 0},
        {1, -1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, -1, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, -1, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, -1, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_SCROLL, {
        {0,  0, "level",       MAP_INTEGER, 0},
        {1, -1, "skill1",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {2, -1, "skill2",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {3, -1, "skill3",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
        {4, -1, "skill4",      MAP_LOOKUP, MAP_LOOKUP_SKILL},
    }},
    {ITEM_MAP, {
        {0,  0, "persist",     MAP_BOOLEAN, 0},
        {1,  0, NULL,          MAP_IGNORE, 0},
        {2,  0, NULL,          MAP_IGNORE, 0},
        {3,  0, NULL,          MAP_IGNORE, 0},
        {4,  0, NULL,          MAP_IGNORE, 0},
    }},
    {ITEM_FURNITURE, {
        {0,  0, "max_people",  MAP_INTEGER, 0},
        {1,  0, "max_weight",  MAP_INTEGER, 0},
        {2,  0, "flags",       MAP_FLAGS, MAP_FLAGS_FURNITURE},
        {3,  0, "heal_rate",   MAP_INTEGER, 0},
        {4,  0, "mana_rate",   MAP_INTEGER, 0},
    }},
    {ITEM_LIGHT, {
        {0,  0, NULL,          MAP_IGNORE, 0},
        {1,  0, NULL,          MAP_IGNORE, 0},
        {2,  0, "duration",    MAP_INTEGER, 0},
        {3,  0, NULL,          MAP_IGNORE, 0},
        {4,  0, NULL,          MAP_IGNORE, 0},
    }},
    {ITEM_PORTAL, {
        {0,  0, "charges",     MAP_INTEGER, 0},
        {1,  0, "exit_flags",  MAP_FLAGS, MAP_FLAGS_EXIT},
        {2,  0, "gate_flags",  MAP_FLAGS, MAP_FLAGS_GATE},
        {3,  0, "to_vnum",     MAP_INTEGER, 0},
        {4,  0, "key",         MAP_INTEGER, 0},
    }},

    #define OBJ_MAP_NO_VALUES(type) \
        { type, { \
            { 0, 0, NULL, MAP_IGNORE, 0 }, { 1, 0, NULL, MAP_IGNORE, 0 }, \
            { 2, 0, NULL, MAP_IGNORE, 0 }, { 3, 0, NULL, MAP_IGNORE, 0 }, \
            { 4, 0, NULL, MAP_IGNORE, 0 } \
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
const DOOR_T door_table[DIR_MAX + 1] = {
    {DIR_NORTH, "north", "from the north", "to the north", DIR_SOUTH, "N"},
    {DIR_EAST,  "east",  "from the east",  "to the east",  DIR_WEST,  "E"},
    {DIR_SOUTH, "south", "from the south", "to the south", DIR_NORTH, "S"},
    {DIR_WEST,  "west",  "from the west",  "to the west",  DIR_EAST,  "W"},
    {DIR_UP,    "up",    "from above",     "above you",    DIR_DOWN,  "U"},
    {DIR_DOWN,  "down",  "from below",     "below you",    DIR_UP,    "D"},
    {0},
};

/* the function table */
const SPEC_T spec_table[SPEC_MAX + 1] = {
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

const COLOUR_SETTING_T colour_setting_table[COLOUR_MAX + 1] = {
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

const COLOUR_T colour_table[] = {
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

/* We use WEAR_LOC_MAX+2 to account for 'none' and a blank wear location. */
const WEAR_LOC_T wear_loc_table[WEAR_LOC_MAX + 2] = {
    {WEAR_LOC_NONE,     "none",     "in the inventory",    "<in inventory>",         0,                  0, "You wear $p nowhere (??).",                 "$n wears $p nowhere (??)." },
    {WEAR_LOC_LIGHT,    "light",    "as a light",          "<used as light>",        ITEM_WEAR_LIGHT,    0, "You light $p and hold it.",                 "$n lights $p and holds it." },
    {WEAR_LOC_FINGER_L, "lfinger",  "on the left finger",  "<worn on L-finger>",     ITEM_WEAR_FINGER,   0, "You wear $p on your left finger.",          "$n wears $p on $s left finger." },
    {WEAR_LOC_FINGER_R, "rfinger",  "on the right finger", "<worn on R-finger>",     ITEM_WEAR_FINGER,   0, "You wear $p on your right finger.",         "$n wears $p on $s right finger." },
    {WEAR_LOC_NECK_1,   "neck1",    "around the neck (1)", "<worn around neck 1>",   ITEM_WEAR_NECK,   100, "You wear $p around your neck.",             "$n wears $p around $s neck." },
    {WEAR_LOC_NECK_2,   "neck2",    "around the neck (2)", "<worn around neck 2>",   ITEM_WEAR_NECK,   100, "You wear $p around your neck.",             "$n wears $p around $s neck." },
    {WEAR_LOC_BODY,     "body",     "on the torso",        "<worn on torso>",        ITEM_WEAR_BODY,   300, "You wear $p on your torso.",                "$n wears $p on $s torso." },
    {WEAR_LOC_HEAD,     "head",     "over the head",       "<worn on head>",         ITEM_WEAR_HEAD,   200, "You wear $p on your head.",                 "$n wears $p on $s head." },
    {WEAR_LOC_LEGS,     "legs",     "on the legs",         "<worn on legs>",         ITEM_WEAR_LEGS,   200, "You wear $p on your legs.",                 "$n wears $p on $s legs." },
    {WEAR_LOC_FEET,     "feet",     "on the feet",         "<worn on feet>",         ITEM_WEAR_FEET,   100, "You wear $p on your feet.",                 "$n wears $p on $s feet." },
    {WEAR_LOC_HANDS,    "hands",    "on the hands",        "<worn on hands>",        ITEM_WEAR_HANDS,  100, "You wear $p on your hands.",                "$n wears $p on $s hands." },
    {WEAR_LOC_ARMS,     "arms",     "on the arms",         "<worn on arms>",         ITEM_WEAR_ARMS,   100, "You wear $p on your arms.",                 "$n wears $p on $s arms." },
    {WEAR_LOC_SHIELD,   "shield",   "as a shield",         "<worn as shield>",       ITEM_WEAR_SHIELD, 100, "You wear $p as a shield.",                  "$n wears $p as a shield." },
    {WEAR_LOC_ABOUT,    "about",    "about the body",      "<worn about body>",      ITEM_WEAR_ABOUT,  200, "You wear $p about your torso.",             "$n wears $p about $s torso." },
    {WEAR_LOC_WAIST,    "waist",    "around the waist",    "<worn about waist>",     ITEM_WEAR_WAIST,  100, "You wear $p about your waist.",             "$n wears $p about $s waist." },
    {WEAR_LOC_WRIST_L,  "lwrist",   "on the left wrist",   "<worn around L-wrist>",  ITEM_WEAR_WRIST,  100, "You wear $p around your left wrist.",       "$n wears $p around $s left wrist." },
    {WEAR_LOC_WRIST_R,  "rwrist",   "on the right wrist",  "<worn around R-wrist>",  ITEM_WEAR_WRIST,  100, "You wear $p around your right wrist.",      "$n wears $p around $s right wrist." },
    {WEAR_LOC_WIELD,    "wielded",  "wielded",             "<wielded>",              ITEM_WIELD,         0, "You wield $p.",                             "$n wields $p." },
    {WEAR_LOC_HOLD,     "hold",     "held in the hands",   "<held>",                 ITEM_HOLD,        100, "You hold $p in your hand.",                 "$n holds $p in $s hand." },
    {WEAR_LOC_FLOAT,    "floating", "floating nearby",     "<floating nearby>",      ITEM_WEAR_FLOAT,    0, "You release $p and it floats next to you.", "$n releases $p to float next to $m." },
    {0},
};

const MATERIAL_T material_table[MATERIAL_MAX + 1] = {
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

RECYCLE_T recycle_table[RECYCLE_MAX + 1] = {
    RECYCLE_N_ENTRY (RECYCLE_BAN_T,         ban,         BAN_T,         name, ban_init,         ban_dispose),
    RECYCLE_N_ENTRY (RECYCLE_AREA_T,        area,        AREA_T,        name, area_init,        area_dispose),
    RECYCLE_ENTRY   (RECYCLE_EXTRA_DESCR_T, extra_descr, EXTRA_DESCR_T,       extra_descr_init, extra_descr_dispose),
    RECYCLE_ENTRY   (RECYCLE_EXIT_T,        exit,        EXIT_T,              exit_init,        exit_dispose),
    RECYCLE_N_ENTRY (RECYCLE_ROOM_INDEX_T,  room_index,  ROOM_INDEX_T,  name, room_index_init,  room_index_dispose),
    RECYCLE_N_ENTRY (RECYCLE_OBJ_INDEX_T,   obj_index,   OBJ_INDEX_T,   name, obj_index_init,   obj_index_dispose),
    RECYCLE_ENTRY   (RECYCLE_SHOP_T,        shop,        SHOP_T,              shop_init,        NULL),
    RECYCLE_ENTRY   (RECYCLE_MOB_INDEX_T,   mob_index,   MOB_INDEX_T,         mob_index_init,   mob_index_dispose),
    RECYCLE_ENTRY   (RECYCLE_RESET_T,       reset_data,  RESET_T,             reset_data_init,  NULL),
    RECYCLE_N_ENTRY (RECYCLE_HELP_T,        help,        HELP_T,     keyword, NULL,             help_dispose),
    RECYCLE_ENTRY   (RECYCLE_MPROG_CODE_T,  mpcode,      MPROG_CODE_T,        mpcode_init,      mpcode_dispose),
    RECYCLE_ENTRY   (RECYCLE_DESCRIPTOR_T,  descriptor,  DESCRIPTOR_T,        descriptor_init,  descriptor_dispose),
    RECYCLE_ENTRY   (RECYCLE_GEN_T,         gen_data,    GEN_T,               NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_AFFECT_T,      affect,      AFFECT_T,            NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_OBJ_T,         obj,         OBJ_T,               NULL,             obj_dispose),
    RECYCLE_ENTRY   (RECYCLE_CHAR_T,        char,        CHAR_T,              char_init,        char_dispose),
    RECYCLE_ENTRY   (RECYCLE_PC_T,          pcdata,      PC_T,                pcdata_init,      pcdata_dispose),
    RECYCLE_ENTRY   (RECYCLE_MEM_T,         mem_data,    MEM_T,               NULL,             NULL),
    RECYCLE_ENTRY   (RECYCLE_BUFFER_T,      buf,         BUFFER_T,            buf_init,         buf_dispose),
    RECYCLE_ENTRY   (RECYCLE_MPROG_LIST_T,  mprog,       MPROG_LIST_T,        mprog_init,       mprog_dispose),
    RECYCLE_N_ENTRY (RECYCLE_HELP_AREA_T,   had,         HELP_AREA_T, filename, NULL,           had_dispose),
    RECYCLE_ENTRY   (RECYCLE_NOTE_T,        note,        NOTE_T,              NULL,             note_dispose),
    RECYCLE_N_ENTRY (RECYCLE_SOCIAL_T,      social,      SOCIAL_T,      name, NULL,             social_dispose),
    RECYCLE_N_ENTRY (RECYCLE_PORTAL_EXIT_T, portal_exit, PORTAL_EXIT_T, name, NULL,             portal_exit_dispose),
    RECYCLE_ENTRY   (RECYCLE_PORTAL_T,      portal,      PORTAL_T,            NULL,             portal_dispose),
    {0}
};

/* Technically not const, but this is a good place to have it! */
BOARD_T board_table[BOARD_MAX + 1] = {
    {"General",  "General discussion",           0, 2,     "all", DEF_INCLUDE, 21, NULL, FALSE},
    {"Ideas",    "Suggestion for improvement",   0, 2,     "all", DEF_NORMAL,  60, NULL, FALSE},
    {"Announce", "Announcements from Immortals", 0, L_IMM, "all", DEF_NORMAL,  60, NULL, FALSE},
    {"Bugs",     "Typos, bugs, errors",          0, 1,     "imm", DEF_NORMAL,  60, NULL, FALSE},
    {"Personal", "Personal messages",            0, 1,     "all", DEF_EXCLUDE, 28, NULL, FALSE},
    {0}
};

/* wiznet table and prototype for future flag setting */
const WIZNET_T wiznet_table[WIZNET_MAX + 1] = {
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

const AFFECT_BIT_T affect_bit_table[AFF_TO_MAX + 1] = {
    {"affects", AFF_TO_AFFECTS, affect_flags, "affect_flags"},
    {"object",  AFF_TO_OBJECT,  extra_flags,  "extra_flags"},
    {"immune",  AFF_TO_IMMUNE,  res_flags,    "res_flags"},
    {"resist",  AFF_TO_RESIST,  res_flags,    "res_flags"},
    {"vuln",    AFF_TO_VULN,    res_flags,    "res_flags"},
    {"weapon",  AFF_TO_WEAPON,  weapon_flags, "weapon_flags"},
    {NULL,      0,              NULL},
};

const DAY_T day_table[DAY_MAX + 1] = {
    { DAY_MOON,       "the Moon" },
    { DAY_BULL,       "the Bull" },
    { DAY_DECEPTION,  "Deception" },
    { DAY_THUNDER,    "Thunder" },
    { DAY_FREEDOM,    "Freedom" },
    { DAY_GREAT_GODS, "the Great Gods" },
    { DAY_SUN,        "the Sun" },
    { -1, NULL }
};

const MONTH_T month_table[MONTH_MAX + 1] = {
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

const SKY_T sky_table[SKY_MAX + 1] = {
    { SKY_CLOUDLESS, "cloudless", "cloudless", 1020,   -1 },
    { SKY_CLOUDY,    "cloudy",    "cloudy",    1000, 1020 },
    { SKY_RAINING,   "raining",   "rainy",      980, 1000 },
    { SKY_LIGHTNING, "lightning", "lit by flashes of lightning", -1, 980 },
    { -1, NULL, NULL },
};

const SUN_T sun_table[SUN_MAX + 1] = {
    { SUN_DARK,  "dark",  TRUE,   0,  5, "The night has begun." },
    { SUN_RISE,  "rise",  FALSE,  5,  6, "The sun rises in the east." },
    { SUN_LIGHT, "light", FALSE,  6, 19, "The day has begun." },
    { SUN_SET,   "set",   TRUE,  19, 20, "The sun slowly disappears in the west." },
    { -1, NULL, 0 }
};

const POSE_T pose_table[] = {
    { "mage", {
        "You sizzle with energy.",                                      "$n sizzles with energy.",
        "You turn into a butterfly, then return to your normal shape.", "$n turns into a butterfly, then returns to $s normal shape.",
        "Blue sparks fly from your fingers.",                           "Blue sparks fly from $n's fingers.",
        "Little red lights dance in your eyes.",                        "Little red lights dance in $n's eyes.",
        "A slimy green monster appears before you and bows.",           "A slimy green monster appears before $n and bows.",
        "You turn everybody into a little pink elephant.",              "You are turned into a little pink elephant by $n.",
        "A small ball of light dances on your fingertips.",             "A small ball of light dances on $n's fingertips.",
        "Smoke and fumes leak from your nostrils.",                     "Smoke and fumes leak from $n's nostrils.",
        "The light flickers as you rap in magical languages.",          "The light flickers as $n raps in magical languages.",
        "Your head disappears.",                                        "$n's head disappears.",
        "A fire elemental singes your hair.",                           "A fire elemental singes $n's hair.",
        "The sky changes colour to match your eyes.",                   "The sky changes colour to match $n's eyes.",
        "The stones dance to your command.",                            "The stones dance to $n's command.",
        "The heavens and grass change colour as you smile.",            "The heavens and grass change colour as $n smiles.",
        "Everyone's clothes are transparent, and you are laughing.",    "Your clothes are transparent, and $n is laughing.",
        "A black hole swallows you.",                                   "A black hole swallows $n.",
        "The world shimmers in time with your whistling.",              "The world shimmers in time with $n's whistling.",
        NULL
    }},
    { "cleric", {
        "You feel very holy.",                                          "$n looks very holy.",
        "You nonchalantly turn wine into water.",                       "$n nonchalantly turns wine into water.",
        "A halo appears over your head.",                               "A halo appears over $n's head.",
        "You recite words of wisdom.",                                  "$n recites words of wisdom.",
        "Deep in prayer, you levitate.",                                "Deep in prayer, $n levitates.",
        "An angel consults you.",                                       "An angel consults $n.",
        "Your body glows with an unearthly light.",                     "$n's body glows with an unearthly light.",
        "A spot light hits you.",                                       "A spot light hits $n.",
        "Everyone levitates as you pray.",                              "You levitate as $n prays.",
        "A cool breeze refreshes you.",                                 "A cool breeze refreshes $n.",
        "The sun pierces through the clouds to illuminate you.",        "The sun pierces through the clouds to illuminate $n.",
        "The ocean parts before you.",                                  "The ocean parts before $n.",
        "A thunder cloud kneels to you.",                               "A thunder cloud kneels to $n.",
        "The Burning Man speaks to you.",                               "The Burning Man speaks to $n.",
        "An eye in a pyramid winks at you.",                            "An eye in a pyramid winks at $n.",
        "Valentine Michael Smith offers you a glass of water.",         "Valentine Michael Smith offers $n a glass of water.",
        "The great god Mota gives you a staff.",                        "The great god Mota gives $n a staff.",
        NULL
    }},
    { "thief", {
        "You perform a small card trick.",                              "$n performs a small card trick.",
        "You wiggle your ears alternately.",                            "$n wiggles $s ears alternately.",
        "You nimbly tie yourself into a knot.",                         "$n nimbly ties $mself into a knot.",
        "You juggle with daggers, apples, and eyeballs.",               "$n juggles with daggers, apples, and eyeballs.",
        "You steal the underwear off every person in the room.",        "Your underwear is gone!  $n stole it!",
        "The dice roll ... and you win again.",                         "The dice roll ... and $n wins again.",
        "You count the money in everyone's pockets.",                   "Check your money, $n is counting it.",
        "You balance a pocket knife on your tongue.",                   "$n balances a pocket knife on your tongue.",
        "You produce a coin from everyone's ear.",                      "$n produces a coin from your ear.",
        "You step behind your shadow.",                                 "$n steps behind $s shadow.",
        "Your eyes dance with greed.",                                  "$n's eyes dance with greed.",
        "You deftly steal everyone's weapon.",                          "$n deftly steals your weapon.",
        "The Grey Mouser buys you a beer.",                             "The Grey Mouser buys $n a beer.",
        "Everyone's pocket explodes with your fireworks.",              "Your pocket explodes with $n's fireworks.",
        "Everyone discovers your dagger a centimeter from their eye.",  "You discover $n's dagger a centimeter from your eye.",
        "Where did you go?",                                            "Where did $n go?",
        "Click.",                                                       "Click.",
        NULL
    }},
    { "warrior", {
        "You show your bulging muscles.",                               "$n shows $s bulging muscles.",
        "You crack nuts between your fingers.",                         "$n cracks nuts between $s fingers.",
        "You grizzle your teeth and look mean.",                        "$n grizzles $s teeth and looks mean.",
        "You hit your head, and your eyes roll.",                       "$n hits $s head, and $s eyes roll.",
        "Crunch, crunch -- you munch a bottle.",                        "Crunch, crunch -- $n munches a bottle.",
        "... 98, 99, 100 ... you do pushups.",                          "... 98, 99, 100 ... $n does pushups.",
        "Arnold Schwarzenegger admires your physique.",                 "Arnold Schwarzenegger admires $n's physique.",
        "Watch your feet, you are juggling granite boulders.",          "Watch your feet, $n is juggling granite boulders.",
        "Oomph!  You squeeze water out of a granite boulder.",          "Oomph!  $n squeezes water out of a granite boulder.",
        "You pick your teeth with a spear.",                            "$n picks $s teeth with a spear.",
        "Everyone is swept off their foot by your hug.",                "You are swept off your feet by $n's hug.",
        "Your karate chop splits a tree.",                              "$n's karate chop splits a tree.",
        "A strap of your armor breaks over your mighty thews.",         "A strap of $n's armor breaks over $s mighty thews.",
        "A boulder cracks at your frown.",                              "A boulder cracks at $n's frown.",
        "Mercenaries arrive to do your bidding.",                       "Mercenaries arrive to do $n's bidding.",
        "Four matched Percherons bring in your chariot.",               "Four matched Percherons bring in $n's chariot.",
        "Atlas asks you to relieve him.",                               "Atlas asks $n to relieve him.",
        NULL
    }},
    {0}
};

SONG_T song_table[MAX_SONGS + 1];
