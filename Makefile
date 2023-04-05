
BUILD_DIR := ./build


build:
	cmake --build

run:
	./build/Clox

configure:
	mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
