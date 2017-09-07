# libmodem
`libmodem` is a serial port abstraction library designed with portability in
mind. The platform-independent parts of libmodem are written in ANSI C, and
should compile with any freestanding or hosted ANSI C compiler.

# Purpose
While `libmodem` provides an API for accessing serial ports regardless of OS,
the main crux of `libmodem` is to provide implementation of common data
transfer routines over a serial port that should compile (space-permitting) in
any environment with a UART. Currently the following transfer protocols are
supported:
* XMODEM

The following platforms are supported:
* WIN32/64- `windows`
* [HDMI2USB](https://hdmi2usb.tv/home/)- `hdmi2usb`

With the following platforms either started or at one point functional:
* WIN16
* Generic POSIX Implementation
* DOS using the PICTOR library

# Building
`libmodem` requires `meson` to build, although nothing in principle
prevents hand-tailored Makefiles for retro platforms from working as well.

Being a portable library that is meant to support various compiler vendors,
a number of `meson` [cross-files](http://mesonbuild.com/Cross-compilation.html#defining-the-environment)
are provided in the `targets` directory. These cross-files serve as a
guideline for incorporating new ports into `meson`. An additional C compiler
toolchain is also not difficult to add to `meson`.

If your platform and compiler toolchain is already supported, the following
invocation should work for a native build:

```
meson build_dir
```

<a name="cc"></cc>Or for a cross-compile:
```
meson build_dir -dmy_var=value --cross-file=targets/{cross-file.txt}
```

Cross-compiling may require some platform-specific variables to be set prior
to running `meson`. See [Cross Files](#cf).

# Code Structure
`libmodem` is separated into platform-independent and platform-dependent
directories. To avoid a large amount of preprocessor defines in
platform-independent interfaces to the serial port, each platform's
platform-dependent files are placed in their own subdirectory under `src`,
referred to as `$PLATFORM`. See [Platform Directories](#pd) for more
information.

## Platform-Dependent Files
Each platform-dependent directory _must_ have at least a source file named
`src/$PLATFORM/serprim.c` that implements the header file `src/serprim.h`.
Other headers and source files may be included as necessary.

## Platform-Independent Files
Platform-independent files include:
* `src/serprim.h` : Header for platform-dependent functions that must be
implemented by a target.
* `src/serial.h` : Header for `src/serial.c`.
* `src/modem.h` : Header for all data transfer functions.
* `src/serial.c` : Implements serial port wrappers to be used by applications
using libmodem.
* `src/xmodem.c` : Provides an XMODEM transmitter and receiver implementation.

## <a name="pd"></a>Platform Directories
As stated above, platform-dependent files are stored in a subdirectory under
src, one directory per platform where `libmodem` runs. To facilitate
code-sharing, `$PLATFORM` is deduced by the `meson.build`, using the following
two variables:

* `$SYSTEM`- Return value of `host_machine.system()` in `meson` for a given cross build.
* `$CPU`- Return value of `host_machine.cpu()` in `meson` for a given cross build.

The build system searches for a directory name either `$SYSTEM`
or `$SYSTEM-$CPU`, _typically the former_, under `src` for platform-specific
source files. Because compilers and tools will certainly vary for the same
platform on different CPUs, cross files are named `$SYSTEM-$CPU.txt` by convention.
`meson` will not search for a `$SYSTEM-$CPU` directory unless [`cpu_override`](#cpuo) is
set in a cross-file.

### Special System Names
Platforms which substantially share code may be given an encompassing
system name if the `host_machine.system()` variable alone is insufficient to
unify all possible platforms that share the code.

This mainly applies to `linux`, `darwin`, and `bsd` systems, which all
use the `posix` platform for `libmodem`; `meson` does not return `posix` from
`build_machine.system()`. If cross-compiling to any of these
systems, the `system` field under `[host_machine]` in the cross-file
should be the actual system to target, and not `posix`. The build system
will perform the conversion, so _all conversions should be handled in
`meson.build`, not the cross-files_.

## <a name="cf"></a>Cross Files
The `targets` directory provides sample
[cross files](http://mesonbuild.com/Cross-compilation.html#defining-the-environment)
which can be used to build variants of `libmodem`. Not all of the provided
files are guaranteed to work without modification. In particular, some
cross compile targets may need to have special variables set under their
cross-file's `[properties]` section (or on the [command-line](#cc)).

The cross files provided cannot encompass all systems where `libmodem`
can theoretically run. However, they provide a base such that adding
a new system requires minimal effort.

### Global Properties
Global properties cannot be overridden from the `meson` command-line.

#### <a name="cpuo"></a>`cpu_override`
Most platform-specific code does not need a separate
directory for each CPU that platform supports, even during a
cross-compile. Set this property to nonzero if a separate directory for
each CPU is required.

_This property defaults to `false` if not set or if doing a native build._
A platform where the compiler runs natively (and where `meson` runs) is
assumed to export the same set of hardware interfaces regardless of
architecture.

### Targets With Special Properties
Target properties can be overridden by using an appropriate `-D` flag as input
to `meson`. See [Cross Files Section](#cf-port) of the Porting Guide.

#### `hdmi2usb`

* `hdmi2usb_dir`- Points to the litex build directory of the generated
System-on-a-Chip. `software` and `gateware` should exist as subdirectories.
_By default, this property is the empty string. A cross-build is likely to fail
without modification._

# Porting Guide

## Porting A New Platform
Porting libmodem basically requires four steps:

1. Determine the name of your platform (`$PLATFORM`), and create `$PLATFORM`
under `src`.
2. Implement all functions exported by `src/serprim.h` in `src/$PLATFORM/serprim.c`.
3. Modify `meson.build` with any platform-dependent setup, such as extra
source files to be compiled, and configuration variables
4. If desired/necessary, create a cross-file under `targets`.

### Platform Name
If porting `libmodem` to a platform that `meson` already runs on, a cross-file
needs to set `host_machine.system()` to use the name that `meson` would set
for `build_machine.system()` during a native build on said platform (to ensure
native and cross builds work the same way). Being a candidate for a native
build, `meson.build` will not search for a `system-cpu` directory.

On the other hand, if your port target is freestanding (no OS), or not capable
of doing a native `meson` build, you have a choice in the name of `$PLATFORM`.
Good candidates for a `system` name within the `[host_machine]` of a cross-file
include:

* Name that encompasses an entire computer system, e.g. a programming
environment and and PCB (such as `hdmi2usb`)
* Name of a microcontroller family whose members all export an identical base
set of peripherals (`msp430-uart`, `msp430-bitbang`)

### <a name="cf-port"></a>Cross-Files
If you add a cross-file under `targets` that requires a new property to build, my
suggestion is to comment the property out, say `my_var`, with the default
value in the cross-file, like this:

```
[properties]
# my_var = ''
```

In `meson.build`, you should add a line under `# Platform-specific Build Helpers`
that looks like the following:

```
cdata = configuration_data()
my_var = meson.get_cross_property('my_var', cdata.get('my_var', ''))
```

This allows a user to copy the cross-file to a new location if they need a
persistent cross-build, while giving users the option to override options
on the command line without needing to modify (and possibly accidentally commit)
a cross-file tailored to a single build configuration.


## Porting A New Toolchain
To Be Written
