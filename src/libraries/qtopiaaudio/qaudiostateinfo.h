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

#ifndef QAUDIOSTATEINFO_H
#define QAUDIOSTATEINFO_H

#include <qtopiaglobal.h>
#include <qglobal.h>

class QAudioStateInfoPrivate;

class QTOPIAAUDIO_EXPORT QAudioStateInfo
{
public:
    QAudioStateInfo(const QByteArray &domain, const QByteArray &profile,
                    const QString &displayName, int priority);
    QAudioStateInfo();
    QAudioStateInfo(const QAudioStateInfo &state);
    ~QAudioStateInfo();

    bool isValid() const;

    QAudioStateInfo &operator=(const QAudioStateInfo &state);

    bool operator==(const QAudioStateInfo &state) const;
    inline bool operator!=(const QAudioStateInfo &state) const { return !operator==(state); }

    QByteArray domain() const;
    void setDomain(const QByteArray &domain);

    QByteArray profile() const;
    void setProfile(const QByteArray &profile);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int priority() const;
    void setPriority(int priority);

private:
    QAudioStateInfoPrivate *m_data;
};

QTOPIAAUDIO_EXPORT uint qHash(const QAudioStateInfo &key);
QTOPIAAUDIO_EXPORT QDebug operator<<(QDebug, const QAudioStateInfo &);

#endif
