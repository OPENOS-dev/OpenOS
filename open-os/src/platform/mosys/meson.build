project('mosys', 'c', default_options : ['platform_intf=generic'])

# Include common. This is passed to all subdir build files as well
include_common = include_directories(['include'])

platform_intf = get_option('platform_intf').to_lower()

add_global_arguments('-std=gnu99', language : 'c')

cc = meson.get_compiler('c')
add_global_arguments('-Wall', language : 'c')
add_global_arguments('-Werror', language : 'c')
add_global_arguments('-Wstrict-prototypes', language : 'c')
add_global_arguments('-Wundef', language : 'c')

# Require that externally-visible symbols have a prototype.  This
# helps find missing usages of 'static'.
add_global_arguments('-Wmissing-variable-declarations', language : 'c')
add_global_arguments('-Wmissing-prototypes', language : 'c')

libmosys_src = files()
platform_support_src = files()
unittest_src = files()

# Subdirs with source to link against
subdir('core')
subdir('lib')
subdir('platform')
subdir('mains')
subdir('unittests')
