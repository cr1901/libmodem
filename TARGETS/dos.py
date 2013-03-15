def generate(env, **kw):
	env['CC'] = 'wcc'
	env['CCCOM'] = '$CC -fo=$TARGET -c $CFLAGS $CCFLAGS $_CCCOMCOM $SOURCES'
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 0
	
def exists(env):
	return 0
