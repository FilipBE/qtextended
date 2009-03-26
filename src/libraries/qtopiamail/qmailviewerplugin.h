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

#ifndef QMAILVIEWERPLUGIN_H
#define QMAILVIEWERPLUGIN_H

#include <QString>
#include <QIcon>
#include <qfactoryinterface.h>

#include <qtopiaglobal.h>
#include <qmailviewer.h>

struct QTOPIAMAIL_EXPORT QMailViewerPluginInterface : public QFactoryInterface
{
    virtual QString key() const = 0;

    virtual bool isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const = 0;
    virtual QMailViewerInterface *create(QWidget *parent) = 0;
};

#define QMailViewerPluginInterface_iid "com.trolltech.Qtopia.Qtopiamail.QMailViewerPluginInterface"
Q_DECLARE_INTERFACE(QMailViewerPluginInterface, QMailViewerPluginInterface_iid)

class QTOPIAMAIL_EXPORT QMailViewerPlugin : public QObject, public QMailViewerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMailViewerPluginInterface:QFactoryInterface)

public:
    QMailViewerPlugin();
    ~QMailViewerPlugin();

    virtual QStringList keys() const;
};

#endif
