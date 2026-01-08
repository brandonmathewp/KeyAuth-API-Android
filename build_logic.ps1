$NDK_ROOT = "your NDK path here"
$LLVM_AR = "$NDK_ROOT\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-ar.exe"
$cwd = (Get-Location).Path
$fat_dir = Join-Path $cwd "fat_temp"

if (Test-Path $fat_dir) { Remove-Item $fat_dir -Recurse -Force }
New-Item -ItemType Directory -Path $fat_dir

# Library paths
$new_core_lib = Join-Path $cwd "KeyAuth_Library\obj\local\arm64-v8a\libkeyauth.a"
$prebuilt_lib = Join-Path $cwd "prebuilt\arm64-v8a\libkeyauth.a"
$libs_vcpkg = Join-Path $cwd "KeyAuth_Library\jni\libs_vcpkg"

if (Test-Path $libs_vcpkg) {
    Write-Host "[*] Found individual dependency libraries. Performing fresh merge..."
    $libs = @(
        $new_core_lib,
        (Join-Path $libs_vcpkg "libcurl.a"),
        (Join-Path $libs_vcpkg "libsodium.a"),
        (Join-Path $libs_vcpkg "libssl.a"),
        (Join-Path $libs_vcpkg "libcrypto.a"),
        (Join-Path $libs_vcpkg "libz.a")
    )
    foreach ($lib in $libs) {
        if (Test-Path $lib) {
            $libName = [System.IO.Path]::GetFileNameWithoutExtension($lib)
            $extractDir = Join-Path $fat_dir $libName
            New-Item -ItemType Directory -Path $extractDir
            Set-Location $extractDir
            & $LLVM_AR x $lib
            Set-Location $cwd
        }
    }
} elseif (Test-Path $prebuilt_lib) {
    Write-Host "[*] Dependencies not found but found prebuilt fat library. Updating it..."
    # 1. Extract the existing fat library
    $extractDir = Join-Path $fat_dir "base_lib"
    New-Item -ItemType Directory -Path $extractDir
    Set-Location $extractDir
    & $LLVM_AR x $prebuilt_lib
    
    # 2. Remove old core objects
    Remove-Item "KeyAuth.o" -ErrorAction SilentlyContinue
    Remove-Item "oxorany.o" -ErrorAction SilentlyContinue
    
    # 3. Extract the new core lib
    $newExtractDir = Join-Path $fat_dir "new_core"
    New-Item -ItemType Directory -Path $newExtractDir
    Set-Location $newExtractDir
    & $LLVM_AR x $new_core_lib
    
    # Move new objects to base dir
    Move-Item "*.o" "$extractDir/" -Force
} else {
    Write-Host "[!] Could not find dependencies or prebuilt. Just using the new core lib as initial fat lib."
    $extractDir = Join-Path $fat_dir "initial_core"
    New-Item -ItemType Directory -Path $extractDir
    Set-Location $extractDir
    & $LLVM_AR x $new_core_lib
}

Set-Location $cwd
$objs = Get-ChildItem -Path $fat_dir -Recurse -Filter *.o | Select-Object -ExpandProperty FullName
$objs = $objs -replace '\\', '/'
[System.IO.File]::WriteAllLines("$cwd/objs.txt", $objs)

Write-Host "[*] Creating final fat library..."
cmd /c """$LLVM_AR"" rc libkeyauth.a @objs.txt"

# Cleanup
Remove-Item $fat_dir -Recurse -Force
Remove-Item objs.txt -Force
