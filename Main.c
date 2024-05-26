#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

GtkWidget *overlay;
GtkWidget *window;
GtkWidget *grid;

#define USER_MODE 0
#define KERNEL_MODE 1
#define MAX_RAM_SIZE 2048 // 2GB
#define MAX_HARD_DRIVE_SIZE 256 // 256GB
#define MAX_NUM_CORES 8
#define MAX_TASKS 100

int ramSize = MAX_RAM_SIZE; // Initialize with maximum available RAM
int hardDriveSize = MAX_HARD_DRIVE_SIZE; // Initialize with maximum available hard drive space
int numCores = MAX_NUM_CORES;
int current_mode = USER_MODE;

typedef struct {
    int ram;
    int hdd;
    int cores;
    pid_t pid;
    char name[256];
} Task;
Task tasks[MAX_TASKS];
int task_count = 0;

void show_loading_screen();
gboolean show_system_specs(GtkWidget *loading_window);
gboolean show_main_window(GtkWidget *specs_window);
void create_main_window();
void display_message(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
void show_loading_screen() {
    GtkWidget *loading_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(loading_window), "Loading");
    gtk_window_set_default_size(GTK_WINDOW(loading_window), 400, 300);

    GtkWidget *label = gtk_label_new("Loading, please wait...");
    gtk_container_add(GTK_CONTAINER(loading_window), label);

    gtk_widget_show_all(loading_window);

    g_timeout_add_seconds(3, (GSourceFunc)show_system_specs, loading_window);
}
gboolean show_system_specs(GtkWidget *loading_window) {
    gtk_widget_destroy(loading_window);

    GtkWidget *specs_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(specs_window), "System Specs");
    gtk_window_set_default_size(GTK_WINDOW(specs_window), 400, 300);

    char specs[512];
    snprintf(specs, sizeof(specs), "RAM Size: %d MB\nHard Drive Size: %d GB\nNumber of Cores: %d",
             MAX_RAM_SIZE, MAX_HARD_DRIVE_SIZE, MAX_NUM_CORES);

    GtkWidget *label = gtk_label_new(specs);
    gtk_container_add(GTK_CONTAINER(specs_window), label);

    gtk_widget_show_all(specs_window);

    g_timeout_add_seconds(5, (GSourceFunc)show_main_window, specs_window);

    return FALSE; // to stop the timeout function from repeating
}
void update_system_status(GtkWidget *widget, gpointer data) {
    char status[512];
    snprintf(status, sizeof(status), "Current Resource Status:\nRAM: %d MB\nHard Drive: %d GB\nCores: %d",
             ramSize, hardDriveSize, numCores);
    display_message(status);
}
gboolean show_main_window(GtkWidget *specs_window) {
    gtk_widget_destroy(specs_window);

    create_main_window();
    gtk_widget_show_all(window);

    return FALSE; // to stop the timeout function from repeating
}void execute_task(const char *task, int required_ram, int required_hdd, int required_cores) {
    if (required_ram > ramSize || required_hdd > hardDriveSize || required_cores > numCores) {
        display_message("Error: Not enough resources to run the task.");
        return;
    }

    ramSize -= required_ram;
    hardDriveSize -= required_hdd;
    numCores -= required_cores;

    pid_t pid;
    if (strstr(task, ".obj") != NULL) {
        // GUI-based application
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execlp("gnome-terminal", "gnome-terminal", "--", task, NULL);
            exit(EXIT_SUCCESS);
        }
    } else {
        // Terminal-based command
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execlp("gnome-terminal", "gnome-terminal", "-x", task, NULL);
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process
    Task new_task = {required_ram, required_hdd, required_cores, pid, ""};
    strncpy(new_task.name, task, sizeof(new_task.name) - 1);
    tasks[task_count++] = new_task;
}


void view_processes(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *scroll_window, *text_view;
    GtkTextBuffer *buffer;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Currently Running Processes", NULL, flags, "_Close", GTK_RESPONSE_CLOSE, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll_window, 600, 400);
    gtk_container_add(GTK_CONTAINER(content_area), scroll_window);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll_window), text_view);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    FILE *fp;
    char output[8192] = "";
    char line[256];
    
    // Run the ps command to get the list of running processes
    fp = popen("ps aux", "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }

    // Read the output of the ps command
    while (fgets(line, sizeof(line), fp) != NULL) {
        strncat(output, line, sizeof(output) - strlen(output) - 1);
    }
    pclose(fp);

    // Set the text buffer with the process list
    gtk_text_buffer_set_text(buffer, output, -1);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void cancel_process(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *entry, *label;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Cancel Process", NULL, flags, "_Cancel", GTK_RESPONSE_CANCEL, "_OK", GTK_RESPONSE_OK, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    label = gtk_label_new("Enter the PID of the process to cancel:");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *pid_text = gtk_entry_get_text(GTK_ENTRY(entry));
        pid_t pid = (pid_t)atoi(pid_text);

        if (pid > 0) {
            printf("Attempting to terminate process with PID: %d\n", pid);
            
            // Check if running as root
            if (getuid() != 0) {
                char command[256];
                snprintf(command, sizeof(command), "pkexec kill -15 %d", pid);
                int ret = system(command);
                if (ret != 0) {
                    perror("Failed to terminate the process with SIGTERM using pkexec");
                    snprintf(command, sizeof(command), "pkexec kill -9 %d", pid);
                    ret = system(command);
                    if (ret == 0) {
                        display_message("Process forcefully terminated with SIGKILL using pkexec.");
                    } else {
                        perror("Failed to terminate the process with SIGKILL using pkexec");
                        display_message("Failed to terminate the process. Check if the PID is correct and you have the necessary permissions.");
                    }
                } else {
                    display_message("Process terminated successfully with SIGTERM using pkexec.");
                }
            } else {
                if (kill(pid, SIGTERM) == 0) {
                    display_message("Process terminated successfully.");
                } else {
                    perror("Failed to terminate the process with SIGTERM");
                    if (kill(pid, SIGKILL) == 0) {
                        display_message("Process forcefully terminated with SIGKILL.");
                    } else {
                        perror("Failed to terminate the process with SIGKILL");
                        display_message("Failed to terminate the process. Check if the PID is correct and you have the necessary permissions.");
                    }
                }
            }
        } else {
            display_message("Invalid PID entered.");
        }
    }

    gtk_widget_destroy(dialog);}
    void user_mode_response_handler(GtkDialog *dialog, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_OK) {
        const char *task = gtk_entry_get_text(GTK_ENTRY(data));
        execute_task(task, 1, 1, 1); // Execute the entered task
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void user_mode(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *label, *grid, *entry;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("User Mode", NULL, flags, "_OK", GTK_RESPONSE_OK, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    label = gtk_label_new("Enter task to execute (e.g., ./calculator):");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

    gtk_widget_show_all(dialog);

    g_signal_connect(dialog, "response", G_CALLBACK(user_mode_response_handler), entry);
}void set_task_resources(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *label, *grid, *entry_ram, *entry_hdd, *entry_cores, *entry_task;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Set Task Resources", NULL, flags, "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    label = gtk_label_new("Enter task name:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    entry_task = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_task, 1, 0, 1, 1);

    label = gtk_label_new("Enter required RAM (MB):");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    entry_ram = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_ram, 1, 1, 1, 1);

    label = gtk_label_new("Enter required HDD (GB):");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    entry_hdd = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_hdd, 1, 2, 1, 1);

    label = gtk_label_new("Enter required Cores:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    entry_cores = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_cores, 1, 3, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *task = gtk_entry_get_text(GTK_ENTRY(entry_task));
        int required_ram = atoi(gtk_entry_get_text(GTK_ENTRY(entry_ram)));
        int required_hdd = atoi(gtk_entry_get_text(GTK_ENTRY(entry_hdd)));
        int required_cores = atoi(gtk_entry_get_text(GTK_ENTRY(entry_cores)));

        execute_task(task, required_ram, required_hdd, required_cores);
    }

    gtk_widget_destroy(dialog);
}
void kernel_mode(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *grid, *view_button, *cancel_button, *set_resources_button;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Kernel Mode", NULL, flags, "_Close", GTK_RESPONSE_CLOSE, NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    view_button = gtk_button_new_with_label("View Processes");
    gtk_widget_set_size_request(view_button, 150, 50);
    g_signal_connect(view_button, "clicked", G_CALLBACK(view_processes), NULL);
    gtk_grid_attach(GTK_GRID(grid), view_button, 0, 0, 1, 1);

    cancel_button = gtk_button_new_with_label("Cancel Process");
    gtk_widget_set_size_request(cancel_button, 150, 50);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(cancel_process), NULL);
    gtk_grid_attach(GTK_GRID(grid), cancel_button, 0, 1, 1, 1);

    set_resources_button = gtk_button_new_with_label("Set Task Resources");
    gtk_widget_set_size_request(set_resources_button, 150, 50);
    g_signal_connect(set_resources_button, "clicked", G_CALLBACK(set_task_resources), NULL);
    gtk_grid_attach(GTK_GRID(grid), set_resources_button, 0, 2, 1, 1);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
void set_background(GtkWidget *window, const gchar *image_file) {
    GtkWidget *fixed;
    GtkWidget *image;

    // Create a fixed container to hold the background image
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    // Load the background image
    image = gtk_image_new_from_file(image_file);

    // Add the image to the fixed container
    gtk_fixed_put(GTK_FIXED(fixed), image, 0, 0);

    // Add the fixed container to the overlay
    gtk_container_add(GTK_CONTAINER(overlay), fixed);
}
void create_main_window() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Fiza's Operating System");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    GdkPixbuf *background_pixbuf = gdk_pixbuf_new_from_file_at_scale("background.jpg", 800, 600, TRUE, NULL);
    GtkWidget *background_image = gtk_image_new_from_pixbuf(background_pixbuf);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), background_image);

    gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(overlay), background_image, TRUE);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(overlay), grid);

    int icon_width = 50;
    int icon_height = 50;
    GdkPixbuf *user_mode_pixbuf = gdk_pixbuf_new_from_file_at_scale("user_mode_icon.png", icon_width, icon_height, TRUE, NULL);
    GdkPixbuf *kernel_mode_pixbuf = gdk_pixbuf_new_from_file_at_scale("kernel_mode_icon.png", icon_width, icon_height, TRUE, NULL);
    GdkPixbuf *exit_pixbuf = gdk_pixbuf_new_from_file_at_scale("exit_icon.png", icon_width, icon_height, TRUE, NULL);

    GtkWidget *user_mode_button = gtk_button_new();
    GtkWidget *kernel_mode_button = gtk_button_new();
    GtkWidget *allocate_resources_button = gtk_button_new();
    GtkWidget *exit_button = gtk_button_new();

    GtkWidget *user_mode_icon = gtk_image_new_from_pixbuf(user_mode_pixbuf);
    GtkWidget *kernel_mode_icon = gtk_image_new_from_pixbuf(kernel_mode_pixbuf);
    GtkWidget *allocate_resources_icon = gtk_image_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    GtkWidget *exit_icon = gtk_image_new_from_pixbuf(exit_pixbuf);

    gtk_button_set_image(GTK_BUTTON(user_mode_button), user_mode_icon);
    gtk_button_set_image(GTK_BUTTON(kernel_mode_button), kernel_mode_icon);
    gtk_button_set_image(GTK_BUTTON(allocate_resources_button), allocate_resources_icon);
    gtk_button_set_image(GTK_BUTTON(exit_button), exit_icon);

    g_signal_connect(user_mode_button, "clicked", G_CALLBACK(user_mode), NULL);
    g_signal_connect(kernel_mode_button, "clicked", G_CALLBACK(kernel_mode), NULL);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    gtk_grid_attach(GTK_GRID(grid), user_mode_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), kernel_mode_button, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), exit_button, 0, 3, 1, 1);

    gtk_widget_show_all(window);
}int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    show_loading_screen();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();

    return 0;
}
