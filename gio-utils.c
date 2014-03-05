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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "glib-utils.h"
#include "gio-utils.h"


/** g_directory_foreach_child **/
typedef struct {
	GFile *file;
	GFileInfo *info;
} DirData;

static DirData * dir_data_new(GFile *file, GFileInfo *info)
{
	DirData *data;

	data = g_new0(DirData, 1);
	data->file = g_file_dup(file);
	data->info = g_file_info_dup(info);

	return data;
}

static void dir_data_free(DirData *data)
{
	if (data == NULL)
		return;

	_g_object_unref(data->file);
	_g_object_unref(data->info);
	g_free(data);
}

static void dir_data_clear(DirData **data)
{
	if (*data == NULL)
		return;

	dir_data_free(*data);

	*data = NULL;
}

typedef struct {
	GFile *base_directory;
	gboolean recursive;
	gboolean follow_links;
	StartDirCallback start_dir_func;
	ForEachChildCallback for_each_file_func;
	ReadyFunc done_func;
	gpointer user_data;

	DirData *current;
	GHashTable *already_visited;
	GList *to_visit;
	char *attributes;
	GCancellable *cancellable;
	GFileEnumerator *enumerator;
	GError *error;
	guint source_id;
	GList *children;
	GList *current_child;
} ForEachChildData;


void g_directory_foreach_child(GFile *directory,
			       gboolean recursive,
			       gboolean follow_links,
			       const char *attributes,
			       GCancellable *cancellable,
			       StartDirCallback start_dir_func,
			       ForEachChildCallback for_each_file_func,
			       gpointer user_data)
{
	ForEachChildData *data;
}

/** g_directory_list_async **/
typedef struct {
	GList *files;
	GList *dirs;
	GFile *directory;
	char *base_dir;
	GCancellable *cancellable;
	DirListReadyCallback done_func;
	gpointer done_data;
	GList *current_dir;
} GetFileListData;


static void get_file_list_data_free(GetFileListData *fdata)
{
	if (fdata == NULL)
		return;

	_g_string_list_free(fdata->files);
	_g_string_list_free(fdata->dirs);
	g_free(fdata->base_dir);

	g_free(fdata);
}

static void get_file_list_done(GError *error, gpointer user_data)
{
}

static void get_file_list_for_each_file(GFile *file,
					GFileInfo *info,
					gpointer user_data)
{
}

static int get_file_list_start_dir(GFile *directory,
				   GFileInfo *info,
				   GError **error,
				   gpointer user_data)
{
	return 0;
}

void g_directory_list_async(GFile *directory,
			    const char *base_dir,
			    gboolean recursive,
			    gboolean follow_links,
			    GCancellable *cancellable,
			    DirListReadyCallback done_func,
			    gpointer done_data)
{
	GetFileListData *fdata;
	const char *attr = "standard::is-hidden,"		\
		"standard::name,"				\
		"standard::display-name,standard::type,";

	fdata = g_new0(GetFileListData, 1);
	fdata->directory = g_file_dup(directory);
	fdata->base_dir = g_strdup(base_dir);
	fdata->done_func = done_func;
	fdata->done_data = done_data;

	printf("%s\n", attr);

	g_directory_foreach_child(directory,
				  recursive,
				  follow_links,
				  attr,
				  cancellable,
				  get_file_list_start_dir,
				  get_file_list_for_each_file,
				  get_file_list_done);
}
