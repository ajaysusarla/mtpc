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

#ifndef MTPC_DEVICELIST_H
#define MTPC_DEVICELIST_H

#include <gtk/gtk.h>

#include "mtpc-device.h"

#define MTPC_TYPE_DEVICELIST              (mtpc_devicelist_get_type())
#define MTPC_DEVICELIST(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_DEVICELIST, MtpcDevicelist))
#define MTPC_DEVICELIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_DEVICELIST, MtpcDevicelistClass))
#define MTPC_IS_DEVICELIST(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_DEVICELIST))
#define MTPC_IS_DEVICELIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_DEVICELIST))
#define MTPC_DEVICELIST_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_DEVICELIST, MtpcDevicelistClass))

typedef struct _MtpcDevicelist MtpcDevicelist;
typedef struct _MtpcDevicelistClass MtpcDevicelistClass;

struct _MtpcDevicelist {
	GtkTreeView parent;
};

struct _MtpcDevicelistClass {
	GtkTreeViewClass parent_class;

	void (*folder_popup) (MtpcDevicelist *device_list,
			      Device *device);

	void (*list_children) (MtpcDevicelist *device_list,
			       Device *device);

	void (*load) (MtpcDevicelist *device_list,
		      Device *device);

	void (*open) (MtpcDevicelist *device_list,
		      Device *device);
};

GType mtpc_devicelist_get_type(void);
GtkWidget *mtpc_devicelist_new(void);
gboolean mtpc_devicelist_append_item(MtpcDevicelist *device_list,
				     int index,
				     GtkTreeIter *iter,
				     Device *device);
void mtpc_devicelist_add_child(MtpcDevicelist *device_list,
			       GtkTreeIter *parent,
			       int storage_id,
			       int storage_type,
			       char *storage_description,
			       char *volume_id,
			       Device *device);
void mtpc_devicelist_clear(MtpcDevicelist *device_list);
#endif /* MTPC_DEVICELIST_H */
