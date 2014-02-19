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

#ifndef MTPC_ACTIONS_CALLBACKS_H
#define MTPC_ACTIONS_CALLBACKS_H
#include <gtk/gtk.h>


void toggle_action_activated(GSimpleAction *action,
			     GVariant      *parameter,
			     gpointer       data);
void mtpc_app_preferences(GSimpleAction *action,
				  GVariant *parameter,
				  gpointer user_data);
void mtpc_app_show_help(GSimpleAction *action,
				GVariant *parameter,
				gpointer user_data);
void mtpc_app_show_about(GSimpleAction *action,
				 GVariant *parameter,
				 gpointer user_data);
void mtpc_app_quit(GSimpleAction *action,
			   GVariant *parameter,
			   gpointer user_data);

void mtpc_window_activate_view_properties(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data);

void mtpc_window_activate_viewer_properties(GSimpleAction *action,
					    GVariant      *state,
					    gpointer       user_data);

void mtpc_window_activate_show_statusbar(GSimpleAction *action,
					 GVariant      *state,
					 gpointer       user_data);

void mtpc_window_activate_show_sidebar(GSimpleAction *action,
				       GVariant      *state,
				       gpointer       user_data);

void mtpc_window_activate_show_home_folder(GSimpleAction *action,
					   GVariant      *state,
					   gpointer       user_data);

void mtpc_window_activate_close(GSimpleAction *action,
				GVariant      *state,
				gpointer       user_data);
#endif /* MTPC_ACTIONS_CALLBACKS_H */
