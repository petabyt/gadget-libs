SET (CMAKE_SYSTEM_NAME Linux)
SET (CMAKE_SYSTEM_PROCESSOR wasm32)
SET (CMAKE_SYSROOT ${CMAKE_CURRENT_LIST_DIR}/../third_party/wasm-micro-runtime/wamr-sdk/app/libc-builtin-sysroot)

SET (CMAKE_C_FLAGS                  "-nostdlib -z stack-size=4096 -I${CMAKE_CURRENT_LIST_DIR}/include"   CACHE INTERNAL "")
SET (CMAKE_C_COMPILER_TARGET        "wasm32")
SET (CMAKE_C_COMPILER               "/usr/bin/clang")

SET (CMAKE_CXX_FLAGS                "-nostdlib -z stack-size=4096 -I${CMAKE_CURRENT_LIST_DIR}/include"   CACHE INTERNAL "")
SET (CMAKE_CXX_COMPILER_TARGET      "wasm32")
SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

SET (CMAKE_EXE_LINKER_FLAGS "-Wl,--initial-memory=65536,--no-entry,--export=get_module" CACHE INTERNAL "")
SET (CMAKE_SHARED_LINKER_FLAGS "-Wl,--initial-memory=65536,--no-entry" CACHE INTERNAL "")

SET (CMAKE_LINKER  "/usr/bin/wasm-ld"                     CACHE INTERNAL "")
SET (CMAKE_AR      "/usr/bin/llvm-ar"                     CACHE INTERNAL "")
SET (CMAKE_NM      "/usr/bin/llvm-nm"                     CACHE INTERNAL "")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-dwarfdump"              CACHE INTERNAL "")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib"                 CACHE INTERNAL "")
SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS},--allow-undefined-file=${CMAKE_CURRENT_LIST_DIR}/symbols.txt" CACHE INTERNAL "")
