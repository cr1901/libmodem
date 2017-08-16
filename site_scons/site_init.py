import string

def perform_compiler_configuration(env, freestanding):
    if not env.GetOption('clean') and not env.GetOption('help'):
        conf = Configure(env, config_h='cc_config.h', custom_tests=\
            { 'CheckCharBit' : check_char_bit })
        conf.CheckHeader('stdio.h', include_quotes='<>', language='C')
        conf.CheckHeader('time.h', include_quotes='<>', language='C')
        conf.CheckType('double')
        #conf.CheckFunc('difftime', '#include <time.h>'):
        conf.CheckDeclaration('difftime', '#include <time.h>')
        conf.CheckCharBit()

        conf.Finish()

def check_char_bit(context):
    context.Message('Checking whether target has an 8-bit character... ' )
    ret = context.TryCompile("""
        #include <limits.h>
        #if CHAR_BIT != 8
            #error
        #endif
        int main(int argc, char **argv)
        {
        return 0;
        }""", '.c')

    SCons.Conftest._Have(context, 'HAVE_CHAR_BIT_8', ret, 'Set to 1 if the target has an 8-bit character.')
    context.Result(ret)
    return ret


platform_signatures = {'dos': ['__DOS__'],
    'win32': ['__WINDOWS__', '_WIN32'],
    'hdmi2usb-lm32': ['__lm32__']}

def platform_sanity_checks(env):
    if not env.GetOption('clean') and not env.GetOption('help'):
        conf = Configure(env, config_h='cc_config.h')
        if not conf.CheckCC():
            print 'Error: C compiler does not work!'
            Exit(1)

        for sig in platform_signatures[env['TARGET_OS']]:
            if conf.CheckDeclaration(sig):
                break
        else:
            print 'Error: The chosen C compiler {0} doesn''t appear ' + \
                'to support the chosen target {1}! ' + \
                + 'Check platform flags!'.format(env['TOOLS'], \
                env['TARGET_OS'])
            Exit(1)

        conf.Finish()

def check_platform_specific_libs(env):
    #If external libraries are required for the base-executable, check for them now.
    conf = Configure(env)

    if env['TARGET_OS'] == 'dos':
        env.Append(CPPPATH = [Dir(env['PICTOR'])])
        env.Append(LIBPATH = [Dir(env['PICTOR'])])
        libname = 'PICTOR' + string.capitalize(env.subst('$MEMMODEL')) + 'W'

        if not env.GetOption('clean'):
            if not conf.CheckLibWithHeader(libname, 'COMLIB.H', 'c'):
                print "PICTOR Library required for DOS target! "
                "Install in the same directory where the top level "
                "of this source tree resides."
                Exit(1)

    env = conf.Finish()
