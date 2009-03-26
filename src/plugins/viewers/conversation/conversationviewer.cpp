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

#include "conversationviewer.h"
#include "conversationdelegate.h"

#include <private/accountconfiguration_p.h>
#ifdef QTOPIA_HOMEUI
#  include <private/homewidgets_p.h>
#  include <private/qtopiainputdialog_p.h>
#endif

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMailFolder>
#include <QMailAccount>
#include <QMailMessage>
#include <QMailMessageListModel>
#include <QMailTimeStamp>
#include <QPushButton>
#include <QSmoothList>
#include <QTimer>
#include <QToolButton>
#include <QtopiaApplication>
#include <QVBoxLayout>

#include <qtopialog.h>


static const int MaximumInstantSubjectLength = 256;

ConversationViewer::ConversationViewer(QWidget* parent)
    : QMailViewerInterface(parent),
      listView(new QSmoothList(parent)),
#ifdef QTOPIA_HOMEUI
      mainWidget(new QWidget(parent)),
      fromButton(new HomeContactButton(tr("From:"), sizer)),
      replyButton(new HomeActionButton(tr("Reply"), QtopiaHome::Green)),
      deleteButton(new HomeActionButton(tr("Delete"), QtopiaHome::Red)),
      backButton(new HomeActionButton(tr("Back"), listView->palette().color(QPalette::Button), listView->palette().color(QPalette::ButtonText))),
#endif
      model(new QMailMessageListModel(parent)),
      currentIndex(-1)
{
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));

    listView->setModel(model);
    listView->setItemDelegate(new ConversationDelegate(parent));

    listView->setVisible(false);

    updateBackground();

    connect(listView, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    connect(listView, SIGNAL(clicked(QModelIndex)), this, SLOT(messageActivated(QModelIndex)));

#ifdef QTOPIA_HOMEUI
    QSizePolicy expandingPolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSizePolicy minimumPolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    fromButton->setSizePolicy(expandingPolicy);
    connect(fromButton, SIGNAL(clicked()), this, SLOT(senderActivated()));

    replyButton->setSizePolicy(minimumPolicy);
    connect(replyButton, SIGNAL(clicked()), this, SLOT(sendReply()));

    deleteButton->setSizePolicy(minimumPolicy);
    connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMessage()));

    backButton->setSizePolicy(minimumPolicy);
    connect(backButton, SIGNAL(clicked()), this, SIGNAL(finished()));

    QHBoxLayout *hb = new QHBoxLayout;
    hb->setSpacing(0);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->addWidget(fromButton);
    hb->addWidget(replyButton);
    hb->addWidget(deleteButton);
    hb->addWidget(backButton);

    QVBoxLayout* vb = new QVBoxLayout(mainWidget);
    vb->setSpacing(0);
    vb->setContentsMargins(0, 0, 0, 0);
    vb->addLayout(hb);
    vb->addWidget(listView);
#endif

    widget()->installEventFilter(this);
}

ConversationViewer::~ConversationViewer()
{
}

void ConversationViewer::scrollToAnchor(const QString& a)
{
    Q_UNUSED(a)
}

QWidget* ConversationViewer::widget() const
{
#ifdef QTOPIA_HOMEUI
    return mainWidget;
#else
    return listView;
#endif
}

void ConversationViewer::addActions(QMenu* menu) const
{
    Q_UNUSED(menu)
}

bool ConversationViewer::handleIncomingMessages(const QMailMessageIdList &list) const
{
    if (list.isEmpty() || qApp->activeWindow() == 0)
        return false;

    // If all of these messages match the current conversation, then we'll handle
    // the event by updating the view
    foreach (const QMailMessageId &id, list) {
        QModelIndex idx = model->indexFromId(id);
        if (!idx.isValid())
            return false;
    }

    // There are no new messages that are not elements of the conversation; 
    // set the highlight to the most recent addition
    listView->setCurrentIndex(model->indexFromId(list.first()));
    return true;
}

bool ConversationViewer::handleOutgoingMessages(const QMailMessageIdList &list) const
{
    if (list.isEmpty())
        return false;

    // If all of these messages match the current conversation, then we'll handle
    // the event by updating the view
    foreach (const QMailMessageId &id, list) {
        QModelIndex idx = model->indexFromId(id);
        if (!idx.isValid())
            return false;
    }

    // There are no new messages that are not elements of the conversation
    return true;
}

bool ConversationViewer::setMessage(const QMailMessage& msg)
{
    if (messageId == msg.id()) {
        // We're already viewing this message - we don't need to reload it,
        // since the model will have emitted dataChanged for the relevant row
        return true;
    }

    clear();

    messageId = msg.id();

    QMailAddress conversationalist;

    if (msg.status() & QMailMessage::Incoming) {
        // Include any messages from the sender of this message
        conversationalist = msg.from();
    } else if (msg.status() & QMailMessage::Outgoing) {
        if (!msg.to().isEmpty()) {
            // Although we may have sent this message to multiple recipients, we will 
            // just use the first recipient as our conversationalist
            conversationalist = msg.to().first();
        }
    }

    if (!conversationalist.isNull()) {
        // See if we have a contact matching this address
        contact = QContact(conversationalist.matchContact());
#ifdef QTOPIA_HOMEUI
        if (!contact.uid().isNull()) {
            fromButton->setValues(contact, contact.label());
        } else {
            fromButton->setValues(contact, conversationalist.displayName());
        }
#endif

        // Include any messages to/from the other party (using this address)
        QMailMessageKey incomingKey(QMailMessageKey::Sender, conversationalist.address());

        QMailMessageKey outgoingKey(QMailMessageKey::Recipients, conversationalist.address(), QMailDataComparator::Includes);
        QMailMessageKey sentKey(QMailMessageKey::Status, QMailMessage::Sent, QMailDataComparator::Includes);
        QMailMessageKey sendingKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::OutboxFolder));
        outgoingKey &= (sentKey | sendingKey);

        // Exclude messages moved to Trash, unless this message is in the trash
        QMailMessageKey trashKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::TrashFolder));
        if (msg.parentFolderId() != QMailFolderId(QMailFolder::TrashFolder))
            trashKey = ~trashKey;

        model->setKey((incomingKey | outgoingKey) & trashKey);
        model->setSortKey(QMailMessageSortKey(QMailMessageSortKey::TimeStamp));

        QModelIndex idx = model->indexFromId(messageId);
        if (idx.isValid())
            listView->setCurrentIndex(idx);
    } else {
        contact = QContact();
        model->setKey(QMailMessageKey());
    }

    listView->setVisible(true);

    return true;
}

void ConversationViewer::setResource(const QUrl& name, QVariant var)
{
    Q_UNUSED(name)
    Q_UNUSED(var)
}

void ConversationViewer::clear()
{
    messageId = QMailMessageId();
    contact = QContact();
}

void ConversationViewer::action(QAction* action)
{
    Q_UNUSED(action)
}

void ConversationViewer::linkClicked(const QUrl& url)
{
    Q_UNUSED(url)
}

bool ConversationViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == widget()) {
        if (event->type() == QEvent::KeyPress) {
            if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
                if (keyEvent->key() == Qt::Key_Back) {
                    emit finished();
                    return true;
                } else if (keyEvent->key() == Qt::Key_Select) {
                    // Not sure why this is necessary...
                    messageActivated(listView->currentIndex());
                    return true;
                }
            }
        } else if (event->type() == QEvent::Show) {
            if (listView->currentIndex().isValid()) {
                // Our current selection may have changed while hidden, which is now relevant
                emit messageChanged(model->idFromIndex(listView->currentIndex()));
            }
        }
    }

    return false;
}

#ifdef QTOPIA_HOMEUI
void ConversationViewer::senderActivated()
{
    if (!contact.uid().isNull()) {
        emit contactDetails(contact);
    } else {
        emit saveSender();
    }
}
#endif

void ConversationViewer::rowsInserted(const QModelIndex &, int start, int end)
{
    // Automatically highlight new outgoing additions
    for ( ; end >= start; --end) {
        QMailMessageId id(model->idFromIndex(model->index(end, 0)));

        if (id.isValid()) {
            QMailMessageMetaData metaData(id);
            if (metaData.status() & QMailMessage::Outgoing) {
                // Ensure that the list has processed this signal before we try to modify it
                currentIndex = end;
                QTimer::singleShot(0, this, SLOT(setCurrentIndex()));
                break;
            }
        }
    }
}

void ConversationViewer::setCurrentIndex()
{
    if (currentIndex != -1) {
        QModelIndex index(model->index(currentIndex, 0));
        if (index.isValid())
            listView->setCurrentIndex(index);

        currentIndex = -1;
    }
}

void ConversationViewer::currentChanged(const QModelIndex &newIndex, const QModelIndex &)
{
    if (newIndex.isValid()) {
        QMailMessageId currentId(model->idFromIndex(newIndex));
        if (currentId.isValid())
            emit messageChanged(currentId);
    }
}

void ConversationViewer::messageActivated(const QModelIndex &index)
{
    if (index.isValid()) {
        QMailMessageId activatedId(model->idFromIndex(index));
        if (activatedId.isValid())
            emit viewMessage(activatedId, QMailViewerFactory::StandardPresentation);
    }
}

void ConversationViewer::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange)
        updateBackground();
}

void ConversationViewer::updateBackground()
{
    QPalette pal(listView->palette());
    QLinearGradient grad(0, 0, 1, 0);
    grad.setCoordinateMode(QGradient::StretchToDeviceMode);
    grad.setColorAt(0.0, pal.color(QPalette::Base).darker());
    grad.setColorAt(0.5, pal.color(QPalette::Base));
    grad.setColorAt(1.0, pal.color(QPalette::Base).darker());
    pal.setBrush(QPalette::Window, grad);
    listView->setPalette(pal);
}

void ConversationViewer::sendReply()
{
#ifdef QTOPIA_HOMEUI
    // Create a message and send directly (note this code is repeated from 
    // genericcomposer.cpp and detailspage.cpp, and should be kept in sync)

    QMailMessageId currentId;
    if (listView->currentIndex().isValid())
        currentId = model->idFromIndex(listView->currentIndex());
    else 
        currentId = messageId;

    if (currentId.isValid()) {
        QMailMessage reply;
        reply.setMessageType(QMailMessage::Instant);

        // Find the address we're replying to
        QMailMessageMetaData metaData(currentId);
        if (metaData.status() & QMailMessage::Incoming) {
            reply.setTo(metaData.from());
        } else {
            reply.setTo(metaData.to().first());
        }

        // Find the address we're replying from
        QMailAccount account(metaData.parentAccountId());
        if (account.id().isValid()) {
            AccountConfiguration config(account.id());
            reply.setFrom(QMailAddress(config.userName(), config.emailAddress()));
            reply.setParentAccountId(account.id());
    
            // Get the reply text from the user
            bool ok = false;
            QString text = QtopiaInputDialog::getMultiLineText(widget(), tr("Reply"), QString(), 
                                                               QtopiaApplication::Words, QString(), QString(), &ok);
            if (ok && !text.isEmpty()) {
                QMailMessageContentType type("text/plain; charset=UTF-8");
                reply.setBody(QMailMessageBody::fromData(text, type, QMailMessageBody::Base64));

                // If the message is small and textual, keeping a copy in the subject will save us loading the body
                if (text.length() > MaximumInstantSubjectLength) {
                    // Append elipsis character
                    reply.setSubject(text.left(MaximumInstantSubjectLength) + QChar(0x2026));
                    reply.appendHeaderField("X-qtopia-internal-truncated-subject", "true");
                } else {
                    reply.setSubject(text);
                }

                reply.setDate(QMailTimeStamp::currentDateTime());
                reply.setStatus(QMailMessage::Outgoing, true);
                reply.setStatus(QMailMessage::Downloaded, true);
                reply.setStatus(QMailMessage::Read,true);

                emit sendMessage(reply);
            }       
        } else {
            qLog(Messaging) << "Unable to find valid account to reply from!";
        }
    } else {
        qLog(Messaging) << "Unable to find current message to reply to!";
    }
#endif
}


QTOPIA_EXPORT_PLUGIN( ConversationViewerPlugin )

ConversationViewerPlugin::ConversationViewerPlugin()
    : QMailViewerPlugin()
{
}

QString ConversationViewerPlugin::key() const
{
    return "ConversationViewer";
}

bool ConversationViewerPlugin::isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const
{
    if ((pres != QMailViewerFactory::AnyPresentation) && (pres != QMailViewerFactory::ConversationPresentation))
        return false;

    return (type == QMailMessage::PlainTextContent);
}

QMailViewerInterface* ConversationViewerPlugin::create( QWidget *parent )
{
    return new ConversationViewer( parent );
}

