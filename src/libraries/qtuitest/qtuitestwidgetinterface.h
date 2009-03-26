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

#ifndef QTUITESTWIDGETINTERFACE_H
#define QTUITESTWIDGETINTERFACE_H

#include <QStringList>
#include <QtGlobal>
#include <qtuitestnamespace.h>
#include <qtuitestglobal.h>

class QRect;
class QRegion;
class QPoint;


namespace QtUiTest
{
#ifndef Q_QDOC
    template<class T> T qtuitest_cast_helper(QObject*,T);
#endif

#define QTUITEST_INTERFACE(Klass)                                    \
    public:                                                            \
        virtual ~Klass() {}                                            \
        static const char* _q_interfaceName() { return #Klass; }       \
    private:                                                           \
        template<class T> friend T qtuitest_cast_helper(QObject*,T);

    class QTUITEST_EXPORT WidgetFactory
    {
        QTUITEST_INTERFACE(WidgetFactory)
    public:
        virtual QObject* create(QObject*) = 0;
        virtual QObject* find(QtUiTest::WidgetType);
        virtual QStringList keys() const = 0;
    };

    class QTUITEST_EXPORT Widget
    {
        QTUITEST_INTERFACE(Widget)
    public:
        virtual const QRect& geometry() const = 0;
        virtual QRect rect() const;
        virtual bool isVisible() const = 0;
        virtual QRegion visibleRegion() const = 0;
        virtual QObject* parent() const = 0;
        virtual QString windowTitle() const;
        virtual const QObjectList &children() const = 0;
        virtual QRegion childrenVisibleRegion() const;
        virtual QPoint mapToGlobal(const QPoint&) const = 0;
        virtual QPoint mapFromGlobal(const QPoint&) const = 0;
        virtual bool ensureVisibleRegion(const QRegion&) = 0;
        bool ensureVisiblePoint(const QPoint&);
        virtual bool setFocus();
        virtual void focusOutEvent();
        virtual bool hasFocus() const = 0;
        virtual Qt::WindowFlags windowFlags() const;
#ifdef Q_WS_QWS
        virtual bool hasEditFocus() const = 0;
        virtual bool setEditFocus(bool)   = 0;
#else
        virtual bool hasEditFocus() const;
        virtual bool setEditFocus(bool);
#endif
        virtual bool inherits(QtUiTest::WidgetType) const;

#ifdef Q_QDOC
    signals:
        void gotFocus();
#endif
    };

    class QTUITEST_EXPORT ActivateWidget
    {
        QTUITEST_INTERFACE(ActivateWidget)
    public:
        virtual bool activate() = 0;

#ifdef Q_QDOC
    signals:
        void activated();
#endif
    };

    class QTUITEST_EXPORT LabelWidget
    {
        QTUITEST_INTERFACE(LabelWidget)
    public:
        virtual QString labelText() const = 0;
    };

    class QTUITEST_EXPORT CheckWidget
    {
        QTUITEST_INTERFACE(CheckWidget)
    public:
        virtual bool isTristate() const;
        virtual Qt::CheckState checkState() const = 0;
        virtual bool setCheckState(Qt::CheckState);

#ifdef Q_QDOC
    signals:
        void stateChanged(int);
#endif
    };

    class QTUITEST_EXPORT TextWidget
    {
        QTUITEST_INTERFACE(TextWidget)
    public:
        virtual QString selectedText() const;
        virtual QString text() const = 0;
    };

    class QTUITEST_EXPORT ListWidget
    {
        QTUITEST_INTERFACE(ListWidget)
    public:
        virtual QStringList list() const = 0;
        virtual QRect visualRect(const QString&) const = 0;
        virtual bool ensureVisible(const QString&);
    };

    class QTUITEST_EXPORT InputWidget
    {
        QTUITEST_INTERFACE(InputWidget)
    public:
        virtual bool canEnter(const QVariant&) const = 0;
        virtual bool enter(const QVariant&,bool) = 0;

#ifdef Q_QDOC
    signals:
        void entered(const QVariant&);
#endif
    };

    class QTUITEST_EXPORT SelectWidget
    {
        QTUITEST_INTERFACE(SelectWidget)
    public:
        virtual bool isMultiSelection() const;

        virtual bool canSelect(const QString&) const = 0;
        virtual bool canSelectMulti(const QStringList&) const;
        virtual bool select(const QString&) = 0;
        virtual bool selectMulti(const QStringList&);

#ifdef Q_QDOC
    signals:
        void selected(const QString&);
#endif
    };

};

Q_DECLARE_INTERFACE(
        QtUiTest::WidgetFactory,
        "com.trolltech.QtUiTest.WidgetFactory/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::Widget,
        "com.trolltech.QtUiTest.Widget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::ActivateWidget,
        "com.trolltech.QtUiTest.ActivateWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::LabelWidget,
        "com.trolltech.QtUiTest.LabelWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::CheckWidget,
        "com.trolltech.QtUiTest.CheckWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::TextWidget,
        "com.trolltech.QtUiTest.TextWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::ListWidget,
        "com.trolltech.QtUiTest.ListWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::InputWidget,
        "com.trolltech.QtUiTest.InputWidget/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::SelectWidget,
        "com.trolltech.QtUiTest.SelectWidget/1.0")

#endif

