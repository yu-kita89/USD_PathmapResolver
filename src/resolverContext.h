#ifndef AR_PATHMAPRESOLVER_RESOLVER_CONTEXT_H
#define AR_PATHMAPRESOLVER_RESOLVER_CONTEXT_H

#include <pxr/pxr.h>
#include <pxr/base/arch/api.h>
#include <pxr/usd/ar/defaultResolverContext.h>

PXR_NAMESPACE_OPEN_SCOPE

class ArPathmapResolverContext : public ArDefaultResolverContext {
public:
    AR_API
    ArPathmapResolverContext();    

    AR_API 
    ArPathmapResolverContext(const std::vector<std::string>& searchPath);

    AR_API
    bool operator==(const ArPathmapResolverContext& rhs) const;

    AR_API
    std::string GetAsString() const;
};

AR_API
size_t hash_value(const ArPathmapResolverContext& context);

inline std::string 
ArGetDebugString(const ArPathmapResolverContext& context)
{
    return context.GetAsString();
}

// コンテキストを宣言;
AR_DECLARE_RESOLVER_CONTEXT(ArPathmapResolverContext);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // AR_PATHMAPRESOLVER_RESOLVER_CONTEXT_H