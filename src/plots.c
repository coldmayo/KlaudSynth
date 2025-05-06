#include <gtk/gtk.h>
#include <glib.h>

GThread *gtk_thread = NULL;

typedef struct {
    int argc;
    char **argv;
} GtkThreadData;

gpointer gtk_thread_func(gpointer user_data) {
    GtkThreadData *data = (GtkThreadData *)user_data;
    GtkWidget *window;

    gtk_init(&data->argc, &data->argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Waveform Veiwer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    gtk_main();

    g_free(data);
    return NULL;
}

void startUp(int argc, char *argv[]) {
    if (!gtk_thread) {
        GtkThreadData *thread_data = g_new(GtkThreadData, 1);
        thread_data->argc = argc;
        thread_data->argv = argv;

        gtk_thread = g_thread_new("gtk_thread", gtk_thread_func, thread_data);

        if (!gtk_thread) {
            g_error("Failed to create GTK thread");
            g_free(thread_data);
        }
    }
}
