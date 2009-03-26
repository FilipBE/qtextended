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

#ifndef QWAITWIDGET_H
#define QWAITWIDGET_H

#include <QDialog>

#include <qtopiaglobal.h>

class QWaitWidgetPrivate;
class QHideEvent;

/*
    A label that obscures the parent display and puts an image of a wait cursor
    in the middle of the argument rect, to indicate the display is being built.
*/
class QTOPIA_EXPORT QWaitWidget : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool wasCancelled READ wasCancelled);
    Q_PROPERTY(bool cancelEnabled WRITE setCancelEnabled);
public:
    explicit QWaitWidget( QWidget *parent = 0 );
    ~QWaitWidget();

    bool wasCancelled() const;

public slots:
    void show();
    void hide();
    void setText( const QString &str );
    void setColor( const QColor &col );
    void setExpiryTime( int msec );
    void setCancelEnabled(bool enabled);
    void done(int r);

signals:
    void cancelled();

protected:
    void hideEvent( QHideEvent * );
    void keyPressEvent( QKeyEvent * );

private slots:
    void timeExpired();

private:
    void reset();

private:
    QWaitWidgetPrivate *d;
};

#endif

