#ifndef AR_PATHMAPRESOLVER_DEBUGCODES_H
#define AR_PATHMAPRESOLVER_DEBUGCODES_H

#include <pxr/pxr.h>
#include <pxr/base/tf/debug.h>

PXR_NAMESPACE_OPEN_SCOPE

// デバッグシンボルの定義
// ここで定義することで、TF_DEBUG(AR_PATHMAPRESOLVER)が使用可能になる
// ※有効にする場合は、別途TF_DEBUG等の環境変数で指定する必要があります;
TF_DEBUG_CODES(
    AR_PATHMAPRESOLVER
);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // AR_PATHMAPRESOLVER_DEBUGCODES_H