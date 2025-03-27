
#include "header.h"
#include <ctype.h>
#include <termios.h>
#include <openssl/sha.h>

void readDate(int *day, int *month, int *year) {
    char input[20] = {0};
    int valid = 0;
    int row = 5;  // Row where the prompt will be displayed
    const char *prompt = "Enter today's date (dd/mm/yyyy): ";

    while (!valid) {
        // Print the prompt at the given position
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();
        
        // Position cursor immediately after the prompt (on the same line)
        move(row, 5 + strlen(prompt));
        clrtoeol();
        refresh();

        // Use echo and mvgetnstr to let the user input on the same line
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, sizeof(input) - 1);
        noecho();

        // Clear any error message on the next line if the user typed something
        if (strlen(input) > 0) {
            move(row + 2, 5);
            clrtoeol();
            refresh();
        }

        // Validate the input format and values
        if (sscanf(input, "%d/%d/%d", day, month, year) == 3) {
            if (isValidDate(*day, *month, *year)) {
                valid = 1;  // Date is valid, exit the loop
            } else {
                attron(COLOR_PAIR(5));  // Turn on error color (red, for example)
                move(row + 2, 5);
                clrtoeol();
                printw("Invalid date values. Please ensure the month is 1-12, the day is valid, and the year is 4 digits.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1000);
            }
        } else {
            attron(COLOR_PAIR(5));
            move(row + 2, 5);
            clrtoeol();
            printw("Invalid format! Use dd/mm/yyyy.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1000);
        }
    }
    
    // Clear the prompt and error messages after successful input
    move(row, 5);
    clrtoeol();
    move(row + 2, 5);
    clrtoeol();
    refresh();
}

int readAccountNumber(User u) {
    char input[20] = {0};
    int valid = 0;
    int row = 5;  // Row where the prompt will be displayed
    const char *prompt = "Enter the Account Number: ";
    int accountNbr = 0;
    Record r;  // Dummy record to check for existing account

    while (!valid) {
        // Print the prompt at the specified row and column
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();

        // Position the cursor immediately after the prompt (on the same line)
        move(row, 5 + strlen(prompt));
        clrtoeol();
        refresh();

        // Enable echo so the user can see their input and get it on the same line
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, sizeof(input) - 1);
        noecho();

        // Clear any previous error message on the next line once input is received
        if (strlen(input) > 0) {
            move(row + 2, 5);
            clrtoeol();
            refresh();
        }

        // Check that the input consists only of digits
        int isNumber = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) {
                isNumber = 0;
                break;
            }
        }

        if (!isNumber) {
            attron(COLOR_PAIR(5));  // Turn on error color (red)
            move(row + 2, 5);
            clrtoeol();
            printw("Invalid input! Enter only digits.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1000);  // Pause for 1 second to let the user read the error
            continue;  // Re-loop to ask for input again
        }

        // Attempt to convert the input string to an integer
        if (sscanf(input, "%d", &accountNbr) == 1) {
            // Check if the account number already exists for this user.
            // Assume getRecordForUser returns 0 if the record exists.
            if (getRecordForUser(u, accountNbr, &r) == 0) {
                attron(COLOR_PAIR(5));
                move(row + 2, 5);
                clrtoeol();
                printw("Account number %d already exists. Please enter a different number.", accountNbr);
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
                continue;  // Re-loop because the account number is already in use
            }
            valid = 1;  // Input is valid and unique
        } else {
            attron(COLOR_PAIR(5));
            move(row + 2, 5);
            clrtoeol();
            printw("Invalid number. Please try again.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1000);
        }
    }

    // Clear prompt and error message lines after successful input
    move(row, 5);
    clrtoeol();
    move(row + 2, 5);
    clrtoeol();
    refresh();

    return accountNbr;
}


void readCountry(char *country, int maxLen) {
    char input[50];
    int valid = 0;
    int row = 5;  // Row for displaying the prompt
    const char *prompt = "Enter the Country: ";

    while (!valid) {
        // Clear the prompt line and display the prompt
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();

        // Move the cursor to the same line after the prompt for input
        move(row, 5 + strlen(prompt));
        clrtoeol();
        refresh();

        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, maxLen - 1);
        noecho();

        // Check that the input contains only letters
        int isValid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isalpha(input[i])) {
                isValid = 0;
                break;
            }
        }

        if (isValid && strlen(input) > 0) {
            strcpy(country, input);
            valid = 1;  // Valid input received, exit loop
        } else {
            move(row + 2, 5);
            clrtoeol();
            attron(COLOR_PAIR(5));  // Turn on error color (red)
            printw("Invalid country name! Use only letters.");
            attroff(COLOR_PAIR(5)); // Turn off error color
            refresh();
            napms(1000);  // Pause briefly to allow the user to read the error message
        }
    }

    // Clear the prompt and error message lines after successful input
    move(row, 5);
    clrtoeol();
    move(row + 2, 5);
    clrtoeol();
    refresh();
}
void readPhone(char *phone, int maxLen) {
    char input[20] = {0};
    int valid = 0;
    int row = 5;  // Row where the prompt will be displayed
    const char *prompt = "Enter Phone Number: ";

    while (!valid) {
        // Display the prompt at the specified row and column
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();

        // Position the cursor immediately after the prompt on the same line
        move(row, 5 + strlen(prompt));
        clrtoeol();
        refresh();

        // Enable input echoing and read the user's input on the same line
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, maxLen - 1);
        noecho();

        // Use the external isValidPhone function to check if the input is valid.
        if (strlen(input) > 0 && isValidPhone(input)) {
            strcpy(phone, input);
            valid = 1;
        } else {
            // Display an error message in red if the input is invalid
            attron(COLOR_PAIR(5));  // Assumes COLOR_PAIR(5) is set to red on black
            move(row + 2, 5);
            clrtoeol();
            printw("Invalid phone! Use digits and an optional leading '+' only.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1000);  // Pause for 1 second before re-prompting
        }
    }

    // Clear the prompt and error lines after successful input
    for (int i = row; i < row + 3; i++) {
        move(i, 5);
        clrtoeol();
    }
    refresh();
}

double readAmount(int allowNegative) {
    char input[20] = {0};
    double amount;
    int valid = 0;
    int row = 5;  // Row where the prompt is displayed
    const char *prompt = "Enter amount to deposit: $";

    while (!valid) {
        // Display the prompt at the specified row and column
        move(row, 5);
        clrtoeol();
        printw("%s", prompt);
        refresh();

        // Move the cursor to the next line for user input
        move(row, 5 + strlen(prompt));
        clrtoeol();
        refresh();

        // Enable echo to show user input and read a string from the user
        echo();
        mvgetnstr(row, 5 + strlen(prompt), input, sizeof(input) - 1);
        noecho();

        // Replace any commas with dots (to support locales that use commas as decimal separators)
        for (int i = 0; input[i] != '\0'; i++) {
            if (input[i] == ',') {
                input[i] = '.';
            }
        }

        // Validate numeric format: allow an optional '+' or '-', digits, and at most one dot.
        int dotCount = 0;
        int isValidFormat = 1;
        int i = 0;
        if (input[0] == '+' || input[0] == '-') {
            i = 1;
        }
        for (; input[i] != '\0'; i++) {
            if (!isdigit(input[i]) && input[i] != '.') {
                isValidFormat = 0;
                break;
            }
            if (input[i] == '.') {
                dotCount++;
                if (dotCount > 1) {
                    isValidFormat = 0;
                    break;
                }
            }
        }

        if (isValidFormat && sscanf(input, "%lf", &amount) == 1) {
            // Check for negative value if not allowed
            if (!allowNegative && amount < 0) {
                move(row + 2, 5);
                clrtoeol();
                attron(COLOR_PAIR(5)); // Error color, e.g., red
                printw("Amount must be positive.");
                attroff(COLOR_PAIR(5));
                refresh();
                napms(1500);
                continue;
            }
            // Check that there are at most 2 decimal places if a dot is present
            if (dotCount == 1) {
                char *dotPos = strchr(input, '.');
                if (dotPos && strlen(dotPos + 1) > 2) {
                    move(row + 2, 5);
                    clrtoeol();
                    attron(COLOR_PAIR(5));
                    printw("Max 2 decimal places allowed.");
                    attroff(COLOR_PAIR(5));
                    refresh();
                    napms(1500);
                    continue;
                }
            }
            valid = 1;
        } else {
            move(row + 2, 5);
            clrtoeol();
            attron(COLOR_PAIR(5));
            printw("Invalid amount! Enter a valid number.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1500);
        }
    }

    // Clear the prompt and error message lines after successful input
    for (int i = row; i <= row + 2; i++) {
        move(i, 5);
        clrtoeol();
    }
    refresh();

    return amount;
}


// Reads the account type from user input, ensuring the input does not exceed maxLen characters.
void readAccountType(char *accountType, int maxLen) {
    int row = 5;  // Starting row for instructions
    int col = 5;  // Column for instructions

    // Display account type options
    move(row, col);
    clrtoeol();
    printw("Choose the type of account:");
    
    move(row + 1, col);
    clrtoeol();
    printw("  -> savings");
    
    move(row + 2, col);
    clrtoeol();
    printw("  -> current");
    
    move(row + 3, col);
    clrtoeol();
    printw("  -> fixed01 (for 1 year)");
    
    move(row + 4, col);
    clrtoeol();
    printw("  -> fixed02 (for 2 years)");
    
    move(row + 5, col);
    clrtoeol();
    printw("  -> fixed03 (for 3 years)");
    
    // Print the prompt on the same line; user input will appear right after it.
    const char *prompt = "Enter your choice: ";
    move(row + 7, col);
    clrtoeol();
    printw("%s", prompt);
    refresh();
    
    int valid = 0;
    while (!valid) {
        // Move the cursor right after the prompt on the same line.
        move(row + 7, col + strlen(prompt));
        clrtoeol();
        refresh();
        
        echo();
        mvgetnstr(row + 7, col + strlen(prompt), accountType, maxLen - 1);
        noecho();
        
        // If the user starts typing, clear the error message on the next line.
        move(row + 8, col);
        clrtoeol();
        refresh();
        
        // Validate input against allowed account types.
        if (strcmp(accountType, "savings") == 0 ||
            strcmp(accountType, "current") == 0 ||
            strcmp(accountType, "fixed01") == 0 ||
            strcmp(accountType, "fixed02") == 0 ||
            strcmp(accountType, "fixed03") == 0) {
            valid = 1;
            clear();
        } else {
            attron(COLOR_PAIR(5));  // Turn on red color
            move(row + 9, col);
            clrtoeol();
            printw("Invalid choice! Use only: savings, current, fixed01, fixed02, fixed03.");
            attroff(COLOR_PAIR(5));
            refresh();
            napms(1000);  // Pause for 1 second
            // Error message is now cleared on the next iteration by clrtoeol() above.
        }
    }
    
    // Clear all lines used for instructions and input.
    for (int i = row; i <= row + 8; i++) {
        move(i, col);
        clrtoeol();
    }
    refresh();
}


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

// Function success: Displays a small menu using ncurses for the user to choose between returning to the main menu or exiting.
// It accepts two additional parameters: the starting row and column where the message and options will be displayed.
void success(User u, int startRow, int startCol) {
    char *options[] = { "Main Menu", "Exit" };
    int n_options = 2;
    int highlight = 0;
    int choice = 0;
    int c;

    // Display "Success!" message at the specified position in color pair 3
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(startRow, startCol, "Success!");
    attroff(COLOR_PAIR(3) | A_BOLD);
    refresh();

    // Menu loop: display options and let the user navigate with arrow keys
    while (1) {
        // Display each option starting two rows below the success message
        for (int i = 0; i < n_options; i++) {
            if (i == highlight)
                attron(A_REVERSE);  // Highlight the currently selected option
            mvprintw(startRow + 2 + i, startCol, options[i]);
            attroff(A_REVERSE);
        }
        c = getch();  // Get user key press

        switch(c) {
            case KEY_UP:
                highlight--;
                if (highlight < 0)
                    highlight = n_options - 1;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight >= n_options)
                    highlight = 0;
                break;
            case 10:  // Enter key (ASCII 10)
                choice = highlight;
                break;
            default:
                break;
        }
        if (c == 10)  // Break the loop when Enter is pressed
            break;
    }

    // Act based on the user's choice
    if (choice == 0) {  // Main Menu
        clear();
        mainMenu(u);
    } else if (choice == 1) {  // Exit
        endwin();
        exitProgram();
    }
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

// Rounds the given value to two decimal places.
double roundToTwoDecimals(double value) {
    return ((int)(value * 100 + 0.5)) / 100.0;
}

int isValidPhone(const char *s) {
    // Ensure s is not NULL or empty.
    if (s == NULL || *s == '\0') {
        return 0;
    }
    
    int len = strlen(s);
    int i = 0;
    
    // If the phone number starts with '+', allow it only if there are digits after.
    if (s[0] == '+') {
        if (len == 1) {
            return 0; // Only a '+' is not valid.
        }
        i = 1;
    }
    
    // Check that every remaining character is a digit.
    for (; i < len; i++) {
        if (!isdigit(s[i])) {
            return 0;
        }
    }
    
    return 1;
}

// Helper function: Displays a menu at (row, col) with two options and returns the selected option (1 for deposit, 2 for withdraw)
int transactionTypeMenu(int row, int col) {
    char *options[] = { "Deposit money", "Withdraw money" };
    int n_options = 2;
    int highlight = 0;  // Index of the currently highlighted option
    int c;

    // Enable keypad for arrow key support
    keypad(stdscr, TRUE);
    
    // Main menu loop
    while (1) {
        // Clear the menu area (for the options)
        for (int i = 0; i < n_options; i++) {
            move(row + i, col);
            clrtoeol();
        }
        // Print the options; use COLOR_PAIR(2) for green on black
        for (int i = 0; i < n_options; i++) {
            if (i == highlight) {
                attron(COLOR_PAIR(2));  // Highlight with green
            }
            mvprintw(row + i, col, options[i]);
            if (i == highlight) {
                attroff(COLOR_PAIR(2));
            }
        }
        refresh();

        c = getch();  // Get user key press
        switch (c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10:  // Enter key
                clear();  // Clear the screen upon selection
                return (highlight == 0) ? 1 : 2;
            default:
                break;
        }
    }
}


void exitProgram() {
    int row = 5;  // Starting row for the exit message
    clear();  // Clear the screen
    attron(COLOR_PAIR(2));  // Turn on color pair 2 (make sure it's defined, e.g., green on black)
    mvprintw(row + 1, 5, "Bye Bye!!");  // Display the exit message at row+1, column 5
    attroff(COLOR_PAIR(2));  // Turn off color pair 2
    refresh();  // Refresh the screen to show changes
    napms(1500);  // Wait for 1500 milliseconds (1.5 seconds)
    endwin();  // End ncurses mode (restore terminal settings)
    exit(0);  // Terminate the program
}
