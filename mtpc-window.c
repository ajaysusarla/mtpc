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

#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "gtk-utils.h"
#include "glib-utils.h"
#include "mtpc-window.h"

typedef struct _MtpWindowPrivate {
	int remove;
}MtpcWindowPrivate;


G_DEFINE_TYPE_WITH_PRIVATE(MtpcWindow, mtpc_window, GTK_TYPE_APPLICATION_WINDOW);

/* callbacks and internal methods */


/* class implementation */
static void mtpc_window_dispose(GObject *object)
{
	MtpcWindow *window = MTPC_WINDOW(object);
	MtpcWindowPrivate *priv;

	priv = mtpc_window_get_instance_private(window);


	G_OBJECT_CLASS(mtpc_window_parent_class)->dispose(object);
}

static void mtpc_window_class_init(MtpcWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = mtpc_window_dispose;
}

static void mtpc_window_init(MtpcWindow *window)
{
}

/* public methods */
GtkWidget *mtpc_window_new(MtpcApp *application)
{
	MtpcWindow *window;
	MtpcWindowPrivate *priv;

	window = g_object_new(MTPC_TYPE_WINDOW,
			      "application", application,
			      NULL);

	priv = mtpc_window_get_instance_private(window);


	return GTK_WIDGET(window);
}
