/*======================================================================*/
/*  header.h - Central include for the whole ATM project                */
/*----------------------------------------------------------------------*/
/*======================================================================*/
#ifndef HEADER_H               /* single include-guard → ONE header     */
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* === Standard headers ================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>

#include <sqlite3.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <openssl/sha.h>   /* for SHA256 in hashPassword */

/* === Data models ====================================================== */
typedef struct {
    int day, month, year;
} Date;

typedef struct {
    int   id;
    int   userId;
    char  name[100];
    char  country[100];
    char  phone[20];
    char  accountType[10];
    int   accountNbr;
    double amount;
    Date  deposit;
    Date  withdraw;
} Record;

typedef struct {
    int  id;
    char name[50];
    char password[50];     /* ← stores hex-encoded SHA-256 hash */
} User;

/* === Main flow ======================================================== */
void initMenu(User *u);
void mainMenu(User  u);

/* === Authentication =================================================== */
void  loginMenu   (User *u);
void  registerMenu(User *u);
const char *getPassword(User u);

/* === Core functionality ============================================== */
void createNewAcc      (User u);
void updateAccountInfo (User u);
void checkAccountDetail(User u);
void checkAllAccounts  (User u);
void makeTransaction   (User u);
void removeAccount     (User u);
void transferOwner     (User u);

/* === Helper utilities ================================================= */
void   readPassword   (char *password, int maxLen);
void   hashPassword   (const char *password, unsigned char *out);
void   clearInputBuffer(void);
int    readIntInput   (void);
int    isLeapYear     (int year);
int    isValidDate    (int d,int m,int y);
int    isAlphaString  (const char *s);
int    isValidPhone   (const char *s);
int    hasAtMostTwoDecimals(const char *s);
double roundToTwoDecimals(double v);
double readAmount(int allowNegative);
void   readAccountType(char *accountType, int maxLen);
void success(User u);
void   successWithPos(User u,int row,int col); /* optional */
void   safeExit(int code);                     /* graceful shutdown */

/* === Database layer - common ========================================= */
sqlite3 *openDatabase(const char *dbFile);

/* === Database layer - users ========================================== */
int initDatabase(const char *dbFile);   /* creates users table */
int getUserId    (const char *username);

/* === Database layer - records ======================================== */
/* one-off */
int ensureRecordsTableExists(void);

/* CRUD */
int insertRecordForUser (const User *u, const Record *r);
int getRecordForUser    (User u, int accountNbr, Record *r);
int setRecordForUser    (const User *u, const Record *r);
int removeRecordForUser (const User *u, int accountNbr);

/* extra utils */
int getAllRecordsForUser(const char *username, sqlite3_stmt **stmtOut);
int getTransactionInfo  (const char *username, int accountNbr,
                         int *recordId, double *amount, char *accountType);
int updateAccountAmount (int recordId, double newAmount);
int accountExistsForUser(const char *username, int accountNbr);
int transferAccountToUser(const char *oldUser,
                          const char *newUser,
                          int newUserId,
                          int accountNbr);

/* === Inter-process notifications ===================================== */
extern int  *sharedTransferNotification;  /* shm flag - 0|1 */
extern char *sharedSenderUsername;        /* shm buffer     */

void initSharedNotification(void);
void cleanupSharedNotification(void);

void *notificationThread(void *arg);      /* pthread entry   */

/* ===================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* HEADER_H */
