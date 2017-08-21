# libmodem
`libmodem` is a serial port abstraction library designed with portability in
mind. The platform-independent parts of libmodem are written in ANSI C, and
should compile with any freestanding or hosted ANSI C compiler.

## Purpose
While `libmodem` provides an API for accessing serial ports regardless of OS,
the main crux of `libmodem` is to provide implementation of common data
transfer routines over a serial port that should compile (space-permitting) in
any environment with a UART. Currently the following transfer protocols are
supported:
* XMODEM

The following platforms are supported:
* WIN32/64
* [HDMI2USB](https://hdmi2usb.tv/home/)

With the following platforms either started or at one point functional:
* WIN16
* Generic POSIX Implementation
* DOS using the PICTOR library

## Building
`libmodem` currently requires `scons` to build although nothing in principle
prevents hand-tailored Makefiles for retro platforms from working as well.

Being a portable library that is meant to support various compiler vendors,
a number of customized `scons` tools are provided in the
`site_scons/site_tools` directory. The existing custom tools serve as a
guideline for adding new compiler support when porting to a new toolchain.

Builds are customized using build variables.
If your platform and compiler toolchain is already supported, the following
invocation should work:

```
scons TARGET_OS={win32, hdmi2usb-lm32}...
```

setting other Platform Specific Defines as necessary. Run `scons -h` for a
list of all supported build variables.

The build script will read `settings.py` to set build variables that persist
between invocations. `settings.py` takes the form of `VAR=value`, one on each line.
Variables from `settings.py` will take effect if the corresponding command line
variable is absent, and will be written to `variables.cache` for retrieval
during the next build (even if the variable is removed from `settings.py`).
Variables specified on the command line override `settings.py` and
`variables.cache` for a single build.

### Platform Specific Defines
#### HDMI2USB
* `HDMI2USB_BUILD`: Define to the top-level of the generated SoC. `software`
and `gateware` should be subdirectories.

## Code Structure
`libmodem` is separated into platform-independent and platform-dependent
directories. To avoid a large amount of preprocessor defines in
platform-independent interfaces to the serial port, each platform's
platform-dependent files are placed in their own subdirectory under `src`.
_The name of the subdirectory must match the `TARGET_OS` `scons` build variable
for the build system to find the platform when building._

Platform-independent files include:
* `src/serprim.h` : Header for platform-dependent functions that must be
implemented by a target.
* `src/serial.h` : Header for `src/serial.c`.
* `src/modem.h` : Header for all data transfer functions.
* `src/serial.c` : Implements serial port wrappers to be used by applications
using libmodem.
* `src/xmodem.c` : Provides an XMODEM transmitter and receiver implementation.

Each platform-dependent directory _must_ have at least a source file named
`src/$TARGET_OS/serprim.c` that implements the header file `src/serprim.h`.
Other headers and source files may be included as necessary.

To allow multiple platforms to be compiled at once, each version of libmodem
for a different platform is compiled in a directory under `build`, named after
the value of `TARGET_OS`.

## Porting A New Platform
To Be Written

## Porting A New Toolchain
To Be Written
