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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

#include "glib-utils.h"


gpointer _g_object_ref(gpointer object)
{
        if (object != NULL)
                return g_object_ref(object);
        else
                return NULL;
}

void _g_object_unref(gpointer object)
{
        if (object != NULL)
                g_object_unref(object);
}

void _g_free(gpointer mem)
{
	if (mem != NULL) {
		g_free(mem);
		mem = NULL;
	}
}

void _g_string_list_free(GList *string_list)
{
	if (string_list == NULL)
		return;

	g_list_foreach(string_list, (GFunc)g_free, NULL);
	g_list_free(string_list);
}
