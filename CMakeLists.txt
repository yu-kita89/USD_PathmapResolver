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
if(WIN32)
    set(AR_HYTHON_EXECUTABLE "${AR_HOUDINI_ROOT}/bin/hython.exe")
else()
    set(AR_HYTHON_EXECUTABLE "${AR_HOUDINI_ROOT}/bin/hython")
endif()

if(NOT EXISTS "${AR_HYTHON_EXECUTABLE}")
    message(FATAL_ERROR "Houdini Python executable not found: ${AR_HYTHON_EXECUTABLE}")
endif()

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
execute_process(
    COMMAND "${AR_HYTHON_EXECUTABLE}" -V
    RESULT_VARIABLE AR_PYTHON_DETECT_RESULT
    OUTPUT_VARIABLE AR_HYTHON_RAW_OUTPUT
    ERROR_VARIABLE  AR_HYTHON_RAW_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT AR_PYTHON_DETECT_RESULT EQUAL 0 OR NOT AR_HYTHON_RAW_OUTPUT MATCHES "Python ([0-9]+\\.[0-9]+)")
    message(FATAL_ERROR "Failed to detect Houdini Python version")
endif()
set(AR_PYTHON_VERSION "${CMAKE_MATCH_1}")

# Example:
#   AR_PYTHON_VERSION         = 3.11
#   AR_PYTHON_VERSION_COMPACT = 311
#   AR_PYTHON_LIB             = python3.11
#   AR_PYTHON_LIB_NUMBER      = python311
string(
    REPLACE "." ""
    AR_PYTHON_VERSION_COMPACT
    "${AR_PYTHON_VERSION}"
)
set(AR_PYTHON_LIB         "python${AR_PYTHON_VERSION}")
set(AR_PYTHON_LIB_NUMBER  "python${AR_PYTHON_VERSION_COMPACT}")
set(AR_PYTHON_INCLUDE_DIR "${AR_HOUDINI_INCLUDE_DIR}/${AR_PYTHON_LIB}")

if(WIN32)
    set(AR_PYTHON_LIB_DIR "${AR_HOUDINI_ROOT}/${AR_PYTHON_LIB_NUMBER}/libs")
    find_library(
        AR_PYTHON_LIBRARY
        NAMES "${AR_PYTHON_LIB_NUMBER}"
        PATHS "${AR_PYTHON_LIB_DIR}"
        NO_DEFAULT_PATH
        REQUIRED
    )
else()
    set(AR_PYTHON_LIB_DIR "${AR_HOUDINI_ROOT}/python/lib")
endif()

# Boost
# Detect whether this OpenUSD uses its internal Boost.Python.
if(EXISTS "${AR_PXR_INCLUDE_DIR}/pxr/external/boost")
    set(AR_USE_INTERNAL_BOOST_PYTHON ON)
    set(AR_BOOST_NAMESPACE      pxr_boost)
    set(AR_BOOST_INCLUDE_PREFIX pxr/external/boost)
    # noncopyable moved under pxr_boost::python
    set(AR_BOOST_NONCOPYABLE "${AR_BOOST_NAMESPACE}::python::noncopyable")

    find_library(
        AR_PXR_PYTHON_LIBRARY
        NAMES "${AR_PXR_LIB_PREFIX}python"
        PATHS "${AR_PXR_LIB_DIR}"
        NO_DEFAULT_PATH
        REQUIRED
    )
    find_library(
        AR_PXR_BOOST_LIBRARY
        NAMES "${AR_PXR_LIB_PREFIX}boost"
        PATHS "${AR_PXR_LIB_DIR}"
        NO_DEFAULT_PATH
        REQUIRED
    )
    set(
        AR_USD_PYTHON_LINK_LIBS
        "${AR_PXR_PYTHON_LIBRARY}"
        "${AR_PXR_BOOST_LIBRARY}"
    )
else()
    set(AR_USE_INTERNAL_BOOST_PYTHON OFF)
    set(AR_BOOST_NAMESPACE      hboost)
    set(AR_BOOST_INCLUDE_PREFIX hboost)
    # Old Boost expects hboost::noncopyable.
    set(AR_BOOST_NONCOPYABLE "${AR_BOOST_NAMESPACE}::noncopyable")

    if(WIN32)
        set(AR_BOOST_PYTHON_LIB_NAME "${AR_BOOST_NAMESPACE}_${AR_PYTHON_LIB_NUMBER}-mt-x64")
    else()
        set(AR_BOOST_PYTHON_LIB_NAME "${AR_BOOST_NAMESPACE}_${AR_PYTHON_LIB_NUMBER}")
    endif()

    find_library(
        AR_BOOST_PYTHON_LIBRARY
        NAMES "${AR_BOOST_PYTHON_LIB_NAME}"
        PATHS "${AR_PXR_LIB_DIR}"
              "${AR_HOUDINI_ROOT}/dsolib"
        NO_DEFAULT_PATH
        REQUIRED
    )
    set(
        AR_USD_PYTHON_LINK_LIBS
        "${AR_BOOST_PYTHON_LIBRARY}"
    )
endif()

if(WIN32)
    list(
        APPEND
        AR_USD_PYTHON_LINK_LIBS
        "${AR_PYTHON_LIBRARY}"
    )
endif()

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
add_library(
    ar_houdini_usd_python
    INTERFACE
)
target_include_directories(
    ar_houdini_usd_python
    INTERFACE
        "${AR_PXR_INCLUDE_DIR}"
        "${AR_PYTHON_INCLUDE_DIR}"
)
target_link_libraries(
    ar_houdini_usd_python
    INTERFACE ${AR_USD_PYTHON_LINK_LIBS}
)
target_compile_definitions(
    ar_houdini_usd_python
    INTERFACE
        AR_BOOST_NAMESPACE=${AR_BOOST_NAMESPACE}
        AR_BOOST_INCLUDE_PREFIX=${AR_BOOST_INCLUDE_PREFIX}
        AR_BOOST_NONCOPYABLE=${AR_BOOST_NONCOPYABLE}
)
add_subdirectory("src")
