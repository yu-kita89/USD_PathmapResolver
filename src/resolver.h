#ifndef AR_PATHMAPRESOLVER_RESOLVER_H
#define AR_PATHMAPRESOLVER_RESOLVER_H

#include <pxr/pxr.h>
#include <pxr/base/arch/api.h>
#include <pxr/usd/ar/defaultResolver.h>

PXR_NAMESPACE_OPEN_SCOPE

class ArPathmapResolver : public ArDefaultResolver {
public:
    AR_API
    ArPathmapResolver() = default;

    AR_API
    ~ArPathmapResolver() override = default;

protected:
    AR_API
    ArResolvedPath _Resolve(
        const std::string& assetPath) const override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // AR_PATHMAPRESOLVER_RESOLVER_H