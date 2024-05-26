#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_RAM_SIZE 2048 // 2GB
#define MAX_HARD_DRIVE_SIZE 256 // 256GB
#define MAX_NUM_CORES 8

int ramSize = MAX_RAM_SIZE; // Initialize with maximum available RAM
int hardDriveSize = MAX_HARD_DRIVE_SIZE; // Initialize with maximum available hard drive space
int numCores = MAX_NUM_CORES; 

typedef struct {
    int ram;
    int hdd;
    int cores;
    pid_t pid;
    char name[256];
} Task;

Task tasks[256];
int task_count = 0;

void display_message(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void execute_task(const char *task, int required_ram, int required_hdd, int required_cores) {
    if (required_ram > ramSize || required_hdd > hardDriveSize || required_cores > numCores) {
        display_message("Error: Not enough resources to run the task.");
        return;
    }

    ramSize -= required_ram;
    hardDriveSize -= required_hdd;
    numCores -= required_cores;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        execlp("gnome-terminal", "gnome-terminal", "--", task, NULL);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        Task new_task = {required_ram, required_hdd, required_cores, pid, ""};
        strncpy(new_task.name, task, sizeof(new_task.name) - 1);
        tasks[task_count++] = new_task;
    }
}

void view_processes(GtkWidget *widget, gpointer data) {
    char task_info[1024];
    snprintf(task_info, sizeof(task_info), "Viewing currently running processes:\n");
    for (int i = 0; i < task_count; ++i) {
        snprintf(task_info + strlen(task_info), sizeof(task_info) - strlen(task_info),
                 "Task: %s, PID: %d, RAM: %dGB, HDD: %dGB, Cores: %d\n",
                 tasks[i].name, tasks[i].pid, tasks[i].ram, tasks[i].hdd, tasks[i].cores);
    }
    display_message(task_info);
}

void cancel_process(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *label, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Cancel Process", NULL, flags, "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    label = gtk_label_new("Enter PID of the process to cancel:");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int pid = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
        for (int i = 0; i < task_count; ++i) {
            if (tasks[i].pid == pid) {
                kill(pid, SIGKILL);
                ramSize += tasks[i].ram;
                hardDriveSize += tasks[i].hdd;
                numCores += tasks[i].cores;
                tasks[i] = tasks[--task_count]; // Remove the task from the array
                display_message("Canceled process.");
                gtk_widget_destroy(dialog);
                return;
            }
        }
        display_message("No process found with the given PID.");
    }

    gtk_widget_destroy(dialog);
}

void user_mode(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *label, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("User Mode", NULL, flags, "_OK", GTK_RESPONSE_OK, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    label = gtk_label_new("Enter task to execute (e.g., ./calculator):");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *task = gtk_entry_get_text(GTK_ENTRY(entry));
        execute_task(task, 1, 1, 1);
    }

    gtk_widget_destroy(dialog);
}

void kernel_mode(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *button1, *button2, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Kernel Mode", NULL, flags, "_Close", GTK_RESPONSE_CLOSE, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    button1 = gtk_button_new_with_label("View Processes");
    g_signal_connect(button1, "clicked", G_CALLBACK(view_processes), NULL);
    gtk_container_add(GTK_CONTAINER(content_area), button1);

    button2 = gtk_button_new_with_label("Cancel Process");
    g_signal_connect(button2, "clicked", G_CALLBACK(cancel_process), NULL);
    gtk_container_add(GTK_CONTAINER(content_area), button2);

    gtk_widget_show_all(dialog);

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}

void create_main_window() {
    GtkWidget *window, *grid, *user_mode_button, *kernel_mode_button, *exit_button;
    GtkWidget *user_mode_image, *kernel_mode_image, *exit_image;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Fiza's Operating System");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    user_mode_button = gtk_button_new();
    user_mode_image = gtk_image_new_from_file("user_icon.png");
    gtk_button_set_image(GTK_BUTTON(user_mode_button), user_mode_image);
    gtk_grid_attach(GTK_GRID(grid), user_mode_button, 0, 0, 1, 1);
    g_signal_connect(user_mode_button, "clicked", G_CALLBACK(user_mode), NULL);

    kernel_mode_button = gtk_button_new();
    kernel_mode_image = gtk_image_new_from_file("kernel_icon.png");
    gtk_button_set_image(GTK_BUTTON(kernel_mode_button), kernel_mode_image);
    gtk_grid
kernel_mode_button = gtk_button_new();
    kernel_mode_image = gtk_image_new_from_file("kernel_icon.png");
    gtk_button_set_image(GTK_BUTTON(kernel_mode_button), kernel_mode_image);
    gtk_grid_attach(GTK_GRID(grid), kernel_mode_button, 1, 0, 1, 1);
    g_signal_connect(kernel_mode_button, "clicked", G_CALLBACK(kernel_mode), NULL);

    exit_button = gtk_button_new();
    exit_image = gtk_image_new_from_file("exit_icon.png");
    gtk_button_set_image(GTK_BUTTON(exit_button), exit_image);
    gtk_grid_attach(GTK_GRID(grid), exit_button, 0, 1, 2, 1);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    create_main_window();

    gtk_main();

    return 0;
}
