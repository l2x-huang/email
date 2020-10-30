CMAKE_COMMON = cmake -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_VCPKG_TOOLCHAINS)
nproc = $(shell expr $(shell nproc) + 1)

.PHONY: all build.debug build.release ninja.debug ninja.release clean

build.debug:
	cmake --build build/Debug -j $(nproc)

build.release:
	cmake --build build/Release -j $(nproc)

clangd: ninja.debug
	$(CMAKE_COMMON) -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -Bbuild/Debug
	mv build/Debug/compile_commands.json .

ninja.debug:
	$(CMAKE_COMMON) -G Ninja -DCMAKE_BUILD_TYPE=Debug -Bbuild/Debug

ninja.release:
	$(CMAKE_COMMON) -G Ninja -DCMAKE_BUILD_TYPE=Release -Bbuild/Release

clean:
	rm -rf build
