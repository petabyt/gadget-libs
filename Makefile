OUT_DIR := ../app/src/main/assets

install: install_veement install_viofo

compile_libfuji:
	cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja -B libfuji/wasmbuild -S libfuji/
	cmake --build libfuji/wasmbuild
install_libfuji:
	jq . libfuji/libfuji.json
	cp libfuji/libfuji.json $(OUT_DIR)/

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
