#include <pxr/pxr.h>
#include <pxr/usd/ar/pyResolverContext.h>
#include <pxr/base/tf/pyUtils.h>

#include "resolverContext.h"

// boost/hboost とでインクルードのパスが変わるので、対応できるように
// マクロを利用してインクルードしています;
#define BOOST_INCLUDE(path) <AR_BOOST_NAMESPACE/path>
#include BOOST_INCLUDE(python/class.hpp)
#include BOOST_INCLUDE(python/operators.hpp)
#include BOOST_INCLUDE(python/return_value_policy.hpp)
#include BOOST_INCLUDE(python/copy_const_reference.hpp)

// boost::pythonのネームスペースを省略できるように;
using namespace AR_BOOST_NAMESPACE::python;

PXR_NAMESPACE_USING_DIRECTIVE

// repr();
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

// hash();
static size_t
_Hash(const ArPathmapResolverContext& ctx)
{
    return hash_value(ctx);
}

// module.cppで宣言したラップ関数の実装;
void
wrapResolverContext()
{
    // クラスのエイリアスとしてThisを使用;
    using This = ArPathmapResolverContext;

    // class_<C++クラス、C++基底クラス、コピー不可>
    // ("ResolverContext", no_init)でPythonのクラス名を設定、no_initで一旦コンストラクターを使わないように;
    class_<This, bases<ArDefaultResolverContext>>("ResolverContext", no_init)
        // .defでクラス内の関数を設定;
        // 改めてコンストラクターを設定;
        .def(init<>())
        .def(init<const std::vector<std::string>&>(args("searchPaths")))
        .def(init<const std::vector<std::string>&, const VtDictionary&>(args("searchPaths","pathmapDict")))
        // C++のGetPathmapDictの戻り値が const の参照なので戻り値のポリシーに copy_const_reference を設定しています
        .def("GetPathmapDict", &This::GetPathmapDict, return_value_policy<copy_const_reference>())
        // オペレータ;
        .def(self == self)
        .def(self != self)
        // 組み込み関数;
        .def("__str__",  &This::GetAsString)
        .def("__repr__", &_Repr)
        .def("__hash__", &_Hash)
    ;
    // コンテキストの登録;
    ArWrapResolverContextForPython<This>();
}
