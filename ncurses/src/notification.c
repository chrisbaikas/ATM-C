#include "header.h"

void *notificationThread(void *arg) {
    User *u = (User *) arg;

    while (1) {
        if (sharedTransferNotification && *sharedTransferNotification) {
            if (strcmp(u->name, sharedSenderUsername) != 0) {
                int row, col;
                
                getmaxyx(stdscr, row, col); 
                (void)row;
                (void)col;

                int y = 17;
                int x = 5;

                attron(COLOR_PAIR(1) | A_BOLD); 
                mvprintw(y, x, "Notification: Someone transferred an account to you!");
                attroff(COLOR_PAIR(1) | A_BOLD);
                refresh();
            }

            *sharedTransferNotification = 0;
        }

        usleep(500000); // 0.5s delay
    }

    return NULL;
}

