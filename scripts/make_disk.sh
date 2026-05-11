#!/bin/bash

TARGET=$1
SIZE=$2

STAGE1_STAGE2_LOCATION_OFFSET=480
DISK_SECTOR_COUNT=$(( (${SIZE} + 511 ) / 512 ))

# generate image file
dd if=/dev/zero of=$TARGET bs=512 count=${DISK_SECTOR_COUNT} >/dev/null

# determine how many reserved sectors
STAGE2_SIZE=$(stat -c%s ${BUILD_DIR}/stage2.bin)
STAGE2_SECTORS=$(( ( ${STAGE2_SIZE} + 511 ) / 512 ))
RESERVED_SECTORS=$(( 1 + ${STAGE2_SECTORS} ))

# create file system
mkfs.fat -F 16 -R ${RESERVED_SECTORS} -n "OS16" $TARGET >/dev/null

# install bootloader
dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 count=3 2>&1 >/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 seek=90 skip=90 2>&1 >/dev/null
dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1

# write lba address of stage2 to bootloader
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$STAGE1_STAGE2_LOCATION_OFFSET
printf "%08x" ${STAGE2_SECTORS} | sed 's/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/' | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$(( $STAGE1_STAGE2_LOCATION_OFFSET + 4 )) status=none

# copy files
mcopy -i $TARGET -s ${IMAGE_DIR}/root/* ::/
mcopy -i $TARGET ${BUILD_DIR}/kernel.sys "::/kernel.sys"