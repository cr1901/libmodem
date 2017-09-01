project('libmodem', 'c')

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
