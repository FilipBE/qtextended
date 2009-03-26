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
#include <qstring.h>
#include <Carbon/Carbon.h>

#include "qtsingleapplication.h"

CFStringRef qstring2cfstring(const QString &str)
{
    return CFStringCreateWithCharacters(0, (UniChar *)str.unicode(), str.length());
}

QString cfstring2qstring(CFStringRef str)
{
    if(!str)
	return QString();

    CFIndex length = CFStringGetLength(str);
    if(const UniChar *chars = CFStringGetCharactersPtr(str))
	return QString((QChar *)chars, length);
    UniChar *buffer = (UniChar*)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
    QString ret((QChar *)buffer, length);
    free(buffer);
    return ret;
}

class QtSingletonSysPrivate
{
public:
    inline QtSingletonSysPrivate() { port = 0; }
    CFMessagePortRef port;
};

class QtSingletonPrivate
{
};

bool QtSingleApplication::isRunning() const
{
    return !!sysd->port;
}

void QtSingleApplication::sysInit()
{
    sysd = new QtSingletonSysPrivate;
}

void QtSingleApplication::sysCleanup()
{
    delete sysd;
}

CFDataRef MyCallBack(CFMessagePortRef, SInt32, CFDataRef data, void *info)
{
    CFIndex index = CFDataGetLength(data);
    const UInt8 *p = CFDataGetBytePtr(data);
    QByteArray ba(index, 0);
    for (int i = 0; i < index; ++i)
        ba[i] = p[i];
    QString message(ba);
    emit static_cast<QtSingleApplication *>(info)->messageReceived(message);
    return 0;
}

void QtSingleApplication::initialize(bool activate)
{
    CFStringRef cfstr = qstring2cfstring(id());
    CFMessagePortContext context;
    context.version = 0;
    context.info = this;
    context.retain = 0;
    context.release = 0;
    context.copyDescription = 0;
    sysd->port = CFMessagePortCreateLocal(kCFAllocatorDefault, cfstr, MyCallBack, &context, 0);
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    if (sysd->port && runloop) {
        CFRunLoopSourceRef source = CFMessagePortCreateRunLoopSource(0, sysd->port, 0);
        if (source)
            CFRunLoopAddSource(runloop, source, kCFRunLoopCommonModes);
        CFRelease(source);
    }
    CFRelease(cfstr);
    if (activate) {
	connect(this, SIGNAL(messageReceived(QString)),
		this, SLOT(activateMainWidget()));
    }
}

bool QtSingleApplication::sendMessage(const QString &message, int timeout)
{
    CFStringRef cfstr = qstring2cfstring(id());
    CFMessagePortRef myport = CFMessagePortCreateRemote(kCFAllocatorDefault, cfstr);
    CFRelease(cfstr);
    if (!myport)
        return false;
    static SInt32 msgid = 0;
    const QByteArray latin1Message = message.toLatin1();
    CFDataRef data = CFDataCreate(0, (UInt8*)latin1Message.constData(), latin1Message.length());
    CFDataRef reply = 0;
    SInt32 result = CFMessagePortSendRequest(myport, ++msgid, data, timeout / 1000,
                                             timeout / 1000, 0, &reply);
    CFRelease(data);
    if (reply)
        CFRelease(reply);
    return result == kCFMessagePortSuccess;

}
