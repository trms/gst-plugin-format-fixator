#pragma once

#include <gst/gst.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

typedef /* [v1_enum] */
enum _BMDDisplayMode
    {
        bmdModeNTSC	= 0x6e747363,
        bmdModeNTSC2398	= 0x6e743233,
        bmdModePAL	= 0x70616c20,
        bmdModeNTSCp	= 0x6e747370,
        bmdModePALp	= 0x70616c70,
        bmdModeHD1080p2398	= 0x32337073,
        bmdModeHD1080p24	= 0x32347073,
        bmdModeHD1080p25	= 0x48703235,
        bmdModeHD1080p2997	= 0x48703239,
        bmdModeHD1080p30	= 0x48703330,
        bmdModeHD1080p4795	= 0x48703437,
        bmdModeHD1080p48	= 0x48703438,
        bmdModeHD1080p50	= 0x48703530,
        bmdModeHD1080p5994	= 0x48703539,
        bmdModeHD1080p6000	= 0x48703630,
        bmdModeHD1080p9590	= 0x48703935,
        bmdModeHD1080p96	= 0x48703936,
        bmdModeHD1080p100	= 0x48703130,
        bmdModeHD1080p11988	= 0x48703131,
        bmdModeHD1080p120	= 0x48703132,
        bmdModeHD1080i50	= 0x48693530,
        bmdModeHD1080i5994	= 0x48693539,
        bmdModeHD1080i6000	= 0x48693630,
        bmdModeHD720p50	= 0x68703530,
        bmdModeHD720p5994	= 0x68703539,
        bmdModeHD720p60	= 0x68703630,
        bmdMode2k2398	= 0x326b3233,
        bmdMode2k24	= 0x326b3234,
        bmdMode2k25	= 0x326b3235,
        bmdMode2kDCI2398	= 0x32643233,
        bmdMode2kDCI24	= 0x32643234,
        bmdMode2kDCI25	= 0x32643235,
        bmdMode2kDCI2997	= 0x32643239,
        bmdMode2kDCI30	= 0x32643330,
        bmdMode2kDCI4795	= 0x32643437,
        bmdMode2kDCI48	= 0x32643438,
        bmdMode2kDCI50	= 0x32643530,
        bmdMode2kDCI5994	= 0x32643539,
        bmdMode2kDCI60	= 0x32643630,
        bmdMode2kDCI9590	= 0x32643935,
        bmdMode2kDCI96	= 0x32643936,
        bmdMode2kDCI100	= 0x32643130,
        bmdMode2kDCI11988	= 0x32643131,
        bmdMode2kDCI120	= 0x32643132,
        bmdMode4K2160p2398	= 0x346b3233,
        bmdMode4K2160p24	= 0x346b3234,
        bmdMode4K2160p25	= 0x346b3235,
        bmdMode4K2160p2997	= 0x346b3239,
        bmdMode4K2160p30	= 0x346b3330,
        bmdMode4K2160p4795	= 0x346b3437,
        bmdMode4K2160p48	= 0x346b3438,
        bmdMode4K2160p50	= 0x346b3530,
        bmdMode4K2160p5994	= 0x346b3539,
        bmdMode4K2160p60	= 0x346b3630,
        bmdMode4K2160p9590	= 0x346b3935,
        bmdMode4K2160p96	= 0x346b3936,
        bmdMode4K2160p100	= 0x346b3130,
        bmdMode4K2160p11988	= 0x346b3131,
        bmdMode4K2160p120	= 0x346b3132,
        bmdMode4kDCI2398	= 0x34643233,
        bmdMode4kDCI24	= 0x34643234,
        bmdMode4kDCI25	= 0x34643235,
        bmdMode4kDCI2997	= 0x34643239,
        bmdMode4kDCI30	= 0x34643330,
        bmdMode4kDCI4795	= 0x34643437,
        bmdMode4kDCI48	= 0x34643438,
        bmdMode4kDCI50	= 0x34643530,
        bmdMode4kDCI5994	= 0x34643539,
        bmdMode4kDCI60	= 0x34643630,
        bmdMode4kDCI9590	= 0x34643935,
        bmdMode4kDCI96	= 0x34643936,
        bmdMode4kDCI100	= 0x34643130,
        bmdMode4kDCI11988	= 0x34643131,
        bmdMode4kDCI120	= 0x34643132,
        bmdMode8K4320p2398	= 0x386b3233,
        bmdMode8K4320p24	= 0x386b3234,
        bmdMode8K4320p25	= 0x386b3235,
        bmdMode8K4320p2997	= 0x386b3239,
        bmdMode8K4320p30	= 0x386b3330,
        bmdMode8K4320p4795	= 0x386b3437,
        bmdMode8K4320p48	= 0x386b3438,
        bmdMode8K4320p50	= 0x386b3530,
        bmdMode8K4320p5994	= 0x386b3539,
        bmdMode8K4320p60	= 0x386b3630,
        bmdMode8kDCI2398	= 0x38643233,
        bmdMode8kDCI24	= 0x38643234,
        bmdMode8kDCI25	= 0x38643235,
        bmdMode8kDCI2997	= 0x38643239,
        bmdMode8kDCI30	= 0x38643330,
        bmdMode8kDCI4795	= 0x38643437,
        bmdMode8kDCI48	= 0x38643438,
        bmdMode8kDCI50	= 0x38643530,
        bmdMode8kDCI5994	= 0x38643539,
        bmdMode8kDCI60	= 0x38643630,
        bmdMode640x480p60	= 0x76676136,
        bmdMode800x600p60	= 0x73766736,
        bmdMode1440x900p50	= 0x77786735,
        bmdMode1440x900p60	= 0x77786736,
        bmdMode1440x1080p50	= 0x73786735,
        bmdMode1440x1080p60	= 0x73786736,
        bmdMode1600x1200p50	= 0x75786735,
        bmdMode1600x1200p60	= 0x75786736,
        bmdMode1920x1200p50	= 0x77757835,
        bmdMode1920x1200p60	= 0x77757836,
        bmdMode1920x1440p50	= 0x31393435,
        bmdMode1920x1440p60	= 0x31393436,
        bmdMode2560x1440p50	= 0x77716835,
        bmdMode2560x1440p60	= 0x77716836,
        bmdMode2560x1600p50	= 0x77717835,
        bmdMode2560x1600p60	= 0x77717836,
        bmdModeCintelRAW	= 0x72776369,
        bmdModeCintelCompressedRAW	= 0x72776363,
        bmdModeUnknown	= 0x69756e6b
    } 	BMDDisplayMode;

typedef /* [v1_enum] */
enum _BMDPixelFormat
    {
        bmdFormatUnspecified	= 0,
        bmdFormat8BitYUV	= 0x32767579,
        bmdFormat10BitYUV	= 0x76323130,
        bmdFormat8BitARGB	= 32,
        bmdFormat8BitBGRA	= 0x42475241,
        bmdFormat10BitRGB	= 0x72323130,
        bmdFormat12BitRGB	= 0x52313242,
        bmdFormat12BitRGBLE	= 0x5231324c,
        bmdFormat10BitRGBXLE	= 0x5231306c,
        bmdFormat10BitRGBX	= 0x52313062,
        bmdFormatH265	= 0x68657631,
        bmdFormatDNxHR	= 0x41566468,
        bmdFormat12BitRAWGRBG	= 0x72313270,
        bmdFormat12BitRAWJPEG	= 0x72313670
    } 	BMDPixelFormat;

static const struct
{
  BMDPixelFormat format;
  gint bpp;
  GstVideoFormat vformat;
} formats[] = {
  /* *INDENT-OFF* */
  {bmdFormat8BitYUV, 2, GST_VIDEO_FORMAT_UYVY},  /* auto */
  {bmdFormat8BitYUV, 2, GST_VIDEO_FORMAT_UYVY},
  {bmdFormat10BitYUV, 4, GST_VIDEO_FORMAT_v210},
#if 0
  {bmdFormat8BitARGB, 4, GST_VIDEO_FORMAT_ARGB},
  {bmdFormat8BitBGRA, 4, GST_VIDEO_FORMAT_BGRA},
#endif
/* Not yet supported
  {bmdFormat10BitRGB, FIXME, FIXME},
  {bmdFormat12BitRGB, FIXME, FIXME},
  {bmdFormat12BitRGBLE, FIXME, FIXME},
  {bmdFormat10BitRGBXLE, FIXME, FIXME},
  {bmdFormat10BitRGBX, FIXME, FIXME} */
  /* *INDENT-ON* */
};

typedef enum {
  GST_DECKLINK_MODE_AUTO,

  GST_DECKLINK_MODE_NTSC,
  GST_DECKLINK_MODE_NTSC2398,
  GST_DECKLINK_MODE_PAL,
  GST_DECKLINK_MODE_NTSC_P,
  GST_DECKLINK_MODE_PAL_P,

  GST_DECKLINK_MODE_1080p2398,
  GST_DECKLINK_MODE_1080p24,
  GST_DECKLINK_MODE_1080p25,
  GST_DECKLINK_MODE_1080p2997,
  GST_DECKLINK_MODE_1080p30,

  GST_DECKLINK_MODE_1080i50,
  GST_DECKLINK_MODE_1080i5994,
  GST_DECKLINK_MODE_1080i60,

  GST_DECKLINK_MODE_1080p50,
  GST_DECKLINK_MODE_1080p5994,
  GST_DECKLINK_MODE_1080p60,

  GST_DECKLINK_MODE_720p50,
  GST_DECKLINK_MODE_720p5994,
  GST_DECKLINK_MODE_720p60,

  GST_DECKLINK_MODE_1556p2398,
  GST_DECKLINK_MODE_1556p24,
  GST_DECKLINK_MODE_1556p25,

  GST_DECKLINK_MODE_2KDCI2398,
  GST_DECKLINK_MODE_2KDCI24,
  GST_DECKLINK_MODE_2KDCI25,
  GST_DECKLINK_MODE_2KDCI2997,
  GST_DECKLINK_MODE_2KDCI30,
  GST_DECKLINK_MODE_2KDCI50,
  GST_DECKLINK_MODE_2KDCI5994,
  GST_DECKLINK_MODE_2KDCI60,

  GST_DECKLINK_MODE_2160p2398,
  GST_DECKLINK_MODE_2160p24,
  GST_DECKLINK_MODE_2160p25,
  GST_DECKLINK_MODE_2160p2997,
  GST_DECKLINK_MODE_2160p30,
  GST_DECKLINK_MODE_2160p50,
  GST_DECKLINK_MODE_2160p5994,
  GST_DECKLINK_MODE_2160p60,

  GST_DECKLINK_MODE_NTSC_WIDESCREEN,
  GST_DECKLINK_MODE_NTSC2398_WIDESCREEN,
  GST_DECKLINK_MODE_PAL_WIDESCREEN,
  GST_DECKLINK_MODE_NTSC_P_WIDESCREEN,
  GST_DECKLINK_MODE_PAL_P_WIDESCREEN
} GstDecklinkModeEnum;

#define NTSC 10, 11, false, "bt601"
#define PAL 12, 11, true, "bt601"
#define NTSC_WS 40, 33, false, "bt601"
#define PAL_WS 16, 11, true, "bt601"
#define HD 1, 1, true, "bt709"
#define UHD 1, 1, true, "bt2020"

typedef struct _GstDecklinkMode {
  BMDDisplayMode mode;
  int width;
  int height;
  int fps_n;
  int fps_d;
  gboolean interlaced;
  int par_n;
  int par_d;
  gboolean tff;
  const gchar *colorimetry;
} GstDecklinkMode;

static const GstDecklinkMode modes[] = {
  {bmdModeNTSC, 720, 486, 30000, 1001, true, NTSC},     // default is ntsc

  {bmdModeNTSC, 720, 486, 30000, 1001, true, NTSC},
  {bmdModeNTSC2398, 720, 486, 24000, 1001, true, NTSC},
  {bmdModePAL, 720, 576, 25, 1, true, PAL},
  {bmdModeNTSCp, 720, 486, 30000, 1001, false, NTSC},
  {bmdModePALp, 720, 576, 25, 1, false, PAL},

  {bmdModeHD1080p2398, 1920, 1080, 24000, 1001, false, HD},
  {bmdModeHD1080p24, 1920, 1080, 24, 1, false, HD},
  {bmdModeHD1080p25, 1920, 1080, 25, 1, false, HD},
  {bmdModeHD1080p2997, 1920, 1080, 30000, 1001, false, HD},
  {bmdModeHD1080p30, 1920, 1080, 30, 1, false, HD},

  {bmdModeHD1080i50, 1920, 1080, 25, 1, true, HD},
  {bmdModeHD1080i5994, 1920, 1080, 30000, 1001, true, HD},
  {bmdModeHD1080i6000, 1920, 1080, 30, 1, true, HD},

  {bmdModeHD1080p50, 1920, 1080, 50, 1, false, HD},
  {bmdModeHD1080p5994, 1920, 1080, 60000, 1001, false, HD},
  {bmdModeHD1080p6000, 1920, 1080, 60, 1, false, HD},

  {bmdModeHD720p50, 1280, 720, 50, 1, false, HD},
  {bmdModeHD720p5994, 1280, 720, 60000, 1001, false, HD},
  {bmdModeHD720p60, 1280, 720, 60, 1, false, HD},

#if 0
  {bmdMode2k2398, 2048, 1556, 24000, 1001, false, HD},
  {bmdMode2k24, 2048, 1556, 24, 1, false, HD},
  {bmdMode2k25, 2048, 1556, 25, 1, false, HD},

  {bmdMode2kDCI2398, 2048, 1080, 24000, 1001, false, HD},
  {bmdMode2kDCI24, 2048, 1080, 24, 1, false, HD},
  {bmdMode2kDCI25, 2048, 1080, 25, 1, false, HD},
  {bmdMode2kDCI2997, 2048, 1080, 30000, 1001, false, HD},
  {bmdMode2kDCI30, 2048, 1080, 30, 1, false, HD},
  {bmdMode2kDCI50, 2048, 1080, 50, 1, false, HD},
  {bmdMode2kDCI5994, 2048, 1080, 60000, 1001, false, HD},
  {bmdMode2kDCI60, 2048, 1080, 60, 1, false, HD},

  {bmdMode4K2160p2398, 3840, 2160, 24000, 1001, false, UHD},
  {bmdMode4K2160p24, 3840, 2160, 24, 1, false, UHD},
  {bmdMode4K2160p25, 3840, 2160, 25, 1, false, UHD},
  {bmdMode4K2160p2997, 3840, 2160, 30000, 1001, false, UHD},
  {bmdMode4K2160p30, 3840, 2160, 30, 1, false, UHD},
  {bmdMode4K2160p50, 3840, 2160, 50, 1, false, UHD},
  {bmdMode4K2160p5994, 3840, 2160, 60000, 1001, false, UHD},
  {bmdMode4K2160p60, 3840, 2160, 60, 1, false, UHD},
#endif

  {bmdModeNTSC, 720, 486, 30000, 1001, true, NTSC_WS},
  {bmdModeNTSC2398, 720, 486, 24000, 1001, true, NTSC_WS},
  {bmdModePAL, 720, 576, 25, 1, true, PAL_WS},
  {bmdModeNTSCp, 720, 486, 30000, 1001, false, NTSC_WS},
  {bmdModePALp, 720, 576, 25, 1, false, PAL_WS}
};

static GstStructure *
gst_decklink_mode_get_generic_structure (GstDecklinkModeEnum e)
{
  const GstDecklinkMode *mode = &modes[e];
  GstStructure *s = gst_structure_new ("video/x-raw",
      "width", G_TYPE_INT, mode->width,
      "height", G_TYPE_INT, mode->height,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, mode->par_n, mode->par_d,
      "interlace-mode", G_TYPE_STRING,
      mode->interlaced ? "interleaved" : "progressive",
      "framerate", GST_TYPE_FRACTION, mode->fps_n, mode->fps_d, NULL);

  return s;
}

static GstStructure *
gst_decklink_mode_get_structure (GstDecklinkModeEnum e, BMDPixelFormat f,
    gboolean input)
{
  const GstDecklinkMode *mode = &modes[e];
  GstStructure *s = gst_decklink_mode_get_generic_structure (e);

  if (input && mode->interlaced) {
    if (mode->tff)
      gst_structure_set (s, "field-order", G_TYPE_STRING, "top-field-first",
          NULL);
    else
      gst_structure_set (s, "field-order", G_TYPE_STRING, "bottom-field-first",
          NULL);
  }

  switch (f) {
    case bmdFormat8BitYUV:     /* '2vuy' */
      gst_structure_set (s, "format", G_TYPE_STRING, "UYVY",
          "colorimetry", G_TYPE_STRING, mode->colorimetry,
          "chroma-site", G_TYPE_STRING, "mpeg2", NULL);
      break;
    case bmdFormat10BitYUV:    /* 'v210' */
      gst_structure_set (s, "format", G_TYPE_STRING, "v210", NULL);
      break;
    case bmdFormat8BitARGB:    /* 'ARGB' */
      gst_structure_set (s, "format", G_TYPE_STRING, "ARGB", NULL);
      break;
    case bmdFormat8BitBGRA:    /* 'BGRA' */
      gst_structure_set (s, "format", G_TYPE_STRING, "BGRA", NULL);
      break;
    case bmdFormat10BitRGB:    /* 'r210' Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10 */
    case bmdFormat12BitRGB:    /* 'R12B' Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component */
    case bmdFormat12BitRGBLE:  /* 'R12L' Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component */
    case bmdFormat10BitRGBXLE: /* 'R10l' Little-endian 10-bit RGB with SMPTE video levels (64-940) */
    case bmdFormat10BitRGBX:   /* 'R10b' Big-endian 10-bit RGB with SMPTE video levels (64-940) */
    default:
      GST_WARNING ("format not supported %d", f);
      gst_structure_free (s);
      s = NULL;
      break;
  }

  return s;
}

static GstCaps *
gst_decklink_mode_get_caps_all_formats (GstDecklinkModeEnum e, gboolean input)
{
  GstCaps *caps;
  guint i;

  caps = gst_caps_new_empty ();
  for (i = 1; i < G_N_ELEMENTS (formats); i++)
    caps =
        gst_caps_merge_structure (caps, gst_decklink_mode_get_structure (e,
            formats[i].format, input));

  return caps;
}

static GstCaps *
gst_decklink_mode_get_template_caps (gboolean input)
{
  int i;
  GstCaps *caps;

  caps = gst_caps_new_empty ();
  for (i = 1; i < (int) G_N_ELEMENTS (modes); i++)
    caps =
        gst_caps_merge (caps,
        gst_decklink_mode_get_caps_all_formats ((GstDecklinkModeEnum) i,
            input));

  return caps;
}

G_END_DECLS