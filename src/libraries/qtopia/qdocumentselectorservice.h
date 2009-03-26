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
#ifndef QDOCUMENTSELECTORSERVICE_H
#define QDOCUMENTSELECTORSERVICE_H
#include <QDocumentSelector>

class QIODevice;

class QDocumentSelectorServicePrivate;

class QTOPIA_EXPORT QDocumentSelectorService : public QObject
{
    Q_OBJECT
public:
    explicit QDocumentSelectorService( QObject *parent = 0 );

    virtual ~QDocumentSelectorService();

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    QContentSortCriteria sortCriteria() const;
    void setSortCriteria( const QContentSortCriteria &sort );

    QContent selectedDocument() const;

    QIODevice *selectedDocumentData();

    bool newDocument( const QString &name, const QString &type, QWidget *widget = 0 );
    bool newDocument( const QString &name, const QStringList &types, QWidget *widget = 0 );
    bool openDocument( QWidget *widget = 0 );
    bool saveDocument( QWidget *widget = 0 );

public slots:
    void close();

signals:
    void documentOpened( const QContent &document, QIODevice *data );

private:
    QDocumentSelectorServicePrivate *d;
};


#endif
