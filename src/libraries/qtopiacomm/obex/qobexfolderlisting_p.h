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

#ifndef QOBEXFOLDERLISTING_P_H
#define QOBEXFOLDERLISTING_P_H

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

#include <QXmlDefaultHandler>
#include <QObject>

#include "qobexfolderlistingentryinfo.h"

class QObexFolderListingHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT

public:
    QObexFolderListingHandler();

    bool startElement(const QString &namespaceURI,
                      const QString &localName,
                      const QString &qName,
                      const QXmlAttributes &attributes);

    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);

    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    bool warning(const QXmlParseException &exception);
    bool error(const QXmlParseException &exception);

signals:
    void info(const QObexFolderListingEntryInfo &info);

private:
    QObexFolderListingEntryInfo m_info;
    bool m_valid_elem;
};

#endif
