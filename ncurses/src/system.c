#include "header.h"
#include <string.h>

void createNewAcc(User u) {
    Record r;
    int rc;
    sqlite3 *db;

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Create the records table if it does not exist
    db = openDatabase("records.db");
    if (db == NULL) {
        endwin();
        exit(1);
    }
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS records ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "userId INTEGER, "
                            "username TEXT, "
                            "accountNbr TEXT, "
                            "depositDate TEXT, "
                            "country TEXT, "
                            "phone TEXT, "
                            "amount REAL, "
                            "accountType TEXT"
                            ");";
    rc = sqlite3_exec(db, sqlCreate, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        mvprintw(2, 2, "SQL error creating records table: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        endwin();
        exit(1);
    }
    sqlite3_close(db);

    clear();

    r.userId = u.id;

    attron(COLOR_PAIR(2));
    mvprintw(2, 2, "===== New record =====");
    attroff(COLOR_PAIR(2));
    
    // Date Input
    readDate(&r.deposit.day, &r.deposit.month, &r.deposit.year);

    // Account Number Input
    r.accountNbr = readAccountNumber(u);
       
    // Country Input
    readCountry(r.country, sizeof(r.country));

    // Phone Number Input
    readPhone(r.phone, sizeof(r.phone));

    // Deposit Amount
    r.amount = readAmount(0);

    // Account Type Input
    readAccountType(r.accountType, sizeof(r.accountType));

    // Insert Record
    rc = insertRecordForUser(&u, &r);
    if (rc != SQLITE_DONE) {
        attron(COLOR_PAIR(5));
        mvprintw(2, 5, "Error saving record.");
        attroff(COLOR_PAIR(3));
    }

    // Finalize
    refresh();
    endwin(); // Exit ncurses
    success(u, 4, 5);

}


void updateAccountInfo(User u) {

    int targetAcc;
    Record r;
    int recordFound = 0;
    int row = 5;  // Starting row for prompts
    char input[50];
    char prompt[100];
    char prompt2[100];

    clear();  // Clear the screen
    attron(COLOR_PAIR(2));
    mvprintw(row - 2, 5, "Welcome to Update Account!");
    attroff(COLOR_PAIR(2));
    refresh();

    // Loop until a record is found for the specified account number
    while (!recordFound) {
        // Build and display the prompt on one line
        sprintf(prompt, "Enter the account number you want to update (0 to exit, 1 for main menu): ");
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();
        
        // Position the cursor immediately after the prompt on the same line and get input
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, sizeof(input) - 1);
        noecho();
        // Wait for an extra Enter press (if desired) so that the user commits the input
        // getch();  // Uncomment if you want an additional pause here.
        
        // Parse input as integer
        if (sscanf(input, "%d", &targetAcc) != 1) {
            attron(COLOR_PAIR(5));  // Turn on error color (red)
            mvprintw(row + 2, 5, "Invalid account number input. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;
        }
        
        if (targetAcc == 0) {
            exitProgram();
        }
        if (targetAcc == 1) {
            mainMenu(u);
            return;
        }
        
        // Check if a record exists for this user with the given account number.
        if (getRecordForUser(u, targetAcc, &r) == 0) {
            recordFound = 1;
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "No record found with account number %d for user %s. Please try again.", targetAcc, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
        }
    }  // End while

    // Display current values for the found record
    clear();
    sprintf(prompt2, "Which field do you want to update? (1: Phone, 2: Country): ");
    
    printw("%s", prompt2);

    mvprintw(3, 5, "Record found for account number %d.", targetAcc);
    mvprintw(4, 5, "Current Country: %s", r.country);
    mvprintw(5, 5, "Current Phone: %s", r.phone);
    mvprintw(7, 5, prompt2);
    refresh();

    int choice;
    while (1) {
        echo();
        mvgetnstr(7, 5 + strlen(prompt2), input, sizeof(input) - 1);
        noecho();
        // Optionally, wait for an extra key press:
        // getch();
        if (sscanf(input, "%d", &choice) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "Invalid input for choice. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            // Clear the error message line and reset the input buffer
            move(9, 5);
            clrtoeol();
            refresh();
            input[0] = '\0';  // Reset the input string
            continue;
        }
        if (choice == 1 || choice == 2)
            break;
        else {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "Invalid choice. Please enter 1 or 2.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            // Clear the error message line and reset the input buffer
            move(9, 5);
            clrtoeol();
            refresh();
            input[0] = '\0';  // Reset the input string
        }
        
    }

    char newValue[100];
    if (choice == 1) {
        // Loop to get a valid phone number
        while (1) {
            move(10, 5);
            clrtoeol();
            printw("Enter new phone number: ");
            refresh();
            echo();
            mvgetnstr(10, 5 + strlen("Enter new phone number: "), newValue, sizeof(newValue) - 1);
            noecho();
            // Optionally, wait for an extra key press:
            // getch();
            if (isValidPhone(newValue)) {
                break;
            } else {
                attron(COLOR_PAIR(5));
                mvprintw(11, 5, "Invalid phone number. Please enter only digits and an optional leading '+'.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
            }
        }
        strcpy(r.phone, newValue);
    } else if (choice == 2) {
        // Loop to get a valid country name
        while (1) {
            move(10, 5);
            clrtoeol();
            printw("Enter new country: ");
            refresh();
            echo();
            mvgetnstr(10, 5 + strlen("Enter new country: "), newValue, sizeof(newValue) - 1);
            noecho();
            // Optionally, wait for an extra key press:
            // getch();
            if (isAlphaString(newValue)) {
                break;
            } else {
                attron(COLOR_PAIR(5));
                mvprintw(11, 5, "Invalid country name. Please enter only letters.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
            }
        }
        strcpy(r.country, newValue);
    }
    
    // Update the record in the database using setRecordForUser
    int rc_update = setRecordForUser(&u, &r);
    if (rc_update != SQLITE_DONE) {
        attron(COLOR_PAIR(5));
        mvprintw(13, 5, "Error updating record.");
        attroff(COLOR_PAIR(5));
        refresh();
    } else {
        mvprintw(13, 5, "Record updated successfully in database.");
        refresh();
    }
    
    napms(200);
    success(u, 16, 5);

}


void checkAccountDetail(User u) {
    int targetAcc;
    Record r;  // Record that will be populated by getRecordForUser
    char input[50] = {0};
    int row = 5;  // Starting row for prompt
    char prompt[100];

    // Repeatedly prompt the user until a valid account number is entered and a corresponding record is found.
    int validInput = 0;

    while (!validInput) {

        clear();
        sprintf(prompt, "Enter the account number you want to update: ");
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();
        
        // Position the cursor immediately after the prompt on the same line and get input
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, sizeof(input) - 1);
        noecho();

        // Validate and convert input to integer
        if (sscanf(input, "%d", &targetAcc) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Invalid account number input. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;  // Ask for input again
        }

        // Attempt to load the record for the given account number
        if (getRecordForUser(u, targetAcc, &r) != 0) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "No record found with account number %d for user %s.", targetAcc, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;  // Ask for input again
        }
        
        // If we reach here, input is valid and a record was found
        validInput = 1;
    }

    // Display account details
    clear();
    mvprintw(3, 5, "Account Details for account number %d:", targetAcc);
    mvprintw(4, 5, "Deposit Date: %02d/%02d/%04d", r.deposit.day, r.deposit.month, r.deposit.year);
    mvprintw(5, 5, "Country: %s", r.country);
    mvprintw(6, 5, "Phone: %s", r.phone);
    mvprintw(7, 5, "Amount Deposited: $%.2lf", r.amount);
    mvprintw(8, 5, "Account Type: %s", r.accountType);

    // Interest calculation (for savings and fixed accounts)
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
        mvprintw(9, 5, "You will not get interests because the account is of type current.");
    
    if (rate > 0) {
        double interest = (r.amount * rate / 100.0) / 12;
        mvprintw(9, 5, "You will get $%.2lf as interest on day %d of every month.", interest, r.deposit.day);
    }
    refresh();

    success(u, 12, 5);

}


void checkAllAccounts(User u) {
    // Open the database (using your helper function)
    sqlite3 *db = openDatabase("records.db");
    if (db == NULL) {
        return;
    }

    const char *sqlSelect = "SELECT accountNbr, depositDate, country, phone, amount, accountType FROM records WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        // Display error message in red using ncurses
        attron(COLOR_PAIR(5));
        mvprintw(1, 1, "Failed to prepare select statement: %s", sqlite3_errmsg(db));
        attroff(COLOR_PAIR(5));
        refresh();
        sqlite3_close(db);
        return;
    }

    // Bind the username to the query
    sqlite3_bind_text(stmt, 1, u.name, -1, SQLITE_STATIC);

    // Clear the screen using ncurses
    clear();
    mvprintw(1, 5, "====== All accounts for user: %s =====", u.name);
    refresh();

    int row = 3;  // Starting row for displaying records
    int found = 0;

    // Iterate over each record returned by the query
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        int accountNbr = sqlite3_column_int(stmt, 0);
        const unsigned char *depositDate = sqlite3_column_text(stmt, 1);
        const unsigned char *country = sqlite3_column_text(stmt, 2);
        const unsigned char *phone = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char *accountType = sqlite3_column_text(stmt, 5);

        // Print a separator and the record details on successive lines
        mvprintw(row, 5, "________________________________________");
        row++;
        mvprintw(row++, 5, "Account Number: %d", accountNbr);
        mvprintw(row++, 5, "Deposit Date: %s", depositDate);
        mvprintw(row++, 5, "Country: %s", country);
        mvprintw(row++, 5, "Phone: %s", phone);
        mvprintw(row++, 5, "Amount Deposited: $%.2lf", amount);
        mvprintw(row++, 5, "Account Type: %s", accountType);
        row++;  // Add an extra blank line between records
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!found) {
        mvprintw(row, 5, "No accounts found for user %s.", u.name);
        row++;
    }

    
    refresh();

    // Call success() starting at the next available row and column 5.
    success(u, row , 5);

}


void makeTransaction(User u) {
    int targetAcc, rc, recordId = 0;
    double amount = 0.0, newAmount = 0.0;
    char accountType[50] = "";  // Fixed buffer for the account type
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char input[50];
    int row = 5;  

    clear(); 

    // Opening the database
    db = openDatabase("records.db");
    if (db == NULL) {
        attron(COLOR_PAIR(5));
        mvprintw(row, 5, "Cannot open database.");
        attroff(COLOR_PAIR(5));
        refresh();
        napms(1500);
        return;
    }

    // Continuous attempts by the user to enter a valid account
    while (1) {

        // Clearing buffer
        for (int i = row; i <= row + 2; i++) {
            move(i, 5);
            clrtoeol();
        }
        refresh();

        // Input account number
        mvprintw(row, 5, "Enter the account number for the transaction: ");
        refresh();
        echo();
        mvgetnstr(row, 5 + strlen("Enter the account number for the transaction: "), input, sizeof(input) - 1);
        noecho();
        flushinp();  // Clearing buffer

        // Convert to integer
        if (sscanf(input, "%d", &targetAcc) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Invalid account number input. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;
        }

        // Finding account in the database
        const char *sqlSelect = "SELECT id, amount, accountType FROM records WHERE username = ? AND accountNbr = ?;";
        rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Failed to prepare select statement: %s", sqlite3_errmsg(db));
            attroff(COLOR_PAIR(5));
            refresh();
            sqlite3_close(db);
            return;
        }
        sqlite3_bind_text(stmt, 1, u.name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, targetAcc);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            recordId = sqlite3_column_int(stmt, 0);
            amount = sqlite3_column_double(stmt, 1);
            strncpy(accountType, (const char *)sqlite3_column_text(stmt, 2), sizeof(accountType) - 1);
            accountType[sizeof(accountType) - 1] = '\0';  // Make sure that it is null-terminated
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "No account found with account number %d for user %s. Please try again.", targetAcc, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            sqlite3_finalize(stmt);
            continue;  // Asking for account number again
        }
        sqlite3_finalize(stmt);

        // Checking if it is a fixed account
        if (strncasecmp(accountType, "fixed", 5) == 0) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Transactions are not allowed on fixed accounts. Choose another.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(2000);
            continue;  // Asking for account number again
        }

        // If a valid account is found that is not fixed, we proceed
        break;
    }

    // Transaction type selection
    int choice = transactionTypeMenu(row, 5);

    newAmount = amount;
    if (choice == 1) {  // Deposit
        double depositAmt = readAmount(0);
        newAmount += depositAmt;
        newAmount = roundToTwoDecimals(newAmount);
        mvprintw(row + 2, 5, "Deposit successful. New balance: $%.2lf", newAmount);
        refresh();
        napms(1500);
    } else if (choice == 2) {  // withdraw
        double withdrawAmt;
        while (1) {
            withdrawAmt = readAmount(0);
            if (withdrawAmt < 0) {
                attron(COLOR_PAIR(5));
                mvprintw(row + 2, 5, "Withdrawal amount cannot be negative. Please try again.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
                continue;
            }
            if (withdrawAmt > newAmount) {
                attron(COLOR_PAIR(5));
                mvprintw(row + 2, 5, "Insufficient funds. Please enter a smaller amount.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
                continue;
            } else {
                newAmount -= withdrawAmt;
                newAmount = roundToTwoDecimals(newAmount);
                mvprintw(row + 2, 5, "Withdrawal successful. New balance: $%.2lf", newAmount);
                refresh();
                napms(1500);
                break;
            }
        }
    }

    // Updating the database
    const char *sqlUpdate = "UPDATE records SET amount = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        attron(COLOR_PAIR(5));
        mvprintw(row + 2, 5, "Failed to prepare update statement: %s", sqlite3_errmsg(db));
        attroff(COLOR_PAIR(5));
        refresh();
        sqlite3_close(db);
        return;
    }
    sqlite3_bind_double(stmt, 1, newAmount);
    sqlite3_bind_int(stmt, 2, recordId);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        attron(COLOR_PAIR(5));
        mvprintw(row + 2, 5, "Failed to update record: %s", sqlite3_errmsg(db));
        attroff(COLOR_PAIR(5));
        refresh();
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Successful transaction - calling the success menu
    success(u, 10, 5);
}



void removeAccount(User u) {
    int targetAccount;
    char input[50] = {0};
    int row = 5;  // Starting row for prompts

    clear();  // Clear the ncurses screen

    while (1) {
        // Prompt for account number
        mvprintw(row, 5, "Enter the account number you want to remove: ");
        clrtoeol();
        refresh();

        echo();
        mvgetnstr(row, 5 + strlen("Enter the account number you want to remove: "), input, sizeof(input) - 1);
        noecho();
        flushinp();

        // Validate integer input
        if (sscanf(input, "%d", &targetAccount) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Invalid input. Please enter a valid account number.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            move(row + 2, 0); clrtoeol();  // Clear error message
            continue;
        }

        // Attempt removal
        int rc_db = removeRecordForUser(&u, targetAccount);
        if (rc_db == SQLITE_DONE) {
            attron(COLOR_PAIR(3));
            mvprintw(row + 2, 5, "Account number %d removed successfully.", targetAccount);
            attroff(COLOR_PAIR(3));
            refresh();
            napms(1500);
            break;  // ✅ Success: break loop and go to success()
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "No account found with number %d for user %s.", targetAccount, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            move(row + 2, 0); clrtoeol();  // Clear error for next try
        }
    }

    success(u, row + 4, 5);
}



void transferOwner(User u)
{
    int targetAcc;
    char newOwner[50] = {0};
    int rc;  // For error checking
    int row = 5;  // Starting row for prompts
    char input[50] = {0};

    clear();  // Clear the ncurses screen

    // Loop to get a valid account number that exists for the current user
    while (1) {
        // Clear previous prompt lines
        for (int i = row; i <= row + 2; i++) {
            move(i, 5);
            clrtoeol();
        }
        refresh();

        // Prompt for account number
        mvprintw(row, 5, "Enter the account number you want to transfer (or 0 to return to main menu): ");
        refresh();
        echo();
        mvgetnstr(row, 5 + strlen("Enter the account number you want to transfer (or 0 to return to main menu): "), input, sizeof(input) - 1);
        noecho();
        flushinp();  // Flush any pending input

        // Validate and convert input to integer
        if (sscanf(input, "%d", &targetAcc) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Invalid account number input. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;
        }
        if (targetAcc == 0) {
            mainMenu(u);
            return;
        }
        // Check if a record exists for this user with the given account number.
        sqlite3 *dbRecords = openDatabase("records.db");
        if (dbRecords == NULL) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Cannot open records database.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            return;
        }
        const char *sqlCheck = "SELECT id FROM records WHERE username = ? AND accountNbr = ?;";
        sqlite3_stmt *stmtCheck;
        rc = sqlite3_prepare_v2(dbRecords, sqlCheck, -1, &stmtCheck, NULL);
        if (rc != SQLITE_OK) {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "Failed to prepare check statement: %s", sqlite3_errmsg(dbRecords));
            attroff(COLOR_PAIR(5));
            refresh();
            sqlite3_close(dbRecords);
            napms(1500);
            return;
        }
        sqlite3_bind_text(stmtCheck, 1, u.name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmtCheck, 2, targetAcc);
        if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
            break;  // Record found; exit loop.
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(row + 2, 5, "No account found with account number %d for user %s. Please try again.", targetAcc, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
            continue;
        }
    } // End account number loop

    // Loop to get a valid new owner's username
    int validInput = 0;
    int newOwnerId = -1;
    while (!validInput) {
        mvprintw(8, 5, "Enter the username of the new owner: ");
        refresh();
        echo();
        mvgetnstr(8, 5 + strlen("Enter the username of the new owner: "), input, sizeof(input) - 1);
        noecho();
        flushinp();  // Flush input events

        if (sscanf(input, "%s", newOwner) != 1) {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "Invalid input for username. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;
        }
        if (strcmp(newOwner, u.name) == 0) {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "New owner cannot be the same as the current owner. Try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
            continue;
        }
        // Check if the new username exists in users.db and retrieve its id.
        sqlite3 *dbUsers = openDatabase("users.db");
        if (dbUsers == NULL)
            return;
        const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(dbUsers, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "Failed to prepare select statement: %s", sqlite3_errmsg(dbUsers));
            attroff(COLOR_PAIR(5));
            refresh();
            sqlite3_close(dbUsers);
            napms(1500);
            return;
        }
        sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            newOwnerId = sqlite3_column_int(stmt, 0);
            validInput = 1;
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(9, 5, "User %s does not exist. Please try again.", newOwner);
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(dbUsers);
    }

    // Update the record in the records database: change both username and userId.
    sqlite3 *dbRecords = openDatabase("records.db");
    if (dbRecords == NULL)
        return;
    const char *sqlUpdate = "UPDATE records SET username = ?, userId = ? WHERE username = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt2;
    rc = sqlite3_prepare_v2(dbRecords, sqlUpdate, -1, &stmt2, NULL);
    if (rc != SQLITE_OK) {
        attron(COLOR_PAIR(5));
        mvprintw(10, 5, "Failed to prepare update statement: %s", sqlite3_errmsg(dbRecords));
        attroff(COLOR_PAIR(5));
        refresh();
        sqlite3_close(dbRecords);
        napms(1500);
        return;
    }
    sqlite3_bind_text(stmt2, 1, newOwner, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 2, newOwnerId);
    sqlite3_bind_text(stmt2, 3, u.name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 4, targetAcc);

    // Save sender's username for notifications (global variable senderUsername)
    strcpy(sharedSenderUsername, u.name);

    rc = sqlite3_step(stmt2);
    if (rc != SQLITE_DONE) {
        attron(COLOR_PAIR(5));
        mvprintw(10, 5, "Failed to transfer record: %s", sqlite3_errmsg(dbRecords));
        attroff(COLOR_PAIR(5));
        refresh();
    } else {
        if (sqlite3_changes(dbRecords) > 0) {
            *sharedTransferNotification = 1; // Set notification flag
            strcpy(sharedSenderUsername, u.name);

            attron(COLOR_PAIR(3));
            mvprintw(10, 5, "Account transferred successfully to %s.", newOwner);
            attroff(COLOR_PAIR(3));
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(10, 5, "No account found with account number %d for user %s.", targetAcc, u.name);
            attroff(COLOR_PAIR(5));
            refresh();
        }
    }
    sqlite3_finalize(stmt2);
    sqlite3_close(dbRecords);

    success(u, 13, 10);
}
