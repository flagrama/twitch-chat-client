#include <iostream>

#include <gtk/gtk.h>

#include "main.h"
#include "Twitch.h"

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    Twitch twitch;
    GtkApplication *application;
    int status;

    application = gtk_application_new("com.flagrama.gtk-twitch-app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);

    return status;
}
