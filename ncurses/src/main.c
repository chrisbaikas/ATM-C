#include "header.h"

void mainMenu(User u) {
    int option = 0;
    int highlight = 0;
    int ch;

    initscr();              // Start NCURSES mode
    noecho();               // Do not echo input characters
    curs_set(FALSE);        // Hide cursor
    keypad(stdscr, TRUE);   // Enable arrow keys
    start_color();          // Enable colors

    // Check if there's a notification
    if (sharedTransferNotification && *sharedTransferNotification) {
        attron(COLOR_PAIR(1)); 
        mvprintw(1, 2, "*** Someone transferred their account to you !! ***");
        attroff(COLOR_PAIR(1));
        *sharedTransferNotification = 0;
        mvprintw(3, 2, "Press any key to continue...");
        refresh();
        getch();
    }

    while (1) {
        clear();  // Clear screen

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(2, 10, "======= ATM =======");
        attroff(COLOR_PAIR(2) | A_BOLD);

        mvprintw(4, 10, "Welcome, %s", u.name);
        mvprintw(6, 10, "-->> Choose an option <<--");

        char *menu_items[] = {
            "Create a new account",
            "Update account information",
            "Check accounts",
            "Check list of owned accounts",
            "Make Transaction",
            "Remove existing account",
            "Transfer ownership",
            "Exit"
        };

        // Print menu items
        for (int i = 0; i < 8; i++) {
            if (i == highlight) {
                attron(COLOR_PAIR(3) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(8 + i, 10, "[%d] %s", i + 1, menu_items[i]);
            attroff(A_BOLD | A_REVERSE);
        }

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                highlight = (highlight == 0) ? 7 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == 7) ? 0 : highlight + 1;
                break;
            case 10:  // Enter key
                option = highlight + 1;
                break;
            default:
                break;
        }

        if (option > 0) {
            break;
        }
    }

    endwin();  // Exit NCURSES mode

    switch (option) {
        case 1:
            createNewAcc(u);
            break;
        case 2:
            updateAccountInfo(u);
            break;
        case 3:
            checkAccountDetail(u);
            break;
        case 4:
            checkAllAccounts(u);
            break;
        case 5:
            makeTransaction(u);
            break;
        case 6:
            removeAccount(u);
            break;
        case 7:
            transferOwner(u);
            break;
        case 8:
            exitProgram();
            break;
        default:
            mainMenu(u);
    }
}

void initMenu(User *u) {
    int option = 0;
    int highlight = 0;
    int ch;

    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    start_color();

    // Colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);  // Notifications
    init_pair(2, COLOR_CYAN, COLOR_BLACK);    // Title
    init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Selected Option / success
    init_pair(4, COLOR_WHITE, COLOR_BLACK);   // Regular Text
    init_pair(5, COLOR_RED, COLOR_BLACK);     // Error messages

    while (1) {
        clear();
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(2, 10, "======= ATM =======");
        attroff(COLOR_PAIR(2) | A_BOLD);
        mvprintw(4, 5, "-->> Login / Register <<--");

        char *menu_items[] = {"Login", "Register", "Exit"};

        for (int i = 0; i < 3; i++) {
            if (i == highlight) {
                attron(COLOR_PAIR(3) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(6 + i, 10, "[%d] %s", i + 1, menu_items[i]);
            attroff(A_BOLD | A_REVERSE);
        }

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                highlight = (highlight == 0) ? 2 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == 2) ? 0 : highlight + 1;
                break;
            case 10:
                option = highlight + 1;
                break;
            default:
                break;
        }

        if (option > 0) {
            break;
        }
    }

    endwin();

    switch (option) {
        case 1:
            while (1) {
                loginMenu(u);
                // Inside a loop where you check the password:
                if (strcmp(u->password, getPassword(*u)) == 0) {
                    break;  // Exit the loop if the password is correct
                } 
            }
            break;
        case 2:
            registerMenu(u);
            break;
        case 3:
            exitProgram();
            break;
    }
}

int main() {
    int rc = initDatabase("users.db");
    if (rc != SQLITE_OK) {
        attron(COLOR_PAIR(5));
        fprintf(stderr, "Database initialization failed.\n");
        attroff(COLOR_PAIR(5));
        return rc;
    }

    User u;

    initSharedNotification();

    initMenu(&u);

    // Start the notifications thread
    pthread_t notif_tid;
    if (pthread_create(&notif_tid, NULL, notificationThread, &u) != 0) {
        perror("pthread_create");
        exit(1);
    }

    mainMenu(u);

    cleanupSharedNotification();
    return 0;
}
