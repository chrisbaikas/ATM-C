#include "header.h"
#include <openssl/sha.h>


// Function loginMenu: accepts two char arrays as arguments for the username and the password (hash).
void loginMenu(User *u) {
    while (1) {
        GtkWidget *dialog, *content_area, *grid;
        GtkWidget *username_label, *password_label;
        GtkWidget *username_entry, *password_entry;
        gint response;

        // Creating a login dialog
        dialog = gtk_dialog_new_with_buttons("User Login",
                                               NULL,
                                               GTK_DIALOG_MODAL,
                                               "_Login", GTK_RESPONSE_OK,
                                               "_Cancel", GTK_RESPONSE_CANCEL,
                                               NULL);
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Creating a grid for placing labels and entries
        grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
        gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
        gtk_container_add(GTK_CONTAINER(content_area), grid);

        username_label = gtk_label_new("Username:");
        password_label = gtk_label_new("Password:");

        username_entry = gtk_entry_new();
        password_entry = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE); // Hiding text
        gtk_entry_set_invisible_char(GTK_ENTRY(password_entry), '*');

        gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

        gtk_widget_show_all(dialog);

        response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return;  // If canceled, the login attempt terminates.
        }

        const char *entered_username = gtk_entry_get_text(GTK_ENTRY(username_entry));
        const char *entered_password = gtk_entry_get_text(GTK_ENTRY(password_entry));

        // Saving the username in the structure u
        strncpy(u->name, entered_username, sizeof(u->name));
        u->name[sizeof(u->name) - 1] = '\0';

        // Calculating hash for the password
        unsigned char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];
        hashPassword(entered_password, hashedPass);

        // Retrieving stored hash for the specific username
        const char *storedHash = getPassword(u);
        if (strcmp((char *)hashedPass, storedHash) == 0) {
            strcpy(u->password, (char *)hashedPass);

            // Retrieving the user id from the database and updating u->id
            sqlite3 *db = openDatabase("users.db");
            if (db != NULL) {
                sqlite3_stmt *stmt;
                const char *sql = "SELECT id FROM users WHERE username = ?;";
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        u->id = sqlite3_column_int(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            }

            gtk_widget_destroy(dialog);

            create_main_menu(u);
            return;  // Exiting the loop after the login is successful
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                               GTK_DIALOG_MODAL,
                                                               GTK_MESSAGE_ERROR,
                                                               GTK_BUTTONS_OK,
                                                               "Wrong username or password. Please try again.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            gtk_widget_destroy(dialog);
            //The loop repeats for a new login attempt.
        }
    }
}


// Function registerMenu: uses a loop to ensure a unique username
void registerMenu(User *u) {
    GtkWidget *dialog, *content_area, *grid;
    GtkWidget *username_label, *password_label;
    GtkWidget *username_entry, *password_entry;
    gint response;
    char username[50];
    char plainPassword[50];
    char hashedPass[SHA256_DIGEST_LENGTH * 2 + 1];
    int rc;

    int unique = 0;

    // Repeating until a unique username is provided
    while (!unique) {
        dialog = gtk_dialog_new_with_buttons("User Registration",
                                               NULL,
                                               GTK_DIALOG_MODAL,
                                               "_Register", GTK_RESPONSE_OK,
                                               "_Cancel", GTK_RESPONSE_CANCEL,
                                               NULL);
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        
        grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
        gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
        gtk_container_add(GTK_CONTAINER(content_area), grid);

        username_label = gtk_label_new("Username:");
        password_label = gtk_label_new("Password:");

        username_entry = gtk_entry_new();
        password_entry = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
        gtk_entry_set_invisible_char(GTK_ENTRY(password_entry), '*');

        gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);

        gtk_widget_show_all(dialog);

        response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_OK) {
            const char *entered_username = gtk_entry_get_text(GTK_ENTRY(username_entry));
            const char *entered_password = gtk_entry_get_text(GTK_ENTRY(password_entry));

            strncpy(username, entered_username, sizeof(username));
            username[sizeof(username)-1] = '\0';

            strncpy(plainPassword, entered_password, sizeof(plainPassword));
            plainPassword[sizeof(plainPassword)-1] = '\0';

            gtk_widget_destroy(dialog);

            // Checking if the username is unique in the database
            sqlite3 *db = openDatabase("users.db");
            if (db == NULL) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_ERROR,
                                                          GTK_BUTTONS_OK,
                                                          "Error opening users database.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                exit(1);
            }
            sqlite3_stmt *stmt;
            const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
            rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_ERROR,
                                                          GTK_BUTTONS_OK,
                                                          "Failed to prepare select statement: %s", sqlite3_errmsg(db));
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                sqlite3_close(db);
                exit(1);
            }
            sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                // The username already exists
                GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                                 GTK_DIALOG_MODAL,
                                                                 GTK_MESSAGE_ERROR,
                                                                 GTK_BUTTONS_OK,
                                                                 "Username already exists! Please choose a different username.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                continue; // Re-displaying the dialog
            }
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            unique = 1;
        } else {
            gtk_widget_destroy(dialog);
            return; // Cancel record
        }
    }

    // Hash of the password
    hashPassword(plainPassword, (unsigned char *)hashedPass);

    sqlite3 *db = openDatabase("users.db");
    if (db == NULL) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Error opening users database.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        exit(1);
    }
    sqlite3_stmt *stmt;
    const char *sqlInsert = "INSERT INTO users(username, passwordHash) VALUES(?, ?);";
    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Failed to prepare insert statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPass, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Failed to insert user: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    }
    sqlite3_finalize(stmt);
    
    // Retrieving the last inserted id
    u->id = (int) sqlite3_last_insert_rowid(db);
    sqlite3_close(db);

    strcpy(u->name, username);
    strcpy(u->password, hashedPass);

    create_main_menu(u);
}


// Function getPassword: returns the hashed password for a given username
const char *getPassword(User *u)
{
    static char savedPass[100];
    sqlite3 *db = openDatabase("users.db");
    if (db == NULL) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Cannot open database users.db");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        exit(1);
    }
    sqlite3_stmt *stmt;
    int rc;
    
    const char *sqlSelect = "SELECT passwordHash FROM users WHERE username = ?;";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to prepare select statement: %s", sqlite3_errmsg(db));
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
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
