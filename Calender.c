#include <gtk/gtk.h>

void on_calendar_day_selected(GtkCalendar *calendar, gpointer user_data) {
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);
    g_print("Selected Date: %02d-%02d-%d\n", day, month + 1, year);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calendar");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *calendar = gtk_calendar_new();
    g_signal_connect(calendar, "day-selected", G_CALLBACK(on_calendar_day_selected), NULL);

    gtk_container_add(GTK_CONTAINER(window), calendar);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
