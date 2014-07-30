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
#include "mtpc-statusbar.h"

typedef struct {
	GtkWidget *list_info;
	GtkWidget *primary_text;
	GtkWidget *secondary_text;
	GtkWidget *action_area;
} MtpcStatusbarPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MtpcStatusbar, mtpc_statusbar, GTK_TYPE_BOX);




/* class implementation */
static void mtpc_statusbar_class_init(MtpcStatusbarClass *klass)
{
}

static void mtpc_statusbar_init(MtpcStatusbar *statusbar)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	gtk_box_set_spacing(GTK_BOX(statusbar), 6);
	gtk_widget_set_margin_top(GTK_WIDGET(statusbar), 3);
	gtk_widget_set_margin_end(GTK_WIDGET(statusbar), 6);
	gtk_widget_set_margin_bottom(GTK_WIDGET(statusbar), 3);
	gtk_widget_set_margin_start(GTK_WIDGET(statusbar), 6);

	/* list info */
	priv->list_info = gtk_label_new(NULL);
	gtk_label_set_ellipsize(GTK_LABEL(priv->list_info),
				PANGO_ELLIPSIZE_END);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->list_info,
			   FALSE, FALSE, 0);

	/* primary text */
	priv->primary_text = gtk_label_new(NULL);
	gtk_label_set_ellipsize(GTK_LABEL(priv->primary_text),
				PANGO_ELLIPSIZE_END);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->primary_text,
			   FALSE, FALSE, 0);

	/* seconary text */
	priv->secondary_text = gtk_label_new(NULL);
	gtk_label_set_ellipsize(GTK_LABEL(priv->secondary_text),
				PANGO_ELLIPSIZE_END);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->secondary_text,
			   FALSE, FALSE, 12);

	/* action area */
	priv->action_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_show(priv->action_area);
	gtk_box_pack_end(GTK_BOX(statusbar), priv->action_area,
			FALSE, FALSE, 0);

	mtpc_statusbar_show_section(statusbar, MTPC_STATUSBAR_SECTION_FILE_LIST);
}

/* public methods */

GtkWidget *mtpc_statusbar_new(void)
{
	return g_object_new(MTPC_TYPE_STATUSBAR, NULL);
}

void mtpc_statusbar_set_list_info(MtpcStatusbar *statusbar, const char *text)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	gtk_label_set_text(GTK_LABEL(priv->list_info), text);
}

void mtpc_statusbar_set_primary_text(MtpcStatusbar *statusbar, const char *text)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	gtk_label_set_text(GTK_LABEL(priv->primary_text), text);
}

void mtpc_statusbar_set_secondary_text(MtpcStatusbar *statusbar, const char *text)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	gtk_label_set_text(GTK_LABEL(priv->secondary_text), text);
}

void mtpc_statusbar_show_section(MtpcStatusbar *statusbar,
				 MtpcStatusbarSection section)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	gtk_widget_set_visible (priv->list_info,
				section == MTPC_STATUSBAR_SECTION_FILE_LIST);
	gtk_widget_set_visible (priv->primary_text,
				section == MTPC_STATUSBAR_SECTION_FILE);
	gtk_widget_set_visible (priv->secondary_text,
				section == MTPC_STATUSBAR_SECTION_FILE);
}

GtkWidget *mtpc_statusbar_get_action_area(MtpcStatusbar *statusbar)
{
	MtpcStatusbarPrivate *priv;

	priv = mtpc_statusbar_get_instance_private(statusbar);

	return priv->action_area;
}
