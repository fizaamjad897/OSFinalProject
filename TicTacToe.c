#include <gtk/gtk.h>

#define SIZE 3

GtkWidget *buttons[SIZE][SIZE];
char board[SIZE][SIZE];
char current_player = 'X';

void check_winner();
void reset_game();
void button_clicked(GtkWidget *widget, gpointer data);
void game_over(const char *message);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tic Tac Toe");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            buttons[i][j] = gtk_button_new_with_label(" ");
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j], j, i, 1, 1);
            g_signal_connect(buttons[i][j], "clicked", G_CALLBACK(button_clicked), GINT_TO_POINTER(i * SIZE + j));
            board[i][j] = ' ';
        }
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

void button_clicked(GtkWidget *widget, gpointer data) {
    int index = GPOINTER_TO_INT(data);
    int row = index / SIZE;
    int col = index % SIZE;

    if (board[row][col] == ' ') {
        board[row][col] = current_player;
        gtk_button_set_label(GTK_BUTTON(widget), current_player == 'X' ? "X" : "O");
        check_winner();
        current_player = current_player == 'X' ? 'O' : 'X';
    }
}

void check_winner() {
    for (int i = 0; i < SIZE; i++) {
        if (board[i][0] == current_player && board[i][1] == current_player && board[i][2] == current_player) {
            game_over(current_player == 'X' ? "X wins!" : "O wins!");
            return;
        }
        if (board[0][i] == current_player && board[1][i] == current_player && board[2][i] == current_player) {
            game_over(current_player == 'X' ? "X wins!" : "O wins!");
            return;
        }
    }

    if (board[0][0] == current_player && board[1][1] == current_player && board[2][2] == current_player) {
        game_over(current_player == 'X' ? "X wins!" : "O wins!");
        return;
    }

    if (board[0][2] == current_player && board[1][1] == current_player && board[2][0] == current_player) {
        game_over(current_player == 'X' ? "X wins!" : "O wins!");
        return;
    }

    int draw = 1;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == ' ') {
                draw = 0;
                break;
            }
        }
    }

    if (draw) {
        game_over("Draw!");
    }
}

void game_over(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    reset_game();
}

void reset_game() {
    current_player = 'X';
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = ' ';
            gtk_button_set_label(GTK_BUTTON(buttons[i][j]), " ");
        }
    }
}
