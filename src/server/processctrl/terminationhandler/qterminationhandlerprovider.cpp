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

#include "qterminationhandlerprovider.h"

#include <QtopiaChannel>
#include <QtopiaServiceRequest>
#include <qtopia/private/qterminationhandler_p.h>
#include "applicationmonitor.h"

#include <QString>

class QtopiaChannel;
class TerminationHandlerPrivate;
class TerminationHandler : public ApplicationTerminationHandler
{
    Q_OBJECT
public:
    TerminationHandler();
    virtual ~TerminationHandler();

    virtual bool terminated(const QString &,
                            ApplicationTypeLauncher::TerminationReason);

    void addHandler() { ++handlers; }
    void remHandler() { Q_ASSERT(handlers); --handlers; }

signals:
    void applicationTerminated(const QString &name, const QString &text,
                               const QPixmap &icon, const QString &buttonText,
                               QtopiaServiceRequest &buttonAction);

protected slots:
    void messageReceived(const QString&, const QByteArray&);

private:
    int handlers;
    TerminationHandlerPrivate* d;
};
static TerminationHandler *termInstance = 0;

struct TerminationHandlerPrivate
{
    QMap<QString,QTerminationHandlerData> installedHandlers;
};

TerminationHandler::TerminationHandler()
: handlers(0)
{
    Q_ASSERT(!termInstance);
    termInstance = this;
    d = new TerminationHandlerPrivate;
    QtopiaChannel* channel = new QtopiaChannel("Qtopia/TerminationHandler", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(messageReceived(QString,QByteArray)));
}

TerminationHandler::~TerminationHandler()
{
    delete d;
}

void TerminationHandler::messageReceived(const QString& msg, const QByteArray& data)
{
    QDataStream istream(data);
    if(msg == "installHandler(QTerminationHandlerData)") {
        // name text buttonText buttonIcon action
        QTerminationHandlerData data;
        istream >> data;
        if(d->installedHandlers.contains(data.name))
            d->installedHandlers.remove(data.name);
        d->installedHandlers[data.name] = data;
    } else if(msg == "removeHandler(QString)") {
        // name
        QString name;
        istream >> name;
        if(d->installedHandlers.contains(name))
            d->installedHandlers.remove(name);
    }
}

bool TerminationHandler::terminated(const QString& app,
                        ApplicationTypeLauncher::TerminationReason reason)
{
    if(d->installedHandlers.contains(app)) {
        QTerminationHandlerData data = d->installedHandlers[app];
        d->installedHandlers.remove(app);
        if(reason == ApplicationTypeLauncher::Normal)
            return false;

        if(handlers) {
            emit applicationTerminated(app, data.text, QPixmap(data.buttonIcon),
                                       data.buttonText, data.action);
            return true;
        } else {
            return false;
        }

    } else {
        return false;
    }
}

QTOPIA_TASK(TerminationHandler, TerminationHandler);
QTOPIA_TASK_PROVIDES(TerminationHandler, ApplicationTerminationHandler);

/*!
  \class QTerminationHandlerProvider
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer
  \brief The QTerminationHandlerProvider class provides the backend for the QTerminationHandler class.

  For termination handling to be tracked successfully, the
  \c {TerminationHandler} task must be running prior to starting any
  applications.

  While it is legal to have more than one concurrent instance of
  QTerminationHandlerProvider, it is generally discorouged.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  Creates a new QTerminationHandlerProvider instance with the specified
  \a parent.
 */
QTerminationHandlerProvider::QTerminationHandlerProvider(QObject *parent)
: QObject(parent)
{
    if(!termInstance) {
        QtopiaServerApplication::qtopiaTask("TerminationHandler");
    }

    if(!termInstance) {
        qWarning("QTerminationHandlerProvider: TerminationHandler task not running.  QTerminationHandlerProvider will not function.");
    } else {
        termInstance->addHandler();
        QObject::connect(termInstance, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)), this, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)));
    }
}

/*!
  Destroys the QTerminationHandlerProvider instance.
 */
QTerminationHandlerProvider::~QTerminationHandlerProvider()
{
    if(termInstance)
        termInstance->remHandler();
}

/*!
  \fn void QTerminationHandlerProvider::applicationTerminated(const QString &name, const QString &text, const QPixmap &icon, const QString &buttonText, QtopiaServiceRequest &buttonAction)

  Emitted whenever an application that has installed a termination handler
  through the QTerminationHandler class abnormally terminates.

  \a name is set to the name or the terminating application, and \a text,
  \a icon, \a buttonText and \a buttonAction are set to those passed to the
  QTerminationHandler class by the terminating application.
 */

#include "qterminationhandlerprovider.moc"
