
BUILD_DIR := ./build


SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.h')


.PHONY: build
build:
	cmake --build build

run:
	./build/bin/Clox

configure:
	mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR)


fmt:
	clang-format --style=file:./.clang-format -i $(SRCS)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)/*