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
#include "qdocumentselectorsocketserver_p.h"
#include <QLineEdit>
#include <QComboBox>
#include <QStorageDeviceSelector>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileSystem>
#include <QtopiaApplication>
#include <QDocumentSelector>
#include <QContentSortCriteria>
#include <QCategorySelector>

Q_IMPLEMENT_USER_METATYPE_ENUM(QIODevice::OpenMode);

class NewDocumentDialog : public QDialog
{
public:
    NewDocumentDialog( QWidget *parent = 0 );

    void setName( const QString &name );
    QString name() const;

    void setTypes( const QStringList &types );
    QString type() const;

    QString location() const;

    void setCategories( const QStringList &categories );
    QStringList categories() const;

private:
    QLineEdit *m_nameEdit;
    QComboBox *m_typeEdit;
    QStorageDeviceSelector *m_locationEdit;
    QCategorySelector *m_categoryEdit;
};

NewDocumentDialog::NewDocumentDialog( QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( QDocumentSelectorServer::tr( "Save As..." ) );

    QFormLayout *layout = new QFormLayout;

    layout->addRow( QDocumentSelectorServer::tr( "Name" ), m_nameEdit = new QLineEdit( this ) );
    layout->addRow( QDocumentSelectorServer::tr( "Location" ), m_locationEdit = new QStorageDeviceSelector( this ) );
    layout->addRow( QDocumentSelectorServer::tr( "Category" ),
                    m_categoryEdit = new QCategorySelector("Documents", QCategorySelector::Editor | QCategorySelector::DialogView) );
    layout->addRow( QDocumentSelectorServer::tr( "Type" ), m_typeEdit = new QComboBox( this ) );

    setLayout( layout );
}

void NewDocumentDialog::setName( const QString &name )
{
    m_nameEdit->setText( name );
}

QString NewDocumentDialog::name() const
{
    return m_nameEdit->text();
}

void NewDocumentDialog::setTypes( const QStringList &types )
{
    m_typeEdit->clear();

    m_typeEdit->addItems( types );
}

QString NewDocumentDialog::type() const
{
    return m_typeEdit->currentText();
}

QString NewDocumentDialog::location() const
{
    return m_locationEdit->installationPath();
}

void NewDocumentDialog::setCategories( const QStringList &categories )
{
    m_categoryEdit->selectCategories( categories );
}

QStringList NewDocumentDialog::categories() const
{
    return m_categoryEdit->selectedCategories();
}

class SaveDocumentDialog : public QDialog
{
public:
    SaveDocumentDialog( QWidget *parent = 0 );

    void setContent( const QContent &content );

    QStringList categories() const;

private:
    QLabel *m_nameLabel;
    QLabel *m_typeLabel;
    QLabel *m_locationLabel;
    QCategorySelector *m_categoryEdit;
};

SaveDocumentDialog::SaveDocumentDialog( QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( QDocumentSelectorServer::tr( "Save..." ) );

    QFormLayout *layout = new QFormLayout;

    layout->addRow( QDocumentSelectorServer::tr( "Name" ), m_nameLabel = new QLabel( this ) );
    layout->addRow( QDocumentSelectorServer::tr( "Location" ), m_locationLabel = new QLabel( this ) );
    layout->addRow( QDocumentSelectorServer::tr( "Category" ),
                    m_categoryEdit = new QCategorySelector("Documents", QCategorySelector::Editor | QCategorySelector::DialogView) );
    layout->addRow( QDocumentSelectorServer::tr( "Type" ), m_typeLabel = new QLabel( this ) );

    setLayout( layout );
}

void SaveDocumentDialog::setContent( const QContent &content )
{
    m_nameLabel->setText( content.name() );
    m_typeLabel->setText( content.type() );
    m_categoryEdit->selectCategories( content.categories() );

    m_locationLabel->setText( QFileSystem::fromFileName( content.fileName() ).name() );
}

QStringList SaveDocumentDialog::categories() const
{
    return m_categoryEdit->selectedCategories();
}

QDocumentSelectorServer::QDocumentSelectorServer( QObject *parent )
    : QDocumentServerHost( "QDocumentSelectorServer", parent )
    , m_selector( 0 )
    , m_selectorDialog( 0 )
    , m_newDocumentDialog( 0 )
    , m_saveDocumentDialog( 0 )
    , m_openMode( QIODevice::ReadOnly )
{
}

QDocumentSelectorServer::~QDocumentSelectorServer()
{
    delete m_selectorDialog;
    delete m_newDocumentDialog;
    delete m_saveDocumentDialog;
}

QDocumentServerMessage QDocumentSelectorServer::invokeMethod( const QDocumentServerMessage &message )
{
    Q_UNUSED( message );

    Q_ASSERT( false );

    return QDocumentServerMessage();
}

void QDocumentSelectorServer::invokeSlot( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "openDocument(QContentFilter,QContentSortCriteria)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        if( !m_selector )
        {
            m_selectorDialog = new QDialog;

            m_selectorDialog->setWindowTitle( tr( "Open..." ) );

            QVBoxLayout *layout = new QVBoxLayout( m_selectorDialog );

            layout->setMargin( 0 );
            layout->setSpacing( 0 );
            layout->addWidget( m_selector = new QDocumentSelector( m_selectorDialog ) );

            connect( m_selector, SIGNAL(documentSelected(QContent)), this, SLOT(documentSelected(QContent)) );
            connect( m_selector, SIGNAL(documentSelected(QContent)), m_selectorDialog, SLOT(hide()) );
            connect( m_selectorDialog, SIGNAL(rejected()), this, SLOT(rejected()) );

            QtopiaApplication::setMenuLike( m_selectorDialog, true );
        }

        m_selector->setFilter( qvariant_cast< QContentFilter >( arguments.at( 0 ) ) );
        m_selector->setSortCriteria( qvariant_cast< QContentSortCriteria >( arguments.at( 1 ).toInt() ) );

        m_openMode = QIODevice::ReadOnly;

        QtopiaApplication::showDialog( m_selectorDialog );
    }
    else if( signature == "newDocument(QString,QStringList)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( !m_newDocumentDialog )
        {
            m_newDocumentDialog = new NewDocumentDialog;

            connect( m_newDocumentDialog, SIGNAL(accepted()), this, SLOT(newDocumentAccepted()) );
            connect( m_newDocumentDialog, SIGNAL(rejected()), this, SLOT(rejected()) );
        }

        m_openMode = QIODevice::WriteOnly;

        m_newDocumentDialog->setName( qvariant_cast< QString >( arguments.at( 0 ) ) );
        m_newDocumentDialog->setTypes( qvariant_cast< QStringList >( arguments.at( 1 ) ) );
        m_newDocumentDialog->setCategories( QStringList() );

        QtopiaApplication::showDialog( m_newDocumentDialog );
    }
    else if( signature == "saveDocument()" )
    {
        Q_ASSERT( arguments.count() == 0 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        if( !m_saveDocumentDialog )
        {
            m_saveDocumentDialog = new SaveDocumentDialog;

            connect( m_saveDocumentDialog, SIGNAL(accepted()), this, SLOT(saveDocumentAccepted()) );
            connect( m_saveDocumentDialog, SIGNAL(rejected()), this, SLOT(rejected()) );
        }

        m_openMode = QIODevice::WriteOnly;

        m_saveDocumentDialog->setContent( m_selectedDocument );

        QtopiaApplication::showDialog( m_saveDocumentDialog );
    }
    else if( signature == "close()" )
    {
        Q_ASSERT( arguments.count() == 0 );

        if( m_selectorDialog && m_selectorDialog->isVisible() )
            m_selectorDialog->hide();

        if( m_newDocumentDialog && m_newDocumentDialog->isVisible() )
            m_newDocumentDialog->hide();

        m_selectedDocument == QContent();
        m_openMode = QIODevice::NotOpen;
    }
}

void QDocumentSelectorServer::rejected()
{
    emitSignalWithArgumentList( "cancelled()", QVariantList() );
}

void QDocumentSelectorServer::documentSelected( const QContent &document )
{
    m_selectedDocument = document;

    QFile file( document.fileName() );

    if( file.open( m_openMode ) )
    {
        emitSignalWithArgumentList( "documentOpened(QContent,QIODevice::OpenMode)",
                QVariantList() << QVariant::fromValue( document ) << QVariant::fromValue( m_openMode ),
                QList< QUnixSocketRights >() << QUnixSocketRights( file.handle() ) );

        file.close();
    }
    else
    {
        qWarning() << file.errorString();
    }
}

void QDocumentSelectorServer::newDocumentAccepted()
{
    QContent document;

    document.setName( m_newDocumentDialog->name() );
    document.setMedia( m_newDocumentDialog->location() );
    document.setType( m_newDocumentDialog->type() );

    QIODevice *device = document.open( QIODevice::WriteOnly );

    if( device )
    {
        device->close();

        delete device;

        document.commit();

        documentSelected( document );
    }
    else
        rejected();
}

void QDocumentSelectorServer::saveDocumentAccepted()
{
    m_selectedDocument.setCategories( m_saveDocumentDialog->categories() );
    m_selectedDocument.commit();

    documentSelected( m_selectedDocument );
}

QDocumentSelectorSocketServer::QDocumentSelectorSocketServer( QObject *parent )
    : QUnixSocketServer( parent )
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QDocumentSelectorServer" )).toLocal8Bit();

    listen( socketPath );
}

void QDocumentSelectorSocketServer::incomingConnection( int socketDescriptor )
{
    QDocumentSelectorServer *server = new QDocumentSelectorServer( this );

    server->setSocketDescriptor( socketDescriptor );
}
