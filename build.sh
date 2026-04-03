#!/system/bin/sh
# Build KeyAuth fat static library on Android using NDK ndk-build

set -e

# ------------------------------------------------------------
# 1. Configuration & Path Setup
# ------------------------------------------------------------
NDK_ROOT="$HOME/android-sdk/ndk/29.0.14033849"
FAT_DIR="$PWD/fat_temp"
OBJ_LIST="$PWD/objs.txt"

# Utility function to wipe build artifacts
clean_build() {
    echo "[*] Cleaning build environment..."
    rm -rf "$FAT_DIR"
    rm -rf "$OBJ_LIST"
    rm -rf "libkeyauth.a"
    rm -rf "KeyAuth_Library/obj"
    rm -rf "KeyAuth_Library/libs"
    rm -rf "KeyAuth_Example/obj"
    rm -rf "KeyAuth_Example/libs"
    rm -rf "KeyAuth_Example/jni/libs"
    echo "[*] Cleaning done"
}

if [ "$1" = "clean" ]; then
    clean_build
    exit 0
fi

if [ ! -d "$NDK_ROOT" ]; then
    echo "ERROR: NDK not found at $NDK_ROOT"
    exit 1
fi

# Detect architecture for toolchain
HOST_TAG="linux-arm64"
[ ! -d "$NDK_ROOT/prebuilt/$HOST_TAG" ] && HOST_TAG="linux-x86_64"

export PATH="$NDK_ROOT:$NDK_ROOT/prebuilt/$HOST_TAG/bin:$NDK_ROOT/toolchains/llvm/prebuilt/$HOST_TAG/bin:$PATH"
export ANDROID_NDK_ROOT="$NDK_ROOT"
LLVM_AR="llvm-ar"

# ------------------------------------------------------------
# 2. Build Core Library
# ------------------------------------------------------------
echo "[*] Building Core Library (KeyAuth_Library)..."
if [ ! -f "KeyAuth_Library/jni/Android.mk" ]; then
    echo "ERROR: KeyAuth_Library/jni/Android.mk not found"
    exit 1
fi

(cd KeyAuth_Library && ndk-build -B)

# ------------------------------------------------------------
# 3. Merge Dependencies
# ------------------------------------------------------------
echo "[*] Merging dependencies into fat library..."
NEW_CORE_LIB="$PWD/KeyAuth_Library/obj/local/arm64-v8a/libkeyauth.a"
PREBUILT_LIB="$PWD/prebuilt/arm64-v8a/libkeyauth.a"
LIBS_VCPKG="$PWD/KeyAuth_Library/jni/libs_vcpkg"

rm -rf "$FAT_DIR" && mkdir -p "$FAT_DIR"

if [ -d "$LIBS_VCPKG" ]; then
    echo "[*] Extracting individual dependencies..."
    LIBS=("$NEW_CORE_LIB" "$LIBS_VCPKG/libcurl.a" "$LIBS_VCPKG/libsodium.a" "$LIBS_VCPKG/libssl.a" "$LIBS_VCPKG/libcrypto.a" "$LIBS_VCPKG/libz.a")
    for lib in "${LIBS[@]}"; do
        if [ -f "$lib" ]; then
            dir="$FAT_DIR/$(basename "$lib" .a)"
            mkdir -p "$dir" && (cd "$dir" && "$LLVM_AR" x "$lib")
        fi
    done
elif [ -f "$PREBUILT_LIB" ]; then
    echo "[*] Updating existing fat lib..."
    BASE_DIR="$FAT_DIR/base"
    mkdir -p "$BASE_DIR" && (cd "$BASE_DIR" && "$LLVM_AR" x "$PREBUILT_LIB")
    rm -f "$BASE_DIR"/KeyAuth.o "$BASE_DIR"/oxorany.o 2>/dev/null
    
    NEW_DIR="$FAT_DIR/new"
    mkdir -p "$NEW_DIR" && (cd "$NEW_DIR" && "$LLVM_AR" x "$NEW_CORE_LIB")
    mv "$NEW_DIR"/*.o "$BASE_DIR/" 2>/dev/null || true
else
    mkdir -p "$FAT_DIR/core" && (cd "$FAT_DIR/core" && "$LLVM_AR" x "$NEW_CORE_LIB")
fi

find "$FAT_DIR" -name "*.o" -type f > "$OBJ_LIST"
"$LLVM_AR" rc libkeyauth.a @"$OBJ_LIST"
rm -rf "$FAT_DIR" "$OBJ_LIST"

# ------------------------------------------------------------
# 4. Deployment
# ------------------------------------------------------------
echo "[*] Updating Prebuilt and Example..."
# Sync Prebuilt
mkdir -p prebuilt/arm64-v8a
cp -f libkeyauth.a prebuilt/arm64-v8a/
cp -f KeyAuth_Library/jni/include/KeyAuth.hpp prebuilt/
cp -rf KeyAuth_Library/jni/include/Json prebuilt/ 2>/dev/null || true

# Sync Example
mkdir -p KeyAuth_Example/jni/libs/arm64-v8a
cp -f libkeyauth.a KeyAuth_Example/jni/libs/arm64-v8a/
cp -f KeyAuth_Library/jni/include/KeyAuth.hpp KeyAuth_Example/jni/include/
cp -rf KeyAuth_Library/jni/include/Json KeyAuth_Example/jni/include/ 2>/dev/null || true

# ------------------------------------------------------------
# 5. Verification
# ------------------------------------------------------------
if [ -f "KeyAuth_Example/jni/Android.mk" ]; then
    echo "[*] Verifying build with Example Project..."
    (cd KeyAuth_Example && ndk-build -B)
fi

echo "\n[+] SUCCESS! libkeyauth.a is ready."
