#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

// ========== Standard Libraries ========== //
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sqlite3.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

// ========== Structures ========== //

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

// ========== Global Variables ========== //
extern char senderUsername[50];
extern int *sharedTransferNotification;

// ========== Main Flow ========== //
void initMenu(User *u);
void mainMenu(User u);

// ========== Auth ========== //
void loginMenu(User *u);
void registerMenu(User *u);
const char *getPassword(User u);

// ========== Core Functionalities ========== //
void createNewAcc(User u);
void updateAccountInfo(User u);
void checkAccountDetail(User u);
void checkAllAccounts(User u);
void makeTransaction(User u);
void removeAccount(User u);
void transferOwner(User u);

// ========== Helpers ========== //
void readPassword(char *password, int maxLen);
void hashPassword(const char *password, unsigned char *hashedOutput);
void clearInputBuffer(void);
int readIntInput(void);
int isLeapYear(int year);
int isValidDate(int day, int month, int year);
int isAlphaString(const char *s);
int isValidPhone(const char *s);
int hasAtMostTwoDecimals(const char *s);
double roundToTwoDecimals(double value);
double readAmount(int allowNegative);
void readAccountType(char *accountType, int maxLen);
void success();
void successWithPos(User u, int row, int col); // Optional if using positional

// ========== Database ========== //
int initDatabase(const char *dbFile);
sqlite3 *openDatabase(const char *dbFile);
int insertRecordForUser(const User *u, const Record *r);
int saveRecordSQL(const User *u, const Record *r);
int setRecordForUser(const User *u, const Record *r);
int getRecordForUser(const User u, int accountNbr, Record *r);
int removeRecordForUser(const User *u, int accountNbr);

// ========== Notifications ========== //

extern int *sharedTransferNotification;
extern char sharedSenderUsername[50];

void initSharedNotification(void);
void cleanupSharedNotification(void);

void *notificationThread(void *arg);

#ifdef __cplusplus
}
#endif

#endif // HEADER_H
