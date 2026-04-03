# KeyAuth C++ Android (Fat Static Library)

A high-performance, standalone version of the KeyAuth C++ library for Android. This version bundles all dependencies into a single static library, simplifying your build process and improving security with built-in string obfuscation.

## Why use this?
*   **All-in-one:** No more messing with separate `libcurl`, `libsodium`, or `openssl` files. Everything is inside `libkeyauth.a`.
*   **Encrypted Strings:** Uses `OBFUSCATE` to automatically protect your sensitive strings from memory scanners and reverse engineering.
*   **Plug & Play:** Just include one header and link one library. It's that simple.

## Project Structure
*   `KeyAuth_Library/`: The core engine. Modify logic and rebuild from here.
*   `KeyAuth_Example/`: A menu-driven sample app to test Login, Register, and more.
*   `prebuilt/`: Ready-to-use binaries for your own projects.

## Quick Start
### Option 1: ndk-build
1.  Copy the `prebuilt/` folder to your Android project.
2.  Update your `Android.mk`:
    ```makefile
    # Import the library
    include $(CLEAR_VARS)
    LOCAL_MODULE    := keyauth
    LOCAL_SRC_FILES := path/to/prebuilt/arm64-v8a/libkeyauth.a
    include $(PREBUILT_STATIC_LIBRARY)
    
    # Link it to your app
    LOCAL_STATIC_LIBRARIES := keyauth
    LOCAL_LDLIBS := -llog -landroid
    ```

### Option 2: CMake
In your `CMakeLists.txt`, add the following:
```cmake
# Define the library
add_library(keyauth STATIC IMPORTED)
set_target_properties(keyauth PROPERTIES 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/path/to/prebuilt/${ANDROID_ABI}/libkeyauth.a"
)

# Link to your target
target_link_libraries(your_app_target keyauth log android)
```
3.  Include it in your code: `#include "KeyAuth.hpp"`.

## Windows Configuration
Before building, you **must** set your Android NDK path in both files:
1.  In `build.bat`: Update `set "NDK_ROOT=..."`
2.  In `build_logic.ps1`: Update `$NDK_ROOT = "..."`

## Android/Linux Configuration
Before building, you **must** set your Android NDK path:
1.  Make the `build.sh` file executable by running `chmod +x build.sh`.
2.  In `build.sh`: Update `NDK_ROOT="$HOME/..."`

## Cleaning
Running `build.sh clean` will clean temporary folders and build files.

## Automation
Running the build script at the root will automatically:
1.  Recompile the core library.
2.  Merge it with dependencies into a fresh `libkeyauth.a`.
3.  Update the `prebuilt/` and `KeyAuth_Example/` folders.

## Credits
Built using these open-source projects:
*   [KeyAuth](https://keyauth.cc/) - Licensing & Auth
*   [Obfuscate](https://github.com/JungliCode/obfuscator) - String Obfuscation
*   [Libcurl](https://curl.se/) - Network
*   [Libsodium](https://doc.libsodium.org/) - Encryption
*   [OpenSSL](https://www.openssl.org/) - SSL/TLS
*   [Nlohmann JSON](https://github.com/nlohmann/json) - JSON parsing
