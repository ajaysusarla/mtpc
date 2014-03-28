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
#include "mtpc-file-data.h"
#include "mtpc-folder-tree.h"

typedef struct {
	GtkWidget *headerbar;

	GtkWidget *grid;
	GtkWidget *toolbar;
	GtkWidget *main_vbox;

	GtkWidget *right_container;
	GtkWidget *left_container;

	GtkWidget *home_scrolled;
	GtkWidget *home_folder_tree;

	GtkWidget *device_scrolled;
	GtkWidget *device_folder_tree;

	GtkWidget *sidebar;
	GtkWidget *device_properties_box;
	GtkWidget *device_properties_grid;
	GtkWidget *statusbar;

	GtkWidget *devicelist;

	GList *dlist;
	GQueue *dev_folder_queue;

	GCancellable *cancellable;

} MtpcWindowPrivate;


G_DEFINE_TYPE_WITH_PRIVATE(MtpcWindow, mtpc_window, GTK_TYPE_APPLICATION_WINDOW);


/* callbacks and internal methods */
static MtpcFileData *create_file_data_from_libmtp_file(LIBMTP_file_t *file)
{
	GFileInfo *info = NULL;
	MtpcFileData *mfile = NULL;
	GIcon *icon = NULL;
	char *content_type = NULL;
	FileDataType ftype = ENTRY_TYPE_NOT_SET;

	info = g_file_info_new();

	g_file_info_set_name(info, file->filename);
	g_file_info_set_display_name(info, file->filename);

	switch (file->filetype) {
	case LIBMTP_FILETYPE_FOLDER:
		g_file_info_set_file_type(info, G_FILE_TYPE_DIRECTORY);
		g_file_info_set_content_type(info, "inode/directory");
		icon = g_themed_icon_new("folder");
		g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, TRUE);
		ftype = ENTRY_TYPE_FOLDER;
		break;
	default:
		g_file_info_set_file_type(info, G_FILE_TYPE_REGULAR);
		g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, FALSE);
		content_type = g_content_type_guess(file->filename, NULL, 0, NULL);
		g_file_info_set_content_type(info, content_type);
		icon = g_content_type_get_icon(content_type);
		ftype = ENTRY_TYPE_FILE;
	}

	g_free(content_type);

	g_file_info_set_size(info, file->filesize);
	GTimeVal modtime = {file->modificationdate, 0};
	g_file_info_set_modification_time(info, &modtime);
	g_file_info_set_attribute_boolean(info,
					  G_FILE_ATTRIBUTE_ACCESS_CAN_READ,
					  TRUE);
	g_file_info_set_attribute_boolean(info,
					  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
					  TRUE);
	g_file_info_set_attribute_boolean(info,
					  G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE,
					  TRUE);
	g_file_info_set_attribute_boolean(info,
					  G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH,
					  FALSE);
	g_file_info_set_attribute_boolean(info,
					  G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME,
					  TRUE);

	if (icon != NULL) {
		g_file_info_set_icon(info, icon);
		g_object_unref(icon);;
	}

	mfile = mtpc_file_data_new(NULL, info);
	mtpc_file_data_set_folder_id(mfile, file->item_id);
	mtpc_file_data_set_parent_folder_id(mfile, file->parent_id);
	mtpc_file_data_set_file_type(mfile, ftype);

	return mfile;
}

static MtpcFileData *create_parent_file_data_from_libmtp_file(MtpcFileData *fdata)
{
	MtpcFileData *parent_fdata;

	parent_fdata = mtpc_file_data_new(NULL, NULL);
	mtpc_file_data_set_dev_info(parent_fdata,
				    mtpc_file_data_get_dev(fdata),
				    mtpc_file_data_get_device_index(fdata));
	mtpc_file_data_set_file_type(parent_fdata, ENTRY_TYPE_PARENT);

	return parent_fdata;
}

void _mtpc_window_device_add(MtpcWindow *window, Device *device)
{
	MtpcWindowPrivate *priv;

	g_return_if_fail(device != NULL);

	priv = mtpc_window_get_instance_private(window);

	priv->dlist = g_list_prepend(priv->dlist, device);
}

void _mtpc_window_devicelist_free(MtpcWindow *window)
{
	MtpcWindowPrivate *priv;
	priv = mtpc_window_get_instance_private(window);

	g_list_foreach(priv->dlist, (GFunc)mtpc_device_destroy, NULL);
	g_list_free(priv->dlist);
	priv->dlist = NULL;
}

static gboolean update_statusbar(gpointer data)
{
	MtpcStatusbar *statusbar = MTPC_STATUSBAR(data);

	MTPC_STATUSBAR_UPDATE(statusbar, "", "Fetching device information", "");

	return TRUE;
}

static void device_folder_tree_open_cb(MtpcFolderTree *folder_tree,
				       MtpcFileData *fdata,
				       gpointer user_data)
{
	LIBMTP_file_t *files = NULL;
	LIBMTP_mtpdevice_t *dev;
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv;
	int dev_index;
	long parent_id = -1;
	long folder_id = -1;
	GList *flist = NULL;
	MtpcFileData *parent_fdata = NULL;
	FileDataType ftype;

	ftype = mtpc_file_data_get_file_type(fdata);

	/* we aren't a opening a file, return */
	if (!mtpc_file_data_is_directory(fdata) &&
	    (ftype != ENTRY_TYPE_PARENT)) {
		return;
	}

	priv = mtpc_window_get_instance_private(window);

	MTPC_STATUSBAR_RESET(MTPC_STATUSBAR(priv->statusbar));

	dev = mtpc_file_data_get_dev(fdata);

	dev_index = mtpc_file_data_get_device_index(fdata);
	parent_id = mtpc_file_data_get_parent_folder_id(fdata);

	/* check if we were requested for parent folder */
	if (ftype == ENTRY_TYPE_PARENT) {
		/* load the parent folder */
		parent_fdata = g_queue_pop_head(priv->dev_folder_queue);
		folder_id = mtpc_file_data_get_folder_id(parent_fdata);
		if (folder_id == 0)
			folder_id = -1;
	} else {
		folder_id = mtpc_file_data_get_folder_id(fdata);
		/* Create the parent directory link
		   only if we aren't the top level folder.
		 */
		parent_id = mtpc_file_data_get_parent_folder_id(fdata);

		parent_fdata = create_parent_file_data_from_libmtp_file(fdata);
		mtpc_file_data_set_folder_id(parent_fdata, parent_id);
		/* pushing the file data to queue */
		g_queue_push_head(priv->dev_folder_queue, parent_fdata);

	}

	/* if parent_fdata is null, we are essentially at the top level
	   folder.
	 */
	if ((ftype == ENTRY_TYPE_PARENT) && (parent_fdata == NULL)) {
		folder_id = -1;
		printf("===Setting folder_id to -1===\n");
	}

	/*
	  setting folder_id & parent_id to -1 will load the top level folder
	  in the device.
	 */
	if (folder_id == -1) {
		parent_id = -1;
		mtpc_file_data_set_has_parent(parent_fdata, FALSE);
		mtpc_file_data_set_folder_id(parent_fdata, parent_id);
	}

	MTPC_STATUSBAR_UPDATE(MTPC_STATUSBAR(priv->statusbar),
			      "Loading Folder",
			      mtpc_file_data_get_file_name(fdata),
			      "");

	files = LIBMTP_Get_Files_And_Folders(dev, dev_index, folder_id);

	while (files != NULL) {
		LIBMTP_file_t *f = files;
		MtpcFileData *mfile;

		mfile = create_file_data_from_libmtp_file(f);

		mtpc_file_data_set_dev_info(mfile, dev, dev_index);

		flist = g_list_prepend(flist, mfile);

		files = files->next;

		LIBMTP_destroy_file_t(f);
	}

	mtpc_folder_tree_set_list(MTPC_FOLDER_TREE(priv->device_folder_tree),
				  parent_fdata,
				  flist);

	MTPC_STATUSBAR_UPDATE(MTPC_STATUSBAR(priv->statusbar),
			      "Loading Folder",
			      mtpc_file_data_get_file_name(fdata),
			      "..Done.");
}


static void home_folder_tree_popup_cb(MtpcFolderTree *folder_tree,
				      GFile *gfile,
				      gpointer user_data)
{
	printf("home_folder_tree_popup_cb\n");
}

static void home_folder_tree_open_cb(MtpcFolderTree *folder_tree,
				     MtpcFileData *fdata,
				     gpointer user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	GFile *parent;
	GFile *gfile;
	GList *flist = NULL;
	GFileEnumerator *enumerator;
	GError *error;
	GFileInfo *info, *parent_info;
	MtpcFileData *parent_fdata;
	const char *path;

	gfile = mtpc_file_data_get_file(fdata);
	parent = g_file_get_parent(gfile);

	parent_info = g_file_query_info(parent,
					"standard::*",
					G_FILE_QUERY_INFO_NONE,
					NULL,
					&error);

	parent_fdata = mtpc_file_data_new(parent, parent_info);

	if (parent == NULL) {
		printf("***HOME: DOESN'T HAVE PARENT\n");
		mtpc_file_data_set_has_parent(parent_fdata, FALSE);
	}

	path = g_file_get_path(gfile);

	enumerator = g_file_enumerate_children(gfile,
					       "standard::*",
					       G_FILE_QUERY_INFO_NONE,
					       NULL,
					       &error);

	if (enumerator == NULL) {
		_g_object_unref(gfile);
		return;
	}

	do {
		GFile *gfile;
		GFileInfo *t_info;
		MtpcFileData *data;

		error = NULL;
		info = g_file_enumerator_next_file(enumerator,
						   NULL,
						   &error);

		if (info == NULL)
			break;

		gfile = g_file_enumerator_get_child(enumerator, info);
		/* XXX: Is this necessary?? We should be able to use
		   the info above. Please test and FIXME.*/
		t_info = g_file_query_info(gfile,
					   "standard::*",
					   G_FILE_QUERY_INFO_NONE,
					   NULL,
					   &error);

		data = mtpc_file_data_new(gfile, t_info);

		flist = g_list_prepend(flist, data);

		_g_object_unref(info);
	} while (1);

	flist = g_list_reverse(flist);

	mtpc_window_set_title(window, path);

	mtpc_folder_tree_set_list(folder_tree,
				  parent_fdata,
				  flist);
}

static void home_folder_tree_load_cb(MtpcFolderTree *folder_tree,
				     GFile *gfile,
				     gpointer user_data)
{
	printf("home_folder_tree_load_cb\n");
}

static void devicelist_device_popup_cb(MtpcDevicelist *devicelist,
				       Device *device,
				       gpointer user_data)
{
	printf("devicelist_device_popup_cb\n");
}

static void devicelist_device_open_cb(MtpcDevicelist *devicelist,
				      Device *device,
				      gpointer user_data)
{
	printf("devicelist_device_open_cb\n");
}

static void devicelist_device_load_cb(MtpcDevicelist *devicelist,
				      int index,
				      Device *device,
				      gpointer user_data)
{
	LIBMTP_file_t *files = NULL;
	LIBMTP_mtpdevice_t *dev = device->device;
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv;
	long parent_id = -1;
	GList *flist = NULL;

	priv = mtpc_window_get_instance_private(window);

	printf("devicelist_device_load_cb\n");
	MTPC_STATUSBAR_UPDATE(MTPC_STATUSBAR(priv->statusbar),
			      "Opening MTP device",
			      device->model,
			      "");

	files = LIBMTP_Get_Files_And_Folders(dev, index, parent_id);

	while (files != NULL) {
		LIBMTP_file_t *f = files;
		MtpcFileData *mfile;

		mfile = create_file_data_from_libmtp_file(f);
		mtpc_file_data_set_dev_info(mfile, dev, index);

		flist = g_list_prepend(flist, mfile);

		files = files->next;

		LIBMTP_destroy_file_t(f);
	}

	mtpc_folder_tree_set_list(MTPC_FOLDER_TREE(priv->device_folder_tree),
				  NULL,
				  flist);

	MTPC_STATUSBAR_UPDATE(MTPC_STATUSBAR(priv->statusbar),
			      "Opening MTP device",
			      device->model,
			      "..Done.");

}

static void folder_tree_drag_data_received_cb(GtkWidget        *tree_view,
					      GdkDragContext   *context,
					      int               x,
					      int               y,
					      GtkSelectionData *selection_data,
					      guint             info,
					      guint             time,
					      gpointer          user_data)
{
	printf("DATA RECEIVED\n");
}

static void folder_tree_drag_data_get_cb(GtkWidget        *widget,
					 GdkDragContext   *drag_context,
					 GtkSelectionData *selection_data,
					 guint             info,
					 guint             time,
					 gpointer          user_data)
{
	printf("DATA GET\n");
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
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(data->window);

	g_return_val_if_fail(g_task_is_valid(result, self), NULL);
	g_source_remove (data->func_ref);

	MTPC_STATUSBAR_UPDATE(MTPC_STATUSBAR(priv->statusbar), "", "Ready", "");
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
		GtkTreeIter iter;
		LIBMTP_devicestorage_t *storage;

		device = mtpc_device_new_from_raw_device(&devices->rawdevices[i]);
		mtpc_devicelist_append_item(MTPC_DEVICELIST(priv->devicelist),
					    i, &iter,
					    device);

		_mtpc_window_device_add(window, device);

		LIBMTP_Get_Storage(device->device,
				   LIBMTP_STORAGE_SORTBY_NOTSORTED);
		for (storage = device->device->storage;
		     storage != 0;
		     storage = storage->next) {
			mtpc_devicelist_add_child(MTPC_DEVICELIST(priv->devicelist),
						  &iter,
						  storage->id,
						  storage->StorageType,
						  storage->StorageDescription,
						  storage->VolumeIdentifier,
						  device);
		}
	}
}

static void fetch_devices_async(MtpcWindow *window)
{
	libmtp_err_t err;
	DeviceAsyncFetchData *data;
	GTask *fetch_devices_task;
	MtpcWindowPrivate *priv;
	MtpcStatusbar *statusbar;
	GList *t;

	data = g_slice_new0(DeviceAsyncFetchData);

	priv = mtpc_window_get_instance_private(window);
	statusbar = MTPC_STATUSBAR(priv->statusbar);

	mtpc_devicelist_clear(MTPC_DEVICELIST(priv->devicelist));
	 _mtpc_window_devicelist_free(window);

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

static void change_toolbar_view_state(GSimpleAction *action,
				      GVariant      *state,
				      gpointer       user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(window);

	if (g_variant_get_boolean(state)) {
		gtk_widget_show(priv->toolbar);
	} else {
		gtk_widget_hide(priv->toolbar);
	}

	g_simple_action_set_state(action, state);
}

static void _mtpc_window_set_home_folder_visibility(GtkWidget *home_folder,
						    gboolean visible)
{
	if (visible) {
		gtk_widget_show(home_folder);
	} else {
		gtk_widget_hide(home_folder);
	}
}

static void change_home_folder_view_state(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(window);

	if (g_variant_get_boolean(state)) {
		_mtpc_window_set_home_folder_visibility(priv->home_scrolled,
							TRUE);
	} else {
		_mtpc_window_set_home_folder_visibility(priv->home_scrolled,
							FALSE);
	}

	g_simple_action_set_state(action, state);
}

static void _mtpc_window_set_device_properties_visibility(GtkWidget *properties,
							  gboolean visible)
{
	if (visible) {
		gtk_widget_show(properties);
	} else {
		gtk_widget_hide(properties);
	}
}

static void change_device_properties_view(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(window);

	if (g_variant_get_boolean(state)) {
		printf("show device properties\n");
		_mtpc_window_set_device_properties_visibility(priv->device_properties_box,
							      TRUE);
	} else {
		_mtpc_window_set_device_properties_visibility(priv->device_properties_box,
							      FALSE);

		printf("hide device properties\n");
	}

	g_simple_action_set_state(action, state);
}

static void _mtpc_window_set_statusbar_visibility(GtkWidget *statusbar,
						 gboolean visible)
{
	if (visible) {
		gtk_widget_show(statusbar);
	} else {
		gtk_widget_hide(statusbar);
	}
}

static void change_statusbar_view_state(GSimpleAction *action,
					GVariant      *state,
					gpointer       user_data)
{
	MtpcWindow *window = MTPC_WINDOW(user_data);
	MtpcWindowPrivate *priv = mtpc_window_get_instance_private(window);

	if (g_variant_get_boolean(state)) {
		printf("show statusbar\n");
		_mtpc_window_set_statusbar_visibility(priv->statusbar, TRUE);
	} else {
		printf("hide statusbar\n");
		_mtpc_window_set_statusbar_visibility(priv->statusbar, FALSE);
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

static void _mtpc_window_setup_home_folder(MtpcWindow *window, GtkWidget *widget)
{
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(widget);
	GFileEnumerator *enumerator;
	GError *error;
	GFile *file, *parent;
	GFileInfo *parent_info;
	const char *home;
	GList *flist = NULL;
	MtpcFileData *parent_fdata;

	/* FIXME, check getpwuid() if this fails */
	home = g_getenv("HOME");
	if (home == NULL) {
		return;
	}

	file = g_file_new_for_path(home);
	parent = g_file_get_parent(file);

	parent_info = g_file_query_info(parent,
					"standard::*",
					G_FILE_QUERY_INFO_NONE,
					NULL,
					&error);

	parent_fdata = mtpc_file_data_new(parent, parent_info);

	enumerator = g_file_enumerate_children(file,
					       "standard::*",
					       G_FILE_QUERY_INFO_NONE,
					       NULL,
					       &error);

	if (enumerator == NULL) {
		_g_object_unref(file);
		return;
	}

	do {
		GFile *gfile;
		GFileInfo *info, *t_info;
		MtpcFileData *fdata;

		info = g_file_enumerator_next_file(enumerator,
						   NULL,
						   &error);

		if (info == NULL)
			break;

		gfile = g_file_enumerator_get_child(enumerator, info);
		/* XXX: Is this necessary?? We should be able to use
		   the info above. Please test and FIXME.*/
		t_info = g_file_query_info(gfile,
					   "standard::*",
					   G_FILE_QUERY_INFO_NONE,
					   NULL,
					   &error);

		fdata = mtpc_file_data_new(gfile, t_info);

		flist = g_list_prepend(flist, fdata);
		_g_object_unref(info);
	} while (1);

	flist = g_list_reverse(flist);

	mtpc_window_set_title(window, home);

	mtpc_folder_tree_set_list(folder_tree, parent_fdata, flist);
}

static GtkWidget *_mtpc_window_create_toolbar()
{
	GtkWidget *toolbar;
	GtkToolItem *button;

	toolbar = gtk_toolbar_new();
	button = gtk_tool_button_new(NULL, NULL);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button), "gtk-refresh");
	gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(button), "win.refresh-device-list");
	gtk_container_add(GTK_CONTAINER(toolbar), GTK_WIDGET(button));

	button = gtk_tool_button_new(NULL, NULL);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button), "media-eject");
	gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(button),
						"win.disconnect");
	gtk_container_add(GTK_CONTAINER(toolbar), GTK_WIDGET(button));

	button = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button),
				      "gtk-properties");
	gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(button),
						"win.toggle-device-properties");
	gtk_container_add(GTK_CONTAINER(toolbar), GTK_WIDGET(button));

	gtk_widget_show_all(toolbar);

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
        { "toggle-toolbar", toggle_action_activated, NULL, "true", change_toolbar_view_state },
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

static void mtpc_window_finalize(GObject *object)
{
	MtpcWindow *window = MTPC_WINDOW(object);
	MtpcWindowPrivate *priv;

	priv = mtpc_window_get_instance_private(window);

	g_queue_free(priv->dev_folder_queue);
	g_list_free(priv->dlist);

	G_OBJECT_CLASS(mtpc_window_parent_class)->finalize(object);
}

static void mtpc_window_class_init(MtpcWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = mtpc_window_dispose;
	obj_class->finalize = mtpc_window_finalize;
}

static void mtpc_window_init(MtpcWindow *win)
{
	GdkScreen *screen;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *devlist_scrolled;

	MtpcWindowPrivate *priv;

	priv = mtpc_window_get_instance_private(win);

	/* init data */
	priv->dlist = NULL;
	priv->dev_folder_queue = g_queue_new();

	window = GTK_WIDGET(win);

	/* window size */
	screen = gdk_screen_get_default();
	_mtpc_window_set_default_size(GTK_WIDGET(win), screen);


	/* actions */
	g_action_map_add_action_entries(G_ACTION_MAP(window),
					win_entries, G_N_ELEMENTS(win_entries),
					window);

	mtpc_window_set_title(win, "MTPc");

	/* foundation */
	priv->grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), priv->grid);

	/* toolbar */
	priv->toolbar = _mtpc_window_create_toolbar();
	gtk_grid_attach(GTK_GRID (priv->grid), priv->toolbar, 0, 0, 1, 1);
	gtk_widget_show(GTK_WIDGET(priv->toolbar));


	/** main view **/
	priv->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_hexpand (priv->main_vbox, TRUE);
	gtk_widget_set_vexpand (priv->main_vbox, TRUE);
	gtk_widget_show(GTK_WIDGET(priv->main_vbox));

        priv->right_container = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(priv->main_vbox),
			   priv->right_container, TRUE, TRUE, 3);
	gtk_widget_show(GTK_WIDGET(priv->right_container));

        priv->left_container = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_paned_pack1(GTK_PANED(priv->right_container),
			priv->left_container, TRUE, TRUE);
	gtk_widget_show(GTK_WIDGET(priv->left_container));

	/* sidebar */
        priv->sidebar = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
        gtk_paned_pack1(GTK_PANED(priv->left_container),
			priv->sidebar, FALSE, FALSE);

        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(vbox),
                                    GTK_STYLE_CLASS_SIDEBAR);
        gtk_widget_set_size_request (vbox, -1, 50);
        gtk_paned_pack1(GTK_PANED(priv->sidebar), vbox, TRUE, FALSE);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_widget_show(GTK_WIDGET(priv->sidebar));

	/* device folder tree */
	priv->device_scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->device_scrolled),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->device_scrolled),
                                            GTK_SHADOW_IN);
	gtk_widget_show(GTK_WIDGET(priv->device_scrolled));

	priv->device_folder_tree = mtpc_folder_tree_new();
	gtk_container_add(GTK_CONTAINER(priv->device_scrolled),
			  priv->device_folder_tree);

	g_signal_connect(priv->device_folder_tree,
			 "open",
			 G_CALLBACK(device_folder_tree_open_cb),
			 win);

        gtk_paned_pack2(GTK_PANED(priv->left_container),
			priv->device_scrolled, FALSE, FALSE);
	gtk_widget_show(GTK_WIDGET(priv->device_folder_tree));

	/* statusbar */
	priv->statusbar = mtpc_statusbar_new();
	gtk_widget_show_all(priv->statusbar);
	gtk_widget_set_vexpand(priv->statusbar, FALSE);

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

	g_signal_connect(priv->devicelist,
			 "folder_popup",
			 G_CALLBACK(devicelist_device_popup_cb),
			 win);
	g_signal_connect(priv->devicelist,
			 "open",
			 G_CALLBACK(devicelist_device_open_cb),
			 win);
	g_signal_connect(priv->devicelist,
			 "load",
			 G_CALLBACK(devicelist_device_load_cb),
			 win);

	gtk_widget_show(GTK_WIDGET(priv->devicelist));

	/* device property box */
	priv->device_properties_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
        gtk_paned_pack2(GTK_PANED(priv->sidebar),
			priv->device_properties_box, TRUE, FALSE);
	_mtpc_window_set_device_properties_visibility(priv->device_properties_box,
						      FALSE);


	/* home folder tree */
	priv->home_scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->home_scrolled),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->home_scrolled),
                                            GTK_SHADOW_IN);

	priv->home_folder_tree = mtpc_folder_tree_new();
	gtk_container_add(GTK_CONTAINER(priv->home_scrolled),
			  priv->home_folder_tree);

	gtk_widget_show(GTK_WIDGET(priv->home_folder_tree));

        gtk_paned_pack2(GTK_PANED(priv->right_container),
			priv->home_scrolled, TRUE, TRUE);

	{
		GtkTargetList *target_list;
		GtkTargetEntry *targets;
		int n_targets;

		target_list = gtk_target_list_new(NULL, 0);
		gtk_target_list_add_uri_targets(target_list, 0);
		gtk_target_list_add_text_targets(target_list, 0);
		targets = gtk_target_table_new_from_list(target_list,
							 &n_targets);
		gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(priv->home_folder_tree),
						     targets,
						     n_targets,
						     GDK_ACTION_MOVE |
						     GDK_ACTION_COPY |
						     GDK_ACTION_ASK);
		mtpc_folder_tree_enable_drag_source(MTPC_FOLDER_TREE(priv->home_folder_tree),
							 GDK_BUTTON1_MASK,
							 targets,
							 n_targets,
							 GDK_ACTION_MOVE |
							 GDK_ACTION_COPY);

		g_signal_connect(priv->home_folder_tree,
				 "drag-data-received",
				 G_CALLBACK(folder_tree_drag_data_received_cb),
				 win);
		g_signal_connect(priv->home_folder_tree,
				 "drag-data-get",
				 G_CALLBACK(folder_tree_drag_data_get_cb),
				 win);

		gtk_target_list_unref(target_list);
		gtk_target_table_free(targets, n_targets);
	}

	g_signal_connect(priv->home_folder_tree,
			 "folder_popup",
			 G_CALLBACK(home_folder_tree_popup_cb),
			 win);
	g_signal_connect(priv->home_folder_tree,
			 "open",
			 G_CALLBACK(home_folder_tree_open_cb),
			 win);
	g_signal_connect(priv->home_folder_tree,
			 "load",
			 G_CALLBACK(home_folder_tree_load_cb),
			 win);

	_mtpc_window_setup_home_folder(win, priv->home_folder_tree);
	_mtpc_window_set_home_folder_visibility(priv->home_scrolled,
						FALSE);

	/* add the main_vbox to the container */
	gtk_grid_attach(GTK_GRID(priv->grid), priv->main_vbox, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(priv->grid), priv->statusbar, 0, 2, 1, 1);
	gtk_widget_show(GTK_WIDGET(priv->grid));
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

void mtpc_window_set_title(MtpcWindow *window, const char *title)
{
	g_return_if_fail(window != NULL);
	gchar *str;

	if (title && strlen(title))
		str = g_strconcat(title, " - ", APP_NAME, NULL);
	else
		str = g_strdup(title);


	gtk_window_set_title(GTK_WINDOW(window), str);

	g_free(str);

	return;
}
