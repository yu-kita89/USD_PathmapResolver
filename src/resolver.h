#ifndef AR_PATHMAPRESOLVER_RESOLVER_H
#define AR_PATHMAPRESOLVER_RESOLVER_H

#include <pxr/pxr.h>
#include <pxr/base/arch/api.h>
#include <pxr/usd/ar/defaultResolver.h>

#include "resolverContext.h"

PXR_NAMESPACE_OPEN_SCOPE

class ArPathmapResolver : public ArDefaultResolver {
public:
    AR_API
    ArPathmapResolver() = default;

    AR_API
    ~ArPathmapResolver() override = default;

    AR_API
    static void SetDefaultSearchPath(
        const std::vector<std::string>& searchPath);

    AR_API
    static void SetDefaultPathmapEnvironment(
        const std::string& envName);

protected:
    AR_API
    ArResolvedPath _Resolve(
        const std::string& assetPath) const override;

    AR_API
    ArResolverContext _CreateDefaultContext() const override;

    AR_API
    ArResolverContext _CreateDefaultContextForAsset(
        const std::string& assetPath) const override; 

    AR_API
    ArResolverContext _CreateContextFromString(
        const std::string& contextStr) const override;

private:
    const ArPathmapResolverContext* _GetCurrentContextPtr() const;

    ArPathmapResolverContext _defaultContext;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // AR_PATHMAPRESOLVER_RESOLVER_H