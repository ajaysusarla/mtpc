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

#ifndef GLIB_UTILS_H
#define GLIB_UTILS_H

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>


#define DEFINE_ACTIONS_CALLBACK(x) void x (GSimpleAction *action, GVariant *parameter, gpointer user_data);

gpointer _g_object_ref(gpointer object);
void _g_object_unref(gpointer object);

void _g_free(gpointer mem);

void _g_string_list_free(GList *string_list);

#endif /* GLIB_UTILS_H */
