#include "header.h"

/* ───────────────────────────── table bootstrap ───────────────────────────── */
int ensureRecordsTableExists(void)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "CREATE TABLE IF NOT EXISTS records ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "userId INTEGER, "
        "username TEXT, "
        "accountNbr INTEGER, "
        "depositDate TEXT, "
        "country TEXT, "
        "phone TEXT, "
        "amount REAL, "
        "accountType TEXT);";

    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
        fprintf(stderr, "SQL error (create records): %s\n", sqlite3_errmsg(db));

    sqlite3_close(db);
    return rc;
}

/* ───────────────────────────── insert ───────────────────────────── */
int insertRecordForUser(const User *u, const Record *r)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "INSERT INTO records (userId, username, accountNbr, depositDate, "
        "country, phone, amount, accountType) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
    } else {
        // Bind values
        char date[20];
        snprintf(date, sizeof(date), "%d/%d/%d",
                 r->deposit.day, r->deposit.month, r->deposit.year);

        sqlite3_bind_int   (stmt, 1, u->id);
        sqlite3_bind_text  (stmt, 2, u->name,   -1, SQLITE_STATIC);
        sqlite3_bind_int   (stmt, 3, r->accountNbr);
        sqlite3_bind_text  (stmt, 4, date,       -1, SQLITE_STATIC);
        sqlite3_bind_text  (stmt, 5, r->country, -1, SQLITE_STATIC);
        sqlite3_bind_text  (stmt, 6, r->phone,   -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 7, r->amount);
        sqlite3_bind_text  (stmt, 8, r->accountType, -1, SQLITE_STATIC);

        // Execute
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Insert record failed: %s\n", sqlite3_errmsg(db));
        }
    }

    // Always finalize and close
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);

    return (rc == SQLITE_DONE) ? 0 : -1;
}


/* ───────────────────────────── select single ───────────────────────────── */
int getRecordForUser(User u, int accountNbr, Record *r)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) {
        return -1;
    }

    const char *sql =
        "SELECT id,userId,accountNbr,depositDate,country,phone,amount,accountType "
        "FROM records WHERE username = ? AND accountNbr = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // failed to compile SQL
        sqlite3_close(db);
        return -1;
    }

    // bind parameters
    sqlite3_bind_text(stmt, 1, u.name,      -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, accountNbr);

    // execute and fetch
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // fill struct from columns
        r->id         = sqlite3_column_int(stmt, 0);
        r->userId     = sqlite3_column_int(stmt, 1);
        r->accountNbr = sqlite3_column_int(stmt, 2);

        // parse date string "dd/mm/yyyy"
        sscanf((const char*)sqlite3_column_text(stmt, 3),
               "%d/%d/%d",
               &r->deposit.day,
               &r->deposit.month,
               &r->deposit.year);

        // copy text fields safely
        strncpy(r->country,     (const char*)sqlite3_column_text(stmt, 4), sizeof r->country - 1);
        strncpy(r->phone,       (const char*)sqlite3_column_text(stmt, 5), sizeof r->phone   - 1);
        r->amount    = sqlite3_column_double(stmt, 6);
        strncpy(r->accountType, (const char*)sqlite3_column_text(stmt, 7), sizeof r->accountType - 1);

        rc = 0; // success
    } else {
        rc = -1; // no matching row
    }

    // cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}

/* ───────────────────────────── update whole record ───────────────────────────── */
int setRecordForUser(const User *u, const Record *r)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) {
        return -1;
    }

    const char *sql =
        "UPDATE records "
        "SET userId = ?, depositDate = ?, country = ?, phone = ?, amount = ?, accountType = ? "
        "WHERE username = ? AND accountNbr = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // failed to prepare
        sqlite3_close(db);
        return -1;
    }

    // format the date as dd/mm/yyyy
    char date[20];
    snprintf(date, sizeof(date), "%d/%d/%d",
             r->deposit.day, r->deposit.month, r->deposit.year);

    // bind parameters
    sqlite3_bind_int   (stmt, 1, u->id);
    sqlite3_bind_text  (stmt, 2, date,               -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 3, r->country,         -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 4, r->phone,           -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, r->amount);
    sqlite3_bind_text  (stmt, 6, r->accountType,     -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 7, u->name,            -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 8, r->accountNbr);

    // execute
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update record failed: %s\n", sqlite3_errmsg(db));
        rc = -1;
    } else {
        rc = 0;
    }

    // cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}


/* ───────────────────────────── delete ───────────────────────────── */
int removeRecordForUser(const User *u, int accountNbr)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) {
        return -1;
    }

    const char *sql =
        "DELETE FROM records WHERE username = ? AND accountNbr = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    // Bind the username and account number
    sqlite3_bind_text(stmt, 1, u->name,      -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, accountNbr);

    // Execute the delete
    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);

    // Finalize and close
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Return success only if a row was actually deleted
    return (rc == SQLITE_DONE && changes > 0) ? 0 : -1;
}


/* ───────────────────────────── bulk select ───────────────────────────── */
int getAllRecordsForUser(const char *username, sqlite3_stmt **stmtOut)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "SELECT accountNbr,depositDate,country,phone,amount,accountType "
        "FROM records WHERE username=?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, stmtOut, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "getAllRecordsForUser: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    sqlite3_bind_text(*stmtOut, 1, username, -1, SQLITE_STATIC);
    /* db handle must stay open while caller steps stmt; caller will close */
    return 0;
}

/* ───────────────────────────── misc helpers ───────────────────────────── */
int getTransactionInfo(const char *username, int accountNbr,
                       int *recordId, double *amount, char *accountType)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "SELECT id,amount,accountType FROM records "
        "WHERE username=? AND accountNbr=?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 2, accountNbr);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            *recordId = sqlite3_column_int(stmt, 0);
            *amount   = sqlite3_column_double(stmt, 1);
            strncpy(accountType,
                    (const char*)sqlite3_column_text(stmt, 2), 49);
            accountType[49] = '\0';
            rc = 0;
        } else {
            rc = -1;
        }
    } else rc = -1;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}

int updateAccountAmount(int recordId, double newAmount)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql = "UPDATE records SET amount=? WHERE id=?;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, newAmount);
        sqlite3_bind_int   (stmt, 2, recordId);
        rc = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int accountExistsForUser(const char *username, int accountNbr)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "SELECT 1 FROM records WHERE username=? AND accountNbr=?;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 2, accountNbr);
        rc = (sqlite3_step(stmt) == SQLITE_ROW) ? 1 : 0;
    } else rc = -1;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}

int transferAccountToUser(const char *oldUsername, const char *newUsername,
                          int newUserId, int accountNbr)
{
    sqlite3 *db = openDatabase("records.db");
    if (!db) return -1;

    const char *sql =
        "UPDATE records SET username=?, userId=? "
        "WHERE username=? AND accountNbr=?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, newUsername, -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 2, newUserId);
        sqlite3_bind_text(stmt, 3, oldUsername, -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 4, accountNbr);
        rc = sqlite3_step(stmt);
    }
    int changes = sqlite3_changes(db);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return (rc == SQLITE_DONE && changes > 0) ? 0 : -1;
}
