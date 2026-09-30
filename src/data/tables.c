// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Store/attack/RNG/etc tables and variables
#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"


// Store owners have different characteristics for pricing and haggling
// Note: Store owners should be added in groups, one for each store
owner_type owners[MAX_OWNERS] = {
    {"Erick the Honest       (Human)      General Store",   250, 175, 108, 4, 0, 12},
    {"Mauglin the Grumpy     (Dwarf)      Armory",        32000, 200, 112, 4, 5,  5},
    {"Arndal Beast-Slayer    (Half-Elf)   Weaponsmith",   10000, 185, 110, 5, 1,  8},
    {"Hardblow the Humble    (Human)      Temple",         3500, 175, 109, 6, 0, 15},
    {"Ga-nat the Greedy      (Gnome)      Alchemist",     12000, 220, 115, 4, 4,  9},
    {"Valeria Starshine      (Elf)        Magic Shop",    32000, 175, 110, 5, 2, 11},
    {"Andy the Friendly      (Halfling)   General Store",   200, 170, 108, 5, 3, 15},
    {"Darg-Low the Grim      (Human)      Armory",        10000, 190, 111, 4, 0,  9},
    {"Oglign Dragon-Slayer   (Dwarf)      Weaponsmith",   32000, 195, 112, 4, 5,  8},
    {"Gunnar the Paladin     (Human)      Temple",         5000, 185, 110, 5, 0, 23},
    {"Mauser the Chemist     (Half-Elf)   Alchemist",     10000, 190, 111, 5, 1,  8},
    {"Gopher the Great!      (Gnome)      Magic Shop",    20000, 215, 113, 6, 4, 10},
    {"Lyar-el the Comely     (Elf)        General Store",   300, 165, 107, 6, 2, 18},
    {"Mauglim the Horrible   (Half-Orc)   Armory",         3000, 200, 113, 5, 6,  9},
    {"Ithyl-Mak the Beastly  (Half-Troll) Weaponsmith",    3000, 210, 115, 6, 7,  8},
    {"Delilah the Pure       (Half-Elf)   Temple",        25000, 180, 107, 6, 1, 20},
    {"Wizzle the Chaotic     (Halfling)   Alchemist",     10000, 190, 110, 6, 3,  8},
    {"Inglorian the Mage     (Human?)     Magic Shop",    32000, 200, 110, 7, 0, 10},
};

// Buying and selling adjustments for character race VS store owner race
uint8_t rgold_adj[MAX_RACES][MAX_RACES] = {
    //Hum, HfE, Elf, Hal, Gno, Dwa, HfO, HfT
    { 100, 105, 105, 110, 113, 115, 120, 125 }, // Human
    { 110, 100, 100, 105, 110, 120, 125, 130 }, // Half-Elf
    { 110, 105, 100, 105, 110, 120, 125, 130 }, // Elf
    { 115, 110, 105,  95, 105, 110, 115, 130 }, // Halfling
    { 115, 115, 110, 105,  95, 110, 115, 130 }, // Gnome
    { 115, 120, 120, 110, 110,  95, 125, 135 }, // Dwarf
    { 115, 120, 125, 115, 115, 130, 110, 115 }, // Half-Orc
    { 110, 115, 115, 110, 110, 130, 110, 110 }, // Half-Troll
};

// object_list[] index of objects that may appear in the store
uint16_t store_choice[MAX_STORES][STORE_CHOICES] = {
    // General Store
    {
      366, 365, 364,  84,  84, 365, 123, 366, 365, 350, 349, 348, 347,
      346, 346, 345, 345, 345, 344, 344, 344, 344, 344, 344, 344, 344
    },
    // Armory
    {
       94,  95,  96, 109, 103, 104, 105, 106, 110, 111, 112, 114, 116,
      124, 125, 126, 127, 129, 103, 104, 124, 125,  91,  92,  95,  96
    },
    // Weaponsmith
    {
      29, 30, 34, 37, 45, 49, 57, 58, 59, 65, 67, 68, 73,
      74, 75, 77, 79, 80, 81, 83, 29, 30, 80, 83, 80, 83
    },
    // Temple
    {
      322, 323, 324, 325, 180, 180, 233, 237, 240, 241, 361, 362,  57,
       58,  59, 260, 358, 359, 265, 237, 237, 240, 240, 241, 323, 359
    },
    // Alchemy shop
    {
      173, 174, 175, 351, 351, 352, 353, 354, 355, 356, 357, 206, 227,
      230, 236, 252, 253, 352, 353, 354, 355, 356, 359, 363, 359, 359
    },
    // Magic-User store
    {
      318, 141, 142, 153, 164, 167, 168, 140, 319, 320, 320, 321, 269,
      270, 282, 286, 287, 292, 293, 294, 295, 308, 269, 290, 319, 282
    },
};

// 6 つの買いとり判定関数はここで再宣言していた（sets.c 定義）。
// externs.h:518-523 に同じ宣言があるので、そちらに任せる。
// 二重に書くと片方だけ直したときに食いちがうが、誰も気づけない。

// Each store will buy only certain items, based on TVAL
bool (*store_buy[MAX_STORES])(int) = {
    general_store,
    armory,
    weaponsmith,
    temple,
    alchemist,
    magic_shop,
};

// Following are arrays for descriptive pieces
const char *colors[MAX_COLORS] = {
    // Do not move the first three
    "Icky Green",  "Light Brown",  "Clear",
    "Azure", "Blue", "Blue Speckled", "Black", "Brown", "Brown Speckled", "Bubbling",
    "Chartreuse", "Cloudy", "Copper Speckled", "Crimson", "Cyan", "Dark Blue",
    "Dark Green", "Dark Red", "Gold Speckled", "Green", "Green Speckled", "Grey",
    "Grey Speckled", "Hazy", "Indigo", "Light Blue", "Light Green", "Magenta",
    "Metallic Blue", "Metallic Red", "Metallic Green", "Metallic Purple", "Misty",
    "Orange", "Orange Speckled", "Pink", "Pink Speckled", "Puce", "Purple",
    "Purple Speckled", "Red", "Red Speckled", "Silver Speckled", "Smoky",
    "Tangerine", "Violet", "Vermilion", "White", "Yellow",
};

const char *mushrooms[MAX_MUSH] = {
    "Blue", "Black", "Black Spotted", "Brown", "Dark Blue", "Dark Green", "Dark Red",
    "Ecru", "Furry", "Green", "Grey", "Light Blue", "Light Green", "Plaid", "Red",
    "Slimy", "Tan", "White", "White Spotted", "Wooden", "Wrinkled", "Yellow",
};

const char *woods[MAX_WOODS] = {
    "Aspen", "Balsa", "Banyan", "Birch", "Cedar", "Cottonwood", "Cypress", "Dogwood",
    "Elm", "Eucalyptus", "Hemlock", "Hickory", "Ironwood", "Locust", "Mahogany",
    "Maple", "Mulberry", "Oak", "Pine", "Redwood", "Rosewood", "Spruce", "Sycamore",
    "Teak", "Walnut",
};

const char *metals[MAX_METALS] = {
    "Aluminum", "Cast Iron", "Chromium", "Copper", "Gold", "Iron", "Magnesium",
    "Molybdenum", "Nickel", "Rusty", "Silver", "Steel", "Tin", "Titanium", "Tungsten",
    "Zirconium", "Zinc", "Aluminum-Plated", "Copper-Plated", "Gold-Plated",
    "Nickel-Plated", "Silver-Plated", "Steel-Plated", "Tin-Plated", "Zinc-Plated",
};

const char *rocks[MAX_ROCKS] = {
    "Alexandrite", "Amethyst", "Aquamarine", "Azurite", "Beryl", "Bloodstone",
    "Calcite", "Carnelian", "Corundum", "Diamond", "Emerald", "Fluorite", "Garnet",
    "Granite", "Jade", "Jasper", "Lapis Lazuli", "Malachite", "Marble", "Moonstone",
    "Onyx", "Opal", "Pearl", "Quartz", "Quartzite", "Rhodonite", "Ruby", "Sapphire",
    "Tiger Eye", "Topaz", "Turquoise", "Zircon"
};

const char *amulets[MAX_AMULETS] = {
    "Amber", "Driftwood", "Coral", "Agate", "Ivory", "Obsidian",
    "Bone", "Brass", "Bronze", "Pewter", "Tortoise Shell",
};

const char *syllables[MAX_SYLLABLES] = {
    "a",    "ab",   "ag",   "aks",  "ala",  "an",  "ankh", "app", "arg",
    "arze", "ash",  "aus",  "ban",  "bar",  "bat", "bek",  "bie", "bin",
    "bit",  "bjor", "blu",  "bot",  "bu",   "byt", "comp", "con", "cos",
    "cre",  "dalf", "dan",  "den",  "doe",  "dok", "eep",  "el",  "eng",
    "er",   "ere",  "erk",  "esh",  "evs",  "fa",  "fid",  "for", "fri",
    "fu",   "gan",  "gar",  "glen", "gop",  "gre", "ha",   "he",  "hyd",
    "i",    "ing",  "ion",  "ip",   "ish",  "it",  "ite",  "iv",  "jo",
    "kho",  "kli",  "klis", "la",   "lech", "man", "mar",  "me",  "mi",
    "mic",  "mik",  "mon",  "mung", "mur",  "nej", "nelg", "nep", "ner",
    "nes",  "nis",  "nih",  "nin",  "o",    "od",  "ood",  "org", "orn",
    "ox",   "oxy",  "pay",  "pet",  "ple",  "plu", "po",   "pot", "prok",
    "re",   "rea",  "rhov", "ri",   "ro",   "rog", "rok",  "rol", "sa",
    "san",  "sat",  "see",  "sef",  "seh",  "shu", "ski",  "sna", "sne",
    "snik", "sno",  "so",   "sol",  "sri",  "sta", "sun",  "ta",  "tab",
    "tem",  "ther", "ti",   "tox",  "trol", "tue", "turs", "u",   "ulk",
    "um",   "un",   "uni",  "ur",   "val",  "viv", "vly",  "vom", "wah",
    "wed",  "werg", "wex",  "whon", "wun",  "x",   "yerg", "yp",  "zun",
};

// used to calculate the number of blows the player gets in combat
uint8_t blows_table[7][6] = {
    // STR/W:   9  18  67  107 117 118  : DEX
    { 1,  1,  1,  1,  1,  1 }, // <2
    { 1,  1,  1,  1,  2,  2 }, // <3
    { 1,  1,  1,  2,  2,  3 }, // <4
    { 1,  1,  2,  2,  3,  3 }, // <5
    { 1,  2,  2,  3,  3,  4 }, // <7
    { 1,  2,  2,  3,  4,  4 }, // <9
    { 2,  2,  3,  3,  4,  4 }, // >9
};
