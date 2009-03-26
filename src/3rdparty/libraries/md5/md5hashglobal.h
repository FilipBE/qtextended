#ifndef QTOPIA_MD5HASH_GLOBAL_H
#define QTOPIA_MD5HASH_GLOBAL_H

#include <qglobal.h>

// The _EXPORT macros...

#if defined(QT_VISIBILITY_AVAILABLE)
#   define QTOPIA_IM_VISIBILITY __attribute__((visibility("default")))
#else
#   define QTOPIA_IM_VISIBILITY
#endif

#ifndef QTOPIA_MD5HASH_EXPORT
#   define QTOPIA_MD5HASH_EXPORT QTOPIA_IM_VISIBILITY
#endif

#endif //QTOPIA_MD5HASH_GLOBAL_H
