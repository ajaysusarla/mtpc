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

#ifndef MTPC_APP_H
#define MTPC_APP_H

#include <gtk/gtk.h>

#define APP_NAME "mtpc"

#define MTPC_TYPE_APP              (mtpc_app_get_type())
#define MTPC_APP(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_APP, MtpcApp))
#define MTPC_APP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_APP, MtpcAppClass))
#define MTPC_IS_APP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_APP))
#define MTPC_IS_APP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_APP))
#define MTPC_APP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_APP, MtpcAppClass))


typedef struct _MtpcApp MtpcApp;
typedef struct _MtpcAppClass MtpcAppClass;
typedef struct _MtpcAppPrivate MtpcAppPrivate;

struct _MtpcApp {
	GtkApplication parent;
	MtpcAppPrivate *priv;
};

struct _MtpcAppClass {
	GtkApplicationClass parent_class;
};

GType mtpc_app_get_type(void);
MtpcApp *mtpc_app_new(void);


#endif /* MTPC_APP_H */
