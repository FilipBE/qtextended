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
#include <qdplugin.h>
#include <center.h>
#include <desktopsettings.h>
#include <qcopenvelope_qd.h>
#include <qcopchannel_qd.h>

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QTabWidget>
#include <QLineEdit>
#include <QBuffer>
#include <QRegExp>
#include <QScrollArea>
#include <qdebug.h>
#include <QCheckBox>
#include <QComboBox>

class PluginsPageApp;

struct PropertyData
{
    QString id;
    QString prop;
    const char *property;
    QWidget *editor;
    QLabel *help;
};

class PluginsPage : public QWidget
{
    Q_OBJECT
public:
    PluginsPage( PluginsPageApp *plugin, QWidget *parent = 0 );
    ~PluginsPage();

public slots:
    void applySettings();
    void revertSettings();
    void propertyMessage( const QString &message, const QByteArray &data );

private:
    PluginsPageApp *plugin;
    QTabWidget *tabs;
    QMap<QString,QWidget*> tabMap;
    QList<PropertyData*> propertyList;
};

// ====================================================================

class PluginsPageApp : public QDAppPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(PluginsPageApp,QDAppPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.app.pluginspage"; }
    QString displayName() { return tr("Plugin Settings"); }

    // QDAppPlugin
    QIcon icon() { return QPixmap(":image/appicon"); }
    QWidget *initApp() { return 0; }
    QWidget *initSettings() { return new PluginsPage( this ); }
};

QD_REGISTER_PLUGIN(PluginsPageApp)

// ====================================================================

PluginsPage::PluginsPage( PluginsPageApp *_plugin, QWidget *parent )
    : QWidget( parent ),
    plugin( _plugin )
{
    TRACE(PM) << "PluginsPage::PluginsPage";
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );
    tabs = new QTabWidget;
    vbox->addWidget( tabs );

    //qDebug() << "reading in the plugin settings";
    DesktopSettings settings;
    foreach ( const QString &id, settings.value("/settings/loadedPlugins").toStringList() ) {
        settings.beginGroup( id + "/properties" );
        QStringList properties;
        foreach ( const QString &prop, settings.childGroups() ) {
            settings.beginGroup( prop );
            if ( !settings.value("editor").toString().isEmpty() )
                properties << prop;
            settings.endGroup();
        }
        QStringList order = settings.value("order").toStringList();
        if ( order.count() ) {
            QStringList unordered = properties;
            properties = QStringList();
            foreach ( const QString &prop, order ) {
                if ( unordered.contains( prop ) ) {
                    properties << prop;
                    unordered.removeAll( prop );
                }
            }
            if ( unordered.count() )
                properties << unordered;
        }
        foreach ( const QString &prop, properties ) {
            //qDebug() << "id" << id << "property" << prop;
            PropertyData *data = new PropertyData;
            data->id = id;
            data->prop = prop;
            if ( !tabMap.contains(id) ) {
                QWidget *w = new QWidget;
                tabMap[id] = w;
                (void)new QGridLayout( w );
                QScrollArea *sa = new QScrollArea;
                sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                sa->setWidget( w );
                sa->setWidgetResizable( true );
                tabs->addTab( sa, plugin->centerInterface()->getPlugin(id)->displayName() );
            }
            settings.beginGroup( prop );
            QString labelText = settings.value("label").toString();
            QString helpText = settings.value("helptext").toString();
            QString value = settings.value("value").toString();
#ifdef USE_HELPER
            QStringList values = settings.value("values").toStringList();
#endif
            QString edtype = settings.value("editor").toString();
            QGridLayout *l = (QGridLayout*)tabMap[id]->layout();
            QLabel *help = 0;
            if ( !helpText.isEmpty() ) {
                help = new QLabel( helpText );
                help->setWordWrap( true );
            }
            QLabel *label = new QLabel( labelText );
            QWidget *editor = 0;
#ifdef USE_HELPER
            QWidget *helper = 0;
#endif
            const char *property = 0;
            if ( edtype == QLatin1String("QLineEdit") ) {
                editor = new QLineEdit;
                property = "text";
            } else if ( edtype == QLatin1String("QCheckBox") ) {
                editor = new QCheckBox;
                property = "checked";
            } else {
                // TODO load a custom editor
            }
            if ( !editor ) {
                WARNING() << "Could not construct widget:" << edtype << "for item" << QString("%1/properties/%2").arg(id).arg(prop);
                editor = new QLineEdit;
                property = "text";
            }
#ifdef USE_HELPER
            if ( values.count() ) {
                helper = new QPushButton;
                connect( helper, SIGNAL(clicked()), new Foo(), SLOT(showList()) );
            } else {
                helper = new QWidget;
            }
#endif
            editor->setProperty( property, value );
            data->property = property;
            data->editor = editor;
            data->help = help;
            propertyList << data;
            int lastRow = l->rowCount() - 1;
            int row = lastRow;
            if ( row != 0 ) {
                // add some space between items
                l->setRowMinimumHeight( row, 12 );
                row++;
            }
            if ( help ) {
                l->addWidget( help, row, 0, 1, 2 );
                row++;
            }
            l->addWidget( label, row, 0 );
            l->addWidget( editor, row, 1 );
#ifdef USE_HELPER
            l->addWidget( helper, row, 0 );
#endif
            l->setRowStretch( lastRow, 0 );
            l->setRowStretch( row + 1, 1 );
            settings.endGroup();
        }
        settings.endGroup();
    }

    QCopChannel *chan = new QCopChannel( "QD/Properties", this );
    connect( chan, SIGNAL(received(QString,QByteArray)),
            this, SLOT(propertyMessage(QString,QByteArray)) );
}

PluginsPage::~PluginsPage()
{
}

void PluginsPage::applySettings()
{
    //qDebug() << "PluginsPage::applySettings";
    DesktopSettings settings;
    foreach ( PropertyData *data, propertyList ) {
        settings.beginGroup( data->id + "/properties/" + data->prop );
        QVariant newValue = data->editor->property(data->property);
        QVariant oldValue = settings.value("value");
        if ( newValue != oldValue ) {
            settings.setValue( "value", data->editor->property(data->property) );
            settings.sync();
            QCopEnvelope e( QString("QD/Plugin/%1").arg(data->id), "propertyChanged(QString)" );
            e << data->prop;
        }
        settings.endGroup();
    }
}

void PluginsPage::revertSettings()
{
    //qDebug() << "PluginsPage::revertSettings";
    DesktopSettings settings;
    foreach ( PropertyData *data, propertyList ) {
        settings.beginGroup( data->id + "/properties/" + data->prop );
        data->editor->setProperty( data->property, settings.value("value") );
        settings.endGroup();
    }
}

void PluginsPage::propertyMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "helptextChanged(QString,QString)" ) {
        QString id;
        QString property;
        stream >> id >> property;
        //qDebug() << "plugin" << id << "changed the help text of property" << property;
        DesktopSettings settings;
        foreach ( PropertyData *data, propertyList ) {
            if ( id == data->id && property == data->prop ) {
                settings.beginGroup( data->id + "/properties/" + data->prop );
                data->help->setText( settings.value("helptext").toString() );
                settings.endGroup();
            }
        }
    }
}

#include "pluginspage.moc"
