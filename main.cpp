#include <gtk/gtk.h>
#include <thread>

#include "main.h"
#include "Twitch.h"

void get_responses(Twitch* twitch, GtkTextView* textView);

int main(int argc, char *argv[]) {
    Twitch twitch;
    GtkWidget *window;
    GtkWidget *irc_messages;
    GtkBuilder *builder;

    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file("twitch-chat.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    irc_messages = GTK_WIDGET(gtk_builder_get_object(builder, "irc_messages"));

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show(window);
    std::thread responses(get_responses, &twitch, GTK_TEXT_VIEW(irc_messages));
    responses.detach();
    gtk_main();

    return EXIT_SUCCESS;
}

void get_responses(Twitch* twitch, GtkTextView* textView) {
    GtkTextIter iter;
    GtkTextBuffer *textBuffer = gtk_text_view_get_buffer(textView);
    char response_buffer[4096];
    std::thread responses (&Twitch::read_responses, twitch, response_buffer);

    while(true) {
        if(strcmp(response_buffer, "")) {
            gtk_text_buffer_get_end_iter(textBuffer, &iter);
            gtk_text_buffer_insert(textBuffer, &iter, response_buffer, -1);
            gtk_text_view_scroll_to_mark(textView, gtk_text_buffer_get_insert(textBuffer), 0, 0, 0, 0);

            memset(response_buffer, 0, sizeof response_buffer);
        }
    }
}
