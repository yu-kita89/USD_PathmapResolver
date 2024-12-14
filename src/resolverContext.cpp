#include <pxr/pxr.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/usd/ar/defaultResolverContext.h>

#include "resolverContext.h"
#include "debugCodes.h"

PXR_NAMESPACE_OPEN_SCOPE

// 引数なしのコンストラクタ、_pathmapDictは空の辞書で初期化されます;
ArPathmapResolverContext::ArPathmapResolverContext()
    : ArDefaultResolverContext(),
      _pathmapDict(VtDictionary()) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

// 検索パスの引数を受け取るコンストラクタ、_pathmapDictは空の辞書で初期化されます;
ArPathmapResolverContext::ArPathmapResolverContext(
    const std::vector<std::string>& searchPath)
    : ArDefaultResolverContext(searchPath),
      _pathmapDict(VtDictionary()) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

// 検索パス、パスマップ辞書の引数を受け取るコンストラクタ;
ArPathmapResolverContext::ArPathmapResolverContext(
    const std::vector<std::string>& searchPath,
    const VtDictionary& pathmapDict) 
    : ArDefaultResolverContext(searchPath),
      _pathmapDict(pathmapDict) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

// == 演算子をオーバーロードして、検索パスとパスマップ辞書が等しいかどうかを比較;
bool 
ArPathmapResolverContext::operator==(const ArPathmapResolverContext& rhs) const
{
    return GetSearchPath() == rhs.GetSearchPath() && _pathmapDict == rhs._pathmapDict;
}

// 検索パスとパスマップ辞書を文字列として返す関数;
std::string 
ArPathmapResolverContext::GetAsString() const
{
    std::string result = ArDefaultResolverContext::GetAsString();
    result += "\nPathmap dict: ";
    if (_pathmapDict.empty()) {
        result += "[ ]";
    }
    else {
        result += "[\n    ";
        std::vector<std::string> formattedEntries;
        for (const auto& pair : _pathmapDict) {
            formattedEntries.push_back(pair.first + " : " + pair.second.Get<std::string>());
        }
        result += TfStringJoin(formattedEntries, "\n    ");
        result += "\n]";
    }
    return result;
}

// 検索パスとパスマップ辞書を組み合わせたハッシュ値を返す関数;
size_t 
hash_value(const ArPathmapResolverContext& context)
{
    return TfHash::Combine(TfHash()(context.GetSearchPath()), TfHash()(context.GetPathmapDict()));
}

PXR_NAMESPACE_CLOSE_SCOPE