#include "header.h"

void *notificationThread(void *arg) {
    User *u = (User *) arg;

    while (1) {
        if (sharedTransferNotification && *sharedTransferNotification) {
            // Αν η μεταφορά προήλθε από άλλον χρήστη, εμφάνισε το όνομά του
            if (strcmp(u->name, sharedSenderUsername) != 0) {
                printf("\n🔔 User '%s' transferred an account to you!\n", sharedSenderUsername);
                fflush(stdout);
            }
            // Μηδενίζουμε τη σημαία
            *sharedTransferNotification = 0;
        }
        usleep(500000); // 0.5s delay
    }
    return NULL;
}
