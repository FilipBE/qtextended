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

#ifndef TESTPHONELAUNCHERVIEW_H
#define TESTPHONELAUNCHERVIEW_H

#include "testwidget.h"

#include <QContent>

class TestPhoneLauncherView : public TestWidget,
    public QtUiTest::ListWidget,
    public QtUiTest::SelectWidget
{
    Q_OBJECT
    Q_INTERFACES(
            QtUiTest::ListWidget
            QtUiTest::SelectWidget)

public:
    TestPhoneLauncherView(QObject*);

    virtual QStringList list() const;
    virtual QRect visualRect(QString const&) const;

    virtual bool canSelect(QString const&) const;
    virtual bool select(QString const&);

    virtual bool inherits(QtUiTest::WidgetType) const;

    static bool canWrap(QObject*);

signals:
    void selected(const QString&);

protected:
    bool eventFilter(QObject*,QEvent*);

private slots:
    void on_pressed(QContent);

private:
    void init();
    QContent *itemAt(int i, int j) const;
    QContent *currentItem() const;

    QObject *q;
    int rows;
    int columns;
    int itemWidth;
    int itemHeight;
};

#endif

