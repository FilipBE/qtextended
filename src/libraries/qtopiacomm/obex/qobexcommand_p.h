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

#ifndef QOBEXCOMMAND_P_H
#define QOBEXCOMMAND_P_H

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

#include <qobexnamespace.h>
#include <qobexheader.h>

#include <QString>
#include <qatomic.h>

class QIODevice;
class QByteArray;

class QObexCommand
{
    public:
        QObexCommand(QObex::Request req, const QObexHeader &header,
                     const QByteArray &data);
        QObexCommand(QObex::Request req,
                     const QObexHeader &header = QObexHeader(),
                     QIODevice *device = 0);
        ~QObexCommand();

        QObex::Request m_req;
        QObexHeader m_header;
        QObex::SetPathFlags m_setPathFlags;
        int m_id;

    // Concept stolen from QFTP
        union {
            QIODevice *device;
            QByteArray *data;
        } m_data;
        bool m_isba;

#if QT_VERSION < 0x040400
        static QBasicAtomic idCounter;
#else
        static QAtomicInt idCounter;
#endif
        static int nextId();

    private:
        Q_DISABLE_COPY(QObexCommand)
};

#endif
