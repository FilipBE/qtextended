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

#include "remotewidget.h"
#include "remotewidgetadaptor.h"
#include "testwidgetslog.h"

#include <QRect>
#include <QRegion>
#include <QtopiaIpcAdaptor>
#include <QUuid>
#include <QPair>
#include <QByteArray>
#include <QHash>
#include <QTimer>

/*! \class RemoteWidget
    \inpublicgroup QtUiTestModule
    \brief LocalWidget and RemoteWidget provide transparent interprocess widget
    operations.

    RemoteWidget encapsulates the widget in a process where the real
    underlying widget is not located.  Every time a function is called or
    a RemoteWidget is cast to another type, a message is sent to the process
    containing the real widget (wrapped in a LocalWidget) and the value is
    returned to this process.

    You do not have to know anything about this class to use it.  Some
    functions in QtUiTest might return a RemoteWidget* without your
    knowledge; as long as you always use qtuitest_cast to get the interface
    that you need, everything will work transparently.

    Note that using this class could be very expensive.  Every function call
    and cast results in at least one round trip between an application and
    the server.
*/

Q_DECLARE_METATYPE(QRect)
Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QList<QUuid>)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(Qt::CheckState)
Q_DECLARE_METATYPE(Qt::WindowFlags)
Q_DECLARE_METATYPE(QtUiTest::WidgetType)

struct RemoteWidgetPrivate
{
    QUuid id;
    QRect geometry;
    QObjectList children;
};

static quint32 request_id = 0;

template<typename Tret>
struct Proxy {

    static Tret call(QUuid const& id, char const* member) {
        quint32 request = request_id++;
        RemoteWidgetAdaptor::instance()->send(member) << request << id;
        QVariant v;
        RemoteWidgetAdaptor::instance()->waitForResponse(request, id, member, &v);
        return qvariant_cast<Tret>(v);
    }

    template<typename Tparam>
    static Tret call(QUuid const& id, char const* member, Tparam const& val) {
        quint32 request = request_id++;
        RemoteWidgetAdaptor::instance()->send(member) << request << id << val;
        QVariant v;
        RemoteWidgetAdaptor::instance()->waitForResponse(request, id, member, &v);
        return qvariant_cast<Tret>(v);
    }

    template<typename Tparam1, typename Tparam2>
    static Tret call(QUuid const& id, char const* member, Tparam1 const& val1, Tparam2 const& val2) {
        quint32 request = request_id++;
        RemoteWidgetAdaptor::instance()->send(member) << request << id << val1 << val2;
        QVariant v;
        RemoteWidgetAdaptor::instance()->waitForResponse(request, id, member, &v);
        return qvariant_cast<Tret>(v);
    }
};

template<>
void Proxy<void>::call(QUuid const& id, char const* member) {
    quint32 request = request_id++;
    RemoteWidgetAdaptor::instance()->send(member) << request << id;
    RemoteWidgetAdaptor::instance()->waitForResponse(request, id, member, 0);
}

template<>
template<typename Tparam>
void Proxy<void>::call(QUuid const& id, char const* member, Tparam const& val) {
    quint32 request = request_id++;
    RemoteWidgetAdaptor::instance()->send(member) << request << id << val;
    RemoteWidgetAdaptor::instance()->waitForResponse(request, id, member, 0);
}


#define PROXY(type, ...) \
    Proxy<type>::call(d->id, \
            RemoteWidgetAdaptor::prettyFunctionToMessage(__func__, __PRETTY_FUNCTION__), ##__VA_ARGS__)

/*!
    Finds and returns a remote widget pointer to the widget of \a type.
    This will send a message to the server resulting in LocalWidget::find()
    being executed, and the widget returned from that will be wrapped in
    a RemoteWidget.
*/
RemoteWidget* RemoteWidget::find(QtUiTest::WidgetType type)
{
    QUuid id = Proxy<QUuid>::call(QUuid(), MESSAGE(find(quint32,QUuid,int)), (int)type);

    TestWidgetsLog() << id;
    if (id.isNull())
        return 0;

    RemoteWidget *ret = create(id);
    RemoteWidgetAdaptor::instance()->addRemoteWidget(ret, id);
    return ret;
}

static QHash<QUuid, QPointer<RemoteWidget> > remoteWidgets;

RemoteWidget* RemoteWidget::create(QUuid const& id)
{
    if (!id.isNull()) {
        QPointer<RemoteWidget> &ptr = remoteWidgets[id];
        if (!ptr)
            remoteWidgets.insert(id, new RemoteWidget(id));
    }
    return remoteWidgets.value(id);
}

RemoteWidget::RemoteWidget(QUuid const& id)
    : d(new RemoteWidgetPrivate)
{
    d->id = id;
}

RemoteWidget::~RemoteWidget()
{ delete d; }

/********************* SIMPLE WRAPPER FUNCTIONS ******************************/

const QRect& RemoteWidget::geometry() const
{
    d->geometry = PROXY(QRect);
    return d->geometry;
}

QRect RemoteWidget::rect() const
{ return PROXY(QRect); }

bool RemoteWidget::isVisible() const
{ return PROXY(bool); }

QObject* RemoteWidget::parent() const
{ return PROXY(QObject*); }

QString RemoteWidget::windowTitle() const
{ return PROXY(QString); }

const QObjectList &RemoteWidget::children() const
{
    d->children = PROXY(QObjectList);
    return d->children;
}

QRegion RemoteWidget::visibleRegion() const
{ return PROXY(QRegion); }

QRegion RemoteWidget::childrenVisibleRegion() const
{ return PROXY(QRegion); }

QPoint RemoteWidget::mapToGlobal(const QPoint& pos) const
{ return PROXY(QPoint, pos); }

QPoint RemoteWidget::mapFromGlobal(const QPoint& pos) const
{ return PROXY(QPoint, pos); }

bool RemoteWidget::ensureVisibleRegion(const QRegion& r)
{ return PROXY(bool, r); }

bool RemoteWidget::setFocus()
{ return PROXY(bool); }

bool RemoteWidget::setEditFocus(bool enable)
{ return PROXY(bool, enable); }

void RemoteWidget::focusOutEvent()
{ PROXY(void); }

bool RemoteWidget::hasFocus() const
{ return PROXY(bool); }

bool RemoteWidget::hasEditFocus() const
{ return PROXY(bool); }

Qt::WindowFlags RemoteWidget::windowFlags() const
{ return PROXY(Qt::WindowFlags); }

bool RemoteWidget::inherits(QtUiTest::WidgetType type) const
{ return PROXY(bool, type); }

bool RemoteWidget::activate()
{ return PROXY(bool); }

QString RemoteWidget::labelText() const
{ return PROXY(QString); }

bool RemoteWidget::isTristate() const
{ return PROXY(bool); }

Qt::CheckState RemoteWidget::checkState() const
{ return PROXY(Qt::CheckState); }

bool RemoteWidget::setCheckState(Qt::CheckState state)
{ return PROXY(bool, state); }

QString RemoteWidget::selectedText() const
{ return PROXY(QString); }

QString RemoteWidget::text() const
{ return PROXY(QString); }

QStringList RemoteWidget::list() const
{ return PROXY(QStringList); }

QRect RemoteWidget::visualRect(const QString& item) const
{ return PROXY(QRect, item); }

bool RemoteWidget::ensureVisible(const QString& item)
{ return PROXY(bool, item); }

bool RemoteWidget::canEnter(const QVariant& item) const
{ return PROXY(bool, item); }

bool RemoteWidget::enter(const QVariant& item, bool noCommit)
{ return PROXY(bool, item, noCommit); }

bool RemoteWidget::isMultiSelection() const
{ return PROXY(bool); }

bool RemoteWidget::canSelect(const QString& item) const
{ return PROXY(bool, item); }

bool RemoteWidget::canSelectMulti(const QStringList& items) const
{ return PROXY(bool, items); }

bool RemoteWidget::select(const QString& item)
{ return PROXY(bool, item); }

bool RemoteWidget::selectMulti(const QStringList& items)
{ return PROXY(bool, items); }


/******************************* MOC *****************************************/

static const uint qt_meta_data_RemoteWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_RemoteWidget[] = {
    "RemoteWidget\0"
};

const QMetaObject RemoteWidget::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RemoteWidget,
      qt_meta_data_RemoteWidget, 0 }
};

const QMetaObject *RemoteWidget::metaObject() const
{
    return &staticMetaObject;
}

void *RemoteWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RemoteWidget))
	return static_cast<void*>(const_cast< RemoteWidget*>(this));

#define REMOTECAST Proxy<bool>::call(d->id, MESSAGE(canCast(quint32,QUuid,QByteArray)), QByteArray(_clname))

    if (!strcmp(_clname, "QtUiTest::Widget") && REMOTECAST)
	return static_cast< QtUiTest::Widget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.Widget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::Widget*>(const_cast< RemoteWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::CheckWidget") && REMOTECAST)
	return static_cast< QtUiTest::CheckWidget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.CheckWidget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::CheckWidget*>(const_cast< RemoteWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::TextWidget") && REMOTECAST)
	return static_cast< QtUiTest::TextWidget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.TextWidget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::TextWidget*>(const_cast< RemoteWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::ListWidget") && REMOTECAST)
	return static_cast< QtUiTest::ListWidget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.ListWidget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::ListWidget*>(const_cast< RemoteWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::InputWidget") && REMOTECAST)
	return static_cast< QtUiTest::InputWidget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.InputWidget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::InputWidget*>(const_cast< RemoteWidget*>(this));

    if (!strcmp(_clname, "QtUiTest::SelectWidget") && REMOTECAST)
	return static_cast< QtUiTest::SelectWidget*>(const_cast< RemoteWidget*>(this));
    if (!strcmp(_clname, "com.trolltech.QtUiTest.SelectWidget/1.0") && REMOTECAST)
	return static_cast< QtUiTest::SelectWidget*>(const_cast< RemoteWidget*>(this));

#undef REMOTECAST

    return QObject::qt_metacast(_clname);
}

