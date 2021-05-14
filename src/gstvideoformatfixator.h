/* GStreamer
 * Copyright (C) 2021 Seungha Yang <seungha@centricular.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __GST_VIDEO_FORMAT_FIXATOR_H__
#define __GST_VIDEO_FORMAT_FIXATOR_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_VIDEO_FORMAT_FIXATOR (gst_video_format_fixator_get_type())
G_DECLARE_FINAL_TYPE (GstVideoFormatFixator, gst_video_format_fixator,
    GST, VIDEO_FORMAT_FIXATOR, GstBin);

G_END_DECLS

#endif /* __GST_VIDEO_FORMAT_FIXATOR_H__ */