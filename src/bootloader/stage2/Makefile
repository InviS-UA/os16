.PHONY: all stage2 clean

all: stage2

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: main.asm
	@$(ASM) $< -f bin -o $@
	@echo "--> Created  stage2.bin"

clean:
	@rm -f $(BUILD_DIR)/stage2.bin