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

#ifndef REMOTEWIDGETADAPTOR_H
#define REMOTEWIDGETADAPTOR_H

#include <QtopiaIpcAdaptor>
#include <QtUiTest>

class QVariant;
class QUuid;
class RemoteWidget;

class RemoteWidgetAdaptorPrivate;

class RemoteWidgetAdaptor : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    ~RemoteWidgetAdaptor();
    static RemoteWidgetAdaptor* instance();
    void waitForResponse(quint32,QUuid const&, char const*, QVariant* =0);
    void addRemoteWidget(RemoteWidget*,QUuid const&);
    static QByteArray prettyFunctionToMessage(char const*,char const*);

public slots:
    void geometry(quint32,QUuid const&);
    void rect(quint32,QUuid const&);
    void isVisible(quint32,QUuid const&);
    void visibleRegion(quint32,QUuid const&);
    void parent(quint32,QUuid const&);
    void windowTitle(quint32,QUuid const&);
    void children(quint32,QUuid const&);
    void childrenVisibleRegion(quint32,QUuid const&);
    void mapToGlobal(quint32,QUuid const&,const QPoint&);
    void mapFromGlobal(quint32,QUuid const&,const QPoint&);
    void ensureVisibleRegion(quint32,QUuid const&,const QRegion&);
    void setFocus(quint32,QUuid const&);
    void setEditFocus(quint32,QUuid const&,bool);
    void focusOutEvent(quint32,QUuid const&);
    void hasFocus(quint32,QUuid const&);
    void hasEditFocus(quint32,QUuid const&);
    void windowFlags(quint32,QUuid const&);
    void inherits(quint32,QUuid const&,QtUiTest::WidgetType);

    void activate(quint32,QUuid const&);

    void labelText(quint32,QUuid const&);

    void isTristate(quint32,QUuid const&);
    void checkState(quint32,QUuid const&);
    void setCheckState(quint32,QUuid const&,Qt::CheckState);

    void selectedText(quint32,QUuid const&);
    void text(quint32,QUuid const&);

    void list(quint32,QUuid const&);
    void visualRect(quint32,QUuid const&,const QString&);
    void ensureVisible(quint32,QUuid const&,const QString&);

    void canEnter(quint32,QUuid const&,const QVariant&);
    void enter(quint32,QUuid const&,const QVariant&,bool);

    void isMultiSelection(quint32,QUuid const&);

    void canSelect(quint32,QUuid const&,const QString&);
    void canSelectMulti(quint32,QUuid const&,const QStringList&);
    void select(quint32,QUuid const&,const QString&);
    void selectMulti(quint32,QUuid const&,const QStringList&);

    void canCast(quint32,QUuid const&, QByteArray const&);

    void find(quint32,QUuid const&,int);

signals:
    void response(quint32,QUuid const&, QByteArray const&, QVariant const&);
    void remoteWidgetDestroyed(QUuid const&);

private slots:
    void handleResponse(quint32,QUuid const&, QByteArray const&, QVariant const&);
    void handleWidgetDestroyed();
    void handleRemoteWidgetDestroyed(QUuid const&);

private:
    RemoteWidgetAdaptor(QObject* =0);
    RemoteWidgetAdaptorPrivate *d;
};

#endif

