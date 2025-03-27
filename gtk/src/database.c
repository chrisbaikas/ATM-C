#include "header.h"


int initDatabase(const char *dbFile) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc = sqlite3_open(dbFile, &db);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot open database: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return rc;
    }
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "username TEXT UNIQUE, "
                            "passwordHash TEXT);";
    rc = sqlite3_exec(db, sqlCreate, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "SQL error in table creation: %s", errMsg);
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_free(errMsg);
    }
    sqlite3_close(db);
    return rc;
}

sqlite3 *openDatabase(const char *dbFile) {
    sqlite3 *db;
    int rc = sqlite3_open(dbFile, &db);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot open database %s: %s", dbFile, sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return NULL;
    }
    return db;
}


int setRecordForUser(const User *u, const Record *r) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open("records.db", &db);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot open records database: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return rc;
    }

    // Update the record based on accountNbr and username
    const char *sqlUpdate = "UPDATE records SET userId = ?, depositDate = ?, country = ?, phone = ?, amount = ?, accountType = ? "
                            "WHERE username = ? AND accountNbr = ?;";
    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to prepare update statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        return rc;
    }
    
    // Bind parameters
    sqlite3_bind_int(stmt, 1, u->id);
    
    // Create a string for the date in the format "dd/mm/yyyy"
    char depositDate[20];
    snprintf(depositDate, sizeof(depositDate), "%d/%d/%d", r->deposit.day, r->deposit.month, r->deposit.year);
    sqlite3_bind_text(stmt, 2, depositDate, -1, SQLITE_STATIC);
    
    sqlite3_bind_text(stmt, 3, r->country, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, r->phone, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, r->amount);
    sqlite3_bind_text(stmt, 6, r->accountType, -1, SQLITE_STATIC);
    
    // Update conditions: record belonging to the specified user with the given account number
    sqlite3_bind_text(stmt, 7, u->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, r->accountNbr);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to update record: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc;
}


int getRecordForUser(const User *u, int accountNbr, Record *r) {
    sqlite3 *db = openDatabase("records.db");
    if (db == NULL)
        return -1;
    
    const char *sqlSelect = "SELECT id, userId, accountNbr, depositDate, country, phone, amount, accountType "
                            "FROM records WHERE username = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Error preparing select statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accountNbr);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        r->id = sqlite3_column_int(stmt, 0);
        r->userId = sqlite3_column_int(stmt, 1);
        r->accountNbr = sqlite3_column_int(stmt, 2);
        const char *dateStr = (const char *)sqlite3_column_text(stmt, 3);
        if (dateStr != NULL) {
            sscanf(dateStr, "%d/%d/%d", &r->deposit.day, &r->deposit.month, &r->deposit.year);
        } else {
            r->deposit.day = r->deposit.month = r->deposit.year = 0;
        }
        const char *country = (const char *)sqlite3_column_text(stmt, 4);
        if (country)
            strncpy(r->country, country, sizeof(r->country) - 1);
        else
            r->country[0] = '\0';
        const char *phone = (const char *)sqlite3_column_text(stmt, 5);
        if (phone)
            strncpy(r->phone, phone, sizeof(r->phone) - 1);
        else
            r->phone[0] = '\0';
        r->amount = sqlite3_column_double(stmt, 6);
        const char *accType = (const char *)sqlite3_column_text(stmt, 7);
        if (accType)
            strncpy(r->accountType, accType, sizeof(r->accountType) - 1);
        else
            r->accountType[0] = '\0';
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0; // Record exists and data loaded
    } else {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1; // Record not found
    }
}


int insertRecordForUser(const User *u, const Record *r) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open("records.db", &db);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot open database: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return rc;
    }

    const char *sqlInsert = "INSERT INTO records (userId, username, accountNbr, depositDate, country, phone, amount, accountType) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to prepare insert statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        return rc;
    }

    sqlite3_bind_int(stmt, 1, u->id);
    sqlite3_bind_text(stmt, 2, u->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, r->accountNbr);

    char depositDate[20];
    snprintf(depositDate, sizeof(depositDate), "%d/%d/%d", r->deposit.day, r->deposit.month, r->deposit.year);
    sqlite3_bind_text(stmt, 4, depositDate, -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 5, r->country, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, r->phone, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 7, r->amount);
    sqlite3_bind_text(stmt, 8, r->accountType, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to insert record: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return rc;
}


int removeRecordForUser(const User *u, int accountNbr) {
    sqlite3 *db = openDatabase("records.db");
    if (db == NULL) {
        return -1;
    }
    
    const char *sqlDelete = "DELETE FROM records WHERE username = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to prepare delete statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        return rc;
    }
    
    sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accountNbr);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to delete record: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
    }
    sqlite3_finalize(stmt);
    
    int changes = sqlite3_changes(db);
    sqlite3_close(db);
    
    // Returns SQLITE_DONE if the deletion was successful and at least one record was deleted.
    return (rc == SQLITE_DONE && changes > 0) ? SQLITE_DONE : -1;
}
