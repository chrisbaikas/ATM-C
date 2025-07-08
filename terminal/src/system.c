#include "header.h"
#include <math.h>

void createNewAcc(User u)
{
  Record r;

    if (ensureRecordsTableExists() != SQLITE_OK) {
        safeExit(1);
    }

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
    int rc = insertRecordForUser(&u, &r);
    if (rc != 0) {
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
    if (rc_update != 0) {
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
    sqlite3_stmt *stmt;
    if (getAllRecordsForUser(u.name, &stmt) != 0) {
        return;
    }

    system("clear");
    printf("\t\t====== All accounts for user: %s =====\n\n", u.name);

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        printf("________________________________________\n");
        printf("Account Number: %d\n", sqlite3_column_int(stmt, 0));
        printf("Deposit Date: %s\n", sqlite3_column_text(stmt, 1));
        printf("Country: %s\n", sqlite3_column_text(stmt, 2));
        printf("Phone: %s\n", sqlite3_column_text(stmt, 3));
        printf("Amount Deposited: $%.2lf\n", sqlite3_column_double(stmt, 4));
        printf("Account Type: %s\n", sqlite3_column_text(stmt, 5));
    }

    sqlite3 *db = sqlite3_db_handle(stmt);
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
    char accountType[50] = "";
    char input[50];

    // Keep asking until a valid account is found
    while (!found) {
        printf("Enter the account number for the transaction: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
        if (sscanf(input, "%d", &targetAcc) != 1) {
            printf("Invalid account number input. Please try again.\n\n");
            continue;
        }

        // Retrieve transaction info from DB
        rc = getTransactionInfo(u.name, targetAcc, &recordId, &amount, accountType);
        if (rc == 0) {
            if (strncasecmp(accountType, "fixed", 5) == 0) {
                printf("Transactions are not allowed on fixed accounts.\n\n");
            } else {
                found = 1; // Valid account found
            }
        } else {
            printf("No account found with account number %d for user %s. Please try again.\n\n", targetAcc, u.name);
        }
    }

    // Ask for transaction type
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

    // Execute deposit or withdrawal
    if (choice == 1) {
        // Deposit branch
        double depositAmt, tentative;
        while (1) {
            depositAmt = readAmount(0);
            // Check for overflow or non-finite
            tentative = amount + depositAmt;
            if (!isfinite(depositAmt) || !isfinite(tentative) || tentative < amount) {
                printf("Invalid deposit amount. Please enter a smaller amount.\n");
                continue;
            }
            newAmount = tentative;
            printf("Deposit successful. New balance: $%.2lf\n", newAmount);
            break;
        }
    } else {
        // Withdrawal process
        double withdrawAmt;
        while (1) {
            withdrawAmt = readAmount(0);

            // Reject negative, non-finite or μεγαλύτερο από το υπόλοιπο
            if (!(withdrawAmt > 0.0) || !isfinite(withdrawAmt) || withdrawAmt > amount) {
                printf("Invalid withdrawal amount. Please enter a positive number up to $%.2f.\n", amount);
                continue;
            }
            newAmount = amount - withdrawAmt;
            printf("Withdrawal successful. New balance: $%.2lf\n", newAmount);
            break;
        }
    }

    // Update balance in DB
    if (updateAccountAmount(recordId, newAmount) != 0) {
        printf("Failed to update account.\n");
    } else {
        printf("\nTransaction completed successfully.\n");
    }

    success(u);
}


void removeAccount(User u) {
    int targetAccount;
    int rc;
    char input[50];

    while (1) {
        printf("Enter the account number you want to remove (or 1 to return to main menu): ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';  // strip newline

        if (sscanf(input, "%d", &targetAccount) != 1) {
            printf("Invalid account number. Please enter a valid number.\n");
            continue;
        }

        // Option to go back
        if (targetAccount == 1) {
            printf("Operation cancelled. Returning to main menu.\n");
            return;  // just return; caller will re-invoke mainMenu
        }

        // Attempt to delete
        rc = removeRecordForUser(&u, targetAccount);
        if (rc == 0) {
            printf("\n✅ Account number %d removed successfully.\n", targetAccount);
            break;
        } else {
            printf("\n❌ No account found with account number %d for user %s.\n", 
                   targetAccount, u.name);
            // and loop to try again
        }
    }

    // Final success message and pause before going back
    success(u);
}

void transferOwner(User u) {
    int targetAcc;
    char newOwner[50];
    int newOwnerId;
    int rc;

    // Prompt for a valid account number (or 0 to cancel)
    while (1) {
        printf("Enter the account number you want to transfer (or 0 to return to main menu): ");
        if (scanf("%d", &targetAcc) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (targetAcc == 0) {
            printf("Operation cancelled. Returning to main menu.\n");
            return;
        }

        if (accountExistsForUser(u.name, targetAcc)) {
            break;
        }
        printf("No account found with account number %d for user %s. Please try again.\n\n",
               targetAcc, u.name);
    }

    // Prompt for a valid new owner username
    while (1) {
        printf("Enter the username of the new owner: ");
        if (scanf("%49s", newOwner) != 1) {
            printf("Invalid input. Please try again.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (strcmp(newOwner, u.name) == 0) {
            printf("New owner cannot be the same as current owner. Please enter a different username.\n");
            continue;
        }

        newOwnerId = getUserId(newOwner);
        if (newOwnerId >= 0) {
            break;
        }
        printf("User \"%s\" does not exist. Please try again.\n", newOwner);
    }

    // Attempt the transfer
    rc = transferAccountToUser(u.name, newOwner, newOwnerId, targetAcc);
    if (rc == 0) {
        // Notify the other thread
        *sharedTransferNotification = 1;
        strncpy(sharedSenderUsername, u.name, strlen(u.name)+1);

        printf("\n✅ Account number %d transferred successfully to %s.\n",
               targetAcc, newOwner);
    } else {
        printf("\n❌ Failed to transfer account number %d.\n", targetAcc);
    }

    // Pause before returning
    success(u);
}
