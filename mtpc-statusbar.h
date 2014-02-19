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
#ifndef MTPC_STATUSBAR_H
#define MTPC_STATUSBAR_H

#include <gtk/gtk.h>

typedef enum {
        MTPC_STATUSBAR_SECTION_FILE_LIST,
        MTPC_STATUSBAR_SECTION_FILE
} MtpcStatusbarSection;

#define MTPC_TYPE_STATUSBAR            (mtpc_statusbar_get_type ())
#define MTPC_STATUSBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MTPC_TYPE_STATUSBAR, MtpcStatusbar))
#define MTPC_STATUSBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MTPC_TYPE_STATUSBAR, MtpcStatusbarClass))
#define MTPC_IS_STATUSBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MTPC_TYPE_STATUSBAR))
#define MTPC_IS_STATUSBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MTPC_TYPE_STATUSBAR))
#define MTPC_STATUSBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MTPC_TYPE_STATUSBAR, MtpcStatusbarClass))

typedef struct _MtpcStatusbar MtpcStatusbar;
typedef struct _MtpcStatusbarClass MtpcStatusbarClass;

struct _MtpcStatusbar {
	GtkBox parent;
};

struct _MtpcStatusbarClass {
	GtkBoxClass parent_class;
};


GType mtpc_statusbar_get_type(void);
GtkWidget *mtpc_statusbar_new(void);

void mtpc_statusbar_set_list_info(MtpcStatusbar *statusbar,
				  const char *text);

void mtpc_statusbar_set_primary_text(MtpcStatusbar *statusbar,
				     const char *text);

void mtpc_statusbar_set_secondary_text(MtpcStatusbar *statusbar,
				       const char *text);

void mtpc_statusbar_show_section(MtpcStatusbar *statusbar,
				 MtpcStatusbarSection section);

GtkWidget *mtpc_statusbar_get_action_area(MtpcStatusbar *statusbar);


#define MTPC_STATUSBAR_RESET(statusbar) do {				\
		mtpc_statusbar_set_list_info(statusbar, "");		\
		mtpc_statusbar_set_primary_text(statusbar, "");		\
		mtpc_statusbar_set_secondary_text(statusbar, "");	\
	} while (0)

#define MTPC_STATUSBAR_UPDATE(statusbar, x, y, z) do {			\
		mtpc_statusbar_set_list_info(statusbar, x);		\
		mtpc_statusbar_set_primary_text(statusbar, y);		\
		mtpc_statusbar_set_secondary_text(statusbar, z);	\
	} while (0)


#endif /* MTPC_STATUSBAR_H */
