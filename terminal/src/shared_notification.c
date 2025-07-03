#include "header.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHM_NAME "/transfer_notification_shm"

int *sharedTransferNotification = NULL;
char *sharedSenderUsername = NULL;

void initSharedNotification(void) {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        safeExit(1);  // graceful shutdown
    }

    // Size: 4 bytes for int + 50 bytes for sender username
    size_t size = sizeof(int) + 50;

    if (ftruncate(fd, size) == -1) {
        perror("ftruncate");
        close(fd);
        safeExit(1);  // graceful shutdown
    }

    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        safeExit(1);  // graceful shutdown
    }
    close(fd);

    sharedTransferNotification = (int *)addr;
    sharedSenderUsername = (char *)addr + sizeof(int);

    // Optional: Init only if first time (could use lock or magic number)
    if (*sharedTransferNotification != 0 && strlen(sharedSenderUsername) == 0) {
        *sharedTransferNotification = 0;
        sharedSenderUsername[0] = '\0';
    }
}

void cleanupSharedNotification(void) {
    if (sharedTransferNotification != NULL) {
        munmap(sharedTransferNotification, sizeof(int) + 50);
        shm_unlink(SHM_NAME);
    }
}
