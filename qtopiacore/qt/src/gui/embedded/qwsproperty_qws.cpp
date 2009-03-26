/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qwsproperty_qws.h"

#ifndef QT_NO_QWS_PROPERTIES
#include "qwscommand_qws_p.h"
#include "qwindowsystem_qws.h"
#include "qhash.h"
#include "qalgorithms.h"
#include "qbytearray.h"

#include <stdio.h>

QT_BEGIN_NAMESPACE

class QWSPropertyManager::Data {
public:
    QByteArray find(int winId, int property)
    {
        return properties.value(winId).value(property);
    }

    typedef QHash<int, QHash<int, QByteArray> > PropertyHash;
    PropertyHash properties;
};

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

QWSPropertyManager::QWSPropertyManager()
{
    d = new Data;
}

QWSPropertyManager::~QWSPropertyManager()
{
    delete d;
}

bool QWSPropertyManager::setProperty(int winId, int property, int mode, const char *data, int len)
{
    QHash<int, QByteArray> props = d->properties.value(winId);
    QHash<int, QByteArray>::iterator it = props.find(property);
    if (it == props.end())
        return false;

    switch (mode) {
    case PropReplace:
        d->properties[winId][property] = QByteArray(data, len);
        break;
    case PropAppend:
        d->properties[winId][property].append(data);
        break;
    case PropPrepend:
        d->properties[winId][property].prepend(data);
        break;
    }
    return true;
}

bool QWSPropertyManager::hasProperty(int winId, int property)
{
    return d->properties.value(winId).contains(property);
}

bool QWSPropertyManager::removeProperty(int winId, int property)
{
    QWSPropertyManager::Data::PropertyHash::iterator it = d->properties.find(winId);
    if (it == d->properties.end())
        return false;
    return d->properties[winId].remove( property );
}

bool QWSPropertyManager::addProperty(int winId, int property)
{
    if( !d->properties[winId].contains(property) )
	d->properties[winId][property] = QByteArray(); // only add if it doesn't exist
    return true;
}

bool QWSPropertyManager::getProperty(int winId, int property, const char *&data, int &len)
{
    QHash<int, QByteArray> props = d->properties.value(winId);
    QHash<int, QByteArray>::iterator it = props.find(property);
    if (it == props.end()) {
        data = 0;
        len = -1;
        return false;
    }
    data = it.value().constData();
    len = it.value().length();

    return true;
}

bool QWSPropertyManager::removeProperties(int winId)
{
    return d->properties.remove(winId);
}

QT_END_NAMESPACE

#endif //QT_NO_QWS_PROPERTIES
