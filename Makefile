linux:
	cmake -Wno-author -Wno-unused-cli -Bbuild -GNinja
	cmake --build build

windows:
	cmake -Wno-author -Wno-unused-cli -Bbuild -GNinja \
	-DCMAKE_TOOLCHAIN_FILE=./cmake/toolchains/mingw64.toolchain \
	-DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++" \
	-DBUILD_TESTING=OFF \
	-DVULKAN=ON \
	-DVULKAN_INCLUDE_DIRS="$(CURDIR)/ddnet-libs/vulkan/include" \
	-DVULKAN_LIBRARIES="$(CURDIR)/ddnet-libs/vulkan/windows/lib64/vulkan-1.lib"
	cmake --build build
