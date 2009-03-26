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

#ifndef PROJECTOBJECT_H
#define PROJECTOBJECT_H

class ProjectItem
{
public:
    enum { Item, Object, ObjectRef, Property } Type;
    Type type() const;

    QList<QByteArray> notes() const; // Added by parser in debugging mode to
                                     // show why things are the way they are
};

class ProjectObject : public ProjectItem
{
public:
    QList<ProjectItem *> items() const;
}:

class ProjectObjectRef : public ProjectItem
{
public:
    ProjectObject *object() const;
}:

class ProjectProperty : public ProjectItem
{
public:
    QByteArray name() const;
    QList<QString> values() const;
};

#endif
