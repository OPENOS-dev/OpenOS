mosys_main = executable(
  'mosys',
  libmosys_src + platform_support_src + ['mosys.c'],
  include_directories : include_common,
  c_args : ['-flto'],
  link_args : ['-flto'],
)
