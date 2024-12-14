#include <pxr/pxr.h>
#include <pxr/base/tf/pyModule.h>

PXR_NAMESPACE_USING_DIRECTIVE

// TF_WRAPで定義した関数をまとめて処理;
TF_WRAP_MODULE
{
    // Pythonで使用できるようにラップするための関数を宣言、呼び出し
    // wrapResolver()、wrapResolverContext()は別途実装する必要があります;
    TF_WRAP(Resolver);
    TF_WRAP(ResolverContext);
}