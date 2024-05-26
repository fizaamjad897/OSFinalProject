#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

GtkWidget *file_list;

void refresh_file_list() {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(file_list)));
    gtk_list_store_clear(store);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, ent->d_name, -1);
            }
        }
        closedir(dir);
    }
}

void create_file(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Create File", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Create", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        const char *filename = gtk_entry_get_text(GTK_ENTRY(entry));
        FILE *file = fopen(filename, "w");
        if (file) {
            fclose(file);
            refresh_file_list();
        }
    }

    gtk_widget_destroy(dialog);
}

void delete_file(GtkWidget *widget, gpointer window) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(file_list));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);
        if (remove(filename) == 0) {
            refresh_file_list();
        }
        g_free(filename);
    }
}

void view_file(GtkWidget *widget, gpointer window) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(file_list));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);
        char *command = g_strdup_printf("gedit %s", filename);
        system(command);
        g_free(filename);
        g_free(command);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
    file_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Files", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_list), column);
    gtk_box_pack_start(GTK_BOX(vbox), file_list, TRUE, TRUE, 0);

    GtkWidget *create_button = gtk_button_new_with_label("Create File");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete File");
    GtkWidget *view_button = gtk_button_new_with_label("View File");

    g_signal_connect(create_button, "clicked", G_CALLBACK(create_file), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_file), window);
    g_signal_connect(view_button, "clicked", G_CALLBACK(view_file), window);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), create_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), delete_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), view_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    refresh_file_list();

    gtk_main();

    return 0;
}
