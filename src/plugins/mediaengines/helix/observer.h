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

#ifndef OBSERVER_H
#define OBSERVER_H

#include <QtCore>

class Subject;

class Observer
{
public:
    virtual ~Observer() { };
    virtual void update( Subject* subject ) = 0;
};

class Subject
{
public:
    virtual ~Subject() { };

    virtual void attach( Observer* observer );
    virtual void detach( Observer* observer );
    virtual void notify();

protected:
    Subject() { };

private:
    QList<Observer*> m_observers;
};

inline void Subject::attach( Observer* observer )
{
    m_observers.append( observer );
}

inline void Subject::detach( Observer* observer )
{
    m_observers.removeAll( observer );
}

inline void Subject::notify()
{
    QList<Observer*>::iterator it = m_observers.begin();
    for (;it != m_observers.end(); ++it)
    {
        (*it)->update( this );
    }
}

#endif
