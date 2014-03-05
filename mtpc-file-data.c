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
	GFile *file;
	GFileInfo *info;
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

	G_OBJECT_CLASS(mtpc_file_data_parent_class)->finalize(object);

}

static void mtpc_file_data_class_init(MtpcFileDataClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = mtpc_file_data_finalize;
}

static void mtpc_file_data_init(MtpcFileData *fdata)
{
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
