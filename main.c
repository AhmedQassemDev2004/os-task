#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void listFiles(GtkWidget *, gpointer);
void changePermissions(GtkWidget *, gpointer);
void createFile(GtkWidget *, gpointer);
void deleteFile(GtkWidget *, gpointer);
void createDirectory(GtkWidget *, gpointer);
void deleteDirectory(GtkWidget *, gpointer);
void createSymbolicLink(GtkWidget *, gpointer);
void checkReturnCode(int);

static void activate(GtkApplication *app, gpointer user_data);

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app =
      gtk_application_new("com.example.FileManager", G_APPLICATION_FLAGS_NONE);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;
  GtkWidget *label;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "File Manager");
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  button = gtk_button_new_with_label("List Files/Directories");
  g_signal_connect(button, "clicked", G_CALLBACK(listFiles), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

  button = gtk_button_new_with_label("Change Permissions of a File");
  g_signal_connect(button, "clicked", G_CALLBACK(changePermissions), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);

  button = gtk_button_new_with_label("Create File");
  g_signal_connect(button, "clicked", G_CALLBACK(createFile), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

  button = gtk_button_new_with_label("Delete File");
  g_signal_connect(button, "clicked", G_CALLBACK(deleteFile), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);

  button = gtk_button_new_with_label("Create Directory");
  g_signal_connect(button, "clicked", G_CALLBACK(createDirectory), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);

  button = gtk_button_new_with_label("Delete Directory");
  g_signal_connect(button, "clicked", G_CALLBACK(deleteDirectory), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 1, 1);

  button = gtk_button_new_with_label("Create Symbolic Link");
  g_signal_connect(button, "clicked", G_CALLBACK(createSymbolicLink), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 6, 1, 1);

  label = gtk_label_new("");
  gtk_grid_attach(GTK_GRID(grid), label, 0, 7, 1, 1);

  gtk_widget_show_all(window);
}
void listFiles(GtkWidget *widget, gpointer window) {
  GtkWidget *list_window;
  GtkWidget *list_view;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkWidget *list_scrolled_window;
  gchar buffer[1024];

  list_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(list_window), "List Files/Directories");
  gtk_window_set_default_size(GTK_WINDOW(list_window), 300, 200);

  list_store = gtk_list_store_new(1, G_TYPE_STRING);
  list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
  list_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(list_scrolled_window), list_view);
  gtk_container_add(GTK_CONTAINER(list_window), list_scrolled_window);

  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
      "Files", renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list_view), column);

  FILE *fp = popen("ls", "r");
  if (fp == NULL) {
    perror("Error executing ls");
    return;
  }

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    buffer[strlen(buffer) - 1] = '\0'; // Remove newline character
    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter, 0, buffer, -1);
  }

  pclose(fp);

  gtk_widget_show_all(list_window);
}

void changePermissions(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *file_entry;
  GtkWidget *read_check, *write_check, *execute_check;
  GtkWidget *content_area;
  const gchar *filename;
  gint result;
  gboolean read, write, execute;

  dialog = gtk_dialog_new_with_buttons("Change Permissions", GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK,
                                       "Cancel", GTK_RESPONSE_CANCEL, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  file_entry = gtk_entry_new();
  read_check = gtk_check_button_new_with_label("Read");
  write_check = gtk_check_button_new_with_label("Write");
  execute_check = gtk_check_button_new_with_label("Execute");

  gtk_container_add(GTK_CONTAINER(content_area),
                    gtk_label_new("Enter filename: "));
  gtk_container_add(GTK_CONTAINER(content_area), file_entry);
  gtk_container_add(GTK_CONTAINER(content_area), read_check);
  gtk_container_add(GTK_CONTAINER(content_area), write_check);
  gtk_container_add(GTK_CONTAINER(content_area), execute_check);

  gtk_widget_show_all(dialog);

  result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK) {
    filename = gtk_entry_get_text(GTK_ENTRY(file_entry));
    read = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(read_check));
    write = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(write_check));
    execute = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(execute_check));

    // Calculate numerical mode representation
    mode_t mode = 0;
    if (read)
      mode |= S_IRUSR;
    if (write)
      mode |= S_IWUSR;
    if (execute)
      mode |= S_IXUSR;

    // Set permissions using chmod
    if (chmod(filename, mode) != 0) {
      perror("chmod");
    } else {
      g_print("Permissions changed successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}

void createFile(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *file_entry;
  const gchar *filename;
  FILE *file;

  dialog = gtk_dialog_new_with_buttons("Create File", GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK,
                                       "Cancel", GTK_RESPONSE_CANCEL, NULL);

  file_entry = gtk_entry_new();
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      file_entry);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    filename = gtk_entry_get_text(GTK_ENTRY(file_entry));
    file = fopen(filename, "w");
    if (file == NULL) {
      perror("Error creating file");
    } else {
      fclose(file);
      g_print("File created successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}

void deleteFile(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *file_entry;
  const gchar *filename;

  dialog = gtk_dialog_new_with_buttons("Delete File", GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK,
                                       "Cancel", GTK_RESPONSE_CANCEL, NULL);

  file_entry = gtk_entry_new();
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      file_entry);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    filename = gtk_entry_get_text(GTK_ENTRY(file_entry));
    if (remove(filename) != 0) {
      perror("Error deleting file");
    } else {
      g_print("File deleted successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}

void createDirectory(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *dir_entry;
  const gchar *dirname;

  dialog = gtk_dialog_new_with_buttons("Create Directory", GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK,
                                       "Cancel", GTK_RESPONSE_CANCEL, NULL);

  dir_entry = gtk_entry_new();
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      dir_entry);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    dirname = gtk_entry_get_text(GTK_ENTRY(dir_entry));
    if (mkdir(dirname, 0777) != 0) {
      perror("Error creating directory");
    } else {
      g_print("Directory created successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}

void deleteDirectory(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *dir_entry;
  const gchar *dirname;

  dialog = gtk_dialog_new_with_buttons("Delete Directory", GTK_WINDOW(window),
                                       GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK,
                                       "Cancel", GTK_RESPONSE_CANCEL, NULL);

  dir_entry = gtk_entry_new();
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      dir_entry);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    dirname = gtk_entry_get_text(GTK_ENTRY(dir_entry));
    if (rmdir(dirname) != 0) {
      perror("Error deleting directory");
    } else {
      g_print("Directory deleted successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}

void createSymbolicLink(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  GtkWidget *target_entry;
  GtkWidget *link_entry;
  const gchar *targetname;
  const gchar *linkname;

  dialog = gtk_dialog_new_with_buttons(
      "Create Symbolic Link", GTK_WINDOW(window), GTK_DIALOG_MODAL, "OK",
      GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);

  target_entry = gtk_entry_new();
  link_entry = gtk_entry_new();
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      gtk_label_new("Enter target filename: "));
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      target_entry);
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      gtk_label_new("Enter symbolic link name: "));
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
      link_entry);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    targetname = gtk_entry_get_text(GTK_ENTRY(target_entry));
    linkname = gtk_entry_get_text(GTK_ENTRY(link_entry));
    if (symlink(targetname, linkname) != 0) {
      perror("Error creating symbolic link");
    } else {
      g_print("Symbolic link created successfully.\n");
    }
  }

  gtk_widget_destroy(dialog);
}
void checkReturnCode(int returnCode) {
  if (returnCode == -1) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Error: Failed to execute command!");
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
  } else {
    GtkWidget *success_dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                               GTK_BUTTONS_OK, "Success: Command executed!");
    gtk_dialog_run(GTK_DIALOG(success_dialog));
    gtk_widget_destroy(success_dialog);
  }
}
