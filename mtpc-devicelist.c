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
	LOAD,
	OPEN,
	RENAME,
	LAST_SIGNAL
};

typedef struct {
	GtkTreeStore *tree_store;
	GtkCellRenderer *text_renderer;
	GtkTreePath *hover_path;
} MtpcDevicelistPrivate;


static guint mtpc_devicelist_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(MtpcDevicelist,
			   mtpc_devicelist,
			   GTK_TYPE_TREE_VIEW);




/* internal private functions */

/* class implementation */
static void mtpc_devicelist_finalize(GObject *object)
{
}

static void mtpc_devicelist_class_init(MtpcDevicelistClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = mtpc_devicelist_finalize;

	/*
	mtpc_devicelist_signals[FOLDER_POPUP] =
		g_signal_new("folder_popup",
			     G_TYPE_FROM_CLASS(class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, folder_popup),
			     NULL, NULL,
			     mtpc_marshal_VOID__OBJECT_UINT,
			     G_TYPE_NONE,
			     2,
			     G_TYPE_OBJECT,
			     G_TYPE_UINT);

	mtpc_devicelist_signals[LOAD] =
		g_signal_new("load",
			     G_TYPE_FROM_CLASS(class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, load),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_OBJECT);

	mtpc_devicelist_signals[OPEN] =
		g_signal_new("open",
			     G_TYPE_FROM_CLASS(class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, open),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__OBJECT,
			     G_TYPE_NONE,
			     1,
			     G_TYPE_OBJECT);

	mtpc_devicelist_signals[RENAME] =
		g_signal_new("rename",
			     G_TYPE_FROM_CLASS(class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET(MtpcDevicelistClass, rename),
			     NULL, NULL,
			     mtpc_marshal_VOID__OBJECT_STRING,
			     G_TYPE_NONE,
			     2,
			     G_TYPE_OBJECT,
			     G_TYPE_STRING);
	*/
}

static void mtpc_devicelist_init(MtpcDevicelist *device_list)
{
}

/* public functions */
GtkWidget *mtpc_devicelist_new(void)
{
	MtpcDevicelist *device_list;

	device_list = g_object_new(MTPC_TYPE_DEVICELIST, NULL);

	return GTK_WIDGET(device_list);
}
