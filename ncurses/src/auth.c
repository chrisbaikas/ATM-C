#include "header.h"
#include <openssl/sha.h>

// Function to display the login menu
void loginMenu(User *u) {
    char enteredPassword[50];
    unsigned char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];
    int rc; // For error checking

    initscr();
    noecho();
    curs_set(TRUE);
    keypad(stdscr, TRUE);
    start_color();

    // Initialize color pairs
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);

    int col = 10;
    clear();
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(2, col, "======= ATM =======");
    attroff(COLOR_PAIR(2) | A_BOLD);
    mvprintw(4, col, "User Login:");
    
    // Prompt for username
    attron(COLOR_PAIR(4));
    mvprintw(6, col + 2, "Enter username: ");
    refresh();
    echo();
    flushinp();
    getnstr(u->name, 49); // Read username into u->name
    noecho();

    // Prompt for password
    mvprintw(8, col + 2, "Enter password: ");
    refresh();
    readPassword(enteredPassword, 50); // Read password with asterisks

    // Calculate the SHA256 hash of the entered password
    hashPassword(enteredPassword, hashedPass);
    
    // Retrieve the stored hash from the database for the given username
    const char *storedHash = getPassword(*u); // getPassword returns the hash for u->name
    if (strcmp((char *)hashedPass, storedHash) == 0) {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(10, col + 2, "Login successful!");
        attroff(COLOR_PAIR(3) | A_BOLD);
        // Save the hashed password in the User structure
        strcpy(u->password, (char *)hashedPass);
        
        // Retrieve the user's id from the database and update u->id
        sqlite3 *db = openDatabase("users.db");
        if (db != NULL) {
            sqlite3_stmt *stmt;
            const char *sql = "SELECT id FROM users WHERE username = ?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
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
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(10, col + 2, "Login failed! Incorrect password.");
        attroff(COLOR_PAIR(5) | A_BOLD);
    }

    mvprintw(12, col + 2, "Press ENTER to continue...");
    refresh();
    getch();
    endwin();
}


// Function to display the registration menu
void registerMenu(User *u) {
    char username[50];
    char plainPassword[50];
    char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    initscr();
    noecho();
    curs_set(TRUE);
    keypad(stdscr, TRUE);
    start_color();

    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);

    int col = 10;

    while (1) {
        clear();
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(2, col, "======= ATM =======");
        attroff(COLOR_PAIR(2) | A_BOLD);
        mvprintw(4, col, "--- Registration ---");

        attron(COLOR_PAIR(4));
        mvprintw(6, col+2, "Enter a username: ");
        refresh();
        echo();
        flushinp();
        getnstr(username, 49); 

        noecho();
        db = openDatabase("users.db");
        if (db == NULL) exit(1);

        const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
        rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(1);
        }
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(8, col+2, "Username already exists! Try again.");
            attroff(COLOR_PAIR(5) | A_BOLD);
            mvprintw(10, col+2, "Press any key to continue...");
            refresh();
            getch();
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            continue;
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        break;
    }

    mvprintw(8, col+2, "Enter a password: ");
    refresh();
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
        attron(COLOR_PAIR(5));  
        mvprintw(20, 5, "Failed to insert user: %s", sqlite3_errmsg(db));
        attroff(COLOR_PAIR(5));
        refresh(); 
        napms(2000);
        
        endwin();  
        exit(1);
        
    }
    sqlite3_finalize(stmt);

    u->id = (int)sqlite3_last_insert_rowid(db);
    sqlite3_close(db);

    strcpy(u->name, username);
    strcpy(u->password, hashedPass);

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(10, col+2, "Registration successful! You can now login.");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(12, col+2, "Press any key to continue...");
    refresh();
    getch();

    endwin();
}


// Function to retrieve a user's password hash from the database
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
        attron(COLOR_PAIR(5));
        mvprintw(20, 5, "Failed to prepare select statement: %s", sqlite3_errmsg(db));
        attroff(COLOR_PAIR(5));        sqlite3_close(db);
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

