#include <pxr/pxr.h>
#include <pxr/usd/ar/pyResolverContext.h>
#include <pxr/base/tf/pyUtils.h>

#include "resolverContext.h"

#define BOOST_INCLUDE(path) <AR_BOOST_NAMESPACE/path>
#include BOOST_INCLUDE(python/class.hpp)
#include BOOST_INCLUDE(python/operators.hpp)
#include BOOST_INCLUDE(python/return_value_policy.hpp)
#include BOOST_INCLUDE(python/copy_const_reference.hpp)

using namespace AR_BOOST_NAMESPACE::python;

PXR_NAMESPACE_USING_DIRECTIVE

static std::string
_Repr(const ArPathmapResolverContext& ctx)
{
    std::string repr = TF_PY_REPR_PREFIX;
    repr += "Context(";
    if (!ctx.GetSearchPath().empty()) {
        repr += TfPyRepr(ctx.GetSearchPath());
    }
    repr += ")";
    return repr;
}

static size_t
_Hash(const ArPathmapResolverContext& ctx)
{
    return hash_value(ctx);
}

void
wrapResolverContext()
{
    using This = ArPathmapResolverContext;

    class_<This, bases<ArDefaultResolverContext>>("ResolverContext", no_init)
        .def(init<>())
        .def(init<const std::vector<std::string>&>(args("searchPaths")))
        .def(init<const std::vector<std::string>&, const VtDictionary&>(args("searchPaths","pathmapDict")))
        .def("GetPathmapDict", &This::GetPathmapDict, return_value_policy<copy_const_reference>())

        .def(self == self)
        .def(self != self)

        .def("__str__",  &This::GetAsString)
        .def("__repr__", &_Repr)
        .def("__hash__", &_Hash)
    ;

    ArWrapResolverContextForPython<This>();
}