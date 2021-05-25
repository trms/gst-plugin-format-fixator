#include <gst/gst.h>
#include <gst/video/video.h>
#include <gtest/gtest.h>
#include "decklink_formats.h"

TEST(basic_test, test_init)
{
  GstElement *fixator =
      gst_element_factory_make ("videoformatfixator", nullptr);

  ASSERT_NE (fixator, nullptr);
  gst_object_unref (fixator);
}

TEST(basic_test, format_change)
{
  GstElement *pipeline = gst_pipeline_new (nullptr);
  GstElement *fixator =
      gst_element_factory_make ("videoformatfixator", nullptr);
  GstElement *sink = gst_element_factory_make ("fakesink", nullptr);
  gboolean ret;
  GstCaps *caps;
  GstPad *sinkpad;
  GstStateChangeReturn state_ret;

  gst_bin_add_many (GST_BIN (pipeline), fixator, sink, nullptr);
  ret = gst_element_link (fixator, sink);
  ASSERT_NE (ret, FALSE);

  sinkpad = gst_element_get_static_pad (fixator, "sink");
  ASSERT_NE (sinkpad, nullptr);

  caps = gst_decklink_mode_get_template_caps (TRUE);
  ASSERT_NE (caps, nullptr);

  guint caps_size = gst_caps_get_size (caps);
  for (guint i = 0; i < caps_size; i++) {
    GstCaps *first_caps = gst_caps_new_empty ();
    GstStructure *s;
    GstVideoInfo first_info;
    gchar *first_caps_str;
    gchar *caps_str;
    GstClockTime first_duration;

    s = gst_caps_get_structure (caps, i);
    gst_caps_append_structure (first_caps, gst_structure_copy (s));

    first_caps_str = gst_structure_to_string (s);

    ret = gst_video_info_from_caps (&first_info, first_caps);
    ASSERT_NE (ret, FALSE);

    first_duration = gst_util_uint64_scale (GST_SECOND,
        first_info.fps_d, first_info.fps_n);

    for (guint j = 0; j < caps_size; j++) {
      GstCaps * second_caps = gst_caps_new_empty ();
      GstVideoInfo second_info;
      GstSegment segment;
      GstBuffer *buffer;
      GstBuffer *other_buffer;
      GstClockTime second_duration;
      GstFlowReturn flow_ret;
      GstEvent *stored_caps_event;
      GstCaps *stored_caps;
      GstSample *sample = NULL;
      GstCaps *other_caps;

      s = gst_caps_get_structure (caps, j);
      gst_caps_append_structure (second_caps, gst_structure_copy (s));

      caps_str = gst_structure_to_string (s);
      gst_println ("[%d-%d] First caps: %s", i, j, first_caps_str);
      gst_println ("[%d-%d] Second caps: %s", i, j, caps_str);
      g_free (caps_str);

      ret = gst_video_info_from_caps (&second_info, second_caps);
      ASSERT_NE (ret, FALSE);

      second_duration = gst_util_uint64_scale (GST_SECOND,
        second_info.fps_d, second_info.fps_n);

      state_ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
      ASSERT_NE (state_ret, GST_STATE_CHANGE_FAILURE);

      ret = gst_pad_send_event (sinkpad, gst_event_new_stream_start ("foo/bar"));
      ASSERT_NE (ret, FALSE);

      ret = gst_pad_send_event (sinkpad, gst_event_new_caps (first_caps));
      ASSERT_NE (ret, FALSE);

      gst_segment_init (&segment, GST_FORMAT_TIME);
      ret = gst_pad_send_event (sinkpad, gst_event_new_segment (&segment));
      ASSERT_NE (ret, FALSE);

      gst_println ("[%d-%d] Sending first buffer", i, j);

      /* Send the first buffer */
      buffer = gst_buffer_new_and_alloc (GST_VIDEO_INFO_SIZE (&first_info));
      ASSERT_NE (buffer, nullptr);
      GST_BUFFER_PTS (buffer) = 0;
      GST_BUFFER_DTS (buffer) = GST_CLOCK_TIME_NONE;
      GST_BUFFER_DURATION (buffer) = first_duration;

      flow_ret = gst_pad_chain (sinkpad, buffer);
      ASSERT_EQ (flow_ret, GST_FLOW_OK);

      /* Check output, this should be identical to the one we pushed */
      g_object_get (sink, "last-sample", &sample, nullptr);
      ASSERT_NE (sample, nullptr);
      other_buffer = gst_sample_get_buffer (sample);
      ASSERT_NE (other_buffer, nullptr);

      other_caps = gst_sample_get_caps (sample);
      ASSERT_NE (other_caps, nullptr);

      ASSERT_EQ (gst_buffer_get_size (other_buffer),
          GST_VIDEO_INFO_SIZE (&first_info));
      ret = gst_caps_can_intersect (other_caps, first_caps);
      s = gst_caps_get_structure (other_caps, 0);
      caps_str = gst_structure_to_string (s);
      gst_println ("[%d-%d] first output caps: %s", i, j, caps_str);
      g_free (caps_str);

      ASSERT_NE (ret, FALSE);

      ASSERT_EQ (GST_BUFFER_PTS (other_buffer), 0);
      ASSERT_EQ (GST_BUFFER_DTS (other_buffer), GST_CLOCK_TIME_NONE);
      ASSERT_EQ (GST_BUFFER_DURATION (other_buffer), first_duration);

      gst_sample_unref (sample);
      sample = nullptr;

      /* Send the second caps and buffer */
      ret = gst_pad_send_event (sinkpad, gst_event_new_caps (second_caps));
      ASSERT_NE (ret, FALSE);

      stored_caps_event = gst_pad_get_sticky_event (sinkpad, GST_EVENT_CAPS, 0);
      ASSERT_NE (stored_caps_event, nullptr);

      gst_event_parse_caps (stored_caps_event, &stored_caps);
      ret = gst_caps_is_equal (second_caps, stored_caps);
      ASSERT_NE (ret, FALSE);

      gst_event_unref (stored_caps_event);

      gst_println ("[%d-%d] Sending second buffer", i, j);
      buffer = gst_buffer_new_and_alloc (GST_VIDEO_INFO_SIZE (&second_info));
      ASSERT_NE (buffer, nullptr);
      GST_BUFFER_PTS (buffer) = first_duration;
      GST_BUFFER_DTS (buffer) = GST_CLOCK_TIME_NONE;
      GST_BUFFER_DURATION (buffer) = second_duration;

      flow_ret = gst_pad_chain (sinkpad, buffer);
      ASSERT_EQ (flow_ret, GST_FLOW_OK);

      /* Check this again */
      g_object_get (sink, "last-sample", &sample, nullptr);
      ASSERT_NE (sample, nullptr);
      other_buffer = gst_sample_get_buffer (sample);
      ASSERT_NE (other_buffer, nullptr);

      other_caps = gst_sample_get_caps (sample);
      ASSERT_NE (other_caps, nullptr);
      other_caps = gst_caps_copy (other_caps);
      /* framerate could might be different */
      gst_caps_set_simple (other_caps, "framerate", GST_TYPE_FRACTION,
          first_info.fps_n, first_info.fps_d, nullptr);

      /* size shouldn't be changed */
      ASSERT_EQ (gst_buffer_get_size (other_buffer),
          GST_VIDEO_INFO_SIZE (&first_info));
      ret = gst_caps_can_intersect (other_caps, first_caps);
      s = gst_caps_get_structure (other_caps, 0);
      caps_str = gst_structure_to_string (s);
      gst_println ("[%d-%d] second output caps: %s", i, j, caps_str);
      g_free (caps_str);
      gst_caps_unref (other_caps);

      ASSERT_NE (ret, FALSE);

      gst_sample_unref (sample);
      sample = nullptr;

      state_ret = gst_element_set_state (pipeline, GST_STATE_NULL);
      ASSERT_NE (state_ret, GST_STATE_CHANGE_FAILURE);
    }

    g_free (first_caps_str);
  }

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (sinkpad);
  gst_object_unref (pipeline);
  gst_caps_unref (caps);
}

int
main(int argc, char **argv)
{
  int ret;

  ::testing::InitGoogleTest(&argc, argv);
  gst_init (nullptr, nullptr);

  ret = RUN_ALL_TESTS();
  gst_deinit ();

  return ret;
}