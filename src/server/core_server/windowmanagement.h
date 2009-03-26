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

#ifndef WINDOWMANAGEMENT_H
#define WINDOWMANAGEMENT_H

#include <qobject.h>
#include <qwindowdefs.h>

class QRect;
class QString;
class QSize;
class QWidget;
class WindowManagementPrivate;

class WindowManagement : public QObject
{
    Q_OBJECT

  public:
    WindowManagement(QObject *parent = 0);
    virtual ~WindowManagement();

    enum DockArea { Top, Bottom, Left, Right };
    static void protectWindow(QWidget *);
    static void dockWindow(QWidget *, DockArea, int screen=-1);
    static void dockWindow(QWidget *, DockArea, const QSize &, int screen=-1);
    static void showDockedWindow(QWidget *);
    static void hideDockedWindow(QWidget *);
    static void setLowestWindow(QWidget *);
    static QString activeAppName();
    static bool supportsSoftMenus(WId winId);
    static void closeWindow(WId winId);

  signals:
    void windowActive(const QString &, const QRect &, WId winId);
    void windowCaption(const QString &);

  private:
    WindowManagementPrivate * d;
};

#endif
