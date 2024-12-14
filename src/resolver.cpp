#include <pxr/pxr.h>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/tf/fileUtils.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/pathUtils.h>
#include <pxr/base/tf/staticData.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/base/js/json.h>
#include <pxr/base/js/converter.h>
#include <pxr/base/vt/dictionary.h>

#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/notice.h>

#include "resolver.h"
#include "resolverContext.h"
#include "debugCodes.h"

/*
 * Depending on the asset count and access frequency, it could be better to
 * store the resolver paths in a sorted vector, rather than a map. That's way
 * faster when we are doing significantly more queries inserts.
 */

PXR_NAMESPACE_OPEN_SCOPE

AR_DEFINE_RESOLVER(ArPathmapResolver, ArDefaultResolver)

// Helper Functions
static bool
_IsFileRelative(const std::string& path) {
    return path.find("./") == 0 || path.find("../") == 0;
}

static bool
_IsRelativePath(const std::string& path)
{
    return (!path.empty() && TfIsRelativePath(path));
}

static bool
_IsSearchPath(const std::string& path)
{
    return _IsRelativePath(path) && !_IsFileRelative(path);
}

static std::vector<std::string>
_ParseSearchPaths(const std::string& pathStr)
{
    return TfStringTokenize(pathStr, ARCH_PATH_LIST_SEP);
}

static const VtDictionary
_ParsePathmapDict(std::string pathmapStr)
{
    if (pathmapStr.empty()) {
        return VtDictionary();
    }
    pathmapStr = TfStringReplace(pathmapStr, "\\", "\\\\");
    pathmapStr = TfStringReplace(pathmapStr, "'", "\"");
    JsParseError error;
    JsValue jsdict = JsParseString(pathmapStr, &error);
    if(!jsdict.IsObject()){
        return VtDictionary();
    }
    const VtValue vtdict = 
        JsValueTypeConverter<VtValue, VtDictionary, /*UseInt64*/false>::Convert(jsdict);
    return vtdict.IsHolding<VtDictionary>() ?
        vtdict.UncheckedGet<VtDictionary>() : VtDictionary();
}

static ArResolvedPath
_ResolveAnchored(
    const std::string& anchorPath,
    const std::string& path)
{
    std::string resolvedPath = path;
    if (!anchorPath.empty()) {
        resolvedPath = TfStringCatPaths(anchorPath, path);
    }

    return TfPathExists(resolvedPath) ?
        ArResolvedPath(TfAbsPath(resolvedPath)) : ArResolvedPath();
}

// Static structure for default resolver context initialization
struct _ArPathmapResolverFallbackContext {
    _ArPathmapResolverFallbackContext() {
        std::vector<std::string> searchPath;
        const std::string envPath = TfGetenv("PXR_AR_DEFAULT_SEARCH_PATH");
        if (!envPath.empty()) {
            searchPath = _ParseSearchPaths(envPath);
        }

        VtDictionary pathmapDict;
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[DefaultPathmapEnvironment] Set : \"%s\"\n",TfGetenv("AR_PATHMAP_ENVIRONMENT", "HOUDINI_PATHMAP").c_str());
        const std::string envPathmap = TfGetenv(TfGetenv("AR_PATHMAP_ENVIRONMENT", "HOUDINI_PATHMAP"));
        if (!envPathmap.empty()) {
            pathmapDict = _ParsePathmapDict(envPathmap);
        }
        context = ArPathmapResolverContext(searchPath, pathmapDict);
    }

    ArPathmapResolverContext context;
};

static TfStaticData<_ArPathmapResolverFallbackContext> _DefaultPath;

// ArPathmapResolver
// Public
void
ArPathmapResolver::SetDefaultSearchPath(
    const std::vector<std::string>& searchPath)
{
    VtDictionary pathmapDict = VtDictionary(_DefaultPath->context.GetPathmapDict());

    ArPathmapResolverContext newFallback = ArPathmapResolverContext(searchPath, pathmapDict);
    if (newFallback == _DefaultPath->context) {
        return;
    }

    _DefaultPath->context = std::move(newFallback);

    ArNotice::ResolverChanged([](const ArResolverContext& ctx){
        return ctx.Get<ArPathmapResolverContext>() != nullptr;
    }).Send();
}

void
ArPathmapResolver::SetDefaultPathmapEnvironment(
    const std::string& envName)
{
    const std::vector<std::string> searchPath = _DefaultPath->context.GetSearchPath();

    VtDictionary pathmapDict;
    const std::string envPathmap = TfGetenv(envName);
    if (!envPathmap.empty()) {
        pathmapDict = _ParsePathmapDict(envPathmap);
    }

    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[SetDefaultPathmapEnvironment] Set : \"%s\"\n",envName.c_str());
    ArPathmapResolverContext newFallback = ArPathmapResolverContext(searchPath, pathmapDict);
    if (newFallback == _DefaultPath->context) {
        return;
    }

    _DefaultPath->context = std::move(newFallback);

    ArNotice::ResolverChanged([](const ArResolverContext& ctx){
        return ctx.Get<ArPathmapResolverContext>() != nullptr;
    }).Send();
}

// Protected
ArResolvedPath ArPathmapResolver::_Resolve(
    const std::string& path) const
{
    if (path.empty()) {
        return ArResolvedPath();
    }

    const ArPathmapResolverContext* contexts[2] =
        {_GetCurrentContextPtr(), &_DefaultPath->context};

    std::string newAssetPath = path;
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve] Trying to resolve the path. : \"%s\"\n", path.c_str());

    if (!_IsFileRelative(newAssetPath) && !TfPathExists(newAssetPath)) {
        for (const ArPathmapResolverContext* ctx : contexts) {
            if (ctx) {
                for (const auto& pathmap : ctx->GetPathmapDict()) {
                    std::string k = pathmap.first;
                    std::string v = pathmap.second.Get<std::string>();
                    if (!k.empty() && TfStringStartsWith(newAssetPath,k)) {
                        newAssetPath.replace(0, k.length(), v);
                        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   Mapped path from \"%s\" to \"%s\".\n", k.c_str(), v.c_str());
                    }
                }
            }
        }
    }

    if (_IsRelativePath(newAssetPath)) {
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is relative.\n");
        ArResolvedPath resolvedPath = _ResolveAnchored(ArchGetCwd(), newAssetPath);
        if (resolvedPath) {
            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
            return resolvedPath;
        }

        if (_IsSearchPath(newAssetPath)) {
            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is search path.\n");
            for (const ArPathmapResolverContext* ctx : contexts) {
                if (ctx) {
                    for (const auto& searchPath : ctx->GetSearchPath()) {
                        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   Searching from \"%s\".\n",searchPath.c_str());
                        resolvedPath = _ResolveAnchored(searchPath, newAssetPath);
                        if (resolvedPath) {
                            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
                            return resolvedPath;
                        }
                    }
                }
            }
        }
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path could not be resolved, returning an empty path.\n");
        return ArResolvedPath();
    }

    ArResolvedPath resolvedPath = _ResolveAnchored(std::string(), newAssetPath);
    if (resolvedPath.IsEmpty()){
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path could not be resolved, returning an empty path.\n");
    }
    else{
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
    }
    return resolvedPath;
}

ArResolverContext 
ArPathmapResolver::_CreateDefaultContext() const
{
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateDefaultContext] Creating default context.\n");
    return _defaultContext;
}

ArResolverContext 
ArPathmapResolver::_CreateDefaultContextForAsset(
    const std::string& assetPath) const
{
    if (assetPath.empty()){
        return ArResolverContext(ArPathmapResolverContext());
    }

    std::string assetDir = TfGetPathName(TfAbsPath(assetPath));
    
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateDefaultContextForAsset] Creating default context for asset. : \"%s\"\n",assetPath.c_str());
    return ArResolverContext(ArPathmapResolverContext(
                                 std::vector<std::string>(1, assetDir)));
}

ArResolverContext
ArPathmapResolver::_CreateContextFromString(
    const std::string& contextStr) const
{
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString] Creating context from string. : \"%s\"\n",contextStr.c_str());

    const size_t index = contextStr.find(",");
    if (index == std::string::npos) {
        return ArPathmapResolverContext(_ParseSearchPaths(contextStr));
    }
    else {
        const std::string searchPath = contextStr.substr(0, index);
        const std::string pathmap    = contextStr.substr(index+1, contextStr.size()-1);
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString]   Search path : \"%s\"\n",searchPath.c_str());
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString]   Pathmap     : \"%s\"\n",pathmap.c_str());
        return ArPathmapResolverContext(_ParseSearchPaths(searchPath), _ParsePathmapDict(pathmap));
    }
}

// Private
const ArPathmapResolverContext* 
ArPathmapResolver::_GetCurrentContextPtr() const
{
    return _GetCurrentContextObject<ArPathmapResolverContext>();
}

PXR_NAMESPACE_CLOSE_SCOPE