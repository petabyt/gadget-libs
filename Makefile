OUT_DIR := ../app/src/main/assets
BUILD_DIR := build
LIBPAK_DIR := ../libpak
CMAKE_FLAGS := -DCMAKE_TOOLCHAIN_FILE=$(PWD)/$(LIBPAK_DIR)/toolchain/toolchain.cmake -DMAKE_EXECUTABLE=ON -DCMAKE_PROJECT_INCLUDE=$(PWD)/$(LIBPAK_DIR)/toolchain/pakrt.cmake
CMAKE_FLAGS += -DCMAKE_BUILD_TYPE=Release
CMAKE_FLAGS += -DPTP_NO_USB=ON -DLIBFUJI_MODULE=ON
CERT_LOC := ~/.fudge.pem

$(CERT_LOC):
	openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:3072 -out $(CERT_LOC)

# $1 directory
# $2 manifest filename
# $3 script output filename
define process_manifest
	jq \
	  --arg s `openssl dgst -sha256 -sign ~/.fudge.pem $(OUT_DIR)/$3 | base64 -w0` \
	  --arg k `openssl rsa -in ~/.fudge.pem -pubout -outform DER | base64 -w0` \
	  --arg hash "`git rev-parse --short HEAD`" \
	  '.signature = {alg:"RS256", key:$$k, value:$$s} | .gitHash = $$hash' \
	  $1$2 > $(OUT_DIR)/$2
endef

# $1 directory
# $2 manifest filename
# $3 script filename
define compile_js
	jq --arg hash "`git rev-parse --short HEAD`" '.gitHash = $$hash' $1$2 > $(OUT_DIR)/$2
	esbuild $1$3 > /dev/zero
	cp $1$3 $(OUT_DIR)/$3
	$(call process_manifest,$1,$2,$3)
endef

# $1 directory
# $2 manifest filename
# $3 wasm executable output filename
define compile_cmake
	cmake $(CMAKE_FLAGS) -G Ninja -B $(BUILD_DIR)/$3 -S $1
	cmake --build $(BUILD_DIR)/$3
	wasm-opt -Oz $(BUILD_DIR)/$3/$3 -o $(BUILD_DIR)/$3/$3.opt
	wasm-strip $(BUILD_DIR)/$3/$3.opt
	cp $(BUILD_DIR)/$3/$3.opt $(OUT_DIR)/$3.wasm
	$(call process_manifest,$1,$2,$3.wasm)
endef

define add_manifest
	jq --arg hash "`git rev-parse --short HEAD`" '.gitHash = $$hash' $1$2 > $(OUT_DIR)/$2
endef

install_fudge: $(CERT_LOC)
	mkdir -p $(BUILD_DIR)
	$(call compile_js,veement/,veement.json,veement.js)
	$(call compile_js,viofo/,viofo.json,viofo.js)
	$(call add_manifest,libfuji/,libfuji.json)
	$(call add_manifest,nothing-buds/,nothing.json)
	$(call add_manifest,goveelife/,goveelife.json)

install_full: $(CERT_LOC)
	mkdir -p $(BUILD_DIR)
	$(call compile_js,veement/,veement.json,veement.js)
	$(call compile_js,viofo/,viofo.json,viofo.js)
	$(call compile_cmake,goveelife/,goveelife.json,goveelife)
	$(call compile_cmake,dummy/,dummy.json,dummy)
	$(call compile_cmake,nothing-buds/,nothing.json,cmfnothingaudio)
	$(call compile_cmake,libfuji/,libfuji.json,fuji)
	$(call compile_cmake,ptp2/,ptp2.json,ptp2)
	$(call compile_cmake,libfurble/glue/,furble.json,furble)

install_full_www: $(CERT_LOC)
	$(MAKE) install_full OUT_DIR=$(PWD)/../../fudge-www/modules

clean:
	rm -rf $(OUT_DIR)/* build/
