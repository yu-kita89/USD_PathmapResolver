# compile_commands.jsonの書き出し
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Configuration ###
set(AR_PROJECT_NAME UsdPathmapResolver) # projectに使う変数をセット
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist"
    CACHE PATH "Default install dir") # installするディレクトリをセット

### Init ###
# ビルドタイプをReleaseに
set(CMAKE_BUILD_TYPE Release)
# C++17を使用
set(CMAKE_CXX_STANDARD 17)
# C++17がサポートされていない場合にエラーを出す
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# CMakeの最小バージョンの指定
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
# プロジェクトを定義 言語はC++を使用
project(${AR_PROJECT_NAME} LANGUAGES CXX)

# Pathmap Resolver
# ライブラリの名前をセット
set(AR_PATHMAPRESOLVER_TARGET_LIB pathmapResolver)
# プラグインの名前をライブラリ名と同じになるようにセット
set(AR_PATHMAPRESOLVER_USD_PLUGIN_NAME ${AR_PATHMAPRESOLVER_TARGET_LIB})
# リゾルバーの名前をセット
set(AR_PATHMAPRESOLVER_USD_CXX_CLASS_NAME ArPathmapResolver)

# Arch
# 各OSでの設定
# 動的ライブラリの拡張子が異なるのでAR_ARCH_LIB_SUFFIXの変数に拡張子をセットしたり、install先のディレクトリを指定
# windows/linux以外はエラーで終了
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
# 環境変数HFSをAR_HOUDINI_ROOTの変数にセット
# Cmakeを実行する前にHFSを設定している必要があります
set(AR_HOUDINI_ROOT $ENV{HFS} CACHE PATH "Houdini install directory")
# includeディレクトリをAR_HOUDINI_INCLUDE_DIRの変数にセット
set(AR_HOUDINI_INCLUDE_DIR ${AR_HOUDINI_ROOT}/toolkit/include)

# Usd
# USDのライブラリが存在するディレクトリ、またライブラリのプリフィックスを各OSで取得
# e.g. /opt/hfs20.0.653/custom/houdini/dsolib
if (WIN32)
    set(AR_PXR_LIB_DIR ${AR_HOUDINI_ROOT}/custom/houdini/dsolib)
    set(AR_PXR_LIB_PREFIX "libpxr_")
else()
    set(AR_PXR_LIB_DIR ${AR_HOUDINI_ROOT}/dsolib)
    set(AR_PXR_LIB_PREFIX "pxr_")
endif()
# USDのincludeディレクトリをAR_PXR_INCLUDE_DIRの変数にセット
# e.g. /opt/hfs20.0.653/toolkit/include
set(AR_PXR_INCLUDE_DIR ${AR_HOUDINI_INCLUDE_DIR})

# Python
# HoudiniのPythonバージョンをチェックし、利用可能な最新のバージョンをAR_PYTHON_LIBの変数にセット
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
# PythonのlibraryディレクトリをAR_PYTHON_LIB_DIRの変数にセット
# e.g. /opt/hfs20.0.653/python/lib
if (WIN32)
    set(AR_PYTHON_LIB_DIR ${AR_HOUDINI_ROOT}/${AR_PYTHON_LIB_NUMBER}/libs)
else()
    set(AR_PYTHON_LIB_DIR ${AR_HOUDINI_ROOT}/python/lib)
endif()
# PythonのincludeディレクトリをAR_PYTHON_INCLUDE_DIRの変数にセット
# e.g. /opt/hfs20.0.653/toolkit/include/python3.10
set(AR_PYTHON_INCLUDE_DIR ${AR_HOUDINI_INCLUDE_DIR}/${AR_PYTHON_LIB})

### CPP Settings ###
# 共有ライブラリ（動的ライブラリ）としてビルドするように設定
set(BUILD_SHARED_LIBS ON)
# Preprocessor Defines (Same as #define)
# boostの自動リンクを無効にする
add_compile_definitions(HBOOST_ALL_NO_LIB BOOST_ALL_NO_LIB)
# ソースコードに特定の定義を追加
if (WIN32)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
else()
    # Notes:
    # Houdini 20 - Switched to the new C++11 ABI for Linux https://www.sidefx.com/docs/houdini/news/20/platforms.html
    # For Houdini versions that use gcc 9.3, please set this to _GLIBCXX_USE_CXX11_ABI=0
    
    # Houdini 20 からC++11 ABIが使用されているので、
    # 互換性を保つために_GLIBCXX_USE_CXX11_ABIを設定

    # AR_HOUDINI_ROOTを絶対パスに変換してAR_HOUDINI_ROOT_RESOLVEDの変数にセット
    file(REAL_PATH ${AR_HOUDINI_ROOT} AR_HOUDINI_ROOT_RESOLVED)
    # AR_HOUDINI_ROOT_RESOLVEDに"19.5"という文字列が含まれるかを検索、
    # その検索結果をAR_HOUDINI_ROOT_IS_H195の変数にセット、見つからない場合は-1がセットされます
    string(FIND ${AR_HOUDINI_ROOT_RESOLVED} "19.5" AR_HOUDINI_ROOT_IS_H195)
    if (${AR_HOUDINI_ROOT_IS_H195} STREQUAL "-1")
        # 19.5以外の場合は_GLIBCXX_USE_CXX11_ABI=1に設定
        add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)
    else()
        # 19.5の場合は_GLIBCXX_USE_CXX11_ABI=0に設定
        message("Houdini 19.5 detected, switching to _GLIBCXX_USE_CXX11_ABI=0")
        add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()
# Compiler Options
# コンパイラに追加のオプションを設定
if (WIN32)
    add_compile_options(/Zc:inline- /w)
else()
    add_compile_options(-fPIC -Wno-deprecated -Wno-deprecated-declarations -Wno-changes-meaning)
endif()

### Targets ###
# srcディレクトリをプロジェクトに追加
add_subdirectory("src")