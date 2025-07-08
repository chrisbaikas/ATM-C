#include "header.h"

/* generic DB opener */
sqlite3 *openDatabase(const char *dbFile)
{
    sqlite3 *db = NULL;
    if (sqlite3_open(dbFile, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database %s: %s\n",
                dbFile, sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}