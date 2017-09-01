project('libmodem', 'c')
cdata = configuration_data()

# Platform-specific Build Helpers
if host_machine.system() == 'hdmi2usb'
    c_flags = run_command(join_paths(meson.source_root(), 'scripts', 'misoc-config.py'),
                          meson.get_cross_property('hdmi2usb_dir'), '--cflags')
    if c_flags.returncode() == 0
        add_project_arguments(c_flags.stdout().strip().split(), language : 'c')
    else
        error('misoc-config.py failed: ' + c_flags.stderr())
    endif
    tests_avail = false
else
    tests_avail = true
endif



## Platform-specific Instructions common to all targets
pd_src = ['serprim.c']
pd_src_path = []
foreach s : pd_src
    prefix = host_machine.system() + '-' + host_machine.cpu()
    pd_src_path += join_paths('src', prefix, s)
endforeach


# Generic Build Instructions
incdir = include_directories('src')
pi_src = ['src/serial.c', 'src/xmodem.c']

lib_src = pi_src + pd_src_path
static_library('modem', lib_src, include_directories : incdir)


# Test
if tests_avail
    test_src = ['test/serprim.c', 'test/unittest.c'] + pi_src
    test_inc = include_directories('test')
    unit_tests = executable('unittest', test_src,
        include_directories : [incdir, test_inc])
    test('unittest', unit_tests)
endif


# Doc
doxygen = find_program('doxygen', required : false)
if doxygen.found()
    # Strip absolute path on Windows- confuses Doxygen
    if build_machine.system() == 'windows'
        res = run_command(join_paths(meson.source_root(), 'scripts', 'get-relpath.py'),
                          meson.source_root(),
                          meson.build_root())
        root = res.stdout().strip()
    else
        root = meson.source_root()
    endif

    cdata.set('TOP_SRCDIR', root)
    doxyfile = configure_file(input: 'Doxyfile.in',
                        output: 'Doxyfile',
                        configuration: cdata,
                        install: false)
    html = custom_target('doc',
                          input: doxyfile,
                          output: 'html',
                          command: [doxygen, doxyfile])
endif
