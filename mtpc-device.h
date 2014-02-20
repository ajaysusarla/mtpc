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
#ifndef MTPC_DEVICE_H
#define MTPC_DEVICE_H

#include <glib.h>
#include <libmtp.h>

/* libmtp wrappers */

/* Errors */
#define MTP_DEVICE_ERROR_PARSE  1
#define MTP_DEVICE_ERROR_DETECT 2

typedef LIBMTP_error_number_t libmtp_err_t; /* For convenience */

typedef struct _libmtp_dev {
        LIBMTP_raw_device_t *rawdevices;
        int numrawdevices;
} libmtp_dev_t;


/* internal Device structure */
typedef struct {
        gchar *devname;
        gchar *model;
        gchar *manufacturer;
        gchar *serialnum;

        guint32 vid;
        guint32 pid;

        LIBMTP_devicestorage_t *primary_storage;
        gint storageid;
        gint storage_count;

        uint16_t *filetypes;
        uint16_t filetypes_len;

        guint8 maxbatt;
        guint8 curbatt;

        guint32 current_folder;
        gboolean opened;
        gboolean connected;

        LIBMTP_file_t *files;

        LIBMTP_mtpdevice_t *device;
} Device;

Device *mtpc_device_new(void);
void mtpc_device_destroy(Device *device);
Device *mtpc_device_new_from_raw_device(LIBMTP_raw_device_t *device);
void mtpc_device_add(Device *device);
GList *mtpc_device_get_list(void);


/* libmtp wrapper functions */
void mtpc_device_libmtp_init (void);
libmtp_dev_t *mtpc_device_alloc_devices (void);
void mtpc_device_free_devices (libmtp_dev_t *devices);
libmtp_err_t mtpc_device_detect_devices (libmtp_dev_t *devices);


#endif /* MTPC_DEVICE_H */
