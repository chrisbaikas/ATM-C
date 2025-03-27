#include "header.h"


#define SHM_NAME "/transfer_notification_shm"
#define SENDER_SHM_NAME "/sender_username_shm"

int *sharedTransferNotification = NULL;
char *sharedSenderUsername = NULL;

void initSharedNotification(void) {
    // Integer notification flag
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1 || ftruncate(fd, sizeof(int)) == -1) {
        perror("shm_open / ftruncate");
        exit(1);
    }
    sharedTransferNotification = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sharedTransferNotification == MAP_FAILED) {
        perror("mmap (int)");
        exit(1);
    }
    close(fd);
    *sharedTransferNotification = 0;

    // Shared sender username (50 bytes should be enough)
    int fd2 = shm_open(SENDER_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd2 == -1 || ftruncate(fd2, 50) == -1) {
        perror("shm_open sender");
        exit(1);
    }
    sharedSenderUsername = mmap(NULL, 50, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    if (sharedSenderUsername == MAP_FAILED) {
        perror("mmap (sender)");
        exit(1);
    }
    close(fd2);
    sharedSenderUsername[0] = '\0';
}

void cleanupSharedNotification(void) {
    if (sharedTransferNotification != NULL) {
        munmap(sharedTransferNotification, sizeof(int));
        shm_unlink(SHM_NAME);
    }

    if (sharedSenderUsername != NULL) {
        munmap(sharedSenderUsername, 50);
        shm_unlink(SENDER_SHM_NAME);
    }
}