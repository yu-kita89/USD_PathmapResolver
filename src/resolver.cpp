#include <pxr/pxr.h>
#include <pxr/usd/ar/defineResolver.h>

#include "resolver.h"
#include "debugCodes.h"

/*
 * Depending on the asset count and access frequency, it could be better to
 * store the resolver paths in a sorted vector, rather than a map. That's way
 * faster when we are doing significantly more queries inserts.
 */

PXR_NAMESPACE_OPEN_SCOPE

// Resolverの登録
// 第一引数に自身のリゾルバ、第二引数に継承している基底クラスを指定;
AR_DEFINE_RESOLVER(ArPathmapResolver, ArDefaultResolver)

// Protected
ArResolvedPath ArPathmapResolver::_Resolve(
    const std::string& assetPath) const
{
    // デバッグメッセージを表示
    // TF_DEBUGの環境変数に "AR_PATHMAPRESOLVER" を設定する事でコンソールに表示されます;
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve] \"%s\"\n",assetPath.c_str());
    // ArDefaultResolverの_Resolveをそのまま使用;
    return ArDefaultResolver::_Resolve(assetPath);
}

PXR_NAMESPACE_CLOSE_SCOPE