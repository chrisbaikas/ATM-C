#include "header.h"

void createNewAcc(User u)
{
  Record r;
    int rc;
    sqlite3 *db;

    // Create the records table if it does not exist
    db = openDatabase("records.db");
    if (db == NULL) {
        safeExit(1);
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
        fprintf(stderr, "SQL error creating records table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        safeExit(1);
    }
    sqlite3_close(db);

    system("clear");
    printf("\t\t\t===== New record =====\n");

    /* Automatically retrieves the current system date from the system clock 
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    r.deposit.day = tm_now->tm_mday;
    r.deposit.month = tm_now->tm_mon + 1;
    r.deposit.year = tm_now->tm_year + 1900;
    printf("\nToday's date automatically set to: %02d/%02d/%04d\n",
           r.deposit.day, r.deposit.month, r.deposit.year);
    */

    int valid = 0;
    while (!valid) {
        printf("\nEnter today's date (dd/mm/yyyy): ");
        if (scanf("%d/%d/%d", &r.deposit.day, &r.deposit.month, &r.deposit.year) != 3) {
            printf("Invalid date format. Please try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (!isValidDate(r.deposit.day, r.deposit.month, r.deposit.year)) {
            printf("\nInvalid date values. Please ensure the month is between 1 and 12, the day is valid for that month, and the year is 4 digits.\n");
            continue;
        }
        valid = 1;
    }

    // Input of the account number with an existence check
    valid = 0;
    do {
        printf("\nEnter the account number: ");
        if (scanf("%d", &r.accountNbr) != 1) {
            printf("Invalid account number. Try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        // If getRecordForUser returns 0, it means that the record already exists
        if (getRecordForUser(u, r.accountNbr, &r) == 0) {
            printf("✖ This Account already exists for this user.\n");
            printf("Please try again.\n");
            valid = 0;
        } else {
            valid = 1;
        }
    } while (!valid);

    while (1) {
        printf("\nEnter the country: ");
        scanf("%s", r.country);
        clearInputBuffer();
        if (isAlphaString(r.country)) {
            break;
        } else {
            printf("Invalid country name. Please enter only letters.\n");
        }
    }

    while (1) {
        printf("\nEnter the phone number: ");
        scanf("%s", r.phone);
        clearInputBuffer();
        if (isValidPhone(r.phone)) {
            break;
        } else {
            printf("Invalid phone number. Please enter only digits and an optional leading '+'.\n");
        }
    }

    r.amount = readAmount(0);

    // Reads the account type using the helper function readAccountType
    readAccountType(r.accountType, sizeof(r.accountType));

    // Insert the new record into the database using the insertRecordForUser function
    rc = insertRecordForUser(&u, &r);
    if (rc != SQLITE_DONE) {
        printf("Error saving record.\n");
    } else {
        printf("\nRecord inserted successfully in database.\n");
    }

    success(u);
}


void updateAccountInfo(User u)
{
    int targetAcc;
    Record r;
    int recordFound = 0;

    while (!recordFound) {
        // Loop repeatedly until a valid account number is entered
        while (1) {
            printf("Enter the account number you want to update: ");
            if (scanf("%d", &targetAcc) != 1) {
                printf("Invalid account number input. Please try again.\n");
                clearInputBuffer();
                continue;
            }
            clearInputBuffer();
            break;
        }
        
        // Check if a record exists for the specified account number
        if (getRecordForUser(u, targetAcc, &r) == 0) {
            recordFound = 1;
        } else {
            printf("\nNo record found with account number %d for user %s.\nPlease try again.\n\n", targetAcc, u.name);
        }
    }
    
    // Display current values
    printf("\nRecord found for account number %d.\n", targetAcc);
    printf("Current Country: %s\n", r.country);
    printf("Current Phone: %s\n", r.phone);
    printf("Which field do you want to update?\n");
    printf("1. Phone number\n");
    printf("2. Country\n");
    
    int choice;
    while (1) {
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input for choice. Please try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        if (choice == 1 || choice == 2)
            break;
        else
            printf("Invalid choice. Please enter 1 or 2.\n");
    }
    
    char newValue[100];
    if (choice == 1) {
        while (1) {
            printf("Enter new phone number: ");
            scanf("%s", newValue);
            clearInputBuffer();
            if (isValidPhone(newValue)) {
                break;
            } else {
                printf("Invalid phone number. Please enter only digits and an optional leading '+'.\n");
            }
        }
        strcpy(r.phone, newValue);
    } else if (choice == 2) {
        while (1) {
            printf("Enter new country: ");
            scanf("%s", newValue);
            clearInputBuffer();
            if (isAlphaString(newValue)) {
                break;
            } else {
                printf("Invalid country name. Please enter only letters.\n");
            }
        }
        strcpy(r.country, newValue);
    }

    // Update the record in the database using setRecordForUser
    int rc_update = setRecordForUser(&u, &r);
    if (rc_update != SQLITE_DONE) {
        printf("Error updating record.\n");
    } else {
        printf("\nRecord updated successfully in database.\n");
    }
    
    success(u);
}


void checkAccountDetail(User u)
{
    int targetAcc;
    Record r;  //The record that will be filled by getRecordForUser

    while (1) {
        printf("\nEnter the account number you want to check: ");
        if (scanf("%d", &targetAcc) != 1) {
            printf("Invalid account number input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
    
        if (getRecordForUser(u, targetAcc, &r) != 0) {
            printf("No account found with account number %d for user %s.\n", targetAcc, u.name);
            continue;
        }
    
        break;
    }
    

    printf("\nAccount Details:\n");
    printf("Account Number: %d\n", r.accountNbr);
    printf("Deposit Date: %02d/%02d/%04d\n", r.deposit.day, r.deposit.month, r.deposit.year);
    printf("Country: %s\n", r.country);
    printf("Phone: %s\n", r.phone);
    printf("Amount Deposited: $%.2lf\n", r.amount);
    printf("Account Type: %s\n", r.accountType);
    
    // Calculation of interest and display of message
    double interest = 0.0;
    int rate = 0;
    
    // For "savings" and fixed accounts, interest is calculated.
    if (strcasecmp(r.accountType, "savings") == 0)
        rate = 7;
    else if (strcasecmp(r.accountType, "fixed01") == 0)
        rate = 4;
    else if (strcasecmp(r.accountType, "fixed02") == 0)
        rate = 5;
    else if (strcasecmp(r.accountType, "fixed03") == 0)
        rate = 8;
    else if (strcasecmp(r.accountType, "current") == 0)
        printf("You will not get interests because the account is of type current.\n"); 
        
    if (strcasecmp(r.accountType, "savings") == 0) {
        interest = (r.amount * rate / 100.0)  / 12;
        printf("You will get $%.2lf as interest on day %d of every month.\n", interest, r.deposit.day);
    } else if (rate > 0) {
        int yearsToAdd = 0;
        if (strcasecmp(r.accountType, "fixed01") == 0)
            yearsToAdd = 1;
        else if (strcasecmp(r.accountType, "fixed02") == 0)
            yearsToAdd = 2;
        else if (strcasecmp(r.accountType, "fixed03") == 0)
            yearsToAdd = 3;
    
        interest = (r.amount * rate / 100.0) * yearsToAdd;
        printf("You will get $%.2lf as interest on %02d/%02d/%04d.\n", interest, r.deposit.day, r.deposit.month, r.deposit.year + yearsToAdd);
    }
    
    printf("\nPress Enter to return to the main menu...");
    clearInputBuffer();  
}


void checkAllAccounts(User u)
{
    sqlite3 *db = openDatabase("records.db");
    if (db == NULL) {
        return;
    }
    
    const char *sqlSelect = "SELECT accountNbr, depositDate, country, phone, amount, accountType FROM records WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, u.name, -1, SQLITE_STATIC);
    
    system("clear");
    printf("\t\t====== All accounts for user: %s =====\n\n", u.name);
    
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        int accountNbr = sqlite3_column_int(stmt, 0);
        const unsigned char *depositDate = sqlite3_column_text(stmt, 1);
        const unsigned char *country = sqlite3_column_text(stmt, 2);
        const unsigned char *phone = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char *accountType = sqlite3_column_text(stmt, 5);
        
        printf("________________________________________\n");
        printf("Account Number: %d\n", accountNbr);
        printf("Deposit Date: %s\n", depositDate);
        printf("Country: %s\n", country);
        printf("Phone: %s\n", phone);
        printf("Amount Deposited: $%.2lf\n", amount);
        printf("Account Type: %s\n", accountType);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (!found) {
        printf("No accounts found for user %s.\n", u.name);
    }
    
    printf("\nPress Enter to return to the main menu...");
    clearInputBuffer();  
}


void makeTransaction(User u) {
    int targetAcc, rc, found = 0, recordId = 0;
    double amount = 0.0, newAmount = 0.0;
    char accountType[50] = "";  // Buffer to correctly store the account type
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char input[50];

    // Opening the database
    db = openDatabase("records.db");
    if (db == NULL) {
        printf("Error: Cannot open database.\n");
        return;
    }

    // Continuous attempts by the user to enter a valid account
    while (!found) {
        printf("Enter the account number for the transaction: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0'; // Remove newline character
        if (sscanf(input, "%d", &targetAcc) != 1) {
            printf("Invalid account number input. Please try again.\n\n");
            continue;
        }
    
        // Query to retrieve account info from the database
        const char *sqlSelect = "SELECT id, amount, accountType FROM records WHERE username = ? AND accountNbr = ?;";
        rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            printf("Error: Failed to prepare select statement.\n");
            sqlite3_close(db);
            return;
        }
    
        sqlite3_bind_text(stmt, 1, u.name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, targetAcc);
    
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Extract values from the result
            recordId = sqlite3_column_int(stmt, 0);
            amount = sqlite3_column_double(stmt, 1);
            strncpy(accountType, (const char *)sqlite3_column_text(stmt, 2), sizeof(accountType) - 1);
            accountType[sizeof(accountType) - 1] = '\0'; // Null-terminate the string
    
            // Check if account type starts with "fixed"
            if (strncasecmp(accountType, "fixed", 5) == 0) {
                printf("Transactions are not allowed on fixed accounts.\n\n");
            } else {
                found = 1; // Account is valid and not fixed – exit the loop
            }
        } else {
            printf("No account found with account number %d for user %s. Please try again.\n\n", targetAcc, u.name);
        }
    
        sqlite3_finalize(stmt);
    }

    // Transaction type selection
    int choice;
    while (1) {
        printf("Enter 1 to deposit money or 2 to withdraw money: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
        if (sscanf(input, "%d", &choice) != 1 || (choice != 1 && choice != 2)) {
            printf("Invalid transaction type. Please try again.\n\n");
            continue;
        }
        break;
    }

    // Deposit or withdrawal process
    if (choice == 1) {
        double depositAmt = readAmount(0);
        newAmount = amount + depositAmt;
        printf("Deposit successful. New balance: $%.2lf\n", newAmount);
    } else {
        double withdrawAmt;
        while (1) {
            withdrawAmt = readAmount(0);
            if (withdrawAmt > amount) {
                printf("Insufficient funds. Please enter a smaller amount.\n");
                continue;
            }
            newAmount = amount - withdrawAmt;
            printf("Withdrawal successful. New balance: $%.2lf\n", newAmount);
            break;
        }
    }

    // Updating the database
    const char *sqlUpdate = "UPDATE records SET amount = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Error: Failed to prepare update statement.\n");
        sqlite3_close(db);
        return;
    }
    sqlite3_bind_double(stmt, 1, newAmount);
    sqlite3_bind_int(stmt, 2, recordId);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("Failed to update record.\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("\nTransaction completed successfully.\n");
    success(u);
}


void removeAccount(User u) {
    int targetAccount;
    int rc;
    char input[50];

    while (1) {
        printf("Enter the account number you want to remove (or 1 main menu): ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0'; // Remove newline character

        if (sscanf(input, "%d", &targetAccount) != 1) {
            printf("Invalid account number input. Please enter a number.\n");
            continue;
        }

        if (targetAccount == 1) {
            printf("Operation cancelled.\n");
            mainMenu(u);
        }

        rc = removeRecordForUser(&u, targetAccount);
        if (rc == SQLITE_DONE) {
            printf("\n✅ Account number %d removed successfully.\n", targetAccount);
            break;
        } else {
            printf("\n❌ No account found with account number %d for user %s.\n", targetAccount, u.name);
            // Loop again
        }
    }

    success(u);
}


void transferOwner(User u)
{
    int targetAcc;
    char newOwner[50];
    int rc;

    // Loop for entering the account number until a record is found for this user.
    while (1) {
        printf("Enter the account number you want to transfer (or 0 to return to main menu): ");
        if (scanf("%d", &targetAcc) != 1) {
            printf("Invalid input for account number. Please try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        if (targetAcc == 0) {
            mainMenu(u);
            return;
        }
        
    // Check if a record exists for user u with this account number.
    sqlite3 *dbRecords = openDatabase("records.db");
        if (dbRecords == NULL) {
            return;
        }
        const char *sqlCheck = "SELECT id FROM records WHERE username = ? AND accountNbr = ?;";
        sqlite3_stmt *stmtCheck;
        rc = sqlite3_prepare_v2(dbRecords, sqlCheck, -1, &stmtCheck, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare check statement: %s\n", sqlite3_errmsg(dbRecords));
            sqlite3_close(dbRecords);
            return;
        }
        sqlite3_bind_text(stmtCheck, 1, u.name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmtCheck, 2, targetAcc);
        if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
            break;  // Record found.
        } else {
            printf("No account found with account number %d for user %s.\nPlease try again.\n\n", targetAcc, u.name);
            sqlite3_finalize(stmtCheck);
            sqlite3_close(dbRecords);
        }
    } // End while for account number

    // Repetitive loop for entering the new username.
    int validInput = 0;
    int newOwnerId = -1;
    while (!validInput) {
        printf("Enter the username of the new owner: ");
        if (scanf("%s", newOwner) != 1) {
            printf("Invalid input for username. Please try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        if (strcmp(newOwner, u.name) == 0) {
            printf("New owner cannot be the same as the current owner. Please try again.\n");
            continue;
        }
        
        // Check for the existence of the new user in the "users.db" database and retrieve their ID.
        sqlite3 *dbUsers = openDatabase("users.db");
        if (dbUsers == NULL)
            return;
        const char *sqlSelect = "SELECT id FROM users WHERE username = ?;";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(dbUsers, sqlSelect, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(dbUsers));
            sqlite3_close(dbUsers);
            return;
        }
        sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            newOwnerId = sqlite3_column_int(stmt, 0);
            validInput = 1;
        } else {
            printf("User %s does not exist. Please try again.\n", newOwner);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(dbUsers);
    }
    
    // Update the record in the "records.db" database: change both username and userId.
    sqlite3 *dbRecords = openDatabase("records.db");
    if (dbRecords == NULL)
        return;
    const char *sqlUpdate = "UPDATE records SET username = ?, userId = ? WHERE username = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt2;
    rc = sqlite3_prepare_v2(dbRecords, sqlUpdate, -1, &stmt2, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(dbRecords));
        sqlite3_close(dbRecords);
        return;
    }
    sqlite3_bind_text(stmt2, 1, newOwner, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 2, newOwnerId);
    sqlite3_bind_text(stmt2, 3, u.name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 4, targetAcc);

    rc = sqlite3_step(stmt2);
if (rc != SQLITE_DONE) {
    fprintf(stderr, "❌ Failed to transfer record: %s\n", sqlite3_errmsg(dbRecords));
} else {
    if (sqlite3_changes(dbRecords) > 0) {
        *sharedTransferNotification = 1;    // Setting the flag for the notification thread
        strncpy(sharedSenderUsername, u.name, sizeof(sharedSenderUsername) - 1);
        sharedSenderUsername[sizeof(sharedSenderUsername) - 1] = '\0';

        printf("\n✅ Account transferred successfully to %s.\n", newOwner);
        fflush(stdout);
    } else {
        printf("\n⚠️ No account found with account number %d for user %s.\n", targetAcc, u.name);
    }
}
sqlite3_finalize(stmt2);
sqlite3_close(dbRecords);

success(u);

}

