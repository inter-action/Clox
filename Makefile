
BUILD_DIR := ./build


.PHONY: build
build:
	cmake --build build

run:
	./build/bin/Clox

configure:
	mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)/*
