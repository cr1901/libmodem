EnsurePythonVersion(2, 7)
EnsureSConsVersion(2, 2)

#Read the variables, add them to the environment.
print 'Reading settings.py to define current build'
vars = Variables('settings.py')
vars.Add(BoolVariable('DEBUG_MESSAGES', 'Set for debugging messages in SConscripts.', 0))
vars.Add(EnumVariable('BUILD_TYPE', 'Set the build type for the current target', \
	'Debug', allowed_values=('Debug', 'Release')))

"""Use Update(env) to add variables to existing environment:
http://stackoverflow.com/questions/9744867/scons-how-to-add-a-new-command-line-variable-to-an-existing-construction-enviro"""

default_env = DefaultEnvironment(variables = vars)

#According to SCons documentation (2.2.0), the user is free to override internal
#variable TARGET_OS

#if

if default_env['TARGET_OS'] is None:
	if DEBUG_MESSAGES:
		print 'No target specified and not using Win32.'
		'Setting host platform as default.'
	default_env['TARGET_OS'] = default_env['PLATFORM']


print 'Host platform is: ' + default_env['PLATFORM']
print 'Target platform is: ' + default_env['TARGET_OS']
#if DEBUG_MESSAGES:
print 'Dumping Environment: ' + default_env.Dump()
Export('default_env')

SConscript(['TARGETS/SConscript'])
SConscript(['SRC/SConscript'])
#SConscript(['SRC/GUI/SConscript'])

