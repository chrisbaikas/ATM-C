#include "header.h"
#include <openssl/sha.h>


// Function loginMenu: accepts two char arrays as arguments for the username and the password (hash).
void loginMenu(User *u) {
    char enteredPassword[50];
    unsigned char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];

    system("clear");
    printf("\n\n\n\t\t\t   Bank Management System\n");
    printf("\t\t\t\tUser Login:\n\n");

    // Prompt for username; store directly in u->name
    printf("\tEnter username: ");
    scanf("%s", u->name);
    clearInputBuffer();

    // Prompt for password (with echo hidden, using our helper function)
    printf("\tEnter password: ");
    readPassword(enteredPassword, 50);

    // Calculate the SHA256 hash of the entered password
    hashPassword(enteredPassword, hashedPass);

    // Retrieve the stored hash from the database for this username
    const char *storedHash = getPassword(*u);
    if (strcmp((char *)hashedPass, storedHash) == 0) {
        printf("\n\tLogin successful!\n");
        strcpy(u->password, (char *)hashedPass);

        // Retrieve the user's id from the database and update u->id
        sqlite3 *db = openDatabase("users.db");
        if (db != NULL) {
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(db, "SELECT id FROM users WHERE username = ?;", -1, &stmt, NULL);
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    u->id = sqlite3_column_int(stmt, 0);
                }
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    } else {
        printf("\n\tLogin failed! Incorrect password.\n");
    }
    
    printf("\tPress ENTER to continue...");
    clearInputBuffer();
}

// Function registerMenu: uses a loop to ensure a unique username
void registerMenu(User *u)
{
    char username[50];
    char plainPassword[50];
    char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    
    while (1) {
        system("clear");
        printf("\n\n\t\t======= ATM =======\n");
        printf("\n\t\t--- Registration ---\n");

        printf("\n\tEnter a username: ");
        scanf("%s", username);
        clearInputBuffer();

        db = openDatabase("users.db");
        if (db == NULL) exit(1);

        const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
        rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "\tFailed to prepare select statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(1);
        }
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            printf("\tUsername already exists! Please choose a different username.\n");
            printf("\tPress ENTER to try again...");
            clearInputBuffer();
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            continue;
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        break;  // unique username
    }
    
    // Prompt for password using readPassword
    printf("\n\tEnter a password: ");
    readPassword(plainPassword, 50);
    hashPassword(plainPassword, (unsigned char *)hashedPass);
    
    db = openDatabase("users.db");
    if (db == NULL) exit(1);
    
    const char *sqlInsert = "INSERT INTO users(username, passwordHash) VALUES(?, ?);";
    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPass, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert user: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
        
    // Retrieve the last inserted row ID for u->id
    u->id = (int) sqlite3_last_insert_rowid(db);
    sqlite3_close(db);
    
    strcpy(u->name, username);
    strcpy(u->password, hashedPass);
    
    printf("\n\tRegistration successful! You can now login.\n");
    printf("\tPress ENTER to continue...");
    clearInputBuffer();
}

// Function getPassword: returns the hashed password for a given username
const char *getPassword(User u)
{
    static char savedPass[100];
    sqlite3 *db = openDatabase("users.db");
    if (db == NULL) exit(1);
    sqlite3_stmt *stmt;
    int rc;
    
    const char *sqlSelect = "SELECT passwordHash FROM users WHERE username = ?;";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_bind_text(stmt, 1, u.name, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *pwd = sqlite3_column_text(stmt, 0);
        strcpy(savedPass, (const char *)pwd);
    } else {
        strcpy(savedPass, "no user found");
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return savedPass;
}

