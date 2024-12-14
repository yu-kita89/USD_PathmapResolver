
#include <pxr/pxr.h>
#include <pxr/base/tf/pyUtils.h>

#include "resolver.h"

#define BOOST_INCLUDE(path) <AR_BOOST_NAMESPACE/path>
#include BOOST_INCLUDE(python/class.hpp)

using namespace AR_BOOST_NAMESPACE::python;

PXR_NAMESPACE_USING_DIRECTIVE

void
wrapResolver()
{
    using This = ArPathmapResolver;

    class_<This, bases<ArDefaultResolver>, AR_BOOST_NAMESPACE::noncopyable>
        ("Resolver", no_init)

        .def("SetDefaultPathmapEnvironment", &This::SetDefaultPathmapEnvironment,
             args("envName"))
        .staticmethod("SetDefaultPathmapEnvironment")
    ;
}