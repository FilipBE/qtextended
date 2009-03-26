/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/
#ifndef QMIMETYPEDATA_P_H
#define QMIMETYPEDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QContent>

class QMimeTypeDataPrivate;

class QMimeTypeData
{
public:
    QMimeTypeData();
    QMimeTypeData( const QString &id );
    QMimeTypeData( const QMimeTypeData &other );
    ~QMimeTypeData();

    QMimeTypeData &operator =( const QMimeTypeData &other );

    bool operator ==( const QMimeTypeData &other );

    QString id() const;

    QContentList applications() const;

    QContent defaultApplication() const;

    QIcon icon( const QContent &application ) const;
    QIcon validDrmIcon( const QContent &application ) const;
    QIcon invalidDrmIcon( const QContent &application ) const;
    QDrmRights::Permission permission( const QContent &application ) const;

    void addApplication( const QContent &application, const QString &iconFile, QDrmRights::Permission permission );
    void removeApplication( const QContent &application );
    void setDefaultApplication( const QContent &application );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer< QMimeTypeDataPrivate > d;
};

Q_DECLARE_USER_METATYPE(QMimeTypeData);

#endif
