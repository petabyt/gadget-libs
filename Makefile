OUT_DIR := ../app/src/main/assets

install: install_veement install_viofo

compile_libfuji:
	cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja -B build
	cmake --build libfuji/build
install_libfuji:
	jq . libfuji/libfuji.json
	cp libfuji/libfuji.json $(OUT_DIR)/

install_veement:
	jq . veement/veement.json > $(OUT_DIR)/veement.json
	esbuild veement/veement.js > /dev/zero
	cp veement/veement.js $(OUT_DIR)/veement.js

install_viofo:
	jq . viofo/viofo.json > $(OUT_DIR)/viofo.json
	esbuild viofo/viofo.js > /dev/zero
	cp viofo/viofo.js $(OUT_DIR)/viofo.js

clean:
	rm -rf $(OUT_DIR)/*
