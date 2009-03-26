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

#include "remotewidgetadaptor.h"
#include "remotewidget.h"
#include "localwidget.h"
#include "localqtopiawidget.h"
#include "testwidgetslog.h"

#include <QTimer>
#include <QUuid>
#include <QRect>
#include <QRegion>
#include <QApplication>

/*! \class RemoteWidgetAdaptor
    \inpublicgroup QtUiTestModule
    \brief RemoteWidgetAdaptor provides the connection between LocalWidget
    and RemoteWidget objects.

    RemoteWidgetAdaptor encapsulates the communication channel for function
    calls between a RemoteWidget and a corresponding LocalWidget, and maintains
    the mappings between them.

    Currently the only supported configuration is to have LocalWidgets in the
    server and RemoteWidgets in applications.
*/

#define RESPOND(value) do {\
    emit response(req, id, prettyFunctionToMessage(__func__, __PRETTY_FUNCTION__), \
            QVariant::fromValue(value)); \
} while(0)

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QList<QUuid>)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(Qt::CheckState)
Q_DECLARE_METATYPE(Qt::WindowFlags)
Q_DECLARE_METATYPE(QtUiTest::WidgetType)

struct RemoteWidgetAdaptorPrivate
{
    QHash< QPair< QString, QByteArray >, QPair< QVariant*, QTimer* > > waiting;
    QHash< QUuid, RemoteWidget* > remoteWidgets;
    QHash< QUuid, LocalWidget* >  localWidgets;
    QHash< LocalWidget*, QUuid >  localWidgetsReverse;

    QUuid insertLocalWidget(LocalWidget*);

    RemoteWidgetAdaptor* q;
};

QDataStream &operator<<(QDataStream &out, const QList<QUuid> &list)
{
    out << list.count();
    foreach (QUuid id, list) out << id;
    return out;
}

QDataStream &operator>>(QDataStream &in, QList<QUuid> &list)
{
    int count;
    in >> count;
    for (int i = 0; i < count; ++i) {
        QUuid id;
        in >> id;
        list << id;
    }
    return in;
}

QDataStream &operator<<(QDataStream &out, const Qt::CheckState &state)
{
    out << state;
    return out;
}

QDataStream &operator>>(QDataStream &in, Qt::CheckState &state)
{
    in >> state;
    return in;
}

QDataStream &operator<<(QDataStream &out, const QtUiTest::WidgetType &type)
{ return (out << type); }

QDataStream &operator>>(QDataStream &in, QtUiTest::WidgetType &type)
{ return (in >> type); }

RemoteWidgetAdaptor* RemoteWidgetAdaptor::instance()
{
    static RemoteWidgetAdaptor a;
    return &a;
}

RemoteWidgetAdaptor::RemoteWidgetAdaptor(QObject* parent)
    : QtopiaIpcAdaptor("QPE/QtUiTest/RemoteWidgets", parent),
      d(new RemoteWidgetAdaptorPrivate)
{
    static struct MetaTypeRegistrar {
        MetaTypeRegistrar() {
            qRegisterMetaTypeStreamOperators< QList<QUuid> >("QList<QUuid>");
            qRegisterMetaType< QList<QUuid> >();
            qRegisterMetaTypeStreamOperators< Qt::CheckState >("Qt::CheckState");
            qRegisterMetaType< Qt::CheckState >();
            qRegisterMetaTypeStreamOperators< QtUiTest::WidgetType >("QtUiTest::WidgetType");
            qRegisterMetaType< QtUiTest::WidgetType >();
        }
    } mtr;
    d->q = this;
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
    if (qApp->type() != qApp->GuiServer) {
        connect( this, MESSAGE(response(quint32,QUuid,QByteArray,QVariant)),
                 this, SLOT(handleResponse(quint32,QUuid,QByteArray,QVariant)),
                 QtopiaIpcAdaptor::SenderIsChannel );
        connect( this, MESSAGE(remoteWidgetDestroyed(QUuid)),
                 this, SLOT(handleRemoteWidgetDestroyed(QUuid)),
                 QtopiaIpcAdaptor::SenderIsChannel );
    }
}

RemoteWidgetAdaptor::~RemoteWidgetAdaptor()
{
    delete d;
}

void RemoteWidgetAdaptor::handleWidgetDestroyed()
{
    if (LocalWidget* lw = qobject_cast<LocalWidget*>(sender())) {
        QUuid id = d->localWidgetsReverse.take(lw);
        d->localWidgets.remove(id);
        emit remoteWidgetDestroyed(id);
    }
}

void RemoteWidgetAdaptor::handleRemoteWidgetDestroyed(QUuid const& id)
{
    if (RemoteWidget* rw = d->remoteWidgets.take(id))
        rw->deleteLater();
}

void RemoteWidgetAdaptor::waitForResponse(quint32 req, QUuid const& id, char const* member,
        QVariant* out)
{
    QPair< QString, QByteArray > waitInfo = qMakePair(QString("%1%2").arg(id.toString()).arg(req), QByteArray(member));

    /* This timeout needs to be rather large, because some actions (e.g. enter
     * into predictive keyboard) really can take minutes. */
    QTimer dummy;
    d->waiting.insert( waitInfo, qMakePair(out, &dummy) );

    QtUiTest::waitForSignal(&dummy, SIGNAL(timeout()), 300000);

    if (d->waiting.contains(waitInfo)) {
        qWarning() << "QtUiTest: remote widget" << id
                   << "failed to reply to request" << req << member;
        d->waiting.remove(waitInfo);
    }
}

void RemoteWidgetAdaptor::handleResponse(quint32 req, QUuid const& id, QByteArray const& member,
        QVariant const& value)
{
    TestWidgetsLog() << qApp->applicationName() << req << id << member << value;

    QPair< QString, QByteArray > waitInfo = qMakePair(QString("%1%2").arg(id.toString()).arg(req), QByteArray(member));
    if (!d->waiting.contains(waitInfo)) return;

    QVariant out = value;

    /* Special case: parent() should be translated from QUuid to QObject* */
    if (member == MESSAGE(parent(quint32,QUuid))) {
        QUuid id = value.value<QUuid>();
        if (id.isNull())
            out = QVariant::fromValue(static_cast<QObject*>(0));
        else {
            RemoteWidget*& rw = d->remoteWidgets[id];
            if (!rw) {
                rw = RemoteWidget::create(id);
                addRemoteWidget(rw, id);
            }
            out = QVariant::fromValue(static_cast<QObject*>(rw));
        }
    }

    /* Special case: children() should be translated
       from QList<QUuid> to QObjectList */
    if (member == MESSAGE(children(quint32,QUuid))) {
        QList<QUuid> ids = value.value< QList<QUuid> >();
        QObjectList ol;
        foreach (QUuid id, ids) {
            RemoteWidget*& rw = d->remoteWidgets[id];
            if (!rw) {
                rw = RemoteWidget::create(id);
                addRemoteWidget(rw, id);
            }
            ol << rw;
        }
        out = QVariant::fromValue(ol);
    }

    QPair< QVariant*, QTimer* > out_ptr = d->waiting.take(waitInfo);
    if (out_ptr.first) *out_ptr.first = out;
    if (out_ptr.second) QMetaObject::invokeMethod(out_ptr.second, "timeout");
}

void RemoteWidgetAdaptor::addRemoteWidget(RemoteWidget* rw, QUuid const& id)
{
    d->remoteWidgets.insert(id, rw);
}

void RemoteWidgetAdaptor::canCast(quint32 req,QUuid const& id, QByteArray const& clname)
{
    TestWidgetsLog() << id << clname;
    LocalWidget *lw = d->localWidgets.value(id);
    if (lw)
        RESPOND( !!lw->qt_metacast( clname.constData() ) );
}

void RemoteWidgetAdaptor::find(quint32 req,QUuid const& id,int type)
{
    if (qApp->type() != QApplication::GuiServer) return;

    TestWidgetsLog() << qApp->applicationName() << type;

    QUuid new_id;

    QObject *o = LocalQtopiaWidget::find((QtUiTest::WidgetType)type);
    new_id = d->insertLocalWidget(LocalWidget::create(o));

    TestWidgetsLog() << "for type" << type << "found widget" << o << "id" << new_id;
    RESPOND(new_id);
}

QUuid RemoteWidgetAdaptorPrivate::insertLocalWidget(LocalWidget* lw)
{
    QUuid id;
    if (lw) {
        if (localWidgetsReverse.contains(lw))
            id = localWidgetsReverse.value(lw);
        else {
            id = QUuid::createUuid();
            localWidgets.insert(id, lw);
            localWidgetsReverse.insert(lw, id);
            QObject::connect(lw, SIGNAL(destroyed()),
                    q, SLOT(handleWidgetDestroyed()));
        }
    }
    return id;
}

void RemoteWidgetAdaptor::parent(quint32 req,QUuid const& id)
{
    /* Special case; need to return QUuid of parent. */
    QtUiTest::Widget* w
        = qtuitest_cast<QtUiTest::Widget*>(d->localWidgets.value(id));
    if (w) {
        QUuid pid = d->insertLocalWidget(LocalWidget::create(w->parent()));
        RESPOND(pid);
    }
}

void RemoteWidgetAdaptor::children(quint32 req,QUuid const& id)
{
    /* Special case; need to return list of QUuid of children. */
    QList<QUuid> ids;
    QtUiTest::Widget* w
        = qtuitest_cast<QtUiTest::Widget*>(d->localWidgets.value(id));
    if (w) {
        foreach (QObject *child, w->children()) {
            ids << d->insertLocalWidget(LocalWidget::create(child));
        }
        RESPOND(ids);
    }
}

/* Proxy helper; returns the appropriate MESSAGE() to be used from within a
 * particular function
 */
QByteArray RemoteWidgetAdaptor::prettyFunctionToMessage(char const* _func, char const* _prettyFunc)
{
    QString func = QString::fromLatin1(_func);
    QString prettyFunc = QString::fromLatin1(_prettyFunc);
    prettyFunc.replace("QtUiTest::WidgetType", "int");
    QString params = prettyFunc.split(QString("::%1(").arg(func)).at(1);
    params = params.left(params.lastIndexOf(')')+1);
    if (params != ")") params.prepend(',');

    QString sig = QString("%1(quint32,QUuid%2").arg(func).arg(params);
    QByteArray message = QMetaObject::normalizedSignature(qPrintable(sig));
    message.replace("quint32,QUuid,quint32,QUuid", "quint32,QUuid");
    message.prepend( qPrintable(QString::number(QMESSAGE_CODE)) );

    return message;
}

/********************* SIMPLE WRAPPER FUNCTIONS ******************************/


#define PROXY(...) \
    LocalWidget *lw = d->localWidgets.value(id); \
    if (lw) \
        RESPOND( lw-> __VA_ARGS__ );

#define PROXY_VOID(...) \
    LocalWidget *lw = d->localWidgets.value(id); \
    if (lw) {\
        lw-> __VA_ARGS__ ; \
        RESPOND(QVariant()); \
    }

void RemoteWidgetAdaptor::geometry(quint32 req,QUuid const& id)
{ PROXY(geometry()); }

void RemoteWidgetAdaptor::windowTitle(quint32 req,QUuid const& id)
{ PROXY(windowTitle()); }

void RemoteWidgetAdaptor::rect(quint32 req,QUuid const& id)
{ PROXY(rect()); }

void RemoteWidgetAdaptor::isVisible(quint32 req,QUuid const& id)
{ PROXY(isVisible()); }

void RemoteWidgetAdaptor::visibleRegion(quint32 req,QUuid const& id)
{ PROXY(visibleRegion()); }

void RemoteWidgetAdaptor::childrenVisibleRegion(quint32 req,QUuid const& id)
{ PROXY(childrenVisibleRegion()); }

void RemoteWidgetAdaptor::mapToGlobal(quint32 req,QUuid const& id,const QPoint& pos)
{ PROXY(mapToGlobal(pos)); }

void RemoteWidgetAdaptor::mapFromGlobal(quint32 req,QUuid const& id,const QPoint& pos)
{ PROXY(mapFromGlobal(pos)); }

void RemoteWidgetAdaptor::ensureVisibleRegion(quint32 req,QUuid const& id,const QRegion& r)
{ PROXY(ensureVisibleRegion(r)); }

void RemoteWidgetAdaptor::setFocus(quint32 req,QUuid const& id)
{ PROXY(setFocus()); }

void RemoteWidgetAdaptor::setEditFocus(quint32 req,QUuid const& id,bool enable)
{ PROXY(setEditFocus(enable)); }

void RemoteWidgetAdaptor::focusOutEvent(quint32 req,QUuid const& id)
{ PROXY_VOID(focusOutEvent()); }

void RemoteWidgetAdaptor::hasFocus(quint32 req,QUuid const& id)
{ PROXY(hasFocus()); }

void RemoteWidgetAdaptor::hasEditFocus(quint32 req,QUuid const& id)
{ PROXY(hasEditFocus()); }

void RemoteWidgetAdaptor::windowFlags(quint32 req,QUuid const& id)
{ PROXY(windowFlags()); }

void RemoteWidgetAdaptor::inherits(quint32 req,QUuid const& id, QtUiTest::WidgetType type)
{ PROXY(inherits(type)); }

void RemoteWidgetAdaptor::activate(quint32 req,QUuid const& id)
{ PROXY(activate()); }

void RemoteWidgetAdaptor::labelText(quint32 req,QUuid const& id)
{ PROXY(labelText()); }

void RemoteWidgetAdaptor::isTristate(quint32 req,QUuid const& id)
{ PROXY(isTristate()); }

void RemoteWidgetAdaptor::checkState(quint32 req,QUuid const& id)
{ PROXY(checkState()); }

void RemoteWidgetAdaptor::setCheckState(quint32 req,QUuid const& id,Qt::CheckState state)
{ PROXY(setCheckState(state)); }

void RemoteWidgetAdaptor::selectedText(quint32 req,QUuid const& id)
{ PROXY(selectedText()); }

void RemoteWidgetAdaptor::text(quint32 req,QUuid const& id)
{ PROXY(text()); }

void RemoteWidgetAdaptor::list(quint32 req,QUuid const& id)
{ PROXY(list()); }

void RemoteWidgetAdaptor::visualRect(quint32 req,QUuid const& id,const QString& item)
{ PROXY(visualRect(item)); }

void RemoteWidgetAdaptor::ensureVisible(quint32 req,QUuid const& id,const QString& item)
{ PROXY(ensureVisible(item)); }

void RemoteWidgetAdaptor::canEnter(quint32 req,QUuid const& id,const QVariant& v)
{ PROXY(canEnter(v)); }

void RemoteWidgetAdaptor::enter(quint32 req,QUuid const& id,const QVariant& v, bool noCommit)
{ PROXY(enter(v, noCommit)); }

void RemoteWidgetAdaptor::isMultiSelection(quint32 req,QUuid const& id)
{ PROXY(isMultiSelection()); }

void RemoteWidgetAdaptor::canSelect(quint32 req,QUuid const& id,const QString& v)
{ PROXY(canSelect(v)); }

void RemoteWidgetAdaptor::canSelectMulti(quint32 req,QUuid const& id,const QStringList& v)
{ PROXY(canSelectMulti(v)); }

void RemoteWidgetAdaptor::select(quint32 req,QUuid const& id,const QString& v)
{ PROXY(select(v)); }

void RemoteWidgetAdaptor::selectMulti(quint32 req,QUuid const& id,const QStringList& v)
{ PROXY(selectMulti(v)); }

