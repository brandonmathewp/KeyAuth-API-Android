#!/system/bin/sh
# Build KeyAuth fat static library on Android using NDK ndk-build

set -e

# ------------------------------------------------------------
# 1. NDK location
# ------------------------------------------------------------
NDK_ROOT="$HOME/android-sdk/ndk/29.0.14033849"
if [ ! -d "$NDK_ROOT" ]; then
    echo "ERROR: NDK not found at $NDK_ROOT"
    exit 1
fi

echo "[*] Using NDK: $NDK_ROOT"

# ------------------------------------------------------------
# 2. Add NDK tools to PATH
# ------------------------------------------------------------
PREBUILT_DIR="$NDK_ROOT/prebuilt/linux-arm64/bin"
if [ ! -d "$PREBUILT_DIR" ]; then
    PREBUILT_DIR="$NDK_ROOT/prebuilt/linux-x86_64/bin"
fi
export PATH="$NDK_ROOT:$PREBUILT_DIR:$PATH"

LLVM_BIN="$NDK_ROOT/toolchains/llvm/prebuilt/linux-arm64/bin"
if [ ! -d "$LLVM_BIN" ]; then
    LLVM_BIN="$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin"
fi
export PATH="$LLVM_BIN:$PATH"

export ANDROID_NDK_ROOT="$NDK_ROOT"

# ------------------------------------------------------------
# 3. Build core library
# ------------------------------------------------------------
echo "[*] Building Core Library (KeyAuth_Library)..."
if [ ! -f "KeyAuth_Library/jni/Android.mk" ]; then
    echo "ERROR: KeyAuth_Library/jni/Android.mk not found"
    exit 1
fi

cd KeyAuth_Library
ndk-build -B
if [ $? -ne 0 ]; then
    echo "ERROR: ndk-build failed for KeyAuth_Library"
    exit 1
fi
cd ..

# ------------------------------------------------------------
# 4. Merge dependencies into fat static library
# ------------------------------------------------------------
echo "[*] Merging dependencies into fat library..."

FAT_DIR="$PWD/fat_temp"
NEW_CORE_LIB="$PWD/KeyAuth_Library/obj/local/arm64-v8a/libkeyauth.a"
PREBUILT_LIB="$PWD/prebuilt/arm64-v8a/libkeyauth.a"
LIBS_VCPKG="$PWD/KeyAuth_Library/jni/libs_vcpkg"

rm -rf "$FAT_DIR"
mkdir -p "$FAT_DIR"

LLVM_AR="$LLVM_BIN/llvm-ar"
if [ ! -f "$LLVM_AR" ]; then
    LLVM_AR="llvm-ar"
fi

if [ -d "$LIBS_VCPKG" ]; then
    echo "[*] Found individual dependencies. Extracting all..."
    LIBS=(
        "$NEW_CORE_LIB"
        "$LIBS_VCPKG/libcurl.a"
        "$LIBS_VCPKG/libsodium.a"
        "$LIBS_VCPKG/libssl.a"
        "$LIBS_VCPKG/libcrypto.a"
        "$LIBS_VCPKG/libz.a"
    )
    for lib in "${LIBS[@]}"; do
        if [ -f "$lib" ]; then
            name=$(basename "$lib" .a)
            dir="$FAT_DIR/$name"
            mkdir -p "$dir"
            (cd "$dir" && "$LLVM_AR" x "$lib")
            echo "  Extracted: $lib"
        else
            echo "  Warning: $lib not found, skipping"
        fi
    done

elif [ -f "$PREBUILT_LIB" ]; then
    echo "[*] Prebuilt fat lib found. Updating with new core objects..."
    BASE_DIR="$FAT_DIR/base"
    mkdir -p "$BASE_DIR"
    (cd "$BASE_DIR" && "$LLVM_AR" x "$PREBUILT_LIB")
    rm -f "$BASE_DIR"/KeyAuth.o "$BASE_DIR"/oxorany.o 2>/dev/null

    NEW_DIR="$FAT_DIR/new"
    mkdir -p "$NEW_DIR"
    (cd "$NEW_DIR" && "$LLVM_AR" x "$NEW_CORE_LIB")
    mv "$NEW_DIR"/*.o "$BASE_DIR/" 2>/dev/null || true

else
    echo "[*] No deps or prebuilt. Using only new core lib."
    mkdir -p "$FAT_DIR/core"
    (cd "$FAT_DIR/core" && "$LLVM_AR" x "$NEW_CORE_LIB")
fi

OBJ_LIST="$PWD/objs.txt"
find "$FAT_DIR" -name "*.o" -type f | sed 's/^\.\///' > "$OBJ_LIST"

echo "[*] Creating final fat library: libkeyauth.a"
"$LLVM_AR" rc libkeyauth.a @"$OBJ_LIST"

rm -rf "$FAT_DIR" "$OBJ_LIST"

# ------------------------------------------------------------
# 5. Copy outputs to prebuilt and example directories
# ------------------------------------------------------------
echo "[*] Updating Prebuilt and Example..."
mkdir -p prebuilt/arm64-v8a
cp -f libkeyauth.a prebuilt/arm64-v8a/
cp -f KeyAuth_Library/jni/include/KeyAuth.hpp prebuilt/
cp -rf KeyAuth_Library/jni/include/Json prebuilt/ 2>/dev/null || true

mkdir -p KeyAuth_Example/jni/libs/arm64-v8a
cp -f libkeyauth.a KeyAuth_Example/jni/libs/arm64-v8a/
cp -f KeyAuth_Library/jni/include/KeyAuth.hpp KeyAuth_Example/jni/include/
cp -rf KeyAuth_Library/jni/include/Json KeyAuth_Example/jni/include/ 2>/dev/null || true

# ------------------------------------------------------------
# 6. Build example project
# ------------------------------------------------------------
echo "[*] Verifying build with Example Project..."
if [ ! -f "KeyAuth_Example/jni/Android.mk" ]; then
    echo "WARNING: KeyAuth_Example/jni/Android.mk not found, skipping example build"
else
    cd KeyAuth_Example
    ndk-build -B
    if [ $? -ne 0 ]; then
        echo "ERROR: Example project build failed"
        exit 1
    fi
    cd ..
fi

echo ""
echo "[+] SUCCESS! Fat library is located at: prebuilt/arm64-v8a/libkeyauth.a"
