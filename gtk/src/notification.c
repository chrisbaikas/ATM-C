#include "header.h"
#include <glib.h>      // for g_idle_add, G_SOURCE_REMOVE

// The show_notification function will be called in the main thread via g_idle_add
gboolean show_notification_dialog(gpointer data) {
    (void) data;
    
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "🔔 Someone transferred an account to you!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return FALSE;  // Executed only once
}


void *notificationThread(void *arg)
{
    // The user that is logged in (defined in header.h)
    User *u = (User *) arg;

    while (1) {
        if (sharedTransferNotification && *sharedTransferNotification) {
            // Displays the notification only if the current user is not the sender
            if (strcmp(u->name, sharedSenderUsername) != 0) {
                g_idle_add((GSourceFunc)show_notification_dialog, NULL);
            }
            *sharedTransferNotification = 0;
        }
        usleep(500000);  // Delay 0.5 seconds
    }
    
    return NULL;
}

