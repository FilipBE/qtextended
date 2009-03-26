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

#include <QApplication>
#include <QTest>
#include <qbenchmark.h>
#include <qtopiamessagehandler_p.h>

#include <errno.h>
#include <string.h>

//TESTED_CLASS=QtopiaMessageHandler
//TESTED_FILES=src/libraries/qtopia/qtopiamessagehandler_p.h

class tst_QtopiaMessageHandler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void output();
    void output_data();

    void snprintf();
    void snprintf_data();

private:
    QList<QByteArray> strings();
};

QTEST_MAIN(tst_QtopiaMessageHandler)
#include "tst_qtopiamessagehandler.moc"

struct StreamRedirector
{
    StreamRedirector(FILE*&, const char*);
    ~StreamRedirector();

private:
    FILE*& m_file;
    FILE*  m_old;
};

struct QtMessageHandlerFreeZone
{
    QtMessageHandlerFreeZone();
    ~QtMessageHandlerFreeZone();

private:
    QtMsgHandler m_old;
};

void tst_QtopiaMessageHandler::initTestCase()
{
}

void tst_QtopiaMessageHandler::init()
{
    // Force handleMessage to use a no-op log format
    QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%s");
    QtopiaMessageHandler::reload();
}

void tst_QtopiaMessageHandler::cleanup()
{
}

enum OutputType {
    FPRINTF,
    QDEBUG,
    QTOPIAMESSAGEHANDLER,
    QTOPIAMESSAGEHANDLER_WITHPID,
    QTOPIAMESSAGEHANDLER_WITHNAME,
    QTOPIAMESSAGEHANDLER_WITHTIME,
    QTOPIAMESSAGEHANDLER_WITHALL
};

Q_DECLARE_METATYPE(OutputType)

enum SnprintfType {
    LIBC,
    QTOPIA
};

Q_DECLARE_METATYPE(SnprintfType)

void tst_QtopiaMessageHandler::output()
{
    QFETCH(QByteArray, message);
    QFETCH(OutputType, mode);
    char const* cstr = message.constData();

    StreamRedirector redir(stderr, "/dev/null");
    QtMessageHandlerFreeZone redirHandler;
    if (mode == QTOPIAMESSAGEHANDLER) {
        QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%s");
        QtopiaMessageHandler::reload();
        QBENCHMARK {
            QtopiaMessageHandler::handleMessage(QtDebugMsg, cstr);
        }
    }
    else if (mode == QTOPIAMESSAGEHANDLER_WITHALL) {
        QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%t %p %n: %s");
        QtopiaMessageHandler::reload();
        QBENCHMARK {
            QtopiaMessageHandler::handleMessage(QtDebugMsg, cstr);
        }
    }
    else if (mode == QTOPIAMESSAGEHANDLER_WITHPID) {
        QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%p: %s");
        QtopiaMessageHandler::reload();
        QBENCHMARK {
            QtopiaMessageHandler::handleMessage(QtDebugMsg, cstr);
        }
    }
    else if (mode == QTOPIAMESSAGEHANDLER_WITHNAME) {
        QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%n: %s");
        QtopiaMessageHandler::reload();
        QBENCHMARK {
            QtopiaMessageHandler::handleMessage(QtDebugMsg, cstr);
        }
    }
    else if (mode == QTOPIAMESSAGEHANDLER_WITHTIME) {
        QSettings("Trolltech", "Log2").setValue("MessageHandler/Format", "%t: %s");
        QtopiaMessageHandler::reload();
        QBENCHMARK {
            QtopiaMessageHandler::handleMessage(QtDebugMsg, cstr);
        }
    }
    else if (mode == QDEBUG) {
        QBENCHMARK {
            qDebug("%s", cstr);
        }
    }
    else if (mode == FPRINTF) {
        QBENCHMARK {
            ::fprintf(stderr, "%s\n", cstr);
            ::fflush(stderr);
        }
    }
}

void tst_QtopiaMessageHandler::output_data()
{
    QTest::addColumn<QByteArray>("message");
    QTest::addColumn<OutputType>("mode");

    foreach (QByteArray const& ba, strings()) {
        QTest::newRow(qPrintable(QString("fprintf--%1").arg(ba.length())))
            << ba
            << FPRINTF
        ;
        QTest::newRow(qPrintable(QString("qdebug--%1").arg(ba.length())))
            << ba
            << QDEBUG
        ;
        QTest::newRow(qPrintable(QString("qtopiamessagehandler--%1").arg(ba.length())))
            << ba
            << QTOPIAMESSAGEHANDLER
        ;
        QTest::newRow(qPrintable(QString("qtopiamessagehandler_all--%1").arg(ba.length())))
            << ba
            << QTOPIAMESSAGEHANDLER_WITHALL
        ;
        QTest::newRow(qPrintable(QString("qtopiamessagehandler_name--%1").arg(ba.length())))
            << ba
            << QTOPIAMESSAGEHANDLER_WITHNAME
        ;
        QTest::newRow(qPrintable(QString("qtopiamessagehandler_pid--%1").arg(ba.length())))
            << ba
            << QTOPIAMESSAGEHANDLER_WITHPID
        ;
        QTest::newRow(qPrintable(QString("qtopiamessagehandler_time--%1").arg(ba.length())))
            << ba
            << QTOPIAMESSAGEHANDLER_WITHTIME
        ;
    }
}

void tst_QtopiaMessageHandler::snprintf()
{
    QFETCH(QByteArray, message);
    QFETCH(QByteArray, format);
    QFETCH(QByteArray, expected);
    QFETCH(SnprintfType, mode);
    char const* cstr = message.constData();
    char const* fmt  = format.constData();

    char buf[8192];
    buf[8191] = 0;

    QBENCHMARK {
        if (mode == LIBC)
            ::snprintf(buf, 8191, fmt, cstr);
        else if (mode == QTOPIA)
            QtopiaMessageHandler::snprintf(buf, 8191, fmt, cstr);
    }

    QCOMPARE(QByteArray(buf), expected);
}

void tst_QtopiaMessageHandler::snprintf_data()
{
    QTest::addColumn<QByteArray>("message");
    QTest::addColumn<QByteArray>("format");
    QTest::addColumn<QByteArray>("expected");
    QTest::addColumn<SnprintfType>("mode");

    foreach (QByteArray const& ba, strings()) {
        QTest::newRow(qPrintable(QString("libc %s--%1").arg(ba.length())))
            << ba
            << QByteArray("%s")
            << ba
            << LIBC
        ;
        QTest::newRow(qPrintable(QString("qtopia %s--%1").arg(ba.length())))
            << ba
            << QByteArray("%s")
            << ba
            << QTOPIA
        ;
        QTest::newRow(qPrintable(QString("qtopia %n %p: %s--%1").arg(ba.length())))
            << ba
            << QByteArray("%n %p: %s")
            << (QString("%1 %2: ")
                .arg("tst_qtopiamessagehandler")
                .arg(::getpid())
                .toLocal8Bit()
                + ba)
            << QTOPIA
        ;
    }
}

QList<QByteArray> tst_QtopiaMessageHandler::strings()
{
    QList<QByteArray> ret;

    QByteArray longstring;
    for (int i = 0; i <= 256; ++i) {
        if (!(i % 16)) ret << longstring;
        longstring += "FA";
    }

    return ret;
}

/*************************************************************************************************/

StreamRedirector::StreamRedirector(FILE*& f, const char* path)
    :     m_file(f)
        , m_old(f)
{
    f = fopen(path, "a");
    if (!f)
        qFatal("fopen: %s", strerror(errno));
}

StreamRedirector::~StreamRedirector()
{
    fclose(m_file);
    m_file = m_old;
}

QtMessageHandlerFreeZone::QtMessageHandlerFreeZone()
    : m_old(qInstallMsgHandler(0))
{}

QtMessageHandlerFreeZone::~QtMessageHandlerFreeZone()
{ qInstallMsgHandler(m_old); }

