#include "header.h"

void *notificationThread(void *arg) {
    User *u = (User *) arg;

    while (1) {
        if (sharedTransferNotification && *sharedTransferNotification) {
            if (strcmp(u->name, sharedSenderUsername) != 0) {
                printf("\n🔔 Someone transferred an account to you!\n");
                fflush(stdout);
            }

            *sharedTransferNotification = 0;
        }

        usleep(500000); // 0.5s delay
    }
    return NULL;
}
