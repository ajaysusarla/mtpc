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
#include <string.h>
#include <glib.h>
#include "gtk-utils.h"
#include "glib-utils.h"


GtkBuilder * _gtk_builder_new_from_file(const char *ui_file)
{
        char *filename;
        GtkBuilder *builder;
        GError *error = NULL;

        filename = g_build_filename(MTPC_UI_DIR, ui_file, NULL);

        builder = gtk_builder_new();
        if (!gtk_builder_add_from_file(builder, filename, &error)) {
                g_warning("%s\n", error->message);
                g_clear_error(&error);
        }

        g_free(filename);

        return builder;

}

void _gtk_widget_hide(GtkWidget *widget)
{
        if (widget != NULL)
                gtk_widget_hide(widget);
}


void _gtk_widget_show(GtkWidget *widget)
{
        if (widget != NULL)
                gtk_widget_show(widget);
}


/* -- _gtk_menu_ask_drag_drop_action --(from gthumb) */


typedef struct {
	GMainLoop     *loop;
	GdkDragAction  action_name;
} DropActionData;


static void
ask_drag_drop_action_menu_deactivate_cb(GtkMenuShell *menushell,
					gpointer      user_data)
{
	DropActionData *drop_data = user_data;

	if (g_main_loop_is_running(drop_data->loop))
		g_main_loop_quit(drop_data->loop);
}


static void
ask_drag_drop_action_item_activate_cb(GtkMenuItem *menuitem,
				      gpointer     user_data)
{
	DropActionData *drop_data = user_data;

	drop_data->action_name = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menuitem), "drop-action"));
	if (g_main_loop_is_running(drop_data->loop))
		g_main_loop_quit(drop_data->loop);
}


static void
_gtk_menu_ask_drag_drop_action_append_item(GtkWidget      *menu,
					   const char     *label,
					   GdkDragAction   actions,
					   GdkDragAction   action,
					   DropActionData *drop_data)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_mnemonic(label);
	g_object_set_data(G_OBJECT(item), "drop-action", GINT_TO_POINTER(action));
	gtk_widget_set_sensitive(item, ((actions & action) != 0));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL (menu), item);

	g_signal_connect(item,
			 "activate",
			 G_CALLBACK(ask_drag_drop_action_item_activate_cb),
			 drop_data);
}


GdkDragAction
_gtk_menu_ask_drag_drop_action(GtkWidget     *widget,
			       GdkDragAction  actions,
			       guint32        activate_time)
{
	DropActionData  drop_data;
	GtkWidget      *menu;
	GtkWidget      *item;

	drop_data.action_name = 0;
	drop_data.loop = g_main_loop_new(NULL, FALSE);

	menu = gtk_menu_new();
	gtk_menu_set_screen(GTK_MENU(menu), gtk_widget_get_screen(widget));

	_gtk_menu_ask_drag_drop_action_append_item(menu,
						   _("_Copy Here"),
						   actions,
						   GDK_ACTION_COPY,
						   &drop_data);
	_gtk_menu_ask_drag_drop_action_append_item(menu,
						   _("_Move Here"),
						   actions,
						   GDK_ACTION_MOVE,
						   &drop_data);
	_gtk_menu_ask_drag_drop_action_append_item(menu,
						   _("_Link Here"),
						   actions,
						   GDK_ACTION_LINK,
						   &drop_data);

	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Cancel"));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	g_signal_connect(menu,
			 "deactivate",
			 G_CALLBACK(ask_drag_drop_action_menu_deactivate_cb),
			 &drop_data);

	gtk_menu_popup(GTK_MENU(menu),
		       NULL,
		       NULL,
		       NULL,
		       NULL,
		       0,
		       activate_time);
	gtk_grab_add(menu);
	g_main_loop_run(drop_data.loop);

	gtk_grab_remove(menu);
	gtk_widget_destroy(menu);
	g_main_loop_unref(drop_data.loop);

	return drop_data.action_name;
}
