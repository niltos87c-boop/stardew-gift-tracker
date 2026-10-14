/**
 * storage.h - Persists weekly gift counters between program runs.
 */
#ifndef STORAGE_H
#define STORAGE_H

#include "villager.h"

#define DEFAULT_SAVE_FILE "tracker_data.txt"
#define SAVE_FILE_ENV_VAR "STARDEW_TRACKER_FILE"

/* Returns the save file path (env override or default). Not heap-allocated. */
const char *storage_get_path(void);

/* Loads counters into db. A missing file is not an error. 0 = ok, -1 = error. */
int storage_load(VillagerDB *db, const char *path);

/* Writes counters to disk. 0 = ok, -1 = error. */
int storage_save(const VillagerDB *db, const char *path);

#endif /* STORAGE_H */
