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
#include "pluginchooser.h"
#include "qtopiadesktopapplication.h"
#include "pluginmanager.h"

#include <qdplugin.h>
#include <private/qdplugin_p.h>
#include <qtopiadesktoplog.h>
#include <trace.h>
QD_LOG_OPTION(PluginChooser)

#include <QPushButton>
#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

// This button emits a signal with a plugin
class PluginButton : public QWidget
{
    Q_OBJECT
public:
    PluginButton( QDAppPlugin *_plugin )
	: QWidget(),
        plugin( _plugin ), text( 0 )
    {
        setFocusPolicy( Qt::NoFocus );
        hbox = new QHBoxLayout( this );
        image = new QLabel;
        text = new QLabel;
        text->hide();
        hbox->addWidget( image );
        hbox->addWidget( text );
        hbox->addStretch( 1 );
        setAutoFillBackground( true );
    }

    virtual ~PluginButton() {}
 
    void setIcon( const QIcon &icon )
    {
        TRACE(PluginChooser) << "PluginButton::setIcon" << icon;
        static int size = -1;
        if ( size == -1 )
            size = QApplication::style()->pixelMetric( QStyle::PM_ButtonIconSize );
        if ( icon.isNull() ) {
            image->hide();
        } else {
            image->setPixmap( icon.pixmap(QSize(size,size)) );
            image->show();
        }
    }

    void setText( const QString &_text )
    {
        TRACE(PluginChooser) << "PluginButton::setText" << _text;
        if ( _text.isEmpty() ) {
            text->hide();
        } else {
            text->setText( _text );
            text->show();
        }
    }

    void highlight( bool on )
    {
        QPalette pal;
        if ( on )
            pal.setBrush( QPalette::Window, pal.dark() );
        setPalette( pal );
        image->setPalette( pal );
        if ( text )
            text->setPalette( pal );
    }

signals:
    void clicked( QDAppPlugin *plugin );

private:
    void mouseReleaseEvent( QMouseEvent *e )
    {
        TRACE(PluginChooser) << "PluginButton::mouseReleaseEvent" << this << e;
        if ( rect().contains( e->pos() ) ) {
            emit clicked( plugin );
        }
    }

public:
    QDAppPlugin *plugin;

private:
    QHBoxLayout *hbox;
    QLabel *image;
    QLabel *text;
};

// ====================================================================

// This action emits a signal with a plugin
class PluginAction : public QAction
{
    Q_OBJECT
public:
    PluginAction( QDAppPlugin *_plugin, const QIcon &icon, const QString &text, QObject *parent = 0 )
	: QAction( icon, text, parent ),
        plugin( _plugin )
    {
        connect( this, SIGNAL(triggered(bool)), this, SLOT(clickme()) );
    }

    virtual ~PluginAction() {}
 
signals:
    void clicked( QDAppPlugin *plugin );

private slots:
    void clickme()
    {
        emit clicked( plugin );
    }

private:
    QDAppPlugin *plugin;
};

// ====================================================================

class PluginChooserPrivate
{
public:
    QMenu *windowMenu;
    QList<QAction*> windowActions;
    int orientation;
    QWidget *buttonHolder;
    QList<PluginButton*> buttons;
    int widgetType;
};

// ====================================================================

PluginChooser::PluginChooser( QWidget *parent )
    : QFrame( parent )
{
    d = new PluginChooserPrivate;
    d->windowMenu = 0;
    d->orientation = PluginChooser::Horizontal;
    d->buttonHolder = 0;
    d->widgetType = PluginChooser::App;

    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );

    connect( qApp, SIGNAL(pluginsChanged()), this, SLOT(pluginsChanged()) );
    connect( this, SIGNAL(showPlugin(QDAppPlugin*)), qApp, SLOT(showPlugin(QDAppPlugin*)) );
    connect( qApp, SIGNAL(showingPlugin(QDAppPlugin*)), this, SLOT(highlightPlugin(QDAppPlugin*)) );
}

PluginChooser::~PluginChooser()
{
    delete d;
}

void PluginChooser::setWindowMenu( QMenu *windowMenu )
{
    d->windowMenu = windowMenu;
}

void PluginChooser::setOrientation( PluginChooser::Orientation orientation )
{
    if ( orientation == d->orientation )
        return;
    d->orientation = orientation;
    if ( d->buttonHolder )
        pluginsChanged();
}

void PluginChooser::setWidgetType( PluginChooser::WidgetType widgetType )
{
    if ( widgetType == d->widgetType )
        return;
    d->widgetType = widgetType;
    if ( d->widgetType == PluginChooser::App ) {
        connect( this, SIGNAL(showPlugin(QDAppPlugin*)), qApp, SLOT(showPlugin(QDAppPlugin*)) );
        disconnect( this, SIGNAL(showPlugin(QDAppPlugin*)), this, SLOT(highlightPlugin(QDAppPlugin*)) );
        connect( qApp, SIGNAL(showingPlugin(QDAppPlugin*)), this, SLOT(highlightPlugin(QDAppPlugin*)) );
    } else {
        disconnect( this, SIGNAL(showPlugin(QDAppPlugin*)), qApp, SLOT(showPlugin(QDAppPlugin*)) );
        connect( this, SIGNAL(showPlugin(QDAppPlugin*)), this, SLOT(highlightPlugin(QDAppPlugin*)) );
        disconnect( qApp, SIGNAL(showingPlugin(QDAppPlugin*)), this, SLOT(highlightPlugin(QDAppPlugin*)) );
    }
    if ( d->buttonHolder )
        pluginsChanged();
}

void PluginChooser::pluginsChanged()
{
    // It's just easier to remove everything than to try and destroy it gracefully
    if ( d->buttonHolder ) {
        delete d->buttonHolder;
        d->buttons.clear();
    }
    d->buttonHolder = new QWidget;
    layout()->addWidget( d->buttonHolder );

    d->buttonHolder->setFocusPolicy( Qt::NoFocus );
    d->buttonHolder->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
    QBoxLayout *hbox;
    if ( d->orientation == PluginChooser::Horizontal )
        hbox = new QHBoxLayout( d->buttonHolder );
    else
        hbox = new QVBoxLayout( d->buttonHolder );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );

    // Clear the window menu
    if ( d->windowMenu && d->windowActions.count() ) {
        foreach ( QAction *action, d->windowActions ) {
            d->windowMenu->removeAction( action );
            delete action;
        }
        d->windowActions.clear();
    }

    QDAppPluginFilter f;
    if ( d->widgetType == PluginChooser::App )
        f.appWidget = QDPluginFilter::Set;
    if ( d->widgetType == PluginChooser::Settings )
        f.settingsWidget = QDPluginFilter::Set;
    foreach( QDAppPlugin *plugin, qdPluginManager()->appPlugins( &f ) ) {
        //qDebug() << "loading" << plugin->id();
        QIcon icon = plugin->icon();
        QString name;
        if ( d->widgetType == PluginChooser::Settings ) {
            QWidget *w = qdPluginManager()->pluginData(plugin)->settingsWidget;
            name = w->windowTitle();
        }
        if ( name.isEmpty() )
            name = plugin->displayName();
        PluginButton *button = new PluginButton( plugin );
        d->buttons << (PluginButton*)button;
        button->setIcon( icon );
        if (  d->widgetType == PluginChooser::Settings )
            button->setText( name );
        button->setToolTip( name );
        connect( button, SIGNAL(clicked(QDAppPlugin*)), this, SIGNAL(showPlugin(QDAppPlugin*)) );
        hbox->addWidget( button, 0 );
        if ( d->windowMenu ) {
            QAction *action = new PluginAction( plugin, icon, name );
            d->windowMenu->addAction( action );
            connect( action, SIGNAL(clicked(QDAppPlugin*)), this, SIGNAL(showPlugin(QDAppPlugin*)) );
            d->windowActions.append( action );
        }
    }

    hbox->addStretch( 1 );
}

void PluginChooser::highlightPlugin( QDAppPlugin *plugin )
{
    TRACE(PluginChooser) << "PluginChooser::highlightPlugin";
    foreach ( PluginButton *button, d->buttons ) {
        LOG() << "button" << button;
        if ( button->plugin == plugin ) {
            LOG() << "highlighted";
            button->highlight( true );
        } else {
            LOG() << "normal";
            button->highlight( false );
        }
    }
}

#include "pluginchooser.moc"
