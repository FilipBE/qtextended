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

#ifndef SCANNERTHREAD_H
#define SCANNERTHREAD_H

#include <QThread>
#include <QFileInfo>

class PackageScanner;

class ScannerThread : public QThread
{
    Q_OBJECT
public:
    ScannerThread( PackageScanner *parent ) : QThread() { mParent = parent; }
    ~ScannerThread() {}
    void run();
private:
    void handleFile( const QString &, const QFileInfo & );
    PackageScanner *mParent;

    // current idea of how many files we have to scan thru
    static int numberOfFiles;
};

#endif
