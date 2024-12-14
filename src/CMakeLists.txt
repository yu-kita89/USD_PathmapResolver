### CPP Settings ###
# Preprocessor Defines (Same as #define)
# This is a hacky way of preprocessing the class name to a custom name.
# It is not advised to do this in a production setup! (https://stackoverflow.com/questions/20979584)
# add_compile_definitions(FileResolver=${AR_FILERESOLVER_USD_CXX_CLASS_NAME})
# NOTE: プリプロセッサによる#defineを使用してのクラス名の置き換えは、本番環境では推奨されていませんとの事

# 関数や変数をエクスポートするためにマクロを定義
add_compile_definitions(AR_EXPORTS)

### Targets ###
# ライブラリを作成
add_library(${AR_PATHMAPRESOLVER_TARGET_LIB}
    SHARED
        debugCodes.cpp
        resolver.cpp
        resolverContext.cpp
)

# Libs
# 指定したターゲット（実行可能ファイルやライブラリ）がリンクするライブラリの検索ディレクトリを追加
target_link_directories(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC # pythonのライブラリでも使用するため PUBLIC に変更
        ${AR_PXR_LIB_DIR}
        ${AR_PYTHON_LIB_DIR}
)
# 指定したターゲットがリンクするライブラリを指定
target_link_libraries(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC # pythonのライブラリでも使用するため PUBLIC に変更
        ${AR_PXR_LIB_PREFIX}ar
        ${AR_PXR_LIB_PREFIX}arch
        ${AR_PXR_LIB_PREFIX}tf
        ${AR_BOOST_PYTHON_LIB} # resolverContextが内部的に使用しているので指定
)
# Headers
# 指定したターゲットが依存するインクルードディレクトリを指定
target_include_directories(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC # pythonのライブラリでも使用するため PUBLIC に変更
        ${AR_PXR_INCLUDE_DIR}
        ${AR_PYTHON_INCLUDE_DIR}
        ${AR_BOOST_INCLUDE_DIR} # resolverContextが内部的に使用しているので指定
)
# Props
# ビルドした動的ライブラリに"lib"のプリフィックスがつかないように設定 e.g. libhoge.so -> hoge.so
set_target_properties(${AR_PATHMAPRESOLVER_TARGET_LIB} PROPERTIES PREFIX "")
# 指定したターゲットのコンパイル時に定義するマクロを追加
target_compile_definitions(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PRIVATE
        # USD Plugin Internal Namings
        MFB_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
        MFB_ALT_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
)
# Install
# テンプレートファイルの処理
configure_file(plugInfo.json.in plugInfo.json)
# ビルドしたターゲットやファイルのインストール先を指定
install(FILES pathmapResolver.json DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/packages)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/resources)
install(TARGETS ${AR_PATHMAPRESOLVER_TARGET_LIB} DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib)

## Target library > pathmapResolver Python ##
# Pythonで使用するためのライブラリを作成
add_library(${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    SHARED
        module.cpp
        moduleDeps.cpp
        wrapResolver.cpp
        wrapResolverContext.cpp
)
# 依存関係を追加
add_dependencies(${AR_PATHMAPRESOLVER_TARGET_PYTHON} ${AR_PATHMAPRESOLVER_TARGET_LIB})
# Libs
# 指定したターゲットがリンクするライブラリを追加
target_link_libraries(${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    PRIVATE
        ${AR_PATHMAPRESOLVER_TARGET_LIB}
)
# Props
# Remove default "lib" prefix
# ビルドした動的ライブラリに"lib"のプリフィックスがつかないように設定 e.g. lib_hoge.so -> _hoge.so
set_target_properties(${AR_PATHMAPRESOLVER_TARGET_PYTHON} PROPERTIES PREFIX "")
if (WIN32)
    # 拡張モジュールとして使用できるように拡張子を.pydに変更
    set_target_properties(${AR_PATHMAPRESOLVER_TARGET_PYTHON} PROPERTIES SUFFIX ".pyd")
endif()
# Preprocessor Defines (Same as #define)
# 指定したターゲットのコンパイル時に定義するマクロを追加
target_compile_definitions(${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    PRIVATE
        # USD Plugin Internal Namings
        MFB_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
        MFB_ALT_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
        MFB_PACKAGE_MODULE=${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME}
        AR_PATHMAPRESOLVER_USD_PLUGIN_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
        AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_FULLNAME=${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_FULLNAME}
        AR_BOOST_NAMESPACE=${AR_BOOST_NAMESPACE}
)
# Install
# ビルドしたターゲットやファイルのインストール先を指定
install (
    FILES __init__.py
    DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib/python/${AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME}/${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME}
)
install(
    TARGETS ${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib/python/${AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME}/${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME}
)