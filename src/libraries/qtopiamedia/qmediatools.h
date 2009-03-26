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

#ifndef QMEDIATOOLS_H
#define QMEDIATOOLS_H

#include <QtCore>

#include <qtopiamedia/qmediacontent.h>

class QMediaContentContextPrivate;

class QTOPIAMEDIA_EXPORT QMediaContentContext : public QObject
{
    Q_OBJECT
public:
    explicit QMediaContentContext( QObject* parent = 0 );
    ~QMediaContentContext();

    void addObject( QObject* object );
    void removeObject( QObject* object );

    QMediaContent* content() const;

signals:
    void contentChanged( QMediaContent* content );

public slots:
    void setMediaContent( QMediaContent* content );

private:
    QMediaContent *m_content;
    QMediaContentContextPrivate *m_d;
};

class QMediaControlNotifierPrivate;

class QTOPIAMEDIA_EXPORT QMediaControlNotifier : public QObject
{
    Q_OBJECT
public:
    explicit QMediaControlNotifier( const QString& control, QObject* parent = 0 );
    ~QMediaControlNotifier();

    QMediaContent* content() const;

signals:
    void valid();
    void invalid();

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void evaluate();

private:
    QString m_control;
    QMediaContent *m_content;
    QMediaControlNotifierPrivate *m_d;
};

#endif
