project('gst-plugin-format-fixator', 'c', 'cpp',
  version : '0.0.1',
  meson_version : '>= 0.54',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized'])

cc = meson.get_compiler('c')

gst_req = '>= 1.18.0'
gst_dep = dependency('gstreamer-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_base_dep'])
gstvideo_dep = dependency('gstreamer-video-1.0', version : gst_req,
    fallback : ['gst-plugins-base', 'video_dep'])
gstcheck_dep = dependency('gstreamer-check-1.0', version : gst_req,
    required : get_option('tests'),
    fallback : ['gstreamer', 'gst_check_dep'])
gtest_dep = dependency('gtest',
    required : get_option('tests'),
    fallback: ['gtest', 'gtest_dep'])

project_deps = [gst_dep, gstbase_dep, gstvideo_dep]
plugins = []

subdir('src')
if not get_option('tests').disabled() and gstcheck_dep.found() and gtest_dep.found()
  subdir('tests')
endif
