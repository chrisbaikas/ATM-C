#include "header.h"

void mainMenu(User u)
{
    int option;

    system("clear");
    printf("\n\n\t\t======= ATM =======\n");
    printf("\n\t\tWelcome, %s \n", u.name);
    printf("\n\t\t-->> Feel free to choose one of the options below <<--\n");
    printf("\n\t\t[1]- Create a new account\n");
    printf("\n\t\t[2]- Update account information\n");
    printf("\n\t\t[3]- Check accounts\n");
    printf("\n\t\t[4]- Check list of owned accounts\n");
    printf("\n\t\t[5]- Make Transaction\n");
    printf("\n\t\t[6]- Remove existing account\n");
    printf("\n\t\t[7]- Transfer ownership\n");
    printf("\n\t\t[8]- Exit\n");
    scanf("%d", &option);
    clearInputBuffer();

    switch (option)
    {
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
            printf("Exiting... Bye!\n");
            safeExit(0);  // Graceful exit
            break;
        default:
            mainMenu(u);
    }
}

void initMenu(User *u)
{
    int r = 0;
    int option;
    system("clear");
    printf("\n\n\t\t======= ATM =======\n");
    printf("\n\t\t-->> Feel free to login / register :\n");
    printf("\n\t\t[1]- Login\n");
    printf("\n\t\t[2]- Register\n");
    printf("\n\t\t[3]- Exit\n");

    while (!r)
    {
        scanf("%d", &option);
        clearInputBuffer();
        switch (option)
        {
            case 1:
                while (1) {
                    loginMenu(u);
                    if (strcmp(u->password, getPassword(*u)) == 0) {
                        printf("\n\nPassword Match!\n");
                        break;
                    } else {
                        printf("\nWrong password or username. Please try again.\n");
                    }
                }
                r = 1;
                break;
            case 2:
                registerMenu(u);
                r = 1;
                break;
            case 3:
                safeExit(0);  // Exit with cleanup
                break;
            default:
                printf("Insert a valid operation!\n");
                break;
        }
    }
}

int main()
{
    int rc = initDatabase("users.db");
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed.\n");
        return rc;
    }

    User u;

    initSharedNotification();

    initMenu(&u); 

    // Start the notifications thread
    pthread_t notif_tid;
    if (pthread_create(&notif_tid, NULL, notificationThread, &u) != 0) {
        perror("pthread_create");
        safeExit(1);  // Clean exit on thread error
    }
    pthread_detach(notif_tid); 

    while (1) {
        mainMenu(u); 
    }

    // Unreachable code – fallback cleanup if needed
    cleanupSharedNotification();
    return 0;
}
