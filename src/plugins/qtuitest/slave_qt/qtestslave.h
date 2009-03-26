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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef QTESTSLAVE_H
#define QTESTSLAVE_H

#include <qtestprotocol_p.h>
#include <recordevent_p.h>

class QWSEvent;
class QTestSlavePrivate;

class QTUITEST_EXPORT QTestSlave : public QTestProtocol
{
    Q_OBJECT
public:
    QTestSlave();
    virtual ~QTestSlave();

    virtual void processMessage(QTestMessage*);
    virtual QTestMessage constructReplyToMessage(QTestMessage const&);

    virtual void showMessageBox(QWidget*,QString const&,QString const&);
    virtual void showDialog(QWidget*,QString const&);

    bool recordingEvents() const;

public slots:
    virtual void onConnected();

protected:
    QString processEnvironment(QString const&) const;
    void setRecordingEvents(bool);

    virtual void recordEvent(RecordEvent::Type,QString const&,QString const&,QVariant const& = QVariant());

private:
    friend class QTestSlavePrivate;
    QTestSlavePrivate *d;
};

#endif
