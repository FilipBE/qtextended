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

#include "qmediaenginefactory.h"


/*!
    \class QMediaEngineFactory
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaEngineFactory class is a factory used by the Media Server to construct Media Engines loaded from libraries.

    The Media Server loads Qt style plugins tagged with "mediaengine". Each
    Media Engine in the Qt Extended Media system is implemented as a Qt style
    plugin. The plugin must implement the QMediaEngineFactory inteface, as it
    used by the Media Server to ensure that the plugin is able to be used in
    the server. The Media Server will call the create() function when it needs
    to construct the Engine.

    \sa QMediaEngine
*/


/*!
    Destruct a QMediaEngineFactory
*/

QMediaEngineFactory::~QMediaEngineFactory()
{
}

/*!
    \fn QMediaEngine* QMediaEngineFactory::create()

    This function is called by the Media Server to create a instance of the
    Media Engine. The factory should construct a new instance of the Media
    Engine on the heap, it will later be deleted by the Media Server. This
    function will only be called once for the lifetime of the plugin.
*/




