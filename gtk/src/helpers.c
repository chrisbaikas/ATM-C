#include "header.h"
#include <ctype.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>


// --- Hash Password ---
// Calculates the SHA256 hash of the password and returns it as a hexadecimal string.
void hashPassword(const char *password, unsigned char *hashedOutput) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf((char *)hashedOutput + (i * 2), "%02x", hash[i]);
    }
    hashedOutput[SHA256_DIGEST_LENGTH * 2] = '\0';
}

// --- readIntInput ---
// Displays a dialog with a GtkEntry to read an integer.
int readIntInput(void) {
    GtkWidget *dialog, *content_area, *entry;
    int result = 0;
    
    dialog = gtk_dialog_new_with_buttons("Enter Number",
                                           NULL,
                                           GTK_DIALOG_MODAL,
                                           "_OK", GTK_RESPONSE_OK,
                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                           NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        result = atoi(text);
    }
    gtk_widget_destroy(dialog);
    return result;
}

// --- isLeapYear ---
// Returns 1 if the year is a leap year, otherwise 0.
int isLeapYear(int year) {
    return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

// --- isValidDate ---
// Checks if the date is valid.
int isValidDate(int day, int month, int year) {
    if (year < 1000 || year > 9999)
        return 0;
    if (month < 1 || month > 12)
        return 0;
    int maxDay;
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            maxDay = 31; break;
        case 4: case 6: case 9: case 11:
            maxDay = 30; break;
        case 2:
            maxDay = isLeapYear(year) ? 29 : 28; break;
        default:
            return 0;
    }
    if (day < 1 || day > maxDay)
        return 0;
    return 1;
}

// --- readAmount ---
 // Displays a dialog for entering a monetary amount and returns the value as a double.
double readAmount(int allowNegative) {
    GtkWidget *dialog, *content_area, *entry;
    char input[100];
    double amount = 0.0;
    
    dialog = gtk_dialog_new_with_buttons("Amount",
        NULL,
        GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK,
        "_Cancel", GTK_RESPONSE_CANCEL,
        NULL);

    // Get content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Create vertical box to organize label and entry
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Add label above entry
    GtkWidget *label = gtk_label_new("Enter the amount:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    // Create entry with placeholder
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g. 100.00");
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Add vbox into the dialog content area
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    // Show all elements
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        strncpy(input, text, sizeof(input) - 1);
        input[sizeof(input) - 1] = '\0';
        // Replace comma with dot
        for (char *p = input; *p; p++) {
            if (*p == ',') *p = '.';
        }
        if (sscanf(input, "%lf", &amount) != 1) {
            GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                               GTK_DIALOG_MODAL,
                                                               GTK_MESSAGE_ERROR,
                                                               GTK_BUTTONS_OK,
                                                               "Invalid amount. Please try again.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            gtk_widget_destroy(dialog);
            return readAmount(allowNegative);
        }
        if (!allowNegative && amount < 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                               GTK_DIALOG_MODAL,
                                                               GTK_MESSAGE_ERROR,
                                                               GTK_BUTTONS_OK,
                                                               "Amount must be positive. Please try again.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            gtk_widget_destroy(dialog);
            return readAmount(allowNegative);
        }
        char *dot = strchr(input, '.');
        if (dot != NULL) {
            int decimals = strlen(dot + 1);
            if (decimals > 2) {
                GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                                   GTK_DIALOG_MODAL,
                                                                   GTK_MESSAGE_ERROR,
                                                                   GTK_BUTTONS_OK,
                                                                   "Please enter an amount with no more than 2 decimal places.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
                gtk_widget_destroy(dialog);
                return readAmount(allowNegative);
            }
        }
    }
    gtk_widget_destroy(dialog);
    amount = roundToTwoDecimals(amount);
    return amount;
}

// --- readAccountType ---
// Displays a dialog with a ComboBox for selecting the account type and returns the selection.
void readAccountType(char *accountType, int maxLen) {
    GtkWidget *dialog, *content_area, *combo;
    gint response;
    
    dialog = gtk_dialog_new_with_buttons("Select Account Type",
                                           NULL,
                                           GTK_DIALOG_MODAL,
                                           "_OK", GTK_RESPONSE_OK,
                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                           NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "savings");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "current");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fixed01");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fixed02");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fixed03");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(content_area), combo, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        const char *choice = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        if (choice) {
            strncpy(accountType, choice, maxLen - 1);
            accountType[maxLen - 1] = '\0';
        }
    } else {
        accountType[0] = '\0';
    }
    gtk_widget_destroy(dialog);
}

// --- isAlphaString ---
// Returns 1 if the string consists only of alphabetic characters or spaces.
int isAlphaString(const char *s) {
    while (*s) {
        if (!isalpha(*s) && !isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

// --- isValidPhone ---
// Returns 1 if the string is a valid phone number (only digits and optionally a '+' at the beginning).
int isValidPhone(const char *s) {
    if (*s == '\0') return 0;
    if (*s == '+')
        s++;
    while (*s) {
        if (!isdigit((unsigned char)*s))
            return 0;
        s++;
    }
    return 1;
}

// --- hasAtMostTwoDecimals ---
// Returns 1 if the string has at most two decimal digits.
int hasAtMostTwoDecimals(const char *s) {
    const char *dot = strchr(s, '.');
    if (!dot)
        return 1;
    return (strlen(dot + 1) <= 2);
}

// --- roundToTwoDecimals ---
// Rounds the value to two decimals.
double roundToTwoDecimals(double value) {
    return ((int)(value * 100 + 0.5)) / 100.0;
}

void free_callback_data(gpointer data, GClosure *closure) {
    (void)closure;
    MenuCallbackData *d = (MenuCallbackData *)data;
    free(d); 
}


