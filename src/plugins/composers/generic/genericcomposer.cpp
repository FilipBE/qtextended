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

#include "genericcomposer.h"
#include "templatetext.h"
#include <private/detailspage_p.h>
#include <private/addressselectorwidget_p.h>
#ifdef QTOPIA_HOMEUI
#include <private/qtopiainputdialog_p.h>
#endif
#include <qcollectivenamespace.h>
#include <QAction>
#include <QContact>
#include <QGsmCodec>
#include <QInputContext>
#include <QKeyEvent>
#include <QLabel>
#include <QMailAccount>
#include <QMailMessage>
#include <QMenu>
#include <QSettings>
#include <QSoftMenuBar>
#include <QStackedWidget>
#include <QTextEdit>
#include <QtopiaApplication>
#include <QVBoxLayout>
#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif

#define SMS_CHAR_LIMIT 459

class ComposerTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    ComposerTextEdit( QWidget *parent, const char *name = 0 );

    void limitedInsert( const QString &str);
#ifndef QT_NO_CLIPBOARD
    void limitedPaste();
#endif

    bool isComposing();
    bool isEmpty();

signals:
    void finished();
    void aboutToChange(int charCount) const;

private slots:
    void updateLabel();

protected:
    void keyPressEvent( QKeyEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void inputMethodEvent( QInputMethodEvent *e );
    bool canInsertFromMimeData(const QMimeData* source) const;
    inline bool withinLengthLimit(int charCount) const;
};

ComposerTextEdit::ComposerTextEdit( QWidget *parent, const char *name )
    : QTextEdit( parent )
{
    setObjectName( name );
    setFrameStyle(NoFrame);
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setLineWrapMode( QTextEdit::WidgetWidth );

    connect(document(), SIGNAL(contentsChanged()), this, SLOT(updateLabel()));

    updateLabel();
}

void ComposerTextEdit::updateLabel()
{
    if (isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
    } else {
        if (isComposing())
            QSoftMenuBar::clearLabel(this, Qt::Key_Select);
        else
            QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Next);

        if (Qtopia::mousePreferred())
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
        else
            QSoftMenuBar::clearLabel(this, Qt::Key_Back);
    }
}

void ComposerTextEdit::keyPressEvent( QKeyEvent *e )
{
    int charCount = toPlainText().length();

    QChar c = e->text()[0];
    if((c.isLetterOrNumber() || c.isPunct() || c.isSpace()))
        if(!withinLengthLimit(charCount+1))
            return;

#ifndef QT_NO_CLIPBOARD
    if( e->modifiers() & Qt::ControlModifier )
    {
        if( e->key() == Qt::Key_V )
        {
            limitedPaste();
            return;
        }
        else if( e->key() == Qt::Key_Y )
            return; //redo could be redo paste, and that could exceed limit
    }
#endif
    if (e->key() == Qt::Key_Select) {
        if (charCount == 0) {
            e->ignore();
        } else {
            e->accept();
            emit finished();
        }
        return;
    }

    if (e->key() == Qt::Key_Back) {
        if( Qtopia::mousePreferred() ) {
            e->accept();
            emit finished();
            return;
        } else if (charCount == 0) {
            e->accept();
            emit finished();
            return;
        }
    }
    QTextEdit::keyPressEvent( e );
}

void ComposerTextEdit::limitedInsert( const QString &str)
{
    int curCharCount = toPlainText().length();
    QString strText = str;
    int strCharCount = strText.length();

    while(!withinLengthLimit(curCharCount+strCharCount))
    {
        strText = strText.left( strText.length() -1 );
        strCharCount = strText.length();
    }
    if( !strText.isEmpty() )
    {
        textCursor().insertText( strText );
        ensureCursorVisible();
        emit textChanged();
    }
}

#ifndef QT_NO_CLIPBOARD
void ComposerTextEdit::limitedPaste()
{
    limitedInsert( QApplication::clipboard()->text() );
}
#endif

bool ComposerTextEdit::isComposing()
{
    return (inputContext() && inputContext()->isComposing());
}

bool ComposerTextEdit::isEmpty()
{
    if (!document()->isEmpty())
        return false;

    // Otherwise there may be pre-edit input queued in the input context
    return !isComposing();
}

void ComposerTextEdit::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == Qt::RightButton )
        return;
    QTextEdit::mousePressEvent( e );
}

void ComposerTextEdit::inputMethodEvent( QInputMethodEvent *e )
{
    int charCount = toPlainText().length();
    if(!e->commitString().isEmpty())
    {
        // Clear the commit string if it is makes the message longer
        // than the limit
        if(!withinLengthLimit(charCount+e->commitString().length()))
            e->setCommitString(QString());
    }
    else if (!e->preeditString().isEmpty())
    {
        int proposedLength = e->preeditString().length() + charCount;
        if(withinLengthLimit(proposedLength))
            emit aboutToChange(proposedLength);
    }

    QTextEdit::inputMethodEvent( e );
}

bool ComposerTextEdit::canInsertFromMimeData(const QMimeData* source) const
{
    return withinLengthLimit(source->text().length() + toPlainText().length() - textCursor().selectedText().length());
}

bool ComposerTextEdit::withinLengthLimit(int charCount) const
{
    return charCount <= SMS_CHAR_LIMIT;
}

static void addActionsFromWidget(QWidget* sourceWidget, QMenu* targetMenu)
{
    if(!sourceWidget) return;
    foreach(QAction* a,sourceWidget->actions())
        targetMenu->addAction(a);
}

GenericComposerInterface::GenericComposerInterface( QWidget *parent )
    : QMailComposerInterface( parent ),
    m_widgetStack(0),
    m_composerWidget(0),
    m_textEdit(0),
    m_smsLimitIndicator(0),
    m_vCard( false ),
    m_vCardData(),
    m_type(QMailMessage::Sms)
{
    init();
}

GenericComposerInterface::~GenericComposerInterface()
{
    QSettings cfg("Trolltech","qtmail");
    cfg.beginGroup( "GenericComposer" );
    cfg.setValue( "showSmsLimitIndicator", m_showLimitAction->isChecked() );
}

void GenericComposerInterface::init()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    QWidget::setLayout(layout);

    //widget stack
    m_widgetStack = new QStackedWidget(this);
    m_widgetStack->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(m_widgetStack);

    //composer widget
    m_composerWidget = new QWidget(m_widgetStack);
    m_widgetStack->addWidget(m_composerWidget);

    QSettings cfg("Trolltech","qtmail");
    cfg.beginGroup( "GenericComposer" );


    m_templateTextAction = new QAction( tr("Insert template"), this );
    connect( m_templateTextAction, SIGNAL(triggered()), this, SLOT(templateText()) );

    QVBoxLayout *l = new QVBoxLayout(m_composerWidget);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

#ifdef QTOPIA_HOMEUI
    QHBoxLayout* toLayout = new QHBoxLayout;
    toLayout->setContentsMargins(0, 0, 0, 0);
    toLayout->setSpacing(0);

    m_toEdit = new HomeFieldButton(tr("To:"), m_sizer, true);
    connect(m_toEdit, SIGNAL(clicked()), this, SLOT(recipientsActivated()));
    toLayout->addWidget(m_toEdit);

    m_contactsButton = new HomeActionButton(tr("Contact"), QtopiaHome::Green);
    m_contactsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_contactsButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_contactsButton->setMaximumWidth(45);
    m_contactsButton->setMinimumWidth(45);
    connect(m_contactsButton, SIGNAL(clicked()), this, SLOT(selectRecipients()));
    toLayout->addWidget(m_contactsButton);

    l->addLayout(toLayout);
#endif

    m_smsLimitIndicator = new QLabel(m_composerWidget);
    m_smsLimitIndicator->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->addWidget( m_smsLimitIndicator );

    m_showLimitAction = new QAction( tr("Show SMS Limit"), this );
    m_showLimitAction->setCheckable( true );
    m_showLimitAction->setChecked( cfg.value( "showSmsLimitIndicator", true ).toBool() );
    connect( m_showLimitAction, SIGNAL(triggered(bool)), m_smsLimitIndicator, SLOT(setVisible(bool)) );


    //main text field
    m_textEdit = new ComposerTextEdit(m_composerWidget);
    m_composerWidget->setFocusProxy(m_textEdit);
    l->addWidget( m_textEdit );

    connect(m_textEdit, SIGNAL(aboutToChange(int)), this, SLOT(updateSmsLimitIndicator(int)));
    connect(m_textEdit, SIGNAL(textChanged()),this, SLOT(textChanged()));

#ifdef QTOPIA_HOMEUI
    connect( m_textEdit, SIGNAL(finished()), this, SIGNAL(sendMessage()));
#else
    connect( m_textEdit, SIGNAL(finished()), this, SLOT(detailsPage()));
#endif

    //details widget
    m_detailsWidget = new DetailsPage(m_widgetStack);
    connect( m_detailsWidget, SIGNAL(changed()), this, SIGNAL(changed()));
    connect( m_detailsWidget, SIGNAL(sendMessage()), this, SIGNAL(sendMessage()));
    connect( m_detailsWidget, SIGNAL(cancel()), this, SIGNAL(cancel()));
    connect( m_detailsWidget, SIGNAL(editMessage()),this, SLOT(composePage()));
    m_widgetStack->addWidget(m_detailsWidget);

    //menus
    QMenu* textEditMenu = QSoftMenuBar::menuFor(m_textEdit);
    textEditMenu->addSeparator();
    textEditMenu->addAction(m_showLimitAction);
    textEditMenu->addAction(m_templateTextAction);
    addActionsFromWidget(QWidget::parentWidget(),textEditMenu);

    composePage();
    setContext("Create " + displayName(m_type));
    updateSmsLimitIndicator();
}

QString GenericComposerInterface::text() const
{
    if ( m_vCard )
        return m_vCardData;
    else
        return m_textEdit->toPlainText();
}

void GenericComposerInterface::setContext(const QString& title)
{
    m_title = title;
    emit contextChanged();
}

bool GenericComposerInterface::isEmpty() const
{
    if (m_vCard)
        return m_vCardData.isEmpty();
    else
        return m_textEdit->isEmpty();
}

void GenericComposerInterface::setMessage(const QMailMessage &mail )
{
    setBody( mail.body().data(), mail.headerField("Content-Type").content() );

    //set the details
    m_detailsWidget->setDetails(mail);
    setTo(m_detailsWidget->to());
}

QMailMessage GenericComposerInterface::message() const
{
    QMailMessage mail;

    QMailMessageContentType type( "text/plain; charset=UTF-8" );
    mail.setBody( QMailMessageBody::fromData( text(), type, QMailMessageBody::Base64 ) );

    mail.setMessageType(m_type);
    if (m_type == QMailMessage::Sms) {
        mail.setHeaderField("X-Sms-Type", "normal");
    }
    if (isVCard()) {
        mail.setHeaderField(QLatin1String("Content-Type"), QLatin1String("text/x-vCard"));
    }

    m_detailsWidget->getDetails(mail);

    if (m_type == QMailMessage::Instant) {
        // Turn any addresses that aren't valid for libcollective into jabber addresses
        QList<QMailAddress> recipients;
        foreach (const QMailAddress &addr, mail.to()) {
            if (addr.isChatAddress()) {
                recipients.append(addr);
            } else {
                QString jabberAddress = QCollective::encodeUri("jabber", addr.address());
                if (addr.name() != addr.address()) {
                    recipients.append(QMailAddress(addr.name(), jabberAddress));
                } else {
                    recipients.append(QMailAddress(jabberAddress));
                }
            }
        }

        mail.setTo(recipients);
    }

    return mail;
}

void GenericComposerInterface::clear()
{
    setBody( QString(), QString() );
}

void GenericComposerInterface::setBody( const QString& t, const QString &type )
{
#ifndef QTOPIA_NO_SMS
    if (type.contains(QLatin1String("text/x-vCard"), Qt::CaseInsensitive)) {
        QList<QContact> contacts = QContact::readVCard(t.toLatin1());

        if ( contacts.count() == 0 ) {
            // Invalid VCard data, so just show raw data
            m_textEdit->setPlainText( t );
        } else if ( contacts.count() == 1 ) {
            QString name = tr( "Message contains vCard for %1" );
            if ( !contacts[0].nickname().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].nickname() ) );
            } else if ( !contacts[0].firstName().isEmpty() &&
                        !contacts[0].lastName().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].firstName() +
                                                    " " +
                                                    contacts[0].lastName() ) );
            } else if ( !contacts[0].firstName().isEmpty() ) {
                m_textEdit->setPlainText( name.arg( contacts[0].firstName() ) );
            } else {
                m_textEdit->setPlainText(
                    tr( "Message contains vCard for a contact" ) );
            }
            m_vCard = true;
            m_vCardData = t;
        } else if ( contacts.count() > 1 ) {
            m_textEdit->setPlainText(
                tr( "Message contains vCard for multiple contacts" ) );
            m_vCard = true;
            m_vCardData = t;
        }
    } else
#else
    Q_UNUSED(type);
#endif
    {
        m_textEdit->setPlainText( t );
    }

    // Update GUI state
    m_templateTextAction->setVisible( !m_vCard );
    m_textEdit->setReadOnly( m_vCard );
    m_textEdit->setEditFocus( !m_vCard );
    if ( m_vCard ) {
        setFocusProxy( 0 );
    } else {
        m_textEdit->moveCursor( QTextCursor::End );
        setFocusProxy( m_textEdit );
    }
}

void GenericComposerInterface::setDefaultAccount(const QMailAccountId& id)
{
    m_detailsWidget->setDefaultAccount(id);
}

void GenericComposerInterface::setTo(const QString& toAddress)
{
    QStringList addressList;

    // If the address is a complicated chat address, simplify it for presentation
    foreach (const QMailAddress &addr, QMailAddress::fromStringList(toAddress)) {
        if (addr.isChatAddress()) {
            // Use the decoded form of the address for appearances
            if (addr.name() != addr.address()) {
                addressList.append(QMailAddress(addr.name(), addr.chatIdentifier()).toString());
            } else {
                addressList.append(addr.chatIdentifier());
            }
        } else {
            addressList.append(addr.toString());
        }
    }

    QString addressStr(addressList.join(", "));
    m_detailsWidget->setTo(addressStr);
#ifdef QTOPIA_HOMEUI
    m_toEdit->setField(addressStr);
#endif
}

void GenericComposerInterface::setFrom(const QString& fromAddress)
{
    m_detailsWidget->setFrom(fromAddress);
}

void GenericComposerInterface::setSubject(const QString& subject)
{
    m_detailsWidget->setSubject(subject);
}

void GenericComposerInterface::setMessageType(QMailMessage::MessageType type)
{
    m_type = type;
    m_detailsWidget->setType(m_type);
}

QString GenericComposerInterface::from() const
{
    return m_detailsWidget->from();
}

QString GenericComposerInterface::to() const
{
    return m_detailsWidget->to();
}

bool GenericComposerInterface::isReadyToSend() const
{
    return !to().trimmed().isEmpty();
}

bool GenericComposerInterface::isDetailsOnlyMode() const
{
    return m_detailsWidget->isDetailsOnlyMode();
}

void GenericComposerInterface::setDetailsOnlyMode(bool val)
{
    m_detailsWidget->setDetailsOnlyMode(val);
    if(val)
        detailsPage();
}

QString GenericComposerInterface::contextTitle() const
{
    return m_title;
}

QMailAccount GenericComposerInterface::fromAccount() const
{
    return m_detailsWidget->fromAccount();
}

void GenericComposerInterface::attach( const QContent &, QMailMessage::AttachmentsAction )
{
    qWarning("Unimplemented function called %s %d, %s", __FILE__, __LINE__, __FUNCTION__ );
}

void GenericComposerInterface::reply(const QMailMessage& source, int action)
{
    QString toAddress;
    QMailMessage mail;

    if (action == Forward) {
        QString bodyText;
        if (source.status() & QMailMessage::Incoming)  {
            bodyText = source.from().displayName();
            bodyText += ":\n\"";
            bodyText += source.body().data();
            bodyText += "\"\n--\n";
        } else {
            bodyText += source.body().data();
        }

        QMailMessageContentType contentType("text/plain; charset=UTF-8");
        mail.setBody(QMailMessageBody::fromData(bodyText, contentType, QMailMessageBody::Base64));
        setMessage(mail);
    } else {
        QMailAddress replyAddress(source.replyTo());
        if (replyAddress.isNull())
            replyAddress = source.from();

        toAddress = replyAddress.address();

        clear();
    }
    if (!toAddress.isEmpty())
        setTo( toAddress );

    QString task;
    if ((action == Create) || (action == Forward)) {
        task = (action == Create ? tr("Create") : tr("Forward"));
        task += " " + displayName(m_type);
    } else if (action == Reply) {
        task = tr("Reply");
    } else if (action == ReplyToAll) {
        task = tr("Reply to all");
    }
    setContext(task);
}

void GenericComposerInterface::updateSmsLimitIndicator(int length)
{
    static bool rtl = QApplication::isRightToLeft();

    int charCount = length == 0 ? m_textEdit->toPlainText().length() : length;
    int remaining = SMS_CHAR_LIMIT - charCount;

#ifndef QTOPIA_NO_SMS
    int numMessages = smsCountInfo();
#else
    int numMessages = 1;
#endif

    QString info = tr("%1/%2","e.g. 5/7").arg( rtl ? numMessages : remaining )
        .arg( rtl ? remaining : numMessages );
    m_smsLimitIndicator->setText( ' ' + info + ' ' );
}

void GenericComposerInterface::textChanged()
{
    if(!m_textEdit->inputContext()->isComposing())
    {
        updateSmsLimitIndicator();
        emit changed();
    }
}

void GenericComposerInterface::templateText()
{
    TemplateTextDialog *templateTextDialog = new TemplateTextDialog( this, "template-text" );
    QtopiaApplication::execDialog( templateTextDialog );

    ComposerTextEdit *composer = qobject_cast<ComposerTextEdit *>( m_textEdit );
    if (templateTextDialog->result() && composer)
        composer->limitedInsert( templateTextDialog->text() );
    delete templateTextDialog;
}

#ifndef QTOPIA_NO_SMS
//Qtmail sends either GSM or UCS2 encoded SMS messages
//If another encoding is ultimately used to send the message,
//these functions will return inaccurate results
void GenericComposerInterface::smsLengthInfo(uint& estimatedBytes, bool& isUnicode)
{
    //calculate the number of consumed bytes
    //considering the gsm charset

    unsigned short c;
    unsigned short d;
    uint count = 0;
    QString unicodestr = text();
    for(int i = 0; i < unicodestr.length(); ++i)
    {
        c = unicodestr[i].unicode();
        if(c >= 256)
        {
            estimatedBytes = unicodestr.length() * 2;
            isUnicode = true;
            return;
        }
        else
        {
            d = QGsmCodec::twoByteFromUnicode(c);
            if(d >= 256)
                count += 2;
            else if(d == 0x10) //0x10 is unrecognised char
            {
                estimatedBytes = unicodestr.length() * 2; //non gsm char, so go unicode
                isUnicode = true;
                return;
            }
            else
                count += 1;
        }
    }
    isUnicode = false;
    estimatedBytes = count;
}
//estimates the number of messages that will be sent

int GenericComposerInterface::smsCountInfo()
{
    bool isUnicode = false;
    uint numBytes = 0;
    int numMessages = 0;
    int len = text().length();

    smsLengthInfo(numBytes,isUnicode);

    if(isUnicode) //all 2 byte UCS2 so ok to use text length
    {
        if (len <= 70 ) {
            numMessages = 1;
        } else {
            // 67 = 70 - fragment_header_size (3).
            numMessages = ( len + 66 ) / 67;
        }
    }
    else
    {
        //use byte length instead of text length
        //as some GSM chars consume 2 bytes
        if ( numBytes <= 160 ) {
            numMessages = 1;
        } else {
        // 153 = 160 - fragment_header_size (7).
            numMessages = ( numBytes + 152 ) / 153;
        }
    }
    return numMessages;
}
#endif

void GenericComposerInterface::detailsPage()
{
    if (isEmpty() && !isDetailsOnlyMode()) {
        emit cancel();
    } else {
        m_widgetStack->setCurrentWidget(m_detailsWidget);
        QWidget::setFocusProxy(m_detailsWidget);
        setContext(displayName(m_type) + " " + tr("details"));
    }
}

void GenericComposerInterface::composePage()
{
    m_widgetStack->setCurrentWidget(m_composerWidget);
    QWidget::setFocusProxy(m_textEdit);
}

#ifdef QTOPIA_HOMEUI
void GenericComposerInterface::selectRecipients()
{
    static const QString addressSeparator(", ");

    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle(tr("Select Contacts"));

    QVBoxLayout *vbl = new QVBoxLayout(&selectionDialog);
    selectionDialog.setLayout(vbl);

    AddressSelectorWidget* addressSelector= new AddressSelectorWidget(AddressSelectorWidget::InstantMessageSelection, &selectionDialog);
    vbl->addWidget(addressSelector);

    // For instant messages, pass only the actual address elements to the selector
    QStringList addressList;
    foreach (const QMailAddress &addr, QMailAddress::fromStringList(to()))
        addressList.append(addr.address());
    addressSelector->setSelectedAddresses(addressList);

    if (QtopiaApplication::execDialog(&selectionDialog) == QDialog::Accepted) {
        setTo(addressSelector->selectedAddresses().join(addressSeparator));
    }
}

void GenericComposerInterface::recipientsActivated()
{
    bool ok = false;
    QString ret = QtopiaInputDialog::getText(this, tr("To"), tr("To"), QLineEdit::Normal, QtopiaApplication::Words, QString(), m_toEdit->field(), &ok);
    if (ok) {
        // Create valid jabber addresses from this input
        QStringList addresses;
        foreach (const QString &portion, ret.split(","))
            addresses.append(QCollective::encodeUri("jabber", portion.trimmed()));

        setTo(addresses.join(", "));
    }
}
#endif


QTOPIA_EXPORT_PLUGIN( GenericComposerPlugin )

GenericComposerPlugin::GenericComposerPlugin()
    : QMailComposerPlugin()
{
}

QMailComposerInterface* GenericComposerPlugin::create( QWidget *parent )
{
    return new GenericComposerInterface( parent );
}

#include "genericcomposer.moc"

