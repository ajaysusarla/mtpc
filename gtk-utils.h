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
#ifndef GTK_UTILS_H
#define GTK_UTILS_H

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>


GtkBuilder *_gtk_builder_new_from_file(const char *ui_file);

void _gtk_widget_hide(GtkWidget *widget);
void _gtk_widget_show(GtkWidget *widget);

GdkDragAction
_gtk_menu_ask_drag_drop_action(GtkWidget     *widget,
			       GdkDragAction  actions,
			       guint32        activate_time);
#endif /* GTK_UTILS_H */
