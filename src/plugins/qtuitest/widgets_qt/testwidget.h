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

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QObject>
#include <qtuitestwidgetinterface.h>

class QWidget;

class TestWidget : public QObject, public QtUiTest::Widget,
    public QtUiTest::InputWidget
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::Widget QtUiTest::InputWidget)

public:
    TestWidget(QObject*);

    virtual const QRect& geometry() const;
    virtual QRect rect() const;
    virtual bool isVisible() const;
    virtual QRegion visibleRegion() const;
    virtual const QObjectList &children() const;
    virtual QObject* parent() const;
    virtual QString windowTitle() const;
    virtual QPoint mapToGlobal(QPoint const&) const;
    virtual QPoint mapFromGlobal(QPoint const&) const;
    virtual bool hasFocus() const;
    virtual Qt::WindowFlags windowFlags() const;
    virtual bool ensureVisibleRegion(QRegion const&);
    virtual bool canEnter(QVariant const&) const;
    virtual bool enter(QVariant const&,bool);
    virtual void focusOutEvent();

#ifdef Q_WS_QWS
    virtual bool hasEditFocus() const;
#endif
    virtual bool setEditFocus(bool);

    static bool canWrap(QObject*);
    static QString printable(QString const&);

signals:
    void gotFocus();

protected:
    bool eventFilter(QObject*,QEvent*);

private:
    static QWidget *focusWidget();

    QWidget *q;
};

#endif

