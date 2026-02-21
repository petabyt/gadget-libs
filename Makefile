OUT_DIR := ../app/src/main/assets

install: install_libfuji install_veement

compile_libfuji:
	cmake -DCMAKE_TOOLCHAIN_FILE=../wasm.cmake -G Ninja -B libfuji/build libfuji
	cmake --build libfuji/build
install_libfuji:
	cp libfuji.json $(OUT_DIR)/

install_veement:
	cp veement/veement.json $(OUT_DIR)/
	cp veement/veement.js $(OUT_DIR)/
