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

// Resolverの登録
// 第一引数に自身のリゾルバ、第二引数に継承している基底クラスを指定;
AR_DEFINE_RESOLVER(ArPathmapResolver, ArDefaultResolver)

// Helper Functions
// ファイルパスが相対パスかどうか確認するための関数
// ファイルパスの中に ./ か ../ が含まれている場合にTrueを返します;
static bool
_IsFileRelative(const std::string& path) {
    return path.find("./") == 0 || path.find("../") == 0;
}

// パスが相対パスかどうかを確認するための関数
// _IsFileRelativeと少し異なり、
// WindowsではWindowsAPIのPathIsRelativeW関数の戻り値がTrueかつ、先頭文字が / や \\ ではない場合、
// Unix系では、先頭文字が / ではない場合にTrueを返します;
static bool
_IsRelativePath(const std::string& path)
{
    return (!path.empty() && TfIsRelativePath(path));
}

// アセット解決の際に、コンテキストによって検索すべきパスかどうかを確認する関数
// 先頭文字が / や \\ 以外で、かつ ./ や ../ を含まない場合にTrueを返します;
static bool
_IsSearchPath(const std::string& path)
{
    return _IsRelativePath(path) && !_IsFileRelative(path);
}

// パスをOSの区切り文字で分割し、std::vector<std::string>で返します
// ARCH_PATH_LIST_SEPはWindowsだと ; Unix系だと : になります;
static std::vector<std::string>
_ParseSearchPaths(const std::string& pathStr)
{
    return TfStringTokenize(pathStr, ARCH_PATH_LIST_SEP);
}

// パスマップの文字列を解析して、その結果をVtDictionaryで返します
// "{'hoge':'fuga', 'foo':'var'}" -> VtDictionary({{"hoge",VtValue("fuga")},{"foo",VtValue("var")}});
static const VtDictionary
_ParsePathmapDict(std::string pathmapStr)
{
    // パスが空だった場合、空の辞書を返す;
    if (pathmapStr.empty()) {
        return VtDictionary();
    }
    // JSONでパースする時に文字列のバックスラッシュが正しく機能するように置き換え;
    pathmapStr = TfStringReplace(pathmapStr, "\\", "\\\\");
    // JSONでパースする時に文字列が ' で囲まれているとエラーになるので " に置き換え;
    pathmapStr = TfStringReplace(pathmapStr, "'", "\"");
    // JSONとしてパース;
    JsParseError error;
    JsValue jsdict = JsParseString(pathmapStr, &error);
    // パースに失敗した場合は、空の辞書を返す;
    if(!jsdict.IsObject()){
        return VtDictionary();
    }
    // JSONをVtValueに変換;
    const VtValue vtdict = 
        JsValueTypeConverter<VtValue, VtDictionary, /*UseInt64*/false>::Convert(jsdict);
    // VtValueが辞書型として値を持っていればその値を返す。持っていなければ空の辞書を返す;
    return vtdict.IsHolding<VtDictionary>() ?
        vtdict.UncheckedGet<VtDictionary>() : VtDictionary();
}

// アンカーパス上でのパスが存在するかどうか確認する関数
// 見つかればそのパスを返します;
static ArResolvedPath
_ResolveAnchored(
    const std::string& anchorPath,
    const std::string& path)
{
    std::string resolvedPath = path;
    if (!anchorPath.empty()) {
        // アンカーパスに値が入っている場合、アンカーパスとパスを結合します;
        resolvedPath = TfStringCatPaths(anchorPath, path);
    }

    // パスが存在する場合は絶対パスを返し、存在しない場合は空のパスを返します;
    return TfPathExists(resolvedPath) ?
        ArResolvedPath(TfAbsPath(resolvedPath)) : ArResolvedPath();
}

// Static structure for default resolver context initialization
// 検索パスとパスマップ辞書を含んだ、デフォルトコンテキストの構造体を作成;
struct _ArPathmapResolverFallbackContext {
    _ArPathmapResolverFallbackContext() {
        std::vector<std::string> searchPath;
        // PXR_AR_DEFAULT_SEARCH_PATHの環境変数を取得、設定されていなければ空の値を返す;
        const std::string envPath = TfGetenv("PXR_AR_DEFAULT_SEARCH_PATH");
        if (!envPath.empty()) {
            // PXR_AR_DEFAULT_SEARCH_PATHの値を区切って、searchPathに代入;
            searchPath = _ParseSearchPaths(envPath);
        }

        VtDictionary pathmapDict;
        // AR_PATHMAP_ENVIRONMENTの環境変数を取得、設定されていなければ HOUDINI_PATHMAP の値を取得;
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[DefaultPathmapEnvironment] Set : \"%s\"\n",TfGetenv("AR_PATHMAP_ENVIRONMENT", "HOUDINI_PATHMAP").c_str());
        const std::string envPathmap = TfGetenv(TfGetenv("AR_PATHMAP_ENVIRONMENT", "HOUDINI_PATHMAP"));
        if (!envPathmap.empty()) {
            // 環境変数の値からパスマップ辞書を作成、pathmapDictに代入;
            pathmapDict = _ParsePathmapDict(envPathmap);
        }
        // コンテキストの作成;
        context = ArPathmapResolverContext(searchPath, pathmapDict);
    }

    ArPathmapResolverContext context; // コンテキストのインスタンス変数;
};

// _ArPathmapResolverFallbackContextを静的なオブジェクトとして宣言;
static TfStaticData<_ArPathmapResolverFallbackContext> _DefaultPath;

// ArPathmapResolver
// Public
// デフォルトの検索パスを変更するための関数;
void
ArPathmapResolver::SetDefaultSearchPath(
    const std::vector<std::string>& searchPath)
{
    // 先に元のコンテキストのパスマップ辞書をコピー;
    VtDictionary pathmapDict = VtDictionary(_DefaultPath->context.GetPathmapDict());

    // 検索パスと、パスマップ辞書からコンテキストを作成;
    ArPathmapResolverContext newFallback = ArPathmapResolverContext(searchPath, pathmapDict);
    // 作成したコンテキストが元のコンテキストと同じなら何もせずに終了;
    if (newFallback == _DefaultPath->context) {
        return;
    }

    // 変更があった場合は、新しいコンテキストを元のコンテキストに代入;
    _DefaultPath->context = std::move(newFallback);

    // リゾルバーの変更があった事を通知;
    ArNotice::ResolverChanged([](const ArResolverContext& ctx){
        return ctx.Get<ArPathmapResolverContext>() != nullptr;
    }).Send();
}

// デフォルトのパスマップ辞書を変更するための関数
// 辞書としては渡さず、パスマップの情報が記述された環境変数の名前を指定します;
void
ArPathmapResolver::SetDefaultPathmapEnvironment(
    const std::string& envName)
{
    // 先に元のコンテキストの検索パスをコピー;
    const std::vector<std::string> searchPath = _DefaultPath->context.GetSearchPath();

    // envNameで指定した環境変数を取得、設定されていればパスマップ辞書を作成;
    VtDictionary pathmapDict;
    const std::string envPathmap = TfGetenv(envName);
    if (!envPathmap.empty()) {
        pathmapDict = _ParsePathmapDict(envPathmap);
    }
    

    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[SetDefaultPathmapEnvironment] Set : \"%s\"\n",envName.c_str());
    // 検索パスと、パスマップ辞書からコンテキストを作成;
    ArPathmapResolverContext newFallback = ArPathmapResolverContext(searchPath, pathmapDict);
    // 作成したコンテキストが元のコンテキストと同じなら何もせずに終了;
    if (newFallback == _DefaultPath->context) {
        return;
    }

    // 変更があった場合は、新しいコンテキストを元のコンテキストに代入;
    _DefaultPath->context = std::move(newFallback);

    // リゾルバーの変更があった事を通知;
    ArNotice::ResolverChanged([](const ArResolverContext& ctx){
        return ctx.Get<ArPathmapResolverContext>() != nullptr;
    }).Send();
}

// Protected
// パスを解決するための関数;
ArResolvedPath ArPathmapResolver::_Resolve(
    const std::string& path) const
{
    // パスが空だった場合、空のパスを返す;
    if (path.empty()) {
        return ArResolvedPath();
    }

    // パスマップでもコンテキストを使用するので、先にコンテキストを取得しておきます;
    const ArPathmapResolverContext* contexts[2] =
        {_GetCurrentContextPtr(), &_DefaultPath->context};

    // パスを変更するので新しい変数にコピー;
    std::string newAssetPath = path;
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve] Trying to resolve the path. : \"%s\"\n", path.c_str());

    // パスマップの処理
    // パスが ./ や　../ を含まず存在しない場合;
    if (!_IsFileRelative(newAssetPath) && !TfPathExists(newAssetPath)) {
        // 現在バインドされているコンテキストと、デフォルトのコンテキストを順番に処理;
        for (const ArPathmapResolverContext* ctx : contexts) {
            if (ctx) {
                // パスマップ辞書を順番に処理;
                for (const auto& pathmap : ctx->GetPathmapDict()) {
                    // キーとバリューを取得;
                    // バリューはVtValue型なのでstd::stringに変換して取得;
                    std::string k = pathmap.first;
                    std::string v = pathmap.second.Get<std::string>();
                    // キーとパスの先頭文字が同じ場合;
                    if (!k.empty() && TfStringStartsWith(newAssetPath,k)) {
                        //　パスの先頭文字をバリューで置き換える;
                        newAssetPath.replace(0, k.length(), v);
                        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   Mapped path from \"%s\" to \"%s\".\n", k.c_str(), v.c_str());
                    }
                }
            }
        }
    }
    // 以降はArDefaultResolverと同じ処理;

    // 相対パスかどうか確認;
    if (_IsRelativePath(newAssetPath)) {
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is relative.\n");
        // パスが現在のディレクトリをアンカーとしたパスか確認し、存在すればパスを返す;
        ArResolvedPath resolvedPath = _ResolveAnchored(ArchGetCwd(), newAssetPath);
        if (resolvedPath) {
            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
            return resolvedPath;
        }
        // 検索すべきパスか確認;
        if (_IsSearchPath(newAssetPath)) {
            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is search path.\n");
            // 現在バインドされているコンテキストと、デフォルトのコンテキストを順番に処理;
            for (const ArPathmapResolverContext* ctx : contexts) {
                if (ctx) {
                    // 検索パスを順番に処理;
                    for (const auto& searchPath : ctx->GetSearchPath()) {
                        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   Searching from \"%s\".\n",searchPath.c_str());
                        // パスが検索パスをアンカーとしたパスか確認し、存在すればパスを返す;
                        resolvedPath = _ResolveAnchored(searchPath, newAssetPath);
                        if (resolvedPath) {
                            TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
                            return resolvedPath;
                        }
                    }
                }
            }
        }
        // 見つからなければ空のパスを返す;
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path could not be resolved, returning an empty path.\n");
        return ArResolvedPath();
    }

    // パスが絶対パスかどうか確認し、見つかればパスを返す
    // 見つからない場合は空のパスを返す;
    ArResolvedPath resolvedPath = _ResolveAnchored(std::string(), newAssetPath);
    if (resolvedPath.IsEmpty()){
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path could not be resolved, returning an empty path.\n");
    }
    else{
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[Resolve]   The path is resolved : \"%s\".\n",resolvedPath.GetPathString().c_str());
    }
    return resolvedPath;
}

// コンテキストが明示的に指定されていない場合にデフォルトのコンテキストを返す関数;
ArResolverContext 
ArPathmapResolver::_CreateDefaultContext() const
{
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateDefaultContext] Creating default context.\n");
    return _defaultContext;
}

// アセットパスを含むディレクトリを追加したコンテキストを返す関数;
ArResolverContext 
ArPathmapResolver::_CreateDefaultContextForAsset(
    const std::string& assetPath) const
{
    // パスが空だった場合、空のパスを返す;
    if (assetPath.empty()){
        return ArResolverContext(ArPathmapResolverContext());
    }

    // アセットパスからディレクトリを取得、相対パスの場合は現在の作業ディレクトリを基準にして絶対パスを返す;
    std::string assetDir = TfGetPathName(TfAbsPath(assetPath));
    
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateDefaultContextForAsset] Creating default context for asset. : \"%s\"\n",assetPath.c_str());
    return ArResolverContext(ArPathmapResolverContext(
                                 std::vector<std::string>(1, assetDir)));
}

// 与えられた文字列からコンテキストを作成する関数;
ArResolverContext
ArPathmapResolver::_CreateContextFromString(
    const std::string& contextStr) const
{
    TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString] Creating context from string. : \"%s\"\n",contextStr.c_str());

    // 文字列の中に , が含まれている場合はそれ以降をパスマップとして処理;
    const size_t index = contextStr.find(",");
    if (index == std::string::npos) {
        return ArPathmapResolverContext(_ParseSearchPaths(contextStr));
    }
    else {
        // 文字列を検索パスと、パスマップ用の文字列に分ける;
        const std::string searchPath = contextStr.substr(0, index);
        const std::string pathmap    = contextStr.substr(index+1, contextStr.size()-1);
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString]   Search path : \"%s\"\n",searchPath.c_str());
        TF_DEBUG(AR_PATHMAPRESOLVER).Msg("[CreateContextFromString]   Pathmap     : \"%s\"\n",pathmap.c_str());
        // それぞれを解析して、コンテキストを作成;
        return ArPathmapResolverContext(_ParseSearchPaths(searchPath), _ParsePathmapDict(pathmap));
    }
}

// Private
// 現在のコンテキストのポインタを返す関数;
const ArPathmapResolverContext* 
ArPathmapResolver::_GetCurrentContextPtr() const
{
    return _GetCurrentContextObject<ArPathmapResolverContext>();
}

PXR_NAMESPACE_CLOSE_SCOPE