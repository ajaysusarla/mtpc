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

#include "mtpc-app.h"
#include "mtpc-window.h"
#include "mtpc-actions-callbacks.h"

typedef struct {
	gboolean show_app_menu;
} MtpcAppPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MtpcApp, mtpc_app, GTK_TYPE_APPLICATION)


/* callbacks and private methods */
/* app menu */
static const GActionEntry app_menu_entries[] = {
        {"preference", mtpc_app_preferences},
        {"help", mtpc_app_show_help},
        {"about", mtpc_app_show_about},
        {"quit", mtpc_app_quit}
};

static void _mtpc_app_setup_accelerators(MtpcApp *app)
{
        gtk_application_add_accelerator (GTK_APPLICATION (app),
					 "<Ctrl>i",
					 "win.toggle-device-properties",
					 NULL);
}

static void _mtpc_app_initialize_app_menu(MtpcApp *app)
{
        GtkBuilder *builder;
	GApplication *application = G_APPLICATION(app);
	MtpcAppPrivate *priv = mtpc_app_get_instance_private(app);

        g_action_map_add_action_entries(G_ACTION_MAP(application),
                                        app_menu_entries,
                                        G_N_ELEMENTS(app_menu_entries),
                                        application);

        builder = _gtk_builder_new_from_file("app-menu.ui");

	if (priv->show_app_menu) {
		gtk_application_set_app_menu(GTK_APPLICATION(application),
					     G_MENU_MODEL(gtk_builder_get_object(builder,
										 "app-menu")));
	} {
		gtk_application_set_menubar (GTK_APPLICATION (application),
					     G_MENU_MODEL (gtk_builder_get_object (builder, "menubar")));
	}
        g_object_unref(builder);
}

static void new_mtpc_window(GApplication *application)
{
        GtkWidget *window;

        window = mtpc_window_new(MTPC_APP(application));

        gtk_widget_show(GTK_WIDGET(window));

        gtk_window_present(GTK_WINDOW(window));
}

/* class implementation */

static void mtpc_app_finalize(GObject *object)
{
        G_OBJECT_CLASS(mtpc_app_parent_class)->finalize(object);
}

static void mtpc_app_startup(GApplication *application)
{
	MtpcApp *app = MTPC_APP(application);

        G_APPLICATION_CLASS(mtpc_app_parent_class)->startup(application);

#if 0
        g_object_set(gtk_settings_get_default(),
                     "gtk-application-prefer-dark-theme",
                     TRUE, NULL);
#endif

        _mtpc_app_initialize_app_menu(app);
	_mtpc_app_setup_accelerators(app);
}

static void mtpc_app_shutdown(GApplication *application)
{
        G_APPLICATION_CLASS(mtpc_app_parent_class)->shutdown(application);
}

static void mtpc_app_activate(GApplication *application)
{
	new_mtpc_window(application);
}


static int mtpc_app_command_line(GApplication *application,
				 GApplicationCommandLine *command_line)
{
        return 0;
}

static void mtpc_app_class_init(MtpcAppClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	app_class->startup = mtpc_app_startup;
	app_class->shutdown = mtpc_app_shutdown;
	app_class->activate = mtpc_app_activate;
	app_class->command_line = mtpc_app_command_line;

	obj_class->finalize = mtpc_app_finalize;
}

static void mtpc_app_init(MtpcApp *app)
{
	MtpcAppPrivate *priv = mtpc_app_get_instance_private(app);

	g_set_application_name(APP_NAME);

	priv->show_app_menu = FALSE;
}

/* public Methods */
MtpcApp *mtpc_app_new(gboolean show_app_menu)
{
	MtpcApp *app;
	MtpcAppPrivate *priv;

	app = g_object_new(mtpc_app_get_type(),
			   "application-id", "org.gtk.mtpc",
			   "register-session",TRUE, /* gtk_application_inhibit */
			   "flags", 0,
			   NULL);

	priv = mtpc_app_get_instance_private(app);
	priv->show_app_menu = show_app_menu;

	return app;
}

