
#include "header.h"
#include <ctype.h>
#include <termios.h>
#include <openssl/sha.h>


// Helper function for reading a password while displaying asterisks (the actual password is hidden)
void readPassword(char *password, int maxLen) {
    struct termios oldt, newt;
    int i = 0;
    char ch;

    // Save current settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < maxLen - 1) {
        ch = getchar();
        if (ch == '\n' || ch == '\r')
            break;
        if (ch == 127 || ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';

    // Restore settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

// HashPassword function: calculates the SHA256 hash of the given password.
void hashPassword(const char *password, unsigned char *hashedOutput)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf((char *)hashedOutput + (i * 2), "%02x", hash[i]);
    }
    hashedOutput[SHA256_DIGEST_LENGTH * 2] = '\0';
}

// Helper function for clearing the input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Helper function for reading an integer from input
int readIntInput(void) {
    int num;
    while (scanf("%d", &num) != 1) {
        printf("Invalid input. Please enter a valid number.\n");
        clearInputBuffer();
    }
    clearInputBuffer();
    return num;
}

// Displays a success message and returning to the main menu.
void success(User u) {
    printf("\n✔ Success for user %s!\n", u.name);
    printf("Press Enter to return to the main menu...");
    clearInputBuffer();
}

// Returns 1 if the given year is a leap year, otherwise returns 0.
int isLeapYear(int year) {
    return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

// Checks if the date (day, month, year) is valid, taking into account month lengths and leap years.
int isValidDate(int day, int month, int year) {

    if (year < 1000 || year > 9999) {
        return 0;
    }

    if (month < 1 || month > 12) {
        return 0;
    }
    
    int maxDay;
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            maxDay = 31;
            break;
        case 4: case 6: case 9: case 11:
            maxDay = 30;
            break;
        case 2:
            maxDay = isLeapYear(year) ? 29 : 28;
            break;
        default:
            return 0;  
    }

    if (day < 1 || day > maxDay) {
        return 0;
    }
    return 1;
}

// Reads a monetary amount from input using the prompt; if allowNegative is false, ensures the value is non-negative.
double readAmount(int allowNegative) {
    char input[100];
    double amount;

    const char *prompt = "\nEnter amount to deposit: $";
    while (1) {
        printf("%s", prompt);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input. Please try again.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';
        
        for (char *p = input; *p; p++) {
            if (*p == ',') {
                *p = '.';
            }
        }
        
        if (sscanf(input, "%lf", &amount) != 1) {
            printf("Invalid amount. Please try again.\n");
            continue;
        }
        
        if (!allowNegative && amount < 0) {
            printf("Amount must be positive. Please try again.\n");
            continue;
        }
        
        char *dot = strchr(input, '.');
        if (dot != NULL) {
            int decimals = strlen(dot + 1);
            if (decimals > 2) {
                printf("Please enter an amount with no more than 2 decimal places.\n");
                continue;
            }
        }
        
        amount = roundToTwoDecimals(amount);
        return amount;
    }
}

// Reads the account type from user input, ensuring the input does not exceed maxLen characters.
void readAccountType(char *accountType, int maxLen) {
    int valid = 0;
    char temp[50];
    while (!valid) {
        printf("\nChoose the type of account:\n");
        printf("\t-> savings\n");
        printf("\t-> current\n");
        printf("\t-> fixed01 (for 1 year)\n");
        printf("\t-> fixed02 (for 2 years)\n");
        printf("\t-> fixed03 (for 3 years)\n");
        printf("\n\tEnter your choice: ");
        scanf("%s", temp);
        while(getchar() != '\n'); // Clear buffer

        // Check if the value is valid (case-insensitive)
        if (strcasecmp(temp, "savings") == 0 ||
            strcasecmp(temp, "current") == 0 ||
            strcasecmp(temp, "fixed01") == 0 ||
            strcasecmp(temp, "fixed02") == 0 ||
            strcasecmp(temp, "fixed03") == 0) {
            valid = 1;
            strncpy(accountType, temp, maxLen - 1);
            accountType[maxLen - 1] = '\0';  // Ensure null termination
        } else {
            printf("Invalid account type. Please try again.\n");
        }
    }
}

// Returns 1 if the string s consists entirely of alphabetic characters; otherwise, returns 0.
int isAlphaString(const char *s) {
    while (*s) {
        if (!isalpha(*s) && !isspace(*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

// Returns 1 if the string s is a valid phone number (contains only digits and at most one leading '+'), otherwise returns 0.
int isValidPhone(const char *s) {
    if (*s == '\0') return 0;
    
    if (*s == '+') {
        s++;
    }
    
    while (*s) {
        if (!isdigit((unsigned char)*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

// Returns 1 if the numeric string s contains no more than two digits after the decimal point, otherwise returns 0.
int hasAtMostTwoDecimals(const char *s) {
    const char *dot = strchr(s, '.');
    if (!dot)
        return 1;  // No decimal point present, so the value is valid
    return (strlen(dot + 1) <= 2);
}

// Rounds the given value to two decimal places.
double roundToTwoDecimals(double value) {
    return ((int)(value * 100 + 0.5)) / 100.0;
}

void safeExit(int code) {
    // Clean shared memory if initialized
    cleanupSharedNotification();

    // You can also log the exit if you want
    if (code != 0) {
        fprintf(stderr, "Exiting with code %d\n", code);
    }

    exit(code);
}
