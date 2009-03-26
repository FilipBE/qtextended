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

#ifndef TESTTHEMEDVIEW_H
#define TESTTHEMEDVIEW_H

#include "testwidget.h"

class ThemedView;
class ThemeItem;
class QThemedView;

#include <QThemeItem>

class TestThemedView : public TestWidget, public QtUiTest::TextWidget,
    public QtUiTest::ListWidget, public QtUiTest::SelectWidget
{
    Q_OBJECT
    Q_INTERFACES(
        QtUiTest::TextWidget
        QtUiTest::SelectWidget
        QtUiTest::ListWidget)


public:
    TestThemedView(QObject*);

    virtual QString text() const;

    virtual QStringList list() const;
    virtual QRect visualRect(QString const&) const;

    virtual bool canSelect(QString const&) const;
    virtual bool select(QString const&);

    static bool canWrap(QObject*);

    static bool old_themeItemLessThan(ThemeItem*, ThemeItem*);
    static bool themeItemLessThan(QThemeItem*, QThemeItem*);

signals:
    void selected(const QString&);

private slots:
    void old_on_itemPressed(ThemeItem*);
    void on_itemPressed(QThemeItem*);

protected:
    ThemeItem*  old_itemWithText(QString const&) const;
    QThemeItem* itemWithText(QString const&) const;

    ThemedView *old_q;
    QThemedView *q;
};

template<>
inline QThemeItem* qgraphicsitem_cast(QGraphicsItem* item)
{ return QThemeItem::themeItem(item) ? static_cast<QThemeItem*>(item) : 0; }

#endif

