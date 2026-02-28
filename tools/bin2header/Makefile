BUILD_DIR = build
OUT_DIR = output
TARGET = bin2header
CC ?= g++
OBJS = $(addprefix $(BUILD_DIR)/$(TARGET)/, \
    main.obj )
export MSYS_NO_PATHCONV=1
all: | $(OUT_DIR) $(OUT_DIR)/$(TARGET).exe
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUT_DIR)
$(OUT_DIR)/$(TARGET).exe: $(OBJS)
	@g++ -o $@ $^
	@echo Building $@ ...
$(BUILD_DIR)/$(TARGET)/%.obj: %.cpp
	@g++ -std=c++20 -O2 -c $< -o $@
	@echo Building $@ ...
$(OBJS): | $(BUILD_DIR)/$(TARGET)
$(BUILD_DIR)/$(TARGET):
	@mkdir -p "$(BUILD_DIR)/$(TARGET)"
$(OUT_DIR):
	@mkdir -p "$(OUT_DIR)"