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

#ifndef LOCALWIDGET_H
#define LOCALWIDGET_H

#include <qtuitestwidgetinterface.h>

class LocalWidgetPrivate;

class LocalWidget : public QObject,
    public QtUiTest::Widget,
    public QtUiTest::ActivateWidget,
    public QtUiTest::LabelWidget,
    public QtUiTest::CheckWidget,
    public QtUiTest::TextWidget,
    public QtUiTest::ListWidget,
    public QtUiTest::InputWidget,
    public QtUiTest::SelectWidget
{
public:
    Q_OBJECT_CHECK
    virtual ~LocalWidget();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);

    virtual const QRect& geometry() const;
    virtual QRect rect() const;
    virtual bool isVisible() const;
    virtual QRegion visibleRegion() const;
    virtual QObject* parent() const;
    virtual QString windowTitle() const;
    virtual const QObjectList &children() const;
    virtual QRegion childrenVisibleRegion() const;
    virtual QPoint mapToGlobal(const QPoint&) const;
    virtual QPoint mapFromGlobal(const QPoint&) const;
    virtual bool ensureVisibleRegion(const QRegion&);
    virtual bool setFocus();
    virtual bool setEditFocus(bool);
    virtual void focusOutEvent();
    virtual bool hasFocus() const;
    virtual bool hasEditFocus() const;
    virtual Qt::WindowFlags windowFlags() const;
    virtual bool inherits(QtUiTest::WidgetType) const;

    virtual bool activate();

    virtual QString labelText() const;

    virtual bool isTristate() const;
    virtual Qt::CheckState checkState() const;
    virtual bool setCheckState(Qt::CheckState);

    virtual QString selectedText() const;
    virtual QString text() const;

    virtual QStringList list() const;
    virtual QRect visualRect(const QString&) const;
    virtual bool ensureVisible(const QString&);

    virtual bool canEnter(const QVariant&) const;
    virtual bool enter(const QVariant&,bool);

    virtual bool isMultiSelection() const;

    virtual bool canSelect(const QString&) const;
    virtual bool canSelectMulti(const QStringList&) const;
    virtual bool select(const QString&);
    virtual bool selectMulti(const QStringList&);

    typedef bool (*FindFilter)(QObject const*);
    static QObject* find(QtUiTest::WidgetType);
    static QObject* find(const QByteArray&,QObject* =0,FindFilter =0);
    static LocalWidget* create(QObject*);

protected:
    LocalWidget(QObject*);

    QObject* wrappedObject() const;
    void setWrappedObject(QObject*);

private:
    QObject *q;
};

#endif

