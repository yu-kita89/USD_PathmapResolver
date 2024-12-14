
#include <pxr/pxr.h>
#include <pxr/base/tf/pyUtils.h>

#include "resolver.h"

// boost/hboost とでインクルードのパスが変わるので、対応できるように
// マクロを利用してインクルードしています;
#define BOOST_INCLUDE(path) <AR_BOOST_NAMESPACE/path>
#include BOOST_INCLUDE(python/class.hpp)

// boost::pythonのネームスペースを省略できるように;
using namespace AR_BOOST_NAMESPACE::python;

PXR_NAMESPACE_USING_DIRECTIVE

// module.cppで宣言したラップ関数の実装;
void
wrapResolver()
{
    // クラスのエイリアスとしてThisを使用;
    using This = ArPathmapResolver;

    // class_<C++クラス、C++基底クラス、コピー不可>
    // ("Resolver", no_init)でPythonのクラス名を設定、no_initでコンストラクターを使わないように;
    class_<This, bases<ArDefaultResolver>, AR_BOOST_NAMESPACE::noncopyable>
        ("Resolver", no_init)
        // .defでクラス内の関数を設定;
        // SetDefaultPathmapEnvironmentの関数を設定、envNameという引数を持つように;
        .def("SetDefaultPathmapEnvironment", &This::SetDefaultPathmapEnvironment,
             args("envName"))
        // インスタンス化しなくても使える関数として静的メソッド（Pythonでのクラスメソッド）を設定;
        .staticmethod("SetDefaultPathmapEnvironment")
    ;
}