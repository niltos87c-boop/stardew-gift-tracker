/**
 * storage.c - Simple "Name|count" text persistence.
 */
#include "storage.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *storage_get_path(void)
{
    const char *override_path = getenv(SAVE_FILE_ENV_VAR);
    return (override_path != NULL && *override_path != '\0')
               ? override_path
               : DEFAULT_SAVE_FILE;
}

int storage_load(VillagerDB *db, const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        /* First run: no save file yet, which is perfectly fine. */
        return (errno == ENOENT) ? 0 : -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *separator = strrchr(line, '|');
        if (separator == NULL) {
            continue; /* Skip malformed line */
        }
        *separator = '\0';

        char *end = NULL;
        long count = strtol(separator + 1, &end, 10);
        if (end == separator + 1 || count < 0 || count > MAX_GIFTS_PER_WEEK) {
            continue; /* Skip invalid counter */
        }

        Villager *villager = db_find_villager(db, line);
        if (villager != NULL) {
            villager->gifts_given_this_week = (int)count;
        }
    }

    int had_error = ferror(file);
    fclose(file);
    return had_error ? -1 : 0;
}

int storage_save(const VillagerDB *db, const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return -1;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (fprintf(file, "%s|%d\n", db->villagers[i].name,
                    db->villagers[i].gifts_given_this_week) < 0) {
            fclose(file);
            return -1;
        }
    }
    return (fclose(file) == 0) ? 0 : -1;
}
