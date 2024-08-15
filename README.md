# FileProtector Project

## Overview

The FileProtector project aims to secure files through encryption and other protective measures. 
> [!IMPORTANT]\
> This project relies on several third-party libraries, including `nlohmann/json`, `OpenCV` and `OpenSSL`, which are compiled from source to create static libraries for use in this project. 

## Third-Party Libraries

### OpenCV 5.x
[OpenCV 5.x Repository](https://github.com/opencv/opencv/tree/5.x)

#### Building from Source

1. **Select the Generator in [CMake](https://cmake.org/download):**
   - Choose `Visual Studio 2022` as the generator.

2. **Set the following flags in CMake:**
   ```cmake
   WITH_JASPER:BOOL=0
   BUILD_opencv_world:BOOL=1
   BUILD_opencv_java_bindings_generator:BOOL=0
   BUILD_SHARED_LIBS:BOOL=0
   BUILD_opencv_python_tests:BOOL=0
   BUILD_TESTS:BOOL=0
   BUILD_JASPER:BOOL=0
   BUILD_JAVA:BOOL=0
   BUILD_opencv_python_bindings_generator:BOOL=0
   BUILD_WITH_DEBUG_INFO:BOOL=0
   BUILD_opencv_js_bindings_generator:BOOL=0
   ```

3. **Build Steps:**
   - After generating the project, locate `OpenCV.sln` in the specified CMake directory.
   - Build the `ALL_BUILD` configuration and then `INSTALL`.
   - The necessary files are located in:
     - `install/include/opencv2`
     - `install/x64/vc17/staticlib`

### OpenSSL 3.3.1 is used
[OpenSSL 3.3.1 Source](https://www.openssl.org/source/)

#### Dependencies

1. [Perl](https://strawberryperl.com/)
2. [NASM](https://www.nasm.us/)
3. [Microsoft Visual C++ (MSVC) C compiler](https://visualstudio.microsoft.com/vs/features/cplusplus/) (included with [Visual Studio](https://visualstudio.microsoft.com))

#### Building from Source

1. **Install Dependencies:**
   - Ensure Perl and NASM are in the PATH and accessible from the command line.

2. **Build Steps:**
   - Open ["x64 Native Tools Command Prompt for VS 2022"](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022) as an administrator.
   - Navigate to the OpenSSL directory (e.g., `openssl-3.3.1\openssl-3.3.1`).
   - Execute the following commands:

   For Debug version:
   ```shell
   perl Configure debug-VC-WIN64A no-shared
   nmake
   nmake test
   nmake install
   ```

   For Release version:
   ```shell
   nmake clean
   perl Configure VC-WIN64A no-shared
   nmake
   nmake test
   nmake install
   ```

### nlohmann/json

[nlohmann/json](https://github.com/nlohmann/json) is single header file library, no compilation needed.

## Project Structure

1. **OpenCV**
   - **Include Directory:**
     - `opencv\build\install\include`
   - **Additional Dependencies:**
     - `FileProtector\opencv\build\install\x64\vc17\staticlib`

2. **nlohmann json**
   - **Include Directory:**
     - `nlohmann`
   - **Notes:**
     - The library has no DLL or LIB files.

3. **OpenSSL**
   - **Include Directory:**
     - `OpenSSL\include`
   - **Library Directory:**
     - `OpenSSL\lib`

### The project structure is organized as follows:

```
FileEncrypter
├── FileProtector
│   ├── config.cpp
│   ├── config.h
│   ├── console.cpp
│   ├── console.h
│   ├── core.cpp
│   ├── file_operations.cpp
│   ├── hash_operations.cpp
│   ├── icon.ico
│   ├── icons
│   │   ├── icon.ico
│   │   ├── icon1.ico
│   │   ├── icon2.ico
│   │   ├── icon3.ico
│   │   └── icon4.ico
│   ├── image_display.cpp
│   ├── imghider.h
│   ├── main.cpp
│   ├── nlohmann
│   │   ├── json.hpp
│   │   └── json_fwd.hpp
│   ├── opencv
│   │   └── build
│   │       └── install
│   │           ├── include
│   │           │   └── ◯ ◯ ◯
│   │           └── x64
│   │               └── vc17
│   │                   └── staticlib
│   │                       └── ◯ ◯ ◯
│   ├── Resource.aps
│   ├── resource.h
│   ├── Resource.rc
│   ├── stdafx.cpp
│   ├── stdafx.h
│   ├── utils.cpp
│   ├── x64Debug.props
│   └── x64Release.props
├── FileProtector.sln
├── HashingLib
│   ├── framework.h
│   ├── hashing.cpp
│   ├── hashing.h
│   ├── OpenSSL
│   │   ├── include
│   │   │   └── ◯ ◯ ◯
│   │   └── lib
│   │       └── ◯ ◯ ◯
│   ├── openSSL_x64.props
│   ├── pch.cpp
│   └── pch.h
├── Python encoding conversion
│   ├── main.py
│   └── requirements.txt
├── VideoEncryptorLib
│   ├── framework.h
│   ├── pch.cpp
│   ├── pch.h
│   ├── rcc.cpp
│   ├── VideoEncryptor.cpp
│   └── VideoEncryptor.h
└── x64
    └── Debug & Release
        ├── FileProtector.exe
        ├── HashingLib.lib
        └── VideoEncryptorLib.lib
```

## Building the Project

To successfully build the project, use the project properties from the `.props` files. Ensure the following settings are applied:

- **FileProtector**: Uses the `x64Debug.props` and `x64Release.props`.
- **HashingLib**: Uses the `openSSL_x64.props`.

This configuration ensures all necessary static libraries and include files are correctly referenced for a successful build.

> [!IMPORTANT]\
>  Project uses c++ standard c++20
>  Use `Runtime Library: Multi-threaded (/MT)` for correct compilation

> [!NOTE]\
> You can set up the project props yourself as follows:
> - `FileProtector project:`
>   - **Include Directories:** ../HashingLib;../VideoEncryptorLib;opencv\build\install\include;nlohmann;
>   - **Library Directories:** `..\x64\{Debug|Release};opencv\build\install\x64\vc17\staticlib;..\HashingLib\OpenSSL\lib`, where `..\x64\{Debug|Release}` is output directory for `HashingLib` and `VideoEncryptorLib` projects.
>   - **Additional Dependencies:**
>     - **Debug:** `aded.lib;IlmImfd.lib;ippicvmt.lib;ippiwd.lib;ittnotifyd.lib;libclapackd.lib;libjpeg-turbod.lib;libopenjp2d.lib;libpngd.lib;libprotobufd.lib;libtiffd.lib;libwebpd.lib;opencv_world500d.lib;zlibd.lib;libcrypto.lib;libssl.lib;ws2_32.lib;crypt32.lib;HashingLib.lib;VideoEncryptorLib.lib`
>     - **Release:** `ade.lib;IlmImf.lib;ippicvmt.lib;ippiw.lib;ittnotify.lib;libclapack.lib;libjpeg-turbo.lib;libopenjp2.lib;libpng.lib;libprotobuf.lib;libtiff.lib;libwebp.lib;opencv_world500.lib;zlib.lib;libcrypto.lib;libssl.lib;ws2_32.lib;crypt32.lib;HashingLib.lib;VideoEncryptorLib.lib`
> - `HashingLib:`
>   - **Include Directories:** `OpenSSL\include`
>   - **Library Directories:** `OpenSSL\lib`

> [!NOTE]\
> You can change the app icon by copying icons from `FileProtector/icons` to `FileProtector` and renaming it to `icon.ico`.

## File Encoding

To successfully upload to GitHub, the file encoding has been changed to UTF-8. If characters appear incorrectly after downloading the repository, run `Python encoding conversion\main.py`, installing requirements.txt and select `windows-1251` encoding.
