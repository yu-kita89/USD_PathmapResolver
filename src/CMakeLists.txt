### CPP Settings ###
# Preprocessor Defines (Same as #define)
# This is a hacky way of preprocessing the class name to a custom name.
# It is not advised to do this in a production setup! (https://stackoverflow.com/questions/20979584)
# add_compile_definitions(FileResolver=${AR_FILERESOLVER_USD_CXX_CLASS_NAME})

add_compile_definitions(AR_EXPORTS)

### Targets ###
add_library(${AR_PATHMAPRESOLVER_TARGET_LIB}
    SHARED
        debugCodes.cpp
        resolver.cpp
        resolverContext.cpp
)

# Libs
target_link_directories(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC
        ${AR_PXR_LIB_DIR}
        ${AR_PYTHON_LIB_DIR}
)
target_link_libraries(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC
        ${AR_PXR_LIB_PREFIX}ar
        ${AR_PXR_LIB_PREFIX}arch
        ${AR_PXR_LIB_PREFIX}tf
        ${AR_PXR_LIB_PREFIX}vt
        ${AR_PXR_LIB_PREFIX}js
        ${AR_BOOST_PYTHON_LIB}
)
# Headers
target_include_directories(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PUBLIC
        ${AR_PXR_INCLUDE_DIR}
        ${AR_PYTHON_INCLUDE_DIR}
        ${AR_BOOST_INCLUDE_DIR}
)
# Props
# Remove default "lib" prefix
set_target_properties(${AR_PATHMAPRESOLVER_TARGET_LIB} PROPERTIES PREFIX "")
# Preprocessor Defines (Same as #define)
target_compile_definitions(${AR_PATHMAPRESOLVER_TARGET_LIB}
    PRIVATE
        # USD Plugin Internal Namings
        MFB_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
        MFB_ALT_PACKAGE_NAME=${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}
)
# Install
configure_file(plugInfo.json.in plugInfo.json)
install(FILES pathmapResolver.json DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/packages)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/resources)
install(TARGETS ${AR_PATHMAPRESOLVER_TARGET_LIB} DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib)

## Target library > pathmapResolver Python ##
add_library(${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    SHARED
        module.cpp
        moduleDeps.cpp
        wrapResolver.cpp
        wrapResolverContext.cpp
)
add_dependencies(${AR_PATHMAPRESOLVER_TARGET_PYTHON} ${AR_PATHMAPRESOLVER_TARGET_LIB})
# Libs
target_link_libraries(${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    PRIVATE
        ${AR_PATHMAPRESOLVER_TARGET_LIB}
)
# Props
# Remove default "lib" prefix
set_target_properties(${AR_PATHMAPRESOLVER_TARGET_PYTHON} PROPERTIES PREFIX "")
if (WIN32)
    set_target_properties(${AR_PATHMAPRESOLVER_TARGET_PYTHON} PROPERTIES SUFFIX ".pyd")
endif()
# Preprocessor Defines (Same as #define)
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
install (
    FILES __init__.py
    DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib/python/${AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME}/${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME}
)
install(
    TARGETS ${AR_PATHMAPRESOLVER_TARGET_PYTHON}
    DESTINATION ${AR_PATHMAPRESOLVER_USD_PLUGIN_NAME}/${AR_ARCH_DIR}/lib/python/${AR_PATHMAPRESOLVER_USD_PYTHON_PACKAGE_NAME}/${AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_NAME}
)