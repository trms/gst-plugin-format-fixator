format_fixator_sources = [
  'gstvideoformatfixator.c',
  'plugin.c',
]

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

gstformatfixator = library('gstformatfixator',
  format_fixator_sources,
  c_args : ['-DPACKAGE="gstreamer"', '-DVERSION="@0@"'.format(meson.project_version())],
  dependencies : project_deps,
  install : true,
  install_dir : plugins_install_dir,
)

plugins += [gstformatfixator]
