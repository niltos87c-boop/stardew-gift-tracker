/**
 * main.c - Command-line entry point for the Stardew Valley Gift Tracker.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage.h"
#include "villager.h"

static void print_usage(const char *program)
{
    printf("Stardew Valley Gift Tracker\n\n"
           "Usage:\n"
           "  %s --gift <villager> <gift>   Record a gift given this week\n"
           "  %s --status                   Show weekly gift status\n"
           "  %s --loved <villager>         List a villager's loved gifts\n"
           "  %s --reset                    Start a new week (reset counters)\n"
           "  %s --help                     Show this message\n\n"
           "Environment:\n"
           "  %s  Path to the save file (default: %s)\n",
           program, program, program, program, program,
           SAVE_FILE_ENV_VAR, DEFAULT_SAVE_FILE);
}

static void print_status(const VillagerDB *db)
{
    printf("\n=== Weekly Gift Status ===\n");
    printf("%-12s %-8s %s\n", "Villager", "Gifts", "Progress");
    printf("-----------------------------------\n");

    for (size_t i = 0; i < db->count; i++) {
        const Villager *v = &db->villagers[i];
        char bar[MAX_GIFTS_PER_WEEK + 1];
        for (int s = 0; s < MAX_GIFTS_PER_WEEK; s++) {
            bar[s] = (s < v->gifts_given_this_week) ? '#' : '.';
        }
        bar[MAX_GIFTS_PER_WEEK] = '\0';

        printf("%-12s %d/%-6d [%s]%s\n", v->name, v->gifts_given_this_week,
               MAX_GIFTS_PER_WEEK, bar,
               (v->gifts_given_this_week >= MAX_GIFTS_PER_WEEK) ? " FULL" : "");
    }
    printf("\n");
}

static int command_gift(VillagerDB *db, const char *name, const char *gift)
{
    int remaining = 0;
    GiftResult result = db_give_gift(db, name, gift, &remaining);

    switch (result) {
    case GIFT_OK:
    case GIFT_OK_NOT_LOVED: {
        const Villager *v = db_find_villager(db, name);
        printf("Gave \"%s\" to %s.", gift, v->name);
        if (result == GIFT_OK) {
            printf(" They LOVE it!\n");
        } else {
            printf("\nWarning: \"%s\" is not on %s's loved gifts list.\n",
                   gift, v->name);
        }
        printf("Gifts remaining this week for %s: %d\n", v->name, remaining);
        return EXIT_SUCCESS;
    }
    case GIFT_ERR_UNKNOWN_VILLAGER:
        fprintf(stderr, "Error: unknown villager \"%s\".\n", name);
        return EXIT_FAILURE;
    case GIFT_ERR_LIMIT_REACHED:
        fprintf(stderr, "Error: weekly gift limit (%d) already reached for %s.\n",
                MAX_GIFTS_PER_WEEK, name);
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}

static int command_loved(VillagerDB *db, const char *name)
{
    const Villager *v = db_find_villager(db, name);
    if (v == NULL) {
        fprintf(stderr, "Error: unknown villager \"%s\".\n", name);
        return EXIT_FAILURE;
    }
    printf("%s's loved gifts:\n", v->name);
    for (size_t i = 0; i < v->gift_count; i++) {
        printf("  - %s\n", v->loved_gifts[i]);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    const char *program = argv[0];

    if (argc < 2) {
        print_usage(program);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "--help") == 0) {
        print_usage(program);
        return EXIT_SUCCESS;
    }

    VillagerDB db;
    db_init(&db);
    int exit_code = EXIT_FAILURE;
    const char *save_path = storage_get_path();
    int should_save = 0;

    if (db_load_defaults(&db) != 0) {
        fprintf(stderr, "Error: out of memory while loading villager data.\n");
        goto cleanup;
    }
    if (storage_load(&db, save_path) != 0) {
        fprintf(stderr, "Error: could not read save file \"%s\".\n", save_path);
        goto cleanup;
    }

    if (strcmp(argv[1], "--gift") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: --gift requires <villager> and <gift>.\n"
                            "Example: %s --gift \"Abigail\" \"Amethyst\"\n", program);
            goto cleanup;
        }
        exit_code = command_gift(&db, argv[2], argv[3]);
        should_save = (exit_code == EXIT_SUCCESS);
    } else if (strcmp(argv[1], "--status") == 0) {
        print_status(&db);
        exit_code = EXIT_SUCCESS;
    } else if (strcmp(argv[1], "--loved") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Error: --loved requires <villager>.\n");
            goto cleanup;
        }
        exit_code = command_loved(&db, argv[2]);
    } else if (strcmp(argv[1], "--reset") == 0) {
        db_reset_week(&db);
        printf("New week started. All gift counters have been reset.\n");
        should_save = 1;
        exit_code = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Error: unknown option \"%s\".\n\n", argv[1]);
        print_usage(program);
    }

    if (should_save && storage_save(&db, save_path) != 0) {
        fprintf(stderr, "Error: could not write save file \"%s\".\n", save_path);
        exit_code = EXIT_FAILURE;
    }

cleanup:
    db_free(&db);
    return exit_code;
}
