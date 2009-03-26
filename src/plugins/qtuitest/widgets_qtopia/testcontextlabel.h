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

#ifndef TESTCONTEXTLABEL_H
#define TESTCONTEXTLABEL_H

#include "testthemedview.h"

#include <QTime>

class QtUiTestRecorder;

class TestContextLabel : public TestThemedView
{
    Q_OBJECT
public:
    TestContextLabel(QObject*);

    virtual QString text() const;

    virtual QStringList list() const;

    virtual bool canSelect(QString const&) const;
    virtual bool select(QString const&);

    virtual bool inherits(QtUiTest::WidgetType) const;

    static bool canWrap(QObject*);

protected:
    virtual void connectNotify(const char*);

private slots:
    void on_keyEvent(int,int,bool,bool);
    void on_keyChanged(int);

private:
    QThemedView* q;
    QString lastOptionsLabel;
    int     optionsIndex;
    QtUiTestRecorder* recorder;
    QTime   lastEmit;
};

#endif

