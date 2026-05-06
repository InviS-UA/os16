include scripts/config.mk

.PHONY: all disk_image bootloader always clean

all: disk_image

#
# Disk image
#
disk_image: $(BUILD_DIR)/disk_image.img

$(BUILD_DIR)/disk_image.img: bootloader
	@./scripts/make_disk.sh $@ $(MAKE_DISK_SIZE)
	@echo "--> Created: " $@

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/bootloader/stage1

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/bootloader/stage2

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	@rm -rf $(BUILD_DIR)/*