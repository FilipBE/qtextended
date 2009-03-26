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

#include "vpnui.h"

#include <QButtonGroup>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QListView>
#include <QListWidget>
#include <QRadioButton>

#include <qvaluespace.h>
#include <qvpnfactory.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qsoftmenubar.h>
#include <QAction>
#include <QMenu>


class VPNListModel : public QAbstractListModel {
    Q_OBJECT
public:
    VPNListModel( QObject* parent )
        : QAbstractListModel( parent )
    {
        vpnSpace = new QValueSpaceItem( "/Network/VPN", this );
        connect( vpnSpace, SIGNAL(contentsChanged()), this, SLOT(updateVPNs()) );
        updateVPNs();
    }

    int rowCount( const QModelIndex& parent = QModelIndex() ) const
    {
        Q_UNUSED( parent );
        return vpnIDList.size();
    }

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const
    {
        if ( !index.isValid() || index.row() < 0 || index.row()>=vpnIDList.count() )
            return QVariant();

        const uint id = vpnIDList.at( index.row() );
        if ( !vpns.contains( id ) )
            return QVariant();

        const QVPNClient *const  client = vpns[id];
        switch ( role ) {
            case Qt::DisplayRole:
                return client->name();
                break;
            case Qt::DecorationRole:
                if ( client->state() == QVPNClient::Disconnected )
                    return QIcon(":icon/vpndown");
                else if ( client->state() == QVPNClient::Pending )
                    return QIcon(":icon/vpnpending");
                else
                    return QIcon(":icon/vpnup");
                break;
        }
        return QVariant();
    }

    /*!
      Adds \a vpn to this model. The model takes ownership of the given vpn client.
      */
    void addVPN( QVPNClient* vpn )
    {
        if ( !vpn )
            return;
        vpn->setParent( this );
        connect( vpn, SIGNAL(connectionStateChanged(bool)), this, SLOT(vpnStateChanged(bool)) );
        uint id = vpn->id();
        beginInsertRows( QModelIndex(), vpnIDList.count(), vpnIDList.count() );
        vpnIDList.append( id );
        vpns[id] = vpn;
        endInsertRows();
    }

    void removeVPN( const QModelIndex& index )
    {
        if ( !index.isValid() || index.row() < 0 || index.row()>= vpnIDList.count() )
            return;
        beginRemoveRows( QModelIndex(), index.row(), index.row() );
        const uint id = vpnIDList.takeAt( index.row() );
        QVPNClient* client = vpns.take( id );
        if ( client ) {
            client->cleanup();
            delete client;
        }
        //TODO work out what to do when deleting an existing vpn
        endRemoveRows();
    }

    QVPNClient* vpn( const QModelIndex& index )
    {
        if ( !index.isValid() || index.row() < 0 || index.row()>= vpnIDList.count() )
            return 0;
        return vpns[vpnIDList.at( index.row() )];
    }

signals:
    void vpnStateChanged( const QModelIndex& index );

private slots:
    void vpnStateChanged( bool ) {
        QVPNClient* client = qobject_cast<QVPNClient*> (sender());
        if ( !client )
            return;
        QModelIndex idx = index( vpnIDList.indexOf( client->id() ), 0 );
        emit vpnStateChanged( idx );
    }

    void updateVPNs()
    {
        QVPNFactory factory;
        QSet<uint> allVPNs = factory.vpnIDs();
        QSet<uint> cpKnownIDs = vpnIDList.toSet();

        QSet<uint> toBeRemoved = cpKnownIDs;
        toBeRemoved.subtract( allVPNs );
        QSet<uint> toBeAdded = allVPNs;
        toBeAdded.subtract( cpKnownIDs );
        if ( !toBeRemoved.isEmpty() ) {
            int start;
            foreach( uint vpnID, toBeRemoved ) {
                start = vpnIDList.indexOf( vpnID );
                beginRemoveRows(QModelIndex(), start, start );
                vpnIDList.removeAt( start );
                delete vpns.take( vpnID );
                endRemoveRows();
            }
        }

        if ( !toBeAdded.isEmpty() ) {
            beginInsertRows( QModelIndex(), vpnIDList.count(), toBeAdded.count() );
            foreach( uint vpnID, toBeAdded ) {
                QVPNClient* newVPN = factory.instance( vpnID, this );
                if ( !newVPN )
                    continue;
                connect( newVPN, SIGNAL(connectionStateChanged(bool)), this, SLOT(vpnStateChanged(bool)) );
                vpnIDList.append( vpnID );
                vpns[vpnID] = newVPN;
            }
            endInsertRows();
        }

        //fire data changed signal in case the update was triggered by state change
        cpKnownIDs.intersect( allVPNs );
        QModelIndex idx;
        foreach(uint vpnID, cpKnownIDs ) {
            idx = index( vpnIDList.indexOf( vpnID ), 0 );
            emit dataChanged( idx, idx );
        }
    }

private:
    QValueSpaceItem* vpnSpace;
    QHash<uint,QVPNClient*> vpns;
    QList<uint> vpnIDList;
};

class NewVPNDialog : public QDialog
{
    Q_OBJECT
public:
    NewVPNDialog( QWidget* parent = 0 )
        :QDialog( parent )
    {
        setWindowTitle( tr("New VPN type", "VPN - virtual private network") );
        setModal( true );

        QVBoxLayout* vb = new QVBoxLayout( this );
        vb->setSpacing( 0 );
        vb->setMargin( 4 );

        bg = new QButtonGroup( this );
        QSet<QVPNClient::Type> set = QVPNFactory::types();
        foreach( QVPNClient::Type t, set ) {
            QString name;
            switch ( t ) {
                case QVPNClient::OpenVPN:
                    name = tr("OpenVPN");
                    break;
                case QVPNClient::IPSec:
                    name = tr("IPSec");
                    break;
                default:
                    continue;
            }

            QRadioButton* radio = new QRadioButton( name, this );
            vb->addWidget( radio );
            bg->addButton( radio );
            bg->setId( radio, t );
        }

        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Next );
    }

    /*!
     Returns -1 if nothing has been selected
    */
    int selectedClientID() {
        return bg->checkedId();
    }

protected:
    void keyPressEvent( QKeyEvent* ke )
    {
        switch( ke->key() ) {
            case Qt::Key_Back:
                accept();
                break;
            default:
                break;
        }
    }

private:
    QButtonGroup* bg;
};


VpnUI::VpnUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    init();
}

VpnUI::~VpnUI()
{
}

void VpnUI::init()
{

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 2 );
    vbox->setSpacing( 4 );

    QLabel* label = new QLabel( this );
    label->setWordWrap( true );
    label->setText( tr("VPN connections:") );
    vbox->addWidget( label );

    model = new VPNListModel( this );
    connect( model, SIGNAL(vpnStateChanged(QModelIndex)),
                this, SLOT(newVPNSelected(QModelIndex)) );

    vpnList = new QListView( this );
    vpnList->setModel( model );
    vpnList->setSelectionBehavior( QAbstractItemView::SelectRows );
    vpnList->setSelectionMode( QAbstractItemView::SingleSelection );
    connect( vpnList, SIGNAL(activated(QModelIndex)),
            this, SLOT(vpnActivated(QModelIndex)) );

    QItemSelectionModel * selModel = vpnList->selectionModel();
    connect( selModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(newVPNSelected(QModelIndex,QModelIndex)) );

    vbox->addWidget( vpnList );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );

    QAction* newAction = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect( newAction, SIGNAL(triggered(bool)), this, SLOT(newConnection()) );
    contextMenu->addAction(newAction);

    removeAction = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(removeConnection()) );
    contextMenu->addAction( removeAction );

    propertyAction = new QAction( QIcon(":icon/settings"), tr("Properties"), this );
    connect(propertyAction, SIGNAL(triggered(bool)), this, SLOT(editConnection()) );
    contextMenu->addAction( propertyAction );

    if ( model->rowCount() )
        selModel->setCurrentIndex( model->index( 0, 0 ), QItemSelectionModel::ClearAndSelect );
}

void VpnUI::newConnection()
{
    NewVPNDialog dlg;
    if ( QtopiaApplication::execDialog( &dlg, true ) == QDialog::Accepted ) {
        int id = dlg.selectedClientID();
        if ( id >= 0 ) {
            QVPNClient::Type t = (QVPNClient::Type) id;
            QVPNFactory factory;
            QVPNClient* client = factory.create( t );
            if ( client ) {
                QDialog* cfgDlg = client->configure( this );
                if ( !cfgDlg )
                    return;

                if ( QtopiaApplication::execDialog( cfgDlg ) == QDialog::Accepted ) {
                    qLog(VPN) << "New VPN created" << client->name() << client->id();
                    model->addVPN( client );
                } else {
                    client->cleanup();
                    delete client;
                }

                delete cfgDlg;
            }
        } else {
            return;
        }
    }
    vpnList->setEditFocus( true );
}

void VpnUI::editConnection()
{
    QVPNClient* vpn = model->vpn( vpnList->currentIndex() );
    if ( !vpn )
        return;

    QDialog* cfgDlg = vpn->configure();
    if ( !cfgDlg )
        return;

    QtopiaApplication::execDialog( cfgDlg );
    delete cfgDlg;
    vpnList->setEditFocus( true );

}

void VpnUI::newVPNSelected( const QModelIndex& cur, const QModelIndex& )
{
    const bool valid = cur.isValid();
    const QVPNClient* vpn = model->vpn( cur );
    if ( !vpn ) {
        propertyAction->setVisible( false );
        removeAction->setVisible( false );
        return;
    }

    propertyAction->setVisible( valid && vpn->state() == QVPNClient::Disconnected );
    removeAction->setVisible( valid && vpn->state() == QVPNClient::Disconnected );
}

void VpnUI::removeConnection()
{
    const QModelIndex idx = vpnList->currentIndex();
    if ( !idx.isValid() )
        return;

    QVPNClient* vpn = model->vpn( idx );
    if ( !vpn )
        return;

    if ( Qtopia::confirmDelete( this, tr("Remove VPN"), vpn->name() ) ) {
        model->removeVPN( idx );
    }
    vpnList->setEditFocus( true );
}

void VpnUI::vpnActivated( const QModelIndex& itemIndex )
{
    if ( !itemIndex.isValid() )
        return;

    QVPNClient* vpn = model->vpn( itemIndex );
    if ( !vpn )
        return;

    QVPNClient::State state = vpn->state();
    if ( state != QVPNClient::Disconnected ) {
        vpn->disconnect();
    } else {
        vpn->connect();
    }
}
#include "vpnui.moc"
