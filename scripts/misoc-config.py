#!/usr/bin/env python3

import os.path
import argparse

def read_vars(root):
    build_settings = os.path.join(root, "software", "include", "generated", "variables.mak")
    with open(build_settings, 'r') as fp:
        misoc_dict = dict()
        for var in fp.read().split('\n'):
            try:
                (name, val) = var.split("=")
                misoc_dict[name] = val
            except:
                # Skip "export" line
                pass
    return misoc_dict

def get_misoc_libs(keys, exclude):
    def in_exclude(s):
        for e in ["BUILDINC", "SOC"] + exclude:
            if s.startswith(e.upper()):
                return True
        return False

    lib_dirs = lambda s : s.endswith("_DIRECTORY") and not in_exclude(s)
    libs = []
    # Convert to lowercase name for the lib as existing on the filesystem.
    # This will break for libraries with capital letters.
    for libdir in filter(lib_dirs, d.keys()):
        libs += [libdir.lower()[:-10]]
    return libs

def get_lib_dir(m_dict, lib):
    return m_dict[lib.upper() + "_DIRECTORY"]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Get command-line arguments for MiSoC design.")
    parser.add_argument("design_root", type=str, help=
        "Root of generated MiSoC/Litex design (subdirectories should include software/ and gateware/)")
    parser.add_argument("--cxx", action='store_true', help="Output --cflags/--libs assuming C++ compile")
    parser.add_argument("--cflags", action='store_true', help="Output all preprocessor and compiler flags")
    parser.add_argument("--cflags-only-I", action='store_true', help="Output -I flags")
    parser.add_argument("--cflags-only-other", action='store_true', help="Output all other --cflags besides -I")
    parser.add_argument("--cpu", action='store_true', help="Get CPU. Ignored if other args are present")

    parser.add_argument("--libs", action='store_true', help="Output all linker flags")
    parser.add_argument("--libs-only-l", action='store_true', help="Output -l flags")
    parser.add_argument("--libs-only-L", action='store_true', help="Output -L flags")
    parser.add_argument("--libs-only-other", action='store_true', help="Output all other --libs besides -L and -l")

    parser.add_argument("--exclude-dirs", nargs='+', type=str, default=[],
        help="Exclude subdirectories under $DESIGN_ROOT/software from --libs.")

    args = parser.parse_args()

    d = read_vars(args.design_root)

    outstr = []
    if args.cpu:
        print(d["CPU"])
    else:
        cflags = ["-Os " + d['CPUFLAGS'] + " -fomit-frame-pointer -Wall -fno-builtin -nostdinc"]
        ccflags = ["-fexceptions -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes"]
        cxxflags = ["-std=c++11 -I" + d['SOC_DIRECTORY'] + "/software/include/basec++ -fexceptions -fno-rtti -ffreestanding"]

        incdirs = ["-I" + d["SOC_DIRECTORY"] + p for p in ['/software/include/base', '/software/include', '/software/common']]
        incdirs += ["-I" + d["BUILDINC_DIRECTORY"]]


        misoc_libs = get_misoc_libs(d.keys(), args.exclude_dirs)
        lflags = ["-nostdlib -nodefaultlibs"]
        # libbase holds libc
        if "libbase" in misoc_libs:
            lflags += [args.design_root + "software/libbase/crt0-" + d["CPU"] + ".o"]

        ldirs = ["-L" + d["BUILDINC_DIRECTORY"]] # Required for linker script
        llibs = []
        for l in misoc_libs:
            ldirs += ["-L" + get_lib_dir(d, l)]
            if l.startswith("lib"):
                llibs += ["-l" + l[3:]]
            else:
                llibs += ["-l" + l]

        def c_other(args):
            o = []
            o += ["-Wall -Wextra -pedantic-errors -std=c99"]
            o += cflags
            if args.cxx:
                o += cxxflags
            else:
                o += ccflags
            return o

        def c_inc():
            return incdirs

        def l_other():
            return lflags

        def l_inc():
            return ldirs

        def l_libs():
            return llibs

        if args.cflags:
            outstr += c_other(args)
            outstr += c_inc()
        if args.cflags_only_other and not args.cflags:
            outstr += c_other(args)
        if args.cflags_only_I and not args.cflags:
            outstr += c_inc()

        if args.libs:
            outstr += l_other()
            outstr += l_inc()
            outstr += l_libs()
        if args.libs_only_other:
            outstr += l_other()
        if args.libs_only_L:
            outstr += l_inc()
        if args.libs_only_l:
            outstr += l_libs()

        print(" ".join(outstr))
