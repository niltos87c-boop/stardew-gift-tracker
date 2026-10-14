/**
 * villager.h - Data model and in-memory database for Stardew Valley villagers.
 */
#ifndef VILLAGER_H
#define VILLAGER_H

#include <stddef.h>

/* Maximum number of gifts a player may give to one villager per week. */
#define MAX_GIFTS_PER_WEEK 2

/* A single villager together with their loved gifts and weekly gift counter. */
typedef struct {
    char   *name;               /* Heap-allocated villager name              */
    char  **loved_gifts;        /* Heap-allocated array of heap-allocated    */
                                /* strings                                   */
    size_t  gift_count;         /* Number of loved gifts stored              */
    size_t  gift_capacity;      /* Allocated slots in loved_gifts            */
    int     gifts_given_this_week;
} Villager;

/* Dynamic array of villagers. */
typedef struct {
    Villager *villagers;
    size_t    count;
    size_t    capacity;
} VillagerDB;

/* Result codes returned by db_give_gift(). */
typedef enum {
    GIFT_OK = 0,
    GIFT_OK_NOT_LOVED,          /* Gift accepted, but it is not a loved gift */
    GIFT_ERR_UNKNOWN_VILLAGER,
    GIFT_ERR_LIMIT_REACHED
} GiftResult;

/* Lifecycle ----------------------------------------------------------- */
void       db_init(VillagerDB *db);
void       db_free(VillagerDB *db);
int        db_load_defaults(VillagerDB *db);   /* 0 on success, -1 on error */

/* Villager management ------------------------------------------------- */
Villager  *db_add_villager(VillagerDB *db, const char *name);
int        villager_add_loved_gift(Villager *villager, const char *gift);
int        villager_loves_gift(const Villager *villager, const char *gift);

/* Queries and actions ------------------------------------------------- */
Villager  *db_find_villager(VillagerDB *db, const char *name);
GiftResult db_give_gift(VillagerDB *db, const char *villager_name,
                        const char *gift, int *remaining_out);
void       db_reset_week(VillagerDB *db);

/* Case-insensitive string comparison helper (1 = equal, 0 = different). */
int        str_equal_ci(const char *a, const char *b);

#endif /* VILLAGER_H */
