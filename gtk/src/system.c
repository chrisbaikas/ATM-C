#include "header.h"
#include <ctype.h>


void createNewAcc(User *u) {
    Record r;
    int rc;
    sqlite3 *db;

    db = openDatabase("records.db");
    if (db == NULL) {
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         "Database error: cannot open records.db");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        exit(1);
    }

    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS records ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "userId INTEGER, "
                            "username TEXT, "
                            "accountNbr INTEGER, "
                            "depositDate TEXT, "
                            "country TEXT, "
                            "phone TEXT, "
                            "amount REAL, "
                            "accountType TEXT"
                            ");";
    rc = sqlite3_exec(db, sqlCreate, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "SQL error creating records table: %s", sqlite3_errmsg(db));
        GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         "%s", errorMsg);
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_close(db);

    // --- Date ---
    while (1) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Date Entry",
            NULL,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Cancel", GTK_RESPONSE_CANCEL,
            NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Create a vertical box to hold the label and the entry
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        // Create and add a label above the entry
        GtkWidget *label = gtk_label_new("Enter Deposit Date - dd/mm/yyyy");
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

        // Create and add the entry field with a placeholder
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "dd/mm/yyyy");
        gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

        // Add the vbox to the dialog's content area
        gtk_container_add(GTK_CONTAINER(content_area), vbox);

        gtk_widget_show_all(dialog);

        gint res = gtk_dialog_run(GTK_DIALOG(dialog));

        if (res == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (sscanf(text, "%d/%d/%d", &r.deposit.day, &r.deposit.month, &r.deposit.year) == 3 &&
                isValidDate(r.deposit.day, r.deposit.month, r.deposit.year)) {
                gtk_widget_destroy(dialog);
                break;
            } else {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Invalid date. Use dd/mm/yyyy format.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    // --- Account Number ---
    while (1) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Account Entry",
            NULL,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Cancel", GTK_RESPONSE_CANCEL,
            NULL);

        // Get the content area of the dialog
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Create a vertical box to hold label + entry
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        // Add label above entry
        GtkWidget *label = gtk_label_new("Enter Account Number");
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

        // Create entry field with placeholder
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., 12345");
        gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

        // Add the vbox to the dialog
        gtk_container_add(GTK_CONTAINER(content_area), vbox);

        // Show everything
        gtk_widget_show_all(dialog);

        // Run dialog and capture result
        gint res = gtk_dialog_run(GTK_DIALOG(dialog));

        if (res == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            
            // Checking if it contains only numbers
            int validInput = 1;
            for (int i = 0; text[i] != '\0'; i++) {
                if (!isdigit((unsigned char)text[i])) {
                    validInput = 0;
                    break;
                }
            }
    
            if (!validInput || strlen(text) == 0) {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Invalid input. Please enter only digits for the account number.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                gtk_widget_destroy(dialog);
                continue;
            }
    
            r.accountNbr = atoi(text);
    
            // Checking if the account already exists
            Record temp;
            if (getRecordForUser(u, r.accountNbr, &temp) != 0) {
                gtk_widget_destroy(dialog);
                break;
            } else {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Account already exists. Choose a different number.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }
    

    // --- Country ---
    while (1) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Country Entry",
            NULL,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Cancel", GTK_RESPONSE_CANCEL,
            NULL);

        // Get the content area
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Create a vertical container (vbox)
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        // Add label above entry
        GtkWidget *label = gtk_label_new("Enter Country:");
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

        // Add entry field with placeholder
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., Greece");
        gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

        // Add vbox to the dialog
        gtk_container_add(GTK_CONTAINER(content_area), vbox);

        // Show everything
        gtk_widget_show_all(dialog);

        // Run dialog and capture response
        gint res = gtk_dialog_run(GTK_DIALOG(dialog));

        if (res == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (isAlphaString(text)) {
                strncpy(r.country, text, sizeof(r.country) - 1);
                r.country[sizeof(r.country) - 1] = '\0';
                gtk_widget_destroy(dialog);
                break;
            } else {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Invalid country name.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    // --- Phone Number ---
    while (1) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Phone Entry",
            NULL,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Cancel", GTK_RESPONSE_CANCEL,
            NULL);

        // Get the content area
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Create a vertical box to hold label + entry
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        // Label above the entry
        GtkWidget *label = gtk_label_new("Enter phone number:");
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

        // Entry with placeholder
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., +306912345678");
        gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

        // Add everything to dialog
        gtk_container_add(GTK_CONTAINER(content_area), vbox);

        // Show UI elements
        gtk_widget_show_all(dialog);

        // Run the dialog
        gint res = gtk_dialog_run(GTK_DIALOG(dialog));

        if (res == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            if (isValidPhone(text)) {
                strncpy(r.phone, text, sizeof(r.phone) - 1);
                r.phone[sizeof(r.phone) - 1] = '\0';
                gtk_widget_destroy(dialog);
                break;
            } else {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Invalid phone number format.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    // --- Amount ---
    r.amount = readAmount(0);

    // --- Account Type ---
    readAccountType(r.accountType, sizeof(r.accountType));

    // --- Save to DB ---
    rc = insertRecordForUser(u, &r);
    if (rc != SQLITE_DONE) {
        GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                "Error saving record to database.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    } else {
        GtkWidget *info = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                 "Record inserted successfully.");
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    }

}


void updateAccountInfo(User *u) {
    int targetAcc = 0;
    Record r;
    int recordFound = 0;
    char input[50];

    // Input account number via GTK+ dialog
    while (!recordFound) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Update Account Info",
            NULL,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Cancel", GTK_RESPONSE_CANCEL,
            NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        GtkWidget *label = gtk_label_new("Please enter the account number:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., 12345");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            strncpy(input, text, sizeof(input) - 1);
            input[sizeof(input) - 1] = '\0';
            if (sscanf(input, "%d", &targetAcc) != 1) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Invalid account number input. Please try again.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                gtk_widget_destroy(dialog);
                continue;
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);

        if (getRecordForUser(u, targetAcc, &r) == 0) {
            recordFound = 1;
        } else {
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "No record found with account number %d for user %s. Please try again.",
                                                     targetAcc, u->name);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
        }
    }

    // Display current values of the record 
    char infoMsg[256];
    snprintf(infoMsg, sizeof(infoMsg),
             "Record found for account number %d.\nCurrent Country: %s\nCurrent Phone: %s",
             targetAcc, r.country, r.phone);
    GtkWidget *info_dialog = gtk_message_dialog_new(NULL,
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "%s", infoMsg);
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);

    // Selecting field to update via dialog
    GtkWidget *choice_dialog = gtk_dialog_new_with_buttons("Update Field",
                                                           NULL,
                                                           GTK_DIALOG_MODAL,
                                                           "Phone number", GTK_RESPONSE_ACCEPT,
                                                           "Country", GTK_RESPONSE_REJECT,
                                                           NULL);
    gint choice_response = gtk_dialog_run(GTK_DIALOG(choice_dialog));
    gtk_widget_destroy(choice_dialog);
    int choice = 0;
    if (choice_response == GTK_RESPONSE_ACCEPT)
        choice = 1;
    else if (choice_response == GTK_RESPONSE_REJECT)
        choice = 2;
    else
        return;

    char newValue[100];
    // Dialog for entering a new value
    GtkWidget *newValDialog = gtk_dialog_new_with_buttons("Enter New Value",
                                                          NULL,
                                                          GTK_DIALOG_MODAL,
                                                          "_OK", GTK_RESPONSE_OK,
                                                          "_Cancel", GTK_RESPONSE_CANCEL,
                                                          NULL);
    GtkWidget *new_content = gtk_dialog_get_content_area(GTK_DIALOG(newValDialog));
    GtkWidget *new_entry = gtk_entry_new();
    if (choice == 1)
        gtk_entry_set_placeholder_text(GTK_ENTRY(new_entry), "Enter new phone number");
    else
        gtk_entry_set_placeholder_text(GTK_ENTRY(new_entry), "Enter new country");
    gtk_box_pack_start(GTK_BOX(new_content), new_entry, TRUE, TRUE, 10);
    gtk_widget_show_all(newValDialog);
    
    gint response2 = gtk_dialog_run(GTK_DIALOG(newValDialog));
    if (response2 == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(new_entry));
        strncpy(newValue, text, sizeof(newValue) - 1);
        newValue[sizeof(newValue) - 1] = '\0';
    } else {
        gtk_widget_destroy(newValDialog);
        return;
    }
    gtk_widget_destroy(newValDialog);

    if (choice == 1) {
        // Repeating until a valid phone number is provided
        while (1) {
            if (!isValidPhone(newValue)) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Invalid phone number. Please enter only digits and an optional leading '+'.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                // New dialog for entering a phone number
                GtkWidget *newValDialog = gtk_dialog_new_with_buttons("Enter New Phone Number",
                                                                        NULL,
                                                                        GTK_DIALOG_MODAL,
                                                                        "_OK", GTK_RESPONSE_OK,
                                                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                                                        NULL);
                GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(newValDialog));
                GtkWidget *entry = gtk_entry_new();
                gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "New phone number");
                gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);
                gtk_widget_show_all(newValDialog);
                gint response = gtk_dialog_run(GTK_DIALOG(newValDialog));
                if (response == GTK_RESPONSE_OK) {
                    const char *new_text = gtk_entry_get_text(GTK_ENTRY(entry));
                    strncpy(newValue, new_text, sizeof(newValue) - 1);
                    newValue[sizeof(newValue) - 1] = '\0';
                } else {
                    gtk_widget_destroy(newValDialog);
                    return;  // If canceled, it terminates the attempt
                }
                gtk_widget_destroy(newValDialog);
                // Repeating the loop to check the new value
                continue;
            } else {
                strcpy(r.phone, newValue);
                break;
            }
        }
    } else if (choice == 2) {
        // Repeating until a valid country name is provided
        while (1) {
            if (!isAlphaString(newValue)) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Invalid country name. Please enter only letters.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                // New dialog for entering the country name
                GtkWidget *newValDialog = gtk_dialog_new_with_buttons("Enter New Country",
                                                                        NULL,
                                                                        GTK_DIALOG_MODAL,
                                                                        "_OK", GTK_RESPONSE_OK,
                                                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                                                        NULL);
                GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(newValDialog));
                GtkWidget *entry = gtk_entry_new();
                gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "New country");
                gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);
                gtk_widget_show_all(newValDialog);
                gint response = gtk_dialog_run(GTK_DIALOG(newValDialog));
                if (response == GTK_RESPONSE_OK) {
                    const char *new_text = gtk_entry_get_text(GTK_ENTRY(entry));
                    strncpy(newValue, new_text, sizeof(newValue) - 1);
                    newValue[sizeof(newValue) - 1] = '\0';
                } else {
                    gtk_widget_destroy(newValDialog);
                    return;
                }
                gtk_widget_destroy(newValDialog);
                continue;
            } else {
                strcpy(r.country, newValue);
                break;
            }
        }
    }

    int rc_update = setRecordForUser(u, &r);
    if (rc_update != SQLITE_DONE) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Error updating record.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    } else {
        GtkWidget *succ = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "Record updated successfully in database.");
        gtk_dialog_run(GTK_DIALOG(succ));
        gtk_widget_destroy(succ);
    }

}


void checkAccountDetail(User *u) {
    int targetAcc;
    Record r;
    char accountStr[50];
    gboolean valid = FALSE;
    
    while (!valid) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Check Account Detail",
                NULL,
                GTK_DIALOG_MODAL,
                "_OK", GTK_RESPONSE_OK,
                "_Cancel", GTK_RESPONSE_CANCEL,
                NULL);
    
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
        // Label
        GtkWidget *label = gtk_label_new("Please enter the account number to check:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);
    
        // Entry
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter account number");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);
    
        gtk_widget_show_all(dialog);
    
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
        if (response == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            strncpy(accountStr, text, sizeof(accountStr) - 1);
            accountStr[sizeof(accountStr) - 1] = '\0';
            targetAcc = atoi(accountStr);
            gtk_widget_destroy(dialog);
    
            // Try to find record
            if (getRecordForUser(u, targetAcc, &r) != 0) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                        GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_OK,
                                                        "No account found with account number %d for user %s.\nTry again.",
                                                        targetAcc, u->name);
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else {
                valid = TRUE;  // Success
            }
    
        } else {
            gtk_widget_destroy(dialog);  // User cancelled
            return;
        }
    }    

    // Creating a message with the account details
    char details[512];
    snprintf(details, sizeof(details),
             "Account Details:\n"
             "Account Number: %d\n"
             "Deposit Date: %02d/%02d/%04d\n"
             "Country: %s\n"
             "Phone: %s\n"
             "Amount Deposited: $%.2lf\n"
             "Account Type: %s\n",
             r.accountNbr,
             r.deposit.day, r.deposit.month, r.deposit.year,
             r.country, r.phone,
             r.amount,
             r.accountType);

    // Calculating interest, if applicable
    double interest = 0.0;
    int rate = 0;
    if (strcasecmp(r.accountType, "savings") == 0)
        rate = 7;
    else if (strcasecmp(r.accountType, "fixed01") == 0)
        rate = 4;
    else if (strcasecmp(r.accountType, "fixed02") == 0)
        rate = 5;
    else if (strcasecmp(r.accountType, "fixed03") == 0)
        rate = 8;
    else if (strcasecmp(r.accountType, "current") == 0)
        strncat(details, "You will not get interests because the account is of type current.\n", sizeof(details) - strlen(details) - 1);

    if (rate > 0) {
        interest = (r.amount * rate / 100.0) / 12;
        char interestStr[128];
        snprintf(interestStr, sizeof(interestStr),
                 "You will get $%.2lf as interest on day %d of every month.\n",
                 interest, r.deposit.day);
        strncat(details, interestStr, sizeof(details) - strlen(details) - 1);
    }

    // Displaying account details
    GtkWidget *info_dialog = gtk_message_dialog_new(NULL,
                                                    GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "%s", details);
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);

}


void checkAllAccounts(User *u) {
    sqlite3 *db = openDatabase("records.db");
    if (db == NULL) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Error opening database.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }
    
    const char *sqlSelect = "SELECT accountNbr, depositDate, country, phone, amount, accountType FROM records WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "Failed to prepare select statement: %s", sqlite3_errmsg(db));
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "%s", errorMsg);
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
    
    // Instead of printing to the terminal, we accumulate the details in a buffer.
    char details[4096];
    details[0] = '\0';
    snprintf(details, sizeof(details), "====== All accounts for user: %s =====\n\n", u->name);
    
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        int accountNbr = sqlite3_column_int(stmt, 0);
        const unsigned char *depositDate = sqlite3_column_text(stmt, 1);
        const unsigned char *country = sqlite3_column_text(stmt, 2);
        const unsigned char *phone = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char *accountType = sqlite3_column_text(stmt, 5);
        
        char accountInfo[512];
        snprintf(accountInfo, sizeof(accountInfo),
                 "----------------------------------------\n"
                 "Account Number: %d\n"
                 "Deposit Date: %s\n"
                 "Country: %s\n"
                 "Phone: %s\n"
                 "Amount Deposited: $%.2lf\n"
                 "Account Type: %s\n",
                 accountNbr,
                 depositDate,
                 country,
                 phone,
                 amount,
                 accountType);
        strncat(details, accountInfo, sizeof(details) - strlen(details) - 1);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (!found) {
        GtkWidget *info = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "No accounts found for user %s.", u->name);
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    } else {
        // Displaying the details in a dialog with a scrolled window
        GtkWidget *dialog = gtk_dialog_new_with_buttons("All Accounts",
                                                          NULL,
                                                          GTK_DIALOG_MODAL,
                                                          "_OK", GTK_RESPONSE_OK,
                                                          NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_set_size_request(scrolled_window, 500, 400);
        gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);
        
        GtkWidget *text_view = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
        gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_set_text(buffer, details, -1);
        
        gtk_widget_show_all(dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
}


void makeTransaction(User *u) {
    int targetAcc = 0, rc, found = 0, recordId = 0;
    double amount = 0.0, newAmount = 0.0;
    char accountType[50] = "";
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char input[50];

    // Opening the database 
    db = openDatabase("records.db");
    if (db == NULL) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Error: Cannot open database.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }

    // Repeated dialog until a valid account is found
    while (!found) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Make Transaction",
                    NULL,
                    GTK_DIALOG_MODAL,
                    "_OK", GTK_RESPONSE_OK,
                    "_Cancel", GTK_RESPONSE_CANCEL,
                    NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        GtkWidget *label = gtk_label_new("Please enter the account number to make a transaction:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., 12345");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            strncpy(input, text, sizeof(input) - 1);
            input[sizeof(input) - 1] = '\0';
            if (sscanf(input, "%d", &targetAcc) != 1) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Invalid account number input. Please try again.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                gtk_widget_destroy(dialog);
                continue;
            }
        } else {
            gtk_widget_destroy(dialog);
            sqlite3_close(db);
            return;
        }
        gtk_widget_destroy(dialog);

        // Searching for an account in the database
        const char *sqlSelect = "SELECT id, amount, accountType FROM records WHERE username = ? AND accountNbr = ?;";
        rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "Error: Failed to prepare select statement.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            sqlite3_close(db);
            return;
        }
        sqlite3_bind_text(stmt, 1, u->name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, targetAcc);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            found = 1;
            recordId = sqlite3_column_int(stmt, 0);
            amount = sqlite3_column_double(stmt, 1);
            strncpy(accountType, (const char *)sqlite3_column_text(stmt, 2), sizeof(accountType) - 1);
            accountType[sizeof(accountType) - 1] = '\0';
        } else {
            char msg[256];
            snprintf(msg, sizeof(msg), "No account found with account number %d for user %s. Please try again.", targetAcc, u->name);
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "%s", msg);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
        }
        sqlite3_finalize(stmt);
    }

    // Checking for fixed account
    if (strncasecmp(accountType, "fixed", 5) == 0) {
        GtkWidget *info = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "Transactions are not allowed on fixed accounts.");
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
        sqlite3_close(db);
        return;
    }

    // Transaction type selection via dialog
    GtkWidget *typeDialog = gtk_dialog_new_with_buttons("Transaction Type",
                                                        NULL,
                                                        GTK_DIALOG_MODAL,
                                                        "Deposit", GTK_RESPONSE_ACCEPT,
                                                        "Withdraw", GTK_RESPONSE_REJECT,
                                                        NULL);
    gint typeResponse = gtk_dialog_run(GTK_DIALOG(typeDialog));
    gtk_widget_destroy(typeDialog);
    int choice = 0;
    if (typeResponse == GTK_RESPONSE_ACCEPT)
        choice = 1;   // Deposit
    else if (typeResponse == GTK_RESPONSE_REJECT)
        choice = 2;   // Withdraw
    else {
        sqlite3_close(db);
        return;
    }

    // Transaction process
    if (choice == 1) {
        double depositAmt = readAmount(0);  // Using GTK+ readAmount
        newAmount = amount + depositAmt;
        char msg[128];
        snprintf(msg, sizeof(msg), "Deposit successful. New balance: $%.2lf", newAmount);
        GtkWidget *info = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "%s", msg);
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    } else {
        double withdrawAmt = 0.0;
        while (1) {
            withdrawAmt = readAmount(0);
            if (withdrawAmt > amount) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Insufficient funds. Please enter a smaller amount.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            }
            newAmount = amount - withdrawAmt;
            char msg[128];
            snprintf(msg, sizeof(msg), "Withdrawal successful. New balance: $%.2lf", newAmount);
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "%s", msg);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
            break;
        }
    }

    // Updating the database

    const char *sqlUpdate = "UPDATE records SET amount = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Error: Failed to prepare update statement.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        sqlite3_close(db);
        return;
    }
    sqlite3_bind_double(stmt, 1, newAmount);
    sqlite3_bind_int(stmt, 2, recordId);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Failed to update record.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    GtkWidget *finalInfo = gtk_message_dialog_new(NULL,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "\nTransaction completed successfully.");
    gtk_dialog_run(GTK_DIALOG(finalInfo));
    gtk_widget_destroy(finalInfo);

}


void removeAccount(User *u) {
    int targetAccount = 0;
    char input[50];

    // Creating a dialog for entering the account number 
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Remove Account",
                NULL,
                GTK_DIALOG_MODAL,
                "_OK", GTK_RESPONSE_OK,
                "_Cancel", GTK_RESPONSE_CANCEL,
                NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // 🔽 Προσθήκη Label
        GtkWidget *label = gtk_label_new("Please enter the account number to remove:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., 12345");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        strncpy(input, text, sizeof(input) - 1);
        input[sizeof(input) - 1] = '\0';
        if (sscanf(input, "%d", &targetAccount) != 1) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "Invalid account number input.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            gtk_widget_destroy(dialog);
            return;
        }
    } else {
        gtk_widget_destroy(dialog);
        return;
    }
    gtk_widget_destroy(dialog);

    // Calling removeRecordForUser to delete the record
    int rc = removeRecordForUser(u, targetAccount);
    if (rc == SQLITE_DONE) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Account number %d removed successfully.", targetAccount);
        GtkWidget *info = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "%s", msg);
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "No account found with account number %d for user %s.", targetAccount, u->name);
        GtkWidget *info = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "%s", msg);
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);
    }
    
}



void transferOwner(User *u) {
    int targetAcc = 0;
    char newOwner[50] = "";
    int rc;
    
    // Repeated loop for entering the account number 
    while (1) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Transfer Ownership",
                    NULL,
                    GTK_DIALOG_MODAL,
                    "_OK", GTK_RESPONSE_OK,
                    "_Cancel", GTK_RESPONSE_CANCEL,
                    NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Add label above entry
        GtkWidget *label = gtk_label_new("Please enter the account number to transfer:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

        // Entry field
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter account number (0 = Main Menu)");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        char input[50];
        if (response == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            strncpy(input, text, sizeof(input) - 1);
            input[sizeof(input) - 1] = '\0';
            if (sscanf(input, "%d", &targetAcc) != 1) {
                GtkWidget *err = gtk_message_dialog_new(NULL,
                                                         GTK_DIALOG_MODAL,
                                                         GTK_MESSAGE_ERROR,
                                                         GTK_BUTTONS_OK,
                                                         "Invalid input for account number. Please try again.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                gtk_widget_destroy(dialog);
                continue;
            }
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
      
        
        // Checking for the existence of a record for user u with the specified account number
        sqlite3 *dbRecords = openDatabase("records.db");
        if (dbRecords == NULL) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "Error opening records database.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            return;
        }
        const char *sqlCheck = "SELECT id FROM records WHERE username = ? AND accountNbr = ?;";
        sqlite3_stmt *stmtCheck;
        rc = sqlite3_prepare_v2(dbRecords, sqlCheck, -1, &stmtCheck, NULL);
        if (rc != SQLITE_OK) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "Failed to prepare check statement: %s", sqlite3_errmsg(dbRecords));
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            sqlite3_close(dbRecords);
            return;
        }
        sqlite3_bind_text(stmtCheck, 1, u->name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmtCheck, 2, targetAcc);
        if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
            break;  // Record found.
        } else {
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "No account found with account number %d for user %s.\nPlease try again.",
                                                     targetAcc, u->name);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
        }
    } // End loop account number

    // Loop for entering the new username (newOwner)
    int validInput = 0;
    int newOwnerId = -1;
    while (!validInput) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Transfer Ownership",
                    NULL,
                    GTK_DIALOG_MODAL,
                    "_OK", GTK_RESPONSE_OK,
                    "_Cancel", GTK_RESPONSE_CANCEL,
                    NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Add label above entry
        GtkWidget *label = gtk_label_new("Please enter the new owner's username:");
        gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 5);

        // Entry field
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter new owner's username");
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 10);

        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
            strncpy(newOwner, text, sizeof(newOwner) - 1);
            newOwner[sizeof(newOwner) - 1] = '\0';
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
        
        if (strcmp(newOwner, u->name) == 0) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "New owner cannot be the same as the current owner. Please try again.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            continue;
        }
        
        // Checking if the new user exists in the users.db database
        sqlite3 *dbUsers = openDatabase("users.db");
        if (dbUsers == NULL)
            return;
        const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(dbUsers, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            GtkWidget *err = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     "Failed to prepare select statement: %s", sqlite3_errmsg(dbUsers));
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            sqlite3_close(dbUsers);
            return;
        }
        sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            newOwnerId = sqlite3_column_int(stmt, 0);
            validInput = 1;
        } else {
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "User %s does not exist. Please try again.", newOwner);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(dbUsers);
    }
    
    // Updating the record in the records.db database for ownership change
    sqlite3 *dbRecords = openDatabase("records.db");
    if (dbRecords == NULL)
        return;
    const char *sqlUpdate = "UPDATE records SET username = ?, userId = ? WHERE username = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt2;
    rc = sqlite3_prepare_v2(dbRecords, sqlUpdate, -1, &stmt2, NULL);
    if (rc != SQLITE_OK) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Failed to prepare update statement: %s", sqlite3_errmsg(dbRecords));
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        sqlite3_close(dbRecords);
        return;
    }
    sqlite3_bind_text(stmt2, 1, newOwner, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 2, newOwnerId);
    sqlite3_bind_text(stmt2, 3, u->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 4, targetAcc);

    strcpy(senderUsername, u->name);

    rc = sqlite3_step(stmt2);
    if (rc != SQLITE_DONE) {
        GtkWidget *err = gtk_message_dialog_new(NULL,
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "Failed to transfer record: %s", sqlite3_errmsg(dbRecords));
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    } else {
        if (sqlite3_changes(dbRecords) > 0) {
            *sharedTransferNotification = 1;  // Setting the flag for the notification thread
            strncpy(sharedSenderUsername, u->name, 49);
            sharedSenderUsername[49] = '\0'; 

            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "Account transferred successfully to %s.", newOwner);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
        } else {
            GtkWidget *info = gtk_message_dialog_new(NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK,
                                                     "No account found with account number %d for user %s.", targetAcc, u->name);
            gtk_dialog_run(GTK_DIALOG(info));
            gtk_widget_destroy(info);
        }
    }
    sqlite3_finalize(stmt2);
    sqlite3_close(dbRecords);
    
}
