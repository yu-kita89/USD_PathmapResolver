set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Configuration ###
set(AR_PROJECT_NAME UsdAssetResolver)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist"
    CACHE PATH "Default install dir")

### Init ###
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(${AR_PROJECT_NAME} LANGUAGES CXX)

# Pathmap Resolver
set(AR_PATHMAPRESOLVER_TARGET_LIB pathmapResolver)
set(AR_PATHMAPRESOLVER_USD_PLUGIN_NAME ${AR_PATHMAPRESOLVER_TARGET_LIB})
set(AR_PATHMAPRESOLVER_USD_CXX_CLASS_NAME ArPathmapResolver)
# python
set(AR_PATHMAPRESOLVER_TARGET_PYTHON _${AR_PATHMAPRESOLVER_TARGET_LIB})
set(AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME usdCustom)
set(AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME PathmapResolver)
set(AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_FULLNAME ${AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME}.${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME})

# Arch
if (WIN32)
    set(AR_ARCH_LIB_SUFFIX "dll")
    set(AR_ARCH_DIR "win")
elseif(LINUX)
    set(AR_ARCH_LIB_SUFFIX "so")
    set(AR_ARCH_DIR "lnx")
else()
    message(FATAL_ERROR "Unsupported platform. Only Windows and Linux are supported.")
endif()

# Houdini
set(AR_HOUDINI_ROOT $ENV{HFS} CACHE PATH "Houdini install directory")
set(AR_HOUDINI_INCLUDE_DIR ${AR_HOUDINI_ROOT}/toolkit/include)

# Usd
if (WIN32)
    set(AR_PXR_LIB_DIR ${AR_HOUDINI_ROOT}/custom/houdini/dsolib)
    set(AR_PXR_LIB_PREFIX "libpxr_")
else()
    set(AR_PXR_LIB_DIR ${AR_HOUDINI_ROOT}/dsolib)
    set(AR_PXR_LIB_PREFIX "pxr_")
endif()
set(AR_PXR_INCLUDE_DIR ${AR_HOUDINI_INCLUDE_DIR})

# Python
if (WIN32)
    if(EXISTS "${AR_HOUDINI_ROOT}/python311")
        set(AR_PYTHON_LIB python3.11)
        set(AR_PYTHON_LIB_NUMBER python311)
    elseif(EXISTS "${AR_HOUDINI_ROOT}/python310")
        set(AR_PYTHON_LIB python3.10)
        set(AR_PYTHON_LIB_NUMBER python310)
    else()
        set(AR_PYTHON_LIB python3.9)
        set(AR_PYTHON_LIB_NUMBER python39)
    endif()
else()
    if(EXISTS "${AR_HOUDINI_ROOT}/python/bin/python3.11")
        set(AR_PYTHON_LIB python3.11)
        set(AR_PYTHON_LIB_NUMBER python311)
    elseif(EXISTS "${AR_HOUDINI_ROOT}/python/bin/python3.10")
        set(AR_PYTHON_LIB python3.10)
        set(AR_PYTHON_LIB_NUMBER python310)
    else()
        set(AR_PYTHON_LIB python3.9)
        set(AR_PYTHON_LIB_NUMBER python39)
    endif()
endif()
if (WIN32)
    set(AR_PYTHON_LIB_DIR ${AR_HOUDINI_ROOT}/${AR_PYTHON_LIB_NUMBER}/libs)
else()
    set(AR_PYTHON_LIB_DIR ${AR_HOUDINI_ROOT}/python/lib)
endif()
set(AR_PYTHON_INCLUDE_DIR ${AR_HOUDINI_INCLUDE_DIR}/${AR_PYTHON_LIB})

# Boost
set(AR_BOOST_NAMESPACE hboost)
if (WIN32)
    set(AR_BOOST_PYTHON_LIB ${AR_BOOST_NAMESPACE}_${AR_PYTHON_LIB_NUMBER}-mt-x64)
else()
    set(AR_BOOST_PYTHON_LIB ${AR_BOOST_NAMESPACE}_${AR_PYTHON_LIB_NUMBER})
endif()
set(AR_BOOST_INCLUDE_DIR ${AR_HOUDINI_INCLUDE_DIR}/${AR_BOOST_NAMESPACE})

### CPP Settings ###
set(BUILD_SHARED_LIBS ON)
# Preprocessor Defines (Same as #define)
add_compile_definitions(HBOOST_ALL_NO_LIB BOOST_ALL_NO_LIB)
if (WIN32)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
else()
    # Notes:
    # Houdini 20 - Switched to the new C++11 ABI for Linux https://www.sidefx.com/docs/houdini/news/20/platforms.html
    # For Houdini versions that use gcc 9.3, please set this to _GLIBCXX_USE_CXX11_ABI=0

    file(REAL_PATH ${AR_HOUDINI_ROOT} AR_HOUDINI_ROOT_RESOLVED)
    string(FIND ${AR_HOUDINI_ROOT_RESOLVED} "19.5" AR_HOUDINI_ROOT_IS_H195)
    if (${AR_HOUDINI_ROOT_IS_H195} STREQUAL "-1")
        add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)
    else()
        message("Houdini 19.5 detected, switching to _GLIBCXX_USE_CXX11_ABI=0")
        add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()
# Compiler Options
if (WIN32)
    add_compile_options(/Zc:inline- /w)
else()
    add_compile_options(-fPIC -Wno-deprecated -Wno-deprecated-declarations -Wno-changes-meaning)
endif()

### Targets ###
add_subdirectory("src")
