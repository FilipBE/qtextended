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

#ifndef QSIMICONREADER_P_H
#define QSIMICONREADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qsimiconreader.h>
#include <qbinarysimfile.h>

class QSimMonoIconReader : public QObject
{
    Q_OBJECT
public:
    QSimMonoIconReader( const QString& service, int iconId,
                        const QString& fileid, int offset,
                        int length, QObject *parent = 0 );
    ~QSimMonoIconReader();

signals:
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private slots:
    void error();
    void readDone( const QByteArray& data );

private:
    int iconId;
    int length;
    QBinarySimFile *file;
    QByteArray contents;
    bool errorReported;
};

class QSimColorIconReader : public QObject
{
    Q_OBJECT
public:
    QSimColorIconReader( const QString& service, int iconId,
                         const QString& fileid, int offset,
                         int length, QObject *parent = 0 );
    ~QSimColorIconReader();

signals:
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private slots:
    void error();
    void readDone( const QByteArray& data );

private:
    int iconId;
    int length;
    int clutLength;
    QBinarySimFile *file;
    QByteArray contents;
    QByteArray clut;
    bool errorReported;

    void read( int offset, int length );
    static int fetchPixel( const char *data, int start, int size );
};

#endif
