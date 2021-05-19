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

#include <gst/gst.h>
#include <gst/video/video.h>
#include "gstvideoformatfixator.h"

GST_DEBUG_CATEGORY (gst_video_format_fixator_debug);
#define GST_CAT_DEFAULT gst_video_format_fixator_debug

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL))
    );

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL))
    );

enum
{
  PROP_0,
  PROP_INITIAL_CAPS,
  PROP_PASSTHROUGH,
  PROP_LAST,
};

#define DEFAULT_PASSTHROUGH FALSE

static GParamSpec *properties[PROP_LAST];

typedef enum
{
  /* Initial status */
  MODE_PASSTHROUGH,
  MODE_INTERLACE,
  MODE_SCALE_INTERLACE,
  MODE_DEINTERLACE
} FixatorMode;

typedef struct
{
  GstElement *bin;

  GstPad *sinkpad;
  GstPad *srcpad;

  FixatorMode mode;
} ProcSlot;

struct _GstVideoFormatFixator
{
  GstBin element;

  GstPad *sinkpad;
  GstPad *srcpad;

  GstElement *capsfilter_front;
  GstElement *capsfilter_rear;
  GstElement *convert_front;
  GstElement *convert_rear;

  ProcSlot *current_slot;

  gboolean got_buffer;
  GstCaps *initial_caps;

  GstVideoInfo target_info;

  gboolean passthrough;
};

static void gst_video_format_fixator_dispose (GObject * object);
static void gst_video_format_fixator_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_video_format_fixator_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);

static gboolean gst_video_format_fixator_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_video_format_fixator_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buffer);
static ProcSlot *gst_video_format_fixator_slot_new (GstVideoFormatFixator *
    self, FixatorMode mode);

#define gst_video_format_fixator_parent_class parent_class
G_DEFINE_TYPE (GstVideoFormatFixator, gst_video_format_fixator, GST_TYPE_BIN);

static void
gst_video_format_fixator_class_init (GstVideoFormatFixatorClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->dispose = gst_video_format_fixator_dispose;
  gobject_class->get_property = gst_video_format_fixator_get_property;
  gobject_class->set_property = gst_video_format_fixator_set_property;

  properties[PROP_INITIAL_CAPS] =
      g_param_spec_boxed ("initial-caps", "Initial Caps",
          "Initial caps", GST_TYPE_CAPS,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  properties[PROP_PASSTHROUGH] =
      g_param_spec_boolean ("passthrough", "Passthrough",
          "Enable passthrough", DEFAULT_PASSTHROUGH,
          G_PARAM_READWRITE | GST_PARAM_MUTABLE_READY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, PROP_LAST, properties);

  gst_element_class_add_static_pad_template (element_class, &sink_template);
  gst_element_class_add_static_pad_template (element_class, &src_template);

  gst_element_class_set_static_metadata (element_class,
      "Video Format Fixator",
      "Filter/Effect/Video",
      "Fixate Video Format", "Seungha Yang <seungha@centricular.com>");

  GST_DEBUG_CATEGORY_INIT (gst_video_format_fixator_debug,
      "videoformatfixator", 0, "videoformatfixator");
}

#define MAKE_ELEMENT_AND_ADD(bin,ptr,factory) G_STMT_START {\
    ptr = gst_element_factory_make (factory, NULL); \
    if (!ptr) { \
      GST_ERROR ("Missing plugin %s", factory); \
      goto error; \
    } \
    gst_bin_add (GST_BIN_CAST (bin), ptr); \
} G_STMT_END

static void
gst_video_format_fixator_init (GstVideoFormatFixator * self)
{
  GstPad *pad;
  GstCaps *caps;

  MAKE_ELEMENT_AND_ADD (self, self->capsfilter_front, "capsfilter");
  MAKE_ELEMENT_AND_ADD (self, self->capsfilter_rear, "capsfilter");
  MAKE_ELEMENT_AND_ADD (self, self->convert_front, "videoconvert");
  MAKE_ELEMENT_AND_ADD (self, self->convert_rear, "videoconvert");

  self->current_slot = gst_video_format_fixator_slot_new (self,
      MODE_PASSTHROUGH);

  gst_bin_add (GST_BIN_CAST (self), self->current_slot->bin);

  /* FIXME: support hardware memory */
  caps = gst_caps_from_string ("video/x-raw");
  g_object_set (self->capsfilter_front, "caps", caps, NULL);
  g_object_set (self->capsfilter_rear, "caps", caps, NULL);
  gst_caps_unref (caps);

  gst_element_link_many (self->capsfilter_front,
      self->convert_front, self->current_slot->bin,
      self->convert_rear, self->capsfilter_rear, NULL);

  pad = gst_element_get_static_pad (self->capsfilter_front, "sink");
  self->sinkpad = gst_ghost_pad_new ("sink", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (self), self->sinkpad);

  pad = gst_element_get_static_pad (self->capsfilter_rear, "src");
  self->srcpad = gst_ghost_pad_new ("src", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (self), self->srcpad);

  gst_pad_set_event_function (self->sinkpad,
      gst_video_format_fixator_sink_event);
  gst_pad_set_chain_function (self->sinkpad, gst_video_format_fixator_chain);

  self->initial_caps = gst_caps_new_any ();
  self->passthrough = DEFAULT_PASSTHROUGH;

  return;

error:
  GST_ERROR_OBJECT (self, "Couldn't configure internal elements");
}

static void
gst_video_format_fixator_dispose (GObject * object)
{
  GstVideoFormatFixator *self = GST_VIDEO_FORMAT_FIXATOR (object);

  gst_clear_caps (&self->initial_caps);
  g_clear_pointer (&self->current_slot, g_free);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_video_format_fixator_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstVideoFormatFixator *self = GST_VIDEO_FORMAT_FIXATOR (object);

  switch (prop_id) {
    case PROP_INITIAL_CAPS:
      GST_OBJECT_LOCK (self);
      gst_value_set_caps (value, self->initial_caps);
      GST_OBJECT_UNLOCK (self);
      break;
    case PROP_PASSTHROUGH:
      g_value_set_boolean (value, self->passthrough);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_video_format_fixator_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVideoFormatFixator *self = GST_VIDEO_FORMAT_FIXATOR (object);

  switch (prop_id) {
    case PROP_PASSTHROUGH:
      self->passthrough = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static ProcSlot *
create_passthrough_slot (GstVideoFormatFixator * self)
{
  ProcSlot *slot = g_new0 (ProcSlot, 1);
  GstElement *scale;
  GstPad *pad;

  slot->bin = gst_bin_new (NULL);
  slot->mode = MODE_PASSTHROUGH;
  MAKE_ELEMENT_AND_ADD (slot->bin, scale, "videoscale");

  pad = gst_element_get_static_pad (scale, "sink");
  slot->sinkpad = gst_ghost_pad_new ("sink", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->sinkpad);

  pad = gst_element_get_static_pad (scale, "src");
  slot->srcpad = gst_ghost_pad_new ("src", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->srcpad);

  return slot;

error:
  gst_object_unref (slot->bin);
  g_free (slot);

  return NULL;
}

static ProcSlot *
create_interlace_slot (GstVideoFormatFixator * self)
{
  ProcSlot *slot = g_new0 (ProcSlot, 1);
  GstElement *scale;
  GstElement *capsfilter;
  GstElement *interlace;
  GstPad *pad;
  GstVideoInfo *target_info = &self->target_info;
  GstCaps *caps;
  gboolean tff;

  slot->bin = gst_bin_new (NULL);
  slot->mode = MODE_INTERLACE;
  MAKE_ELEMENT_AND_ADD (slot->bin, scale, "videoscale");
  MAKE_ELEMENT_AND_ADD (slot->bin, capsfilter, "capsfilter");
  MAKE_ELEMENT_AND_ADD (slot->bin, interlace, "interlace");

  caps = gst_caps_new_simple ("video/x-raw", "width", G_TYPE_INT,
      target_info->width, "height", G_TYPE_INT, target_info->height,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, target_info->par_n,
      target_info->par_d, NULL);
  g_object_set (capsfilter, "caps", caps, NULL);
  gst_caps_unref (caps);

  /* FIXME: which interlace mode?? */
  tff = GST_VIDEO_INFO_FIELD_ORDER (target_info) ==
      GST_VIDEO_FIELD_ORDER_TOP_FIELD_FIRST;
  g_object_set (interlace, "field-pattern", 0, "top-field-first", tff, NULL);

  if (!gst_element_link_many (scale, capsfilter, interlace, NULL)) {
    GST_ERROR_OBJECT (self, "Failed to link elements");
    goto error;
  }

  pad = gst_element_get_static_pad (scale, "sink");
  slot->sinkpad = gst_ghost_pad_new ("sink", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->sinkpad);

  pad = gst_element_get_static_pad (interlace, "src");
  slot->srcpad = gst_ghost_pad_new ("src", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->srcpad);

  return slot;

error:
  gst_object_unref (slot->bin);
  g_free (slot);

  return NULL;
}

static ProcSlot *
create_scale_interlace_slot (GstVideoFormatFixator * self)
{
  ProcSlot *slot = g_new0 (ProcSlot, 1);
  GstElement *deinterlace;
  GstElement *convert;
  GstElement *scale;
  GstElement *capsfilter;
  GstElement *interlace;
  GstPad *pad;
  GstVideoInfo *target_info = &self->target_info;
  GstCaps *caps;
  gboolean tff;

  slot->bin = gst_bin_new (NULL);
  slot->mode = MODE_SCALE_INTERLACE;
  MAKE_ELEMENT_AND_ADD (slot->bin, deinterlace, "deinterlace");
  MAKE_ELEMENT_AND_ADD (slot->bin, convert, "videoconvert");
  MAKE_ELEMENT_AND_ADD (slot->bin, scale, "videoscale");
  MAKE_ELEMENT_AND_ADD (slot->bin, capsfilter, "capsfilter");
  MAKE_ELEMENT_AND_ADD (slot->bin, interlace, "interlace");

  caps = gst_caps_new_simple ("video/x-raw", "width", G_TYPE_INT,
      target_info->width, "height", G_TYPE_INT, target_info->height,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, target_info->par_n,
      target_info->par_d, "interlace-mode", G_TYPE_STRING, "progressive", NULL);
  g_object_set (capsfilter, "caps", caps, NULL);
  gst_caps_unref (caps);

  /* FIXME: which deinterlace method ?? */
  g_object_set (deinterlace, "method", 10, NULL);

  /* FIXME: which interlace mode?? */
  tff = GST_VIDEO_INFO_FIELD_ORDER (target_info) ==
      GST_VIDEO_FIELD_ORDER_TOP_FIELD_FIRST;
  g_object_set (interlace, "field-pattern", 0, "top-field-first", tff, NULL);

  if (!gst_element_link_many (deinterlace, convert, scale, capsfilter,
          interlace, NULL)) {
    GST_ERROR_OBJECT (self, "Failed to link elements");
    goto error;
  }

  pad = gst_element_get_static_pad (deinterlace, "sink");
  slot->sinkpad = gst_ghost_pad_new ("sink", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->sinkpad);

  pad = gst_element_get_static_pad (interlace, "src");
  slot->srcpad = gst_ghost_pad_new ("src", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->srcpad);

  return slot;

error:
  gst_object_unref (slot->bin);
  g_free (slot);

  return NULL;
}

static ProcSlot *
create_deinterlace_slot (GstVideoFormatFixator * self)
{
  ProcSlot *slot = g_new0 (ProcSlot, 1);
  GstElement *deinterlace;
  GstElement *scale;
  GstPad *pad;

  slot->bin = gst_bin_new (NULL);
  slot->mode = MODE_DEINTERLACE;
  MAKE_ELEMENT_AND_ADD (slot->bin, deinterlace, "deinterlace");
  MAKE_ELEMENT_AND_ADD (slot->bin, scale, "videoscale");

  /* FIXME: which deinterlace method ?? */
  g_object_set (deinterlace, "method", 10, NULL);

  if (!gst_element_link_many (deinterlace, scale, NULL)) {
    GST_ERROR_OBJECT (self, "Failed to link elements");
    goto error;
  }

  pad = gst_element_get_static_pad (deinterlace, "sink");
  slot->sinkpad = gst_ghost_pad_new ("sink", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->sinkpad);

  pad = gst_element_get_static_pad (scale, "src");
  slot->srcpad = gst_ghost_pad_new ("src", pad);
  gst_object_unref (pad);
  gst_element_add_pad (GST_ELEMENT_CAST (slot->bin), slot->srcpad);

  return slot;

error:
  gst_object_unref (slot->bin);
  g_free (slot);

  return NULL;
}

static ProcSlot *
gst_video_format_fixator_slot_new (GstVideoFormatFixator * self,
    FixatorMode mode)
{
  switch (mode) {
    case MODE_PASSTHROUGH:
      return create_passthrough_slot (self);
    case MODE_INTERLACE:
      return create_interlace_slot (self);
    case MODE_SCALE_INTERLACE:
      return create_scale_interlace_slot (self);
    case MODE_DEINTERLACE:
      return create_deinterlace_slot (self);
    default:
      g_assert_not_reached ();
      return NULL;
  }
}

static void
gst_video_format_fixator_fixate_target_format (GstVideoFormatFixator * self,
    GstBuffer * buffer)
{
  GstVideoInfo *info = &self->target_info;

  gst_video_info_from_caps (info, self->initial_caps);

  if (GST_VIDEO_INFO_IS_INTERLACED (info)) {
    if (GST_VIDEO_INFO_FIELD_ORDER (info) == GST_VIDEO_FIELD_ORDER_UNKNOWN) {
      if (GST_BUFFER_FLAG_IS_SET (buffer, GST_VIDEO_BUFFER_FLAG_TFF)) {
        GST_VIDEO_INFO_FIELD_ORDER (info) =
            GST_VIDEO_FIELD_ORDER_TOP_FIELD_FIRST;
      } else {
        GST_VIDEO_INFO_FIELD_ORDER (info) =
            GST_VIDEO_FIELD_ORDER_BOTTOM_FIELD_FIRST;
      }
    }
  }
}

static GstFlowReturn
gst_video_format_fixator_chain (GstPad * pad, GstObject * parent,
    GstBuffer * buffer)
{
  GstVideoFormatFixator *self = GST_VIDEO_FORMAT_FIXATOR (parent);

  if (self->passthrough)
    return gst_proxy_pad_chain_default (pad, parent, buffer);

  if (!self->got_buffer) {
    GstCaps *caps;
    GstVideoInfo *info = &self->target_info;
    gchar *color;

    gst_video_format_fixator_fixate_target_format (self, buffer);

    caps = gst_caps_new_simple ("video/x-raw",
        "format", G_TYPE_STRING,
        gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (info)),
        "width", G_TYPE_INT, info->width,
        "height", G_TYPE_INT, info->height,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, info->par_n, info->par_d,
        NULL);

    color =
        gst_video_colorimetry_to_string (&GST_VIDEO_INFO_COLORIMETRY (info));
    if (color) {
      gst_caps_set_simple (caps, "colorimetry", G_TYPE_STRING, color, NULL);
      g_free (color);
    }

    if (GST_VIDEO_INFO_IS_INTERLACED (info)) {
      GValue mode_list = G_VALUE_INIT;
      GValue mode = G_VALUE_INIT;

      g_value_init (&mode_list, GST_TYPE_LIST);
      g_value_init (&mode, G_TYPE_STRING);
      g_value_set_static_string (&mode,
          gst_video_interlace_mode_to_string
          (GST_VIDEO_INTERLACE_MODE_INTERLEAVED));
      gst_value_list_append_and_take_value (&mode_list, &mode);

      g_value_init (&mode, G_TYPE_STRING);
      g_value_set_static_string (&mode,
          gst_video_interlace_mode_to_string (GST_VIDEO_INTERLACE_MODE_MIXED));
      gst_value_list_append_and_take_value (&mode_list, &mode);

      gst_caps_set_value (caps, "interlace-mode", &mode_list);
      g_value_unset (&mode_list);

      gst_caps_set_simple (caps, "field-order", G_TYPE_STRING,
          gst_video_field_order_to_string (GST_VIDEO_INFO_FIELD_ORDER (info)),
          NULL);
    } else {
      gst_caps_set_simple (caps, "interlace-mode", G_TYPE_STRING,
          gst_video_interlace_mode_to_string (info->interlace_mode), NULL);
    }

    GST_INFO_OBJECT (self, "Setting target caps %" GST_PTR_FORMAT, caps);
    g_object_set (self->capsfilter_rear, "caps", caps, NULL);
    gst_caps_unref (caps);

    self->got_buffer = TRUE;
  }

  return gst_proxy_pad_chain_default (pad, parent, buffer);
}

static GstPadProbeReturn
eos_drop_probe (GstPad * pad, GstPadProbeInfo * info,
    GstVideoFormatFixator * self)
{
  GstEvent *ev = GST_PAD_PROBE_INFO_EVENT (info);

  if (GST_EVENT_TYPE (ev) != GST_EVENT_EOS)
    return GST_PAD_PROBE_OK;

  GST_DEBUG_OBJECT (self, "Saw EOS on pad %" GST_PTR_FORMAT, pad);
  gst_event_unref (ev);

  return GST_PAD_PROBE_HANDLED;
}

static gboolean
gst_video_format_fixator_switch_slot (GstVideoFormatFixator * self,
    ProcSlot * new_slot, GstCaps * in_caps)
{
  guint block_id = 0;
  GstPad *srcpad = NULL;
  GstPad *sinkpad = NULL;
  ProcSlot *current_slot = self->current_slot;
  GstPadLinkReturn link_ret;

  /* do block while switching slot */
  block_id = gst_pad_add_probe (self->srcpad, GST_PAD_PROBE_TYPE_BLOCK_UPSTREAM,
      NULL, NULL, NULL);

  /* And add probe to drop EOS event */
  gst_pad_add_probe (self->current_slot->srcpad,
      GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, (GstPadProbeCallback) eos_drop_probe,
      self, NULL);

  /* Send EOS event to drain */
  gst_pad_send_event (self->current_slot->sinkpad, gst_event_new_eos ());

  gst_element_set_locked_state (current_slot->bin, TRUE);

  /* Unlink current slot */
  srcpad = gst_element_get_static_pad (self->convert_front, "src");
  sinkpad = gst_element_get_static_pad (self->convert_rear, "sink");

  if (!gst_pad_unlink (srcpad, current_slot->sinkpad)) {
    GST_ERROR_OBJECT (self, "Failed to unlink pads %" GST_PTR_FORMAT " and %"
        GST_PTR_FORMAT, srcpad, current_slot->sinkpad);
    goto error;
  }

  if (!gst_pad_unlink (current_slot->srcpad, sinkpad)) {
    GST_ERROR_OBJECT (self, "Failed to unlink pads %" GST_PTR_FORMAT " and %"
        GST_PTR_FORMAT, current_slot->srcpad, sinkpad);
    goto error;
  }

  gst_element_set_state (current_slot->bin, GST_STATE_NULL);
  gst_bin_remove (GST_BIN_CAST (self), current_slot->bin);
  g_free (current_slot);

  self->current_slot = new_slot;

  /* And link new slot */
  gst_bin_add (GST_BIN_CAST (self), new_slot->bin);
  if ((link_ret = gst_pad_link (srcpad, new_slot->sinkpad)) != GST_PAD_LINK_OK) {
    GST_ERROR_OBJECT (self, "Failed to link pads %" GST_PTR_FORMAT " and %"
        GST_PTR_FORMAT ", error %s", srcpad, new_slot->sinkpad,
        gst_pad_link_get_name (link_ret));
    goto error;
  }

  if ((link_ret = gst_pad_link (new_slot->srcpad, sinkpad)) != GST_PAD_LINK_OK) {
    GST_ERROR_OBJECT (self, "Failed to link pads %" GST_PTR_FORMAT " and %"
        GST_PTR_FORMAT ", error %s", srcpad, new_slot->sinkpad,
        gst_pad_link_get_name (link_ret));
    goto error;
  }

  gst_pad_remove_probe (self->srcpad, block_id);
  gst_element_sync_state_with_parent (new_slot->bin);

  gst_object_unref (srcpad);
  gst_object_unref (sinkpad);

  return TRUE;

error:
  return FALSE;
}

/* Should be called from streaming thread */
static gboolean
gst_video_format_fixator_reconfigure (GstVideoFormatFixator * self,
    GstCaps * caps)
{
  GstVideoInfo info;
  GstVideoInfo *target_info = &self->target_info;
  FixatorMode mode = MODE_PASSTHROUGH;
  FixatorMode current_mode = self->current_slot->mode;
  ProcSlot *new_slot;

  if (self->passthrough)
    return TRUE;

  if (!gst_video_info_from_caps (&info, caps)) {
    GST_ERROR_OBJECT (self, "Wrong caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }

  if (GST_VIDEO_INFO_IS_INTERLACED (target_info)) {
    if (GST_VIDEO_INFO_IS_INTERLACED (&info)) {
      /* interlaced -> interlaced.
       * XXX: If resolution is changed, we need
       * deinterlace -> scale -> interlace since videoscale assuming progressive
       */
      if (info.width != target_info->width || info.height != target_info->height
          || info.par_n != target_info->par_n
          || info.par_d != target_info->par_d) {
        mode = MODE_SCALE_INTERLACE;
      } else {
        mode = MODE_PASSTHROUGH;
      }
    } else {
      /* progressive -> interlaced */
      mode = MODE_INTERLACE;
    }
  } else {
    if (GST_VIDEO_INFO_IS_INTERLACED (&info)) {
      /* interlaced -> progressive */
      mode = MODE_DEINTERLACE;
    } else {
      /* progressive -> progressive */
      mode = MODE_PASSTHROUGH;
    }
  }

  if (current_mode == mode)
    return TRUE;

  GST_DEBUG_OBJECT (self, "Mode change %d -> %d", current_mode, mode);

  new_slot = gst_video_format_fixator_slot_new (self, mode);

  return gst_video_format_fixator_switch_slot (self, new_slot, caps);
}

static gboolean
gst_video_format_fixator_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstVideoFormatFixator *self = GST_VIDEO_FORMAT_FIXATOR (parent);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      GST_DEBUG_OBJECT (self, "Got %" GST_PTR_FORMAT, caps);

      if (!self->got_buffer) {
        GST_OBJECT_LOCK (self);
        gst_clear_caps (&self->initial_caps);
        self->initial_caps = gst_caps_ref (caps);
        GST_OBJECT_UNLOCK (self);

        g_object_notify_by_pspec (G_OBJECT (self),
            properties[PROP_INITIAL_CAPS]);
      } else {
        if (!gst_video_format_fixator_reconfigure (self, caps)) {
          GST_ELEMENT_ERROR (self, CORE, NEGOTIATION, (NULL),
              ("Failed to reconfigure with new format"));
          return FALSE;
        }
      }
      break;
    }
    default:
      break;
  }

  return gst_pad_event_default (pad, parent, event);
}
