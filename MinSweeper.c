#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 10
#define MINES 10

GtkWidget *buttons[SIZE][SIZE];
int mines[SIZE][SIZE];
int revealed[SIZE][SIZE];

void reset_game();
void button_clicked(GtkWidget *widget, gpointer data);
void reveal(int row, int col);
void game_over();
void reveal_all_mines();
int count_adjacent_mines(int row, int col);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL));

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minesweeper");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            buttons[i][j] = gtk_button_new_with_label("");
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j], j, i, 1, 1);
            g_signal_connect(buttons[i][j], "clicked", G_CALLBACK(button_clicked), GINT_TO_POINTER(i * SIZE + j));
        }
    }

    reset_game();
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

void reset_game() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            mines[i][j] = 0;
            revealed[i][j] = 0;
            gtk_button_set_label(GTK_BUTTON(buttons[i][j]), "");
            gtk_widget_set_sensitive(buttons[i][j], TRUE);
        }
    }

    for (int k = 0; k < MINES; k++) {
        int i, j;
        do {
            i = rand() % SIZE;
            j = rand() % SIZE;
        } while (mines[i][j] == 1);
        mines[i][j] = 1;
    }
}

void button_clicked(GtkWidget *widget, gpointer data) {
    int index = GPOINTER_TO_INT(data);
    int row = index / SIZE;
    int col = index % SIZE;

    if (mines[row][col] == 1) {
        gtk_button_set_label(GTK_BUTTON(widget), "M");
        game_over();
    } else {
        reveal(row, col);
    }
}

void reveal(int row, int col) {
    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE || revealed[row][col]) {
        return;
    }

    revealed[row][col] = 1;
    gtk_widget_set_sensitive(buttons[row][col], FALSE);

    int adjacent_mines = count_adjacent_mines(row, col);
    if (adjacent_mines > 0) {
        char label[2];
        sprintf(label, "%d", adjacent_mines);
        gtk_button_set_label(GTK_BUTTON(buttons[row][col]), label);
    } else {
        gtk_button_set_label(GTK_BUTTON(buttons[row][col]), " ");
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                reveal(row + dr, col + dc);
            }
        }
    }
}

void game_over() {
    reveal_all_mines();
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Game Over");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    reset_game();
}

void reveal_all_mines() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (mines[i][j] == 1) {
                gtk_button_set_label(GTK_BUTTON(buttons[i][j]), "M");
            }
        }
    }
}

int count_adjacent_mines(int row, int col) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            int new_row = row + dr;
            int new_col = col + dc;
            if (new_row >= 0 && new_row < SIZE && new_col >= 0 && new_col < SIZE && mines[new_row][new_col]) {
                count++;
            }
        }
    }
    return count;
}
