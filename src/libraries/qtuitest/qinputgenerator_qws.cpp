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

#include "qinputgenerator_p.h"

#include <QCopChannel>
#include <QDebug>
#include <QKeySequence>
#include <QScreen>
#include <QWSServer>
#include <QWidget>

#include "qtuitestnamespace.h"

#ifdef QTOPIA_TARGET
# include <qtopialog.h>
# define QINPUTGENERATOR_DEBUG() qLog(QtUitest)
#else
# define QINPUTGENERATOR_DEBUG() if (1); else qDebug() << "QInputGenerator:"
#endif

#define QINPUTGENERATOR_QCOP          "QtUiTest/QInputGenerator"
#define QINPUTGENERATOR_QCOP_KEYEVENT "keyEvent(int,int,int,bool)"
#define QINPUTGENERATOR_QCOP_VERSION  QDataStream::Qt_4_0

struct QInputGeneratorPrivate
{
    QInputGeneratorPrivate();
    QInputGenerator* q;

    enum KeyEventType   { KeyPress,   KeyRelease,   KeyClick   };
    enum MouseEventType { MousePress, MouseRelease, MouseClick };

    void keyEvent  (Qt::Key, Qt::KeyboardModifiers, KeyEventType, bool = false);
    void mouseEvent(QPoint const&, Qt::MouseButtons, MouseEventType);
};

struct QInputGeneratorService : public QCopChannel
{
    QInputGeneratorService(QInputGeneratorPrivate*,QObject* =0);

    virtual void receive(QString const&,QByteArray const&);

    QInputGeneratorPrivate* d;
};

QInputGeneratorService::QInputGeneratorService(QInputGeneratorPrivate* _d, QObject* parent)
    :   QCopChannel(QINPUTGENERATOR_QCOP, parent),
        d(_d)
{
    Q_ASSERT(qApp->type() == qApp->GuiServer);
}

void QInputGeneratorService::receive(QString const& message, QByteArray const& data)
{
    if (message == QLatin1String(QINPUTGENERATOR_QCOP_KEYEVENT)) {
        int key         = 0;
        int mod         = 0;
        int type        = 0;
        bool autoRepeat = false;

        QByteArray copy(data);
        QDataStream ds(&copy, QIODevice::ReadOnly);
        ds.setVersion(QINPUTGENERATOR_QCOP_VERSION);
        ds >> key >> mod >> type >> autoRepeat;

        if (ds.status() != QDataStream::Ok) {
            qWarning() <<   "QtUitest: error deserializing " QINPUTGENERATOR_QCOP_KEYEVENT
                            " message, a key event might be lost.";
        } else {
            d->keyEvent(Qt::Key(key), Qt::KeyboardModifiers(mod), static_cast<QInputGeneratorPrivate::KeyEventType>(type), autoRepeat);
        }
    } else {
        qWarning() << "QtUitest: QInputGenerator received unknown message" << message;
    }
}

QInputGeneratorPrivate::QInputGeneratorPrivate()
{}

QInputGenerator::QInputGenerator(QObject* parent)
    : QObject(parent),
      d(new QInputGeneratorPrivate)
{
    d->q = this;
    if (qApp->type() == qApp->GuiServer) {
        new QInputGeneratorService(d, this);
    }
}

QInputGenerator::~QInputGenerator()
{ d->q = 0; delete d; d = 0; }

void QInputGeneratorPrivate::keyEvent(Qt::Key key, Qt::KeyboardModifiers mod, KeyEventType type, bool autoRepeat)
{
    if (qApp->type() != qApp->GuiServer) {
        QByteArray data;
        {
            QDataStream ds(&data,QIODevice::WriteOnly);
            ds.setVersion(QINPUTGENERATOR_QCOP_VERSION);
            ds << int(key) << int(mod) << int(type) << autoRepeat;
            if (ds.status() != QDataStream::Ok) {
                qWarning() <<   "QtUitest: error serializing " QINPUTGENERATOR_QCOP_KEYEVENT
                                " message, a key event might be lost.";
            }
        }
        if (!QCopChannel::send(QINPUTGENERATOR_QCOP, QINPUTGENERATOR_QCOP_KEYEVENT, data)) {
            qWarning() <<   "QtUitest: error sending " QINPUTGENERATOR_QCOP_KEYEVENT
                            " message, a key event might be lost.";
        }
        return;
    }

    QINPUTGENERATOR_DEBUG() << "about to simulate key"
        << ((type==KeyPress) ? "press" : ((type==KeyRelease) ? "release" : "click"))
        << QKeySequence(key|mod).toString() << "autorepeat" << QString("%1").arg(autoRepeat);

    ushort unicode = 0;
    if (key <= 0xff) {
        QChar ch(key);
        // FIXME this should be less of a hack.  The case should not implicitly depend on mod.
        if (mod & Qt::ShiftModifier) ch = ch.toUpper();
        else                         ch = ch.toLower();
        unicode = ch.unicode();
    }

    /*
        The screensaver may consume key events.
        There is no way to query the server to determine if the next key event will be consumed,
        or even if the screensaver is currently active.  All we can do is force the screensaver
        (if any) to wake up before every key event to ensure none of them are consumed.
    */
    if (!QMetaObject::invokeMethod(QWSServer::instance(), "_q_screenSaverWake"))
        Q_ASSERT(0);

    QWSServer::processKeyEvent(unicode, key, mod, (type != KeyRelease), autoRepeat);

    if (type == KeyClick) {
        // Key press/release should not occur in same millisecond
        QtUiTest::wait(10);
        QWSServer::processKeyEvent(unicode, key, mod, false, autoRepeat);
    }

    QINPUTGENERATOR_DEBUG() << "simulated key"
        << ((type==KeyPress) ? "press" : ((type==KeyRelease) ? "release" : "click"))
        << QKeySequence(key|mod).toString() << "autorepeat" << QString("%1").arg(autoRepeat);
}

void QInputGenerator::keyPress(Qt::Key key, Qt::KeyboardModifiers mod, bool autoRepeat)
{ d->keyEvent(key, mod, d->KeyPress, autoRepeat); }

void QInputGenerator::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod)
{ d->keyEvent(key, mod, d->KeyRelease); }

void QInputGenerator::keyClick(Qt::Key key, Qt::KeyboardModifiers mod)
{ d->keyEvent(key, mod, d->KeyClick); }

void QInputGeneratorPrivate::mouseEvent(QPoint const& local, Qt::MouseButtons state, MouseEventType type)
{
    QPoint pos(q->mapFromActiveWindow(local));

    QINPUTGENERATOR_DEBUG() << "about to simulate mouse"
        << ((type==MousePress) ? "press" : ((type==MouseRelease) ? "release" : "click"));

    Q_ASSERT(pos.x() >= 0 && pos.x() < QScreen::instance()->width()
        && pos.y() >= 0 && pos.y() < QScreen::instance()->height());

    // When a mouse click occurs while an input method is currently active,
    // and the click goes to a non-qpe window, there's a fair chance it will
    // cause the current IM to change.
    // We'd better wait for it, because it can cause widgets to be resized
    // and hence screw up subsequent clicks.
    struct InputMethods {
        static QWidget* widget()
        {
            static QWidget* inputMethods = 0;
            if (!inputMethods) {
                QWidgetList wl(QApplication::topLevelWidgets());
                foreach (QWidget *w, wl) {
                    inputMethods = w->findChild<QWidget*>("InputMethods");
                    if (inputMethods) break;
                }
            }
            return inputMethods;
        }

        static QString current()
        {
            QString ret;
            if (InputMethods::widget()) {
                QMetaObject::invokeMethod(InputMethods::widget(), "currentShown",
                        Qt::DirectConnection, Q_RETURN_ARG(QString, ret));
            }
            return ret;

        }
    };

    QWSWindow *win = QWSServer::instance()->windowAt(pos);
    QString client_before_click = (win ? win->client()->identity() : QString());
    QString im_before_click = InputMethods::current();

    QWSServer::sendMouseEvent(pos, (MouseRelease == type) ? Qt::MouseButtons(0) : state, 0);

    if (type == MouseClick) {
        // This wait is to avoid the mouse press and release occurring in
        // the same millisecond, which surely won't happen with real
        // hardware and wouldn't be unlikely to confuse some apps.
        QtUiTest::wait(10);
        QWSServer::sendMouseEvent(pos, 0, 0);
    }

    if (type == MouseClick && client_before_click != "qpe"
            && !im_before_click.isEmpty()) {
        for (int i = 0;
             i < 500 && InputMethods::current() == im_before_click;
             i += 50, QtUiTest::wait(50)) {}
    }

    QINPUTGENERATOR_DEBUG() << "simulated mouse"
        << ((type==MousePress) ? "press" : ((type==MouseRelease) ? "release" : "click"));
}

void QInputGenerator::mousePress(QPoint const& pos, Qt::MouseButtons state)
{ d->mouseEvent(pos, state, d->MousePress); }

void QInputGenerator::mouseRelease(QPoint const& pos, Qt::MouseButtons state)
{ d->mouseEvent(pos, state, d->MouseRelease); }

void QInputGenerator::mouseClick(QPoint const& pos, Qt::MouseButtons state)
{ d->mouseEvent(pos, state, d->MouseClick); }

