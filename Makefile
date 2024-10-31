# .PHONY: prepare
# prepare:
# 	rm -rf build && mkdir build

.PHONY: default
default: build

.PHONY: cmake
cmake:
	cmake -B build -S .  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug

.PHONY: format
format:
	cmake --build ./build --target format

.PHONY: build
build:
	# cd build && make -j
	# cmake --build build --config Release
	cmake --build build --config Debug
