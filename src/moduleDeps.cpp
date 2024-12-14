#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/scriptModuleLoader.h>
#include <pxr/base/tf/preprocessorUtilsLite.h>
#include <pxr/base/tf/token.h>

PXR_NAMESPACE_OPEN_SCOPE

// Pythonモジュールが依存しているライブラリを適切に読み込めるようにするための処理;
TF_REGISTRY_FUNCTION(TfScriptModuleLoader) {
    // List of direct dependencies for this library.
    // 依存するライブラリを指定;
    const std::vector<TfToken> reqs = {
        TfToken("ar"),
        TfToken("arch"),
        TfToken("tf")
    };
    // ライブラリの登録。引数にそれぞれ、C++のプラグイン名、Pythonのフルインポートパス、依存するライブラリのリストをセット
    // e.g. RegisterLibrary(TfToken("pathmapResolver"),TfToken("usdCustom.PathmapResolver"),reqs);
    TfScriptModuleLoader::GetInstance().
        RegisterLibrary(TfToken(TF_PP_STRINGIZE(AR_PATHMAPRESOLVER_USD_PLUGIN_NAME)), 
                        TfToken(TF_PP_STRINGIZE(AR_PATHMAPRESOLVER_USD_PYTHON_MODULE_FULLNAME)), reqs);
}

PXR_NAMESPACE_CLOSE_SCOPE