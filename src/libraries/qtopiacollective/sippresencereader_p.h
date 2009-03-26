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

#ifndef SIPPRESENCEREADER_P_H
#define SIPPRESENCEREADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaglobal.h>
#include <QXmlStreamReader>
#include <QCollectivePresenceInfo>
#include <qcollectivenamespace.h>
#include <QMap>
class QString;

class QTOPIACOLLECTIVE_EXPORT SipPresenceReader : public QXmlStreamReader
{
public:
    SipPresenceReader(const QString &contentType, const QByteArray &data);
    ~SipPresenceReader();

    const QCollectivePresenceInfo &info() const;
    static const QMap<QString, QCollectivePresenceInfo::PresenceType> &statusTypes();

private:

    struct SipPresenceData {
        QString id;
        QString uri;
        double priority;
        QString status;
        QMap<QString, QString> langNotes;
        QMap<QString, QString> customProperties;
    };

    void readPresence();
    // Pidf elements
    void readTuple();
    void readStatus();
    void readBasicStatus();
    void readExtendedStatus();
    void readCustomProperty();
    void readContact();
    void readNote();
    void readTimestamp();
    void readPerson();
    void readDisplayName();
    void readPresentity();

    QString obtainLangNote(const QMap<QString, QString> &) const;

    // Xpidf elements
    void readAtom();
    void readAddress();
    void readStatusAttributes();
    void readMsnSubStatus();

    QCollectivePresenceInfo m_info;
    QString m_contentType;
    QList<SipPresenceData> m_presenceList;
    QMap<QString, QString> m_globalLangNotes;
    SipPresenceData m_curPresence;
};

#endif
