EnsurePythonVersion(2, 7)
EnsureSConsVersion(2, 2)

#Read the variables, add them to the environment.
print 'Reading settings.py to define current build'
vars = Variables(['variables.cache', 'settings.py'])
vars.Add(BoolVariable('DEBUG_MESSAGES', 'Set for debugging messages in SConscripts.', 0))
vars.Add(EnumVariable('BUILD_TYPE', 'Set the build type for the current target', \
	'Debug', allowed_values=('Debug', 'Release')))
vars.Add(PathVariable('HOST_INSTALL_DIR', 'Install directory of host executables.', '#/BIN'))
vars.Add(EnumVariable('TARGET_OS', 'Set the current target OS', \
	'WIN32', allowed_values=('WIN32', 'DOS')))

vars.Add(PathVariable('XFER_PATH', 'Path to the serial transfer application.', '#/BIN'))

"""Use Update(env) to add variables to existing environment:
http://stackoverflow.com/questions/9744867/scons-how-to-add-a-new-command-line-variable-to-an-existing-construction-enviro"""

env = Environment(variables = vars)
vars.Save('variables.cache', env)
Help(vars.GenerateHelpText(env))

#According to SCons documentation (2.2.0), the user is free to override internal
#variable TARGET_OS

if env['TARGET_OS'] is None:
	if env['DEBUG_MESSAGES']:
		print 'No target specified and not using Win32.'
		'Setting host platform as default.'
	env['TARGET_OS'] = env['PLATFORM']


print 'Host platform is: ' + env['PLATFORM']
print 'Target platform is: ' + env['TARGET_OS']
if env['DEBUG_MESSAGES']:
	print 'Dumping Environment: ' + env.Dump()

env.Append(CPPPATH = '.') #See pitfalls for rationale.
build_dir = Dir('BUILD/' + env['TARGET_OS'])
env = SConscript(['TARGETS/SConscript'], exports = ['env'])
SConscript(['SRC/SConscript'], exports = ['env'], variant_dir = build_dir, duplicate=0)
SConscript(['TEST/SConscript'], exports = ['env', 'build_dir'], variant_dir = build_dir.Dir('TEST'), duplicate=0)

#http://www.knowthytools.com/2009/05/scons-cleaning-variantdir.html
#Clean('.', 'BIN/' + env['TARGET_OS'])
#SConscript(['SRC/GUI/SConscript'])

#Additional help goes here...



#Potential bugs/pitfalls
#Possible SCons bugs?
#Don't combine directory then string, unless Dir is a single-element list.
#env.Append(CPPPATH = Dir('SRC'))
#env.Append(CPPPATH = '.') #Breaks

#When using a VariantDir, includes become relative to the variantdir if the following
#line is used:
#env.Append(CPPPATH = '.')
#env.Append(CPPPATH = 'SRC') #This does not work, being the "true" variantdir
#env.Append(CPPPATH = '#/SRC') #Neither does this
#env.Append(CPPPATH = Dir('.')) #Or this...
#env.Append(CPPPATH = Dir('SRC')) #Or this...
#env.Append(CPPPATH = Dir('#/SRC')) #Or this!
#See SRC/SConscript for more fun!
