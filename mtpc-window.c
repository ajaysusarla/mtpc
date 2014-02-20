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
#include "mtpc-devicelist.h"
#include "mtpc-statusbar.h"

typedef struct {
	GtkWidget *grid;
	GtkWidget *toolbar;
	GtkWidget *main_vbox;

	GtkWidget *right_container;
	GtkWidget *left_container;
	GtkWidget *sidebar;
	GtkWidget *device_property_box;
	GtkWidget *statusbar;

	GtkWidget *devicelist;

	GCancellable *cancellable;

} MtpcWindowPrivate;


G_DEFINE_TYPE_WITH_PRIVATE(MtpcWindow, mtpc_window, GTK_TYPE_APPLICATION_WINDOW);


/* callbacks and internal methods */
static gboolean update_statusbar(gpointer data)
{
	MtpcStatusbar *statusbar = MTPC_STATUSBAR(data);

	mtpc_statusbar_set_list_info(statusbar, "|");
	mtpc_statusbar_set_list_info(statusbar, "/");
	mtpc_statusbar_set_list_info(statusbar, "-");
	mtpc_statusbar_set_list_info(statusbar, "\\");
	mtpc_statusbar_set_list_info(statusbar, "|");
	return TRUE;
}

typedef struct {
	libmtp_dev_t *devices;
	MtpcWindow *window;
	gint func_ref;
} DeviceAsyncFetchData;

static void fetch_devices_data_free(gpointer user_data)
{
	DeviceAsyncFetchData *data = (DeviceAsyncFetchData *)user_data;

	mtpc_device_free_devices(data->devices);
	g_slice_free(DeviceAsyncFetchData, data);
}

static void fetch_devices_task_finish(GObject *self,
				      GAsyncResult *result,
				      gpointer task_data)
{
	DeviceAsyncFetchData *data =  (DeviceAsyncFetchData *)task_data;

	g_return_val_if_fail(g_task_is_valid(result, self), NULL);
	g_source_remove (data->func_ref);
}

static void fetch_devices_task_thread_func(GTask *task,
					   gpointer source,
					   gpointer task_data,
					   GCancellable *cancellable)
{
	int i;
	DeviceAsyncFetchData *data =  (DeviceAsyncFetchData *)task_data;
	libmtp_dev_t *devices = data->devices;
	MtpcWindow *window = data->window;
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(window);

	i = devices->numrawdevices;

	while (--i >= 0) {
		Device *device;

		device = mtpc_device_new_from_raw_device(&devices->rawdevices[i]);
		mtpc_devicelist_append_item(MTPC_DEVICELIST(priv->devicelist),
					    i, NULL,
					    device);
	}
}

static void fetch_devices_async(MtpcWindow *window)
{
	libmtp_err_t err;
	DeviceAsyncFetchData *data;
	GTask *fetch_devices_task;
	MtpcWindowPrivate *priv;
	MtpcStatusbar *statusbar;

	data = g_slice_new0(DeviceAsyncFetchData);

	priv = mtpc_window_get_instance_private(window);
	statusbar = MTPC_STATUSBAR(priv->statusbar);

	mtpc_devicelist_clear(MTPC_DEVICELIST(priv->devicelist));

	data->window = window;
	data->devices = mtpc_device_alloc_devices();

	err = mtpc_device_detect_devices(data->devices);

	if (err == LIBMTP_ERROR_NO_DEVICE_ATTACHED) {
		MTPC_STATUSBAR_UPDATE(statusbar, "", "No MTP devices found", "");
		mtpc_device_free_devices(data->devices);
	} else if (err == LIBMTP_ERROR_NONE) {
		MTPC_STATUSBAR_UPDATE(statusbar, "", "MTP devices found..", "");
		data->func_ref = g_timeout_add(100, update_statusbar,
					       statusbar);
		fetch_devices_task = g_task_new(NULL, priv->cancellable,
						fetch_devices_task_finish,
						data);
		g_task_set_task_data(fetch_devices_task, data,
				     fetch_devices_data_free);

		g_task_run_in_thread(fetch_devices_task,
				     fetch_devices_task_thread_func);

		g_object_unref(fetch_devices_task);
	}

	return;
}

static void refresh_device_list_cb(GSimpleAction *action,
				   GVariant *parameter,
				   gpointer user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv;
	MtpcStatusbar *statusbar;

	priv = mtpc_window_get_instance_private(window);

	statusbar = MTPC_STATUSBAR(priv->statusbar);

	MTPC_STATUSBAR_UPDATE(statusbar, "", "Detecting MTP Devices..", "");

	fetch_devices_async(window);

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

static void change_statusbar_view_state(GSimpleAction *action,
					GVariant      *state,
					gpointer       user_data)
{
	if (g_variant_get_boolean(state)) {
		printf("show statusbar\n");
	} else {
		printf("hide statusbar\n");
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
        { "toggle-status-bar", toggle_action_activated, NULL, "true", change_statusbar_view_state },
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
	GtkWidget *devlist_scrolled;

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

	/* statusbar */
	priv->statusbar = mtpc_statusbar_new();
	gtk_widget_show(priv->statusbar);
        gtk_box_pack_end(GTK_BOX(priv->main_vbox),
			 priv->statusbar, FALSE, FALSE, 3);

	/*
        g_signal_connect_after(G_OBJECT(main_vbox), "realize",
                               G_CALLBACK(mainwindow_view_three_pane), NULL);
	*/

	/* device list */
	devlist_scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(devlist_scrolled),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(devlist_scrolled),
                                            GTK_SHADOW_IN);
	gtk_widget_show(devlist_scrolled);

	priv->devicelist = mtpc_devicelist_new();
	gtk_container_add(GTK_CONTAINER(devlist_scrolled), priv->devicelist);
	gtk_box_pack_start(GTK_BOX(vbox), devlist_scrolled, TRUE, TRUE, 0);



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

