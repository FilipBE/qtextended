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
#include "statusbar.h"

#include <qdebug.h>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    ClickableLabel() : QLabel() {}
    virtual ~ClickableLabel() {}

signals:
    void clicked();

private:
    void mousePressEvent( QMouseEvent *e )
    {
        if ( e->button() == Qt::LeftButton )
            emit clicked();
        QLabel::mousePressEvent( e );
    }
};

// ====================================================================

StatusBar::StatusBar( QWidget *parent )
    : QFrame( parent )
{
    setFrameShadow( Raised );
    setFrameShape( StyledPanel );
    layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    statusLabel = new ClickableLabel;
    connect( statusLabel, SIGNAL(clicked()), this, SIGNAL(clicked()) );
    addWidget( statusLabel, 1 );
    statusClearer = new QTimer( this );
    statusClearer->setSingleShot( true );
    connect( statusClearer, SIGNAL(timeout()), statusLabel, SLOT(clear()) );
}

StatusBar::~StatusBar()
{
}

void StatusBar::addWidget( QWidget *widget, int stretch, bool /*permanent*/ )
{
    QFrame *holder = new QFrame;
    holder->setFrameShadow( Sunken );
    holder->setFrameShape( StyledPanel );
    QHBoxLayout *hbox = new QHBoxLayout( holder );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );
    hbox->addWidget( widget );
    widgets[widget] = holder;
    layout->addWidget( holder, stretch );
}

void StatusBar::removeWidget( QWidget *widget )
{
    QFrame *holder = widgets[widget];
    widgets.remove( widget );
    layout->removeWidget( holder );
    widget->setParent( 0 );
    delete holder;
}

void StatusBar::showMessage( const QString &message, int timeout )
{
    if ( statusClearer->isActive() )
        statusClearer->stop();
    statusLabel->setText( message );
    if ( timeout > 0 )
        statusClearer->start( timeout );
}

#include "statusbar.moc"
