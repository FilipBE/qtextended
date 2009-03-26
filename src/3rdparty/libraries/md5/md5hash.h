#ifndef MD5HASH_H
#define MD5HASH_H

#include "md5hashglobal.h"

#include <QString>

#ifndef Q_QDOC
namespace MD5
{
#else
class MD5
{
public:
#endif

    QTOPIA_MD5HASH_EXPORT QString hash( const QString & );

};

#endif
