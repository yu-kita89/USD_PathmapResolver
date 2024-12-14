#include <pxr/pxr.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/usd/ar/defaultResolverContext.h>

#include "resolverContext.h"

PXR_NAMESPACE_OPEN_SCOPE

// コンストラクタ;
ArPathmapResolverContext::ArPathmapResolverContext()
    : ArDefaultResolverContext(){}

// コンストラクタ、検索パスを引数で初期化;
ArPathmapResolverContext::ArPathmapResolverContext(
    const std::vector<std::string>& searchPath)
    : ArDefaultResolverContext(searchPath){}

// == オペレータ、検索パスを比較して同じであればTrueを返す;
bool
ArPathmapResolverContext::operator==(const ArPathmapResolverContext& rhs) const
{
    return GetSearchPath() == rhs.GetSearchPath();
}

// 検索パスを文字列として返す関数;
std::string 
ArPathmapResolverContext::GetAsString() const
{
    std::string result = ArDefaultResolverContext::GetAsString();
    return result;
}

// 検索パスからハッシュ値を返す関数;
size_t 
hash_value(const ArPathmapResolverContext& context)
{
    return TfHash()(context.GetSearchPath());
}

PXR_NAMESPACE_CLOSE_SCOPE