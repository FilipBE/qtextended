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

#ifndef SIMICONS_H
#define SIMICONS_H

#include <QSimIconReader>
#include <QIcon>
#include <QMap>

class QWaitWidget;

class SimIcons : public QObject
{
    Q_OBJECT
public:
    SimIcons( QSimIconReader *reader, QWidget *parent );
    ~SimIcons();

    QIcon icon( int iconId );
    QString iconFile( int iconId );

public slots:
    void needIcon( int iconId );
    void needIconInFile( int iconId );
    void requestIcons();

signals:
    void iconsReady();

private slots:
    void iconDone( int iconId );

private:
    QSimIconReader *reader;
    QWidget *parent;
    QWaitWidget *waitWidget;
    QList<int> pendingIcons;
    QList<int> loadedIcons;
    QList<int> fileIcons;
    QMap<int, QString> files;

    void copyIconsToFiles();
};

#endif

