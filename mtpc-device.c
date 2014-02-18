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

#include <gdk/gdk.h>
#include "mtpc-device.h"

#define mtpc_debug(x)

static GList *device_list = NULL;

/* Callbacks */

/* Internal */

/* External */
Device *mtpc_device_new(void)
{
	Device *device;

	device = g_new0(Device, 1);

	return device;
}

Device *mtpc_device_new_from_raw_device(LIBMTP_raw_device_t *rawdevice)
{
	Device *device;
	LIBMTP_devicestorage_t *storage;
	char *tmp;
	guint8 max;
	guint8 cur;
	int storage_num = 0;

	device = g_new0(Device, 1);

	/* Raw Device */
	mtpc_debug("raw device uncached\n");
	device->device = LIBMTP_Open_Raw_Device_Uncached(rawdevice);

	/* update storage */
	mtpc_debug("storage \n");
	LIBMTP_Get_Storage(device->device, LIBMTP_STORAGE_SORTBY_NOTSORTED);
	device->primary_storage = device->device->storage;
	for (storage = device->device->storage; storage != 0; storage = storage->next)
		storage_num++;
	device->storage_count = storage_num;

	/* vid & pid */
	mtpc_debug("vid and pid\n");
	device->vid = rawdevice->device_entry.vendor_id;
	device->pid = rawdevice->device_entry.product_id;

	/* Friendly name*/
	mtpc_debug("friendly name\n");
	tmp = LIBMTP_Get_Friendlyname(device->device);
	if (tmp == NULL)
		device->devname = g_strdup("-");
	else {
		device->devname = g_strdup(tmp);
		g_free(tmp);
		tmp = NULL;
	}

	/* Model */
	mtpc_debug("model name\n");
	tmp = LIBMTP_Get_Modelname(device->device);
	if (tmp == NULL)
		device->model = g_strdup("-");
	else {
		device->model = g_strdup(tmp);
		g_free(tmp);
		tmp = NULL;
	}

	/* Device Manufacturer */
	mtpc_debug("Device manufacturer\n");
	tmp = LIBMTP_Get_Manufacturername(device->device);
	if (tmp == NULL)
		device->manufacturer = g_strdup("-");
	else {
		device->manufacturer = g_strdup(tmp);
		g_free (tmp);
		tmp = NULL;
	}

	/* Serial Number */
	mtpc_debug ("serial number\n");
	tmp = LIBMTP_Get_Serialnumber(device->device);
	if (tmp == NULL)
		device->serialnum = g_strdup("-");
	else {
		device->serialnum = g_strdup(tmp);
		g_free(tmp);
		tmp = NULL;
	}

	/* Supported Filetypes */
	mtpc_debug("supported filetypes\n");
	if (LIBMTP_Get_Supported_Filetypes(device->device,
					   &device->filetypes,
					   &device->filetypes_len) != 0) {
		LIBMTP_Dump_Errorstack(device->device);
		LIBMTP_Clear_Errorstack(device->device);

		device->filetypes = NULL;
		device->filetypes_len = 0;
	}

	/* Battery Level, if provided */
	mtpc_debug("battery level\n");
	if (LIBMTP_Get_Batterylevel(device->device, &max, &cur) == 0) {
		device->maxbatt = max;
		device->curbatt = cur;
	} else {
		device->maxbatt = 0;
		device->curbatt = 0;
	}

	/* Device is opened */
	device->opened = TRUE;

	return device;
}

void mtpc_device_destroy(Device *device)
{
	printf("device_destroy...");
	g_return_if_fail(device != NULL);


	printf("..");
	g_free(device->devname);
	printf("..");
	g_free(device->model);
	printf("..");
	g_free(device->manufacturer);
	printf("..");
	g_free(device->serialnum);
	printf("..");

	g_free(device->filetypes);
	device->filetypes_len = 0;
	printf("..");

	device->primary_storage = NULL;
	device->storage_count = 0;
	printf("..");

	LIBMTP_Release_Device(device->device);
	printf("..");

	device->opened = FALSE;
	printf("..");

	device->connected = FALSE;
	printf("..");

	device_list = g_list_remove(device_list, device);
	printf("..");

	g_free(device);
	printf("..\n");
}

void mtpc_device_add(Device *device)
{
	g_return_if_fail(device != NULL);

	device_list = g_list_insert(device_list, device, -1);
}

GList *mtpc_device_get_list(void)
{
	return device_list;
}

