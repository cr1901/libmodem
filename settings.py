#BUILD =
#TOOLCHAIN = WATCOM #MINGW, CL, WATCOM, CC65, GCC
#TARGET = 'WIN32' #WIN32, WIN16, DOS, UNIX, VIC20, OS/2
DEBUG_MESSAGES = False
BUILD_TYPE = 'Release'
TARGET_OS = 'hdmi2usb-lm32'
#TARGET_OS = 'DOS'
#GUI = True
EXTRA_PATH='C:\\msys64\\mingw32\\bin'
HOST_INSTALL_DIR = 'C:\W2CUTILS'

XFER_CMD = 'TRXCOM'
XFER_PATH = HOST_INSTALL_DIR
XFER_FLAGS= '/T /X /CRC /1K /B:19200 /P:4'

#Combine Toolchain and Target to determine all valid combinations.
