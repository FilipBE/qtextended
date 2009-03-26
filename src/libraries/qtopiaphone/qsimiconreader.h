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

#ifndef QSIMICONREADER_H
#define QSIMICONREADER_H

#include <qtopiaglobal.h>
#include <QImage>

class QSimIconReaderPrivate;

class QTOPIAPHONE_EXPORT QSimIconReader : public QObject
{
    Q_OBJECT
public:
    explicit QSimIconReader( const QString& service = QString(), QObject *parent = 0 );
    ~QSimIconReader();

    bool haveIcon( int iconId ) const;
    QImage icon( int iconId ) const;
    void requestIcon( int iconId );

signals:
    void iconAvailable( int iconId );
    void iconNotFound( int iconId );

private slots:
    void indexError();
    void indexFileInfo( int numRecords, int recordSize );
    void indexRead( const QByteArray& data, int recno );
    void emitPendingAvailable();
    void emitPendingNotFound();
    void processPendingOnIndex();
    void iconFetched( int iconId, const QImage& image );
    void iconFetchFailed( int iconId );

private:
    QSimIconReaderPrivate *d;
};

#endif
