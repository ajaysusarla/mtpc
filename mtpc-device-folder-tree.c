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

#include "mtpc-device-folder-tree.h"
#include "glib-utils.h"



enum {
        FOLDER_POPUP,
        LOAD,
        OPEN,
        LAST_SIGNAL
};

enum {
	COLUMN_TYPE,
	COLUMN_ICON,
	COLUMN_NAME,
	COLUMN_SIZE,
	COLUMN_GFILE,
	NUM_COLUMNS
};

typedef struct {
	GtkTreeStore *tree_store;
	GtkCellRenderer *name_text_renderer;
	GtkCellRenderer *size_text_renderer;

	/* d-n-d */
	gboolean drag_source_enabled;
	GdkModifierType drag_start_button_mask;
	GtkTargetList *drag_target_list;
	GdkDragAction drag_actions;

	gboolean dragging:1; /* if dragging items */
	gboolean drag_started:1; /* drag started */
	int drag_start_x;
	int drag_start_y;
} MtpcDeviceFolderTreePrivate;


static guint mtpc_device_folder_tree_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(MtpcDeviceFolderTree,
			   mtpc_device_folder_tree,
			   GTK_TYPE_TREE_VIEW);

/* internal and callbacks */
static void add_columns(MtpcDeviceFolderTree *folder_tree,
			GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	MtpcDeviceFolderTreePrivate *priv;

	priv = mtpc_device_folder_tree_get_instance_private(folder_tree);

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new ();
	g_object_set (renderer,
		      "follow-state", TRUE,
		      NULL);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer,
					     "gicon", COLUMN_ICON,
					     NULL);

	/* folder/file name */
	gtk_tree_view_column_set_spacing(column, 1);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);
        gtk_tree_view_column_set_title(column, "Name");

	priv->name_text_renderer = gtk_cell_renderer_text_new();

	g_object_set(priv->name_text_renderer,
		     "ellipsize", PANGO_ELLIPSIZE_END,
		     NULL);

	gtk_tree_view_column_pack_start(column, priv->name_text_renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, priv->name_text_renderer,
					    "text", COLUMN_NAME,
					    NULL);

        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), column);


	/* size */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_spacing(column, 1);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_column_set_sort_column_id(column, COLUMN_SIZE);
        gtk_tree_view_column_set_title(column, "Size");

	priv->size_text_renderer = gtk_cell_renderer_text_new();

	g_object_set(priv->size_text_renderer,
		     "ellipsize", PANGO_ELLIPSIZE_END,
		     NULL);

	gtk_tree_view_column_pack_start(column, priv->size_text_renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, priv->size_text_renderer,
					    "text", COLUMN_SIZE,
					    NULL);

        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), column);

}

/* class implementation */
static void mtpc_device_folder_tree_finalize(GObject *object)
{
	G_OBJECT_CLASS(mtpc_device_folder_tree_parent_class)->finalize(object);
}

static void mtpc_device_folder_tree_class_init(MtpcDeviceFolderTreeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);


	object_class->finalize = mtpc_device_folder_tree_finalize;

	widget_class->drag_begin = NULL;


        mtpc_device_folder_tree_signals[FOLDER_POPUP] =
                g_signal_new("folder_popup",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcDeviceFolderTreeClass,
					     folder_popup),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

        mtpc_device_folder_tree_signals[LOAD] =
                g_signal_new("load",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcDeviceFolderTreeClass,
					     load),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

        mtpc_device_folder_tree_signals[OPEN] =
                g_signal_new("open",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcDeviceFolderTreeClass,
					     open),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

}


static void mtpc_device_folder_tree_init(MtpcDeviceFolderTree *folder_tree)
{
	GtkTreeSelection *selection;
	MtpcDeviceFolderTreePrivate *priv;


	priv = mtpc_device_folder_tree_get_instance_private(folder_tree);


	priv->drag_source_enabled = FALSE;
	priv->dragging  = FALSE;
	priv->drag_started = FALSE;
	priv->drag_target_list = NULL;

	priv->tree_store = gtk_tree_store_new(NUM_COLUMNS,
					      G_TYPE_INT,
					      G_TYPE_ICON,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_POINTER);

	gtk_tree_view_set_model(GTK_TREE_VIEW(folder_tree),
				GTK_TREE_MODEL(priv->tree_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(folder_tree), TRUE);

	add_columns(folder_tree, GTK_TREE_VIEW(folder_tree));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(folder_tree), TRUE);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(folder_tree), TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(folder_tree), COLUMN_NAME);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(folder_tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

}

/* public functions */
GtkWidget *mtpc_device_folder_tree_new(void)
{
	MtpcDeviceFolderTree *folder_tree;

	folder_tree = g_object_new(MTPC_TYPE_DEVICE_FOLDER_TREE, NULL);

	return GTK_WIDGET(folder_tree);
}
