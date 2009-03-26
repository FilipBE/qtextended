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

#include "localwidget.h"
#include "testwidgetslog.h"

#include <QApplication>
#include <QHash>
#include <QPointer>
#include <QRect>
#include <QRegion>
#include <QWidget>

/*! \class LocalWidget
    \inpublicgroup QtUiTestModule
    \brief LocalWidget and RemoteWidget provide transparent interprocess widget
    operations.

    LocalWidget encapsulates the widget in the process where the real
    underlying widget is located.  It is a very thin wrapper whose only purpose
    is to provide an interface through which all QtUiTest widget functions
    can be called but casts still work correctly.

    LocalWidget simply passes all function calls through to the real underlying
    test widget.

    You do not have to know anything about this class to use it.  Some
    functions in QtUiTest might return a LocalWidget* without your
    knowledge; as long as you always use qtuitest_cast to get the interface
    that you need, everything will work transparently.
*/

/*!
    Finds and returns a pointer to an object which inherits \a klass in this
    process.  If \a parent is given, only search among the children of
    \a parent.  Only returns the raw object, does not wrap it in any kind of
    test widget.

    If \a filter is given, only returns an object for which filter evaluates
    to true.
*/
QObject* LocalWidget::find(const QByteArray& klass, QObject *parent, FindFilter filter)
{
    QObjectList topLevel;
    if (parent)
        topLevel << parent;
    else {
        topLevel << qApp;
        foreach (QWidget *w, qApp->topLevelWidgets())
            topLevel << w;
    }

    struct Find {
        static QObject* find(QByteArray const& klass, QObject *parent, FindFilter filter) {
            //TestWidgetsLog() << "looking at" << parent << "to find" << klass;
            if (!parent
                || (parent->inherits(klass)
                    && (!filter || filter(parent)))
                )
                return parent;
            foreach (QObject* o, parent->children())
                if (QObject *ret = Find::find(klass, o, filter)) return ret;
            return 0;
        }
    };

    foreach (QObject *o, topLevel) {
        if (QObject *ret = Find::find(klass, o, filter))
            return ret;
    }
    return 0;
}

/*!
    Finds and returns a pointer to the widget of \a type in this process.
    Only returns the raw object, does not wrap it in any kind of test widget.
*/
QObject* LocalWidget::find(QtUiTest::WidgetType type)
{
    struct Find {
        static bool tabBarFilter(const QObject* o) {
            return !qobject_cast<const QWidget*>(o)->visibleRegion().isEmpty();
        }
        static QObject* tabBar() {
            return find("QTabBar", 0, Find::tabBarFilter);
        }

        static QObject* focus() {
            if (!QApplication::activeWindow()) return 0;

            QObject *f = 0;
            if (!f)
                f = QWidget::keyboardGrabber();
            if (!f) {
                if (QWidget *apw = QApplication::activePopupWidget())
                    f = apw->focusWidget() ? apw->focusWidget() : apw;
            }
            if (!f)
                f = QApplication::focusWidget();

            return f;
        }
    };

    if (QtUiTest::TabBar == type) {
        return Find::tabBar();
    }
    if (QtUiTest::Focus == type) {
        return Find::focus();
    }

    return 0;
}

static QHash<QObject*, QPointer<LocalWidget> > localWidgets;

LocalWidget* LocalWidget::create(QObject* o)
{
    if (o) {
        QPointer<LocalWidget> &ptr = localWidgets[o];
        if (!ptr) {
            ptr = new LocalWidget(o);
            connect(o, SIGNAL(destroyed()), ptr, SLOT(deleteLater()));
        }
    }
    return localWidgets.value(o);
}

/*!
    Create a local widget to wrap \a _q.
    \a _q can be a real widget or a test widget.
*/
LocalWidget::LocalWidget(QObject* _q)
    : q(_q)
{
}

LocalWidget::~LocalWidget()
{
    if (localWidgets.contains(q) && localWidgets.value(q) == this)
        localWidgets.remove(q);
}

QObject* LocalWidget::wrappedObject() const
{ return q; }

void LocalWidget::setWrappedObject(QObject* new_q)
{
    /* This function should not be called on a widget created from
     * LocalWidget::create(). */
    Q_ASSERT(!localWidgets.contains(q) || localWidgets.value(q) != this);
    q = new_q;
}

/*********************** SIMPLE WRAPPER FUNCTIONS ****************************/

const QRect& LocalWidget::geometry() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->geometry(); }

QRect LocalWidget::rect() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->rect(); }

bool LocalWidget::isVisible() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->isVisible(); }

QRegion LocalWidget::visibleRegion() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->visibleRegion(); }

QObject* LocalWidget::parent() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->parent(); }

QString LocalWidget::windowTitle() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->windowTitle(); }

const QObjectList &LocalWidget::children() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->children(); }

QRegion LocalWidget::childrenVisibleRegion() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->childrenVisibleRegion(); }

QPoint LocalWidget::mapToGlobal(const QPoint& pos) const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->mapToGlobal(pos); }

QPoint LocalWidget::mapFromGlobal(const QPoint& pos) const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->mapFromGlobal(pos); }

bool LocalWidget::ensureVisibleRegion(const QRegion& r)
{ return qtuitest_cast<QtUiTest::Widget*>(q)->ensureVisibleRegion(r); }

bool LocalWidget::setFocus()
{ return qtuitest_cast<QtUiTest::Widget*>(q)->setFocus(); }

bool LocalWidget::setEditFocus(bool enable)
{ return qtuitest_cast<QtUiTest::Widget*>(q)->setEditFocus(enable); }

void LocalWidget::focusOutEvent()
{ qtuitest_cast<QtUiTest::Widget*>(q)->focusOutEvent(); }

bool LocalWidget::hasFocus() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->hasFocus(); }

bool LocalWidget::hasEditFocus() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->hasEditFocus(); }

Qt::WindowFlags LocalWidget::windowFlags() const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->windowFlags(); }

bool LocalWidget::inherits(QtUiTest::WidgetType type) const
{ return qtuitest_cast<QtUiTest::Widget*>(q)->inherits(type); }

bool LocalWidget::activate()
{ return qtuitest_cast<QtUiTest::ActivateWidget*>(q)->activate(); }

QString LocalWidget::labelText() const
{ return qtuitest_cast<QtUiTest::LabelWidget*>(q)->labelText(); }

bool LocalWidget::isTristate() const
{ return qtuitest_cast<QtUiTest::CheckWidget*>(q)->isTristate(); }

Qt::CheckState LocalWidget::checkState() const
{ return qtuitest_cast<QtUiTest::CheckWidget*>(q)->checkState(); }

bool LocalWidget::setCheckState(Qt::CheckState state)
{ return qtuitest_cast<QtUiTest::CheckWidget*>(q)->setCheckState(state); }

QString LocalWidget::selectedText() const
{ return qtuitest_cast<QtUiTest::TextWidget*>(q)->selectedText(); }

QString LocalWidget::text() const
{ return qtuitest_cast<QtUiTest::TextWidget*>(q)->text(); }

QStringList LocalWidget::list() const
{ return qtuitest_cast<QtUiTest::ListWidget*>(q)->list(); }

QRect LocalWidget::visualRect(const QString& item) const
{ return qtuitest_cast<QtUiTest::ListWidget*>(q)->visualRect(item); }

bool LocalWidget::ensureVisible(const QString& item)
{ return qtuitest_cast<QtUiTest::ListWidget*>(q)->ensureVisible(item); }

bool LocalWidget::canEnter(const QVariant& v) const
{ return qtuitest_cast<QtUiTest::InputWidget*>(q)->canEnter(v); }

bool LocalWidget::enter(const QVariant& v, bool noCommit)
{ return qtuitest_cast<QtUiTest::InputWidget*>(q)->enter(v, noCommit); }

bool LocalWidget::isMultiSelection() const
{ return qtuitest_cast<QtUiTest::SelectWidget*>(q)->isMultiSelection(); }

bool LocalWidget::canSelect(const QString& v) const
{ return qtuitest_cast<QtUiTest::SelectWidget*>(q)->canSelect(v); }

bool LocalWidget::canSelectMulti(const QStringList& v) const
{ return qtuitest_cast<QtUiTest::SelectWidget*>(q)->canSelectMulti(v); }

bool LocalWidget::select(const QString& v)
{ return qtuitest_cast<QtUiTest::SelectWidget*>(q)->select(v); }

bool LocalWidget::selectMulti(const QStringList& v)
{ return qtuitest_cast<QtUiTest::SelectWidget*>(q)->selectMulti(v); }



/******************************* MOC *****************************************/
/* A custom meta object is written to make sure casts work as expected.
 * i.e., when one does qtuitest_cast<Interface>(some_local_widget), it
 * succeeds if and only if the real testwidget supports Interface.
 */

static const uint qt_meta_data_LocalWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_LocalWidget[] = {
    "LocalWidget\0"
};

const QMetaObject LocalWidget::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_LocalWidget,
      qt_meta_data_LocalWidget, 0 }
};

const QMetaObject *LocalWidget::metaObject() const
{
    return &staticMetaObject;
}

void *LocalWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LocalWidget))
	return static_cast<void*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::Widget") && qtuitest_cast<QtUiTest::Widget*>(q))
	return static_cast< QtUiTest::Widget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.Widget/1.0") && qtuitest_cast<QtUiTest::Widget*>(q))
	return static_cast< QtUiTest::Widget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::CheckWidget") && qtuitest_cast<QtUiTest::CheckWidget*>(q))
	return static_cast< QtUiTest::CheckWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.CheckWidget/1.0") && qtuitest_cast<QtUiTest::CheckWidget*>(q))
	return static_cast< QtUiTest::CheckWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::ActivateWidget") && qtuitest_cast<QtUiTest::ActivateWidget*>(q))
	return static_cast< QtUiTest::ActivateWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.ActivateWidget/1.0") && qtuitest_cast<QtUiTest::ActivateWidget*>(q))
	return static_cast< QtUiTest::ActivateWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::LabelWidget") && qtuitest_cast<QtUiTest::LabelWidget*>(q))
	return static_cast< QtUiTest::LabelWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.LabelWidget/1.0") && qtuitest_cast<QtUiTest::LabelWidget*>(q))
	return static_cast< QtUiTest::LabelWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::TextWidget") && qtuitest_cast<QtUiTest::TextWidget*>(q))
	return static_cast< QtUiTest::TextWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.TextWidget/1.0") && qtuitest_cast<QtUiTest::TextWidget*>(q))
	return static_cast< QtUiTest::TextWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::ListWidget") && qtuitest_cast<QtUiTest::ListWidget*>(q))
	return static_cast< QtUiTest::ListWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.ListWidget/1.0") && qtuitest_cast<QtUiTest::ListWidget*>(q))
	return static_cast< QtUiTest::ListWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::InputWidget") && qtuitest_cast<QtUiTest::InputWidget*>(q))
	return static_cast< QtUiTest::InputWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.InputWidget/1.0") && qtuitest_cast<QtUiTest::InputWidget*>(q))
	return static_cast< QtUiTest::InputWidget*>(const_cast< LocalWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::SelectWidget") && qtuitest_cast<QtUiTest::SelectWidget*>(q))
	return static_cast< QtUiTest::SelectWidget*>(const_cast< LocalWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.SelectWidget/1.0") && qtuitest_cast<QtUiTest::SelectWidget*>(q))
	return static_cast< QtUiTest::SelectWidget*>(const_cast< LocalWidget*>(this));

    return QObject::qt_metacast(_clname);
}

