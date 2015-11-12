/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2015 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/**
 * SECTION:dfu-image
 * @short_description: Object representing a binary image
 *
 * This object represents an binary blob of data in a DFU file.
 */

#include "config.h"

#include <fwupd.h>
#include <string.h>
#include <stdio.h>

#include "dfu-common.h"
#include "dfu-image.h"

static void dfu_image_finalize			 (GObject *object);

/**
 * DfuImagePrivate:
 *
 * Private #DfuImage data
 **/
typedef struct {
	GBytes			*contents;
	gchar			*name;
	guint32			 target_size;
	guint8			 alt_setting;
} DfuImagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DfuImage, dfu_image, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (dfu_image_get_instance_private (o))

/**
 * dfu_image_class_init:
 **/
static void
dfu_image_class_init (DfuImageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = dfu_image_finalize;
}

/**
 * dfu_image_init:
 **/
static void
dfu_image_init (DfuImage *image)
{
}

/**
 * dfu_image_finalize:
 **/
static void
dfu_image_finalize (GObject *object)
{
	DfuImage *image = DFU_IMAGE (object);
	DfuImagePrivate *priv = GET_PRIVATE (image);

	g_free (priv->name);
	if (priv->contents != NULL)
		g_bytes_unref (priv->contents);

	G_OBJECT_CLASS (dfu_image_parent_class)->finalize (object);
}

/**
 * dfu_image_new:
 *
 * Creates a new DFU image object.
 *
 * Return value: a new #DfuImage
 *
 * Since: 0.5.4
 **/
DfuImage *
dfu_image_new (void)
{
	DfuImage *image;
	image = g_object_new (DFU_TYPE_IMAGE, NULL);
	return image;
}

/**
 * dfu_image_get_contents:
 * @image: a #DfuImage
 *
 * Gets the image data.
 *
 * Return value: (transfer none): image data
 *
 * Since: 0.5.4
 **/
GBytes *
dfu_image_get_contents (DfuImage *image)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_val_if_fail (DFU_IS_IMAGE (image), NULL);
	return priv->contents;
}

/**
 * dfu_image_get_alt_setting:
 * @image: a #DfuImage
 *
 * Gets the alternate setting.
 *
 * Return value: integer, or 0x00 for unset
 *
 * Since: 0.5.4
 **/
guint8
dfu_image_get_alt_setting (DfuImage *image)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_val_if_fail (DFU_IS_IMAGE (image), 0xff);
	return priv->alt_setting;
}

/**
 * dfu_image_get_name:
 * @image: a #DfuImage
 *
 * Gets the target name.
 *
 * Return value: a string, or %NULL for unset
 *
 * Since: 0.5.4
 **/
const gchar *
dfu_image_get_name (DfuImage *image)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_val_if_fail (DFU_IS_IMAGE (image), NULL);
	return priv->name;
}

/**
 * dfu_image_set_contents:
 * @image: a #DfuImage
 * @contents: image data
 *
 * Sets the image data.
 *
 * Since: 0.5.4
 **/
void
dfu_image_set_contents (DfuImage *image, GBytes *contents)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);

	g_return_if_fail (DFU_IS_IMAGE (image));
	g_return_if_fail (contents != NULL);

	if (priv->contents == contents)
		return;
	if (priv->contents != NULL)
		g_bytes_unref (priv->contents);

	/* no need to pad */
	if (priv->target_size == 0 ||
	    g_bytes_get_size (contents) >= priv->target_size) {
		priv->contents = g_bytes_ref (contents);
	} else {
		const guint8 *data;
		gsize length;
		guint8 *buf;
		g_autoptr(GBytes) contents_padded = NULL;

		/* reallocate and copy */
		data = g_bytes_get_data (contents, &length);
		buf = g_malloc0 (priv->target_size);
		memcpy (buf, data, length);
		priv->contents = g_bytes_new_take (buf, priv->target_size);
	}
}

/**
 * dfu_image_set_alt_setting:
 * @image: a #DfuImage
 * @alt_setting: vendor ID, or 0xffff for unset
 *
 * Sets the vendor ID.
 *
 * Since: 0.5.4
 **/
void
dfu_image_set_alt_setting (DfuImage *image, guint8 alt_setting)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_if_fail (DFU_IS_IMAGE (image));
	priv->alt_setting = alt_setting;
}

/**
 * dfu_image_set_name:
 * @image: a #DfuImage
 * @name: a target string, or %NULL
 *
 * Sets the target name.
 *
 * Since: 0.5.4
 **/
void
dfu_image_set_name (DfuImage *image, const gchar *name)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_if_fail (DFU_IS_IMAGE (image));
	g_free (priv->name);
	priv->name = g_strdup (name);
}

/**
 * dfu_image_to_string:
 * @image: a #DfuImage
 *
 * Returns a string representaiton of the object.
 *
 * Return value: NULL terminated string, or %NULL for invalid
 *
 * Since: 0.5.4
 **/
gchar *
dfu_image_to_string (DfuImage *image)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	GString *str;
	gsize length;
	const guint8 *data;

	g_return_val_if_fail (DFU_IS_IMAGE (image), NULL);

	str = g_string_new ("");
	g_string_append_printf (str, "alt_setting: 0x%02x\n", priv->alt_setting);
	g_string_append_printf (str, "name:        %s\n", priv->name);
	if (priv->target_size > 0)
		g_string_append_printf (str, "target:      0x%04x\n", priv->target_size);
	if (priv->contents != NULL) {
		data = g_bytes_get_data (priv->contents, &length);
		g_string_append_printf (str, "contents:    0x%04x\n", (guint32) length);
	}

	g_string_truncate (str, str->len - 1);
	return g_string_free (str, FALSE);
}

/**
 * dfu_image_set_target_size:
 * @image: a #DfuImage
 * @target_size: size in bytes
 *
 * Sets a target size for the image. If the prepared image is smaller
 * than this then it will be padded with NUL bytes up to the required size.
 *
 * Since: 0.5.4
 **/
void
dfu_image_set_target_size (DfuImage *image, guint32 target_size)
{
	DfuImagePrivate *priv = GET_PRIVATE (image);
	g_return_if_fail (DFU_IS_IMAGE (image));
	priv->target_size = target_size;
}
