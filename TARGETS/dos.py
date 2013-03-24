def generate(env, **kw):
	import os
	#Get environment for the WATCOM compiler from the OS.
	env['ENV']['WATCOM'] = os.environ['WATCOM']
	env['ENV']['INCLUDE'] = os.environ['INCLUDE']
	env['ENV']['EDPATH'] = os.environ['EDPATH']
	env['ENV']['WIPFC'] = os.environ['WIPFC']
	env['ENV']['WHTMLHELP'] = os.environ['WHTMLHELP']
	env.AppendENVPath('PATH', env['ENV']['WATCOM'] + '\BINNT')
	env.AppendENVPath('PATH', env['ENV']['WATCOM'] + '\BINW')
	
	#Set dos-specific information
	env['MEMMODEL'] = 'S'
	
	#Then set up the compiler tools themselves.
	env['CC'] = 'wcl'
	env['CCFLAGS'] = '-0 -c -m' + env['MEMMODEL'] + ' -za -zq -os -bt=dos'
	env['CCCOM'] = '$CC -fo=$TARGET $CFLAGS $CCFLAGS $_CCCOMCOM $SOURCES'
	env['INCPREFIX'] = '-i='
	env['AR'] = 'wlib'
	#$(LIB) $(LIBFLAGS) $(LIBNAME) +$(OBJS)
	env['LINK'] = 'wcl'
	env['LINKFLAGS'] = '-l=dos -zq'
	env['LIBDIRPREFIX'] = '-"libpath '
	env['LIBDIRSUFFIX'] = '"'
	env['LIBLINKPREFIX'] = '-"library '
	env['LIBLINKSUFFIX'] = '"'
	#  ${TEMPFILE("$LINK $LINKFLAGS /OUT:$TARGET.windows $_LIBDIRFLAGS $_LIBFLAGS $_PDB $SOURCES.windows")}
	env['LINKCOM'] = '$LINK $LINKFLAGS $_LIBDIRFLAGS $_LIBFLAGS $SOURCES -fe=$TARGET'
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
	del os
	
	
	#Old configuration
	#env['CC'] = 'wcc'
	#env['CCFLAGS'] = '-za -os -bt=dos'
	#env['CCCOM'] = '$CC -fo=$TARGET $CFLAGS $CCFLAGS $_CCCOMCOM $SOURCES'
	#env['INCPREFIX'] = '-i='
	#env['AR'] = 'wlib'
	#env['LINK'] = 'wlink'
	#env['LINKFLAGS'] = 'system dos'
	#env['LIBDIRPREFIX'] = 'libpath '
	#env['LIBLINKPREFIX'] = 'library '
	#  ${TEMPFILE("$LINK $LINKFLAGS /OUT:$TARGET.windows $_LIBDIRFLAGS $_LIBFLAGS $_PDB $SOURCES.windows")}
	#env['LINKCOM'] = '$LINK $LINKFLAGS $_LIBDIRFLAGS $_LIBFLAGS file{$SOURCES} name $TARGET'
	
def exists(env):
	return check_for_watcom()
	
def check_for_watcom():
	try:
		return env['WATCOM']
	except KeyError:
		return None
	
		
