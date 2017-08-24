#main SConstruct
EnsurePythonVersion(2, 7)
EnsureSConsVersion(2, 3)

#Read the variables, add them to the environment.
print 'Reading settings.py to define current build'
vars = Variables(['variables.cache', 'settings.py'])
vars.Add(BoolVariable('DEBUG_MESSAGES', 'Set for debugging messages in SConscripts.', 0))
vars.Add(EnumVariable('BUILD_TYPE', 'Set the build type for the current target', \
    'Debug', allowed_values=('Debug', 'Release')))
vars.Add(PathVariable('HOST_INSTALL_DIR', 'Install directory of host executables.', 'bin'))
vars.Add(EnumVariable('TARGET_OS', 'Set the current target OS', \
    'win32', allowed_values=('win32', 'dos', 'posix', 'hdmi2usb-lm32')))
vars.Add(PathVariable('EXTRA_PATH', 'Path to compilers if autodetection fails.', 'bin'))
vars.Add(PathVariable('XFER_PATH', 'Path to the serial transfer application.', 'bin'))
vars.Add(PathVariable('PICTOR', 'Path to SoftCircuits PICTOR library', None))
vars.Add(PathVariable('HDMI2USB_BUILD', 'Path to HDMI2USB build root (should have "software" as subdirectory)', None))

"""Use Update(env) to add variables to existing environment:
http://stackoverflow.com/questions/9744867/scons-how-to-add-a-new-command-line-variable-to-an-existing-construction-enviro"""

env = Environment(variables = vars, tools = ["default", "doxygen"])
vars.Save('variables.cache', env)
Help(vars.GenerateHelpText(env))

#According to SCons documentation (2.2.0), the user is free to override internal
#variable TARGET_OS

if env['TARGET_OS'] is None:
    if env['DEBUG_MESSAGES']:
        print 'No target specified and not using Win32.'
        'Setting host platform as default.'
    env['TARGET_OS'] = env['PLATFORM']

build_dir = Dir('build/' + env['TARGET_OS'])

print 'Host platform is: ' + env['PLATFORM']
print 'Target platform is: ' + env['TARGET_OS']
print 'Build dir will be: ' + str(build_dir) + '\n'
if env['DEBUG_MESSAGES']:
    print 'Dumping Environment: ' + env.Dump()
    import os
    print os.environ['PATH']

env = SConscript(['targets/SConscript'], exports = ['env'])
pi_objs = SConscript(['src/SConscript'], exports = ['env'], variant_dir = build_dir, duplicate=0)
SConscript(['test/SConscript'], exports = ['env', 'build_dir', 'pi_objs'], variant_dir = build_dir.Dir('test'), duplicate=0)

#http://www.knowthytools.com/2009/05/scons-cleaning-variantdir.html
Clean('.', 'bin/' + env['TARGET_OS'])
#SConscript(['SRC/GUI/SConscript'])

if 'docs' in COMMAND_LINE_TARGETS:
    api_docs = env.Doxygen('Doxyfile')
    doc_alias = env.Alias('docs', [api_docs])
    env.Clean(doc_alias, api_docs)


#Additional help goes here...
