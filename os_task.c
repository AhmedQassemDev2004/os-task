#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>

// Global variables
GtkWidget *file_list_view; // File list TextView
GtkWidget *source_entry;
GtkWidget *destination_entry;
GtkWidget *read_check;
GtkWidget *write_check;
GtkWidget *execute_check;

// Function declarations
void listFiles(GtkWidget *widget, gpointer data);
void showChangePermissionsDialog(GtkWidget *widget, gpointer data);
void changePermissions(GtkWidget *read_check, GtkWidget *write_check,
                       GtkWidget *execute_check, gpointer data);
void showCreateFileDialog(GtkWidget *widget, gpointer data);
void createFile(GtkWidget *widget, gpointer data);
void showDeleteFileDialog(GtkWidget *widget, gpointer data);
void deleteFile(GtkWidget *widget, gpointer data);
void showMoveFileDialog(GtkWidget *widget, gpointer data);
void moveFile(GtkWidget *widget, gpointer data);
void showCopyFileDialog(GtkWidget *widget, gpointer data);
void copyFile(GtkWidget *widget, gpointer data);
void showSymLinkDialog(GtkWidget *widget, gpointer data);
void createSymLink(GtkWidget *widget, gpointer data);
void displayMessage(const gchar *message);

gboolean file_exists(const char *filename) {
  return access(filename, F_OK) != -1;
}

void execute_system_command(const char *command, const char *success_message) {
  int returnCode = system(command);
  if (returnCode == -1) {
    displayMessage("Error: Failed to execute command!");
  } else {
    displayMessage(success_message);
  }
}

void displayMessage(const gchar *message) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK, "%s", message);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void createAndShowWindow(GtkWidget *window) { gtk_widget_show_all(window); }

void listFiles(GtkWidget *widget, gpointer data) {
  // Clear the existing text in the TextView
  GtkTextBuffer *buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(file_list_view));
  gtk_text_buffer_set_text(buffer, "", -1);

  // Execute 'ls' command and display the result in TextView
  FILE *fp;
  char line[256];
  fp = popen("ls -l", "r");
  if (fp == NULL) {
    displayMessage("Error: Failed to list files!");
    return;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    gtk_text_buffer_insert_at_cursor(buffer, line, -1);
  }
  pclose(fp);
}

void showChangePermissionsDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *filename_label;
  GtkWidget *filename_entry;
  GtkWidget *read_check;
  GtkWidget *write_check;
  GtkWidget *execute_check;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons(
      "Change Permissions", NULL, GTK_DIALOG_MODAL, "Cancel",
      GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  filename_label = gtk_label_new("Filename/Directory:");
  gtk_grid_attach(GTK_GRID(grid), filename_label, 0, 0, 1, 1);
  filename_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), filename_entry, 1, 0, 1, 1);

  read_check = gtk_check_button_new_with_label("Read");
  gtk_grid_attach(GTK_GRID(grid), read_check, 0, 1, 1, 1);

  write_check = gtk_check_button_new_with_label("Write");
  gtk_grid_attach(GTK_GRID(grid), write_check, 1, 1, 1, 1);

  execute_check = gtk_check_button_new_with_label("Execute");
  gtk_grid_attach(GTK_GRID(grid), execute_check, 2, 1, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(changePermissions), dialog);

  gtk_widget_show_all(dialog);
}

void changePermissions(GtkWidget *read_check, GtkWidget *write_check,
                       GtkWidget *execute_check, gpointer data) {
  const gchar *filename = gtk_entry_get_text(GTK_ENTRY(data));
  gchar *permission = g_strdup("");

  if (!file_exists(filename)) {
    displayMessage("Error: File or directory does not exist!");
    g_free(permission);
    return;
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(read_check))) {
    permission = g_strconcat(permission, "r", NULL);
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(write_check))) {
    permission = g_strconcat(permission, "w", NULL);
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(execute_check))) {
    permission = g_strconcat(permission, "x", NULL);
  }

  char command[150];
  snprintf(command, sizeof(command), "chmod %s %s", permission, filename);
  execute_system_command(command, "Permissions changed successfully!");
  g_free(permission);
}

void showCreateFileDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *filename_label;
  GtkWidget *filename_entry;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons("Create File", NULL, GTK_DIALOG_MODAL,
                                       "Cancel", GTK_RESPONSE_CANCEL, "Create",
                                       GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  filename_label = gtk_label_new("Filename:");
  gtk_grid_attach(GTK_GRID(grid), filename_label, 0, 0, 1, 1);
  filename_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), filename_entry, 1, 0, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(createFile), filename_entry);

  createAndShowWindow(dialog);
}

void createFile(GtkWidget *widget, gpointer data) {
  const gchar *filename = gtk_entry_get_text(GTK_ENTRY(data));

  if (file_exists(filename)) {
    displayMessage("Error: File already exists!");
    return;
  }

  char command[150];
  snprintf(command, sizeof(command), "touch %s", filename);
  execute_system_command(command, "File created successfully!");
}

void showDeleteFileDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *filename_label;
  GtkWidget *filename_entry;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons("Delete File", NULL, GTK_DIALOG_MODAL,
                                       "Cancel", GTK_RESPONSE_CANCEL, "Delete",
                                       GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  filename_label = gtk_label_new("Filename:");
  gtk_grid_attach(GTK_GRID(grid), filename_label, 0, 0, 1, 1);
  filename_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), filename_entry, 1, 0, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(deleteFile), filename_entry);

  createAndShowWindow(dialog);
}

void deleteFile(GtkWidget *widget, gpointer data) {
  const gchar *filename = gtk_entry_get_text(GTK_ENTRY(data));

  if (!file_exists(filename)) {
    displayMessage("Error: File does not exist!");
    return;
  }

  char command[150];
  snprintf(command, sizeof(command), "rm %s", filename);
  execute_system_command(command, "File deleted successfully!");
}

void showMoveFileDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *source_label;
  GtkWidget *destination_label;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons("Move File", NULL, GTK_DIALOG_MODAL,
                                       "Cancel", GTK_RESPONSE_CANCEL, "Move",
                                       GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  source_label = gtk_label_new("Source:");
  gtk_grid_attach(GTK_GRID(grid), source_label, 0, 0, 1, 1);
  source_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), source_entry, 1, 0, 1, 1);

  destination_label = gtk_label_new("Destination:");
  gtk_grid_attach(GTK_GRID(grid), destination_label, 0, 1, 1, 1);
  destination_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), destination_entry, 1, 1, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(moveFile), NULL);

  createAndShowWindow(dialog);
}

void moveFile(GtkWidget *widget, gpointer data) {
  const gchar *source = gtk_entry_get_text(GTK_ENTRY(source_entry));
  const gchar *destination = gtk_entry_get_text(GTK_ENTRY(destination_entry));

  if (!file_exists(source)) {
    displayMessage("Error: Source file does not exist!");
    return;
  }

  char command[250];
  snprintf(command, sizeof(command), "mv %s %s", source, destination);
  execute_system_command(command, "File moved successfully!");
}

void showCopyFileDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *source_label;
  GtkWidget *destination_label;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons("Copy File", NULL, GTK_DIALOG_MODAL,
                                       "Cancel", GTK_RESPONSE_CANCEL, "Copy",
                                       GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  source_label = gtk_label_new("Source:");
  gtk_grid_attach(GTK_GRID(grid), source_label, 0, 0, 1, 1);
  source_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), source_entry, 1, 0, 1, 1);

  destination_label = gtk_label_new("Destination:");
  gtk_grid_attach(GTK_GRID(grid), destination_label, 0, 1, 1, 1);
  destination_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), destination_entry, 1, 1, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(copyFile), NULL);

  createAndShowWindow(dialog);
}

void copyFile(GtkWidget *widget, gpointer data) {
  const gchar *source = gtk_entry_get_text(GTK_ENTRY(source_entry));
  const gchar *destination = gtk_entry_get_text(GTK_ENTRY(destination_entry));

  if (!file_exists(source)) {
    displayMessage("Error: Source file does not exist!");
    return;
  }

  char command[250];
  snprintf(command, sizeof(command), "cp %s %s", source, destination);
  execute_system_command(command, "File copied successfully!");
}

void showSymLinkDialog(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog;
  GtkWidget *grid;
  GtkWidget *source_label;
  GtkWidget *destination_label;
  GtkWidget *button;
  GtkWidget *content_area;

  dialog = gtk_dialog_new_with_buttons(
      "Create Symbolic Link", NULL, GTK_DIALOG_MODAL, "Cancel",
      GTK_RESPONSE_CANCEL, "Create", GTK_RESPONSE_OK, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(content_area), grid);

  source_label = gtk_label_new("Source:");
  gtk_grid_attach(GTK_GRID(grid), source_label, 0, 0, 1, 1);
  source_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), source_entry, 1, 0, 1, 1);

  destination_label = gtk_label_new("Destination:");
  gtk_grid_attach(GTK_GRID(grid), destination_label, 0, 1, 1, 1);
  destination_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), destination_entry, 1, 1, 1, 1);

  button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  g_signal_connect(button, "clicked", G_CALLBACK(createSymLink), NULL);

  createAndShowWindow(dialog);
}

void createSymLink(GtkWidget *widget, gpointer data) {
  const gchar *source = gtk_entry_get_text(GTK_ENTRY(source_entry));
  const gchar *destination = gtk_entry_get_text(GTK_ENTRY(destination_entry));

  if (!file_exists(source)) {
    displayMessage("Error: Source file does not exist!");
    return;
  }

  char command[250];
  snprintf(command, sizeof(command), "ln -s %s %s", source, destination);
  execute_system_command(command, "Symbolic link created successfully!");
}

int main(int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "File Manager");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_widget_set_size_request(window, 400, 400);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  // List files by default
  listFiles(NULL, NULL);

  button = gtk_button_new_with_label("List Files");
  g_signal_connect(button, "clicked", G_CALLBACK(listFiles), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

  button = gtk_button_new_with_label("Change Permissions");
  g_signal_connect(button, "clicked", G_CALLBACK(showChangePermissionsDialog),
                   NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  button = gtk_button_new_with_label("Create File");
  g_signal_connect(button, "clicked", G_CALLBACK(showCreateFileDialog), NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  button = gtk_button_new_with_label("Delete File");
  g_signal_connect(button, "clicked", G_CALLBACK(showDeleteFileDialog), NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  button = gtk_button_new_with_label("Move File");
  g_signal_connect(button, "clicked", G_CALLBACK(showMoveFileDialog), NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  button = gtk_button_new_with_label("Copy File");
  g_signal_connect(button, "clicked", G_CALLBACK(showCopyFileDialog), NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  button = gtk_button_new_with_label("Create Symbolic Link");
  g_signal_connect(button, "clicked", G_CALLBACK(showSymLinkDialog), NULL);
  gtk_grid_attach_next_to(GTK_GRID(grid), button, NULL, GTK_POS_BOTTOM, 1, 1);

  file_list_view = gtk_text_view_new();
  gtk_grid_attach(GTK_GRID(grid), file_list_view, 1, 0, 1, 7);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
