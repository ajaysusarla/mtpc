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
#ifndef MTPC_HOME_FOLDER_TREE_H
#define MTPC_HOME_FOLDER_TREE_H

#include <gtk/gtk.h>

#define MTPC_TYPE_HOME_FOLDER_TREE              (mtpc_home_folder_tree_get_type())
#define MTPC_HOME_FOLDER_TREE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), MTPC_TYPE_HOME_FOLDER_TREE, MtpcHomeFolderTree))
#define MTPC_HOME_FOLDER_TREE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), MTPC_TYPE_HOME_FOLDER_TREE, MtpcHomeFolderTreeClass))
#define MTPC_IS_HOME_FOLDER_TREE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), MTPC_TYPE_HOME_FOLDER_TREE))
#define MTPC_IS_HOME_FOLDER_TREE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), MTPC_TYPE_HOME_FOLDER_TREE))
#define MTPC_HOME_FOLDER_TREE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), MTPC_TYPE_HOME_FOLDER_TREE, MtpcHomeFolderTreeClass))

typedef struct _MtpcHomeFolderTree MtpcHomeFolderTree;
typedef struct _MtpcHomeFolderTreeClass MtpcHomeFolderTreeClass;

struct _MtpcHomeFolderTree {
	GtkTreeView parent;
};

struct _MtpcHomeFolderTreeClass {
	GtkTreeViewClass parent_class;

        void (*folder_popup) (MtpcHomeFolderTree *folder_tree,
                              GFileInfo *info);

        void (*load) (MtpcHomeFolderTree *folder_tree,
                              GFileInfo *info);

        void (*open) (MtpcHomeFolderTree *folder_tree,
                              GFileInfo *info);
};


GType mtpc_home_folder_tree_get_type(void);
GtkWidget *mtpc_home_folder_tree_new(void);
void mtpc_home_folder_tree_set_list(MtpcHomeFolderTree *folder_tree,
				    const char *current_path,
				    GFile *parent,
				    GList *file_list);
void mpc_home_folder_tree_clear(MtpcHomeFolderTree *folder_tree);

#endif /* MTPC_HOME_FOLDER_TREE_H */
