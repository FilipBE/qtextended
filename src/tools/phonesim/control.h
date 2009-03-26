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

#ifndef CONTROL_H
#define CONTROL_H

#include <HardwareManipulator>

class ControlWidget;

class Control: public HardwareManipulator
{
Q_OBJECT

public:
    Control(const QString& ruleFile, QObject *parent=0);
    virtual ~Control();

public slots:
    void handleFromData( const QString& );
    void handleToData( const QString& );
    void setPhoneNumber( const QString& );

protected:
    virtual void warning( const QString&, const QString& );

private:
    ControlWidget *widget;
    friend class ControlWidget;
};

class ControlFactory : public HardwareManipulatorFactory
{
public:
    inline virtual HardwareManipulator *create(QObject *parent)
        { return new Control(ruleFile(), parent); }
};


#endif
