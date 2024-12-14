#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>

#include "debugCodes.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfDebug)
{
    TF_DEBUG_ENVIRONMENT_SYMBOL(AR_PATHMAPRESOLVER,
        "Print debug output during path resolution for ArPathmapResolver");
    TF_DEBUG_ENVIRONMENT_SYMBOL(AR_PATHMAPRESOLVER_CONTEXT,
        "Print debug output during path resolution for ArPathmapResolverContext");
}

PXR_NAMESPACE_CLOSE_SCOPE
