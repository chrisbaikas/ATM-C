#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
    #endif

    // --- Standard Libraries --- //
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>
    #include <strings.h>
    #include <sqlite3.h>
    #include <pthread.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <ncurses.h>

    // --- Structures --- //

    typedef struct {
        int month, day, year;
    } Date;

    typedef struct {
        int id;
        int userId;
        char name[100];
        char country[100];
        char phone[20];
        char accountType[10];
        int accountNbr;
        double amount;
        Date deposit;
        Date withdraw;
    } Record;

    typedef struct {
        int id;
        char name[50];
        char password[50];
    } User;

    // --- Main Functions --- //

    void mainMenu(User u);
    void initMenu(User *u);

    // --- Authentication Functions --- //

    void loginMenu(User *u);
    void registerMenu(User *u);
    const char *getPassword(User u);

    // --- System Functions --- //

    void createNewAcc(User u);
    void updateAccountInfo(User u);
    void checkAccountDetail(User u);
    void checkAllAccounts(User u);
    void makeTransaction(User u);
    void removeAccount(User u);
    void transferOwner(User u);

    // --- Helper Functions --- //

    void readDate(int *day, int *month, int *year);
    int readAccountNumber(User u);
    void readPhone(char *phone, int maxLen);
    void readCountry(char *country, int maxLen);
    double readAmount(int allowNegative);
    void readAccountType(char *accountType, int maxLen);
    void readPassword(char *password, int maxLen);
    void hashPassword(const char *password, unsigned char *hashedOutput);
    void success(User u, int startRow, int startCol);
    int isLeapYear(int year);
    int isValidDate(int day, int month, int year);
    int isAlphaString(const char *s);
    int isValidPhone(const char *s);
    double roundToTwoDecimals(double value);
    int transactionTypeMenu(int row, int col);
    void exitProgram();

    // --- Global Variables --- //

    extern char senderUsername[50];

    #ifdef __cplusplus
}
#endif

#endif // HEADER_H

// --- Global Variables Module --- //
#ifndef GLOBALS_H
#define GLOBALS_H

extern int transferNotification; // 0 = no transfer notification, 1 = account transferred

#endif // GLOBALS_H

// --- Database Module --- //
#ifndef DATABASE_H
#define DATABASE_H

int initDatabase(const char *dbFile);
sqlite3 *openDatabase(const char *dbFile);
int setRecordForUser(const User *u, const Record *r);
int getRecordForUser(const User u, int accountNbr, Record *r);
int insertRecordForUser(const User *u, const Record *r);
int saveRecordSQL(const User *u, const Record *r);
int removeRecordForUser(const User *u, int accountNbr);

#endif /* DATABASE_H */

// --- Notification Module --- //
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <pthread.h>

void *notificationThread(void *arg);

#endif // NOTIFICATION_H

// --- Shared Notification Module --- //
#ifndef SHARED_NOTIFICATION_H
#define SHARED_NOTIFICATION_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// Making the shared pointer available for notifications from all files
extern int *sharedTransferNotification;
extern char *sharedSenderUsername;

void initSharedNotification(void);
void cleanupSharedNotification(void);

#endif /* SHARED_NOTIFICATION_H */
