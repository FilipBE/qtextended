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

#ifndef QPREFIXMATCHER_P_H
#define QPREFIXMATCHER_P_H

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

#include <qobject.h>
#include <qset.h>

class QPrefixMatcherNode;

class QPrefixMatcher : public QObject
{
    Q_OBJECT
public:
    explicit QPrefixMatcher( QObject *parent );
    ~QPrefixMatcher();

    enum Type
    {
        Unknown,
        OKTerminator,
        Terminator,
        FuzzyTerminator,
        TerminatorOrNotification,
        Ignore,
        Notification,
        NotificationWithPDU,
        CommandEcho
    };

    void add( const QString& prefix, QPrefixMatcher::Type type,
              bool mayBeCommand = false, QObject *target = 0,
              const char *slot = 0 );
    QPrefixMatcher::Type lookup
        ( const QString& value, const QString& command ) const;

private slots:
    void targetDestroyed();

private:
    QSet<QObject *> targets;
    QPrefixMatcherNode *root;
};

#endif
