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


#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>

#include "mtpc-device.h"
#include "glib-utils.h"

#define mtpc_debug(x)

/* Callbacks */

/* Internal */

/* External */
Device *mtpc_device_new(void)
{
	Device *device;

	mtpc_debug("mtpc_device_new\n");

	device = g_new0(Device, 1);

	return device;
}

Device *mtpc_device_new_from_raw_device(LIBMTP_raw_device_t *rawdevice)
{
	Device *device;
	char *tmp;
	guint8 max;
	guint8 cur;

	mtpc_debug("mtpc_device_new_from_raw_device\n");
	device = g_new0(Device, 1);

	/* Raw Device */
	mtpc_debug("raw device uncached\n");
	device->device = LIBMTP_Open_Raw_Device_Uncached(rawdevice);

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
		_g_free(tmp);
		tmp = NULL;
	}

	/* Model */
	mtpc_debug("model name\n");
	tmp = LIBMTP_Get_Modelname(device->device);
	if (tmp == NULL)
		device->model = g_strdup("-");
	else {
		device->model = g_strdup(tmp);
		_g_free(tmp);
		tmp = NULL;
	}

	/* Device Manufacturer */
	mtpc_debug("Device manufacturer\n");
	tmp = LIBMTP_Get_Manufacturername(device->device);
	if (tmp == NULL)
		device->manufacturer = g_strdup("-");
	else {
		device->manufacturer = g_strdup(tmp);
		_g_free(tmp);
		tmp = NULL;
	}

	/* Serial Number */
	mtpc_debug ("serial number\n");
	tmp = LIBMTP_Get_Serialnumber(device->device);
	if (tmp == NULL)
		device->serialnum = g_strdup("-");
	else {
		device->serialnum = g_strdup(tmp);
		_g_free(tmp);
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
	mtpc_debug("mtpc_device_destroy\n");
	g_return_if_fail(device != NULL);

	_g_free(device->devname);
	_g_free(device->model);
	_g_free(device->manufacturer);
	_g_free(device->serialnum);

	_g_free(device->filetypes);
	device->filetypes_len = 0;

	LIBMTP_Release_Device(device->device);
	device->device = NULL;

	device->opened = FALSE;

	device->connected = FALSE;

	_g_free(device);
}

/* libmtp wrappers */
void mtpc_device_libmtp_init(void)
{
	mtpc_debug("mtpc_device_libmtp_init\n");
        LIBMTP_Init();
}

libmtp_dev_t *mtpc_device_alloc_devices(void)
{
        libmtp_dev_t *devices;

	mtpc_debug("mtpc_device_alloc_devices\n");

        devices = malloc(sizeof (*devices));
        if (!devices) {
                fprintf(stderr, "Memory allocation failure!!!\n");
                exit(EXIT_FAILURE);
        }

        memset(devices, 0, sizeof(*devices));

        return devices;
}

void mtpc_device_free_devices (libmtp_dev_t *devices)
{
	mtpc_debug("mtpc_device_free_devices\n");

        if (devices) {
                if (devices->rawdevices) {
                        free(devices->rawdevices);
                        devices->numrawdevices = 0;
                }
                free(devices);
        }

        return;
}

libmtp_err_t mtpc_device_detect_devices(libmtp_dev_t *devices)
{
        libmtp_err_t err;

	mtpc_debug("mtpc_device_detect_devices\n");

        err = LIBMTP_Detect_Raw_Devices(&devices->rawdevices,
					&devices->numrawdevices);

        return err;
}
