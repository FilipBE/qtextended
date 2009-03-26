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

#ifndef UIFACTORY_H
#define UIFACTORY_H

#include <QDialog>

class UIFactory
{
public:
    typedef QWidget* (*WidgetCreateFunction)(QWidget*,Qt::WFlags);
    static QWidget* createWidget( const QByteArray &widgetClassName,
            QWidget *parent = 0, Qt::WFlags flags = 0);
    static QDialog* createDialog( const QByteArray &dialogClassName,
            QWidget *parent = 0, Qt::WFlags flags = 0);
    static bool isAvailable( const QByteArray &widgetClassName );

    static void install( const QMetaObject *meta, WidgetCreateFunction function );
};

#define UIFACTORY_REGISTER_WIDGET(CLASSNAME) \
    static QWidget *install_##CLASSNAME( QWidget *p, Qt::WFlags f ) \
        { return new CLASSNAME(p,f); } \
    static UIFactory::WidgetCreateFunction append_##CLASSNAME() \
        { UIFactory::install( &CLASSNAME::staticMetaObject, install_##CLASSNAME ); \
          return install_##CLASSNAME; }\
    static UIFactory::WidgetCreateFunction dummy_##CLASSNAME = \
        append_##CLASSNAME();

#endif
