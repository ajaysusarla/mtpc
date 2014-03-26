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

#include "glib-utils.h"
#include "mtpc-file-data.h"


typedef struct {
	FileDataType ftype;

	GFile *file;
	GFileInfo *info;

	LIBMTP_mtpdevice_t *dev;
	int dev_index;

	long folder_id;
	long parent_id;
} MtpcFileDataPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MtpcFileData, mtpc_file_data, G_TYPE_OBJECT);


/* internal */


/* class implementation */
static void mtpc_file_data_finalize(GObject *object)
{
	MtpcFileData *self = MTPC_FILE_DATA(object);
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(self);

	_g_object_unref(priv->file);
	_g_object_unref(priv->info);

	priv->dev = NULL;
	priv->dev_index = -1;

	priv->folder_id = -1;
	priv->parent_id = -1;

	G_OBJECT_CLASS(mtpc_file_data_parent_class)->finalize(object);

}

static void mtpc_file_data_class_init(MtpcFileDataClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = mtpc_file_data_finalize;
}

static void mtpc_file_data_init(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	priv->ftype = ENTRY_TYPE_NOT_SET;
}

/* public */
MtpcFileData *mtpc_file_data_new(GFile *file, GFileInfo *info)
{
	MtpcFileData *fdata;

	fdata = g_object_new(MTPC_TYPE_FILE_DATA, NULL);

	mtpc_file_data_set_file(fdata, file);
	mtpc_file_data_set_file_info(fdata, info);

	return fdata;
}

void mtpc_file_data_set_file(MtpcFileData *fdata, GFile *file)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	_g_object_ref(file);

	if (priv->file != NULL) {
		g_object_unref(priv->file);
		priv->file = NULL;
	}

	if (file != NULL)
		priv->file = file;
}

void mtpc_file_data_set_file_info(MtpcFileData *fdata, GFileInfo *info)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	_g_object_ref(info);

	_g_object_unref(priv->info);

	if (info != NULL)
		priv->info = info;
	else
		priv->info = g_file_info_new();
}

void mtpc_file_data_set_folder_id(MtpcFileData *fdata, long folder_id)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	priv->folder_id = folder_id;
}

void mtpc_file_data_set_parent_folder_id(MtpcFileData *fdata, long parent_id)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	priv->parent_id = parent_id;
}

void mtpc_file_data_set_dev_info(MtpcFileData *fdata,
				 LIBMTP_mtpdevice_t *dev,
				 int device_index)
{
	MtpcFileDataPrivate *priv;

	priv = mtpc_file_data_get_instance_private(fdata);

	priv->dev = dev;
	priv->dev_index = device_index;
}

GFileInfo * mtpc_file_data_get_file_info(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL) {
		priv = mtpc_file_data_get_instance_private(fdata);

		if (priv->info != NULL)
			return priv->info;
	}

	return NULL;
}

GFile * mtpc_file_data_get_file(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL) {
		priv = mtpc_file_data_get_instance_private(fdata);

		if (priv->info != NULL)
			return priv->file;
	}

	return NULL;
}

long mtpc_file_data_get_folder_id(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL)
		priv = mtpc_file_data_get_instance_private(fdata);

	return priv->folder_id;
}

long mtpc_file_data_get_parent_folder_id(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL)
		priv = mtpc_file_data_get_instance_private(fdata);
	return priv->parent_id;
}

LIBMTP_mtpdevice_t * mtpc_file_data_get_dev(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL) {
		priv = mtpc_file_data_get_instance_private(fdata);

		return priv->dev;
	}

	return NULL;
}

int mtpc_file_data_get_device_index(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata != NULL) {
		priv = mtpc_file_data_get_instance_private(fdata);

		return priv->dev_index;
	}

	return -1;
}

void mtpc_file_data_set_file_type(MtpcFileData *fdata, FileDataType type)
{
	MtpcFileDataPrivate *priv;

	if (fdata == NULL)
		return;

	priv = mtpc_file_data_get_instance_private(fdata);

	priv->ftype = type;
}

FileDataType mtpc_file_data_get_file_type(MtpcFileData *fdata)
{
	MtpcFileDataPrivate *priv;

	if (fdata == NULL)
		return -1;

	priv = mtpc_file_data_get_instance_private(fdata);

	return priv->ftype;

}

gboolean mtpc_file_data_is_directory(MtpcFileData *fdata)
{
	GFileInfo *info;

	info = mtpc_file_data_get_file_info(fdata);

	if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY)
		return TRUE;
	else
		return FALSE;
}
