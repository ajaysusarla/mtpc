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

#include <glib/gi18n.h>

#include "mtpc-actions-callbacks.h"
#include "mtpc-app.h"
#include "glib-utils.h"


void toggle_action_activated(GSimpleAction *action,
			     GVariant      *parameter,
			     gpointer       data)
{
        GVariant *state;

        state = g_action_get_state(G_ACTION (action));
        g_action_change_state(G_ACTION(action), g_variant_new_boolean(! g_variant_get_boolean(state)));

        g_variant_unref(state);
}

static GtkWidget * _mtpc_app_get_current_window(GApplication *application)
{
	GList *windows;

	windows = gtk_application_get_windows(GTK_APPLICATION(application));
	if (windows == NULL)
		return NULL;

	return GTK_WIDGET(windows->data);
}

void mtpc_app_refresh_device_list(GSimpleAction *action,
				  GVariant *parameter,
				  gpointer user_data)
{
	printf("mtpc_app_refresh_device_list\n");
	return;
}


void mtpc_app_preferences(GSimpleAction *action,
			  GVariant *parameter,
			  gpointer user_data)
{
}

void mtpc_app_show_help(GSimpleAction *action,
			GVariant *parameter,
			gpointer user_data)
{
}

void mtpc_app_show_about(GSimpleAction *action,
			 GVariant *parameter,
			 gpointer user_data)
{
	GApplication *application = user_data;
	GtkWidget *window;
	const char *authors[] = {
		"Parthasarathi Susarla",
		NULL
	};
	const char *documenters[] = {
		"",
		NULL
	};

	char *license_text;
	const char *license[] = {
		N_("mtpc is free software; you can redistribute it and/or modify "
		   "it under the terms of the GNU General Public License as published by "
		   "the Free Software Foundation; either version 2 of the License, or "
		   "(at your option) any later version."),
		N_("mtpc is distributed in the hope that it will be useful, "
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		   "GNU General Public License for more details."),
		N_("You should have received a copy of the GNU General Public License "
		   "along with mtpc.  If not, see http://www.gnu.org/licenses/.")
	};

	GdkPixbuf *logo = NULL;


	window = _mtpc_app_get_current_window(application);
	license_text = g_strconcat(_(license[0]), "\n\n",
				   _(license[1]), "\n\n",
				   _(license[2]),
				   NULL);

	gtk_show_about_dialog (GTK_WINDOW (window),
			       "version", VERSION_STRING,
			       "copyright", "Copyright \xc2\xa9 2001-2013 Free Software Foundation, Inc.",
			       "comments", _("A gtk+ based mtp client."),
			       "authors", authors,
			       "documenters", documenters,
			       "translator-credits", _("translator-credits"),
			       "license", license_text,
			       "wrap-license", TRUE,
			       "website", "https://github.com/ajaysusarla/mtpc",
			       (logo != NULL ? "logo" : NULL), logo,
			       NULL);

	_g_object_unref(logo);
	g_free(license_text);
}

void mtpc_app_quit(GSimpleAction *action,
		   GVariant *parameter,
		   gpointer user_data)
{
	MtpcApp *app  = MTPC_APP(user_data);
	GList *l;

        while ((l = gtk_application_get_windows (GTK_APPLICATION (app)))) {
                gtk_application_remove_window (GTK_APPLICATION (app),
                                               GTK_WINDOW (l->data));
        }
}

void mtpc_window_activate_view_properties(GSimpleAction *action,
					  GVariant      *state,
					  gpointer       user_data)

{
}

void mtpc_window_activate_viewer_properties(GSimpleAction *action,
					    GVariant      *state,
					    gpointer       user_data)

{
}

void mtpc_window_activate_show_statusbar(GSimpleAction *action,
					 GVariant      *state,
					 gpointer       user_data)
{
}

void mtpc_window_activate_show_sidebar(GSimpleAction *action,
				       GVariant      *state,
				       gpointer       user_data)
{
}

void mtpc_window_activate_show_home_folder(GSimpleAction *action,
					   GVariant      *state,
					   gpointer       user_data)
{
}


void mtpc_window_activate_close(GSimpleAction *action,
				GVariant      *state,
				gpointer       user_data)
{
}
