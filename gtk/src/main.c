#include <gtk/gtk.h>
#include "header.h"
#include <pthread.h>

// Setting the senderUsername
char senderUsername[50] = "";

// --- Callbacks for initMenu ---
void on_login_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;

    User *u = (User *)data;  //  cast gpointer
    loginMenu(u);          
}


void on_register_clicked(GtkWidget *widget, gpointer data){
    (void)widget;

    User *u = (User *)data; 

    registerMenu(u);
}


void on_exit_clicked() {

    GtkWidget *bye_dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "Bye bye!");
    gtk_dialog_run(GTK_DIALOG(bye_dialog));
    gtk_widget_destroy(bye_dialog);

    gtk_main_quit();
}

void create_init_menu(User *u) {
    GtkWidget *window, *vbox, *button;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ATM - Login/Register");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    button = gtk_button_new_with_label("Login");
    g_signal_connect(button, "clicked", G_CALLBACK(on_login_clicked), u);  // pass User *
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Register");
    g_signal_connect(button, "clicked", G_CALLBACK(on_register_clicked), u);  // pass User *
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Exit");
    g_signal_connect(button, "clicked", G_CALLBACK(on_exit_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
}


// --- Callbacks for mainMenu ---
void on_menu_option_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;

    MenuCallbackData *callbackData = (MenuCallbackData *)data;
    User *u = callbackData->u;
    int option = callbackData->option;

    switch (option) {
        case 1: createNewAcc(u); break;
        case 2: updateAccountInfo(u); break;
        case 3: checkAccountDetail(u); break;
        case 4: checkAllAccounts(u); break;
        case 5: makeTransaction(u); break;
        case 6: removeAccount(u); break;
        case 7: transferOwner(u); break;
        case 8: gtk_main_quit(); break;
        default: break;
    }
}


void create_main_menu(User *u) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ATM - Main Menu");
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);

    GtkWidget *menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), menu_box);

    // === Welcome label ===
    char welcome_msg[100];
    snprintf(welcome_msg, sizeof(welcome_msg), "Welcome, %s", u->name);

    GtkWidget *welcome_label = gtk_label_new(welcome_msg);
    gtk_box_pack_start(GTK_BOX(menu_box), welcome_label, FALSE, FALSE, 5);

    // === Menu buttons ===
    const char *menu_labels[] = {
        "Create a new account",
        "Update account information",
        "Check accounts",
        "Check list of owned accounts",
        "Make Transaction",
        "Remove existing account",
        "Transfer ownership",
        "Exit"
    };

    for (int i = 1; i <= 8; i++) {
        GtkWidget *btn = gtk_button_new_with_label(menu_labels[i - 1]);
    
        MenuCallbackData *data = malloc(sizeof(MenuCallbackData));
        
        // Deep copy 
        data->u = malloc(sizeof(User));
        memcpy(data->u, u, sizeof(User));  
    
        data->option = i;
    
        g_signal_connect_data(btn, "clicked", G_CALLBACK(on_menu_option_clicked), data, free_callback_data, 0);
        gtk_container_add(GTK_CONTAINER(menu_box), btn);
    }
    

    gtk_widget_show_all(window);
}




// --- main ---
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    User *u = malloc(sizeof(User));
    if (u == NULL) {
        g_printerr("User pointer is NULL\n");
        return 1;
    }

    int rc = initDatabase("users.db");
    if (rc != SQLITE_OK) {
        g_printerr("Database initialization failed.\n");
        return rc;
    }

    initSharedNotification();

    create_init_menu(u);  

    // Notification thread
    pthread_t notif_tid;
    if (pthread_create(&notif_tid, NULL, notificationThread, u) != 0) {
        perror("pthread_create");
        exit(1);
    }

    gtk_main();

    cleanupSharedNotification();

    return 0;
}
