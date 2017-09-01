project('libmodem', 'c')
cdata = configuration_data()

# Platform-specific Build Helpers



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
test_src = ['test/serprim.c', 'test/unittest.c'] + pi_src
test_inc = include_directories('test')
unit_tests = executable('unittest', test_src, include_directories : [incdir, test_inc])
test('unittest', unit_tests)


# Doc
doxygen = find_program('doxygen', required : false)
if doxygen.found()
    # Strip absolute path of drive letter- confuses Doxygen
    if build_machine.system() == 'windows'
        res = run_command(join_paths(meson.source_root(), 'scripts', 'get_relpath.py'),
                          meson.source_root(),
                          meson.build_root())
        root = res.stdout()
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
