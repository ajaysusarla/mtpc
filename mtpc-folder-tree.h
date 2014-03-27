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

#ifndef MTPC_FOLDER_TREE_H
#define MTPC_FOLDER_TREE_H

#include <gtk/gtk.h>

#include "mtpc-file-data.h"

#define MTPC_TYPE_FOLDER_TREE              (mtpc_folder_tree_get_type())
#define MTPC_FOLDER_TREE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_FOLDER_TREE, MtpcFolderTree))
#define MTPC_FOLDER_TREE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_FOLDER_TREE, MtpcFolderTreeClass))
#define MTPC_IS_FOLDER_TREE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_FOLDER_TREE))
#define MTPC_IS_FOLDER_TREE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_FOLDER_TREE))
#define MTPC_FOLDER_TREE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_FOLDER_TREE, MtpcFolderTreeClass))

typedef struct _MtpcFolderTree MtpcFolderTree;
typedef struct _MtpcFolderTreeClass MtpcFolderTreeClass;

struct _MtpcFolderTree {
GtkTreeView parent;
};

struct _MtpcFolderTreeClass {
GtkTreeViewClass parent_class;

void (*folder_popup) (MtpcFolderTree *folder_tree,
                              GFileInfo *info);

void (*load) (MtpcFolderTree *folder_tree,
		      GFileInfo *info);

void (*open) (MtpcFolderTree *folder_tree,
		      GFileInfo *info);
};


GType mtpc_folder_tree_get_type(void);
GtkWidget *mtpc_folder_tree_new(void);
void mtpc_folder_tree_set_list(MtpcFolderTree *folder_tree,
			       const char *current_path,
			       long parent_id,
			       MtpcFileData *parent_fdata,
			       GList *file_list);
void mtpc_folder_tree_clear(MtpcFolderTree *folder_tree);
void mtpc_folder_tree_enable_drag_source(MtpcFolderTree *folder_tree,
					 GdkModifierType start_button_mask,
					 const GtkTargetEntry *targets,
					 int n_targets,
					 GdkDragAction actions);
void mtpc_folder_tree_unset_drag_source(MtpcFolderTree *folder_tree);

#endif /* MTPC_FOLDER_TREE_H */

