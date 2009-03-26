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

#ifndef EMAILCLIENT_H
#define EMAILCLIENT_H

#include "editaccount.h"
#include "maillist.h"
#include "messagelistview.h"

#include <QMailStore>
#include <QMailMessageServer>
#include <QMailServiceAction>
#include <QMailNewMessageHandler>
#include <QMailViewerFactory>

#include <QContent>
#include <QList>
#include <QMainWindow>
#include <QStack>
#include <QTime>
#include <QTimer>
#include <QValueSpaceItem>

class MessageFolder;
class MessageStore;
class ActionFolderModel;
class ActionFolderView;
class EmailFolderModel;
class EmailFolderView;
class SearchView;
class UILocation;
class ReadMail;
class WriteMail;
class NewMessagesDialog;
class SearchProgressDialog;
class QAction;
class QDSActionRequest;
class QMailAccount;
class QMailMessageSet;
class QMailRetrievalAction;
class QMailSearchAction;
class QMailTransmitAction;
class QStackedWidget;
class QStringList;

class MessageUiBase : public QWidget
{
    Q_OBJECT

public:
    enum Location { NoLocation = 0, ActionList, FolderList, MessageList, SearchResults, Viewer, Composer };

    MessageUiBase(QWidget* parent);
    virtual ~MessageUiBase() {}

    Location currentLocation() const;
    bool locationIncludes(Location loc) const;

signals:
    void raiseWidget(QWidget *, const QString &);

    void statusVisible(bool);
    void updateStatus(const QString&);
    void updateProgress(uint, uint);
    void clearStatus();

protected:
    void viewActionList(const QString& title = QString());
    void viewFolderList(const QString& title = QString());
#ifdef QTOPIA_HOMEUI
    void viewMessageList(const MessageListView::DisplayMode& mode, const QString& title = QString());
#else
    void viewMessageList(const QMailMessageKey& filter, const QString& title = QString());
#endif
    void viewSearchResults(const QMailMessageKey& filter, const QString& title = QString());
    void viewMessage(const QMailMessageId& id, bool permitNavigation, const QString& title = QString());
    void viewComposer(const QString& title = QString());

    void clearLocationStack();

protected:
    WriteMail* writeMailWidget() const;
    ReadMail* readMailWidget() const;

    EmailFolderView* folderView() const;
    MessageListView* messageListView() const;
    ActionFolderView* actionView() const;

    MessageStore* messageStore() const;

    EmailFolderModel* emailFolderModel() const;
    ActionFolderModel* actionFolderModel() const;

    void pushLocation(const UILocation& location);
    void popLocation();
    void displayLocation(const UILocation& location);

    void suspendMailCounts();
    void resumeMailCounts();

    virtual void contextStatusUpdate();

    virtual void showActionStatus(QMailMessageSet* item);
    virtual void showFolderStatus(QMailMessageSet* item);

    virtual void setMarkingMode(bool set);

    virtual void clearStatusText();

    virtual WriteMail* createWriteMailWidget();
    virtual ReadMail* createReadMailWidget();

    virtual EmailFolderView* createFolderView();
    virtual MessageListView* createMessageListView();
    virtual ActionFolderView* createActionView();

    virtual MessageStore* createMessageStore();

    virtual EmailFolderModel* createEmailFolderModel();
    virtual ActionFolderModel* createActionFolderModel();

protected slots:
    virtual void leaveLocation();
    virtual void showMessageStatus();
    virtual void messageSelectionChanged();
    virtual void presentMessage(const QMailMessageId &, QMailViewerFactory::PresentationType);
#ifdef QTOPIA_HOMEUI
    virtual void displayModeChanged(const MessageListView::DisplayMode&);
#endif

    // Slots to be implemented by subclass
    virtual void actionSelected(QMailMessageSet*) = 0;
    virtual void folderSelected(QMailMessageSet*) = 0;
#ifndef QTOPIA_HOMEUI
    virtual void actionActivated(QMailMessageSet*) = 0;
    virtual void folderActivated(QMailMessageSet*) = 0;
#endif //QTOPIA_HOMEUI

    virtual void emailActivated() = 0;
    virtual void composeActivated() = 0;

    virtual void emptyTrashFolder() = 0;
    virtual void messageActivated() = 0;

    virtual void viewNextMessage() = 0;
    virtual void viewPreviousMessage() = 0;

    virtual void allWindowsClosed() = 0;

private:
    void showActionList(const QString&);
    void showFolderList(const QString& title);
#ifdef QTOPIA_HOMEUI
    void showMessageList(const MessageListView::DisplayMode& mode, const QString&);
#endif //QTOPIA_HOMEUI
    void showMessageList(const QMailMessageKey&, const QString&);
    void showMessageList(bool, const QString&);
    void showViewer(const QMailMessageId&, bool, const QString&);
    void showComposer(const QString&);

    void showWidget(QWidget*, const QString& = QString());

    void setCurrentMailboxWidget(QWidget *widget);

protected:
    QStackedWidget *mailboxView;

    QString appTitle;
    bool suspendMailCount;
    bool markingMode;
    QMailMessageId selectedMessageId;
    int selectionCount;
    bool actionCountSuspended;
    bool emailCountSuspended;
};


class EmailClient : public MessageUiBase
{
    Q_OBJECT

public:
    EmailClient(QWidget* parent);
    ~EmailClient();

    void raiseApplication();
    bool cleanExit(bool force);
    bool closeImmediately();

public slots:
    void sendMessageTo(const QMailAddress &address, QMailMessage::MessageType type);

protected:
    void showEvent(QShowEvent* e);

protected slots:
    void cancelOperation();

    void noSendAccount(QMailMessage::MessageType);
    void enqueueMail(const QMailMessage&);
    void saveAsDraft(const QMailMessage&);
    void discardMail();

    void sendAllQueuedMail(bool userRequest = false);
    void sendSingleMail(const QMailMessageMetaData& message);

    void getSingleMail(const QMailMessageMetaData& message);
    void messageActivated();
    void acknowledgeMessageArrivals();
    void emptyTrashFolder();

    bool confirmDeleteWithoutSIM(int);

    void moveSelectedMessages();
    void copySelectedMessages();
    void restoreSelectedMessages();
    void deleteSelectedMessages();

    void moveSelectedMessagesTo(MessageFolder* destination);
    void copySelectedMessagesTo(MessageFolder* destination);

    void selectAll();

    void moveMailFront(const QMailMessageMetaData& message);

    void connectivityChanged(QMailServiceAction::Connectivity connectivity);
    void activityChanged(QMailServiceAction::Activity activity);
    void statusChanged(const QMailServiceAction::Status &status);
    void progressChanged(uint progress, uint total);

    void transmitCompleted();
    void retrievalCompleted();
    void searchCompleted();

    void leaveLocation();

    void actionSelected(QMailMessageSet*);
    void folderSelected(QMailMessageSet*);
#ifndef QTOPIA_HOMEUI    
    void actionActivated(QMailMessageSet*);
    void folderActivated(QMailMessageSet*);
#endif //QTOPIA_HOMEUI 

    void emailActivated();
    void composeActivated();

    void getAllNewMail();
    void getAccountMail();
    void getNewMail();

    void synchronizeFolder();

    void updateAccounts();

    void transferFailure(const QMailAccountId& accountId, const QString&, int);

    void setStatusText(QString &);

    void search();
    void searchRequested();

    void automaticFetch();

    void externalEdit(const QString &);

    void resend(const QMailMessage& message, int);
    void modify(const QMailMessage& message);

    void replyToMessage(const QMailMessageId& id);

    bool moveMessage(const QMailMessageId& id, MessageFolder *target);
    bool copyMessage(const QMailMessageId& id, MessageFolder *target);
    bool removeMessage(const QMailMessageId& id, bool userRequest);

    void readReplyRequested(const QMailMessageMetaData&);

    void settings();
    void deleteAccount(const QMailAccountId& id);

    void accountsAdded(const QMailAccountIdList& ids);
    void accountsRemoved(const QMailAccountIdList& ids);
    void accountsUpdated(const QMailAccountIdList& ids);

    void messagesAdded(const QMailMessageIdList& ids);
    void messagesUpdated(const QMailMessageIdList& ids);

    void messageSelectionChanged();

    void viewNextMessage();
    void viewPreviousMessage();

    void allWindowsClosed();

    void planeModeChanged();
    void jabberStateChanged();

private slots:
    void delayedInit();
    void openFiles();
    void initActions();
    void updateActions();
    void smsVCard(const QDSActionRequest& request);
    void smsVCard(const QString&, const QString&);
    void emailVCard(const QDSActionRequest& request);
    void emailVCard(const QString&, const QString&);
    void newMessages(bool);
    void abortViewNewMessages();
    void newMessageAction(int);
    void viewMessages();
    void viewEmails();
    void composeMessage(QMailMessage::MessageType, 
                        const QMailAddressList&, 
                        const QString&, 
                        const QString&, 
                        const QContentList&, 
                        QMailMessage::AttachmentsAction,
                        bool detailsOnly = false);
    void composeMessage(const QMailMessage&);
    void writeSms(const QString&, const QString&, const QString&);
    void writeInstantMessage(const QString&);
    void writeMailAction(const QString& name, const QString& email);
    void writeMailAction(const QString &name, const QString &addrStr, const QStringList &docAttachments, const QStringList &fileAttachments);
    void markMessages();
    void displayMessage(const QMailMessageId& id);
    void cleanupMessages(const QDate& removalDate, int removalSize);
    void resumeInterruptedComposition();
    void newCountChanged(uint);
    void processArrivedMessages();

private:
    EmailFolderView* createFolderView();
    MessageListView* createMessageListView();
    ActionFolderView* createActionView();

    void init();
    void userInvocation();

    void mailPreviewed(const QMailMessageMetaData&);
    void mailArrived(const QMailMessageMetaData&);

    void previewRetrievalCompleted();
    void completionRetrievalCompleted();

    void mailResponded();

    void closeAfterTransmissionsFinished();
    bool isTransmitting();
    bool isSending();
    bool isRetrieving();

    bool checkMailConflict(const QString& msg1, const QString& msg2);
    void writeMailAction(const QMap<QString, QString> map );
    void writeSmsAction(const QString& name, const QString& email,
                        const QString& body = QString(), bool vcard = false);
    void writeMessageAction(const QString &name, 
                            const QString &addrStr,
                            const QStringList &docAttachments, 
                            const QStringList &fileAttachments,
                            int type = QMailMessage::Sms | QMailMessage::Mms | QMailMessage::Email );
    void promptNewMessageView(bool respondingToRaise);
    void viewNewMessages(bool respondingToRaise);
    void ignoreNewMessages();
    int newMessageCount(QMailMessage::MessageType type);
    void setNewMessageCount(QMailMessage::MessageType type, uint);
    void newMessageArrival();
#ifdef QTOPIA_HOMEUI
    void viewAllMessages();
#else
    void viewInbox(bool email);
#endif
    void emailVCard(const QByteArray& data);

    void readSettings();
    bool saveSettings();

    void displayCachedMail();

    void accessError(const MessageFolder &box);
    void copyError(const MessageFolder &dest);
    void moveError(const MessageFolder &dest);

    void getNextNewMail();
    bool verifyAccounts(bool outgoing);

    void addMailToDownloadList(const QMailMessageMetaData& message);

    void setSendingInProgress(bool y);
    void setRetrievalInProgress(bool y);

    void transferStatusUpdate(int status);
    void setSuspendPermitted(bool y);
    void clearOutboxFolder();

    void updateGetMailButton();
    void updateGetAccountButton();

    QString mailType(QMailMessage::MessageType type);

    void setActionVisible(QAction*, bool);

    bool copyMessages(const QMailMessageIdList& ids, MessageFolder* target);
    bool moveMessages(const QMailMessageIdList& ids, MessageFolder* target);
    bool restoreMessages(const QMailMessageIdList& ids, MessageFolder*);
    bool deleteMessages(const QMailMessageIdList& ids, MessageFolder*);

    bool foreachListElement(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*), 
                            const QMailMessageIdList& list, MessageFolder *target);

    bool foreachListBatch(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*), 
                          const QMailMessageIdList& list, MessageFolder *target, int batchSize);

    bool applyToList(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*), 
                     const QMailMessageIdList& list, MessageFolder *target = 0);

    void contextStatusUpdate();

    void setMarkingMode(bool set);

    MessageFolder* containingFolder(const QMailMessageId& id);

    bool applyToSelectedFolder(void (EmailClient::*function)(MessageFolder*));

    void sendFailure(const QMailAccountId &accountId);
    void receiveFailure(const QMailAccountId &accountId);

    void closeApplication();

    QMailAccountIdList emailAccounts() const;

    SearchProgressDialog* searchProgressDialog() const;

    void clearNewMessageStatus(const QMailMessageKey& key);

private:
    // Whether the initial action for the app was to view incoming messages 
    enum InitialAction { None = 0, IncomingMessages, NewMessages, View, Compose, Cleanup };

    bool filesRead;
    QMailMessageId cachedDisplayMailId;

    int transferStatus;
    bool suspendStatus;
    int primaryActivity;
    int retrievalPhase;

    uint totalSize;
    QMailAccountId defaultAccountId;

    MailList mailDownloadList;
    QMailAccountId mailAccountId;

    QAction *getMailButton;
    QAction *getAccountButton;
    QAction *composeButton;
    QAction *searchButton;
    QAction *cancelButton;
    QAction *synchronizeAction;
    QAction *settingsAction;
    QAction *emptyTrashAction;
    QAction *deleteMailAction;
    QAction *markAction;
    bool enableMessageActions;

    QMenu *actionContext;
    QMenu *folderContext;
    QMenu *messageContext;

    bool actionContextPrepared;
    bool folderContextPrepared;
    bool messageContextPrepared;

    QAction *moveAction;
    QAction *copyAction;
    QAction *restoreAction;
    QAction *selectAllAction;
    bool closeAfterTransmissions;
    bool closeAfterWrite;

    QTimer fetchTimer;
    bool autoGetMail;

    QMailMessageId repliedFromMailId;
    quint64 repliedFlags;

    QList<int> queuedAccountIds;

    QValueSpaceItem planeMode;
    QValueSpaceItem smsReady;
    QValueSpaceItem jabberState;

    bool jabberRaiseAttempted;

    NewMessagesDialog* newMessagesBox;
    QTimer newMessageResponseTimer;
    int ignoredMessageCount;

    InitialAction initialAction;

    QMap<QAction*, bool> actionVisibility;

    SearchView *searchView;
    int preSearchWidgetId;

    QSet<QMailFolderId> locationSet;

    QMailMessageCountMap newMessageCounts;

    QMailAccountIdList pendingAccountIds;

    bool pendingSmtpAccounts;

    QStack<QMailMessageId> flashMessageIds;
    QMailMessageId lastDraftId;

    QMailMessageIdList arrivedMessageIds;

    QMailTransmitAction *transmitAction;
    QMailRetrievalAction *retrievalAction;
    QMailSearchAction *searchAction;

    QMailNewSmsHandler *newSmsHandler;
    QMailNewMmsHandler *newMmsHandler;
    QMailNewEmailHandler *newEmailHandler;
    QMailNewInstantMessageHandler *newInstantMessageHandler;
    QMailNewSystemMessageHandler *newSystemMessageHandler;

    QSet<QMailNewMessageHandler*> pendingHandlers;
};

#endif
