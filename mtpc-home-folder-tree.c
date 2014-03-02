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

#include "mtpc-home-folder-tree.h"

typedef enum {
	ENTRY_TYPE_FILE,
	ENTRY_TYPE_FOLDER,
	ENTRY_TYPE_CWD,
	ENTRY_TYPE_PARENT
} EntryType;

enum {
	COLUMN_TYPE,
	COLUMN_NAME,
	COLUMN_SIZE,
	NUM_COLUMNS
};

typedef struct {
	int remove;
	GtkListStore *list_store;

	/* d-n-d */
	gboolean drag_source_enabled;
	GdkModifierType drag_start_button_mask;
	GtkTargetList *drag_target_list;
	GdkDragAction drag_actions;

	gboolean dragging:1; /* if dragging items */
	gboolean drag_started:1; /* drag started */
	int drag_start_x;
	int drag_start_y;

} MtpcHomeFolderTreePrivate;


G_DEFINE_TYPE_WITH_PRIVATE(MtpcHomeFolderTree,
			   mtpc_home_folder_tree,
			   GTK_TYPE_TREE_VIEW);



/* class implementation */
static void mtpc_home_folder_tree_finalize(GObject *object)
{
	G_OBJECT_CLASS(mtpc_home_folder_tree_parent_class)->finalize(object);
}

static void mtpc_home_folder_tree_class_init(MtpcHomeFolderTreeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);


	object_class->finalize = mtpc_home_folder_tree_finalize;
}

static void mtpc_home_folder_tree_init(MtpcHomeFolderTree *folder_tree)
{
	GtkTreeSelection *selection;
	MtpcHomeFolderTreePrivate *priv;

	priv = mtpc_home_folder_tree_get_instance_private(folder_tree);


	priv->list_store = gtk_list_store_new(NUM_COLUMNS,
					      G_TYPE_INT,
					      G_TYPE_STRING,
					      G_TYPE_INT);

	gtk_tree_view_set_model(GTK_TREE_VIEW(folder_tree),
				GTK_TREE_MODEL(priv->list_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(folder_tree), TRUE);
}

/* public functions */
GtkWidget *mtpc_home_folder_tree_new(void)
{
	MtpcHomeFolderTree *folder_tree;

	folder_tree = g_object_new(MTPC_TYPE_HOME_FOLDER_TREE, NULL);

	return GTK_WIDGET(folder_tree);
}
