MAKE_DISK_SIZE = 16777216 # 16MB

export ASM = nasm
export ASMFLAGS =

export TARGET = ia16-elf
export TARGET_CC = $(TARGET)-gcc
export TARGET_CFLAGS =
export TARGET_LD = $(TARGET)-gcc
export TARGET_LINKFLAGS =
export TARGET_LIBS =

export PROJECT_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)
export IMAGE_DIR = $(abspath image)