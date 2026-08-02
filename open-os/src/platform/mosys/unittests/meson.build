cmocka_dep = dependency('cmocka')

foreach f: unittest_src
    path = '@0@'.format(f)
    # Strip the leading path and suffix to get the
    # test name. E.g. 'a/b/test.c' -> 'test'.
    file_name = path.split('/').get(-1)
    name = file_name.split('.').get(0)

    # Find any symbols the unit test wraps.  We do this by searching
    # the file for __wrap_NAME and creating a list of symbols.  Meson
    # has nothing good to do this search natively, so unfortunately we
    # have to rely on sed to do our heavy-lifiting here.
    r = run_command('sed', '-n', 's/^.*__wrap_\\([A-Za-z0-9_]\\+\\).*$/\\1/p',
                    '../@0@'.format(f))
    output = r.stdout().split()

    # Make unique list of symbols
    wrapped_symbols = []
    foreach line : output
      if line not in wrapped_symbols
        wrapped_symbols += [line]
      endif
    endforeach

    # Convert to link arguments
    link_args = []
    foreach sym : wrapped_symbols
      link_args += ['-Wl,--wrap=@0@'.format(sym)]
    endforeach

    test_target = executable(
        name,
        [f] + libmosys_src + platform_support_src,
        include_directories: include_common,
        dependencies: [cmocka_dep],
        link_args : link_args,
    )
endforeach
