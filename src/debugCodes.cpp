#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>

#include "debugCodes.h"

PXR_NAMESPACE_OPEN_SCOPE

// デバッグシンボルにディスクリプションを設定
// この記述をしなかった場合でも、TF_DEBUG自体は使用可能です;
TF_REGISTRY_FUNCTION(TfDebug)
{
    // 第一引数にデバッグコード、第二引数にディスクリプションを記述します;
    TF_DEBUG_ENVIRONMENT_SYMBOL(AR_PATHMAPRESOLVER,
        "Print debug output during path resolution for ArPathmapResolver");
}

PXR_NAMESPACE_CLOSE_SCOPE
