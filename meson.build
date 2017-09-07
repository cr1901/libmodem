project('libmodem', 'c')
cdata = configuration_data()

# Set cross-file [property] vars to defaults if not set in cross-file or
# command-line. Command-line overrides [property], cross-file overrides
# default.
cpu_override = meson.get_cross_property('cpu_override', cdata.get('cpu_override', false))


# Determine platform portion of canonical name
# POSIX handled specially
posix = ['linux', 'bsd', 'darwin']
posix_match = false
foreach p : posix
    if (host_machine.system() == p and not posix_match)
        posix_match = true
    endif
endforeach

if not (posix_match)
    platform = 'posix'
else
    platform = host_machine.system()
endif


# Platform-specific Build Helpers
pd_src = []
if platform == 'hdmi2usb'
    hdmiusb_dir = meson.get_cross_property('hdmi2usb_dir', cdata.get('hdmi2usb_dir', ''))
    c_flags = run_command(join_paths(meson.source_root(), 'scripts', 'misoc-config.py'),
                          hdmi2usb_dir), '--cflags')
    if c_flags.returncode() == 0
        add_project_arguments(c_flags.stdout().strip().split(), language : 'c')
    else
        error('misoc-config.py failed: ' + c_flags.stderr())
    endif
    tests_avail = false
else
    tests_avail = true
endif


# Platform-specific Instructions common to all targets
if cpu_override
    pd_prefix = platform + '-' + host_machine.cpu()
else
    pd_prefix = platform
endif

pd_src += ['serprim.c']
pd_src_path = []
foreach s : pd_src
    pd_src_path += join_paths('src', pd_prefix, s)
endforeach


# Generic Build Instructions
incdir = include_directories('src')
pi_src = ['src/serial.c', 'src/xmodem.c']

lib_src = pi_src + pd_src_path
static_library('modem', lib_src, include_directories : incdir)

# TODO: Provide sample applications for hosted platforms.
# if meson.get_compiler('c').get_define('__STDC_HOSTED__')
# Build sample applications.
# endif


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
