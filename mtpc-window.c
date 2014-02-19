/*
 * mtpc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "gtk-utils.h"
#include "glib-utils.h"
#include "mtpc-window.h"
#include "mtpc-actions-callbacks.h"

typedef struct {
	GtkWidget *grid;
	GtkWidget *toolbar;
	GtkWidget *main_vbox;

	GtkWidget *right_container;
	GtkWidget *left_container;
	GtkWidget *sidebar;
	GtkWidget *device_property_box;
} MtpcWindowPrivate;


G_DEFINE_TYPE_WITH_PRIVATE(MtpcWindow, mtpc_window, GTK_TYPE_APPLICATION_WINDOW);


/* callbacks and internal methods */
void refresh_device_list_cb(GSimpleAction *action,
			    GVariant *parameter,
			    gpointer user_data)
{
	printf("refresh_device_list_cb\n");
	return;
}

static void device_connect_cb(GSimpleAction *action,
			      GVariant      *state,
			      gpointer       user_data)
{
}

static void device_disconnect_cb(GSimpleAction *action,
				 GVariant      *state,
				 gpointer       user_data)
{
	printf("disconnect device\n");
}

static void cut_cb(GSimpleAction *action,
		   GVariant      *state,
		   gpointer       user_data)
{
}

static void copy_cb(GSimpleAction *action,
		    GVariant      *state,
		    gpointer       user_data)
{
}

static void paste_cb(GSimpleAction *action,
		     GVariant      *state,
		     gpointer       user_data)
{
}

static void change_home_folder_view_state(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)
{
	if (g_variant_get_boolean(state)) {
		printf("show home folder\n");
	} else {
		printf("hide home folder\n");
	}

	g_simple_action_set_state(action, state);
}

static void change_device_properties_view(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)
{
	if (g_variant_get_boolean(state)) {
		printf("show device properties\n");
	} else {
		printf("hide device properties\n");
	}

	g_simple_action_set_state(action, state);
}

static void _mtpc_window_set_default_size(GtkWidget *window, GdkScreen *screen)
{
        int max_width;
        int max_height;

        max_width = gdk_screen_get_width(screen) * 5/6;
        max_height = gdk_screen_get_height(screen) * 3/4;

        gtk_window_set_default_size(GTK_WINDOW(window), max_width, max_height);
}

static GtkWidget *_mtpc_window_create_toolbar()
{
	GtkWidget *toolbar;
	GtkToolItem *button;

	toolbar = gtk_toolbar_new ();
	button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "gtk-refresh");
	gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.refresh-device-list");
	gtk_container_add (GTK_CONTAINER (toolbar), GTK_WIDGET (button));

	button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "media-eject");
	gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.disconnect");
	gtk_container_add (GTK_CONTAINER (toolbar), GTK_WIDGET (button));

	button = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "gtk-properties");
	gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "win.toggle-device-properties");
	gtk_container_add (GTK_CONTAINER (toolbar), GTK_WIDGET (button));


	return toolbar;
}

static GActionEntry win_entries[] = {
	/* device */
        { "refresh-device-list", refresh_device_list_cb, NULL, NULL, NULL },
        { "connect", device_connect_cb, NULL, NULL, NULL },
        { "disconnect", device_disconnect_cb, NULL, NULL, NULL },
	/* edit */
        { "cut", cut_cb, NULL, NULL, NULL },
        { "copy", copy_cb, NULL, NULL, NULL },
        { "paste", paste_cb, NULL, NULL, NULL },
	/* view */
        { "toggle-home-folder", toggle_action_activated, NULL, "false", change_home_folder_view_state },
        { "toggle-device-properties", toggle_action_activated, NULL, "false", change_device_properties_view },
};

/* class implementation */
static void mtpc_window_dispose(GObject *object)
{
	MtpcWindow *window = MTPC_WINDOW(object);
	MtpcWindowPrivate *priv;

	priv = mtpc_window_get_instance_private(window);


	G_OBJECT_CLASS(mtpc_window_parent_class)->dispose(object);
}

static void mtpc_window_class_init(MtpcWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = mtpc_window_dispose;
}

static void mtpc_window_init(MtpcWindow *win)
{
	GdkScreen *screen;
	GtkWidget *window;
	GtkWidget *vbox;

	MtpcWindowPrivate *priv;

	priv = mtpc_window_get_instance_private(win);

	window = GTK_WIDGET(win);

	/* window size */
	screen = gdk_screen_get_default();
	_mtpc_window_set_default_size(GTK_WIDGET(win), screen);


	/* actions */
	g_action_map_add_action_entries(G_ACTION_MAP(window),
					win_entries, G_N_ELEMENTS(win_entries),
					window);

	/* foundation */
	priv->grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER (window), priv->grid);

	/* toolbar */
	priv->toolbar = _mtpc_window_create_toolbar();
	gtk_grid_attach (GTK_GRID (priv->grid), priv->toolbar, 0, 0, 1, 1);


	/** main view **/
	priv->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_hexpand (priv->main_vbox, TRUE);
	gtk_widget_set_vexpand (priv->main_vbox, TRUE);

        priv->right_container = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(priv->main_vbox),
			   priv->right_container, TRUE, TRUE, 3);

        priv->left_container = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_paned_pack1(GTK_PANED(priv->right_container),
			priv->left_container, TRUE, TRUE);

        priv->sidebar = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
        gtk_paned_pack1(GTK_PANED(priv->left_container),
			priv->sidebar, FALSE, FALSE);

        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(vbox),
                                    GTK_STYLE_CLASS_SIDEBAR);
        gtk_widget_set_size_request (vbox, -1, 50);
        gtk_paned_pack1(GTK_PANED(priv->sidebar), vbox, TRUE, FALSE);

	priv->device_property_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
        gtk_paned_pack2(GTK_PANED(priv->sidebar),
			priv->device_property_box, TRUE, FALSE);

	/*
        g_signal_connect_after(G_OBJECT(main_vbox), "realize",
                               G_CALLBACK(mainwindow_view_three_pane), NULL);
	*/



	/* add the main_vbox to the container */
	gtk_grid_attach (GTK_GRID (priv->grid), priv->main_vbox, 0, 1, 1, 1);

}

/* public methods */
GtkWidget *mtpc_window_new(MtpcApp *application)
{
	MtpcWindow *window;
	MtpcWindowPrivate *priv;

	window = g_object_new(MTPC_TYPE_WINDOW,
			      "application", application,
			      NULL);

	priv = mtpc_window_get_instance_private(window);


	return GTK_WIDGET(window);
}
