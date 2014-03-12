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
#include <string.h>
#include <gtk/gtk.h>
#include "mtpc-devicelist.h"

enum {
	FOLDER_POPUP,
	LIST_CHILDREN,
	LOAD,
	OPEN,
	LAST_SIGNAL
};

enum {
        COL_DEVICE_INDEX = 0,
        COL_DEVICE_MODEL,
        COL_DEVICE_MFR,
        COL_DEVICE_ITEM,
        COL_DEVICE_ICON,
        COL_DEVICE_EJECT,
        COL_DEVICE_NO_EJECT,
        N_COLS
};


typedef struct {
	GHashTable *entry_points;

	GtkTreeStore *tree_store;
	GtkCellRenderer *text_renderer;
	GtkTreePath *hover_path;
} MtpcDevicelistPrivate;


static guint mtpc_devicelist_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(MtpcDevicelist,
			   mtpc_devicelist,
			   GTK_TYPE_TREE_VIEW);



/* internal private functions */
static void add_columns(MtpcDevicelist *device_list, GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_spacing(column, 1);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, COL_DEVICE_MODEL);
        gtk_tree_view_column_set_title(column, "Device");


	renderer = gtk_cell_renderer_pixbuf_new();
        g_object_set(renderer, "ypad", 0, NULL);
        gtk_tree_view_column_pack_start(column, renderer, FALSE);
        gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", COL_DEVICE_ICON,
					    "pixbuf-eject", COL_DEVICE_EJECT,
					    "pixbuf-no-eject", COL_DEVICE_EJECT,
					    NULL);

        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "ypad", 0, NULL);
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_set_attributes(column, renderer,
					    "text", COL_DEVICE_MODEL,
					    NULL);

        gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
        gtk_tree_view_set_expander_column (GTK_TREE_VIEW(treeview), column);
}

static gboolean popup_menu_cb(GtkWidget *widget, gpointer user_data)
{
	MtpcDevicelist *devicelist = MTPC_DEVICELIST(user_data);
	MtpcDevicelistPrivate *priv;
	GtkTreeStore *tree_store;
	GtkTreeIter iter;
	Device *device;

	priv = mtpc_devicelist_get_instance_private(devicelist);
	tree_store = priv->tree_store;

	if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(devicelist)), NULL, &iter)) {

			gtk_tree_model_get(GTK_TREE_MODEL(tree_store),
					   &iter,
					   COL_DEVICE_ITEM, &device,
					   -1);

			if (!device)
				return FALSE;
	}

	g_signal_emit(devicelist,
		      mtpc_devicelist_signals[FOLDER_POPUP],
		      0,
		      device);


	return TRUE;
}

static gboolean button_press_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       gpointer user_data)
{
	MtpcDevicelist *devicelist = MTPC_DEVICELIST(user_data);
	MtpcDevicelistPrivate *priv;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean retval;
	GtkTreeViewColumn *column;
	int cell_x, cell_y;

	retval = FALSE;

	priv = mtpc_devicelist_get_instance_private(devicelist);

	gtk_widget_grab_focus(widget);

	if ((event->state & GDK_SHIFT_MASK) || (event->state & GDK_CONTROL_MASK))
		return retval;

	if ((event->button != 1) && (event->button != 3))
		return retval;

	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(devicelist),
					   event->x, event->y,
					   &path,
					   &column,
					   &cell_x,
					   &cell_y))
	{
		if (event->button == 3) {
			g_signal_emit(devicelist,
				      mtpc_devicelist_signals[FOLDER_POPUP],
				      0,
				      NULL);
			retval = TRUE;
		}

		return retval;
	}

	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->tree_store),
				    &iter,
				    path))
	{
		gtk_tree_path_free(path);
		return retval;
	}

	if (event->button == 3) {
		Device *device;

		gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store),
				   &iter,
				   COL_DEVICE_ITEM, &device,
				   -1);

		if (device && device->device) {
			g_signal_emit(devicelist,
				      mtpc_devicelist_signals[FOLDER_POPUP],
				      0,
				      device);

			retval = TRUE;
		}
	}

	gtk_tree_path_free(path);

	return retval;
}

static gboolean button_release_event_cb(GtkWidget *widget,
					GdkEventButton *event,
					gpointer user_data)
{
	return FALSE;
}

static void load_contents_of_device(MtpcDevicelist *devicelist, Device *device)
{
	g_signal_emit(devicelist,
		      mtpc_devicelist_signals[LOAD],
		      0,
		      device);
}

static gboolean selection_changed_cb(GtkTreeSelection *selection,
				     gpointer user_data)
{
	MtpcDevicelist *devicelist = user_data;
	GtkTreeIter iter;
	GtkTreePath *selected_path;
	MtpcDevicelistPrivate *priv;
	Device *device;

	priv = mtpc_devicelist_get_instance_private(devicelist);


	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return FALSE;

	selected_path = gtk_tree_model_get_path(GTK_TREE_MODEL(priv->tree_store),
						&iter);

	gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store),
			   &iter,
			   COL_DEVICE_ITEM, &device,
			   -1);

	/* load the device contents here */
	load_contents_of_device(devicelist, device);

	gtk_tree_path_free(selected_path);

	return FALSE;
}

static void open_device(MtpcDevicelist *devicelist, Device *device)
{
	g_signal_emit(devicelist, mtpc_devicelist_signals[OPEN], 0, device);
}

static gboolean row_activated_cb(GtkTreeView *tree_view,
				 GtkTreePath *path,
				 GtkTreeViewColumn *column,
				 gpointer user_data)
{
	MtpcDevicelist *devicelist = MTPC_DEVICELIST(user_data);
	MtpcDevicelistPrivate *priv;
	GtkTreeIter iter;
	Device *device;

	priv = mtpc_devicelist_get_instance_private(devicelist);

	if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->tree_store),
				    &iter,
				    path))
	{
		return FALSE;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store),
			   &iter,
			   COL_DEVICE_ITEM, &device,
			   -1);

	open_device(devicelist, device);

	return TRUE;
}

static gboolean row_expanded_cb(GtkTreeView  *tree_view,
				GtkTreeIter  *expanded_iter,
				GtkTreePath  *expanded_path,
				gpointer      user_data)
{
	return FALSE;
}

static gboolean row_collapsed_cb(GtkTreeView *tree_view,
				 GtkTreeIter *iter,
				 GtkTreePath *path,
				 gpointer     user_data)
{
	return FALSE;
}

static void _mtpc_devicelist_add_empty_item(MtpcDevicelist *device_list,
					    GtkTreeIter *parent,
					    gboolean forced)
{
}

/* From gthumb: */
/* After changing the children list, the node expander is not highlighted
 * anymore, this prevents the user to close the expander without moving the
 * mouse pointer.  The problem can be fixed emitting a fake motion notify
 * event, this way the expander gets highlighted again and a click on the
 * expander will correctly collapse the node. */
static void emit_fake_motino_notify_event(MtpcDevicelist *device_list)
{
	GtkWidget *widget = GTK_WIDGET(device_list);
	GdkDevice *device;
	GdkWindow *window;
	GdkEventMotion event;
	int x, y;

	if (!gtk_widget_get_realized(widget))
		return;

	device = gdk_device_manager_get_client_pointer(
		gdk_display_get_device_manager(
			gtk_widget_get_display(GTK_WIDGET(device_list))));

	window = gdk_window_get_device_position(gtk_widget_get_window(widget),
						device,
						&x,
						&y,
						NULL);

	event.type = GDK_MOTION_NOTIFY;
	event.window = (window != NULL) ? window : gtk_tree_view_get_bin_window(GTK_TREE_VIEW(device_list));
	event.send_event = TRUE;
	event.time = GDK_CURRENT_TIME;
	event.x = x;
	event.y = y;
	event.axes = NULL;
	event.state = 0;
	event.is_hint = FALSE;
	event.device = device;

	GTK_WIDGET_GET_CLASS(device_list)->motion_notify_event(widget, &event);
}

/* class implementation */
static void mtpc_devicelist_finalize(GObject *object)
{
	MtpcDevicelist *device_list = MTPC_DEVICELIST(object);
	MtpcDevicelistPrivate *priv;

	priv = mtpc_devicelist_get_instance_private(device_list);


	g_hash_table_unref(priv->entry_points);

	G_OBJECT_CLASS(mtpc_devicelist_parent_class)->finalize(object);
}

static void mtpc_devicelist_class_init(MtpcDevicelistClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = mtpc_devicelist_finalize;


	mtpc_devicelist_signals[FOLDER_POPUP] =
		g_signal_new("folder_popup",
			     G_TYPE_FROM_CLASS(klass),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, folder_popup),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);

	mtpc_devicelist_signals[LIST_CHILDREN] =
		g_signal_new("list_children",
			     G_TYPE_FROM_CLASS(klass),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, list_children),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);

	mtpc_devicelist_signals[LOAD] =
		g_signal_new("load",
			     G_TYPE_FROM_CLASS(klass),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, load),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);

	mtpc_devicelist_signals[OPEN] =
		g_signal_new("open",
			     G_TYPE_FROM_CLASS(klass),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, open),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__POINTER,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_POINTER);
}

static void mtpc_devicelist_init(MtpcDevicelist *device_list)
{
	GtkTreeSelection *selection;
	MtpcDevicelistPrivate *priv;

	priv = mtpc_devicelist_get_instance_private(device_list);

	priv->entry_points = g_hash_table_new_full(g_file_hash,
						   (GEqualFunc)g_file_equal,
						   g_object_unref,
						   NULL);

	priv->tree_store = gtk_tree_store_new(N_COLS,
					      G_TYPE_INT,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_POINTER,
					      GDK_TYPE_PIXBUF,
					      G_TYPE_BOOLEAN,
					      G_TYPE_BOOLEAN);
	gtk_tree_view_set_model(GTK_TREE_VIEW(device_list),
				GTK_TREE_MODEL(priv->tree_store));

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(device_list), FALSE);

	add_columns(device_list, GTK_TREE_VIEW(device_list));

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(device_list), TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(device_list),
					COL_DEVICE_MODEL);
        gtk_tree_view_set_reorderable(GTK_TREE_VIEW(device_list), FALSE);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(device_list));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	g_signal_connect(selection, "changed",
			 G_CALLBACK(selection_changed_cb),
			 device_list);

	/* signal handlers */
	g_signal_connect(device_list,
			 "popup-menu",
			 G_CALLBACK(popup_menu_cb),
			 device_list);
	g_signal_connect(device_list,
			 "button-press-event",
			 G_CALLBACK(button_press_cb),
			 device_list);
	g_signal_connect(device_list,
			 "button-release-event",
			 G_CALLBACK(button_release_event_cb),
			 device_list);
	g_signal_connect(device_list,
			 "row-activated",
			 G_CALLBACK(row_activated_cb),
			 device_list);
	g_signal_connect(device_list,
			 "row-expanded",
			 G_CALLBACK(row_expanded_cb),
			 device_list);
	g_signal_connect(device_list,
			 "row-collapsed",
			 G_CALLBACK(row_collapsed_cb),
			 device_list);
}

/* public functions */
GtkWidget *mtpc_devicelist_new(void)
{
	MtpcDevicelist *device_list;

	device_list = g_object_new(MTPC_TYPE_DEVICELIST, NULL);

	return GTK_WIDGET(device_list);
}

gboolean mtpc_devicelist_append_item(MtpcDevicelist *device_list,
				     int index,
				     GtkTreeIter *iter,
				     Device *device)
{
	MtpcDevicelistPrivate *priv;
	GdkPixbuf *pixbuf = NULL;

        g_return_val_if_fail(device != NULL, FALSE);
        g_return_val_if_fail(device->device != NULL, FALSE);

	priv = mtpc_devicelist_get_instance_private(device_list);

	mtpc_device_add(device);

	gtk_tree_store_append(priv->tree_store, iter, NULL);

	gtk_tree_store_set(priv->tree_store, iter,
			   COL_DEVICE_INDEX, index,
			   COL_DEVICE_MODEL, device->model,
			   COL_DEVICE_MFR, device->manufacturer,
			   COL_DEVICE_ITEM, device,
			   COL_DEVICE_ICON, pixbuf,
			   COL_DEVICE_EJECT, TRUE,
			   COL_DEVICE_NO_EJECT, FALSE,
			   -1);

	return TRUE;
}

void mtpc_devicelist_add_child(MtpcDevicelist *device_list,
			       GtkTreeIter *parent,
			       int storage_id,
			       char *storage_description,
			       char *volume_id,
			       Device *device)
{
	GtkTreeIter child;
	MtpcDevicelistPrivate *priv;
	GdkPixbuf *pixbuf = NULL;
        g_return_val_if_fail(device != NULL, FALSE);
        g_return_val_if_fail(device->device != NULL, FALSE);

	priv = mtpc_devicelist_get_instance_private(device_list);

	gtk_tree_store_append(priv->tree_store, &child, parent);

	gtk_tree_store_set(priv->tree_store, &child,
			   COL_DEVICE_INDEX, storage_id,
			   COL_DEVICE_MODEL, storage_description,
			   COL_DEVICE_MFR, volume_id,
			   COL_DEVICE_ITEM, device,
			   COL_DEVICE_ICON, pixbuf,
			   COL_DEVICE_EJECT, TRUE,
			   COL_DEVICE_NO_EJECT, FALSE,
			   -1);

	return;
}

void mtpc_devicelist_clear(MtpcDevicelist *device_list)
{
	MtpcDevicelistPrivate *priv;
        GList *cur;

	priv = mtpc_devicelist_get_instance_private(device_list);

        for (cur = mtpc_device_get_list(); cur != NULL; cur = cur->next)
                mtpc_device_destroy((Device *)cur->data);

        while (gtk_events_pending())
                gtk_main_iteration();

        gtk_tree_store_clear(priv->tree_store);
}
