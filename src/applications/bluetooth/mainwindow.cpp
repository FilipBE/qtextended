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

#include "mainwindow.h"
#include "dirdeleterdialog.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>
#include <QAbstractItemModel>
#include <QListView>
#include <QModelIndex>
#include <QIODevice>
#include <QLineEdit>
#include <QLabel>

#include <QBluetoothRemoteDeviceDialog>
#include <QBluetoothSdpQuery>
#include <QBluetoothLocalDevice>
#include <QBluetoothRfcommSocket>

#include <QObexFtpClient>

#include <QtopiaItemDelegate>
#include <QWaitWidget>
#include <QMimeType>
#include <QSoftMenuBar>
#include <QContent>
#include <QDocumentSelectorDialog>
#include <QtopiaApplication>
#include <QFileInfo>
#include <QCommDeviceSession>
#include <QFileSystem>

#include <QObexFolderListingEntryInfo>

#include <sys/vfs.h>    // statfs

class ObexFtpModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ObexFtpModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex addInfo(const QObexFolderListingEntryInfo &info);
    QModelIndex mkdir(const QString &name);
    void clear();
    void removeInfo(int index);

    inline const QObexFolderListingEntryInfo &info(int index) const;
    inline QObexFolderListingEntryInfo & info(int index);
    int size() const { return m_infos.size(); }

private:
    QList<QObexFolderListingEntryInfo> m_infos;
    QIcon m_folderPix;
    QIcon m_filePix;
    int m_editableIndex;
};

ObexFtpModel::ObexFtpModel(QObject *parent)
    : QAbstractItemModel(parent),
       m_folderPix(":icon/folder"),
       m_filePix(":icon/fileopen"),
       m_editableIndex(-1)
{

}

Qt::ItemFlags ObexFtpModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (index.row() == m_editableIndex)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QModelIndex ObexFtpModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();

    if ((row < 0) || (column < 0))
        return QModelIndex();

    if ((row < m_infos.size()) && (column < 12))
        return createIndex(row, column, 0);

    return QModelIndex();
}

QModelIndex ObexFtpModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int ObexFtpModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_infos.size();
}

int ObexFtpModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 12;
}

QVariant ObexFtpModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ((role == Qt::DisplayRole || role == Qt::EditRole) && (index.column() == 0)) {

        if (m_infos[index.row()].isParent())
            return QVariant::fromValue(tr("Parent Folder"));

        if (m_infos[index.row()].description().isEmpty())
            return QVariant::fromValue(m_infos[index.row()].name());
        else
            return QVariant::fromValue(m_infos[index.row()].description());
    }

    if ((role == Qt::DecorationRole) && (index.column() == 0)) {
        if (m_infos[index.row()].isParent() || m_infos[index.row()].isFolder())
            return QVariant::fromValue(m_folderPix);
        else {
            // Try to determine the mimetype icon

            QMimeType mimetype;

            if (!m_infos[index.row()].type().isEmpty()) {
                mimetype = QMimeType::fromId(m_infos[index.row()].type());
            } else {
                mimetype = QMimeType::fromFileName(m_infos[index.row()].name());
            }

            if (!mimetype.isNull())
                return QVariant::fromValue(mimetype.icon());

            return QVariant::fromValue(m_filePix);
        }
    }

    return QVariant();
}

bool ObexFtpModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (index.row() != m_editableIndex)
        return false;

    if (index.column() != 0)
        return false;

    m_infos[index.row()].setName(value.toString());
    emit dataChanged(index, index);
    return true;
}

QVariant ObexFtpModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case 0:
            return QVariant::fromValue(tr("Name"));
        case 1:
            return QVariant::fromValue(tr("Size"));
        case 2:
            return QVariant::fromValue(tr("Last Modified"));
        case 3:
            return QVariant::fromValue(tr("Created On"));
        case 4:
            return QVariant::fromValue(tr("Last Accessed"));
        case 5:
            return QVariant::fromValue(tr("User Permissions"));
        case 6:
            return QVariant::fromValue(tr("Group Permissions"));
        case 7:
            return QVariant::fromValue(tr("Other Permissions"));
        case 8:
            return QVariant::fromValue(tr("Owner"));
        case 9:
            return QVariant::fromValue(tr("Group"));
        case 10:
            return QVariant::fromValue(tr("Mimetype"));
        case 11:
            return QVariant::fromValue(tr("Description"));
        default:
            break;
    };

    return QVariant();
}

QModelIndex ObexFtpModel::addInfo(const QObexFolderListingEntryInfo &info)
{
    if (info.isParent()) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_infos.push_front(info);
        endInsertRows();

        return createIndex(0, 0, 0);
    }

    int row = m_infos.size();
    beginInsertRows(QModelIndex(), row, row);
    m_infos.push_back(info);
    endInsertRows();

    return createIndex(row, 0, 0);
}

QModelIndex ObexFtpModel::mkdir(const QString &name)
{
    QObexFolderListingEntryInfo info;
    info.setName(name);
    info.setFolder(true);
    info.setParent(false);
    info.setFile(false);

    QModelIndex ret = addInfo(info);
    if (ret.isValid())
        m_editableIndex = ret.row();

    return ret;
}

void ObexFtpModel::clear()
{
    if (m_infos.size() == 0)
        return;

    beginRemoveRows(QModelIndex(), 0, m_infos.size()-1);
    m_infos.clear();
    endRemoveRows();
}

void ObexFtpModel::removeInfo(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_infos.removeAt(index);
    endRemoveRows();
}

inline const QObexFolderListingEntryInfo &ObexFtpModel::info(int index) const
{
    return m_infos[index];
}

inline QObexFolderListingEntryInfo & ObexFtpModel::info(int index)
{
    return m_infos[index];
}

class BtFtpProgressBar : public QProgressBar
{
public:
    BtFtpProgressBar(QWidget *parent = 0);

    void setText(const QString &text);
    QString text() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private:
    QString m_text;
};

BtFtpProgressBar::BtFtpProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
}

void BtFtpProgressBar::setText(const QString &text)
{
    m_text = text;
}

QString BtFtpProgressBar::text() const
{
    return m_text;
}

QSize BtFtpProgressBar::sizeHint() const
{
    return QSize( fontMetrics().height(), fontMetrics().height() / 2);
}

QSize BtFtpProgressBar::minimumSizeHint() const
{
    return QSize( fontMetrics().height(), fontMetrics().height() / 2);
}

class BtFtpPrivate : public QObject
{
    Q_OBJECT

public slots:
    void browseDevice();
    void searchCompleted(const QBluetoothSdpQueryResult &);
    void searchCancelled();
    void cancelSearch();

    void rfcommConnected();
    void rfcommError(QBluetoothAbstractSocket::SocketError error);
    void rfcommDisconnected();

    void itemActivated(const QModelIndex &index);
    void listInfo(const QObexFolderListingEntryInfo &info);

    void commandStarted(int id);
    void commandFinished(int id, bool error);

    void createFolder();

    void putFile();
    void deleteFileOrFolder();
    void disconnectFromServer();

    void getFile();
    void commitFile();
    void getInfo(const QString &, const QString &, const QDateTime &);

    void dataTransferProgress(qint64, qint64);

    void closeConnection();
    void cancelCommand();
    void done(bool error);

    void refresh();

private slots:
    void queryUserExit();
    void updateActions();
    void delayedClose();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public:
    enum CdCommandType {
        CdToRootFolder,
        CdToSubFolder
    };

    BtFtpPrivate(QMainWindow *parent);

    QWidget *m_central;
    QMenu *m_menu;
    QBluetoothAddress m_addr;

    QBluetoothSdpQuery *m_sdap;
    bool m_sdapCanceled;

    QWaitWidget *m_waitWidget;

    QListView *m_files;
    ObexFtpModel *m_model;

    BtFtpProgressBar *m_progress;

    QObexFtpClient *m_client;

    QPixmap m_folderPix;
    QPixmap m_filePix;
    QPixmap m_parentPix;

    QAction *m_browseAction;
    QAction *m_deleteAction;
    QAction *m_createAction;
    QAction *m_putAction;
    QAction *m_cancelAction;
    QAction *m_disconnectAction;

    QMainWindow *m_parent;

    QBluetoothRfcommSocket *m_rfcommSock;
    QCommDeviceSession *m_session;

    QContent m_fileBeingObtained;

    QHash<int, CdCommandType> m_pendingCdTypes;
    int m_dirDepth;

private:
    void enableActions();
    void disableActions();
};

BtFtpPrivate::BtFtpPrivate(QMainWindow *parent)
    : m_sdap(0),
      m_client(0),
      m_parent(parent),
      m_session(0)
{
    m_rfcommSock = new QBluetoothRfcommSocket;
    connect(m_rfcommSock, SIGNAL(connected()), this, SLOT(rfcommConnected()));
    connect(m_rfcommSock, SIGNAL(disconnected()), this,
            SLOT(rfcommDisconnected()));
}

BtFtp::BtFtp(QWidget *parent, Qt::WFlags fl) : QMainWindow(parent, fl)
{
    m_data = new BtFtpPrivate(this);

    m_data->m_browseAction = new QAction(QIcon(":icon/new"), tr("Browse"), this);
    connect(m_data->m_browseAction, SIGNAL(triggered()), m_data, SLOT(browseDevice()));
    m_data->m_browseAction->setWhatsThis(tr("Browse a device"));

    m_data->m_deleteAction = new QAction(QIcon(":icon/trash"), tr("Delete"), this);
    connect( m_data->m_deleteAction, SIGNAL(triggered()), m_data, SLOT(deleteFileOrFolder()));
    m_data->m_deleteAction->setWhatsThis(tr("Delete File or Folder"));

    m_data->m_createAction = new QAction(QIcon(":icon/new"), tr("New Folder"), this);
    connect(m_data->m_createAction, SIGNAL(triggered()), m_data, SLOT(createFolder()));
    m_data->m_createAction->setWhatsThis(tr("Create Folder"));

    m_data->m_putAction = new QAction(QIcon(":icon/right"), tr("Put..."), this);
    connect(m_data->m_putAction, SIGNAL(triggered()), m_data, SLOT(putFile()));
    m_data->m_putAction->setWhatsThis(tr("Put File"));

    m_data->m_disconnectAction = new QAction(tr("Disconnect"), this);
    connect(m_data->m_disconnectAction, SIGNAL(triggered()), m_data, SLOT(disconnectFromServer()));
    m_data->m_disconnectAction->setWhatsThis(tr("Disconnect from Server"));

    m_data->m_cancelAction = new QAction(tr("Cancel"), this);
    connect(m_data->m_cancelAction, SIGNAL(triggered()), m_data, SLOT(cancelCommand()));
    m_data->m_cancelAction->setWhatsThis(tr("Cancel current action"));

    m_data->m_menu = QSoftMenuBar::menuFor(this);
    m_data->m_menu->addAction(m_data->m_browseAction);
    m_data->m_menu->addAction(m_data->m_deleteAction);
    m_data->m_menu->addAction(m_data->m_createAction);
    m_data->m_menu->addAction(m_data->m_putAction);
    m_data->m_menu->addAction(m_data->m_disconnectAction);
    m_data->m_menu->addAction(m_data->m_cancelAction);

    m_data->m_disconnectAction->setVisible(false);
    m_data->m_putAction->setVisible(false);
    m_data->m_createAction->setVisible(false);
    m_data->m_deleteAction->setVisible(false);
    m_data->m_cancelAction->setVisible(false);

    m_data->m_waitWidget = new QWaitWidget( 0 );

    m_data->m_central = new QWidget(this);
    setCentralWidget(m_data->m_central);

    m_data->m_model = new ObexFtpModel(this);

    m_data->m_files = new QListView(m_data->m_central);
    m_data->m_files->setItemDelegate(new QtopiaItemDelegate);
    m_data->m_files->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_data->m_files->installEventFilter(m_data);
    m_data->m_files->setFocusPolicy(Qt::StrongFocus);
    m_data->m_files->setFocus();
    m_data->m_files->setModel(m_data->m_model);
    QSoftMenuBar::setLabel(m_data->m_files, Qt::Key_Select, QSoftMenuBar::NoLabel);

    connect(m_data->m_files, SIGNAL(activated(QModelIndex)),
            m_data, SLOT(itemActivated(QModelIndex)));
    connect(m_data->m_files->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            m_data, SLOT(updateActions()));

    m_data->m_progress = new BtFtpProgressBar(m_data->m_central);
    m_data->m_progress->setRange(0, 1);
    m_data->m_progress->setValue(0);
    m_data->m_progress->setTextVisible(false);
    m_data->m_progress->setFocusPolicy(Qt::NoFocus);

    m_data->m_dirDepth = 0;

    QVBoxLayout *widgetLayout = new QVBoxLayout(m_data->m_central);
    widgetLayout->addWidget(m_data->m_files);
    widgetLayout->addWidget(m_data->m_progress);

    setWindowTitle(tr("Bluetooth FTP Client"));

    QTimer::singleShot(0, m_data, SLOT(browseDevice()));
}

BtFtp::~BtFtp()
{
    delete m_data->m_rfcommSock;
    delete m_data->m_sdap;
    delete m_data->m_waitWidget;
    delete m_data->m_session;
    delete m_data;
}

void BtFtpPrivate::listInfo(const QObexFolderListingEntryInfo &info)
{
    m_model->addInfo(info);

    if (!m_files->currentIndex().isValid()) {
        m_files->setCurrentIndex(m_model->index(0,0));
        m_files->setFocus();
    }
}

void BtFtpPrivate::itemActivated(const QModelIndex &index)
{
    if (m_client->currentId())
        return;

    if (m_model->info(index.row()).isParent()) {
        m_client->cdUp();
    }
    else if (m_model->info(index.row()).isFolder()) {
        int id = m_client->cd(m_model->info(index.row()).name());
        m_pendingCdTypes.insert(id, CdToSubFolder);
    }
    else {
        getFile();
    }
}

void BtFtpPrivate::closeConnection()
{
    if (!m_client)
        return;

    m_client->abort();
    m_client->deleteLater();
    m_client = 0;
}

void BtFtpPrivate::browseDevice()
{
    QBluetoothLocalDevice device;

    if (!device.isValid()) {
        QMessageBox::warning(0, tr("Bluetooth error"),
                             "<qt>"+tr("There are no Bluetooth devices present!")+"</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }

    m_waitWidget->setText( tr( "Starting..." ) );
    m_waitWidget->setCancelEnabled(false);
    m_waitWidget->show();

    m_session = QCommDeviceSession::session(device.deviceName().toLatin1());

    if (!m_session) {
        m_waitWidget->setText( tr("Could not start Bluetooth session!") );
        m_waitWidget->setCancelEnabled(true);
        return;
    }

    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(QBluetooth::FileTransferProfile);

    m_addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(m_parent, profiles);

    if (!m_addr.isValid()) {
        m_waitWidget->hide();
        return;
    }

    if (!m_sdap) {
        m_sdap = new QBluetoothSdpQuery(this);
        QObject::connect(m_sdap, SIGNAL(searchCancelled()),
                         this, SLOT(searchCancelled()));

        QObject::connect(m_sdap,
                         SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
                         this, SLOT(searchCompleted(QBluetoothSdpQueryResult)));

        QObject::connect(m_waitWidget, SIGNAL(cancelled()),
                         this, SLOT(cancelSearch()));
    }

    m_sdap->searchServices(m_addr, device,
                                   QBluetooth::FileTransferProfile);

    m_sdapCanceled = false;
    m_waitWidget->setText( tr( "Searching..." ) );
    m_waitWidget->setCancelEnabled(true);
}

void BtFtpPrivate::cancelSearch()
{
    if (m_sdapCanceled)
        return;

    m_waitWidget->setText( tr( "Canceling...") );
    m_waitWidget->setCancelEnabled(false);

    m_sdap->cancelSearch();

    m_sdapCanceled = true;
}

void BtFtpPrivate::searchCompleted(const QBluetoothSdpQueryResult &result)
{
    m_waitWidget->setText(tr("Connecting..."));
    m_sdapCanceled = false;

    if (!result.isValid()) {
        QMessageBox::warning(0, tr("Connection error"),
                             "<qt>"+tr("Unable to connect.  Please ensure the Bluetooth device is in range and try again.")+"</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }

    int channel = -1;
    foreach (QBluetoothSdpRecord record, result.services()) {
        if (record.isInstance(QBluetooth::FileTransferProfile)) {
            channel = QBluetoothSdpRecord::rfcommChannel(record);
            break;
        }
    }

    if (channel == -1) {
        // This really shouldn't happen, as we just validated.
        QMessageBox::warning(0, tr("Connection error"),
                             "<qt>"+tr("The selected device does not support this service!")+"</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }

    connect(m_rfcommSock, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
            this, SLOT(rfcommError(QBluetoothAbstractSocket::SocketError)));

    if (m_rfcommSock->connect(QBluetoothAddress::any, m_addr, channel)) {
    }
    else {
        QMessageBox::warning(0, tr("Connection error"),
                             "<qt>" + tr("Could not connect to the remote service!") + "</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

void BtFtpPrivate::rfcommConnected()
{
    qLog(Bluetooth) << "BtFtpPrivate: RFCOMM connection opened";

    m_waitWidget->hide();
    disconnect(m_rfcommSock, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
            this, SLOT(rfcommError(QBluetoothAbstractSocket::SocketError)));

    m_client = new QObexFtpClient(m_rfcommSock, this);
    QObject::connect(m_client, SIGNAL(commandFinished(int,bool)),
                        this, SLOT(commandFinished(int,bool)));
    QObject::connect(m_client, SIGNAL(commandStarted(int)),
                        this, SLOT(commandStarted(int)));
    QObject::connect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
                        this, SLOT(listInfo(QObexFolderListingEntryInfo)));
    QObject::connect(m_client, SIGNAL(getInfo(QString,QString,QDateTime)),
                        this, SLOT(getInfo(QString,QString,QDateTime)));
    QObject::connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                        this, SLOT(dataTransferProgress(qint64,qint64)));
    QObject::connect(m_client, SIGNAL(done(bool)),
                     this, SLOT(done(bool)));

    m_model->clear();

    m_client->connect();
    m_pendingCdTypes.insert(m_client->cd(), CdToRootFolder);

    QSoftMenuBar::clearLabel(m_files, Qt::Key_Select);
}

void BtFtpPrivate::rfcommDisconnected()
{
    done(false);
}

void BtFtpPrivate::rfcommError(QBluetoothAbstractSocket::SocketError error)
{
    qLog(Bluetooth) << "Connection Error occurred in BtFTP: " << error <<
            m_rfcommSock->errorString();

    disconnect(m_rfcommSock, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
            this, SLOT(rfcommError(QBluetoothAbstractSocket::SocketError)));

    m_waitWidget->setCancelEnabled(true);
    m_waitWidget->setText(tr("Could not connect to the remote service!"));
    QTimer::singleShot(2000, m_waitWidget, SLOT(hide()));
}

void BtFtpPrivate::searchCancelled()
{
    m_waitWidget->hide();
    m_sdapCanceled = false;

    m_progress->setRange(0, 1);
    m_progress->setValue(0);
}

void BtFtpPrivate::commandStarted(int id)
{
    Q_UNUSED(id)

    disableActions();
}

void BtFtpPrivate::disableActions()
{
    QObexFtpClient::Command command = m_client->currentCommand();
    m_progress->setRange(0, 0);

    m_createAction->setVisible(false);
    m_deleteAction->setVisible(false);
    m_putAction->setVisible(false);
    m_disconnectAction->setVisible(false);

    if ((command == QObexFtpClient::Get) || (command == QObexFtpClient::Put)) {
        m_cancelAction->setVisible(true);
    }
}

void BtFtpPrivate::updateActions()
{
    if (!m_client) {
        m_createAction->setVisible(false);
        m_deleteAction->setVisible(false);
        m_putAction->setVisible(false);
        m_disconnectAction->setVisible(false);
        return;
    }

    QObexFtpClient::Command command = m_client->currentCommand();

    if (command != QObexFtpClient::None)
        return;

    QModelIndex index = m_files->currentIndex();
    if (!index.isValid())
        m_deleteAction->setVisible(false);
    else
        m_deleteAction->setVisible(!m_model->info(index.row()).isParent());
}

void BtFtpPrivate::enableActions()
{
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_progress->reset();

    m_cancelAction->setVisible(false);

    m_createAction->setVisible(true);
    m_putAction->setVisible(true);
    m_disconnectAction->setVisible(true);

    QModelIndex index = m_files->currentIndex();
    if (index.isValid())
        m_deleteAction->setVisible(!m_model->info(index.row()).isParent());
}

void BtFtpPrivate::commandFinished(int id, bool error)
{
    QObexFtpClient::Command command = m_client->currentCommand();

    if (error) {

        switch (m_client->error()) {
            case QObexFtpClient::Aborted:
                enableActions();
                return;
            case QObexFtpClient::AuthenticationFailed:
                qWarning() << "Authentication Required!";
                delete m_client;
                m_client = 0;
                m_rfcommSock->close();
                return;
            case QObexFtpClient::InvalidRequest:
            case QObexFtpClient::InvalidResponse:
            case QObexFtpClient::UnknownError:
                QMessageBox::warning(0, tr("Connection error"),
                             "<qt>"+tr("The connection has been lost.")+"</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                delete m_client;
                m_client = 0;
                m_rfcommSock->close();
                return;
            default:
                break;
        };

        enableActions();

        QString message;
        switch (command) {
            case QObexFtpClient::Connect:
                message = tr("Connection failed.");
                m_client->deleteLater();
                m_client = 0;
                m_rfcommSock->close();
                break;
            case QObexFtpClient::Cd:
                message = tr("Could not change the current working folder");
                m_pendingCdTypes.remove(id);
                break;
            case QObexFtpClient::CdUp:
                message = tr("Could not change the current working folder");
                break;
            case QObexFtpClient::List:
                if (m_client->error() == QObexFtpClient::RequestFailed) {
                    message = tr("Parsing the folder listing failed!");
                } else {
                    message = tr("Could not obtain the folder listing");
                }
                break;
            case QObexFtpClient::Get:
                m_client->currentDevice()->close();
                delete m_client->currentDevice();
                m_fileBeingObtained.removeFiles();
                message = tr("Get failed");
                break;
            case QObexFtpClient::Put:
                message = tr("Put failed");
                delete m_client->currentDevice();
                break;
            case QObexFtpClient::Mkdir:
                message = tr("Could not create a new folder");
                break;
            case QObexFtpClient::Rmdir:
                message = tr("Could not remove folder");
                break;
            case QObexFtpClient::Remove:
                message = tr("Could not remove file");
                break;
            case QObexFtpClient::Disconnect:
                m_client->deleteLater();
                m_client = 0;
                m_rfcommSock->close();
                break;
            default:
                break;
        };

        if (!message.isEmpty())
            QMessageBox::warning(0, tr("Bluetooth error"),
                                 "<qt>" + message + "</qt>",
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);

        if (command == QObexFtpClient::Mkdir && error) {
            QTimer::singleShot(0, this, SLOT(refresh()));
        }

        return;
    }

    // Cd and Mkdir, Remove, Put, Rmdir and CdToParent are followed by List
    // So don't touch the progress bar in this case
    if ((command == QObexFtpClient::List) ||
        (command == QObexFtpClient::Disconnect) ||
        (command == QObexFtpClient::Get)) {
        enableActions();
    } else if (command != QObexFtpClient::Connect) {
        m_client->list();
        m_model->clear();
        m_browseAction->setVisible(false);
        m_disconnectAction->setVisible(true);
        m_putAction->setVisible(true);
        m_createAction->setVisible(true);
        m_deleteAction->setVisible(true);
    }

    switch (command) {
        case QObexFtpClient::Disconnect:
            delete m_client;
            m_client = 0;
            m_rfcommSock->close();
            break;

        case QObexFtpClient::Get:
            commitFile();
            break;

        case QObexFtpClient::Put:
            m_client->currentDevice()->close();
            delete m_client->currentDevice();

        case QObexFtpClient::Cd:
            if (m_pendingCdTypes.contains(id)) {
                if (m_pendingCdTypes.take(id) == CdToRootFolder)
                    m_dirDepth = 0;
                else
                    m_dirDepth++;
            }
            break;

        case QObexFtpClient::CdUp:
            if (m_dirDepth > 0)
                m_dirDepth--;
            break;

        default:
            break;
    };
}

void BtFtpPrivate::disconnectFromServer()
{
    m_client->disconnect();
}

void BtFtpPrivate::refresh()
{
    m_model->clear();
    m_client->list();
}

bool BtFtpPrivate::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = reinterpret_cast<QKeyEvent *>(event);
        switch ( ke->key() ) {
            case Qt::Key_Left:
                //if (m_model->info(0).isParent()) {    // some clients don't send parent-folder
                if (m_dirDepth > 0) {
                    m_client->cdUp();
                    m_model->clear();
                }
                return true;

            case Qt::Key_Back:
            {
                if (!m_client || (m_client->state() == QObexFtpClient::Unconnected))
                    return false;

                QTimer::singleShot(0, this, SLOT(queryUserExit()));
                ke->accept();
                return true;
            }

            default:
                return QObject::eventFilter(obj, event);
        }
    }

    return QObject::eventFilter(obj, event);
}

void BtFtpPrivate::queryUserExit()
{
    QMessageBox::StandardButton result = QMessageBox::question(0, tr("Really Disconnect?"),
        tr("<qt>You are currently connected to a remote device, do you really wish to disconnect?</qt>"),
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::No) {
        return;
    }

    QObject::disconnect(m_client, SIGNAL(commandFinished(int,bool)),
                    this, SLOT(commandFinished(int,bool)));
    QObject::disconnect(m_client, SIGNAL(commandStarted(int)),
                    this, SLOT(commandStarted(int)));
    QObject::disconnect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
                    this, SLOT(listInfo(QObexFolderListingEntryInfo)));
    QObject::disconnect(m_client, SIGNAL(done(bool)),
                     this, SLOT(done(bool)));
    QObject::disconnect(m_rfcommSock, SIGNAL(disconnected()),
                        this, SLOT(rfcommDisconnected()));
    QObject::connect(m_client, SIGNAL(done(bool)),
                        qApp, SLOT(quit()));

    m_client->abort();
    m_client->disconnect();

    QTimer::singleShot(2000, this, SLOT(delayedClose()));
}

void BtFtpPrivate::delayedClose()
{
    if (m_rfcommSock && m_rfcommSock->state() != QBluetoothAbstractSocket::UnconnectedState) {
        delete m_client;
        m_client = 0;
        m_rfcommSock->close();
        qApp->quit();
    }
}

void BtFtpPrivate::deleteFileOrFolder()
{
    QModelIndex index = m_files->currentIndex();

    if (!index.isValid())
        return;

    QObexFolderListingEntryInfo &info = m_model->info(index.row());

    if (info.isParent())
        return;

    if (!Qtopia::confirmDelete(0, tr("Delete"), info.name()))
        return;

    if (info.isFolder()) {
        QObject::disconnect(m_client, SIGNAL(commandFinished(int,bool)),
                         this, SLOT(commandFinished(int,bool)));
        QObject::disconnect(m_client, SIGNAL(commandStarted(int)),
                         this, SLOT(commandStarted(int)));
        QObject::disconnect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
                         this, SLOT(listInfo(QObexFolderListingEntryInfo)));

        if (DirDeleterDialog::deleteDirectory(info.name(), m_client, m_parent) ==
            DirDeleterDialog::Failed) {
            QMessageBox::warning(0, tr("Bluetooth error"),
                            "<qt>"+tr("Removing directory failed.")+"</qt>",
                            QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        }

        QObject::connect(m_client, SIGNAL(commandFinished(int,bool)),
                         this, SLOT(commandFinished(int,bool)));
        QObject::connect(m_client, SIGNAL(commandStarted(int)),
                         this, SLOT(commandStarted(int)));
        QObject::connect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
                         this, SLOT(listInfo(QObexFolderListingEntryInfo)));

        m_model->clear();
        m_client->list();
    } else {
        m_client->remove(info.name());
    }
}

void BtFtpPrivate::createFolder()
{
    QDialog d;
    d.setWindowTitle(tr("New Folder"));
    QLabel label(tr("Enter folder name:"));
    QLineEdit lineEdit;
    QVBoxLayout layout;
    layout.addWidget(&label);
    layout.addWidget(&lineEdit);
    d.setLayout(&layout);
    connect(&lineEdit, SIGNAL(editingFinished()), &d, SLOT(accept()));
    if (QtopiaApplication::execDialog(&d) != QDialog::Accepted
        || lineEdit.text().trimmed().isEmpty()) {
        return;
    }

    QModelIndex index = m_model->mkdir(lineEdit.text());
    if (index.isValid()) {
        m_files->setCurrentIndex(index);
        m_client->mkdir(lineEdit.text());
    }
}

void BtFtpPrivate::putFile()
{
    QDocumentSelectorDialog dialog;
    dialog.setFilter(QContentFilter(QContent::Document));
    if (QtopiaApplication::execDialog(&dialog) == QDialog::Accepted) {
        QContent content = dialog.selectedDocument();
        QIODevice *io = content.open(QIODevice::ReadOnly);

        if (!io) {
            QString message(tr("Document System Error"));
            QMessageBox::warning(0, tr("Bluetooth error"),
                                 "<qt>" + message + "</qt>",
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            return;
        }

        QFileInfo fileInfo(content.fileName());

        m_client->put(io, fileInfo.fileName(), content.size(), content.type(), content.name(),
                      content.lastUpdated());
    }
}

void BtFtpPrivate::getFile()
{
    m_fileBeingObtained = QContent();

    QModelIndex index = m_files->currentIndex();

    struct statfs fs;
    if (statfs(QFileSystem::documentsFileSystem().documentsPath().toLocal8Bit(), &fs) == 0) {
        qint64 availableStorage = fs.f_bavail * fs.f_bsize;
        if (m_model->info(index.row()).size() > availableStorage) {
            QString message(tr("Not enough free disk space for this file."));
            QMessageBox::warning(0, tr("Bluetooth error"),
                                 "<qt>" + message + "</qt>",
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            return;
        }
    }

    const QString &name = m_model->info(index.row()).name();
    m_fileBeingObtained.setName(name);

    // Mimetypes are sometimes not provided in the listing info
    // They can be provided once the file is being obtained
    // through the getInfo signal
    // Attempt to set the mimetype to something reasonable here
    // and override it if the OBEX server does provide a mimetype
    // on the get
    if (!m_model->info(index.row()).type().isEmpty())
        m_fileBeingObtained.setType(m_model->info(index.row()).type());
    else {
        int pos;
        if ((pos = name.lastIndexOf(QChar('.'))) != -1) {
            QString extension = name.mid(pos+1);

            //TODO: Fix this to use a QMimetype::fromExtension or equivalent
            // This sets it to application/octet-stream
            // If it doesn't understand the extension
            QMimeType mimetype(extension);  
            m_fileBeingObtained.setType(mimetype.id());
        }
        else {
            m_fileBeingObtained.setType("application/octet-stream");
        }
    }

    QIODevice *io = m_fileBeingObtained.open(QIODevice::WriteOnly);

    if (!io) {
        QString message(tr("Document System Error"));
        QMessageBox::warning(0, tr("Bluetooth error"),
                             "<qt>" + message + "</qt>",
                             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }

    if (!m_model->info(index.row()).description().isEmpty()) {
        m_fileBeingObtained.setName(m_model->info(index.row()).description());
    }

    m_client->get(name, io);
}

void BtFtpPrivate::commitFile()
{
    QIODevice *current = m_client->currentDevice();
    current->close();
    delete current;

    m_fileBeingObtained.commit();
}

void BtFtpPrivate::getInfo(const QString &mimetype,
                           const QString &description,
                           const QDateTime &lastModified)
{
    Q_UNUSED(description)
    Q_UNUSED(lastModified)

    if (!mimetype.isEmpty())
        m_fileBeingObtained.setType(mimetype);
}

void BtFtpPrivate::dataTransferProgress(qint64 bytes, qint64 total)
{
    if (total) {
        m_progress->setRange(0, total);
        m_progress->setValue(bytes);
    }
    else {
        m_progress->setRange(0, 0);
        m_progress->setValue(0);
    }
}

void BtFtpPrivate::cancelCommand()
{
    if (!m_client)
        return;

    m_client->abort();
}

void BtFtpPrivate::done(bool error)
{
    Q_UNUSED(error)

    if (m_rfcommSock->state() == QBluetoothAbstractSocket::UnconnectedState) {
        if (m_client) {
            m_client->deleteLater();
            m_client = 0;
        }

        if (m_session) {
            m_session->endSession();
            delete m_session;
            m_session = 0;
        }

        m_model->clear();
        m_browseAction->setVisible(true);
        m_disconnectAction->setVisible(false);
        m_putAction->setVisible(false);
        m_createAction->setVisible(false);
        m_deleteAction->setVisible(false);

        m_progress->setRange(0, 1);
        m_progress->setValue(0);

        QSoftMenuBar::setLabel(m_files, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }
}

#include "mainwindow.moc"
