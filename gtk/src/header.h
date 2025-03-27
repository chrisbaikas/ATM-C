#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------- Standard Libraries ---------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sqlite3.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>

/* ---------------------- GTK+ Library ---------------------- */
#include <gtk/gtk.h>

/* ---------------------- Structures ---------------------- */
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

typedef struct {
    User *u;
    int option;
} MenuCallbackData;


/* ---------------------- GTK UI Functions ---------------------- */
// Using the new UI with GTK+ instead of the old mainMenu/initMenu
void create_init_menu(User *u);
void create_main_menu(User *u);

/* ---------------------- Callbacks for the GTK+ UI ---------------------- */
void on_login_clicked(GtkWidget *widget, gpointer data);
void on_register_clicked(GtkWidget *widget, gpointer data);
void on_exit_clicked();
void on_menu_option_clicked(GtkWidget *widget, gpointer data);

/* ---------------------- Authentication Functions ---------------------- */
void loginMenu(User *u);
void registerMenu(User *u);
const char *getPassword(User *u);

/* ---------------------- System Functions ---------------------- */
void createNewAcc(User *u);
void updateAccountInfo(User *u);
void checkAccountDetail(User *u);
void checkAllAccounts(User *u);
void makeTransaction(User *u);
void removeAccount(User *u);
void transferOwner(User *u);

/* ---------------------- Helper Functions ---------------------- */
void hashPassword(const char *password, unsigned char *hashedOutput);
int readIntInput(void);
int isLeapYear(int year);
int isValidDate(int day, int month, int year);
double readAmount(int allowNegative);
void readAccountType(char *accountType, int maxLen);
int isAlphaString(const char *s);
int isValidPhone(const char *s);
int hasAtMostTwoDecimals(const char *s);
double roundToTwoDecimals(double value);
void free_callback_data(gpointer data, GClosure *closure);

/* ---------------------- Global Variables ---------------------- */
extern char senderUsername[50];
extern int transferNotification;  // 0 = no transfer notification, 1 = account transferred

/* ---------------------- Database Module Functions ---------------------- */
int initDatabase(const char *dbFile);
sqlite3 *openDatabase(const char *dbFile);
int setRecordForUser(const User *u, const Record *r);
int getRecordForUser(const User *u, int accountNbr, Record *r);
int insertRecordForUser(const User *u, const Record *r);
int removeRecordForUser(const User *u, int accountNbr);

/* ---------------------- Notification Module ---------------------- */
void *notificationThread(void *arg);

/* ---------------------- Shared Notification Module ---------------------- */
gboolean show_notification_dialog(gpointer data);

extern int *sharedTransferNotification;
extern char *sharedSenderUsername;

void initSharedNotification(void);
void cleanupSharedNotification(void);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_H */
