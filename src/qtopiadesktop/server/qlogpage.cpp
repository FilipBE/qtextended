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
#include <desktopsettings.h>
#include <qtopiadesktoplog.h>
#include <trace.h>
QD_LOG_OPTION(QLOG)

#include <QWidget>
#include <QMap>
#include <QString>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <qdebug.h>
#include <QCheckBox>

class QLogPageApp;

class QLogPage : public QWidget
{
    Q_OBJECT
public:
    QLogPage( QLogPageApp *plugin, QWidget *parent = 0 );
    ~QLogPage();

public slots:
    void applySettings();
    void revertSettings();
    void recheckSettings();
    void allClicked();
    void itemClicked();

private:
    QLogPageApp *plugin;
    QMap<QString,QTreeWidgetItem*> itemMap;
    QTreeWidget *list;
    QCheckBox *allChecked;
    bool ignoreChecked;
};

// ====================================================================

class QLogPageApp : public QDAppPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(QLogPageApp,QDAppPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.app.qlogpage"; }
    QString displayName() { return tr("QLog Categories"); }

    // QDAppPlugin
    QIcon icon() { return QPixmap(":image/appicon"); }
    QWidget *initApp() { return 0; }
    QWidget *initSettings() { return new QLogPage( this ); }
};

QD_REGISTER_PLUGIN(QLogPageApp)

// ====================================================================

QLogPage::QLogPage( QLogPageApp *_plugin, QWidget *parent )
    : QWidget( parent ),
    plugin( _plugin )
{
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );

    // Padding for the checkbox (added below)
    QWidget *padding = new QWidget;
    vbox->addWidget( padding );

    // Using a tree since when the number of log streams gets large, we
    // should put them in an hierarchy.
    list = new QTreeWidget;

    list->setRootIsDecorated(false);
    list->setColumnCount(1);
    list->header()->hide();
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    //list->setFrameStyle(QFrame::NoFrame);
    connect( list, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemClicked()) );
    vbox->addWidget(list);

    vbox = new QVBoxLayout( padding );
    vbox->setMargin( 6 );
    vbox->setSpacing( 0 );

    allChecked = new QCheckBox;
    allChecked->setTristate( true );
    // Cannot add this string due to the string freeze in effect on 4.3
    //allChecked->setText( tr("All") );
    connect( allChecked, SIGNAL(stateChanged(int)), this, SLOT(allClicked()) );
    vbox->addWidget( allChecked );
    ignoreChecked = false;

    recheckSettings();
}

QLogPage::~QLogPage()
{
}

void QLogPage::applySettings()
{
    TRACE(QLOG) << "QLogPage::applySettings";
    DesktopSettings settings( "Log" );
    foreach ( const QString &name, settings.childKeys() ) {
        QTreeWidgetItem *i = itemMap[name];
        if ( !i ) continue;
        LOG() << name << "=" << (i->checkState(0) == Qt::Checked);
        settings.setValue( name, ( i->checkState(0) == Qt::Checked ) );
    }
    // clear the cache (write the file first)
    settings.sync();
    qtopiadesktopLogEnabled( 0 );
}

void QLogPage::revertSettings()
{
    TRACE(QLOG) << "QLogPage::revertSettings";
    ignoreChecked = true;
    DesktopSettings settings( "Log" );
    int all = -1;
    foreach ( const QString &name, settings.childKeys() ) {
        QTreeWidgetItem *i = itemMap[name];
        if ( !i ) continue;
        bool enabled = settings.value(name).toBool();
        if ( all == -1 )
            all = enabled?Qt::Checked:Qt::Unchecked;
        if ( all == Qt::Checked && !enabled )
            all = Qt::PartiallyChecked;
        if ( all == Qt::Unchecked && enabled )
            all = Qt::PartiallyChecked;
        LOG() << name << "=" << enabled;
        i->setCheckState( 0, ( enabled ? Qt::Checked : Qt::Unchecked ) );
    }
    LOG() << "all" << "=" << all;
    allChecked->setCheckState( (Qt::CheckState)all );
    ignoreChecked = false;
}

void QLogPage::recheckSettings()
{
    TRACE(QLOG) << "QLogPage::recheckSettings";
    ignoreChecked = true;
    list->clear();
    DesktopSettings settings( "Log" );
    QStringList sortedKeys = settings.childKeys();
    sortedKeys.sort();
    int all = -1;
    foreach ( const QString &name, sortedKeys ) {
        QTreeWidgetItem *i = new QTreeWidgetItem( list, QStringList() << name );
        bool enabled = qtopiadesktopLogEnabled( name.toLocal8Bit().constData() );
        if ( all == -1 )
            all = enabled?Qt::Checked:Qt::Unchecked;
        if ( all == Qt::Checked && !enabled )
            all = Qt::PartiallyChecked;
        if ( all == Qt::Unchecked && enabled )
            all = Qt::PartiallyChecked;
        LOG() << name << "=" << enabled;
        i->setCheckState( 0, ( enabled ? Qt::Checked : Qt::Unchecked ) );
        itemMap.insert(name,i);
    }
    LOG() << "all" << "=" << all;
    allChecked->setCheckState( (Qt::CheckState)all );
    ignoreChecked = false;

    list->setCurrentItem(list->topLevelItem(0));
}

void QLogPage::allClicked()
{
    TRACE(QLOG) << "QLogPage::allClicked";
    Qt::CheckState checked = allChecked->checkState();
    LOG() << "all" << "=" << checked;
    if ( ignoreChecked ) {
        LOG() << "ignoring";
        return;
    }
    ignoreChecked = true;
    // Strange logic because the state is changed before the slot is called
    // Qt::Unchecked = Qt::Checked was clicked
    // Qt::PartiallyChecked = Qt::Unchecked was clicked
    // Qt::Checked = Qt::PartiallyChecked was clicked
    checked = (checked == Qt::PartiallyChecked || checked == Qt::Checked)?Qt::Checked:Qt::Unchecked;
    DesktopSettings settings( "Log" );
    foreach ( const QString &name, settings.childKeys() ) {
        QTreeWidgetItem *i = itemMap[name];
        if ( !i ) continue;
        LOG() << name << "=" << (checked == Qt::Checked);
        i->setCheckState( 0, checked );
    }
    LOG() << "all" << "=" << checked;
    allChecked->setCheckState( checked );
    ignoreChecked = false;
}

void QLogPage::itemClicked()
{
    TRACE(QLOG) << "QLogPage::itemClicked";
    if ( ignoreChecked ) {
        LOG() << "ignoring";
        return;
    }
    ignoreChecked = true;
    DesktopSettings settings( "Log" );
    int all = -1;
    foreach ( const QString &name, settings.childKeys() ) {
        QTreeWidgetItem *i = itemMap[name];
        if ( !i ) continue;
        bool enabled = i->checkState(0) == Qt::Checked;
        LOG() << name << "=" << enabled;
        if ( all == -1 )
            all = enabled?Qt::Checked:Qt::Unchecked;
        if ( all == Qt::Checked && !enabled )
            all = Qt::PartiallyChecked;
        if ( all == Qt::Unchecked && enabled )
            all = Qt::PartiallyChecked;
    }
    LOG() << "all" << "=" << all;
    allChecked->setCheckState( (Qt::CheckState)all );
    ignoreChecked = false;
}

#include "qlogpage.moc"
