/* hotelbookingreservation-window.c
 *
 * Copyright 2022 LucasFrank254

 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hotelbookingreservation-config.h"
#include "hotelbookingreservation-window.h"
static void open_file (HotelbookingreservationWindow *self,
           GFile            *file);
static void
open_file_complete (GObject          *source_object,
                    GAsyncResult     *result,
                    HotelbookingreservationWindow *self);
static void
hotelbookingreservation__open_file_dialog (GAction          *action G_GNUC_UNUSED,
                                      GVariant         *parameter G_GNUC_UNUSED,
                                      HotelbookingreservationWindow *self);
static void
on_open_response (GtkNativeDialog  *native,
                  int               response,
                  HotelbookingreservationWindow *self);
struct _HotelbookingreservationWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkTextView         *main_text_view;
  GtkButton           *open_button;
};

G_DEFINE_TYPE (HotelbookingreservationWindow, hotelbookingreservation_window, GTK_TYPE_APPLICATION_WINDOW)

static void hotelbookingreservation_window_class_init (HotelbookingreservationWindowClass *klass){
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/hotelReservation/hotelbookingreservation-window.ui");
  gtk_widget_class_bind_template_child (widget_class, HotelbookingreservationWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, HotelbookingreservationWindow, main_text_view);
  gtk_widget_class_bind_template_child (widget_class, HotelbookingreservationWindow, open_button);
}

static void
hotelbookingreservation_window_init (HotelbookingreservationWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  g_autoptr (GSimpleAction) open_action = g_simple_action_new ("open", NULL);
  g_signal_connect (open_action, "activate", G_CALLBACK (hotelbookingreservation__open_file_dialog), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (open_action));
}

static void
hotelbookingreservation__open_file_dialog (GAction          *action G_GNUC_UNUSED,
                                      GVariant         *parameter G_GNUC_UNUSED,
                                      HotelbookingreservationWindow *self)
{
  // Create a new file selection dialog, using the "open" mode
  GtkFileChooserNative *native =
    gtk_file_chooser_native_new ("Open File",
                                 GTK_WINDOW (self),
                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                 "_Open",
                                 "_Cancel");

  // Connect the "response" signal of the file selection dialog;
  // this signal is emitted when the user selects a file, or when
  // they cancel the operation
  g_signal_connect (native,
                    "response",
                    G_CALLBACK (on_open_response),
                    self);

  // Present the dialog to the user
  gtk_native_dialog_show (GTK_NATIVE_DIALOG (native));
}

static void
on_open_response (GtkNativeDialog  *native,
                  int               response,
                  HotelbookingreservationWindow *self)
{
  // If the user selected a file...
  if (response == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);

      // ... retrieve the location from the dialog...
      g_autoptr (GFile) file = gtk_file_chooser_get_file (chooser);

      // ... and open it
      open_file (self, file);
    }

  // Release the reference on the file selection dialog now that we
  // do not need it any more
  g_object_unref (native);
}

static void open_file (HotelbookingreservationWindow *self,
           GFile            *file)//pointer to the file
{
  g_file_load_contents_async (file,
                              NULL,
                              (GAsyncReadyCallback) open_file_complete,
                              self);
}

static void
open_file_complete (GObject          *source_object,
                    GAsyncResult     *result,
                    HotelbookingreservationWindow *self)
{
  GFile *file = G_FILE (source_object);

  g_autofree char *contents = NULL;
  gsize length = 0;

  g_autoptr (GError) error = NULL;

  // Complete the asynchronous operation; this function will either
  // give you the contents of the file as a byte array, or will
  // set the error argument
  g_file_load_contents_finish (file,
                               result,
                               &contents,
                               &length,
                               NULL,
                               &error);

   // Query the display name for the file
  g_autofree char *display_name = NULL;
  g_autoptr (GFileInfo) info =
    g_file_query_info (file,
                       "standard::display-name",
                       G_FILE_QUERY_INFO_NONE,
                       NULL,
                       NULL);
  if (info != NULL)
    {
      display_name =
        g_strdup (g_file_info_get_attribute_string (info, "standard::display-name"));
    }
  else
    {
      display_name = g_file_get_basename (file);
    }
  // In case of error, print a warning to the standard error output
  if (error != NULL)
    {
      g_printerr ("Unable to open the “%s”: %s\n",
                  g_file_peek_path (file),
                  error->message);
      return;
    }

  // Ensure that the file is encoded with UTF-8
  if (!g_utf8_validate (contents, length, NULL))
    {
      g_printerr ("Unable to load the contents of “%s”: "
                  "the file is not encoded with UTF-8\n",
                  g_file_peek_path (file));
      return;
    }

  // Retrieve the GtkTextBuffer instance that stores the
  // text displayed by the GtkTextView widget
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (self->main_text_view);

  // Set the text using the contents of the file
  gtk_text_buffer_set_text (buffer, contents, length);

  // Reposition the cursor so it's at the start of the text
  GtkTextIter start;
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_place_cursor (buffer, &start);
  gtk_window_set_title (GTK_WINDOW (self), display_name);
}


