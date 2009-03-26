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

#include "deskphone_emailcomposer.h"

#include <qsoftmenubar.h>
#include <qmimetype.h>
#include <qmailmessage.h>
#include <qtopiaglobal.h>
#include <QAction>
#include <QDocumentSelector>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <qtopiaapplication.h>
#include "addatt.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QToolButton>
#include <QMailStore>
#include <QMailAccount>
#include <private/addressselectorwidget_p.h>
#include <private/homewidgets_p.h>
#include <private/qtopiainputdialog_p.h>
#include <private/accountconfiguration_p.h>


// sharp 1839 to here
static void checkOutlookString(QString &str)
{
    int  pos = 0;
    int  newPos;
    QString  oneAddr;

    QStringList  newStr;
    if (str.indexOf(";") == -1) {
        // not Outlook style
        return;
    }

    while ((newPos = str.indexOf(";", pos)) != -1) {
        if (newPos > pos + 1) {
            // found some string
            oneAddr = str.mid(pos, newPos-pos);

            if (oneAddr.indexOf("@") != -1) {
                // not Outlook comment
                newStr.append(oneAddr);
            }
        }
        if ((pos = newPos + 1) >= str.length()) {
            break;
        }
    }

    str = newStr.join(", ");
}

EmailComposerInterface::EmailComposerInterface( QWidget *parent )
    : QMailComposerInterface( parent ),
      m_index( -1 ),
      m_bodyEdit(0),
      m_contactsButton(0),
      m_toEdit(0),
      m_subjectEdit(0),
      m_sizer(0),
      m_attachmentsButton(0)
{
    init();
}

EmailComposerInterface::~EmailComposerInterface()
{
    // Delete any temporary files we don't need
    foreach (const AttachmentDetail &att, m_attachments) {
        if (att.second == QMailMessage::CopyAndDeleteAttachments) {
            const_cast<QContent&>(att.first).removeFiles();
        }
    }

    delete m_sizer;
}

void EmailComposerInterface::init()
{
    m_sizer = new ColumnSizer();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    QWidget::setLayout(layout);

    QLayout* toLayout = new QHBoxLayout();
    layout->addLayout(toLayout);

    m_toEdit = new HomeFieldButton("To:","",*m_sizer,true);
    connect(m_toEdit,SIGNAL(clicked()),this,SLOT(toClicked()));
    toLayout->addWidget(m_toEdit);

    m_contactsButton= new HomeActionButton("Contact",QtopiaHome::Green);
    m_contactsButton->setMaximumWidth(45);
    m_contactsButton->setMinimumWidth(45);
    connect(m_contactsButton,SIGNAL(clicked()),this,SLOT(selectRecipients()));
    toLayout->addWidget(m_contactsButton);

    m_subjectEdit = new HomeFieldButton("Subject:","",*m_sizer,true);
    connect(m_subjectEdit,SIGNAL(clicked()),this,SLOT(subjectClicked()));
    layout->addWidget(m_subjectEdit);

    m_bodyEdit = new QTextEdit(this);
    m_bodyEdit->setFrameStyle(QFrame::NoFrame);
    m_bodyEdit->setWordWrapMode(QTextOption::WordWrap);
    m_bodyEdit->setFrameStyle(QFrame::Raised | QFrame::Box);
    connect(m_bodyEdit, SIGNAL(textChanged()), this, SIGNAL(changed()) );
    layout->addWidget(m_bodyEdit);
    QWidget::setFocusProxy(m_bodyEdit);

    m_attachmentsButton = new QToolButton(this);
    m_attachmentsButton->setText("No attachments");
    m_attachmentsButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout->addWidget(m_attachmentsButton);
    connect(m_attachmentsButton,SIGNAL(clicked()),this,SLOT(selectAttachments()));
    connect(this,SIGNAL(attachmentsChanged()),this,SLOT(updateAttachmentsLabel()));
    connect(this,SIGNAL(attachmentsChanged()),this,SIGNAL(changed()));

    m_addAttDialog = new AddAttDialog(this, "attachmentDialog");
    connect(m_addAttDialog,SIGNAL(attachmentsChanged()),this,SIGNAL(attachmentsChanged()));

    setContext("Create " + displayName(QMailMessage::Email));
}

void EmailComposerInterface::setPlainText( const QString& text, const QString& signature )
{
    if (!signature.isEmpty()) {
        QString msgText(text);
        if (msgText.endsWith(signature)) {
            // Signature already exists
            m_index = msgText.length() - (signature.length() + 1);
        } else {
            // Append the signature
            msgText.append('\n').append(signature);
            m_index = text.length();
        }

        m_bodyEdit->setPlainText(msgText);

        // Move the cursor before the signature - setting directly fails...
        QTimer::singleShot(0, this, SLOT(setCursorPosition()));
    } else {
        m_bodyEdit->setPlainText(text);
        m_bodyEdit->moveCursor(QTextCursor::End);
    }
}

void EmailComposerInterface::setContext(const QString& context)
{
    m_title = context;
    emit contextChanged();
}

bool EmailComposerInterface::isEmpty() const
{
    return (m_bodyEdit->toPlainText().isEmpty() && m_attachments.isEmpty());
}

QMailMessage EmailComposerInterface::message() const
{
    QMailMessage mail;

    QString messageText( m_bodyEdit->toPlainText() );

    QMailMessageContentType type("text/plain; charset=UTF-8");
    if (m_attachments.isEmpty()) {
        mail.setBody( QMailMessageBody::fromData( messageText, type, QMailMessageBody::Base64 ) );
    } else {
        QMailMessagePart textPart;
        textPart.setBody(QMailMessageBody::fromData(messageText.toUtf8(), type, QMailMessageBody::Base64));
        mail.setMultipartType(QMailMessagePartContainer::MultipartMixed);
        mail.appendPart(textPart);

        foreach (const AttachmentDetail &current, m_attachments) {
            QFileInfo fi(current.first.fileName());
            QString partName(fi.fileName());
            QString filePath(fi.absoluteFilePath());

            QString mimeTypeName(current.first.type());
            if (mimeTypeName.isEmpty())
                mimeTypeName = QMimeType(filePath).id();

            QMailMessageContentType type(mimeTypeName.toLatin1());
            type.setName(current.first.name().toLatin1());

            QMailMessageContentDisposition disposition( QMailMessageContentDisposition::Attachment );
            disposition.setFilename(partName.toLatin1());

            QMailMessagePart part;

            if ((current.second != QMailMessage::LinkToAttachments) ||
                (filePath.startsWith(Qtopia::tempDir()))) {
                // This file is temporary - extract the data and create a part from that
                QFile dataFile(filePath);
                if (dataFile.open(QIODevice::ReadOnly)) {
                    QDataStream in(&dataFile);

                    part = QMailMessagePart::fromStream(in, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
                } else {
                    qWarning() << "Unable to open temporary file:" << filePath;
                }
            } else {
                part = QMailMessagePart::fromFile(filePath, disposition, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
            }

            mail.appendPart(part);
        }
    }

    //if the from address is present, find the account matching the email
    //otherwise use the first valid account
    QMailAccountKey key(QMailAccountKey::MessageType, QMailMessage::Email);
    foreach (const QMailAccountId& id, QMailStore::instance()->queryAccounts(key)) {
        AccountConfiguration config(id);

        QString emailAddress = config.emailAddress();
        if (!emailAddress.isEmpty()) {
            if (emailAddress == from() || !mail.parentAccountId().isValid()) {
                mail.setFrom(QMailAddress(emailAddress));
                mail.setParentAccountId(id);
                break;
            }
        }
    }

    mail.setMessageType( QMailMessage::Email );
    mail.setTo(QMailAddress(m_toEdit->field()));
    mail.setSubject(m_subjectEdit->field());

    return mail;
}

void EmailComposerInterface::clear()
{
    m_bodyEdit->clear();
    m_addAttDialog->clear();

    // Delete any temporary files we don't need
    foreach (const AttachmentDetail &att, m_attachments) {
        if (att.second == QMailMessage::CopyAndDeleteAttachments) {
            const_cast<QContent&>(att.first).removeFiles();
        }
    }

    m_attachments.clear();
}

void EmailComposerInterface::setMessage( const QMailMessage &mail )
{
    if (mail.multipartType() == QMailMessagePartContainer::MultipartNone) {
        if (mail.hasBody())
            setBody( mail.body().data(), mail.contentType().content() );
    } else {
        // The only type of multipart message we currently compose is Mixed, with
        // all but the first part as out-of-line attachments
        int textPart = -1;
        for ( uint i = 0; i < mail.partCount(); ++i ) {
            QMailMessagePart &part = const_cast<QMailMessagePart&>(mail.partAt(i));

            if (textPart == -1 && part.contentType().type().toLower() == "text") {
                // This is the first text part, we will use as the forwarded text body
                textPart = i;
            } else {
                QString attPath = part.attachmentPath();
                QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments;

                // Detach the part data to a temporary file if necessary
                if (attPath.isEmpty()) {
                    if (part.detachAttachment(Qtopia::tempDir())) {
                        attPath = part.attachmentPath();
                        action = QMailMessage::CopyAndDeleteAttachments;

                        // Create a content object for the file
                        QContent doc(attPath);

                        if (part.hasBody()) {
                            QMailMessageContentType type(part.contentType());

                            if (doc.drmState() == QContent::Unprotected)
                                doc.setType(type.content());
                        }

                        doc.setName(part.displayName());
                        doc.setRole(QContent::Data);
                        doc.commit();
                    }
                }

                if (!attPath.isEmpty())
                    attach(QContent(attPath), action);
            }
        }

        if (textPart != -1) {
            const QMailMessagePart& part = mail.partAt(textPart);
            setBody(part.body().data(), part.contentType().content());
        }
    }

    //set the details
    setTo(QMailAddress::toStringList(mail.to()).join(", "));
    setFrom(mail.from().address());

    if (mail.subject() != tr("(no subject)")) {
        setSubject(mail.subject());
    }
}

void EmailComposerInterface::setBody( const QString &text, const QString &type )
{
    setPlainText(text,m_signature);
    Q_UNUSED(type)
}

void EmailComposerInterface::attach( const QContent &lnk, QMailMessage::AttachmentsAction action )
{
    m_attachments.append(qMakePair(lnk, action));
    m_addAttDialog->attach(lnk, action);
}

void EmailComposerInterface::setSignature( const QString &sig )
{
    QString msgText( m_bodyEdit->toPlainText() );

    if ( !msgText.isEmpty() && !m_signature.isEmpty() ) {
        // See if we need to remove the old signature
        if ( msgText.endsWith( m_signature ) )
            msgText.chop( m_signature.length() + 1 );
    }

    m_signature = sig;
    setPlainText( msgText, m_signature );
}


void EmailComposerInterface::reply(const QMailMessage& source, int action)
{
    const QString fwdIndicator(tr("Fwd"));
    const QString shortFwdIndicator(tr("Fw", "2 letter short version of Fwd for forward"));
    const QString replyIndicator(tr("Re"));

    const QString subject = source.subject().toLower();

    QString toAddress;
    QString fromAddress;
    QString ccAddress;
    QString subjectText;

    if (source.parentAccountId().isValid()) {
        QMailAccount sendingAccount(source.parentAccountId());

        if (sendingAccount.id().isValid()) {
            AccountConfiguration config(sendingAccount.id());
            fromAddress = config.emailAddress();
        }
    }

    // work out the kind of mail to response
    // type of reply depends on the type of message
    // a reply to an mms is just a new mms message with the sender as recipient
    // a reply to an sms is a new sms message with the sender as recipient

    // EMAIL
    QString originalText;
    int textPart = -1;
    QMailMessage mail;

    // Find the body of this message
    if ( source.hasBody() ) {
        originalText = source.body().data();
    } else {
        for ( uint i = 0; i < source.partCount(); ++i ) {
            const QMailMessagePart &part = source.partAt(i);

            if (part.contentType().type().toLower() == "text") {
                // This is the first text part, we will use as the forwarded text body
                originalText = part.body().data();
                textPart = i;
                break;
            }
        }
    }

    if ( action == Forward ) {
        // Copy the existing mail
        mail = source;
        mail.setId(QMailMessageId());

        if ((subject.left(fwdIndicator.length() + 1) == (fwdIndicator.toLower() + ":")) ||
                (subject.left(shortFwdIndicator.length() + 1) == (shortFwdIndicator.toLower() + ":"))) {
            subjectText = source.subject();
        } else {
            subjectText = fwdIndicator + ": " + source.subject();
        }
    } else {
        if (subject.left(replyIndicator.length() + 1) == (replyIndicator.toLower() + ":")) {
            subjectText = source.subject();
        } else {
            subjectText = replyIndicator + ": " + source.subject();
        }

        QMailAddress replyAddress(source.replyTo());
        if (replyAddress.isNull())
            replyAddress = source.from();

        QString str = replyAddress.address();
        checkOutlookString(str);
        toAddress = str;

        QString messageId = mail.headerFieldText( "message-id" ).trimmed();
        if ( !messageId.isEmpty() )
            mail.setInReplyTo( messageId );
    }

    QString bodyText;
    if (action == Forward) {
        bodyText = "\n------------ Forwarded Message ------------\n";
        bodyText += "Date: " + source.date().toString() + "\n";
        bodyText += "From: " + source.from().toString() + "\n";
        bodyText += "To: " + QMailAddress::toStringList(source.to()).join(", ") + "\n";
        bodyText += "Subject: " + source.subject() + "\n";
        bodyText += "\n" + originalText;
    } else {
        QDateTime dateTime = source.date().toLocalTime();
        bodyText = "\nOn " + QTimeString::localYMDHMS(dateTime, QTimeString::Long) + ", ";
        bodyText += source.from().name() + " wrote:\n> ";

        int pos = bodyText.length();
        bodyText += originalText;
        while ((pos = bodyText.indexOf('\n', pos)) != -1)
            bodyText.insert(++pos, "> ");

        bodyText.append("\n");
    }

    // Whatever text subtype it was before, it's now plain...
    QMailMessageContentType contentType("text/plain; charset=UTF-8");

    if (mail.partCount() == 0) {
        // Set the modified text as the body
        mail.setBody(QMailMessageBody::fromData(bodyText, contentType, QMailMessageBody::Base64));
    } else if (textPart != -1) {
        // Replace the original text with our modified version
        QMailMessagePart& part = mail.partAt(textPart);
        part.setBody(QMailMessageBody::fromData(bodyText, contentType, QMailMessageBody::Base64));
    }

    if (action == ReplyToAll) {
        // Set the reply-to-all address list
        QList<QMailAddress> all;
        foreach (const QMailAddress& addr, source.to() + source.cc())
            if ((addr.address() != fromAddress) && (addr.address() != toAddress))
                all.append(addr);

        QString cc = QMailAddress::toStringList(all).join(", ");
        checkOutlookString( cc );
        ccAddress = cc;
    }

    mail.removeHeaderField("From");
    setMessage( mail );

    if (!toAddress.isEmpty())
        setTo( toAddress );
    if (!subjectText.isEmpty())
        setSubject( subjectText );

    QString task;
    if ((action == Create) || (action == Forward)) {
        task = (action == Create ? tr("Create") : tr("Forward"));
        task += " " + displayName(QMailMessage::Email);
    } else if (action == Reply) {
        task = tr("Reply");
    } else if (action == ReplyToAll) {
        task = tr("Reply to all");
    }
    setContext(task);
}


void EmailComposerInterface::selectAttachments()
{
    if (m_attachments.isEmpty() && m_addAttDialog->documentSelector()->documents().isEmpty()) {
        QMessageBox::warning(this,
                             tr("No documents"),
                             tr("There are no existing documents to attach"),
                             tr("OK") );
    } else {
        if (QtopiaApplication::execDialog(m_addAttDialog) == QDialog::Accepted) {
            m_attachments.clear();
            foreach (const AttachmentItem *item, m_addAttDialog->attachedFiles())
                m_attachments.append(qMakePair(item->document(), item->action()));

            emit attachmentsChanged();
        } else {
            m_addAttDialog->clear();
            foreach (const AttachmentDetail &att, m_attachments)
                m_addAttDialog->attach(att.first, att.second);
        }
    }
}

void EmailComposerInterface::selectRecipients()
{
    static const QString addressSeparator(", ");

    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle(tr("Select Contacts"));

    QVBoxLayout *vbl = new QVBoxLayout(&selectionDialog);
    selectionDialog.setLayout(vbl);

    AddressSelectorWidget* addressSelector= new AddressSelectorWidget(AddressSelectorWidget::EmailSelection, &selectionDialog);
    vbl->addWidget(addressSelector);
    addressSelector->setSelectedAddresses(to().split(addressSeparator,QString::SkipEmptyParts));

    if(QtopiaApplication::execDialog(&selectionDialog) == QDialog::Accepted)
    {
        QStringList selectedAddresses = addressSelector->selectedAddresses();
        m_toEdit->setField(selectedAddresses.join(addressSeparator));
    }
}

void EmailComposerInterface::updateAttachmentsLabel()
{
    int count = 0;
    int sizeKB = 0;

    foreach (const AttachmentItem* item, m_addAttDialog->attachedFiles()) {
        ++count;
        sizeKB += item->sizeKB();
    }

    if (count == 0) {
        m_attachmentsButton->setText("No attachments");
    } else {
        m_attachmentsButton->setText(tr("%n Attachment(s): %1KB","", count).arg(sizeKB));
    }
}

void EmailComposerInterface::setCursorPosition()
{
    if (m_index != -1) {
        QTextCursor cursor(m_bodyEdit->textCursor());
        cursor.setPosition(m_index, QTextCursor::MoveAnchor);
        m_bodyEdit->setTextCursor(cursor);

        m_index = -1;
    }
}

void EmailComposerInterface::subjectClicked()
{
    bool ok = false;
    QString ret = QtopiaInputDialog::getText(this, tr("Subject"),tr("Subject"), QLineEdit::Normal, QtopiaApplication::Words, QString(), m_subjectEdit->field(), &ok);
    if(ok)
        m_subjectEdit->setField(ret);
}

void EmailComposerInterface::toClicked()
{
    bool ok = false;
    QString ret = QtopiaInputDialog::getText(this, tr("To"),tr("To"), QLineEdit::Normal, QtopiaApplication::Words, QString(), m_toEdit->field(), &ok);
    if(ok)
        m_toEdit->setField(ret);

}

void EmailComposerInterface::keyPressEvent( QKeyEvent *e )
{
    static const bool keypadAbsent(style()->inherits("QThumbStyle"));

    if (keypadAbsent) {
        if (e->key() == Qt::Key_Back) {
            e->accept();
            return;
        }
    } else {
        if (e->key() == Qt::Key_Select) {
            if (!isEmpty()) {
                e->accept();
            } else {
                e->ignore();
            }
            return;
        }

        if (e->key() == Qt::Key_Back) {
            if( Qtopia::mousePreferred() ) {
                e->ignore();
                return;
            } else if (m_bodyEdit->toPlainText().isEmpty()) {
                e->accept();
                return;
            }
        }
    }

    QWidget::keyPressEvent( e );
}


void EmailComposerInterface::setDefaultAccount(const QMailAccountId& id)
{
    Q_UNUSED(id);
}

void EmailComposerInterface::setTo(const QString& toAddress)
{
    m_toEdit->setField(toAddress);
}

void EmailComposerInterface::setFrom(const QString& fromAddress)
{
    m_from = fromAddress;
}

void EmailComposerInterface::setSubject(const QString& subject)
{
    m_subjectEdit->setField(subject);
}

QString EmailComposerInterface::from() const
{
    return m_from;
}

QString EmailComposerInterface::to() const
{
    return m_toEdit->field();
}

bool EmailComposerInterface::isReadyToSend() const
{
    return !to().trimmed().isEmpty();
}

bool EmailComposerInterface::isDetailsOnlyMode() const
{
    return !m_bodyEdit->isEnabled();
}

void EmailComposerInterface::setDetailsOnlyMode(bool val)
{
    m_bodyEdit->setEnabled(!val);
}

QString EmailComposerInterface::contextTitle() const
{
    return m_title;
}

QMailAccount EmailComposerInterface::fromAccount() const
{
    return QMailAccount();
}

QTOPIA_EXPORT_PLUGIN( EmailComposerPlugin )

EmailComposerPlugin::EmailComposerPlugin()
    : QMailComposerPlugin()
{
}

QMailComposerInterface* EmailComposerPlugin::create( QWidget *parent )
{
    return new EmailComposerInterface( parent );
}

