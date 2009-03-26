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

#ifndef QANNOTATOR_P_H
#define QANNOTATOR_P_H

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

#include <QByteArray>

#include <quniqueid.h>

class QAnnotator {
public:
    QAnnotator();
    ~QAnnotator();

    QUniqueId add(const QByteArray &, const QString &mimetype = QString());

    bool set(const QUniqueId &, const QByteArray &, const QString &mimetype = QString());
    void remove(const QUniqueId &);

    bool contains(const QUniqueId &) const;

    QString mimetype(const QUniqueId &) const;
    QByteArray blob(const QUniqueId &) const;

    /* add IO device functions later.
       Should be able to open an iodevice for append/create
       should be able to open an iodevice for read
   */
private:
    QUniqueIdGenerator mIdGen;
};

#endif
