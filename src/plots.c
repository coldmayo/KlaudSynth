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
    GtkWidget *image;
	GtkWidget * freq_image;
    gtk_init(&data->argc, &data->argv);

	// Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Waveform Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	// Create tab container
	GtkWidget * notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(window), notebook);

	// Tab 1: Waveform Veiwer:
	GtkWidget *tab1_label = gtk_label_new("Waveform Veiwer");
    image = gtk_image_new_from_file("plot.png");
    if (image == NULL) {
        g_warning("Failed to load image plot.png");
        image = gtk_label_new("Image not found");
    }
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), image, tab1_label);

    // Tab 2: Frequency Veiwer
    GtkWidget *tab2_label = gtk_label_new("Frequency Veiwer");
    freq_image = gtk_image_new_from_file("freq_plot.png");
    if (image == NULL) {
        g_warning("Failed to load image freq_plot.png");
        image = gtk_label_new("Image not found");
    }
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), freq_image, tab2_label);
    
    gtk_widget_show_all(window);

    gtk_main();

    g_free(data);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (!gtk_thread) {
        GtkThreadData *thread_data = g_new(GtkThreadData, 1);
        thread_data->argc = argc;
        thread_data->argv = argv;

        // Initialize GTK in main thread first
        if (!g_thread_supported()) g_thread_init(NULL);
        gdk_threads_init();
        gdk_threads_enter();

        gtk_thread = g_thread_new("gtk_thread", gtk_thread_func, thread_data);

        if (!gtk_thread) {
            g_error("Failed to create GTK thread");
            g_free(thread_data);
            return 1;
        }

        while (g_thread_self() != NULL) {
            g_usleep(100000);
        }

        gdk_threads_leave();
    }
    return 0;
}
