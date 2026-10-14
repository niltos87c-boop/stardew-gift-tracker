/**
 * villager.c - Implementation of the villager data model.
 */
#include "villager.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

/* ---------------------------------------------------------------------- */
/* Static data: default villagers and their loved gifts                    */
/* ---------------------------------------------------------------------- */

#define MAX_DEFAULT_GIFTS 10

typedef struct {
    const char *name;
    const char *gifts[MAX_DEFAULT_GIFTS]; /* NULL-terminated */
} DefaultVillager;

static const DefaultVillager DEFAULT_VILLAGERS[] = {
    { "Abigail",  { "Amethyst", "Banana Pudding", "Blackberry Cobbler",
                    "Chocolate Cake", "Pufferfish", "Pumpkin", "Spicy Eel", NULL } },
    { "Sebastian",{ "Frozen Tear", "Obsidian", "Pumpkin Soup", "Sashimi",
                    "Void Egg", "Frog Egg", NULL } },
    { "Haley",    { "Coconut", "Fruit Salad", "Pink Cake", "Sunflower", NULL } },
    { "Leah",     { "Goat Cheese", "Poppyseed Muffin", "Salad", "Stir Fry",
                    "Truffle", "Vegetable Medley", "Wine", NULL } },
    { "Emily",    { "Amethyst", "Aquamarine", "Cloth", "Emerald", "Jade",
                    "Ruby", "Survival Burger", "Topaz", "Wool", NULL } },
    { "Shane",    { "Beer", "Hot Pepper", "Pepper Poppers", "Pizza", NULL } },
    { "Elliott",  { "Crab Cakes", "Duck Feather", "Lobster", "Pomegranate",
                    "Squid Ink", "Tom Kha Soup", NULL } },
    { "Sam",      { "Cactus Fruit", "Maple Bar", "Pizza", "Tigerseye", NULL } },
};

/* ---------------------------------------------------------------------- */
/* Helpers                                                                 */
/* ---------------------------------------------------------------------- */

/* Portable strdup replacement; returns NULL on allocation failure. */
static char *duplicate_string(const char *source)
{
    size_t length = strlen(source) + 1;
    char *copy = malloc(length);
    if (copy != NULL) {
        memcpy(copy, source, length);
    }
    return copy;
}

int str_equal_ci(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

/* Releases all memory owned by a single villager. */
static void villager_destroy(Villager *villager)
{
    for (size_t i = 0; i < villager->gift_count; i++) {
        free(villager->loved_gifts[i]);
    }
    free(villager->loved_gifts);
    free(villager->name);

    villager->name = NULL;
    villager->loved_gifts = NULL;
    villager->gift_count = 0;
    villager->gift_capacity = 0;
}

/* ---------------------------------------------------------------------- */
/* Database lifecycle                                                      */
/* ---------------------------------------------------------------------- */

void db_init(VillagerDB *db)
{
    db->villagers = NULL;
    db->count = 0;
    db->capacity = 0;
}

void db_free(VillagerDB *db)
{
    for (size_t i = 0; i < db->count; i++) {
        villager_destroy(&db->villagers[i]);
    }
    free(db->villagers);
    db_init(db);
}

int db_load_defaults(VillagerDB *db)
{
    size_t total = sizeof(DEFAULT_VILLAGERS) / sizeof(DEFAULT_VILLAGERS[0]);

    for (size_t i = 0; i < total; i++) {
        Villager *villager = db_add_villager(db, DEFAULT_VILLAGERS[i].name);
        if (villager == NULL) {
            return -1;
        }
        for (size_t g = 0; DEFAULT_VILLAGERS[i].gifts[g] != NULL; g++) {
            if (villager_add_loved_gift(villager, DEFAULT_VILLAGERS[i].gifts[g]) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------- */
/* Villager management                                                     */
/* ---------------------------------------------------------------------- */

/*
 * Appends a new villager. The returned pointer is only valid until the next
 * call that may grow the array (realloc can move the block).
 */
Villager *db_add_villager(VillagerDB *db, const char *name)
{
    if (db->count == db->capacity) {
        size_t new_capacity = (db->capacity == 0) ? INITIAL_CAPACITY
                                                  : db->capacity * 2;
        Villager *grown = realloc(db->villagers, new_capacity * sizeof(Villager));
        if (grown == NULL) {
            return NULL; /* Original block is still valid and will be freed */
        }
        db->villagers = grown;
        db->capacity = new_capacity;
    }

    Villager *villager = &db->villagers[db->count];
    villager->name = duplicate_string(name);
    if (villager->name == NULL) {
        return NULL;
    }
    villager->loved_gifts = NULL;
    villager->gift_count = 0;
    villager->gift_capacity = 0;
    villager->gifts_given_this_week = 0;

    db->count++;
    return villager;
}

int villager_add_loved_gift(Villager *villager, const char *gift)
{
    if (villager->gift_count == villager->gift_capacity) {
        size_t new_capacity = (villager->gift_capacity == 0)
                                  ? INITIAL_CAPACITY
                                  : villager->gift_capacity * 2;
        char **grown = realloc(villager->loved_gifts,
                               new_capacity * sizeof(char *));
        if (grown == NULL) {
            return -1;
        }
        villager->loved_gifts = grown;
        villager->gift_capacity = new_capacity;
    }

    char *copy = duplicate_string(gift);
    if (copy == NULL) {
        return -1;
    }
    villager->loved_gifts[villager->gift_count++] = copy;
    return 0;
}

int villager_loves_gift(const Villager *villager, const char *gift)
{
    for (size_t i = 0; i < villager->gift_count; i++) {
        if (str_equal_ci(villager->loved_gifts[i], gift)) {
            return 1;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------- */
/* Queries and actions                                                     */
/* ---------------------------------------------------------------------- */

Villager *db_find_villager(VillagerDB *db, const char *name)
{
    for (size_t i = 0; i < db->count; i++) {
        if (str_equal_ci(db->villagers[i].name, name)) {
            return &db->villagers[i];
        }
    }
    return NULL;
}

GiftResult db_give_gift(VillagerDB *db, const char *villager_name,
                        const char *gift, int *remaining_out)
{
    Villager *villager = db_find_villager(db, villager_name);
    if (villager == NULL) {
        return GIFT_ERR_UNKNOWN_VILLAGER;
    }
    if (villager->gifts_given_this_week >= MAX_GIFTS_PER_WEEK) {
        return GIFT_ERR_LIMIT_REACHED;
    }

    villager->gifts_given_this_week++;
    if (remaining_out != NULL) {
        *remaining_out = MAX_GIFTS_PER_WEEK - villager->gifts_given_this_week;
    }
    return villager_loves_gift(villager, gift) ? GIFT_OK : GIFT_OK_NOT_LOVED;
}

void db_reset_week(VillagerDB *db)
{
    for (size_t i = 0; i < db->count; i++) {
        db->villagers[i].gifts_given_this_week = 0;
    }
}
