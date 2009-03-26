#include "md5hash.h"

#include <QCryptographicHash>
#include <QString>

#include <stdlib.h>

QString MD5::hash( const QString &str )
{
    char *inbuf;
    int inbufSize = str.length() * sizeof(QChar);
    inbuf = (char *)::malloc( inbufSize );
    Q_CHECK_PTR( inbuf );
    ::memcpy( inbuf, str.unicode(), inbufSize );

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(inbuf, inbufSize);

    QByteArray digest = hash.result();
    return QString::fromLatin1(digest.toHex());
}
