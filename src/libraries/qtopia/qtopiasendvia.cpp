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

#include <qtopiasendvia.h>
#include <qcontent.h>
#include <qmimetype.h>
#include <qtopiaservices.h>

#include <QMap>
#include <QDialog>
#include <QVBoxLayout>
#include <qtopiaapplication.h>

#include <QListWidget>
#include <QDesktopWidget>

#include <QDSServices>
#include <QDSServiceInfo>
#include <QDSData>
#include <QDSServiceInfo>
#include <QDSAction>
#include <QtopiaItemDelegate>

#if defined(QTOPIA_INFRARED)
#include <qvaluespace.h>
#endif

class QtopiaSendFileDialog : public QDialog
{
    Q_OBJECT

public:
    QtopiaSendFileDialog(QWidget *parent,
                         const QString &filename,
                         const QString &mimetype,
                         const QString &description,
                         bool autodelete);
    ~QtopiaSendFileDialog();

protected slots:
    void serverClicked( QListWidgetItem* );

private:
    QString m_filename;
    QString m_mimetype;
    QString m_description;
    bool m_autodelete;
};

QtopiaSendFileDialog::QtopiaSendFileDialog(QWidget *parent,
                                           const QString &filename,
                                           const QString &mimetype,
                                           const QString &description,
                                           bool autodelete)
    : QDialog(parent)
{
    m_filename = filename;
    m_mimetype = mimetype;
    m_description = description;
    m_autodelete = autodelete;

    setModal(true);
    setWindowTitle( tr ( "Send via..."));
    QVBoxLayout* l = new QVBoxLayout( this );
    l->setMargin(0);
    QListWidget* lb = new QListWidget( this );
    lb->setFrameStyle(QFrame::NoFrame);
    l->addWidget( lb );

    connect( lb,
                SIGNAL(itemActivated(QListWidgetItem*)),
                this,
                SLOT(serverClicked(QListWidgetItem*)) );

#ifdef QTOPIA_BLUETOOTH
    {
        QListWidgetItem *item = new QListWidgetItem( tr("Bluetooth"), lb );
        item->setIcon( QPixmap(":icon/bluetooth/bluetooth-online") );
        item->setData( Qt::UserRole, QByteArray("Bluetooth") );
    }
#endif

#ifdef QTOPIA_INFRARED
    {
        QListWidgetItem *item = new QListWidgetItem( tr("Infrared"), lb );
        item->setIcon( QPixmap(":icon/beam") );
        item->setData( Qt::UserRole, QByteArray("Infrared") );
    }
#endif

    lb->sortItems();
    lb->setCurrentRow(0);
}

QtopiaSendFileDialog::~QtopiaSendFileDialog()
{

}

void QtopiaSendFileDialog::serverClicked(QListWidgetItem *item)
{
    if (item->data( Qt::UserRole ).toByteArray() == "Bluetooth") {
        QtopiaServiceRequest req("BluetoothPush", "pushFile(QString,QString,QString,bool)");
        req << m_filename << m_mimetype << m_description << m_autodelete;
        req.send();
    }

    else if (item->data( Qt::UserRole ).toByteArray()  == "Infrared") {
        QtopiaServiceRequest req("InfraredBeaming", "beamFile(QString,QString,QString,bool)");
        req << m_filename << m_mimetype << m_description << m_autodelete;
        req.send();
    }

    accept();
}

class QtopiaSendViaDialog : public QDialog
{
    Q_OBJECT

public:
    QtopiaSendViaDialog( const QByteArray &data,
                         const QString &mimetype, QWidget* parent );
    ~QtopiaSendViaDialog();

protected slots:
    void serverClicked( QListWidgetItem* );

private:
    QDSData m_data;
    QMap< QListWidgetItem*, QDSServiceInfo > mItemToService;
};

QtopiaSendViaDialog::QtopiaSendViaDialog(const QByteArray &data,
                                         const QString &mimetype,
                                         QWidget *parent) : QDialog(parent)
{
    QDSServices services(mimetype,
                         QString( "" ),
                         QStringList( QString ( "send" ) ) );

    m_data = QDSData( data, QMimeType( mimetype ) );

    if ( services.count() > 0) {
        setModal(true);
        setWindowTitle( tr ( "Send via..."));
        QVBoxLayout* l = new QVBoxLayout( this );
        l->setMargin(0);
        QListWidget* lb = new QListWidget( this );
        lb->setItemDelegate(new QtopiaItemDelegate);
        lb->setFrameStyle(QFrame::NoFrame);
        l->addWidget( lb );

        connect( lb,
                 SIGNAL(itemActivated(QListWidgetItem*)),
                 this,
                 SLOT(serverClicked(QListWidgetItem*)) );

        foreach ( QDSServiceInfo serviceinfo, services ) {
            QListWidgetItem* item
                    = new QListWidgetItem( serviceinfo.description(), lb );
            item->setIcon( QIcon( ":icon/" + serviceinfo.icon() ) );
            mItemToService.insert( item, serviceinfo );
        }

        lb->sortItems();
        lb->setCurrentRow(0);
    }
}

QtopiaSendViaDialog::~QtopiaSendViaDialog()
{

}

void QtopiaSendViaDialog::serverClicked(QListWidgetItem *item)
{
    // Create QDS action and make request
    QDSAction action( mItemToService[item] );
    action.invoke(m_data);
    accept();
}

/*!
    \class QtopiaSendVia
    \inpublicgroup QtBaseModule
    \brief The QtopiaSendVia class provides an easy way of sending files via communications services.

    QtopiaSendVia makes it easy to send files, business cards,
    tasks and calendar items to other devices via supported
    communications protocols, e.g. Bluetooth, Infrared, SMS, Email, etc.
*/

/*!
    Returns true if there is a handler in the system that can
    handle sending the \a mimetype.

    \sa isFileSupported()
*/
bool QtopiaSendVia::isDataSupported(const QString &mimetype)
{
    QDSServices services(mimetype,
                    QString( "" ),
                    QStringList( QString ( "send" ) ) );

    return services.count() > 0;
}

/*!
    Returns true if there is a handler in the system that
    can handle sending files.  Not all services can send
    files, but some, like Bluetooth and Infrared can.

    \sa isDataSupported()
*/
bool QtopiaSendVia::isFileSupported()
{
#if defined(QTOPIA_BLUETOOTH)
    return true;
#endif

#if defined(QTOPIA_INFRARED)
    QValueSpaceItem item("/Communications/Infrared/");
    bool ret = item.value("Available").toBool();

    return ret;
#endif
    return false;
}

/*!
    Send \a data with \a mimetype via a communications link.  The
    user will first be asked what communications medium to use
    via a standard dialog.  Once the user has made their choice,
    the appropriate handler will be invoked and the data will be
    sent.  The \a parent specifies the parent widget for the dialog.
    If the \a parent is NULL, then the choice dialog is constructed as
    a top level dialog.

    The user should use isDataSupported() to check that the
    data can be sent.

    Returns true if the request could be started, and false otherwise.

    \sa sendFile(), isDataSupported(), QDialog
*/
bool QtopiaSendVia::sendData(QWidget *parent, const QByteArray &data, const QString &mimetype)
{
    QDSServices services(mimetype,
                    QString( "" ),
                    QStringList( QString ( "send" ) ) );

    if (services.count() == 0)
        return false;

    if (services.count() > 1) {
        QtopiaSendViaDialog *dlg = new QtopiaSendViaDialog(data, mimetype, parent);

        QtopiaApplication::setMenuLike( dlg, true );
        QtopiaApplication::execDialog( dlg );
        delete dlg;
    } else {
        QDSData qdsdata = QDSData( data, QMimeType( mimetype ) );
        QDSServiceInfo serviceinfo = services.at(0);
        QDSAction action(serviceinfo);
        action.invoke(qdsdata);
    }

    return true;
}

/*!
    Send \a filename with \a mimetype via a communications link.  The
    user will first be asked what communications medium to use
    via a standard dialog.  Once the user has made their choice, the
    appropriate handler will be invoked and the data will be sent.

    The \a description will be used as a user-friendly
    description of the file where necessary in the user interface.

    The \a parent specifies the parent widget for the dialog.  If the
    \a parent is NULL, then the choice dialog is constructed as a top
    level dialog.

    Set \a autodelete to true if the file is a temporary file.  The
    invoked handler will take care of deleting the file once it has
    been sent.  If you do not wish the file to be deleted, set
    \a autodelete to false.

    The user should use isFileSupported() first to find out whether
    file sending is supported.

    Returns true if the request could be started, and false otherwise.

    \sa isFileSupported(), sendData(), QDialog
*/
bool QtopiaSendVia::sendFile(QWidget *parent, const QString &filename, const QString &mimetype,
                             const QString &description, bool autodelete)
{
#if defined(QTOPIA_BLUETOOTH) || defined(QTOPIA_INFRARED)
    if (!isFileSupported())
        return false;

#if defined(QTOPIA_BLUETOOTH) && defined(QTOPIA_INFRARED) // Present a choice
    QtopiaSendFileDialog *dlg = new QtopiaSendFileDialog(parent, filename, mimetype, description, autodelete);

    QtopiaApplication::setMenuLike( dlg, true );
    QtopiaApplication::execDialog( dlg );
    delete dlg;
#elif defined(QTOPIA_BLUETOOTH) // send straight to bluetooth
    Q_UNUSED(parent);
    QtopiaServiceRequest req("BluetoothPush", "pushFile(QString,QString,QString,bool)");
    req << filename << mimetype << description << autodelete;
    req.send();
#elif defined(QTOPIA_INFRARED) // send straight to infrared
    Q_UNUSED(parent);
    QtopiaServiceRequest req("InfraredBeaming", "beamFile(QString,QString,QString,bool)");
    req << filename << mimetype << description << autodelete;
    req.send();
#endif

#else
    Q_UNUSED(parent);
    Q_UNUSED(filename);
    Q_UNUSED(mimetype);
    Q_UNUSED(description);
    Q_UNUSED(autodelete);
#endif

    return true;
}

/*!
    This is an overloaded member function, provided for convenience.

    This version works on QContent objects.

    The file to send is given by \a content.  Set \a autodelete to
    true if the file should be deleted once
    the request is processed.  The \a parent holds the parent widget
    for the dialog that will be shown to the user.

    The QContent::name() of \a content will be used as a user-friendly
    description of the object to be sent.
*/
bool QtopiaSendVia::sendFile(QWidget *parent, const QContent &content, bool autodelete)
{
    QMimeType mime(content);
    return sendFile(parent, content.fileName(), mime.id(), content.name(), autodelete);
}

#include "qtopiasendvia.moc"
