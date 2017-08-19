import os.path
import SCons.Tool.gcc
import SCons.Util

def import_vars(env):
    build_settings = os.path.join(env['HDMI2USB_BUILD'], "software", "include", "generated", "variables.mak")
    path_hack = os.path.split(SCons.Util.WhereIs("lm32-elf-gcc"))[0]
    env.AppendENVPath('PATH', path_hack)

    with open(build_settings, 'r') as fp:
        hdmi_dict = dict()
        for var in fp.read().split('\n'):
            try:
                (name, val) = var.split("=")
                # When using gcc build, SCons doesn't treat "C:/" as a
                # top-level. Manually replace.
                hdmi_dict[name] = val.replace("\\\\", "/").replace("\r", "").replace("C:/", "/c/")
            except:
                # Skip "export" line
                pass
        env['HDMI2USB'] = hdmi_dict

def exists(env):
    return SCons.Tool.gcc.exists(env)

def generate(env):
    import_vars(env)
    env["CC"] = "lm32-elf-gcc" # Setting CC directly allows override of
    # default gcc.
    SCons.Tool.gcc.generate(env)
