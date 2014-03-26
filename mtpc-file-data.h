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

#ifndef MTPC_FILE_DATA_H
#define MTPC_FILE_DATA_H

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <libmtp.h>

#define MTPC_TYPE_FILE_DATA              (mtpc_file_data_get_type())
#define MTPC_FILE_DATA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_FILE_DATA, MtpcFileData))
#define MTPC_FILE_DATA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_FILE_DATA, MtpcFileDataClass))
#define MTPC_IS_FILE_DATA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_FILE_DATA))
#define MTPC_IS_FILE_DATA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_FILE_DATA))
#define MTPC_FILE_DATA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_FILE_DATA, MtpcFileDataClass))

typedef enum {
	ENTRY_TYPE_FILE,
	ENTRY_TYPE_FOLDER,
	ENTRY_TYPE_CWD,
	ENTRY_TYPE_PARENT,
	ENTRY_TYPE_INVALID,
	ENTRY_TYPE_NOT_SET
} FileDataType;

typedef struct _MtpcFileData MtpcFileData;
typedef struct _MtpcFileDataClass MtpcFileDataClass;

struct _MtpcFileData {
	GObject parent;
};

struct _MtpcFileDataClass {
	GObjectClass parent_class;
};

GType mtpc_file_data_get_type(void);
MtpcFileData *mtpc_file_data_new(GFile *file, GFileInfo *info);

void mtpc_file_data_set_file(MtpcFileData *fdata, GFile *file);
GFile * mtpc_file_data_get_file(MtpcFileData *fdata);

void mtpc_file_data_set_file_info(MtpcFileData *fdata, GFileInfo *info);
GFileInfo * mtpc_file_data_get_file_info(MtpcFileData *fdata);

void mtpc_file_data_set_folder_id(MtpcFileData *fdata, long folder_id);
long mtpc_file_data_get_folder_id(MtpcFileData *fdata);

void mtpc_file_data_set_parent_folder_id(MtpcFileData *fdata, long parent_id);
long mtpc_file_data_get_parent_folder_id(MtpcFileData *fdata);

void mtpc_file_data_set_dev_info(MtpcFileData *fdata,
				 LIBMTP_mtpdevice_t *dev,
				 int device_index);
LIBMTP_mtpdevice_t * mtpc_file_data_get_dev(MtpcFileData *fdata);
int mtpc_file_data_get_device_index(MtpcFileData *fdata);


void mtpc_file_data_set_file_type(MtpcFileData *fdata, FileDataType type);
FileDataType mtpc_file_data_get_file_type(MtpcFileData *fdata);

gboolean mtpc_file_data_is_directory(MtpcFileData *fdata);

#endif /* MTPC_FILE_DATA_H */
