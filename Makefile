
BUILD_DIR := ./build


SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.h')


run:
	./build/bin/Cloxd

.PHONY: build
build:
	cmake --build build

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*

# build debug version by default
configure: clean
	mkdir -p $(BUILD_DIR)
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B $(BUILD_DIR)


fmt:
	clang-format --style=file:./.clang-format -i $(SRCS)

debug_gdb:
	gdb ./build/bin/Cloxd
