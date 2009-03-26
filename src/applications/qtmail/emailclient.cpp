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

#include "qtmailwindow.h"
#include "emailclient.h"
#include "selectfolder.h"
#include "accountsettings.h"
#include "emailpropertysetter.h"
#include "readmail.h"
#include "writemail.h"
#include "messagefolder.h"
#include "messagestore.h"
#include "actionfoldermodel.h"
#include "actionfolderview.h"
#include "emailfoldermodel.h"
#include "emailfolderview.h"

#include <private/accountconfiguration_p.h>
#include <private/longstream_p.h>

#include <qtopianamespace.h>
#include <qtopialog.h>

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDSActionRequest>
#include <QDSData>
#include <QFile>
#include <QHeaderView>
#include <QMailAccount>
#include <QMailAddress>
#include <QMailComposerInterface>
#include <QMailStore>
#include <QMailTimeStamp>
#include <QMenuBar>
#include <QMessageBox>
#include <QSoftMenuBar>
#include <QStack>
#include <QStackedWidget>
#include <QtopiaApplication>
#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QWapAccount>
#include <QWaitWidget>

#ifndef QTOPIA_NO_SMS
#include <QSMSMessage>
#endif //QTOPIA_NO_SMS

#ifndef QTOPIA_NO_COLLECTIVE
#include <QNetworkRegistration>
#endif

#ifndef QTOPIA_HOMEUI
#include "searchview.h"
#else
#include <private/homewidgets_p.h>
#endif //QTOPIA_HOMEUI


#ifndef LED_MAIL
#define LED_MAIL 0
#endif

enum ActivityType {
    Inactive = 0,
    Retrieving = 1,
    Sending = 2
};

enum RetrievalPhase {
    None = 0,
    Previewing = 1,
    PreviewCompleted = 2,
    Completing = 3,
    CompletionCompleted = 4
};

// Time in ms to show new message dialog.  0 == Indefinate
static const int NotificationVisualTimeout = 0;

// Number of messages required before we use a progress indicator
static const int MinimumForProgressIndicator = 20;
static const int SearchMinimumForProgressIndicator = 100;

// Minimum time between screen updates during long-running operations
static const int ProgressIndicatorUpdatePeriod = 200;

// Maximum messages processed per batch operation
static const int MaxBatchSize = 50;
static const int BatchMinimumForProgressIndicator = 2 * MaxBatchSize + 1;

// This is used regularly:
static const QMailMessage::MessageType nonEmailType = static_cast<QMailMessage::MessageType>(QMailMessage::Mms |
                                                                                             QMailMessage::Sms |
                                                                                             QMailMessage::Instant |
                                                                                             QMailMessage::System);

static QSet<const char*> taskRegistry;

static void registerTask(const char* name)
{
    qLog(Messaging) << "Registering task:" << name;
    if (taskRegistry.contains(name)) {
        qWarning() << "Task already registered:" << name;
    } else {
        taskRegistry.insert(name);
    }
    QtopiaApplication::instance()->registerRunningTask(QLatin1String(name));
}

static void unregisterTask(const char* name)
{
    qLog(Messaging) << "Unregistering task:" << name;
    if (!taskRegistry.contains(name)) {
        qWarning() << "Task is not currently registered:" << name;
    } else {
        taskRegistry.remove(name);
    }
    QtopiaApplication::instance()->unregisterRunningTask(QLatin1String(name));
}

static bool taskRegistered(const char* name)
{
    return taskRegistry.contains(name);
}

class SearchProgressDialog : public QWaitWidget
{
    Q_OBJECT

public:
    SearchProgressDialog(QMailSearchAction* action);

public slots:
    void progressChanged(uint progress, uint max);
};

SearchProgressDialog::SearchProgressDialog(QMailSearchAction* action)
{
    setText(tr("Searching"));
    setCancelEnabled(true);
    connect(action,SIGNAL(progressChanged(uint,uint)),this,SLOT(progressChanged(uint,uint)));
    connect(this,SIGNAL(cancelled()),action,SLOT(cancelOperation()));
}

void SearchProgressDialog::progressChanged(uint value, uint max)
{
    QString percentageText;

    if((value + max) > 0)
    {
        float percentage = (max == 0) ? 0 : static_cast<float>(value)/max*100;
        percentageText = QString::number(percentage,'f',0) + "%";
    }

    QString text = tr("Searching") + "\n" + percentageText;
    setText(text);
}

#ifdef QTOPIA_HOMEUI
class ReplyDialog : public QDialog
{
    Q_OBJECT

public:
    ReplyDialog(QWidget *parent = 0)
        : QDialog(parent)
    {
        setObjectName("HomeReplyDialog");
        QVBoxLayout *vbl = new QVBoxLayout;
        vbl->addStretch(1);
        QGridLayout *grid = new QGridLayout;
        replyButton = new HomeActionButton(tr("Reply"), QtopiaHome::Green);
        connect(replyButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
        grid->addWidget(replyButton, 0, 0);
        replyAllButton = new HomeActionButton(tr("ReplyAll"), QtopiaHome::Green);
        connect(replyAllButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
        grid->addWidget(replyAllButton, 0, 1);
        forwardButton = new HomeActionButton(tr("Forward"), QtopiaHome::Green);
        connect(forwardButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
        grid->addWidget(forwardButton, 0, 2);
        cancelButton = new HomeActionButton(tr("Cancel"), QtopiaHome::Red);
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
        grid->addWidget(cancelButton, 1, 1);
        vbl->addLayout(grid);
        vbl->addStretch(1);
        setLayout(vbl);
        QtopiaHome::setPopupDialogStyle(this);
        mComposeAction = QMailComposerInterface::Reply;
        mRepliedFlags = QMailMessage::Replied;
    }

    QMailComposerInterface::ComposeContext composeAction()
    {
        return mComposeAction;
    }

    quint64 messageStatusFlag()
    {
        return mRepliedFlags;
    }

public slots:
    void buttonClicked()
    {
        if (sender() == replyButton) {
            mComposeAction = QMailComposerInterface::Reply;
            mRepliedFlags = QMailMessage::Replied;
        } else if (sender() == replyAllButton) {
            mComposeAction = QMailComposerInterface::ReplyToAll;
            mRepliedFlags = QMailMessage::RepliedAll;
        } else if (sender() == forwardButton) {
            mComposeAction = QMailComposerInterface::Forward;
            mRepliedFlags = QMailMessage::Forwarded;
        }
        accept();
    }

private:
    HomeActionButton *replyButton;
    HomeActionButton *replyAllButton;
    HomeActionButton *forwardButton;
    HomeActionButton *cancelButton;
    QMailComposerInterface::ComposeContext mComposeAction;
    quint64 mRepliedFlags;
};
#endif

#ifdef QTOPIA_HOMEUI
class NewMessagesDialog : public QDialog
{
    Q_OBJECT

public:
    NewMessagesDialog(const QString &title,
                      QWidget *parent = 0,
                      Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint)
        : QDialog(parent, f)
        {
            QVBoxLayout* vboxLayout = new QVBoxLayout(this);
            QHBoxLayout* h1boxLayout = new QHBoxLayout;
            QHBoxLayout* h2boxLayout = new QHBoxLayout;
            QHBoxLayout* h3boxLayout = new QHBoxLayout;
            QHBoxLayout* h4boxLayout = new QHBoxLayout;
            vboxLayout->addLayout(h1boxLayout);
            vboxLayout->addLayout(h2boxLayout);
            vboxLayout->addLayout(h3boxLayout);
            vboxLayout->addLayout(h4boxLayout);
            vboxLayout->setSpacing(0);
            vboxLayout->addStretch(1);
            h1boxLayout->addStretch(1);
            QLabel *titleLabel = new QLabel(title);
            QFont titleFont(titleLabel->font());
            titleFont.setPointSize(titleFont.pointSize()*4/3);
            titleLabel->setFont(titleFont);
            h1boxLayout->addWidget(titleLabel);
            h1boxLayout->addStretch(1);
            h1boxLayout->addStretch(3);
            h2boxLayout->setSpacing(kSpacing);
            mMessageLabel = new QLabel("");
            h2boxLayout->addWidget(mMessageLabel);
            mPortraitLabel = new QLabel("");
            h2boxLayout->addWidget(mPortraitLabel);
            mSenderLabel = new QLabel("");
            h2boxLayout->addWidget(mSenderLabel);
            h2boxLayout->addStretch(1);
            mTimeLabel = new QLabel("");
            h2boxLayout->addWidget(mTimeLabel);
            h2boxLayout->addStretch(1);
            mTextLabel = new QLabel("");
            h3boxLayout->addWidget(mTextLabel);
            h3boxLayout->addStretch(1);
            h4boxLayout->addStretch(1);
            mViewButton = new HomeActionButton(tr("View"), QtopiaHome::Green);
            connect(mViewButton, SIGNAL(clicked()), this, SLOT(emitFinished()));
            mIgnoreButton = new HomeActionButton(tr("Ignore"), QtopiaHome::Red);
            connect(mIgnoreButton, SIGNAL(clicked()), this, SLOT(reject()));
            h4boxLayout->addWidget(mViewButton);
            h4boxLayout->addWidget(mIgnoreButton);
            h4boxLayout->setSpacing(kSpacing);
            QtopiaHome::setPopupDialogStyle(this);
        }
    ~NewMessagesDialog()
        {
        }
    void setText(const QString &text)
        {
            mTextLabel->setText(text);
            mMessageLabel->hide();
            mPortraitLabel->hide();
            mSenderLabel->hide();
            mTimeLabel->hide();
            mViewButton->setText(tr("View"));
            update();
        }
    void setMessage(const QMailMessageId &id)
        {
            QMailMessage message(id);
            mTextLabel->setText(message.subject());
            mPortraitPixmap = message.from().matchContact().portrait().scaled(kPortraitWidth, kPortraitHeight);
            mPortraitLabel->setPixmap(mPortraitPixmap);
            mSenderLabel->setText(message.from().displayName());
            mTimeLabel->setText(message.date().toLocalTime().time().toString("h:mmap"));
            if ((message.messageType() == QMailMessage::Email) &&
                (message.content() == QMailMessage::VoicemailContent)) {
                mMessagePixmap = QIcon(":icon/home/voicemail").pixmap(kPortraitWidth, kPortraitHeight);
                mViewButton->setText(tr("Listen", "Listen to current message"));
            } else {
                mMessagePixmap = QIcon(":icon/home/message").pixmap(kPortraitWidth, kPortraitHeight);
                mViewButton->setText(tr("View", "View this message"));
            }
            mMessageLabel->setPixmap(mMessagePixmap);

            mMessageLabel->show();
            mPortraitLabel->show();
            mSenderLabel->show();
            mTimeLabel->show();
            update();
        }
private slots:    
    void emitFinished()
        {
            emit finished(QMessageBox::Yes);
        }

    
private:
    HomeActionButton *mViewButton;
    HomeActionButton *mIgnoreButton;
    QLabel *mTextLabel;
    QLabel *mSenderLabel;
    QLabel *mTimeLabel;
    QLabel *mPortraitLabel;
    QPixmap mPortraitPixmap;
    QLabel *mMessageLabel;
    QPixmap mMessagePixmap;
    enum { kPortraitWidth = 26 };
    enum { kPortraitHeight = 26 };
    enum { kSpacing = 4 };
};
#else
class NewMessagesDialog : public QMessageBox
{
    Q_OBJECT

public:
    NewMessagesDialog(Icon icon,
                      const QString &title,
                      const QString &text,
                      StandardButtons buttons = NoButton,
                      QWidget *parent = 0,
                      Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint)
        : QMessageBox(icon, title, text, buttons, parent, f)
        {}
    ~NewMessagesDialog()
        {}
};
#endif

class AcknowledgmentBox : public QMessageBox
{
    Q_OBJECT

public:
    static void show(const QString& title, const QString& text);

private:
    AcknowledgmentBox(const QString& title, const QString& text);
    ~AcknowledgmentBox();

    virtual void keyPressEvent(QKeyEvent* event);

    static const int _timeout = 3 * 1000;
};

AcknowledgmentBox::AcknowledgmentBox(const QString& title, const QString& text)
    : QMessageBox(0)
{
    setWindowTitle(title);
    setText(text);
    setIcon(QMessageBox::Information);
    setAttribute(Qt::WA_DeleteOnClose);

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);

    QDialog::show();

    QTimer::singleShot(_timeout, this, SLOT(accept()));
}

AcknowledgmentBox::~AcknowledgmentBox()
{
}

void AcknowledgmentBox::show(const QString& title, const QString& text)
{
    (void)new AcknowledgmentBox(title, text);
}

void AcknowledgmentBox::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Select) {
        event->accept();
        accept();
    } else {
        QMessageBox::keyPressEvent(event);
    }
}

#ifndef QTOPIA_HOMEUI
static QString dateToString(const QDateTime& dateTime)
{
    QDate endWeek = QDate::currentDate();
    endWeek.addDays( 7 - endWeek.dayOfWeek() );

    int daysTo = abs( dateTime.date().daysTo(endWeek) );
    if ( daysTo < 7 ) {
        if ( QDate::currentDate() == dateTime.date() ) {
            QString day = qApp->translate("EmailClient", "Today %1").arg( QTimeString::localHM( dateTime.time() ) );
            return day;
        } else if ( dateTime.daysTo(QDateTime::currentDateTime()) == 1 ) {
            return qApp->translate("EmailClient", "Yesterday");
        } else {
            return QTimeString::localDayOfWeek(dateTime.date());
        }
    } else {
        return QTimeString::localYMD( dateTime.date() );
    }
}
#endif

// Keep track of where we are within the program
struct UILocation
{
    UILocation(MessageUiBase::Location location = MessageUiBase::NoLocation)
        : loc(location)
    {
    }

    MessageUiBase::Location location() const { return loc; }

    template<typename T>
    void append(const T& value) { data.append(QVariant(value)); }

    QVariant at(int i) const { return data.at(i); }

private:
    MessageUiBase::Location loc;
    QList<QVariant> data;
};


QDebug& operator<< (QDebug& debug, const UILocation& location)
{
    MessageUiBase::Location loc(location.location());

    if (loc == MessageUiBase::Viewer)
        return debug << "Viewer -" << location.at(1).value<QMailMessageId>();

    return debug << (loc == MessageUiBase::NoLocation ? "NoLocation" :
                    (loc == MessageUiBase::ActionList ? "ActionList" :
                    (loc == MessageUiBase::FolderList ? "FolderList" :
                    (loc == MessageUiBase::MessageList ? "MessageList" :
                    (loc == MessageUiBase::SearchResults ? "SearchResults" :
                    (loc == MessageUiBase::Composer ? "Composer" : "Unknown!"))))));
}


static QStack<UILocation> locationStack;

static void pushLocation(const UILocation& location)
{
    qLog(Messaging) << "pushLocation -" << locationStack.count() << ":" << location;
    locationStack.push(location);
}

static void popLocation()
{
    locationStack.pop();
    if (!locationStack.isEmpty())
        qLog(Messaging) << "popLocation  -" << locationStack.count() - 1 << ":" << locationStack.top();
    else
        qLog(Messaging) << "popLocation  - empty";
}

static bool haveLocation()
{
    return !locationStack.isEmpty();
}

static const UILocation& currentLocation()
{
    return locationStack.top();
}

static bool locationIncludes(MessageUiBase::Location loc)
{
    if (locationStack.isEmpty())
        return false;

    QStack<UILocation>::const_iterator it = locationStack.begin(), end = locationStack.end();
    for ( ; it != end; ++it)
        if ((*it).location() == loc)
            return true;

    return false;
}


MessageUiBase::MessageUiBase(QWidget* parent)
    : QWidget(parent),
      mailboxView(0),
      appTitle(tr("Messages")),
      suspendMailCount(true),
      markingMode(false),
      selectionCount(0), 
      actionCountSuspended(false),
      emailCountSuspended(false)
{
    mailboxView = new QStackedWidget( this );
    mailboxView->setObjectName( "mailboxView" );
    mailboxView->setFrameStyle( QFrame::NoFrame );

    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setContentsMargins( 0, 0, 0, 0 );
    vb->setSpacing( 0 );
    vb->addWidget( mailboxView );

    setWindowTitle( appTitle );
}

MessageUiBase::Location MessageUiBase::currentLocation() const
{
    if (::haveLocation())
        return ::currentLocation().location();

    return NoLocation;
}

bool MessageUiBase::locationIncludes(MessageUiBase::Location loc) const
{
    return ::locationIncludes(loc);
}

void MessageUiBase::viewActionList(const QString& title)
{
    QString caption(title.isNull() ? appTitle : title);

    UILocation loc(ActionList);
    loc.append(caption);

    pushLocation(loc);
}

void MessageUiBase::viewFolderList(const QString& title)
{
    UILocation loc(FolderList);
    loc.append(title);

    pushLocation(loc);
}

#ifdef QTOPIA_HOMEUI
void MessageUiBase::viewMessageList(const MessageListView::DisplayMode& mode, const QString& title)
{
    UILocation loc(MessageList);
    loc.append(title);
    loc.append(mode);

    pushLocation(loc);
}
#else
void MessageUiBase::viewMessageList(const QMailMessageKey& filter, const QString& title)
{
    UILocation loc(MessageList);
    loc.append(title);
    loc.append(filter);

    pushLocation(loc);
}
#endif

void MessageUiBase::viewSearchResults(const QMailMessageKey& filter, const QString& title)
{
    QString caption(title);
    if (caption.isNull())
        caption = tr("Search Results");
    
    UILocation loc(SearchResults);
    loc.append(caption);
    loc.append(filter);

    pushLocation(loc);
}

void MessageUiBase::viewMessage(const QMailMessageId& id, bool permitNavigation, const QString& title)
{
    UILocation loc(Viewer);
    loc.append(title);
    loc.append(id);
    loc.append(permitNavigation);

    pushLocation(loc);
}

void MessageUiBase::viewComposer(const QString& title)
{
#ifdef QTOPIA_HOMEUI
    QtopiaApplication::execDialog(writeMailWidget());
    Q_UNUSED(title);
#else
    UILocation loc(Composer);
    loc.append(title);
    pushLocation(loc);
#endif
}

void MessageUiBase::clearLocationStack()
{
    while (::haveLocation())
        popLocation();
}

void MessageUiBase::popLocation()
{
    if (!::haveLocation())
        return;

    MessageUiBase::Location loc(currentLocation());

    if (markingMode && ((loc == MessageList) || (loc == SearchResults))) {
        // If we are leaving the message list, ensure it is not in marking mode
        setMarkingMode(false);
    } else if (loc == Viewer) {
        // If we are closing the viewer, inform it and let it close itself
        readMailWidget()->closeView();
        return;
    }

    ::popLocation();
}

void MessageUiBase::pushLocation(const UILocation& location)
{
    // Do not nest search results, but replace the prior one
    if ((location.location() == SearchResults) && (currentLocation() == SearchResults)) 
        popLocation();

    ::pushLocation(location);

    displayLocation(::currentLocation());
}

void MessageUiBase::leaveLocation()
{
    MessageUiBase::Location loc(currentLocation());

    // If we have never raised the app, we will have no locations
    if (loc == NoLocation)
        return;

    // If we are leaving the message list, ensure it is not in marking mode
    if (markingMode && ((loc == MessageList) || (loc == SearchResults)))
        setMarkingMode(false);

    ::popLocation();

    if (!::haveLocation()) {
        // We have finished
        allWindowsClosed();
    } else {
        // Clear any status information remaining from the previous location
        clearStatusText();

        displayLocation(::currentLocation());
    }
}

void MessageUiBase::displayLocation(const UILocation& location)
{
    QString caption(location.at(0).value<QString>());

    MessageUiBase::Location loc(location.location());

    if (loc == Composer) {
        showComposer(caption);
    } else if (loc == Viewer) {
        QMailMessageId id(location.at(1).value<QMailMessageId>());
        bool permitNavigation(location.at(2).value<bool>());
        showViewer(id, permitNavigation, caption);
#ifdef QTOPIA_HOMEUI
    } else if (loc == MessageList) {
        MessageListView::DisplayMode mode = static_cast<MessageListView::DisplayMode>(location.at(1).value<int>());
        showMessageList(mode, caption);
    } else if (loc == SearchResults) {
#else
    } else if ((loc == MessageList) || (loc == SearchResults)) {
#endif //QTOPIA_HOMEUI
        QMailMessageKey filter(location.at(1).value<QMailMessageKey>());
        showMessageList(filter, caption);
    } else if (loc == FolderList) {
        showFolderList(caption);
    } else if (loc == ActionList) {
        showActionList(caption);
    } else {
        qLog(Messaging) << "Unknown location cannot be displayed:" << loc;
    }
}

void MessageUiBase::showActionList(const QString& caption)
{
    setCurrentMailboxWidget(actionView());

    showWidget(this, caption);
}

void MessageUiBase::showFolderList(const QString& title)
{
    setCurrentMailboxWidget(folderView());

    showWidget( this, title );
}

#ifdef QTOPIA_HOMEUI
void MessageUiBase::showMessageList(const MessageListView::DisplayMode& mode, const QString& title)
{
    bool contentsChanged = (mode != messageListView()->displayMode());
    if (contentsChanged)
        messageListView()->setDisplayMode(mode);

    showMessageList(contentsChanged, title);
}
#endif //QTOPIA_HOMEUI

void MessageUiBase::showMessageList(const QMailMessageKey& filter, const QString& title)
{
    bool contentsChanged = (filter != messageListView()->key());
    if (contentsChanged)
        messageListView()->setKey(filter);

    showMessageList(contentsChanged, title);
}

void MessageUiBase::showMessageList(bool contentsChanged, const QString& title)
{
    if (contentsChanged) {
        if (selectedMessageId.isValid()) {
            messageListView()->setSelected(selectedMessageId);
            selectedMessageId = QMailMessageId();
        }

        messageSelectionChanged();
    }

    setCurrentMailboxWidget(messageListView());

    QString caption;
    if (!title.isNull()) {
        caption = title;

        if (caption.contains("%s")) {
            int count = messageListView()->rowCount();
            caption.replace("%s", QString::number(count));
        }
    }

    showWidget(this, caption);
    showMessageStatus();
}

void MessageUiBase::showViewer(const QMailMessageId& id, bool permitNavigation, const QString& caption)
{
    bool hasNext = (permitNavigation && messageListView()->hasNext());
    bool hasPrevious = (permitNavigation && messageListView()->hasPrevious());

    if (readMailWidget()->displayedMessage() != id)
        readMailWidget()->displayMessage(id, QMailViewerFactory::AnyPresentation, hasNext, hasPrevious);

    showWidget(readMailWidget(), caption);
    if (caption.isNull())
        readMailWidget()->updateWindowTitle();
}

void MessageUiBase::showComposer(const QString& caption)
{
    showWidget(writeMailWidget(), caption);
}

WriteMail* MessageUiBase::writeMailWidget() const
{
    static WriteMail* writeMail = const_cast<MessageUiBase*>(this)->createWriteMailWidget();
    return writeMail;
}

ReadMail* MessageUiBase::readMailWidget() const
{
    static ReadMail* readMail = const_cast<MessageUiBase*>(this)->createReadMailWidget();
    return readMail;
}

EmailFolderView* MessageUiBase::folderView() const
{
    static EmailFolderView* view = const_cast<MessageUiBase*>(this)->createFolderView();
    return view;
}

MessageListView* MessageUiBase::messageListView() const
{
    static MessageListView* view = const_cast<MessageUiBase*>(this)->createMessageListView();
    return view;
}

ActionFolderView* MessageUiBase::actionView() const
{
    static ActionFolderView* view = const_cast<MessageUiBase*>(this)->createActionView();
    return view;
}

MessageStore* MessageUiBase::messageStore() const
{
    static MessageStore* list = const_cast<MessageUiBase*>(this)->createMessageStore();
    return list;
}

EmailFolderModel* MessageUiBase::emailFolderModel() const
{
    static EmailFolderModel* model = const_cast<MessageUiBase*>(this)->createEmailFolderModel();
    return model;
}

ActionFolderModel* MessageUiBase::actionFolderModel() const
{
    static ActionFolderModel* model = const_cast<MessageUiBase*>(this)->createActionFolderModel();
    return model;
}

void MessageUiBase::setCurrentMailboxWidget(QWidget* widget)
{
    if (mailboxView && widget) {
        mailboxView->setCurrentWidget(widget);
        contextStatusUpdate();
    }
}

void MessageUiBase::showWidget(QWidget* widget, const QString& title)
{
#ifndef QTOPIA_HOMEUI
    emit statusVisible(widget == this);
#endif
    emit raiseWidget(widget, title);
}

void MessageUiBase::showActionStatus(QMailMessageSet* item)
{
    if (currentLocation() != ActionList)
        return;

    if (item)
        emit updateStatus(item->data(ActionFolderModel::FolderStatusDetailRole).value<QString>());
}

void MessageUiBase::showFolderStatus(QMailMessageSet* item)
{
    if (item)
        emit updateStatus(item->data(EmailFolderModel::FolderStatusDetailRole).value<QString>());
}

void MessageUiBase::showMessageStatus()
{
    if ((currentLocation() != MessageList) && (currentLocation() != SearchResults))
        return;

    if (markingMode)
        return;

#ifndef QTOPIA_HOMEUI
    QMailMessageId currentId = messageListView()->current();
    if (currentId.isValid()) {
        QMailMessageMetaData message(currentId);
        emit updateStatus(dateToString(message.date().toLocalTime()));
    } else {
        clearStatusText();
    }
#endif
}

void MessageUiBase::contextStatusUpdate()
{
    if (suspendMailCount)
        return;

#ifdef QTOPIA_HOMEUI
    if (markingMode) {
        QString text(tr("%n message(s) selected", "%1: number of messages", selectionCount));
        emit updateStatus(text);
    } else {
        clearStatusText();
    }
#else
    // Only update the status if we're the currently visible widget
    if (QTMailWindow::singleton()->currentWidget() == this) {
        Location location = currentLocation();
        if ( location == FolderList ) {
            // Show the status of the currently selected folder
            showFolderStatus(folderView()->currentItem());
        } else if ( location == ActionList ) {
            // Show the status of the currently selected action
            showActionStatus(actionView()->currentItem());
        } else if ( ( location == MessageList ) || ( location == SearchResults ) ) {
            if (markingMode) {
                QString text(tr("%n message(s) selected", "%1: number of messages", selectionCount));
                emit updateStatus(text);
            } else {
                // Show the status of the selected message
                showMessageStatus();
            }
        }
    }
#endif
}

void MessageUiBase::suspendMailCounts()
{
    suspendMailCount = true;

    if (!actionFolderModel()->ignoreMailStoreUpdates()) {
        actionFolderModel()->setIgnoreMailStoreUpdates(true);
        actionCountSuspended = true;
    }
    if (!emailFolderModel()->ignoreMailStoreUpdates()) {
        emailFolderModel()->setIgnoreMailStoreUpdates(true);
        emailCountSuspended = true;
    }
}

void MessageUiBase::resumeMailCounts()
{
    suspendMailCount = false;

    if (actionCountSuspended) {
        actionFolderModel()->setIgnoreMailStoreUpdates(false);
        actionCountSuspended = false;
    }
    if (emailCountSuspended) {
        emailFolderModel()->setIgnoreMailStoreUpdates(false);
        emailCountSuspended = false;
    }

    contextStatusUpdate();
}

void MessageUiBase::messageSelectionChanged()
{
    selectionCount = messageListView()->selected().count();
    contextStatusUpdate();
}

#ifdef QTOPIA_HOMEUI
void MessageUiBase::displayModeChanged(const MessageListView::DisplayMode &mode)
{
    // If we're currently viewing the message list, replace the mode instead of nesting
    if ((currentLocation() == MessageList) || (currentLocation() == SearchResults))
        popLocation();

    // Change to the new mode
    viewMessageList(mode);
}
#endif

void MessageUiBase::setMarkingMode(bool set)
{
    markingMode = set;

    messageListView()->setMarkingMode(markingMode);
    if (!markingMode) {
        // Clear whatever selections were previously made
        messageListView()->clearSelection();

#ifdef QTOPIA_HOMEUI
        emit statusVisible(false);
#endif
    }
    contextStatusUpdate();
}

void MessageUiBase::clearStatusText()
{
    emit clearStatus();

#ifdef QTOPIA_HOMEUI
    emit statusVisible(markingMode);
#endif
}

void MessageUiBase::presentMessage(const QMailMessageId &id, QMailViewerFactory::PresentationType type)
{
    readMailWidget()->displayMessage(id, type, false, false);
}

WriteMail* MessageUiBase::createWriteMailWidget()
{
    WriteMail* writeMail = new WriteMail(this);
#ifndef QTOPIA_HOMEUI
    if ( parentWidget()->inherits("QStackedWidget") )
        static_cast<QStackedWidget*>(parentWidget())->addWidget(writeMail);
#endif //QTOPIA_HOMEUI

    writeMail->setObjectName("write-mail");

    connect(writeMail, SIGNAL(enqueueMail(QMailMessage)), this, SLOT(enqueueMail(QMailMessage)));
    connect(writeMail, SIGNAL(discardMail()), this, SLOT(discardMail()));
    connect(writeMail, SIGNAL(saveAsDraft(QMailMessage)), this, SLOT(saveAsDraft(QMailMessage)));
    connect(writeMail, SIGNAL(noSendAccount(QMailMessage::MessageType)), this, SLOT(noSendAccount(QMailMessage::MessageType)));
    connect(writeMail, SIGNAL(editAccounts()), this, SLOT(settings()));
#ifndef QTOPIA_HOMEUI
    connect(writeMail, SIGNAL(finished()), this, SLOT(leaveLocation()));
#endif //QTOPIA_HOMEUI

    return writeMail;
}

ReadMail* MessageUiBase::createReadMailWidget()
{
    ReadMail* readMail = new ReadMail(this);
    if ( parentWidget()->inherits("QStackedWidget") )
        static_cast<QStackedWidget*>(parentWidget())->addWidget(readMail);

    readMail->setObjectName("read-message");

    readMail->setGeometry(geometry());

    connect(readMail, SIGNAL(resendRequested(QMailMessage,int)), this, SLOT(resend(QMailMessage,int)) );
    connect(readMail, SIGNAL(modifyRequested(QMailMessage)),this, SLOT(modify(QMailMessage)));
    connect(readMail, SIGNAL(removeMessage(QMailMessageId, bool)), this, SLOT(removeMessage(QMailMessageId, bool)) );
    connect(readMail, SIGNAL(viewingMail(QMailMessageMetaData)), this, SLOT(moveMailFront(QMailMessageMetaData)));
    connect(readMail, SIGNAL(getMailRequested(QMailMessageMetaData)),this, SLOT(getSingleMail(QMailMessageMetaData)) );
    connect(readMail, SIGNAL(sendMailRequested(QMailMessageMetaData)),this, SLOT(sendSingleMail(QMailMessageMetaData)));
    connect(readMail, SIGNAL(readReplyRequested(QMailMessageMetaData)),this, SLOT(readReplyRequested(QMailMessageMetaData)));
    connect(readMail, SIGNAL(sendMessageTo(QMailAddress,QMailMessage::MessageType)), this, SLOT(sendMessageTo(QMailAddress,QMailMessage::MessageType)) );
    connect(readMail, SIGNAL(viewNext()), this, SLOT(viewNextMessage()) );
    connect(readMail, SIGNAL(viewPrevious()), this, SLOT(viewPreviousMessage()) );
    connect(readMail, SIGNAL(viewMessage(QMailMessageId,QMailViewerFactory::PresentationType)), this, SLOT(presentMessage(QMailMessageId,QMailViewerFactory::PresentationType)) );
    connect(readMail, SIGNAL(sendMessage(QMailMessage)), this, SLOT(enqueueMail(QMailMessage)) );
    connect(readMail, SIGNAL(cancelView()), this, SLOT(leaveLocation()) );

    return readMail;
}

EmailFolderView* MessageUiBase::createFolderView()
{
    EmailFolderView* view = new EmailFolderView(mailboxView);

    view->setObjectName("read-email");
    view->setModel(emailFolderModel());

    connect(view, SIGNAL(selected(QMailMessageSet*)), this, SLOT(folderSelected(QMailMessageSet*)));
#ifndef QTOPIA_HOMEUI
    connect(view, SIGNAL(activated(QMailMessageSet*)), this, SLOT(folderActivated(QMailMessageSet*)));
#endif //QTOPIA_HOMEUI
    connect(view, SIGNAL(backPressed()), this, SLOT(leaveLocation()));

    mailboxView->addWidget( view );
    return view;
}

MessageListView* MessageUiBase::createMessageListView()
{
    MessageListView* view = new MessageListView(mailboxView);

    // Default sort is order of arrival.
    QMailMessageSortKey sortKey(QMailMessageSortKey::Id, Qt::DescendingOrder);
    view->setSortKey(sortKey);

    connect(view, SIGNAL(clicked(QMailMessageId)), this, SLOT(messageActivated()));
    connect(view, SIGNAL(currentChanged(QMailMessageId,QMailMessageId)), this, SLOT(showMessageStatus()) );
    connect(view, SIGNAL(selectionChanged()), this, SLOT(messageSelectionChanged()) );
#ifdef QTOPIA_HOMEUI
    connect(view, SIGNAL(displayModeChanged(MessageListView::DisplayMode)), this, SLOT(displayModeChanged(MessageListView::DisplayMode)) );
#endif
    connect(view, SIGNAL(backPressed()), this, SLOT(leaveLocation()) );
    connect(view, SIGNAL(resendRequested(QMailMessage,int)), this, SLOT(resend(QMailMessage,int)) );

    mailboxView->addWidget(view);
    return view;
}

ActionFolderView* MessageUiBase::createActionView()
{
    ActionFolderView* view = new ActionFolderView(mailboxView);

    view->setObjectName("actionView");
    view->setModel(actionFolderModel());

#ifndef QTOPIA_HOMEUI
    connect(view, SIGNAL(selected(QMailMessageSet*)), this, SLOT(actionSelected(QMailMessageSet*)));
    connect(view, SIGNAL(composeActionActivated(QMailMessageSet*)), this, SLOT(composeActivated()));
    connect(view, SIGNAL(emailActionActivated(QMailMessageSet*)), this, SLOT(emailActivated()));
    connect(view, SIGNAL(folderActivated(QMailMessageSet*)), this, SLOT(actionActivated(QMailMessageSet*)));
    connect(view, SIGNAL(backPressed()), this, SLOT(leaveLocation()) );
#endif //QTOPIA_HOMEUI

    mailboxView->addWidget( view );
    return view;
}

MessageStore* MessageUiBase::createMessageStore()
{
    MessageStore* list = new MessageStore(this);

    connect(list, SIGNAL(stringStatus(QString&)), this, SLOT(setStatusText(QString&)) );
    connect(list, SIGNAL(externalEdit(QString)), this,SLOT(externalEdit(QString)) );

    return list;
}

EmailFolderModel* MessageUiBase::createEmailFolderModel()
{
    EmailFolderModel* model = new EmailFolderModel(this);
    return model;
}

ActionFolderModel* MessageUiBase::createActionFolderModel()
{
    ActionFolderModel* model = new ActionFolderModel(this);
    return model;
}


EmailClient::EmailClient( QWidget* parent )
    : MessageUiBase( parent ),
      enableMessageActions(false), 
      fetchTimer(this),
      planeMode("/UI/Profile/PlaneMode"),
      smsReady("/Telephony/Status/SMSReady"),
      jabberState("/Communications/QNetworkRegistration/jabber/state"),
      jabberRaiseAttempted(false),
      newMessagesBox(0),
      initialAction(None),
      searchAction(0)
{
    setObjectName( "EmailClient" );

    autoGetMail = false;
    retrievalPhase = None;
    closeAfterTransmissions = false;
    closeAfterWrite = false;
    primaryActivity = Inactive;
    transferStatus = Inactive;
    suspendStatus = false;
    filesRead = false;
    searchView = 0;
    preSearchWidgetId = -1;
    pendingSmtpAccounts = false;
    ignoredMessageCount = 0;

    init();
}

EmailClient::~EmailClient()
{
}

void EmailClient::openFiles()
{
    delayedInit();

    if ( filesRead ) {
        if ( cachedDisplayMailId.isValid() )
            displayCachedMail();

        return;
    }

    filesRead = true;

    messageStore()->openMailboxes();

    MessageFolder* outbox = messageStore()->mailbox(QMailFolder::OutboxFolder);
    if (outbox->messageCount(MessageFolder::All)) {
        // There are messages ready to be sent
        QTimer::singleShot( 0, this, SLOT(sendAllQueuedMail()) );
    }

    if ( cachedDisplayMailId.isValid() ) {
        displayCachedMail();
    }
}

void EmailClient::displayCachedMail()
{
    viewMessage(cachedDisplayMailId, false);

    cachedDisplayMailId = QMailMessageId();
}

void EmailClient::resumeInterruptedComposition()
{
    QSettings mailconf("Trolltech", "qtmail");
    mailconf.beginGroup("restart");

    QVariant var = mailconf.value("lastDraftId");
    if (!var.isNull()) {
        lastDraftId = QMailMessageId(var.toULongLong());
        mailconf.remove("lastDraftId");
    }

    mailconf.endGroup();

    if (lastDraftId.isValid()) {
        if (QMessageBox::information(0, 
                                     tr("Incomplete message"), 
                                     tr("Messages was previously interrupted while composing a message.\n"
                                        "Do you want to resume composing the message?"),
                                     QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            QMailMessage message(lastDraftId);
            modify(message);
        }
    }
}

void EmailClient::displayMessage(const QMailMessageId &id)
{
    initialAction = View;
    delayedInit();

    if (!checkMailConflict(tr("Should this message be saved in Drafts before viewing the new message?"),
                           tr("'View Mail' message will be ignored")) ) {
        cachedDisplayMailId = id;
        openFiles();
    }
}

void EmailClient::raiseApplication()
{
    if ((currentLocation() == NoLocation) && (closeAfterTransmissions)) {
        // Although still running hidden, we have been re-raised by QPE in response to a user request
        initialAction = None;
        closeAfterTransmissions = false;

        userInvocation();
    }
}

bool EmailClient::cleanExit(bool force)
{
    bool result = true;

    if (isTransmitting()) {
        if (force) {
            qLog(Messaging) << "EmailClient::cleanExit: forcing cancel to exit";
            cancelOperation();   //abort all transfer
        }
        result = false;
    }

    saveSettings();
    return result;
}

bool EmailClient::closeImmediately()
{
    if ((currentLocation() == Composer) && (writeMailWidget()->hasContent())) {
        // We need to save whatever is currently being worked on
        writeMailWidget()->forcedClosure();

        if (lastDraftId.isValid()) {
            // Store this value to remind the user on next startup
            QSettings mailconf("Trolltech", "qtmail");
            mailconf.beginGroup("restart");
            mailconf.setValue("lastDraftId", lastDraftId.toULongLong() );
            mailconf.endGroup();
        }
    }

    if (isTransmitting()) {
        closeAfterTransmissionsFinished();
        return false;
    }

    return true;
}

void EmailClient::closeAfterTransmissionsFinished()
{
    closeAfterWrite = false;
    closeAfterTransmissions = true;
}

void EmailClient::closeApplication()
{
    cleanExit(false);

    // If we're still transmitting, just hide until it completes
    if (isTransmitting()) {
        QTMailWindow::singleton()->hide();

        // Clear our location stack
        clearLocationStack();
    } else {
        QTMailWindow::singleton()->close();
    }
}

void EmailClient::allWindowsClosed()
{
    closeAfterTransmissionsFinished();
    closeApplication();
}

bool EmailClient::isTransmitting()
{
    return (transferStatus != Inactive);
}

bool EmailClient::isSending()
{
    return (transferStatus & Sending);
}

bool EmailClient::isRetrieving()
{
    return (transferStatus & Retrieving);
}

void EmailClient::initActions()
{
    if (!getMailButton) {
        getMailButton = new QAction( QIcon(":icon/getmail"), tr("Get all mail"), this );
        connect(getMailButton, SIGNAL(triggered()), this, SLOT(getAllNewMail()) );
        getMailButton->setWhatsThis( tr("Get new mail from all your accounts.") );
        setActionVisible(getMailButton, false);

        getAccountButton = new QAction( QIcon(":icon/account"), QString(), this );
        connect(getAccountButton, SIGNAL(triggered()), this, SLOT(getAccountMail()) );
        getAccountButton->setWhatsThis( tr("Get new mail from current account.") );
        setActionVisible(getAccountButton, false);

        cancelButton = new QAction( QIcon(":icon/reset"), tr("Cancel transfer"), this );
        connect(cancelButton, SIGNAL(triggered()), this, SLOT(cancelOperation()) );
        cancelButton->setWhatsThis( tr("Abort all transfer of mail.") );
        setActionVisible(cancelButton, false);

        composeButton = new QAction( QIcon(":icon/new"), tr("New"), this );
        connect(composeButton, SIGNAL(triggered()), this, SLOT(composeActivated()) );
        composeButton->setWhatsThis( tr("Write a new message.") );

        searchButton = new QAction( QIcon(":icon/find"), tr("Search"), this );
        connect(searchButton, SIGNAL(triggered()), this, SLOT(search()) );
        searchButton->setWhatsThis( tr("Search for messages in your folders.") );

        synchronizeAction = new QAction( this );
        connect(synchronizeAction, SIGNAL(triggered()), this, SLOT(synchronizeFolder()) );
        synchronizeAction->setWhatsThis( tr("Decide whether messages in this folder should be retrieved.") );
        setActionVisible(synchronizeAction, false);

        settingsAction = new QAction( QIcon(":icon/settings"), tr("Account settings..."), this );
        connect(settingsAction, SIGNAL(triggered()), this, SLOT(settings()));

        emptyTrashAction = new QAction( QIcon(":icon/trash"), tr("Empty trash"), this );
        connect(emptyTrashAction, SIGNAL(triggered()), this, SLOT(emptyTrashFolder()));
        setActionVisible(emptyTrashAction, false);

        moveAction = new QAction( this );
        connect(moveAction, SIGNAL(triggered()), this, SLOT(moveSelectedMessages()));
        setActionVisible(moveAction, false);

        copyAction = new QAction( this );
        connect(copyAction, SIGNAL(triggered()), this, SLOT(copySelectedMessages()));
        setActionVisible(copyAction, false);

        restoreAction = new QAction( this );
        connect(restoreAction, SIGNAL(triggered()), this, SLOT(restoreSelectedMessages()));
        setActionVisible(restoreAction, false);

        selectAllAction = new QAction( tr("Select all"), this );
        connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
        setActionVisible(selectAllAction, false);

        deleteMailAction = new QAction( this );
        deleteMailAction->setIcon( QIcon(":icon/trash") );
        connect(deleteMailAction, SIGNAL(triggered()), this, SLOT(deleteSelectedMessages()));
        setActionVisible(deleteMailAction, false);

        markAction = new QAction( tr("Mark messages"), this );
        connect(markAction, SIGNAL(triggered()), this, SLOT(markMessages()));
        setActionVisible(markAction, true);
    }

    if (actionContext && !actionContextPrepared) {
        actionContext->addAction( emptyTrashAction );
        actionContext->addAction( settingsAction );
        actionContext->addAction( searchButton );

        actionContextPrepared = true;
    }

    if (folderContext && !folderContextPrepared) {
        folderContext->addAction( composeButton );
        folderContext->addAction( getMailButton );
        folderContext->addAction( getAccountButton );
        folderContext->addAction( searchButton );
        folderContext->addAction( cancelButton );
        folderContext->addAction( emptyTrashAction );
        folderContext->addSeparator();
        folderContext->addAction( synchronizeAction );
        folderContext->addAction( settingsAction );

        updateGetMailButton();

        folderContextPrepared = true;
    }

    if (messageContext && !messageContextPrepared) {
        messageContext->addAction( composeButton );
        messageContext->addAction( deleteMailAction );
#ifndef QTOPIA_HOMEUI
        messageContext->addAction( moveAction );
        messageContext->addAction( copyAction );
        messageContext->addAction( restoreAction );
        messageContext->addAction( selectAllAction );
        messageContext->addAction( markAction );
#else
        messageContext->addAction( restoreAction );
        messageContext->addAction( searchButton );
        messageContext->addAction( settingsAction );
        messageContext->addAction( emptyTrashAction );
        messageContext->addAction( getMailButton );
        messageContext->addAction( cancelButton );
        messageContext->addAction( selectAllAction );
        messageContext->addAction( markAction );

        updateGetMailButton();
#endif //QTOPIA_HOMEUI

        messageContextPrepared = true;
    }
}

void EmailClient::updateActions()
{
    openFiles();

    // Ensure that the actions have been initialised
    initActions();

    if (currentLocation() == MessageList) {
        //Enable marking and selectAll actions only if we have messages.
        int messageCount = messageListView()->rowCount();
        setActionVisible(markAction, messageCount > 0);
        setActionVisible(selectAllAction, (messageCount > 1 && messageCount != selectionCount));
    } else {
        // Only enable empty trash action if the trash has messages in it
        QMailMessage::MessageType type = QMailMessage::AnyType;
        if ( currentLocation() == ActionList ) {
            type = nonEmailType;
        } else if (currentLocation() == FolderList) {
            type = QMailMessage::Email;
        }

        static MessageFolder* const trashFolder = messageStore()->mailbox(QMailFolder::TrashFolder);

        int messageCount = trashFolder->messageCount(MessageFolder::All, type);
        setActionVisible(emptyTrashAction, (messageCount > 0) && !markingMode);
    }

    // Set the visibility for each action to whatever was last configured
    QMap<QAction*, bool>::iterator it = actionVisibility.begin(), end = actionVisibility.end();
    for ( ; it != end; ++it)
        it.key()->setVisible(it.value());
}

void EmailClient::delayedInit()
{
    if (moveAction)
        return; // delayedInit already done


    QMailStore* store = QMailStore::instance();
    if (!store->initialized()) {
        // The mailstore isn't working - abort
        QMessageBox::warning(0,
                             tr("No Mail Store"),
                             tr("Unable to initialize the Mail Store!\n\nMessages cannot continue and will now terminate."),
                             QMessageBox::Ok);

        closeImmediately();
        QTMailWindow::singleton()->close();
        return;
    }

    // Whenever these actions occur, we need to reload accounts that may have changed
    connect(store, SIGNAL(accountsAdded(QMailAccountIdList)), this, SLOT(accountsAdded(QMailAccountIdList)));
    connect(store, SIGNAL(accountsRemoved(QMailAccountIdList)), this, SLOT(accountsRemoved(QMailAccountIdList)));
    connect(store, SIGNAL(accountsUpdated(QMailAccountIdList)), this, SLOT(accountsUpdated(QMailAccountIdList)));

    connect(store, SIGNAL(messagesAdded(QMailMessageIdList)), this, SLOT(messagesAdded(QMailMessageIdList)));

    // We need to detect when messages are marked as deleted during downloading
    connect(store, SIGNAL(messagesUpdated(QMailMessageIdList)), this, SLOT(messagesUpdated(QMailMessageIdList)));

    connect(&fetchTimer, SIGNAL(timeout()), this, SLOT(automaticFetch()) );
    connect(&planeMode, SIGNAL(contentsChanged()), this, SLOT(planeModeChanged()) );
    connect(&jabberState, SIGNAL(contentsChanged()), this, SLOT(jabberStateChanged()) );

    // Ideally would make actions functions methods and delay their
    // creation until context menu is shown.
    initActions();



    QTimer::singleShot(0, this, SLOT(openFiles()) );
}

EmailFolderView* EmailClient::createFolderView()
{
    EmailFolderView* view = MessageUiBase::createFolderView();

    folderContext = QSoftMenuBar::menuFor(view);
    connect(folderContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()));

    return view;
}

MessageListView* EmailClient::createMessageListView()
{
    MessageListView* view = MessageUiBase::createMessageListView();

    messageContext = QSoftMenuBar::menuFor(view);
    connect(messageContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()));
#ifdef QTOPIA_HOMEUI
    initActions();
#endif

    return view;
}

ActionFolderView* EmailClient::createActionView()
{
    ActionFolderView* view = MessageUiBase::createActionView();

    actionContext = QSoftMenuBar::menuFor(view);
    connect(actionContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()));

    return view;
}

void EmailClient::init()
{
    getMailButton = 0;
    getAccountButton = 0;
    cancelButton = 0;
    composeButton = 0;
    searchButton = 0;
    synchronizeAction = 0;
    settingsAction = 0;
    emptyTrashAction = 0;
    moveAction = 0;
    copyAction = 0;
    restoreAction = 0;
    selectAllAction = 0;
    deleteMailAction = 0;
    actionContext = 0;
    folderContext = 0;
    messageContext = 0;
    actionContextPrepared = false;
    folderContextPrepared = false;
    messageContextPrepared = false;

   
    // Connect our service action signals
    retrievalAction = new QMailRetrievalAction(this);
    transmitAction = new QMailTransmitAction(this);
    searchAction = new QMailSearchAction(this);

    foreach (QMailServiceAction *action, QList<QMailServiceAction*>() << retrievalAction
                                                                      << transmitAction
                                                                      << searchAction) {
        connect(action, SIGNAL(connectivityChanged(QMailServiceAction::Connectivity)), this, SLOT(connectivityChanged(QMailServiceAction::Connectivity)));
        connect(action, SIGNAL(activityChanged(QMailServiceAction::Activity)), this, SLOT(activityChanged(QMailServiceAction::Activity)));
        connect(action, SIGNAL(statusChanged(QMailServiceAction::Status)), this, SLOT(statusChanged(QMailServiceAction::Status)));
        if(action != searchAction)
            connect(action, SIGNAL(progressChanged(uint, uint)), this, SLOT(progressChanged(uint, uint)));
    }

    // New message event handlers
    newSmsHandler = new QMailNewSmsHandler(this);
    newMmsHandler = new QMailNewMmsHandler(this);
    newEmailHandler = new QMailNewEmailHandler(this);
    newInstantMessageHandler = new QMailNewInstantMessageHandler(this);
    newSystemMessageHandler = new QMailNewSystemMessageHandler(this);

    foreach (QMailNewMessageHandler *handler, QList<QMailNewMessageHandler*>() << newSmsHandler
                                                                               << newMmsHandler
                                                                               << newEmailHandler
                                                                               << newInstantMessageHandler
                                                                               << newSystemMessageHandler) {
        connect(handler, SIGNAL(newCountChanged(uint)), this, SLOT(newCountChanged(uint)));
    }

    // We need to load the settings in case they affect our service handlers
    readSettings();
}

void EmailClient::cancelOperation()
{
    if ( !cancelButton->isEnabled() )
        return;

    clearStatusText();

    pendingAccountIds.clear();

    if (isSending()) {
        transmitAction->cancelOperation();
        setSendingInProgress( false );
    }
    if (isRetrieving()) {
        retrievalAction->cancelOperation();
        setRetrievalInProgress( false );
    }
}

/*  Enqueue mail must always store the mail in the outbox   */
void EmailClient::enqueueMail(const QMailMessage& mailIn)
{
    static MessageFolder* const outboxFolder = messageStore()->mailbox(QMailFolder::OutboxFolder);
    static MessageFolder* const draftsFolder = messageStore()->mailbox(QMailFolder::DraftsFolder);

    QMailMessage mail(mailIn);

    // if uuid is not valid , it's a new mail
    bool isNew = !mail.id().isValid();
    if ( isNew ) {
        mailResponded();
    }

#ifndef QTOPIA_NO_COLLECTIVE
    if (mailIn.messageType() == QMailMessage::Instant) {
        // Currently this means jabber...
        if (!jabberState.value().toBool()) {
            if (!jabberRaiseAttempted) {
                // See if the user wants to raise the connection
                if (QMessageBox::question(0, 
                                        tr("Network Offline"),
                                        tr("Connection is currently unavailable.\nDo you want to connect to the network?"),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::Yes) == QMessageBox::Yes) {
                    QNetworkRegistration network("jabber");
                    network.setCurrentOperator(QTelephony::OperatorModeAutomatic);

                    jabberRaiseAttempted = true;
                }
            }

            if (!jabberRaiseAttempted) {
                // Move the message to drafts
                if (!draftsFolder->insertMessage(mail))
                    accessError(*draftsFolder);

                AcknowledgmentBox::show(tr("Saved to Drafts"), tr("Message has been saved to the Drafts folder"));
                return;
            }
        }
    }
#endif

    if ( !outboxFolder->insertMessage(mail) ) {
        accessError(*outboxFolder);
        return;
    }

    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        QMessageBox::information(0,
                                 tr("Airplane safe mode"),
                                 tr("Saved message to Outbox. Message will be sent after exiting Airplane Safe mode."));
    } else {
        sendAllQueuedMail(true);
    }

    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        closeApplication();
    }
}

/*  Simple, do nothing  */
void EmailClient::discardMail()
{
    // Reset these in case user chose reply but discarded message
    repliedFromMailId = QMailMessageId();
    repliedFlags = 0;

    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        closeApplication();
    }
}

void EmailClient::saveAsDraft(const QMailMessage& mailIn)
{
    static MessageFolder* const draftsFolder = messageStore()->mailbox(QMailFolder::DraftsFolder);

    QMailMessage mail(mailIn);

    // if uuid is not valid, it's a new mail
    bool isNew = !mail.id().isValid();
    if ( isNew ) {
        mailResponded();
    }

    if ( !draftsFolder->insertMessage(mail) ) {
        accessError(*draftsFolder);
    } else {
        lastDraftId = mail.id();
    }
}

/*  Mark a message as replied/repliedall/forwarded  */
void EmailClient::mailResponded()
{
    if ( repliedFromMailId.isValid() ) {
        QMailMessageMetaData replyMail(repliedFromMailId);
        replyMail.setStatus(replyMail.status() | repliedFlags);
        QMailStore::instance()->updateMessage(&replyMail);
    }

    repliedFromMailId = QMailMessageId();
    repliedFlags = 0;
}

// send all messages in outbox, by looping through the outbox, sending
// each message that belongs to the current found account
void EmailClient::sendAllQueuedMail(bool userRequest)
{
    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        return;
    }

    static MessageFolder* const outboxFolder = messageStore()->mailbox(QMailFolder::OutboxFolder);
    static MessageFolder* const draftsFolder = messageStore()->mailbox(QMailFolder::DraftsFolder);

    QMailMessageIdList outgoingIds = outboxFolder->messages();
    int outgoingCount(outgoingIds.count());
    if (outgoingCount == 0)
        return;

    if (userRequest) {
        // See if the message viewer wants to suppress the 'Sending messages' notification
        if (!readMailWidget()->handleOutgoingMessages(outgoingIds)) {
            // Tell the user we're responding
            QString detail;
            if (outgoingCount == 1) {
                QMailMessageMetaData mail(*outgoingIds.begin());
                detail = mailType(mail.messageType());
            } else {
                detail = tr("%n message(s)", "%1: number of messages", outgoingCount);
            }

            AcknowledgmentBox::show(tr("Sending"), tr("Sending:") + " " + detail);
        }
    }

    bool verifiedAccounts = false;
    bool haveValidAccount = false;
    bool emailOnly = pendingSmtpAccounts;

    pendingSmtpAccounts = false;

    QMailAccountId sendAccountId;

    QMailMessageIdList sendIds;
    foreach(const QMailMessageId &mailId, outgoingIds) {
        QMailMessageMetaData mail(mailId);

        // mail not previously sent, and has recipients defined, add to queue
        if ( !(mail.status() & QMailMessage::Sent) && !mail.to().isEmpty() ) {
            if (mail.messageType() == QMailMessage::Email) {
                // Make sure we have a valid account
                if (!verifiedAccounts) {
                    haveValidAccount = verifyAccounts(true);
                    verifiedAccounts = true;
                    if (!haveValidAccount)
                        qWarning("Queued mail requires valid email accounts but none available.");
                }

                /* The first mail determines which range of mails to first
                send.  As we allow use of several SMTP accounts we may
                need more than one connection, but the total number of connections
                needed will never exceed the number of SMTP accounts
                */
                if (haveValidAccount) {
                    if (!sendAccountId.isValid()) {
                        sendAccountId = mail.parentAccountId();
                    }
                    if (sendAccountId == mail.parentAccountId()) {
                        sendIds.append(mailId);
                    } else {
                        pendingSmtpAccounts = true;
                    }
                } else {
                    // We can't send this message - move it to the Drafts folder
                    moveMessage(mailId, draftsFolder);
                }
            } else if (!emailOnly) {
                // TODO: False assumption?
                // We can only have one account of other types
                sendIds.append(mailId);
            }
        }
    }

    if (sendIds.count() > 0) {
        setSendingInProgress(true);

        transmitAction->send(sendIds);
    } else {
        qWarning("no more messages to send");
    }
}

void EmailClient::sendSingleMail(const QMailMessageMetaData& message)
{
    if (isSending()) {
        qWarning("sending in progress, no action performed");
        return;
    }

    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        return;
    }

    bool needAccount = false;
    if ( message.messageType() == QMailMessage::Email )
        needAccount = true;

    if ( needAccount && !verifyAccounts(true) ) {
        qWarning("Mail requires valid email accounts but none available.");

        clearOutboxFolder();
        return;
    }

    pendingSmtpAccounts = false;

    setSendingInProgress(true);

    transmitAction->send(QMailMessageIdList() << message.id());
}

bool EmailClient::verifyAccounts(bool outgoing)
{
    bool ok = true;
    QString caption;
    QString text;

    QMailAccountIdList accountIds = QMailStore::instance()->queryAccounts();
    if (accountIds.isEmpty()) {
        caption = tr("No account selected");
        text = tr("<qt>You must create an account.</qt>");
        ok = false;
    } else {
        if (outgoing) {
            bool canSend(false);
            foreach (const QMailAccountId &id, accountIds) {
                QMailAccount account(id);
                if (account.canSendMail()) {
                    canSend = true;
                    break;
                }
            }

            if (!canSend) {
                caption = tr("No SMTP Server");
                text = tr("<qt>No valid SMTP server defined.<br><br>No emails could be sent.</qt>");
                ok = false;
            }
        } else {
            if (!mailAccountId.isValid()) {
                caption = tr("No POP or IMAP accounts defined");
                text = tr("<qt>Get mail only works with POP or IMAP.</qt>");
                ok = false;
            }
        }
    }

    if (!ok) {
        QMessageBox box(caption, text, QMessageBox::Warning, QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton);
        box.exec();
    }

    return ok;
}

void EmailClient::transmitCompleted()
{
    // If there are more SMTP accounts to service, continue
    if (pendingSmtpAccounts) {
        sendAllQueuedMail();
    } else {
        if (primaryActivity == Sending)
            clearStatusText();

        setSendingInProgress(false);
    }
}

void EmailClient::retrievalCompleted()
{
    if (retrievalPhase == Previewing) {
        previewRetrievalCompleted();
    } else if (retrievalPhase == Completing) {
        completionRetrievalCompleted();
    }
}

void EmailClient::searchCompleted()
{
    clearStatusText();
    viewSearchResults(QMailMessageKey(searchAction->matchingMessageIds()));
    searchProgressDialog()->hide();
}

void EmailClient::addMailToDownloadList(const QMailMessageMetaData& mail)
{
    if ( !mailAccountId.isValid() )
        return; // mail check cancelled

    if ( mail.status() & QMailMessage::Downloaded
         || mail.parentAccountId() != mailAccountId )
        return;

    static int maxMailSize = -1;
    static QMailAccountId maxMailAccountId;

    if (mailAccountId != maxMailAccountId) {
        maxMailAccountId = mailAccountId;

        AccountConfiguration config(maxMailAccountId);
        maxMailSize = config.maxMailSize();
    }

    if ( (maxMailSize > -1) && (mail.size() > static_cast<uint>(maxMailSize * 1024)) )
        return;

    mailDownloadList.sizeInsert(mail.serverUid(), mail.size(), mail.id(), mail.fromMailbox() );
}

void EmailClient::getNewMail()
{
    static MessageFolder* const inboxFolder = messageStore()->mailbox(QMailFolder::InboxFolder);

    if ( !verifyAccounts(false) )
        return;

    // Try to preserve the message list selection
    selectedMessageId = messageListView()->current();
    if (!selectedMessageId.isValid())
        selectedMessageId = QMailMessageId();

    //get any previous mails not downloaded and add to queue
    mailDownloadList.clear();
    QMailMessageIdList incomingIds = inboxFolder->messages(QMailMessage::Downloaded,false);
    foreach(const QMailMessageId &id, incomingIds){
        QMailMessageMetaData mail(id);
            addMailToDownloadList( mail );
    }

    setRetrievalInProgress(true);

    retrievalAction->retrieve(mailAccountId);
    retrievalPhase = Previewing;
}

void EmailClient::getAllNewMail()
{
    pendingAccountIds.clear();

    foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) {
        QMailAccount account(id);
        if ( account.canCollectMail() )
            pendingAccountIds.append(id);
    }

    if (!pendingAccountIds.isEmpty())
        getNextNewMail();
}

void EmailClient::getAccountMail()
{
    pendingAccountIds.clear();

    if (const QAction* action = static_cast<const QAction*>(sender())) {
        QMailAccountId accountId(action->data().value<QMailAccountId>());
        pendingAccountIds.append(accountId);
        getNextNewMail();
    }
}

void EmailClient::getSingleMail(const QMailMessageMetaData& message)
{
    if (isRetrieving()) {
        if ( mailAccountId == message.parentAccountId() ) {
            mailDownloadList.append(message.serverUid(), message.size(), message.id(), message.fromMailbox() );
        } else {
            qWarning("receiving in progress, no action performed");
        }
        return;
    }

    mailAccountId = message.parentAccountId();

    mailDownloadList.clear();
    mailDownloadList.sizeInsert(message.serverUid(), message.size(), message.id(), message.fromMailbox() );

    setRetrievalInProgress(true);

    retrievalAction->completeRetrieval(mailDownloadList.mailIds());
    retrievalPhase = Completing;
}

void EmailClient::readReplyRequested(const QMailMessageMetaData& mail)
{
# ifndef QTOPIA_NO_MMS
    static MessageFolder* const outboxFolder = messageStore()->mailbox(QMailFolder::OutboxFolder);

    QString netCfg;
    if (mail.parentAccountId().isValid()) {
        AccountConfiguration config(mail.parentAccountId());
        netCfg = config.networkConfig();
    }
    if ( netCfg.isEmpty() ) {
        qLog(Messaging) << "Unable to send MMS read reply without configuration!";
        return;
    }

    QWapAccount wapAccount( netCfg );
    if ( wapAccount.mmsDeliveryReport() ) {
        QString msg(tr("<qt>Do you wish to send a Read Reply?</qt>"));
        if (QMessageBox::information(0, tr("Multimedia Message"), msg,
                                     QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            QMailMessage rrmail;
            rrmail.setMessageType(QMailMessage::Mms);
            rrmail.setTo(mail.from());
            rrmail.setSubject(mail.subject());
            rrmail.setHeaderField("X-Mms-Message-Class", "Auto");
            rrmail.setHeaderField("X-Mms-Delivery-Report", "No");
            rrmail.setHeaderField("X-Mms-Read-Reply", "No");
            rrmail.setStatus(QMailMessage::Outgoing, true);
            rrmail.setStatus(QMailMessage::Downloaded, true);

            QString msg = tr("Sent MMS \"%1\" was read on: %2", "%1 = subject %2 = date");
            msg = msg.arg(mail.subject());
            msg = msg.arg(QDateTime::currentDateTime().toString());

            QMailMessagePart part;
            QMailMessageContentType type("text/plain; charset=ISO-8859-1");
            part.setBody(QMailMessageBody::fromData(msg, type, QMailMessageBody::EightBit));
            rrmail.appendPart(part);

            if ( !outboxFolder->insertMessage(rrmail) ) {
                accessError(*outboxFolder);
                return;
            }

            sendSingleMail(rrmail);
        }
    }
# endif
    Q_UNUSED(mail);
}

void EmailClient::mailPreviewed(const QMailMessageMetaData& mail)
{
    static MessageFolder* const inboxFolder = messageStore()->mailbox( QMailFolder::InboxFolder );
    static MessageFolder* const serverFolder = messageStore()->serverMailbox();

    openFiles();

    // If this message is owned by the message server, move it to our inbox
    if (serverFolder->contains(mail.id())) {
        if (!inboxFolder->moveMessage(mail.id())) {
            cancelOperation();
            accessError(*inboxFolder);
        }
    }

    if ((retrievalPhase == Previewing) || (retrievalPhase == PreviewCompleted)) {
        // See if we want to download the rest of this message immediately
        addMailToDownloadList(mail);
    }
}

#ifdef QTOPIA_HOMEUI
static void extractMessageDetails(const QMailMessage &message, QString *originator, QString *number, quint32 *seconds, QMailMessage::ContentType* type)
{
    // NOTE: the following code works with voicemail email notifications from Asterisk:
    if (message.partCount() > 0) {
        // Asterisk:
        // TODO - verify that the voicemail system is asterisk...

        // Alternatively, this same format is produced by our demonstration videomail composer

        // Part 1 is a textual description containing:
        //   From: "identifier" <number>
        //   Length: length

        QString description = message.partAt(0).body().data(QMailMessageBody::Decoded);
        if (!description.isEmpty()) {
            QRegExp from("From:\\s*\"([^\"]*)\"(?:\\s*<([^>]*)>)?");
            if (description.indexOf(from) != -1) {
                *originator = from.cap(1);
                *number = from.cap(2);
            }

            QRegExp length("Length:\\s*(\\S*)");
            if (description.indexOf(length) != -1) {
                QRegExp duration("(\\d{1,2}:)?(\\d{1,2}):(\\d{2})");
                if (duration.exactMatch(length.cap(1))) {
                    *seconds = duration.cap(1).toULong() * 60 * 60;
                    *seconds += duration.cap(2).toULong() * 60;
                    *seconds += duration.cap(3).toULong();
                }
            }
        }

        QMailMessageContentType contentType = message.partAt(1).contentType();
        if (contentType.type().toLower() == "video") {
            // This must be a videomail message
            *type = QMailMessage::VideomailContent;
        }
    }
}
#endif

void EmailClient::mailArrived(const QMailMessageMetaData& m)
{
    static MessageFolder* const inboxFolder = messageStore()->mailbox( QMailFolder::InboxFolder );
    static MessageFolder* const serverFolder = messageStore()->serverMailbox();

    openFiles();

    bool modified = false;
    QMailMessageMetaData mail(m);

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/TaskBar"), QLatin1String("setLed(int,bool)"));
        e << LED_MAIL << true;
    }

#ifndef QTOPIA_NO_MMS
    if (mail.messageType() == QMailMessage::Mms) {
        QMailMessage mailContent(mail.id());
        QString mmsType = mailContent.headerFieldText("X-Mms-Message-Type");
        if (mmsType.contains("m-delivery-ind")) {
            QString msg;
            QString mmsStatus = mailContent.headerFieldText("X-Mms-Status");
            if (mmsStatus.contains("Retrieved")) {
                msg = tr("<qt>Multimedia message delivered to %1.</qt>");
            } else if (mmsStatus.contains("Rejected")) {
                msg = tr("<qt>Multimedia message rejected by %1.</qt>");
            } else if (mmsStatus.contains("Deferred")) {
                msg = tr("<qt>Multimedia message deferred by %1.</qt>");
            } else if (mmsStatus.contains("Expired")) {
                msg = tr("<qt>Multimedia message to %1 expired.</qt>");
            } else {
                qLog(Messaging) << "Unknown X-MMS-Status:" << mmsStatus;
            }

            if (!msg.isEmpty()) {
                QString to = mailContent.headerFieldText("To");
                if (to.isEmpty())
                    to = tr("Unspecified", "MMS recipient");
                QMessageBox::information(0, tr("Multimedia message"), msg.arg(to), QMessageBox::Ok);
            }
            return;
        }
    }
#endif
#ifdef QTOPIA_HOMEUI
    if ((mail.messageType() == QMailMessage::Email) &&
        (mail.content() == QMailMessage::VoicemailContent)) {
        // Modify this message to appear as if it came directly from the caller
        QString originator;
        QString number;
        quint32 seconds = 0;
        QMailMessage::ContentType type = QMailMessage::VoicemailContent;

        QMailMessage message(mail.id());
        extractMessageDetails(message, &originator, &number, &seconds, &type);

        if (type != QMailMessage::VoicemailContent) {
            mail.setContent(type);
            modified = true;
        }

        QMailAddress addr;
        if (!number.isEmpty()) {
            // See if we can find a contact matching this number
            addr = QMailAddress(number);
        }
        if (!originator.isEmpty() && addr.matchContact().uid().isNull()) {
            // See if we can find a contact matching this string
            addr = QMailAddress(originator);
        }

        if (!addr.matchContact().uid().isNull()) {
            mail.setFrom(addr);
            modified = true;
        }
        if (seconds != 0) {
            QTime duration((seconds / 60 * 60) % 24, (seconds / 60) % 60, seconds % 60);
            QString durationText(duration.toString("m:ss"));

            if (type == QMailMessage::VideomailContent) {
                mail.setSubject(tr("[%1 Videomail]").arg(durationText));
            } else {
                mail.setSubject(tr("[%1 Voicemail]").arg(durationText));
            }
            modified = true;
        }
    }
#endif

    if (modified) {
        QMailStore::instance()->updateMessage(&mail);
    }

    // If this message is owned by the message server, move it to our inbox
    if (serverFolder->contains(mail.id())) {
        if (!inboxFolder->moveMessage(mail.id())) {
            cancelOperation();
            accessError(*inboxFolder);
        }
    }
}

void EmailClient::previewRetrievalCompleted()
{
    retrievalPhase = PreviewCompleted;

    if (arrivedMessageIds.isEmpty()) {
        if (mailAccountId.isValid()) {
            retrievalAction->completeRetrieval(mailDownloadList.mailIds());
            retrievalPhase = Completing;
        }
    }
}

void EmailClient::completionRetrievalCompleted()
{
    retrievalPhase = CompletionCompleted;

    if (arrivedMessageIds.isEmpty()) {
        if (mailAccountId.isValid()) {
            getNextNewMail();
        } else {
            autoGetMail = false;

            if (primaryActivity == Retrieving)
                clearStatusText();

            setRetrievalInProgress(false);
        }
    }
}

void EmailClient::getNextNewMail()
{
    if (!pendingAccountIds.isEmpty()) {
        mailAccountId = pendingAccountIds.takeFirst();
        getNewMail();
    } else {
        // We have processed all accounts
        autoGetMail = false;

        if (primaryActivity == Retrieving)
            clearStatusText();

        setRetrievalInProgress(false);

        if (!pendingHandlers.isEmpty()) {
            // We have an outstanding new message arrival event
            newMessageArrival();
        }
    }
}

void EmailClient::moveMailFront(const QMailMessageMetaData& message)
{
    if ( !(message.status() & QMailMessage::Incoming)
         || (message.status() & QMailMessage::Downloaded) )
        return;

    if ( isRetrieving() && (message.parentAccountId() == mailAccountId ) )
        mailDownloadList.moveFront( message.id() );
}

void EmailClient::sendFailure(const QMailAccountId &accountId)
{
    setSendingInProgress(false);

    Q_UNUSED(accountId)
}

void EmailClient::receiveFailure(const QMailAccountId &accountId)
{
    setRetrievalInProgress(false);

    autoGetMail = false;

    // Try the next account if we're working through a set of accounts
    if (!pendingAccountIds.isEmpty())
        getNextNewMail();

    Q_UNUSED(accountId)
}

void EmailClient::transferFailure(const QMailAccountId& accountId, const QString& text, int code)
{
    QString caption, action;
    if (isSending()) {
        caption = tr("Send Failure");
        action = tr("Error sending %1: %2", "%1: message type, %2: error text");
    } else if (isRetrieving()) {
        caption = autoGetMail ? tr("Automatic Fetch Failure") : tr("Retrieve Failure");
        action = tr("Error retrieving %1: %2", "%1: message type, %2: error text");
    }

    if (!action.isEmpty()) {
        if (accountId.isValid()) {
            QMailAccount account(accountId);
            QMailMessage::MessageType type(account.messageType());
            action = action.arg(mailType(type)).arg(text);

            // If we could have multiple accounts, name the relevant one
            if (type == QMailMessage::Email)
                action.prepend(" - ").prepend(account.accountName());
        } else {
            action = action.arg(tr("message")).arg(text);
        }

        qLog(Messaging) << "transferFailure:" << caption << '-' << action;
        if (code != QMailMessageServer::ErrCancel) {
            clearStatusText();
            QMessageBox::warning(0, caption, action, QMessageBox::Ok);
        } else {
            emit updateStatus(tr("Transfer cancelled"));
        }

        if (isSending()) {
            sendFailure(accountId);
        } else {
            receiveFailure(accountId);
        }
    }
}

QString EmailClient::mailType(QMailMessage::MessageType type)
{
    QString key(QMailComposerFactory::defaultKey(type));
    if (!key.isEmpty())
        return QMailComposerFactory::displayName(key, type);

    return tr("Message");
}

void EmailClient::messageActivated()
{
    static const QMailFolderId draftsFolderId = messageStore()->mailbox(QMailFolder::DraftsFolder)->mailFolder().id();

    QMailMessageId currentId = messageListView()->current();
    if(!currentId.isValid())
        return;

    MessageFolder* source(containingFolder(currentId));
    if (source && (source->id() == draftsFolderId)) {
        QMailMessage message(currentId);
        modify(message);
    } else {
        viewMessage(currentId, true);
    }

    QtopiaIpcEnvelope e( "QPE/TaskBar", "setLed(int,bool)" );
    e << LED_MAIL << false;
}

void EmailClient::acknowledgeMessageArrivals()
{
    // If no handlers are pending, we can't update the server
    if (!pendingHandlers.isEmpty()) {
        // Tell the server that we have handled the message arrival
        foreach (QMailNewMessageHandler *handler, pendingHandlers) {
            handler->setHandled(true);
        }
        pendingHandlers.clear();

        // Clear our recorded counts
        QMailMessageCountMap::iterator it = newMessageCounts.begin(), end = newMessageCounts.end();
        for ( ; it != end; ++it)
            it.value() = 0;

        if (initialAction == IncomingMessages) {
            // We've finished doing this
            initialAction = None;
        }
    }
}

void EmailClient::accessError(const MessageFolder &box)
{
    QString msg = tr("<qt>Cannot access %1. Either there is insufficient space, or another program is accessing the mailbox.</qt>").arg(box.mailbox());

    QMessageBox::critical( 0, tr("Save error"), msg );
}

void EmailClient::copyError(const MessageFolder& dest)
{
    QString msg = tr("<qt>Cannot copy message to %1. Either there is insufficient space, or another program is accessing the folders.</qt>").arg(dest.mailbox());

    QMessageBox::critical( 0, tr("Copy error"), msg );
}

void EmailClient::moveError(const MessageFolder& dest)
{
    QString msg = tr("<qt>Cannot move message to %1. Either there is insufficient space, or another program is accessing the folders.</qt>").arg(dest.mailbox());

    QMessageBox::critical( 0, tr("Move error"), msg );
}

void EmailClient::readSettings()
{
    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("qtmailglobal");
    defaultAccountId = QMailAccountId(mailconf.value("defaultAccountId").toULongLong());
    ignoredMessageCount = mailconf.value("ignoredMessageCount").toULongLong();
    mailconf.endGroup();

    mailconf.beginGroup("settings");
    int val = mailconf.value("interval", -1 ).toInt();
    if ( val == -1 ) {
        fetchTimer.stop();
    } else {
        fetchTimer.start( val * 60 * 1000);
    }
    mailconf.endGroup();
}

bool EmailClient::saveSettings()
{
    const int QTMAIL_CONFIG_VERSION = 100;
    QSettings mailconf("Trolltech","qtmail");

    mailconf.beginGroup("qtmailglobal");
    mailconf.remove("");
    mailconf.setValue("version", QTMAIL_CONFIG_VERSION );
    mailconf.endGroup();

    mailconf.beginGroup("qtmailglobal");

    mailconf.setValue("defaultAccountId", defaultAccountId.toULongLong() );
    mailconf.setValue("ignoredMessageCount", ignoredMessageCount );
    mailconf.endGroup();
    return true;
}

void EmailClient::updateGetMailButton()
{
    bool visible(false);

    // We can get only mail if we're currently inactive
    if (!isTransmitting()) {
        // At least one account must be able to retrieve mail
        foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) {
            QMailAccount account(id);
            if (account.canCollectMail()) {
                visible = true;
                break;
            }
        }
    }

    setActionVisible(getMailButton, visible);

    updateGetAccountButton();
}

void EmailClient::updateGetAccountButton()
{
    // We can get only mail if we're currently inactive
    bool inactive(!isTransmitting());

    if (currentLocation() == FolderList) {
        if (QMailMessageSet* item = folderView()->currentItem()) {
            QMailAccountId accountId(item->data(EmailFolderModel::ContextualAccountIdRole).value<QMailAccountId>());
            bool accountContext(accountId.isValid());

            // Only show the get mail for account button if there are multiple accounts to retrieve from
            bool multipleMailAccounts = (emailAccounts().count() > 1);
            setActionVisible(getAccountButton, (inactive && accountContext && multipleMailAccounts));
        }
    }
}

void EmailClient::updateAccounts()
{
    queuedAccountIds.clear();
    updateGetMailButton();
}

bool EmailClient::copyMessage(const QMailMessageId& id, MessageFolder *target)
{
    if (!target->copyMessage(id)) {
        copyError(*target);
        return false;
    }

    return true;
}

bool EmailClient::copyMessages(const QMailMessageIdList& ids, MessageFolder *target)
{
    if (!target->copyMessages(ids)) {
        copyError(*target);
        return false;
    }

    return true;
}

bool EmailClient::moveMessage(const QMailMessageId& id, MessageFolder *target)
{
    if (!target->moveMessage(id)) {
        moveError(*target);
        return false;
    }

    return true;
}

bool EmailClient::moveMessages(const QMailMessageIdList& ids, MessageFolder *target)
{
    if (!target->moveMessages(ids)) {
        moveError(*target);
        return false;
    }

    return true;
}

bool EmailClient::restoreMessages(const QMailMessageIdList& ids, MessageFolder*)
{
    QMailStore::instance()->restoreToPreviousFolder(QMailMessageKey(ids));
    return true;
}

bool EmailClient::deleteMessages(const QMailMessageIdList& ids, MessageFolder*)
{
    QMailStore::instance()->removeMessages(QMailMessageKey(ids), QMailStore::CreateRemovalRecord);
    return true;
}

bool EmailClient::confirmDeleteWithoutSIM(int deleteCount)
{
    QString text(tr("The SIM card is not ready. Do you want to delete the message(s) without removal from the SIM card?",
                    "",
                    deleteCount));

    return (QMessageBox::question(0,
                                  tr("SIM not ready"),
                                  text,
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::Yes);
}

void EmailClient::deleteSelectedMessages()
{
    clearNewMessageStatus(QMailMessageKey(messageListView()->selected()));
    static MessageFolder* const outboxFolder = messageStore()->mailbox( QMailFolder::OutboxFolder );
    static MessageFolder* const trashFolder = messageStore()->mailbox( QMailFolder::TrashFolder );

    // Do not delete messages from the outbox folder while we're sending
    if (locationSet.contains(outboxFolder->id()) && isSending())
        return;

    QMailMessageIdList deleteList = messageListView()->selected();
    int deleteCount = deleteList.count();
    if (deleteCount == 0)
        return;

    const bool deleting((locationSet.count() == 1) && (*locationSet.begin() == trashFolder->id()));

    if (deleteCount >= MinimumForProgressIndicator) {
        QString caption;
        if (deleting) {
            caption = tr("Deleting message(s)", "", deleteCount);
        } else {
            caption = tr("Moving message(s)", "", deleteCount);
        }

        emit updateProgress(0, deleteCount);
        emit updateStatus(caption);
        qApp->processEvents();
    } else {
        // Tell the user we're doing what they asked for
        QString action;
        QString actionDetails;
        if ( deleting ) {
            QString item(tr("%n message(s)", "%1: number of messages", deleteCount));
            if ( !Qtopia::confirmDelete( this, tr("Delete"), item ) )
                return;

            action = tr("Deleting");
            actionDetails = tr("Deleting %n message(s)", "%1: number of messages", deleteCount);
        } else {
            // Received messages will be removed from SIM on move to Trash
            QMailMessageKey smsKey(QMailMessageKey::Type,QMailMessage::Sms);
            QMailMessageKey incomingKey(QMailMessageKey::Status, QMailMessage::Incoming, QMailDataComparator::Includes);

            int smsCount = QMailStore::instance()->countMessages(QMailMessageKey(deleteList) & smsKey & incomingKey);
            if ((smsCount > 0) && !smsReady.value().toBool()) {
                if (confirmDeleteWithoutSIM(deleteCount) == false)
                    return;
            }

            action = tr("Moving");
            actionDetails = tr("Moving %n message(s) to Trash", "%1: number of messages", deleteCount);
        }

        AcknowledgmentBox::show(action, actionDetails);
    }

    if (deleting)
        applyToList(&EmailClient::deleteMessages, deleteList);
    else
        applyToList(&EmailClient::moveMessages, deleteList, trashFolder);

    if (markingMode) {
        // After deleting the messages, clear marking mode
        setMarkingMode(false);
    }
}

void EmailClient::moveSelectedMessagesTo(MessageFolder* destination)
{
    QMailMessageIdList moveList = messageListView()->selected();
    clearNewMessageStatus(QMailMessageKey(moveList));
    if ( moveList.isEmpty() )
        return;

    if (moveList.count() >= MinimumForProgressIndicator) {
        emit updateProgress(0, moveList.count());
        emit updateStatus(tr("Moving message(s)", "", moveList.count()));
        qApp->processEvents();
    }

    applyToList(&EmailClient::moveMessages, moveList, destination);
}

void EmailClient::copySelectedMessagesTo(MessageFolder* destination)
{

    QMailMessageIdList copyList = messageListView()->selected();
    clearNewMessageStatus(QMailMessageKey(copyList));
    if ( copyList.isEmpty() )
        return;

    unsigned int size = QMailStore::instance()->sizeOfMessages(QMailMessageKey(copyList));
    if (!LongStream::freeSpace( "", size + 1024*10 )) {
        QString title( tr("Copy error") );
        QString msg( "<qt>" + tr("Storage for messages is full.<br><br>Could not copy messages.") + "</qt>" );
        QMessageBox::warning(0, title, msg, tr("OK") );
        return;
    }

    if (copyList.count() >= MinimumForProgressIndicator) {
        emit updateProgress(0, copyList.count());
        emit updateStatus(tr("Copying message(s)", "", copyList.count()));
        qApp->processEvents();
    }

    applyToList(&EmailClient::copyMessages, copyList, destination);
}

bool EmailClient::foreachListElement(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*),
                                     const QMailMessageIdList& list, MessageFolder *target)
{
    bool result(true);
    const int count(list.count());

    QTime time;
    int progress = 0;
    foreach (const QMailMessageId& id, list) {
        // Process the next item
        result &= (this->*func)(QMailMessageIdList() << id, target);
        ++progress;

        // We still need to process events during this loop
        if ((progress == 1) || (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
            emit updateProgress(progress, count);
            qApp->processEvents();
            time.start();
        }
    }

    clearStatusText();

    return result;
}

bool EmailClient::foreachListBatch(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*),
                                   const QMailMessageIdList& list, MessageFolder *target, int batchSize)
{
    bool result(true);
    const int count(list.count());

    QTime time;
    int progress = 0;
    while (progress < count) {
        // Process the next batch
        result &= (this->*func)(list.mid(progress, batchSize), target);
        progress += batchSize;
        
        // We still need to process events during this loop
        if ((progress == batchSize) || (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
            emit updateProgress(qMin(progress, count), count);
            qApp->processEvents();
            time.start();
        }
    }

    clearStatusText();

    return result;
}

bool EmailClient::applyToList(bool (EmailClient::*func)(const QMailMessageIdList&, MessageFolder*),
                              const QMailMessageIdList& list, MessageFolder *target)
{
    bool result;
    const int count(list.count());

    suspendMailCounts();

    if (count >= BatchMinimumForProgressIndicator) {
        // Process this list in batches of roughly equivalent size
        int batchCount = (count / MaxBatchSize) + (count % MaxBatchSize ? 1 : 0);
        int batchSize = ((count / batchCount) + (count % batchCount ? 1 : 0));

        result = foreachListBatch(func, list, target, batchSize);
    } else if (count >= MinimumForProgressIndicator) {
        // Process this list's item individually
        result = foreachListElement(func, list, target);
    } else {
        // No progress indication is required; just perform the task in one batch
        result = (this->*func)(list, target);
    }

    resumeMailCounts();

    return result;
}

bool EmailClient::applyToSelectedFolder(void (EmailClient::*function)(MessageFolder*))
{
    static MessageFolder* const outboxFolder = messageStore()->mailbox( QMailFolder::OutboxFolder );
    static MessageFolder* const serverFolder = messageStore()->serverMailbox();

    if (!locationSet.isEmpty()) {
        QMailFolderIdList list = messageStore()->standardFolders();

        // If the message(s) are in a single location, do not permit that as a destination
        if (locationSet.count() == 1) {
            QMailFolder folder(*locationSet.begin());
            list.removeAll(folder.id());
        }

        // Also, do not permit messages to be copied/moved to the Outbox manually
        list.removeAll(outboxFolder->id());

        // Do not allow the server's folder to be selected
        list.removeAll(serverFolder->id());

        SelectFolderDialog selectFolderDialog(list);
        QtopiaApplication::execDialog( &selectFolderDialog );

        if (selectFolderDialog.result() == QDialog::Accepted) {
            // Apply the function to the selected messages, with the selected folder argument
            (this->*function)(messageStore()->mailbox(selectFolderDialog.selectedFolderId()));
            return true;
        }
    }

    return false;
}

void EmailClient::moveSelectedMessages()
{
    static MessageFolder* const outboxFolder = messageStore()->mailbox( QMailFolder::OutboxFolder );

    // Do not move messages from the outbox folder while we're sending
    if (locationSet.contains(outboxFolder->id()) && isSending())
        return;

    if (applyToSelectedFolder(&EmailClient::moveSelectedMessagesTo)) {
        if (markingMode) {
            // After moving the messages, clear marking mode
            setMarkingMode(false);
        }
    }
}

void EmailClient::copySelectedMessages()
{
    if (applyToSelectedFolder(&EmailClient::copySelectedMessagesTo)) {
        if (markingMode) {
            // After copying the messages, clear marking mode
            setMarkingMode(false);
        }
    }
}

void EmailClient::restoreSelectedMessages()
{
    static const QMailFolderId trashFolderId = messageStore()->mailbox(QMailFolder::TrashFolder)->id();

    QMailMessageIdList restoreList;
    foreach (const QMailMessageId &id, messageListView()->selected()) {
        // Only messages currently in the trash folder should be restored
        QMailMessageMetaData message(id);
        if (message.parentFolderId() == trashFolderId)
            restoreList.append(id);
    }
        
    if (restoreList.isEmpty())
        return;

    if (restoreList.count() >= MinimumForProgressIndicator) {
        emit updateProgress(0, restoreList.count());
        emit updateStatus(tr("Restoring message(s)", "", restoreList.count()));
        qApp->processEvents();
    }

    applyToList(&EmailClient::restoreMessages, restoreList);
}

void EmailClient::selectAll()
{
    if (!markingMode) {
        // No point selecting messages unless we're in marking mode
        setMarkingMode(true);
    }

    messageListView()->selectAll();
}

void EmailClient::emptyTrashFolder()
{
    QMailMessage::MessageType type = QMailMessage::AnyType;
    if (currentLocation() == ActionList) {
        type = nonEmailType;
    } else if (currentLocation() == FolderList) {
        type = QMailMessage::Email;
    }

    static MessageFolder* const trashFolder = messageStore()->mailbox(QMailFolder::TrashFolder);

    QMailMessageIdList trashIds = trashFolder->messages(type);
    if (trashIds.isEmpty())
        return;

    if (Qtopia::confirmDelete(this, "Empty trash", tr("all messages in the trash"))) {
        int count = trashIds.count();
        if (count >= MinimumForProgressIndicator) {
            emit updateProgress(0, count);
            emit updateStatus(tr("Deleting %n message(s)", "%1: number of messages", count));
            qApp->processEvents();
        }

        applyToList(&EmailClient::deleteMessages, trashIds);
    }
}

void EmailClient::setStatusText(QString &txt)
{
    emit updateStatus(txt);
}

void EmailClient::connectivityChanged(QMailServiceAction::Connectivity /*connectivity*/)
{
}

void EmailClient::activityChanged(QMailServiceAction::Activity activity)
{
    if (QMailServiceAction *action = static_cast<QMailServiceAction*>(sender())) {
        if (activity == QMailServiceAction::Successful) {
            if (action == transmitAction) {
                transmitCompleted();
            } else if (action == retrievalAction) {
                retrievalCompleted();
            } else if (action == searchAction) {
                searchCompleted();
            }
        } else if (activity == QMailServiceAction::Failed) {
            const QMailServiceAction::Status status(action->status());
            transferFailure(status.accountId, status.text, status.errorCode);
        }
    }
}

void EmailClient::statusChanged(const QMailServiceAction::Status &status)
{
    if (QMailServiceAction *action = static_cast<QMailServiceAction*>(sender())) {
        // If we have completed, don't show the status info
        if (action->activity() == QMailServiceAction::InProgress) {
            QString text = status.text;
            if (status.accountId.isValid()) {
                QMailAccount account(status.accountId);
                text.prepend(account.displayName() + " - ");
            }
            emit updateStatus(text);
        }
    }
}

void EmailClient::progressChanged(uint progress, uint total)
{
    emit updateProgress(progress, total);
}

void EmailClient::leaveLocation()
{
    MessageUiBase::Location loc(currentLocation());

    if ((loc == MessageList) || (loc == SearchResults)) {
        // When leaving the message list, ensure displayed messages are not marked new any longer
        clearNewMessageStatus(messageListView()->key());
    } else if (loc == Viewer) {
        if (!flashMessageIds.isEmpty()) {
            UILocation location(::currentLocation());
            QMailMessageId id(location.at(1).value<QMailMessageId>());

            if (id == flashMessageIds.top()) {
                flashMessageIds.pop();
            }
        }
    }

    MessageUiBase::leaveLocation();
}

void EmailClient::actionSelected(QMailMessageSet *item)
{
    if (item)
        contextStatusUpdate();
}

void EmailClient::folderSelected(QMailMessageSet *item)
{
    if (item) {
        contextStatusUpdate();

        bool synchronizeAvailable(false);

        QMailAccountId accountId(item->data(EmailFolderModel::ContextualAccountIdRole).value<QMailAccountId>());
        if (accountId.isValid()) {
            QMailAccount account(accountId);
            getAccountButton->setText(tr("Get mail for %1", "%1:account name").arg(account.displayName()));
            getAccountButton->setData(accountId);

            // See if this is a folder that can be included/excluded
            QMailFolderId folderId(item->data(EmailFolderModel::FolderIdRole).value<QMailFolderId>());
            if (folderId.isValid()) {
                synchronizeAction->setData(folderId);
                synchronizeAvailable = true;

                if (item->data(EmailFolderModel::FolderSynchronizationEnabledRole).value<bool>())
                    synchronizeAction->setText(tr("Exclude folder"));
                else
                    synchronizeAction->setText(tr("Include folder"));
            }
        }

        setActionVisible(synchronizeAction, synchronizeAvailable);

        updateGetAccountButton();
    }
}

#ifndef QTOPIA_HOMEUI
void EmailClient::actionActivated(QMailMessageSet *item)
{
    if (item) {
        viewMessageList(item->messageKey(), item->displayName());
    }
}

void EmailClient::folderActivated(QMailMessageSet *item)
{
    if (item) {
        viewMessageList(item->messageKey(), item->displayName());
    }
}
#endif //QTOPIA_HOMEUI

void EmailClient::search()
{
#ifdef QTOPIA_HOMEUI
    messageListView()->setDisplayMode(MessageListView::DisplayFilter);
#else
    if (!searchView) {
        searchView = new SearchView(this);
        searchView->setObjectName("search"); // No tr
        connect(searchView, SIGNAL(accepted()), this, SLOT(searchRequested()));
    }

    searchView->reset();

    QtopiaApplication::execDialog(searchView);
#endif //QTOPIA_HOMEUI
}

void EmailClient::searchRequested()
{
#ifndef QTOPIA_HOMEUI
    QString bodyText(searchView->bodyText());

    QMailMessageKey searchKey(searchView->searchKey());
    QMailMessageKey emailKey(QMailMessageKey::Type, QMailMessage::Email, QMailDataComparator::Includes);

    if (currentLocation() == ActionList) {
        searchKey &= ~emailKey;
    } else if (currentLocation() == FolderList) {
        searchKey &= emailKey;
    }
    searchProgressDialog()->show();
    searchAction->searchMessages(searchKey, bodyText);
    emit updateStatus(tr("Searching"));
#endif //QTOPIA_HOMEUI
}

void EmailClient::automaticFetch()
{
    if (isRetrieving())
        return;

    // TODO: remove this code - obsoleted by messageserver interval checking

    qWarning("get all new mail automatic");
    autoGetMail = true;
    getAllNewMail();
}

/*  TODO: Is external edit still relevant?

    Someone external are making changes to the mailboxes.  By this time
    we won't know what changes has been made (nor is it feasible to try
    to determine it).  Close all actions which can have become
    invalid due to the external edit.  A writemail window will as such close, but
    the information will be kept in memory (pasted when you reenter the
    writemail window (hopefully the external edit is done by then)
*/
void EmailClient::externalEdit(const QString &mailbox)
{
    cancelOperation();
#ifdef QTOPIA_HOMEUI
    viewMessageList(messageListView()->displayMode());
#else
    viewMessageList(messageListView()->key());
#endif

    QString msg = mailbox + " "; //no tr
    msg += tr("was edited externally");
    emit updateStatus(msg);
}

bool EmailClient::checkMailConflict(const QString& msg1, const QString& msg2)
{
    if ( currentLocation() == Composer ) {
        QString message = tr("<qt>You are currently editing a message:<br>%1</qt>").arg(msg1);
        switch( QMessageBox::warning( 0, tr("Messages conflict"), message,
                                      tr("Yes"), tr("No"), 0, 0, 1 ) ) {

            case 0:
            {
                if ( !writeMailWidget()->saveChangesOnRequest() ) {
                    QMessageBox::warning(0,
                                        tr("Autosave failed"),
                                        tr("<qt>Autosave failed:<br>%1</qt>").arg(msg2));
                    return true;
                }
                break;
            }
            case 1: break;
        }
    }
    return false;
}

void EmailClient::writeMailAction(const QMap<QString, QString> propertyMap )
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should it be saved in Drafts before writing the new message?"),
            tr("'Write Mail' message will be ignored")) )
        return;

    QMailMessage mail;

    // Set all the properties defined in the supplied map
    EmailPropertySetter setter( mail );
    setter.setProperties( propertyMap );

    modify( mail );

    openFiles();
}

void EmailClient::smsVCard( const QDSActionRequest& request )
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    writeSmsAction( QString(), QString(), request.requestData().toString(), true );

    QDSActionRequest( request ).respond();
}

void EmailClient::smsVCard(const QString& filename, const QString& description)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    QFile f( filename );
    if (! f.open(QIODevice::ReadOnly) ) {
        qWarning("could not open file: %s", filename.toLatin1().constData() );
    } else {
        QString body = QString::fromLocal8Bit( f.readAll() );
        writeSmsAction( QString(), QString(), body, true );
    }

    Q_UNUSED(description)
}

void EmailClient::writeSmsAction(const QString&, const QString& number,
                                 const QString& body, bool vcard)
{
#ifndef QTOPIA_NO_SMS
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should this message be saved in Drafts before writing the new message?"),
            tr("'Write SMS' message will be ignored")) )
        return;

    if (writeMailWidget()->newMail(QMailMessage::Sms, vcard)) {
        if (!number.isEmpty()) {
            writeMailWidget()->setSmsRecipient( number );
        }

        if (!body.isNull()) {
            writeMailWidget()->setBody(body, vcard ? QLatin1String("text/x-vCard")
                                                   : QLatin1String("text/plain"));
        }

        viewComposer();

        openFiles();
    }
#else
    Q_UNUSED(number);
    Q_UNUSED(body);
    Q_UNUSED(vcard);
#endif
}

void EmailClient::writeMailAction(const QString& name, const QString& address)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should this message be saved in Drafts before writing the new message?"),
            tr("'Write Mail' message will be ignored")) )
        return;

    writeMailWidget()->newMail(QMailMessage::Email);
    if ( writeMailWidget()->composer().isEmpty() ) {
        // failed to create new composer, maybe due to no email account
        // being present. So hide/quit qtmail.
        closeApplication();
        return;
    }
    writeMailWidget()->setRecipient( QMailAddress(name, address).toString() );
    viewComposer();

    openFiles();
}

void EmailClient::writeMailAction(const QString &name,
                                  const QString &addrStr,
                                  const QStringList &docAttachments,
                                  const QStringList &fileAttachments)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    writeMessageAction(name, addrStr, docAttachments, fileAttachments, QMailMessage::Email);
}

void EmailClient::emailVCard(const QByteArray& data)
{
    QString leafname("email");

    QList<QContact> cardData( QContact::readVCard( data ) );
    if (!cardData.isEmpty()) {
        const QContact& contact = cardData.first();
        QString name(contact.firstName() + contact.lastName());
        if (!name.isEmpty()) {
            // Remove any non-word chars to ensure we have a valid filename
            leafname = name.remove(QRegExp("\\W"));
        }
    }

    leafname += ".vcf";

    // Save the VCard data to a temporary document
    QString filename = Qtopia::tempDir() + leafname;
    {
        QFile temp( filename );
        if ( !temp.open( QIODevice::WriteOnly ) ) {
            qWarning() << "Unable to open path for write:" << filename;
            return;
        }

        temp.write( data );
        temp.close();
    }

    QContent doc( filename );
    doc.setName( leafname );
    doc.setRole( QContent::Data );
    doc.commit();

    // write the Email
    composeMessage(QMailMessage::Email,
                   QMailAddressList(),
                   QString(),
                   QString(),
                   (QContentList() << doc),
                   QMailMessage::CopyAndDeleteAttachments,
                   true);
}

void EmailClient::emailVCard(const QString& filename, const QString& description)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    writeMessageAction( QString(),
                        QString(),
                        QStringList(),
                        QStringList( filename ),
                        QMailMessage::Email );

    Q_UNUSED(description)
}

void EmailClient::emailVCard( const QDSActionRequest& request )
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    emailVCard( request.requestData().data() );

    // Respond to the request
    QDSActionRequest( request ).respond();
}

void EmailClient::writeMessageAction(const QString &name, 
                                     const QString &addrStr, 
                                     const QStringList &docAttachments, 
                                     const QStringList &fileAttachments, 
                                     int type)
{
    QMailMessage::MessageType diid = QMailMessage::AnyType;
    if ((type == QMailMessage::Sms) || 
        (type == QMailMessage::Mms) ||
        (type == QMailMessage::Email) || 
        (type == QMailMessage::Instant))
        diid = static_cast<QMailMessage::MessageType>(type);

    QList<QMailAddress> addresses;

    if (!addrStr.isEmpty()) {
        foreach (const QMailAddress& addr, QMailAddress::fromStringList(addrStr)) {
            if ((addr.isEmailAddress()) &&
                (addr.name() == addr.address()) &&
                !name.isEmpty()) {
                // This address has no specific name - use the supplied name
                addresses.append(QMailAddress(name, addr.address()));
            } else {
                addresses.append(addr);
            }
        }
    }

    QList<QContent> attachments;
    foreach (const QString& doc, docAttachments)
        attachments.append(QContent(doc, false));
    foreach (const QString& file, fileAttachments)
        attachments.append(QContent(file, false));

    composeMessage(diid,
                   addresses,
                   QString(),
                   QString(),
                   attachments,
                   QMailMessage::LinkToAttachments);
}

void EmailClient::cleanupMessages( const QDate &removalDate, int removalSize )
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Cleanup;
    }

    bool closeAfterCleanup = isHidden();

    openFiles();

    // Removal size is specified in KB
    QMailMessageKey statusFilter(QMailMessageKey::Status, QMailMessage::Downloaded, QMailDataComparator::Includes);
    QMailMessageKey sizeFilter(QMailMessageKey::Size, removalSize * 1024, QMailDataComparator::GreaterThanEqual);
    QMailMessageKey dateFilter(QMailMessageKey::TimeStamp, removalDate, QMailDataComparator::LessThan);

    // Delete messages matching filters
    QMailStore::instance()->removeMessages(statusFilter & sizeFilter & dateFilter, QMailStore::NoRemovalRecord);

    if (closeAfterCleanup) {
        closeAfterTransmissionsFinished();
        closeApplication();
    }
}

void EmailClient::resend(const QMailMessage& message, int replyType)
{
    repliedFromMailId = message.id();

#ifdef QTOPIA_HOMEUI
    if (!replyType) {
        ReplyDialog replyDialog(this);
        QtopiaApplication::execDialog(&replyDialog);
        int result = replyDialog.result();
        if (result == QDialog::Accepted) {
            repliedFlags = replyDialog.messageStatusFlag();
            replyType = replyDialog.composeAction();
        } else {
            return;
        }
    } else
#endif
    if (replyType == ReadMail::Reply) {
        repliedFlags = QMailMessage::Replied;
    } else if (replyType == ReadMail::ReplyToAll) {
        repliedFlags = QMailMessage::RepliedAll;
    } else if (replyType == ReadMail::Forward) {
        repliedFlags = QMailMessage::Forwarded;
    } else {
        return;
    }
    writeMailWidget()->reply(message, replyType);
    if ( writeMailWidget()->composer().isEmpty() ) {
        // failed to create new composer, maybe due to no email account
        // being present.
        return;
    }
    viewComposer();
}

void EmailClient::modify(const QMailMessage& message)
{
    // Is this type editable?
    QString key(QMailComposerFactory::defaultKey(message.messageType()));
    if (!key.isEmpty()) {
        writeMailWidget()->modify(message);
        if ( writeMailWidget()->composer().isEmpty() ) {
            // failed to create new composer, maybe due to no email account
            // being present.
            return;
        }
        viewComposer();
    } else {
        QMessageBox::warning(0,
                             tr("Error"),
                             tr("Cannot edit a message of this type."),
                             tr("OK"));
    }
}

void EmailClient::replyToMessage(const QMailMessageId &id)
{
    QMailMessage message(id);

#ifdef QTOPIA_HOMEUI
    resend(message, 0);
#else
    resend(message, ReadMail::Reply);
#endif    
}

void EmailClient::composeActivated()
{
    delayedInit();

    if (writeMailWidget()->newMail()) {
        viewComposer();
    }
}

void EmailClient::sendMessageTo(const QMailAddress &address, QMailMessage::MessageType type)
{
    if (type == QMailMessage::AnyType)
        type = QMailMessage::Email;

    // Some address types imply message types
    if (address.isEmailAddress() && ((type != QMailMessage::Email) && (type != QMailMessage::Mms))) {
        type = QMailMessage::Email;
#ifndef QTOPIA_NO_SMS
    } else if (address.isPhoneNumber() && ((type != QMailMessage::Sms) && (type != QMailMessage::Mms))) {
        type = QMailMessage::Sms;
#endif
#ifndef QTOPIA_NO_COLLECTIVE
    } else if (address.isChatAddress()) {
        type = QMailMessage::Instant;
#endif
    }

    if (writeMailWidget()->newMail(type)) {
        writeMailWidget()->setRecipient(address.address());
        viewComposer();
    }
}

bool EmailClient::removeMessage(const QMailMessageId& id, bool userRequest)
{
    static const QMailFolderId outboxFolderId = messageStore()->mailbox(QMailFolder::OutboxFolder)->id();
    static MessageFolder* const trashFolder = messageStore()->mailbox(QMailFolder::TrashFolder);

    MessageFolder* source(containingFolder(id));
    if (isSending()) {
        // Don't delete from Outbox when sending
        if (source && (source->id() == outboxFolderId))
            return false; 
    }

    bool flashDelete(false);
    QMailMessageMetaData message(id);

    if (message.messageType() == QMailMessage::Sms) {
        if (userRequest) {
            if (!smsReady.value().toBool() && (message.status() & QMailMessage::Incoming)) {
                // The SIM card is not ready
                if (confirmDeleteWithoutSIM(1) == false)
                    return false;
            }
        } else {
            // This is a flash message auto-deletion
            if (flashMessageIds.contains(id) &&
                (QMessageBox::question(0, 
                                       tr("Flash message"),
                                       tr("Do you wish to save this Flash message?"),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes) == QMessageBox::No)) {
                flashDelete = true;
            } else {
                return false;
            }
        }
    }

    bool deleting(flashDelete);
    QString type = mailType(message.messageType());
    if (!deleting) {
        if (source && (source->id() == trashFolder->id())) {
            if (!Qtopia::confirmDelete( this, "Delete", type ))
                return false;

            deleting = true;
        }
    }

    // If mail is in queue for download, remove it from queue if possible
    mailDownloadList.remove(id);

    if ( deleting ) {
        if (!flashDelete) {
            AcknowledgmentBox::show(tr("Deleting"), tr("Deleting: %1","%1=Email/Message/MMS").arg(type));
        }

        return trashFolder->deleteMessage(id);
    } else {
        AcknowledgmentBox::show(tr("Moving"), tr("Moving to Trash: %1", "%1=Email/Message/MMS").arg(type));
        return moveMessage(id, trashFolder);
    }
}

void EmailClient::viewNextMessage()
{
    messageListView()->setNextCurrent();

    popLocation();
    viewMessage(messageListView()->current(), true);
}

void EmailClient::viewPreviousMessage()
{
    messageListView()->setPreviousCurrent();

    popLocation();
    viewMessage(messageListView()->current(), true);
}

void EmailClient::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    clearStatusText();

    if ((currentLocation() == NoLocation) && (initialAction == None)) {
        closeAfterTransmissions = false;

        // We have been launched and raised by QPE in response to a user request
        userInvocation();
    }

    suspendMailCount = false;

    QTimer::singleShot(0, this, SLOT(delayedInit()) );
}

void EmailClient::userInvocation()
{
#ifndef QTOPIA_HOMEUI
    // Start in the action list
    viewActionList();

    // Since the action list hasn't been created until now, it wasn't given focus
    // before the application was shown.  Give it focus now.
    actionView()->setFocus();

    // See if there is a draft whose composition was interrupted by the Red Key (tm)
    QTimer::singleShot(0, this, SLOT(resumeInterruptedComposition()));
#endif
}

void EmailClient::setSendingInProgress(bool set)
{
    if (currentLocation() == Viewer)
        readMailWidget()->setSendingInProgress(set);

    if (set) {
        if (!isRetrieving())
            primaryActivity = Sending;
    } else {
        if (primaryActivity == Sending)
            primaryActivity = Inactive;

        // Anything we could not send should move back to the drafts folder
        clearOutboxFolder();
    }

    if (isSending() != set) {
        int newStatus = (set ? transferStatus | Sending : transferStatus & ~Sending);
        transferStatusUpdate(newStatus);
    }
}

void EmailClient::setRetrievalInProgress(bool set)
{
    if (currentLocation() == Viewer)
        readMailWidget()->setRetrievalInProgress(set);

    if (set) {
        if (!isSending())
            primaryActivity = Retrieving;
    } else {
        if (primaryActivity == Retrieving)
            primaryActivity = Inactive;

        retrievalPhase = None;
    }

    if (isRetrieving() != set) {
        int newStatus = (set ? transferStatus | Retrieving : transferStatus & ~Retrieving);
        transferStatusUpdate(newStatus);
    }
}

void EmailClient::transferStatusUpdate(int status)
{
    if (status != transferStatus) {
        transferStatus = status;

        // See if we need to enable or disable suspend
        setSuspendPermitted(transferStatus == Inactive);

        if (transferStatus != Inactive) {
            if (!taskRegistered("transfer"))
                registerTask("transfer");
        } else {
            unregisterTask("transfer");

            if (closeAfterTransmissions)
                QTMailWindow::singleton()->close();
        }

        // UI updates
        setActionVisible(cancelButton, transferStatus != Inactive);
        updateGetMailButton();
    }
}

void EmailClient::setSuspendPermitted(bool ok)
{
    if (ok != suspendStatus) {
        QtopiaApplication::setPowerConstraint(ok ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
        suspendStatus = ok;
    }
}

void EmailClient::clearOutboxFolder()
{
    static MessageFolder* const outbox = messageStore()->mailbox(QMailFolder::OutboxFolder);
    static MessageFolder* const sent = messageStore()->mailbox(QMailFolder::SentFolder);
    static MessageFolder* const drafts = messageStore()->mailbox(QMailFolder::DraftsFolder);

    // Move any sent messages to the sent folder
    moveMessages(outbox->messages(QMailMessage::Sent, true), sent);

    // Move any messages stuck in the outbox to the drafts folder
    moveMessages(outbox->messages(QMailMessage::Sent, false), drafts);
}

void EmailClient::emailActivated()
{
    delayedInit();

    viewFolderList(tr("Email"));
}

void EmailClient::contextStatusUpdate()
{
    if (isTransmitting())
        return;

    MessageUiBase::contextStatusUpdate();
}

void EmailClient::settings()
{
    AccountSettings settings(this, "create-account", false, defaultAccountId);
    connect(&settings, SIGNAL(deleteAccount(QMailAccountId)),
            this, SLOT(deleteAccount(QMailAccountId)));

    settings.showMaximized();
    QtopiaApplication::execDialog(&settings);
    defaultAccountId = settings.defaultAccountId();
    writeMailWidget()->setDefaultAccount(defaultAccountId);

    clearStatusText();
    contextStatusUpdate();
}

void EmailClient::deleteAccount(const QMailAccountId& id)
{
    QMailAccount account(id);

    // We could simply delete the account since QMailStore::deleteAccount
    // will remove all folders and messages, but for now we will remove the
    // messages manually so we can give progress indication (eventually, we
    // might add progress notification to QMailStore)

    suspendMailCounts();

    // Remove the messages and folders from this account (not just in the Inbox)
    QMailMessageIdList removedMessageIds = messageStore()->messagesFromAccount(account);
    if (!removedMessageIds.isEmpty()) {
        if  (removedMessageIds.count() >= MinimumForProgressIndicator) {
            emit updateProgress(0, removedMessageIds.count());
            emit updateStatus(tr("Deleting messages"));
            qApp->processEvents();
        }

        applyToList(&EmailClient::deleteMessages, removedMessageIds);
    }

    QMailStore::instance()->removeAccount(account.id());

    resumeMailCounts();
}

void EmailClient::accountsAdded(const QMailAccountIdList&)
{
    updateGetAccountButton();
    updateAccounts();
}

void EmailClient::accountsRemoved(const QMailAccountIdList&)
{
    updateGetAccountButton();
    updateAccounts();
}

void EmailClient::accountsUpdated(const QMailAccountIdList&)
{
    updateGetAccountButton();
    updateAccounts();
}

void EmailClient::messagesAdded(const QMailMessageIdList& ids)
{
    if (isRetrieving()) {
        if (arrivedMessageIds.isEmpty())
            QTimer::singleShot(0, this, SLOT(processArrivedMessages()));

        arrivedMessageIds += ids;
    }
}

void EmailClient::messagesUpdated(const QMailMessageIdList& ids)
{
    if (isRetrieving()) {
        const QMailMessageIdList downloadIds(mailDownloadList.mailIds());

        if (!downloadIds.isEmpty()) {
            bool displayed(false);

            foreach (const QMailMessageId &id, ids) {
                if (downloadIds.contains(id)) {
                    QMailMessageMetaData updatedMessage(id);
                    if (updatedMessage.status() & QMailMessage::Removed) {
                        // This message has been removed
                        if (!displayed) {
                            QMessageBox::warning(0,
                                                tr("Message deleted"),
                                                tr("Message cannot be downloaded, because it has been deleted from the server."),
                                                QMessageBox::Ok);
                            displayed = true;
                        }

                        mailDownloadList.remove(id);
                    }
                }
            }
        }
    }
}

void EmailClient::processArrivedMessages()
{
    // Note: throughput can be increased at a cost to interactivity by increasing batchSize:
    static const int batchSize = 1;

    // Only process a small number before returning to the event loop
    int count = 0;
    while (!arrivedMessageIds.isEmpty() && (count < batchSize)) {
        QMailMessageMetaData message(arrivedMessageIds.takeFirst());

        if (message.parentAccountId() == mailAccountId) {
            bool partial((message.status() & QMailMessage::Downloaded) == 0);
            if (partial) {
                mailPreviewed(message);
            } else {
                mailArrived(message);
            }
        }

        ++count;
    }

    if (!arrivedMessageIds.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(processArrivedMessages()));
    } else {
        if (retrievalPhase == PreviewCompleted) {
            // We have finished the preview phase
            previewRetrievalCompleted();
        } else if (retrievalPhase == CompletionCompleted) {
            // We have finished the completion phase
            completionRetrievalCompleted();
        }
    }
}

void EmailClient::planeModeChanged()
{
    if (planeMode.value().toBool() == false) {
        // We have left airplane mode
        qLog(Messaging) << "Leaving Airplane Safe Mode";

        MessageFolder* outbox = messageStore()->mailbox(QMailFolder::OutboxFolder);
        if (outbox->messageCount(MessageFolder::All)) {
            // Send any queued messages
            sendAllQueuedMail();
        }
    }
}

void EmailClient::jabberStateChanged()
{
    if (jabberRaiseAttempted) {
        if (jabberState.value().toBool()) {
            // The jabber connection has come up - we can raise again later if required
            jabberRaiseAttempted = false;
        }
    }
}

void EmailClient::messageSelectionChanged()
{
    static MessageFolder* const trashFolder = messageStore()->mailbox( QMailFolder::TrashFolder );

    if (!moveAction)
        return; // initActions hasn't been called yet

    MessageUiBase::messageSelectionChanged();

    locationSet.clear();

    int count = messageListView()->rowCount();
    if ((count > 0) && (selectionCount > 0)) {
        // Find the locations for each of the selected messages
        QMailMessageKey key(messageListView()->selected());
        QMailMessageMetaDataList messages = QMailStore::instance()->messagesMetaData(key, QMailMessageKey::ParentFolderId);

        foreach (const QMailMessageMetaData &message, messages)
            locationSet.insert(message.parentFolderId());

        // We can delete only if all selected messages are in the Trash folder
#ifndef QTOPIA_HOMEUI
        if ((locationSet.count() == 1) && (*locationSet.begin() == trashFolder->id())) {
#endif
            deleteMailAction->setText(tr("Delete message(s)", "", selectionCount));
#ifndef QTOPIA_HOMEUI
        } else {
            deleteMailAction->setText(tr("Move to Trash"));
        }
#endif
        moveAction->setText(tr("Move message(s)...", "", selectionCount));
        copyAction->setText(tr("Copy message(s)...", "", selectionCount));
        restoreAction->setText(tr("Restore message(s)", "", selectionCount));
    }

    setActionVisible(composeButton, !markingMode);
#ifdef QTOPIA_HOMEUI
    setActionVisible(searchButton, !markingMode);
    setActionVisible(settingsAction, !markingMode);
    setActionVisible(emptyTrashAction, !markingMode);
    setActionVisible(getMailButton, !markingMode);
#endif

    // Ensure that the per-message actions are hidden, if not usable
    const bool messagesSelected(selectionCount != 0);
#ifdef QTOPIA_HOMEUI
    const int deleteThreshold = (markingMode ? 1 : 2);
    setActionVisible(deleteMailAction, (selectionCount >= deleteThreshold));
#else
    setActionVisible(deleteMailAction, messagesSelected);
#endif

    // We cannot move/copy messages in the trash
    const bool trashMessagesSelected(locationSet.contains(trashFolder->id()));
    setActionVisible(moveAction, (messagesSelected && !trashMessagesSelected));
    setActionVisible(copyAction, (messagesSelected && !trashMessagesSelected));
    setActionVisible(restoreAction, (messagesSelected && trashMessagesSelected));
}

void EmailClient::newMessages(bool userRequest)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = NewMessages;

        openFiles();

        // We need to find the new message counts ourselves
        QMailMessageKey key(QMailMessageKey::Status, QMailMessage::New, QMailDataComparator::Includes);
        foreach (QMailMessage::MessageType type, QList<QMailMessage::MessageType>() << QMailMessage::Sms
                                                                                    << QMailMessage::Mms
                                                                                    << QMailMessage::Email
                                                                                    << QMailMessage::Instant
                                                                                    << QMailMessage::System) {
            int count = QMailStore::instance()->countMessages(key & QMailMessageKey(QMailMessageKey::Type, type));
            if (count)
                setNewMessageCount(type, count);
        }
    }

    bool respondingToRaise = (initialAction == NewMessages);
    if (userRequest) {
        // The user requested to see the new messages, so go there directly
        viewNewMessages(respondingToRaise);
    } else {
        // Ask if the user wants to see the new messages
        promptNewMessageView(respondingToRaise);
    }
}

int EmailClient::newMessageCount(QMailMessage::MessageType type)
{
    int total = 0;

    QMap<QMailMessage::MessageType, int>::const_iterator it = newMessageCounts.begin(), end = newMessageCounts.end();
    for ( ; it != end; ++it) {
        if ((it.key() == type) || (type == QMailMessage::AnyType))
            total += it.value();
    }

    return total;
}

void EmailClient::setNewMessageCount(QMailMessage::MessageType type, uint count)
{
    newMessageCounts[type] = static_cast<int>(count);
}

void EmailClient::newCountChanged(uint count)
{
    if (QMailNewMessageHandler *handler = static_cast<QMailNewMessageHandler*>(sender())) {
        if (!QtopiaApplication::instance()->willKeepRunning()) {
            initialAction = IncomingMessages;
        }
        setNewMessageCount(handler->messageType(), count);

        pendingHandlers.insert(handler);

        if (retrievalPhase == None) {
            newMessageArrival();
        } else {
            // We should wait until we have processed the added messages
        }
    }
}

void EmailClient::newMessageArrival()
{
    static MessageFolder* const serverFolder = messageStore()->serverMailbox();

    // The new messages may still be held by the server
    QMailMessageIdList newList = serverFolder->messages(QMailMessage::New,
                                                        true,
                                                        QMailMessage::AnyType,
                                                        MessageFolder::DescendingDate);

    // Process incoming message
    foreach (const QMailMessageId& id, newList) {
        QMailMessageMetaData incoming(id);

        bool partial((incoming.status() & QMailMessage::Downloaded) == 0);
        if (partial)
            mailPreviewed(incoming);
        else
            mailArrived(incoming);
    }

    // Prompt the user to view new messages if desired (if the new count is
    // less than ignored, then we don't know about some messages having been
    // marked not new, and thus can't safely ignore this update...)
    if (newMessageCount(QMailMessage::AnyType) != ignoredMessageCount) {
        // Are we responding to a raise request from messageserver?
        promptNewMessageView(initialAction == IncomingMessages);
    }
}

void EmailClient::promptNewMessageView(bool respondingToRaise)
{
    static MessageFolder* const inboxFolder = messageStore()->mailbox( QMailFolder::InboxFolder );

    // Start the message ring
    QtopiaServiceRequest req("Ringtone", "startMessageRingtone()");
    req.send();

    int newSmsCount = newMessageCount(QMailMessage::Sms);
    if (newSmsCount) {
        // If any of these messages are 'flash' messages, view automatically
        QMailMessageIdList newList = inboxFolder->messages(QMailMessage::New,
                                                           true,
                                                           QMailMessage::Sms,
                                                           MessageFolder::DescendingDate);

        foreach (const QMailMessageId& id, newList) {
            QMailMessage sms(id);
            if (sms.headerFieldText("X-Sms-Class") == "0") {
                flashMessageIds.push(id);
                break;
            }
        }
    }

    if (!flashMessageIds.isEmpty()) {
        if (newMessagesBox) {
            newMessagesBox->deleteLater();
            newMessagesBox = 0;
        }

        // There is a flash sms
        viewNewMessages(respondingToRaise);
    } else {
        int totalNewMessageCount = newMessageCount(QMailMessage::AnyType);

        if (!respondingToRaise && (currentLocation() == Viewer)) {
            // See if the message viewer wants to suppress the new message dialog
            QMailMessageIdList newMessageIds = MessageStore::messages(QMailMessage::New,
                                                                      true,
                                                                      QMailMessage::AnyType, 
                                                                      MessageFolder::DescendingDate);

            if (newMessageIds.count() > totalNewMessageCount)
                newMessageIds = newMessageIds.mid(0, totalNewMessageCount);

            if (readMailWidget()->handleIncomingMessages(newMessageIds)) {
                // The viewer has handled the new message event
                acknowledgeMessageArrivals();
                return;
            }
        }

        // Ask if the user wants to view the incoming message(s)
#ifdef QTOPIA_HOMEUI
        QString text;
        if (totalNewMessageCount == 1)
            text = tr("1 new message has arrived.");
        else
            text = tr("%n new messages have arrived.", "" , totalNewMessageCount);
#else
        QString text(tr("%n new message(s) have arrived. Do you wish to view them now?", "", totalNewMessageCount));
#endif

        bool existingDialog(newMessagesBox != 0);

#ifdef QTOPIA_HOMEUI
        if (existingDialog) {
            // If the box is already open, close it (otherwise it will still be in the part
            // of the screen grabbed behind the 'updated dialog)
            newMessagesBox->reject();
            existingDialog = false;
        }

        newMessagesBox = new NewMessagesDialog(tr("New message"));
        newMessagesBox->setText(text);

        if (totalNewMessageCount == 1) {
            // With just a single message, show its details in the dialog box
            QMailMessageIdList newList = MessageStore::messages(QMailMessage::New,
                                                                true,
                                                                QMailMessage::AnyType,
                                                                MessageFolder::DescendingDate);

            if (!newList.isEmpty())
                newMessagesBox->setMessage(newList.first());
        }
#else
        if (existingDialog) {
            // Update the text in the existing dialog and restart the timer
            newMessagesBox->setText(text);
        } else {
            // Ask the user whether to view the message(s)
            newMessagesBox = new NewMessagesDialog(QMessageBox::Information, tr("New message"), text, QMessageBox::Yes | QMessageBox::No);
        }
#endif
    
        if (!existingDialog) {
            connect(newMessagesBox, SIGNAL(finished(int)), this, SLOT(newMessageAction(int)));
            QtopiaApplication::showDialog(newMessagesBox);

            connect(&newMessageResponseTimer, SIGNAL(timeout()), this, SLOT(abortViewNewMessages()));
        }

        if (NotificationVisualTimeout)
            newMessageResponseTimer.start(NotificationVisualTimeout);
    }
}

void EmailClient::viewNewMessages(bool respondingToRaise)
{
    bool savedAsDraft(false);
    int totalNewMessageCount = newMessageCount(QMailMessage::AnyType);

    if (!respondingToRaise) {
        // We were already operating when this new message notification arrived; if we
        // are composing, we need to save as draft
        if (currentLocation() == Composer) {
            savedAsDraft = writeMailWidget()->forcedClosure();
        }
    }

    QMailMessageKey newKey(QMailMessageKey::Status, QMailMessage::New, QMailDataComparator::Includes);
    QMailMessageKey incomingKey(QMailMessageKey::Status, QMailMessage::Incoming, QMailDataComparator::Includes);

    QMailMessageSortKey sortKey(QMailMessageSortKey::TimeStamp, Qt::DescendingOrder);
    QMailMessageIdList newList = QMailStore::instance()->queryMessages(newKey & incomingKey, sortKey);

    if (!newList.isEmpty()) {
        bool flashSms(!flashMessageIds.isEmpty());
        if ((totalNewMessageCount == 1) && !flashSms) {
            // Find the newest incoming message and display it
            viewMessage(newList.first(), false);
        } else {
            if (totalNewMessageCount > 1) {
#ifdef QTOPIA_HOMEUI
                // Just display the unified inbox
                viewAllMessages();
#else
                // Show just the new messages in a list
                if (newList.count() > totalNewMessageCount)
                    newList = newList.mid(0, totalNewMessageCount);

                viewMessageList(QMailMessageKey(newList), tr("New messages"));
#endif
            }

            if (flashSms) {
                // Display the flash SMS directly
                viewMessage(flashMessageIds.top(), false);
            }
        }

        // Acknowledge any new message arrival events we have now handled
        acknowledgeMessageArrivals();

        // We're no longer ignoring any messages
        ignoredMessageCount = 0;
    } else {
        qLog(Messaging) << "No unread messages? newMessageCount:" << totalNewMessageCount;
    }

    if (savedAsDraft) {
        // The composer had a partial message, now saved as a draft
        AcknowledgmentBox::show(tr("Saved to Drafts"), tr("Incomplete message has been saved to the Drafts folder"));
    }
}

void EmailClient::ignoreNewMessages()
{
    ignoredMessageCount = newMessageCount(QMailMessage::AnyType);

    foreach (QMailNewMessageHandler *handler, pendingHandlers)
        handler->setHandled(false);

    pendingHandlers.clear();
}

void EmailClient::viewEmails()
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = View;
    }

    openFiles();

#ifdef QTOPIA_HOMEUI
    viewAllMessages();
#else
    viewInbox(true);
#endif
}

void EmailClient::viewMessages()
{
    bool alreadyRunning = QtopiaApplication::instance()->willKeepRunning();
    if (!alreadyRunning) {
        initialAction = View;
    }

    openFiles();

#ifdef QTOPIA_HOMEUI
    viewAllMessages();

    if (alreadyRunning) {
        if (newMessageCount(QMailMessage::AnyType))
            viewNewMessages(false);
    }
#else
    viewInbox(false);
#endif
}

#ifdef QTOPIA_HOMEUI
void EmailClient::viewAllMessages()
{
    const bool startingUp = (currentLocation() == MessageUiBase::NoLocation);

    // Wherever we are currently located, drop back to an empty location stack
    clearLocationStack();
    viewMessageList(MessageListView::DisplayMessages);

    if (startingUp) {
        // Starting up - look for an interrupted draft once queued events are processed
        QTimer::singleShot(0, this, SLOT(resumeInterruptedComposition()));
    }
}
#else
void EmailClient::viewInbox(bool email)
{
    if (email) {
        // View the message set represented by the Inbox folder in the email folder tree
        viewMessageList(InboxMessageSet::contentKey());
    } else {
        // View the message set represented by the Inbox folder in the action list
        viewMessageList(ActionFolderMessageSet::contentKey(QMailFolderId(QMailFolder::InboxFolder)));
    }
}
#endif

void EmailClient::writeSms(const QString& name, const QString& number, const QString& filename)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    QString body;
    if (!filename.isEmpty()) {
        QFile f( filename );
        if (! f.open(QIODevice::ReadOnly) ) {
            qWarning("could not open file: %s", filename.toLatin1().constData() );
        } else {
            body = QString::fromLocal8Bit( f.readAll() );
            f.close();
            f.remove();
        }
    }

    writeSmsAction(name, number, body, false);
}

void EmailClient::writeInstantMessage(const QString& uri)
{
#ifndef QTOPIA_NO_COLLECTIVE
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = Compose;
    }

    delayedInit();

    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if (checkMailConflict(tr("Should it be saved in Drafts before writing the new message?"), 
                          tr("'Write Instant Message' message will be ignored")) )
        return;

    writeMessageAction(QString(), uri, QStringList(), QStringList(), QMailMessage::Instant);
#endif
}

void EmailClient::newMessageAction(int choice)
{
    newMessageResponseTimer.stop();

    // Stop the message ring, if necessary
    QtopiaServiceRequest req("Ringtone", "stopMessageRingtone()");
    req.send();

    // Are we responding to a raise request from messageserver?
    bool respondingToRaise(initialAction == IncomingMessages);

    if (choice == QMessageBox::Yes) {
        viewNewMessages(respondingToRaise);
    } else {
        ignoreNewMessages();
    }

    newMessagesBox->deleteLater();
    newMessagesBox = 0;
}

void EmailClient::abortViewNewMessages()
{
    newMessagesBox->setResult(QMessageBox::No);
    newMessagesBox->reject();
}

void EmailClient::noSendAccount(QMailMessage::MessageType type)
{
    QString key(QMailComposerFactory::defaultKey(type));
    QString name(QMailComposerFactory::name(key, type));

    QMessageBox::warning(0,
                         tr("Send Error"),
                         tr("%1 cannot be sent, because no account has been configured to send with.","%1=MMS/Email/TextMessage").arg(name),
                         QMessageBox::Ok);
}

void EmailClient::setActionVisible(QAction* action, bool visible)
{
    if (action)
        actionVisibility[action] = visible;
}

void EmailClient::composeMessage(QMailMessage::MessageType type,
                                 const QMailAddressList& to,
                                 const QString& subject,
                                 const QString& text,
                                 const QContentList& attachments,
                                 QMailMessage::AttachmentsAction action,
                                 bool detailsOnly)
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if (type == QMailMessage::AnyType) {
        // Some attachment types can be sent in an SMS
        bool textOnly(true);
        foreach (const QContent& attachment, attachments) {
            if ((attachment.type() != "text/plain") &&
                (attachment.type() != "text/x-vCalendar") &&
                (attachment.type() != "text/x-vCard")) {
                textOnly = false;
            }
        }

        // Determine what type of addresses we're sending to
        bool emailRecipients(false);
        bool phoneRecipients(false);
        foreach (const QMailAddress& address, to) {
            emailRecipients |= address.isEmailAddress();
            phoneRecipients |= address.isPhoneNumber();
        }

        if (!emailRecipients && textOnly) {
            type = QMailMessage::Sms;
        } else if (!phoneRecipients) {
            type = QMailMessage::Email;
        } else {
            type = QMailMessage::Mms;
        }
    }

    writeMailWidget()->newMail(type, detailsOnly);
    if ( writeMailWidget()->composer().isEmpty() ) {
        // failed to create new composer, maybe due to no email account
        // being present. So hide/quit qtmail.
        closeApplication();
        return;
    }

    writeMailWidget()->setRecipient( QMailAddress::toStringList(to).join(",") );
    writeMailWidget()->setSubject( subject );
    writeMailWidget()->setBody( text, "text/plain; charset=UTF-8" );

    foreach (const QContent& attachment, attachments)
        writeMailWidget()->attach( attachment, action );

    viewComposer();
}

void EmailClient::composeMessage(const QMailMessage& message)
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    modify(message);
}

MessageFolder* EmailClient::containingFolder(const QMailMessageId& id)
{
    return messageStore()->owner(id);
}

QMailAccountIdList EmailClient::emailAccounts() const
{
    QMailAccountKey key(QMailAccountKey::MessageType, QMailMessage::Email);
    return QMailStore::instance()->queryAccounts(key);
}

SearchProgressDialog* EmailClient::searchProgressDialog() const
{
    Q_ASSERT(searchAction);
    static SearchProgressDialog* spd = new SearchProgressDialog(searchAction);
    return spd;
}

void EmailClient::clearNewMessageStatus(const QMailMessageKey& key)
{
    QMailMessageKey clearNewKey = key & QMailMessageKey(QMailMessageKey::Status, QMailMessage::New, QMailDataComparator::Includes);

    int count = QMailStore::instance()->countMessages(clearNewKey);
    if (count) {
        QMailStore::instance()->updateMessagesMetaData(clearNewKey, QMailMessage::New, false);

        if (ignoredMessageCount) {
            // Reduce the ignored count by however many are not new anymore
            if (count >= ignoredMessageCount) {
                ignoredMessageCount = 0;
            } else {
                ignoredMessageCount -= count;
            }
        }
    }
}

void EmailClient::setMarkingMode(bool set)
{
    MessageUiBase::setMarkingMode(set);

    if (markingMode) {
        markAction->setText(tr("Cancel"));
    } else {
        markAction->setText(tr("Mark messages"));
    }
}

void EmailClient::markMessages()
{
    setMarkingMode(!markingMode);
}

void EmailClient::synchronizeFolder()
{
    if (const QAction* action = static_cast<const QAction*>(sender())) {
        QMailFolderId folderId(action->data().value<QMailFolderId>());

        if (folderId.isValid()) {
            QMailFolder folder(folderId);
            bool excludeFolder = (folder.status() & QMailFolder::SynchronizationEnabled);

            if (QMailStore *store = QMailStore::instance()) {
                if (excludeFolder) {
                    // Delete any messages which are in this folder or its sub-folders
                    QMailMessageKey messageKey(QMailMessageKey::ParentFolderId, folderId, QMailDataComparator::Equal);
                    QMailMessageKey descendantKey(QMailMessageKey::AncestorFolderIds, folderId, QMailDataComparator::Includes);
                    store->removeMessages(messageKey | descendantKey, QMailStore::NoRemovalRecord);
                }

                // Find any subfolders of this folder
                QMailFolderKey subfolderKey(QMailFolderKey::AncestorFolderIds, folderId, QMailDataComparator::Includes);
                QMailFolderIdList folderIds = QMailStore::instance()->queryFolders(subfolderKey);

                // Mark all of these folders as {un}synchronized
                folderIds.append(folderId);
                foreach (const QMailFolderId &id, folderIds) {
                    QMailFolder folder(id);
                    folder.setStatus(QMailFolder::SynchronizationEnabled, !excludeFolder);
                    store->updateFolder(&folder);
                }
            }

            // Update the action to reflect the change
            folderSelected(folderView()->currentItem());
        }
    }
}

#include "emailclient.moc"

