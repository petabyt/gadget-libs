OUT_DIR := ../app/src/main/assets
LIBPAK_DIR := ../libpak
CMAKE_FLAGS := -DCMAKE_TOOLCHAIN_FILE=$(PWD)/$(LIBPAK_DIR)/toolchain/toolchain.cmake -DMAKE_EXECUTABLE=ON -DCMAKE_PROJECT_INCLUDE=$(PWD)/$(LIBPAK_DIR)/toolchain/pakrt.cmake

# $1 directory
# $2 manifest filename
# $3 script filename
define compile_js
	jq --arg hash "`git rev-parse --short HEAD`" '.gitHash = $$hash' $1$2 > $(OUT_DIR)/$2
	esbuild $1$3 > /dev/zero
	cp $1$3 $(OUT_DIR)/$3
endef

# $1 directory
# $2 manifest filename
# $3 wasm executable output filename
define compile_cmake
	cmake $(CMAKE_FLAGS) -G Ninja -B build/$3 -S $1
	cmake --build build/$3
	jq --arg hash "`git rev-parse --short HEAD`" '.gitHash = $$hash' $1$2 > $(OUT_DIR)/$2
	cp build/$3/$3 $(OUT_DIR)/$3.wasm
endef

install:
	mkdir -p build
	$(call compile_js,veement/,veement.json,veement.js)
	$(call compile_js,viofo/,viofo.json,viofo.js)

install_full: install
	$(call compile_cmake,dummy/,dummy.json,dummy)
#$(call compile_cmake,libfuji/,libfuji.json,libfuji)
	$(call compile_cmake,ptp2/,ptp2.json,ptp2)
#$(call compile_cmake,furble/glue/,libfuji.json,libfuji)

clean:
	rm -rf $(OUT_DIR)/* build/
