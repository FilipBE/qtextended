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

#include "qdocumentproperties.h"
#include <qstoragedeviceselector.h>

#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>
#include <qcategoryselector.h>
#include <qtopiaipcenvelope.h>
#include <qstorage.h>
#include <qtopianamespace.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qdesktopwidget.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <QScrollArea>
#include <QDir>
#include <QPushButton>
#include <QDebug>
#include <QFormLayout>
#include <QWaitWidget>
#include <QCoreApplication>

#include <stdlib.h>


class QDocumentPropertiesWidgetPrivate
{
public:
    QDocumentPropertiesWidgetPrivate()
        : locationCombo(0)
        , categoryEdit(0)
        , docname(0)
        , comment(0)
        , fileSize(0)
        , fastLoad(0)
        , licensesDialog(0)
    {
    }

    QString humanReadable(quint64 size);

    QStorageDeviceSelector *locationCombo;
    QCategorySelector *categoryEdit;
    QLineEdit *docname;
    QLabel *doctype;
    QLabel *comment;
    QLabel *fileSize;
    QCheckBox *fastLoad;
    QDialog *licensesDialog;
};

QString QDocumentPropertiesWidgetPrivate::humanReadable(quint64 size)
{
    if(size == 1)
        return QObject::tr("1 byte");
    else if(size < 1024)
        return QObject::tr("%1 bytes").arg(size);
    else if(size < (1024 * 1024))
        return QObject::tr("%1 KB").arg(((float)size)/1024.0, 0, 'f', 1);
    else if(size < (1024 * 1024 * 1024))
        return QObject::tr("%1 MB").arg(((float)size)/(1024.0 * 1024.0), 0, 'f', 1);
    else
        return QObject::tr("%1 GB").arg(((float)size)/(1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}


/*!
  \class QDocumentPropertiesWidget
    \inpublicgroup QtBaseModule
  \ingroup content

  \brief The QDocumentPropertiesWidget class provides controls for viewing and modifying the
   properties of a document.

  The QDocumentPropertiesWidget allows modification of the name, location,
  and category of a document.  
  
  In addition, the following operations are available: 
    \table
    \header
        \o Operation
        \o Slot
    \row
        \o Beaming
        \o beamLnk()
    \row
        \o Duplicating or copying
        \o duplicateLnk()
    \row
        \o Deleting
        \o unlinkLnk()
    \endtable 

  On the phone edition of Qtopia, the slots would typically be invoked through the context
  menu. 
    
  If phone edition is not used, buttons to facilitate these operations will appear on the 
  QDocumentPropertiesWidget, however the slots are still available for use if required.

  \sa QDocumentPropertiesDialog
*/

/*!
  Constructs a QDocumentPropertiesWidget with a QContent,\a doc, representing the
  document, and a \a parent widget.

  Ensure that \a doc refers to a document as opposed to an application.
 */
QDocumentPropertiesWidget::QDocumentPropertiesWidget( const QContent &doc, QWidget* parent )
    : QWidget( parent ), lnk( doc )
{
    bool isDocument = lnk.isDocument();
    d = new QDocumentPropertiesWidgetPrivate;

    QFileSystem fileSystem;

    if (isDocument) {
        fileSystem = doc.isValid()
                     ? QFileSystem::fromFileName(doc.fileName())
                     : QFileSystem::documentsFileSystem();
    }

    QFormLayout *layout = new QFormLayout;

    layout->addRow(tr("Name"), d->docname = new QLineEdit);

    if (!fileSystem.isNull()) {
        d->docname->setText(lnk.name());
    } else {
        d->docname->setText(Qtopia::dehyphenate(lnk.name()));
        d->docname->setEnabled( false );
    }

    if (!fileSystem.isNull()) {
        layout->addRow(tr("Location"), d->locationCombo = new QStorageDeviceSelector);

        QFileSystemFilter *fsf = new QFileSystemFilter;
        fsf->documents = QFileSystemFilter::Set;
        d->locationCombo->setFilter(fsf);

        d->locationCombo->setLocation(fileSystem.documentsPath());

        if (!(doc.permissions() & QDrmRights::Distribute))
            d->locationCombo->setEnabled(false);

        layout->addRow(tr("Category"), d->categoryEdit = new QCategorySelector(
                "Documents", QCategorySelector::Editor | QCategorySelector::DialogView));

        d->categoryEdit->selectCategories(lnk.categories());
    }

    if (!lnk.type().isEmpty()) {
        layout->addRow(tr("Type"), d->doctype = new QLabel);

        d->doctype->setText(lnk.type() == "application/octet-stream"
                ? tr("Unknown", "Unknown document type")
                : lnk.type().replace("/", " / "));
        d->doctype->setWordWrap(true);
    }

    if (!lnk.comment().isEmpty()) {
        layout->addRow(tr("Comment"), d->comment = new QLabel("<qt>" + lnk.comment() + "</qt>"));

        d->comment->setWordWrap(true);
        d->comment->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    }

    if (lnk.isValid())
        layout->addRow(tr("File Size"), new QLabel("<qt>" + d->humanReadable(lnk.size()) + "</qt>"));

    if (!isDocument && lnk.property("CanFastload") != "0")  {
        layout->addRow(d->fastLoad = new QCheckBox(tr("Fast load (consumes memory)")));

        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toStringList();
        d->fastLoad->setChecked(apps.contains(lnk.executableName()));
    }

    if (doc.drmState() == QContent::Protected) {
        QPushButton *licensesButton = new QPushButton(tr("Show Licenses"));

        connect(licensesButton, SIGNAL(clicked()), this, SLOT(showLicenses()));

        layout->addRow(licensesButton);
    }

    setLayout(layout);
}

/*!
  Destroys the widget.
 */
QDocumentPropertiesWidget::~QDocumentPropertiesWidget()
{
    delete d;
}

/*!
    Returns the document the widget displays the properties of.
*/
QContent QDocumentPropertiesWidget::document() const
{
    return lnk;
}

/*!
  Applies any changes made on the QDocumentPropertiesWidget 
 */
void QDocumentPropertiesWidget::applyChanges()
{
    bool changed=false;
    bool isDocument = lnk.isDocument();
    if ( isDocument && lnk.name() != d->docname->text() ) {
        lnk.setName(d->docname->text());
        changed=true;

        if (lnk.isValid() && !d->fastLoad && !(d->locationCombo && d->locationCombo->isChanged())) {
            QDir dir = QFileInfo(lnk.fileName()).absoluteDir();

            QString path = dir.absoluteFilePath(safeName(d->docname->text(), dir, lnk.fileName()));

            if (QFile::rename(lnk.fileName(), path))
                lnk.setFile(path);
        }
    }
    if ( d->categoryEdit ) {
        QList<QString> tmp = d->categoryEdit->selectedCategories();
        if ( lnk.categories() != tmp ) {
            lnk.setCategories( tmp );
            changed = true;
        }
    }

    if (!lnk.isValid()) {
        lnk.setMedia(d->locationCombo->documentPath());
    } else if (!d->fastLoad && d->locationCombo && d->locationCombo->isChanged()) {
        moveLnk();
    } else if ( changed ) {
        lnk.commit();
    }

    if ( d->fastLoad ) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toStringList();
        QString exe = lnk.executableName();
        if ( (apps.contains(exe) > 0) != d->fastLoad->isChecked() ) {
            if ( d->fastLoad->isChecked() ) {
                apps.append(exe);
                QtopiaIpcEnvelope e("QPE/Application/"+exe,
                               "enablePreload()");
            } else {
                apps.removeAll(exe);
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "disablePreload()");
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "quitIfInvisible()");
            }
            cfg.setValue("PreloadApps", apps);
        }
    }
}

/*!
  Create a duplicate of the document in the document system.
 */
void QDocumentPropertiesWidget::duplicateLnk()
{
    // The duplicate takes the new properties.
    if( !lnk.copyTo( safePath( d->docname->text(), d->locationCombo->documentPath(), lnk.type(), lnk.fileName() ) ) )
    {
        QMessageBox::warning( this, tr("Duplicate"), tr("<qt>File copy failed.</qt>") );
        return;
    }
    emit done();
}

/*!
    \internal
*/
QString QDocumentPropertiesWidget::safePath( const QString &name, const QString &location, const QString &type, const QString &oldPath ) const
{
    QDir dir(location + type);

    if (!dir.exists())
        QDir::root().mkpath(dir.absolutePath());

    return dir.absoluteFilePath(safeName(name, dir, oldPath));
}

QString QDocumentPropertiesWidget::safeName(const QString &name, const QDir &directory, const QString &oldPath) const
{
    static const char SAFE_SPACE = '_';

    QString safename;
    // Remove all special ASCII characters and ensure that name does not start with a number
    QByteArray ascii = name.toAscii();
    for ( int i = 0; i < ascii.length(); i++ ){
        QChar c = ascii.at(i);
        if ( c.isLetterOrNumber() || c == QLatin1Char('.') )
            safename += c;
        else
            safename += SAFE_SPACE;
    }
    if ( safename.isEmpty() )
        safename = SAFE_SPACE;
    else if ( safename.at(0).isNumber() )
        safename.prepend( SAFE_SPACE );

    int pos = oldPath.lastIndexOf( '/' );

    pos = oldPath.indexOf( '.', pos != -1 ? pos : 0 );
    QString fileExtn;

    if ( pos > 0 )
        fileExtn = oldPath.mid( pos );

    QString possibleName = safename + fileExtn;

    int n=1;

    while (directory.exists(possibleName))
        possibleName = safename + QLatin1Char('_') + QString::number(n++) + fileExtn;

    return possibleName;
}

/*!
  \internal
 */
bool QDocumentPropertiesWidget::moveLnk()
{
    bool moved = false;
    bool error = true;

    QString path = safePath(
            d->docname->text(), d->locationCombo->documentPath(), lnk.type(), lnk.fileName());

    QFile destination(path);
    QFile source(lnk.fileName());

    QWaitWidget wait(this);
    wait.setWindowModality(Qt::WindowModal);
    wait.setCancelEnabled(true);
    wait.setText(tr("Moving %1", "%1 name of document being moved").arg(lnk.name()));

    if (destination.open(QIODevice::WriteOnly)) {
        if (source.open(QIODevice::ReadWrite)) {
            char buffer[65536];

            wait.show();

            error = false;

            while (!error && !source.atEnd() && !wait.wasCancelled()) {
                int size = source.read(buffer, 65536);

                if (size == destination.write(buffer, size))
                    QCoreApplication::processEvents();
                else
                    error = true;
            }

            moved = !error && source.atEnd();


            source.close();
        }

        destination.close();
    }

    if (moved) {
        lnk.setFile(path);

        if (lnk.commit()) {
            source.remove();
        } else {
            error = true;
            moved = false;

            destination.remove();
        }
    } else {
        destination.remove();
    }

    wait.hide();

    if (error)
        QMessageBox::warning(this, tr("Details"), tr("<qt>Moving Document failed.</qt>"));

    return moved;
}

/*!
  Beams the document. 
 */
void QDocumentPropertiesWidget::beamLnk()
{
    QtopiaSendVia::sendFile(this, lnk);
    emit done();
}

/*!
  Deletes the document.  
 */
void QDocumentPropertiesWidget::unlinkLnk()
{
    if ( Qtopia::confirmDelete( this, tr("Delete"), lnk.name() ) ) {
        lnk.removeFiles();
        if ( QFile::exists(lnk.fileName()) ) {
            QMessageBox::warning( this, tr("Delete"), tr("<qt>File deletion failed.</qt>") );
        } else {
            emit done();
        }
    }
}

/*!
  Show licenses in a new QDialog.
*/
void QDocumentPropertiesWidget::showLicenses()
{
    if (!d->licensesDialog) {
        QFormLayout *layout = new QFormLayout;
        layout->setRowWrapPolicy(QFormLayout::WrapAllRows);

        if (lnk.permissions() & QDrmRights::Play)
            addRights(lnk.rights(QDrmRights::Play), layout);

        if (lnk.permissions() & QDrmRights::Display)
            addRights(lnk.rights(QDrmRights::Display ), layout);

        if (lnk.permissions() & QDrmRights::Execute)
            addRights(lnk.rights(QDrmRights::Execute), layout);

        if (lnk.permissions() & QDrmRights::Print)
            addRights(lnk.rights(QDrmRights::Print), layout);

        if (lnk.permissions() & QDrmRights::Export)
            addRights(lnk.rights(QDrmRights::Export), layout);

        if (layout->rowCount() == 0)
            layout->addRow(new QLabel( tr( "<qt><u>No licenses</u></qt>" )));

        QWidget *main = new QWidget;
        main->setLayout(layout);

        QScrollArea *sa = new QScrollArea;
        sa->setFrameStyle(QFrame::NoFrame);
        sa->setWidget(main);
        sa->setWidgetResizable(true);

        QBoxLayout *scrollLayout = new QVBoxLayout;
        scrollLayout->addWidget(sa);

        d->licensesDialog = new QDialog(this);
        d->licensesDialog->setWindowTitle(tr("Licenses"));
        d->licensesDialog->setModal(true);
        d->licensesDialog->setLayout(scrollLayout);
    }

    d->licensesDialog->showMaximized();
}

void QDocumentPropertiesWidget::addRights(const QDrmRights &rights, QFormLayout *layout)
{
    QFormLayout *constraintsLayout = new QFormLayout;

    foreach (QDrmRights::Constraint c, rights.constraints()) {
        if (c.attributeCount() > 0) {
            QFormLayout *attributesLayout = new QFormLayout;

            for( int i = 0; i < c.attributeCount(); i++ )
                attributesLayout->addRow(QString("<qt><em>%1</em></qt>").arg(c.attributeName(i)),
                                        new QLabel(c.attributeValue( i ).toString()));

            constraintsLayout->addRow(c.name(), attributesLayout);
        } else {
            constraintsLayout->addRow(c.name(), new QLabel(c.value().toString()));
        }
    }

    QString rightsString = QString("<qt><u>%1</u></qt>")
            .arg(QDrmRights::toString(rights.permission(), rights.status()));

    layout->addRow(rightsString, constraintsLayout);
}

/*!
  \fn void QDocumentPropertiesWidget::done()

  This signal is emitted when a file is deleted, duplicated or beamed from the
  QDocumentPropertiesWidget.
 */

/*!
  \class QDocumentPropertiesDialog
    \inpublicgroup QtBaseModule
  \ingroup content

  \brief The QDocumentPropertiesDialog class allows the user to examine and 
         modify properties of a document. 

  The QDocumentPropertiesDialog is a convenience class which is built around 
  a \l QDocumentPropertiesWidget, which provides controls for modifying properties 
  associated with a document via it's associated \l QContent.

  \sa QDocumentPropertiesWidget
*/


/*!
  Constructs a QDocumentPropertiesDialog with a QContent, \a doc, representing the 
  document, and a \a parent widget.   

  Ensure that \a doc refers to a document as opposed to an application. 
 */
QDocumentPropertiesDialog::QDocumentPropertiesDialog( const QContent &doc, QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( tr("Properties") );
    setObjectName("properties");

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin(0);

    QScrollArea *scrollArea = new QScrollArea( this );
    scrollArea->setFrameStyle( QFrame::NoFrame );
    scrollArea->setFocusPolicy( Qt::NoFocus );
    scrollArea->setWidgetResizable( true );
    scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    vbox->addWidget( scrollArea );
    d = new QDocumentPropertiesWidget( doc, this );
    scrollArea->setWidget( d );
    connect( d, SIGNAL(done()), this, SLOT(reject()) );
}

/*!
  Destroys the dialog. 
 */
QDocumentPropertiesDialog::~QDocumentPropertiesDialog()
{
}

/*!
    Returns the document the dialog displays the properties of.
*/
QContent QDocumentPropertiesDialog::document() const
{
    return d->document();
}

/*!
  \reimp
 */
void QDocumentPropertiesDialog::done(int ok)
{
    if ( ok )
        d->applyChanges();
    QDialog::done( ok );
}

