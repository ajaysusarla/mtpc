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
#include "gtk-utils.h"
#include "glib-utils.h"


GtkBuilder * _gtk_builder_new_from_file(const char *ui_file)
{
        char *filename;
        GtkBuilder *builder;
        GError *error = NULL;

        filename = g_build_filename(MTPC_UI_DIR, ui_file, NULL);

        builder = gtk_builder_new();
        if (!gtk_builder_add_from_file(builder, filename, &error)) {
                g_warning("%s\n", error->message);
                g_clear_error(&error);
        }

        g_free(filename);

        return builder;

}

void _gtk_widget_hide(GtkWidget *widget)
{
        if (widget != NULL)
                gtk_widget_hide(widget);
}


void _gtk_widget_show(GtkWidget *widget)
{
        if (widget != NULL)
                gtk_widget_show(widget);
}
