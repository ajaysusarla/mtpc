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

#include "mtpc-folder-tree.h"
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
	COLUMN_FOLDER_ID,
	COLUMN_FDATA,
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
	GtkTreePath *double_click_path[2]; /* To monitor double clicks */

	gboolean dragging:1; /* if dragging items */
	gboolean drag_started:1; /* drag started */
	gboolean drag_selection:1; /* should row be selected or not*/
	int drag_start_x;
	int drag_start_y;
} MtpcFolderTreePrivate;


static guint mtpc_folder_tree_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(MtpcFolderTree,
			   mtpc_folder_tree,
			   GTK_TYPE_TREE_VIEW);

/* internal methods and callbacks */
static gboolean selection_changed_cb(GtkTreeSelection *selection,
				     gpointer user_data)
{
	return FALSE;
}

static void open_folder(MtpcFolderTree *folder_tree, MtpcFileData *fdata)
{
	g_signal_emit(folder_tree,
		      mtpc_folder_tree_signals[OPEN],
		      0, fdata);
}

static gboolean popup_menu_cb(GtkWidget *widget, gpointer user_data)
{
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	MtpcFolderTreePrivate *priv;
	GtkTreeStore *tree_store;
	GtkTreeIter iter;
	MtpcFileData *fdata;
	GList *file_list;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);
	tree_store = priv->tree_store;

	file_list = mtpc_folder_tree_get_selected_items(folder_tree);
	if (file_list == NULL)
		return FALSE;

	g_signal_emit(folder_tree,
		      mtpc_folder_tree_signals[FOLDER_POPUP],
		      0,
		      file_list);

	return TRUE;
}

static gboolean row_activated_cb(GtkTreeView *tree_view,
				 GtkTreePath *path,
				 GtkTreeViewColumn *column,
				 gpointer user_data)
{

	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	MtpcFolderTreePrivate *priv;
	GtkTreeIter iter;
	MtpcFileData *fdata;


	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->tree_store),
				    &iter,
				    path))
	{
		return FALSE;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(priv->tree_store),
			   &iter,
			   COLUMN_FDATA, &fdata,
			   -1);

	open_folder(folder_tree, fdata);
	return TRUE;
}

static gboolean folder_tree_selection_func(GtkTreeSelection *selection,
					   GtkTreeModel *model,
					   GtkTreePath *path,
					   gboolean path_currently_selected,
					   gpointer data)
{
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(MTPC_FOLDER_TREE(data));

	return priv->drag_selection;
}

static GIcon * get_drag_surface(MtpcFolderTree *folder_tree)
{
	MtpcFolderTreePrivate *priv;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	GIcon *icon;

	icon = NULL;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(folder_tree),
					  priv->drag_start_x,
					  priv->drag_start_y,
					  &path, NULL, NULL, NULL)) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(folder_tree));
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model,
				   &iter,
				   COLUMN_ICON,
				   &icon,
				   -1);

		gtk_tree_path_free(path);
	}

	return icon;
}

static void drag_begin_cb(GtkWidget *widget,
			  GdkDragContext *context,
			  gpointer user_data)
{
	/*
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	MtpcFolderTreePrivate *priv;
	GList *selection_cache;
	GIcon *icon;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	icon = get_drag_surface(folder_tree);
	if (icon) {
		gtk_drag_set_icon_gicon(context, icon, 0, 5);
	} else {
		gtk_drag_set_icon_default(context);
	}

	priv->drag_started = TRUE;
	*/
	GIcon *icon;

	icon = g_themed_icon_new("folder");

	gtk_drag_source_set_icon_gicon(widget, icon);
}

static gboolean button_event_modifies_selection(GdkEventButton *event)
{
        return (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) != 0;
}

static gboolean button_press_event_cb(GtkWidget *widget,
				      GdkEventButton *event,
				      gpointer user_data)
{
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	GtkTreeView *tree_view;
	MtpcFolderTreePrivate *priv;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkWidgetClass *tree_view_class;
	GtkTreeIter iter;
	gboolean retval = FALSE;
	gboolean call_parent, path_selected, is_simple_click;
	GtkTreeViewColumn *column;
	int cell_x, cell_y;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	tree_view = GTK_TREE_VIEW(folder_tree);
	tree_view_class = GTK_WIDGET_GET_CLASS(tree_view);
	selection = gtk_tree_view_get_selection(tree_view);

	gtk_widget_grab_focus(widget);

	/* We don't handle extra mouse buttons */
	if (event->button > 5) {
		return FALSE;
	}

	if (event->window != gtk_tree_view_get_bin_window(tree_view)) {
		return FALSE;
	}

	is_simple_click = ((event->button == 1 || event->button == 2) &&
			   (event->type == GDK_BUTTON_PRESS));

	priv->drag_selection = TRUE;
	priv->drag_start_x = -1;
	priv->drag_start_y = -1;

	if (!gtk_tree_view_get_path_at_pos(tree_view, event->x, event->y,
					   &path, NULL, NULL, NULL)) {
		if (is_simple_click) {
			g_clear_pointer(&priv->double_click_path[1],
					gtk_tree_path_free);
			priv->double_click_path[1] = priv->double_click_path[0];
			priv->double_click_path[0] = NULL;
		}

		/* deselect if clicked outside any row. */
		gtk_tree_selection_unselect_all(selection);
		tree_view_class->button_press_event(widget, event);

		if (event->button == 3) {
			g_signal_emit(folder_tree,
				      mtpc_folder_tree_signals[FOLDER_POPUP],
				      0,
				      NULL);
			retval = TRUE;
		}

		return retval;
	}

	call_parent = TRUE;
	path_selected = gtk_tree_selection_path_is_selected(selection, path);

	/* make sure double clicks only happen on the same item */
	if (is_simple_click) {
		g_clear_pointer(&priv->double_click_path[1],
				gtk_tree_path_free);
		priv->double_click_path[1] = priv->double_click_path[0];
		priv->double_click_path[0] = gtk_tree_path_copy(path);
	}

	if (event->type == GDK_2BUTTON_PRESS) {
		/* double clicking doesn't mean d-n-d */
		priv->dragging = TRUE;

		if (priv->double_click_path[1] &&
		    gtk_tree_path_compare(priv->double_click_path[0],
					  priv->double_click_path[1]) == 0) {
			if ((event->button == 1) &&
			    button_event_modifies_selection(event)) {
				printf("Modifies selection\n");
			} else {
				printf("Does not modify selection\n");
			}
		} else {
			tree_view_class->button_press_event(widget, event);
		}
	} else {
		if (event->button == 3 && path_selected)
			call_parent = FALSE;

		if ((event->button == 1 || event->button == 2) &&
		    ((event->state & GDK_CONTROL_MASK) !=0 ||
		     (event->state & GDK_SHIFT_MASK) == 0)) {
			if (path_selected) {
				printf("Path Selected\n");
			} else if ((event->state & GDK_CONTROL_MASK) != 0) {
				GList *selected_rows, *l;

				call_parent = FALSE;

				if ((event->state & GDK_SHIFT_MASK) != 0) {
					GtkTreePath *cursor;
					gtk_tree_view_get_cursor(tree_view,
								 &cursor,
								 NULL);
					if (cursor != NULL) {
						gtk_tree_selection_select_range(selection, cursor, path);
					} else {
						gtk_tree_selection_select_path(selection, path);
					}
				} else {
					gtk_tree_selection_select_path(selection,
								       path);
				}
				selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);
				printf("---->selcted items:%d\n", g_list_length(selected_rows));

				/* this unselects everything... */
				gtk_tree_view_set_cursor(tree_view, path, NULL, FALSE);
				printf("---->unselecting everything\n");

				/* so select it again */
				for (l = selected_rows; l != NULL; l = l->next)
					gtk_tree_selection_select_path(selection,
						l->data);

				printf("---->selecting everything back\n");
				g_list_free_full(selected_rows,
						 (GDestroyNotify)gtk_tree_path_free);
				printf("---->done freeing\n");
			} else {
				printf("ignore button release");
			}
		}

		if (call_parent) {
			g_signal_handlers_block_by_func(tree_view,
							row_activated_cb,
							folder_tree);
			tree_view_class->button_press_event(widget, event);
			g_signal_handlers_unblock_by_func(tree_view,
							  row_activated_cb,
							  folder_tree);
		} else if (path_selected) {
			gtk_widget_grab_focus(widget);
		}

		if (is_simple_click) {
			printf("button_press_event_cb:...dragging..\n");
			priv->dragging = TRUE;
			priv->drag_start_x = event->x;
			priv->drag_start_y = event->y;
			priv->drag_selection = FALSE;
		}

		if (event->button == 3) {
			printf("*** popup menu should come here\n");
		}
	}

	gtk_tree_path_free(path);

	/* we have chained the default handler, so don't let it run now. */
	return TRUE;
}

static gboolean button_release_event_cb(GtkWidget *widget,
					GdkEventButton *event,
					gpointer user_data)
{
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	if (priv->dragging) {
		if(priv->drag_start_x != -1) {
			priv->drag_selection = TRUE;
			if(priv->drag_start_x == event->x &&
			   priv->drag_start_y == event->y) {
				GtkTreePath *path = NULL;
				GtkTreeViewColumn *col;
				if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget),
								 event->x, event->y,
								 &path,
								 &col,
								 NULL, NULL)) {
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, col, FALSE);
				}
				if(path)
					gtk_tree_path_free(path);
			}
		}
		priv->drag_start_x = -1;
		priv->drag_start_y = -1;
		priv->dragging = FALSE;
		priv->drag_started = FALSE;
	}

	return FALSE;
}

static gboolean motion_notify_event_cb(GtkWidget      *widget,
				       GdkEventButton *event,
				       gpointer        user_data)
{
	MtpcFolderTree *folder_tree = MTPC_FOLDER_TREE(user_data);
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	if (!priv->drag_source_enabled)
		return FALSE;

	if (priv->dragging) {
		if (!priv->drag_started
		    && gtk_drag_check_threshold(widget,
						priv->drag_start_x,
						priv->drag_start_y,
						event->x,
						event->y))
		{
			GtkTreePath *path = NULL;
			GdkDragContext *context;
			int cell_x;
			int cell_y;

			if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(folder_tree),
							   event->x,
							   event->y,
							   &path,
							   NULL,
							   &cell_x,
							   &cell_y))
				return FALSE;

			gtk_tree_view_set_cursor(GTK_TREE_VIEW(folder_tree),
						 path,
						 NULL,
						 FALSE);
			priv->drag_started = TRUE;

			context = gtk_drag_begin_with_coordinates(widget,
								  priv->drag_target_list,
								  priv->drag_actions,
								  1,
								  (GdkEvent *)event,
								  -1,
								  -1);

			gtk_tree_path_free(path);

			printf("motion_notify_event_cb:...DRAGGING...\n");
		}

		return TRUE;
	}

	return FALSE;
}

static void add_columns(MtpcFolderTree *folder_tree,
			GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

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

static void mtpc_folder_tree_finalize(GObject *object)
{
	G_OBJECT_CLASS(mtpc_folder_tree_parent_class)->finalize(object);
}

static void mtpc_folder_tree_class_init(MtpcFolderTreeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);


	object_class->finalize = mtpc_folder_tree_finalize;

	widget_class->drag_begin = NULL;


        mtpc_folder_tree_signals[FOLDER_POPUP] =
                g_signal_new("folder_popup",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcFolderTreeClass,
					     folder_popup),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

        mtpc_folder_tree_signals[LOAD] =
                g_signal_new("load",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcFolderTreeClass,
					     load),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

        mtpc_folder_tree_signals[OPEN] =
                g_signal_new("open",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST,
                             G_STRUCT_OFFSET(MtpcFolderTreeClass,
					     open),
                             NULL, NULL,
                             g_cclosure_marshal_VOID__POINTER,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_POINTER);

}

static void mtpc_folder_tree_init(MtpcFolderTree *folder_tree)
{
	GtkTreeSelection *selection;
	MtpcFolderTreePrivate *priv;


	priv = mtpc_folder_tree_get_instance_private(folder_tree);


	priv->drag_source_enabled = FALSE;
	priv->dragging  = FALSE;
	priv->drag_started = FALSE;
	priv->drag_target_list = NULL;

	priv->tree_store = gtk_tree_store_new(NUM_COLUMNS,
					      G_TYPE_INT,
					      G_TYPE_ICON,
					      G_TYPE_STRING,
					      G_TYPE_STRING,
					      G_TYPE_LONG,
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
	gtk_tree_selection_set_select_function(selection,
					       folder_tree_selection_func,
					       folder_tree,
					       NULL);

	/* signal handlers */
	g_signal_connect(selection, "changed",
			 G_CALLBACK(selection_changed_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "popup-menu",
			 G_CALLBACK(popup_menu_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "button-press-event",
			 G_CALLBACK(button_press_event_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "button-release-event",
			 G_CALLBACK(button_release_event_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "drag-begin",
			 G_CALLBACK(drag_begin_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "row-activated",
			 G_CALLBACK(row_activated_cb),
			 folder_tree);
	g_signal_connect(folder_tree,
			 "motion-notify-event",
			 G_CALLBACK(motion_notify_event_cb),
			 folder_tree);

}

/* public methods */

GtkWidget *mtpc_folder_tree_new(void)
{
	MtpcFolderTree *folder_tree;

	folder_tree = g_object_new(MTPC_TYPE_FOLDER_TREE, NULL);

	return GTK_WIDGET(folder_tree);
}

void mtpc_folder_tree_clear(MtpcFolderTree *folder_tree)
{
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	gtk_tree_store_clear(priv->tree_store);
}

void mtpc_folder_tree_set_list(MtpcFolderTree *folder_tree,
			       MtpcFileData *parent_fdata,
			       GList *file_list)
{
	MtpcFolderTreePrivate *priv;
	GList *l;

	l = file_list;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	gtk_tree_store_clear(priv->tree_store);

	if (mtpc_file_data_has_parent(parent_fdata)) {
		GtkTreeIter iter;
		GIcon *icon;
		long parent_id;

		parent_id = mtpc_file_data_get_folder_id(parent_fdata);
		icon = g_themed_icon_new("folder");

		gtk_tree_store_append(priv->tree_store, &iter, NULL);

		gtk_tree_store_set(priv->tree_store, &iter,
				   COLUMN_TYPE, G_FILE_TYPE_DIRECTORY,
				   COLUMN_ICON, icon,
				   COLUMN_NAME, "../",
				   COLUMN_SIZE, "",
				   COLUMN_FOLDER_ID, parent_id,
				   COLUMN_FDATA, parent_fdata,
				   -1);

		_g_object_unref(icon);
	}

	while(l) {
		MtpcFileData *fdata = l->data;
		GFileInfo *info;
		GtkTreeIter iter;
		GIcon *icon;
		const char *name;
		char size[20];
		GFileType ftype;
		long folder_id;

		info = mtpc_file_data_get_file_info(fdata);

                if (!g_file_info_get_is_hidden(info)) {

			folder_id = mtpc_file_data_get_folder_id(fdata);

			name = g_file_info_get_name(info);

			sprintf(size, "%dKB", (int)g_file_info_get_size(info)/1024);
			ftype = g_file_info_get_file_type(info);

			icon = g_file_info_get_icon(info);

			gtk_tree_store_append(priv->tree_store, &iter, NULL);

			gtk_tree_store_set(priv->tree_store, &iter,
					   COLUMN_TYPE, ftype,
					   COLUMN_ICON, icon,
					   COLUMN_NAME, name,
					   COLUMN_SIZE, size,
					   COLUMN_FOLDER_ID, folder_id,
					   COLUMN_FDATA, fdata,
					   -1);
		}

		l = l->next;
	}

}

void mtpc_folder_tree_enable_drag(MtpcFolderTree *folder_tree,
				  GdkModifierType start_button_mask,
				  const GtkTargetEntry *targets,
				  int n_targets,
				  GdkDragAction actions)
{
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	if (priv->drag_target_list != NULL)
		gtk_target_list_unref(priv->drag_target_list);

	priv->drag_source_enabled = TRUE;
	priv->drag_start_button_mask = start_button_mask;
	priv->drag_target_list = gtk_target_list_new(targets, n_targets);
	priv->drag_actions = actions;

	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(folder_tree),
					     targets,
					     n_targets,
					     actions |
					     GDK_ACTION_ASK);

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(folder_tree),
					       GDK_BUTTON1_MASK,
					       targets,
					       n_targets,
					       actions);
}


void mtpc_folder_tree_disable_drag(MtpcFolderTree *folder_tree)
{
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

	priv->drag_source_enabled = FALSE;
	if (priv->drag_target_list != NULL)
		gtk_target_list_unref(priv->drag_target_list);
}

MtpcFileData *mtpc_folder_tree_get_file(MtpcFolderTree *folder_tree,
					GtkTreePath   *path)
{
        GtkTreeModel *tree_model;
	MtpcFolderTreePrivate *priv;
        GtkTreeIter   iter;
        MtpcFileData  *file_data;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

        tree_model = GTK_TREE_MODEL(priv->tree_store);

        if (!gtk_tree_model_get_iter(tree_model, &iter, path))
		return NULL;

        file_data = NULL;
        gtk_tree_model_get(tree_model,
			   &iter,
			   COLUMN_FDATA, &file_data,
			   -1);

        return file_data;
}

MtpcFileData *mtpc_folder_tree_get_selected_item(MtpcFolderTree *folder_tree)
{
        GtkTreeSelection *selection;
        GtkTreeModel     *tree_model;
        GtkTreeIter       iter;
        MtpcFileData      *file_data;
	MtpcFolderTreePrivate *priv;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(folder_tree));
        if (selection == NULL)
                return NULL;

        tree_model = GTK_TREE_MODEL(priv->tree_store);
        if (!gtk_tree_selection_get_selected(selection, &tree_model, &iter))
                return NULL;

        gtk_tree_model_get(tree_model,
			   &iter,
			   COLUMN_FDATA, &file_data,
			   -1);

        return file_data;
}

GList *mtpc_folder_tree_get_selected_items(MtpcFolderTree *folder_tree)
{
        GtkTreeSelection *selection;
        GtkTreeModel     *tree_model;
        GtkTreeIter       iter;
	MtpcFolderTreePrivate *priv;
	GList *paths, *file_list, *t;

	priv = mtpc_folder_tree_get_instance_private(folder_tree);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(folder_tree));
        if (selection == NULL)
                return NULL;

        tree_model = GTK_TREE_MODEL(priv->tree_store);

	paths = gtk_tree_selection_get_selected_rows(selection,
						     &tree_model);

	if (paths == NULL)
		return NULL;


	t = paths;
	file_list = NULL;

	while (t != NULL) {
		MtpcFileData *fdata;
		GtkTreePath *path = (GtkTreePath *)t->data;

		if (gtk_tree_model_get_iter(tree_model, &iter, path)) {

			gtk_tree_model_get(tree_model,
					   &iter,
					   COLUMN_FDATA, &fdata,
					   -1);

			file_list = g_list_prepend(file_list, fdata);
		}

		t = t->next;
	}

	return file_list;
}
