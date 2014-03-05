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

#ifndef GIO_UTILS_H
#define GIO_UTILS_H

#include <glib.h>
#include <gio/gio.h>

typedef int (*StartDirCallback) (GFile *directory,
				 GFileInfo *info,
				 GError **error,
				 gpointer user_data);

typedef void (*ForEachChildCallback) (GFile *file,
				      GFileInfo *info,
				      gpointer user_data);

typedef void (*DirListReadyCallback) (GList *files,
				      GList *dirs,
				      GError *erro,
				      gpointer user_data);

typedef void (*ReadyFunc)        (GError     *error,
			 	  gpointer    user_data);

typedef void (*ReadyCallback)    (GObject    *object,
				  GError     *error,
			   	  gpointer    user_data);

void g_directory_list_async(GFile *directory,
			    const char *base_dir,
			    gboolean recursive,
			    gboolean follow_links,
			    GCancellable *cancellable,
			    DirListReadyCallback done_func,
			    gpointer done_data);

#endif /* GIO_UTILS_H */
