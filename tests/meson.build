test_sources = [
  'test.cpp'
]

exe = executable('videoformatfixator',
      test_sources,
      dependencies : project_deps + [gstcheck_dep, gtest_dep])

test('videoformatfixator', exe)
