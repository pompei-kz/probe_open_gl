.PHONY: configure build test clean rebuild

PRESET := debug-clang
BUILD_DIR := cmake-build-debug-clang

configure:
	cmake --preset $(PRESET)

build:
	cmake --build --preset $(PRESET)

test: configure build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

rebuild:
	rm -rf $(BUILD_DIR)
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
