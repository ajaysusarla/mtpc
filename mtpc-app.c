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


struct _MtpcAppPrivate {
	int remove;
};

G_DEFINE_TYPE (MtpcApp, mtpc_app, GTK_TYPE_APPLICATION)


/* callbacks and private methods */
static void mtpc_app_finalize(GObject *object)
{
        G_OBJECT_CLASS(mtpc_app_parent_class)->finalize(object);
}

static void mtpc_app_startup(GApplication *application)
{
        G_APPLICATION_CLASS(mtpc_app_parent_class)->startup(application);

        g_object_set(gtk_settings_get_default(),
                     "gtk-application-prefer-dark-theme",
                     TRUE, NULL);

}

static void mtpc_app_shutdown(GApplication *application)
{
        G_APPLICATION_CLASS(mtpc_app_parent_class)->shutdown(application);
}

static void mtpc_app_activate(GApplication *application)
{
}


static int mtpc_app_command_line(GApplication *application,
				 GApplicationCommandLine *command_line)
{
        return 0;
}



/* class implementation */
static void mtpc_app_class_init(MtpcAppClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(MtpcAppPrivate));

	app_class->startup = mtpc_app_startup;
	app_class->shutdown = mtpc_app_shutdown;
	app_class->activate = mtpc_app_activate;
	app_class->command_line = mtpc_app_command_line;

	obj_class->finalize = mtpc_app_finalize;
}

static void mtpc_app_init(MtpcApp *app)
{
	g_set_application_name(APP_NAME);
}

/* public Methods */
MtpcApp *mtpc_app_new(void)
{
	return g_object_new(mtpc_app_get_type(),
                            "application-id", "org.gtk.mtpc",
                            "register-session",TRUE, /* gtk_application_inhibit */
                            "flags", 0,
                            NULL);

}
