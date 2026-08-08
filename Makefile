OUT_DIR := ../app/src/main/assets
LIBPAK_DIR := ../libpak
CMAKE_FLAGS := -DCMAKE_TOOLCHAIN_FILE=$(PWD)/$(LIBPAK_DIR)/toolchain/toolchain.cmake -DCMAKE_PROJECT_INCLUDE=$(PWD)/$(LIBPAK_DIR)/toolchain/pakrt.cmake

install: install_veement install_viofo

compile_libfuji:
	cmake -DCMAKE_TOOLCHAIN_FILE=../$(LIBPAK_DIR)/toolchain/toolchain.cmake -G Ninja -B libfuji/wasmbuild -S libfuji/
	cmake --build libfuji/wasmbuild
install_libfuji:
	jq . libfuji/libfuji.json
	cp libfuji/libfuji.json $(OUT_DIR)/

compile_ptp2:
	cmake $(CMAKE_FLAGS) -G Ninja -B ptp2/wasmbuild -S ptp2/
	cmake --build ptp2/wasmbuild

compile_libfurble:
	cmake $(CMAKE_FLAGS) -G Ninja -B libfurble/glue/wasmbuild -S libfurble/glue/
	cmake --build libfurble/glue/wasmbuild

define compile_js
	jq --arg hash "`git rev-parse --short HEAD`" '.gitHash = $$hash' $1$2
	jq . $1$2 > $(OUT_DIR)/$2
	esbuild $1$3 > /dev/zero
	cp $1$3 $(OUT_DIR)/$3
endef

install_veement:
	$(call compile_js,veement/,veement.json,veement.js)

install_viofo:
	$(call compile_js,viofo/,viofo.json,viofo.js)

clean:
	rm -rf $(OUT_DIR)/*
