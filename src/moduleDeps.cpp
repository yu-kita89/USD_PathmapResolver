#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/scriptModuleLoader.h>
#include <pxr/base/tf/preprocessorUtilsLite.h>
#include <pxr/base/tf/token.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfScriptModuleLoader) {
    // List of direct dependencies for this library.
    const std::vector<TfToken> reqs = {
        TfToken("ar"),
        TfToken("arch"),
        TfToken("tf"),
        TfToken("vt"),
        TfToken("js")
    };
    
    TfScriptModuleLoader::GetInstance().
        RegisterLibrary(TfToken(TF_PP_STRINGIZE(AR_PATHMAPRESOLVER_USD_PLUGIN_NAME)), 
                        TfToken(TF_PP_STRINGIZE(AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_FULLNAME)), reqs);
}

PXR_NAMESPACE_CLOSE_SCOPE