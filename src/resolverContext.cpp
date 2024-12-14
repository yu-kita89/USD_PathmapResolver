#include <pxr/pxr.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/usd/ar/defaultResolverContext.h>

#include "resolverContext.h"
#include "debugCodes.h"

PXR_NAMESPACE_OPEN_SCOPE

ArPathmapResolverContext::ArPathmapResolverContext()
    : ArDefaultResolverContext(),
      _pathmapDict(VtDictionary()) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

ArPathmapResolverContext::ArPathmapResolverContext(
    const std::vector<std::string>& searchPath)
    : ArDefaultResolverContext(searchPath),
      _pathmapDict(VtDictionary()) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

ArPathmapResolverContext::ArPathmapResolverContext(
    const std::vector<std::string>& searchPath,
    const VtDictionary& pathmapDict) 
    : ArDefaultResolverContext(searchPath),
      _pathmapDict(pathmapDict) { TF_DEBUG(AR_PATHMAPRESOLVER_CONTEXT).Msg("%s\n",GetAsString().c_str()); }

bool 
ArPathmapResolverContext::operator==(const ArPathmapResolverContext& rhs) const
{
    return GetSearchPath() == rhs.GetSearchPath() && _pathmapDict == rhs._pathmapDict;
}

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

size_t 
hash_value(const ArPathmapResolverContext& context)
{
    return TfHash::Combine(TfHash()(context.GetSearchPath()), TfHash()(context.GetPathmapDict()));
}

PXR_NAMESPACE_CLOSE_SCOPE