
#include <pxr/pxr.h>
#include <pxr/base/tf/pyUtils.h>

#include "resolver.h"

#define BOOST_INCLUDE(path) <AR_BOOST_INCLUDE_PREFIX/path>
#include BOOST_INCLUDE(python/class.hpp)

PXR_NAMESPACE_USING_DIRECTIVE

using namespace AR_BOOST_NAMESPACE::python;

void
wrapResolver()
{
    using This = ArPathmapResolver;

    class_<This, bases<ArDefaultResolver>, AR_BOOST_NONCOPYABLE>
        ("Resolver", no_init)

        .def("SetDefaultPathmapEnvironment", &This::SetDefaultPathmapEnvironment,
             args("envName"))
        .staticmethod("SetDefaultPathmapEnvironment")
    ;
}
