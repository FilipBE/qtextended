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

#include "addatt.h"

#include <qtopiaapplication.h>
#include <qtopiaglobal.h>
#include <qdocumentselector.h>
#include <qmimetype.h>

#include <qmailmessage.h>

#include <QAction>
#include <QLayout>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QWhatsThis>
#include <qsoftmenubar.h>

AttachmentItem::AttachmentItem(QListWidget *parent, const QContent& att, QMailMessage::AttachmentsAction action)
  : QListWidgetItem(parent), mAttachment(att), mAction(action)
{
    QFileInfo fi(att.fileName());
    mSizeKB = fi.size() /1024; //to kb

    setText(att.name());
    setIcon(att.icon());
}

AttachmentItem::~AttachmentItem()
{
}

const QContent& AttachmentItem::document() const
{
    return mAttachment;
}

int AttachmentItem::sizeKB() const
{
    return mSizeKB;
}

QMailMessage::AttachmentsAction AttachmentItem::action() const
{
    return mAction;
}


class AddAttachmentItem : public QListWidgetItem 
{
public:
    AddAttachmentItem(QListWidget *parent);
};

AddAttachmentItem::AddAttachmentItem(QListWidget *parent)
    : QListWidgetItem(parent)
{
    setText(QApplication::translate("AddAtt", "Add Attachment"));
    setIcon(QIcon(":icon/attach"));
}


AddAtt::AddAtt(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(parent,f)
{
    setObjectName(name);

    attView = new QListWidget(this);
    attView->setFrameStyle(QFrame::NoFrame);
    connect(attView, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), 
            this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)) );
    connect(attView, SIGNAL(itemActivated(QListWidgetItem*)), 
            this, SLOT(itemActivated(QListWidgetItem*)) );

    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin(0);
    top->addWidget(attView);

    clear();
}

AddAttBase::~AddAttBase()
{
}

void AddAttBase::setHighlighted()
{
    attView->setCurrentItem(addItem);
}

QStringList AddAttBase::mimeTypes()
{
    QString homedocs = Qtopia::documentDir();
    QDir d( homedocs );
    QStringList l = d.entryList( QDir::Dirs );
    l.removeAll(".");
    l.removeAll("..");
    return l;
}

void AddAttBase::clear()
{
    attView->clear();
    addItem = new AddAttachmentItem(attView);

    modified = false;
    getFiles();

    setHighlighted();
}

bool AddAttBase::addAttachment(const QContent &dl, QMailMessage::AttachmentsAction action)
{
    new AttachmentItem(attView, dl, action);
    setHighlighted();
    return true;
}

QList<AttachmentItem*> AddAttBase::attachedFiles() const
{
    QList<AttachmentItem*> list;

    int i = 1;
    AttachmentItem *iterItem = static_cast<AttachmentItem *>( attView->item(i) );
    while (iterItem != NULL) {
        list.append( iterItem );
        iterItem = static_cast<AttachmentItem *>( attView->item(++i) );
    }

    return list;
}

void AddAttBase::setMailMessageParts(QMailMessage *mail)
{
    for ( uint i = 0; i < mail->partCount(); i++ ) {
        QMailMessagePart &part = mail->partAt( i );

        QString attPath = part.attachmentPath();
        if(!attPath.isEmpty())
        {
            QContent d;
            d.setFile(attPath);

            if ( part.hasBody() )
            {
                QMailMessageContentType type( part.body().contentType() );
                d.setName( type.name() );
                d.setType( type.content() );
            }

            new AttachmentItem(attView, d, QMailMessage::LinkToAttachments);
        }
    }
}

int AddAttBase::totalSizeKB()
{
    int total = 0;
    foreach(const AttachmentItem* i, attachedFiles())
        total += i->sizeKB();

    return total;
}

void AddAtt::currentItemChanged(QListWidgetItem *item, QListWidgetItem*)
{
    bool hasItem = (item && (item != addItem));
    emit currentChanged( hasItem );

    // TODO: this doesn't seem to work...
    //QSoftMenuBar::setLabel(attView, Qt::Key_Select, (hasItem ? QSoftMenuBar::Deselect : QSoftMenuBar::Select));
}

void AddAtt::itemActivated(QListWidgetItem *item)
{
    if (item == addItem)
        emit addNewAttachment();
    else
        emit selected(static_cast<AttachmentItem*>(item));
}

void AddAtt::removeCurrentAttachment()
{
    if (attView->currentItem()) {
        attView->takeItem(attView->row(attView->currentItem()));
    }
    modified = true;
}

AddAttDialog::AddAttDialog(QWidget *parent, QString name, Qt::WFlags f)
  : QDialog( parent,f)
{
    setObjectName(name);
    setModal(true);
    QVBoxLayout *l = new QVBoxLayout( this );
    addAtt = new AddAtt( this );
    connect( addAtt, SIGNAL(addNewAttachment()), this, SLOT(selectAttachment()) );
    connect( addAtt, SIGNAL(selected(AttachmentItem*)), this, SLOT(removeAttachment(AttachmentItem*)) );
    connect( addAtt, SIGNAL(currentChanged(bool)), this, SLOT(updateDisplay(bool)));
    l->addWidget( addAtt );
    setWindowTitle( tr("Attachments") );

    QMenu *context = QSoftMenuBar::menuFor(this);
    QAction *attachAction = context->addAction(QIcon(":icon/attach"),tr("Add attachment"));
    connect( attachAction, SIGNAL(triggered()), this, SLOT(selectAttachment()) );
    attachAction->setWhatsThis( tr("Attach a document to this mail.") );

    removeAction = context->addAction(QIcon(":icon/attach"), tr("Remove attachment"));
    connect( removeAction, SIGNAL(triggered()), addAtt, SLOT(removeCurrentAttachment()) );
    removeAction->setVisible(false);

    fileSelector = new QDocumentSelector( this );
    fileSelector->setFilter( QContentFilter( QContent::Document ) );
    fileSelector->enableOptions( QDocumentSelector::TypeSelector | QDocumentSelector::NestTypes );
    fileSelector->setObjectName("fileselector");
    fileSelector->setSelectPermission( QDrmRights::Distribute );
    fileSelector->setMandatoryPermissions( QDrmRights::Distribute );
    connect( fileSelector, SIGNAL(documentSelected(QContent)), this, SLOT(openFile(QContent)) );

    fileSelectorDialog = new QDialog( this);
    fileSelectorDialog->setObjectName("fileSelectorDialog");
    fileSelectorDialog->setModal(true);
    QtopiaApplication::setMenuLike( fileSelectorDialog, true );
    fileSelectorDialog->setWindowTitle( tr("Add attachment") );

    QVBoxLayout *fl = new QVBoxLayout( fileSelectorDialog );
    fl->addWidget( fileSelector );

    connect(this,SIGNAL(accepted()),this,SIGNAL(attachmentsChanged()));
}

void AddAttDialog::done(int r)
{
    // Ensure that the add action is highlighted if we're shown again
    addAtt->setHighlighted();

    QDialog::done(r);
}

QList<AttachmentItem*> AddAttDialog::attachedFiles() const
{
    return addAtt->attachedFiles();
}

void AddAttDialog::clear()
{
    addAtt->clear();
}

void AddAttDialog::setMailMessageParts(QMailMessage *mail)
{
    addAtt->setMailMessageParts( mail );
}

QDocumentSelector* AddAttDialog::documentSelector() const
{
    return fileSelector;
}

void AddAttDialog::removeAttachment(AttachmentItem*)
{
    addAtt->removeCurrentAttachment();
}

void AddAttDialog::removeCurrentAttachment()
{
    addAtt->removeCurrentAttachment();
}

void AddAttDialog::updateDisplay(bool onAttachmentItem)
{
    removeAction->setVisible(onAttachmentItem);
}

void AddAttDialog::attach( const QContent &doclnk, QMailMessage::AttachmentsAction action )
{
    addAtt->addAttachment( doclnk, action );
    emit attachmentsChanged();
}

void AddAttDialog::openFile(const QContent& dl)
{
    fileSelectorDialog->accept();
    addAtt->addAttachment( dl, QMailMessage::LinkToAttachments );
}

void AddAttDialog::selectAttachment()
{
    (void)QtopiaApplication::execDialog( fileSelectorDialog );
    // don't need to do anything with return value, openFile slot called from signal
}

