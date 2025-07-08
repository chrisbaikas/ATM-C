#include "header.h"

/* -------------------------------------------------------------------------*/
int initDatabase(const char *dbFile)
{
    sqlite3 *db = openDatabase(dbFile);
    if (!db) return -1;

    const char *sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE, "
        "passwordHash TEXT);";

    char *err = NULL;
    int rc = sqlite3_exec(db, sql, 0, 0, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (create users): %s\n", err);
        sqlite3_free(err);
    }
    sqlite3_close(db);
    return rc;
}

/* -------------------------------------------------------------------------*/
int getUserId(const char *username)
{
    sqlite3 *db = openDatabase("users.db");
    if (!db) return -1;

    const char *sql = "SELECT id FROM users WHERE username = ?;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    int userId = -1;

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
            userId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return userId;      /* -1 = not-found / error */
}
