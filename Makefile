BUILD_DIR := build
APP := app
TYPE ?= Debug
COMPILER := clang++

SRCS := $(shell find src -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null)

.PHONY: all run debug gdb tidy valgrind asan clean reconfigure

all: build

# 1. Configuration (No more 'rm -rf' here)
$(BUILD_DIR)/build.ninja: CMakeLists.txt
	@echo "--- Configuring project ($(TYPE)) ---"
	mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_BUILD_TYPE=$(TYPE) \
		-DCMAKE_CXX_COMPILER=$(COMPILER) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

# 2. The Build Target (Depends on source files)
$(BUILD_DIR)/$(APP): $(SRCS) $(BUILD_DIR)/build.ninja
	@echo "--- Building $(APP) ---"
	@ninja -C $(BUILD_DIR)

build: $(BUILD_DIR)/$(APP)

# 3. Running & Debugging
run: $(BUILD_DIR)/$(APP)
	./$(BUILD_DIR)/$(APP)

debug:
	$(MAKE) build TYPE=Debug

gdb: debug
	gdb ./$(BUILD_DIR)/$(APP)

# 4. Specialized Tools (Restored)
asan:
	@echo "--- Building with AddressSanitizer ---"
	mkdir -p $(BUILD_DIR)_asan
	cmake -S . -B $(BUILD_DIR)_asan -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DUSE_SANITIZERS=ON \
		-DCMAKE_CXX_COMPILER=$(COMPILER)
	ninja -C $(BUILD_DIR)_asan
	./$(BUILD_DIR)_asan/$(APP)

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(BUILD_DIR)/$(APP)

tidy: $(BUILD_DIR)/build.ninja
	run-clang-tidy -p $(BUILD_DIR) -fix -header-filter='^src/.*' -- -system-headers=0

# 5. Maintenance
clean:
	rm -rf $(BUILD_DIR) $(BUILD_DIR)_asan compile_commands.json

reconfigure:
	rm -rf $(BUILD_DIR)
	$(MAKE) build