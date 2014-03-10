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
#ifndef MTPC_WINDOW_H
#define MTPC_WINDOW_H

#include <gtk/gtk.h>
#include "gtk-utils.h"
#include "mtpc-app.h"

#define MTPC_TYPE_WINDOW              (mtpc_window_get_type())
#define MTPC_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_WINDOW, MtpcWindow))
#define MTPC_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_WINDOW, MtpcWindowClass))
#define MTPC_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_WINDOW))
#define MTPC_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_WINDOW))
#define MTPC_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_WINDOW, MtpcWindowClass))

typedef struct _MtpcWindow MtpcWindow;
typedef struct _MtpcWindowClass MtpcWindowClass;

struct _MtpcWindow {
        GtkApplicationWindow parent;
};

struct _MtpcWindowClass {
        GtkApplicationWindowClass parent_class;

        void (*close) (MtpcWindow *window);
};

GType mtpc_window_get_type(void);
GtkWidget *mtpc_window_new(MtpcApp *application);
void mtpc_window_set_title(MtpcWindow *window, const char *title);

#endif /* MTPC_WINDOW_H */
