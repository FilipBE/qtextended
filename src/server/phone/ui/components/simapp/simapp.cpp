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

#include "simapp.h"
#include "simwidgets.h"
#include <QSimToolkit>
#include <QSimIconReader>
#include <QSimInfo>
#include <qtopiaservices.h>
#include <QtopiaApplication>
#include <QStackedWidget>
#include <QTimer>
#include <QLabel>
#include <QCloseEvent>
#include <QValueSpaceObject>

#include <QSoftMenuBar>
#include <QVBoxLayout>
#include <QDebug>
#ifndef QTOPIA_TEST
#include "messageboard.h"
#include "uifactory.h"

#include "qtopiainputevents.h"
#endif

#define IDLEMODEIMAGENAME Qtopia::tempDir() + "idlemodeimage.png"

class SimKeyFilter;

static SimKeyFilter *ptr = 0;

class SimKeyFilter
    : public QObject
#ifndef QTOPIA_TEST
    , public QtopiaKeyboardFilter
#endif
{

    Q_OBJECT
public:
    static SimKeyFilter * instance() {
        if ( !ptr )
            ptr = new SimKeyFilter();
        return ptr;
    }

    ~SimKeyFilter()
    {
        ptr = 0;
    }

private:
    SimKeyFilter() {}

    virtual bool filter(int, int, int, bool, bool)
    {
        emit keyPressed();
        return false;
    }

signals:
    void keyPressed();
};

#ifndef QTOPIA_TEST
static QDialog *waitDlg = 0;
#endif

SimApp::SimApp(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f), view(0), notification(0),
      hasStk(false), simToolkitAvailable(false), eventList(0), failLabel(0),
      commandOutsideMenu(false), idleModeMsgId(-1), hasSustainedDisplayText(false)
{
    status = new QValueSpaceObject("/Telephony/Status", this);

    setWindowTitle(tr("SIM Applications"));
    stack = new QStackedWidget(this);
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setCentralWidget(stack);

    stk = new QSimToolkit( QString(), this );
    iconReader = 0;

    connect(stk, SIGNAL(command(QSimCommand)),
            this, SLOT(simCommand(QSimCommand)));
    connect(stk, SIGNAL(beginFailed()), this, SLOT(beginFailed()));

    QSimInfo *simInfo = new QSimInfo(QString(), this);
    connect(simInfo, SIGNAL(removed()), this, SLOT(simRemoved()));

#ifndef QTOPIA_TEST
    waitDlg = UIFactory::createDialog( "DelayedWaitDialog", this );
    if ( waitDlg ) {
        QMetaObject::invokeMethod( waitDlg, "setText", Qt::DirectConnection, 
                Q_ARG(QString, tr( "Waiting for response..." ) ) );
        QMetaObject::invokeMethod( waitDlg, "setDelay", Qt::DirectConnection, Q_ARG(int, 500 ) );
    } else {
        qLog(Component) << "SimApp: DelayedWaitDialog not available";
    }
#endif
}

SimApp::~SimApp()
{
}

void SimApp::activate()
{
    // request the main menu list
    stk->begin();
    hasStk = true;
#ifndef QTOPIA_TEST
    // might take some time to fetch the menu
    if ( waitDlg )
        waitDlg->show();
#endif
}

void SimApp::beginFailed()
{
    QSimInfo info;
    if ( info.identity().isEmpty() )
        failLabel = new QLabel(tr("<center>No SIM present.</center>"), stack);
    else
        failLabel = new QLabel(tr("<center>The installed SIM does not support additional applications.</center>"), stack);
    failLabel->setWordWrap(true);
    failLabel->setMargin(10);
    stack->addWidget(failLabel);
    stack->setCurrentWidget(failLabel);
    hasStk = false;
#ifndef QTOPIA_TEST
    if ( waitDlg )
        waitDlg->hide();
#endif
}

void SimApp::simCommand(const QSimCommand &cmd)
{
    switch (cmd.type()) {
        case QSimCommand::SetupMenu: {
            // prevent No SIM message from appearing while fetching main menu
            if ( failLabel ) {
                stack->removeWidget( failLabel );
                delete failLabel;
                failLabel = 0;
            }
            QWidget *oldView = view;
            if (listViewPreferred(cmd))
                cmdMenu(cmd);
            else
                softKeysMenu(cmd);
            removeView(oldView);
            break;
        }
        case QSimCommand::DisplayText:
            cmdDisplayText(cmd);
            break;
        case QSimCommand::GetInkey:
            cmdInKey(cmd);
            break;
        case QSimCommand::GetInput:
            cmdInput(cmd);
            break;
        case QSimCommand::SetupCall:
            cmdSetupCall(cmd);
            break;
        case QSimCommand::PlayTone:
            cmdTone(cmd);
            break;
        case QSimCommand::SelectItem:
            if (listViewPreferred(cmd))
                cmdMenu(cmd);
            else
                softKeysMenu(cmd);
            break;
        case QSimCommand::Refresh:
            cmdRefresh(cmd);
            break;
        case QSimCommand::SendSS:
        case QSimCommand::SendSMS:
        case QSimCommand::SendUSSD:
        case QSimCommand::SendDTMF:
        case QSimCommand::RunATCommand:
        case QSimCommand::CloseChannel:
        case QSimCommand::ReceiveData:
        case QSimCommand::SendData:
            // Pop up a notification to the user if text is not empty.
            showNotification(cmd);
            break;
        case QSimCommand::Timeout:
            // Wavecom only - ignore this.
            break;
        case QSimCommand::EndSession:
            qApp->quit();
            break;
        case QSimCommand::LaunchBrowser:
            cmdLaunchBrowser(cmd);
            break;
        case QSimCommand::OpenChannel:
            cmdChannel(cmd);
            break;
        case QSimCommand::SetupIdleModeText:
            cmdIdleModeText(cmd);
            break;
        case QSimCommand::NoCommand:
        case QSimCommand::MoreTime:
        case QSimCommand::PollInterval:
        case QSimCommand::PollingOff:
        case QSimCommand::ProvideLocalInformation:
        case QSimCommand::PerformCardAPDU:
        case QSimCommand::PowerOffCard:
        case QSimCommand::PowerOnCard:
        case QSimCommand::GetReaderStatus:
        case QSimCommand::TimerManagement:
        case QSimCommand::GetChannelStatus:
        case QSimCommand::LanguageNotification:
            // Ignore these commands: they are handled by modem vendor plugins.
            break;
        case QSimCommand::SetupEventList:
            cmdSetupEventList(cmd);
            break;

        default: break;
    }
}

void SimApp::simRemoved()
{
    if(simToolkitAvailable) {
        simToolkitAvailable = false;
        updateValueSpace();
    }
    changeEventList(0);
}

void SimApp::updateValueSpace()
{
    QIcon icon;
    if ( !idleModeImage.isNull() ) {
        if ( idleModeImage.format() == QImage::Format_Mono ) {
            // Use the monochrome image as a mask to create a transparent icon.
            QPixmap pixmap( idleModeImage.size() );
            pixmap.fill( QColor(0,0,0) );
            pixmap.setAlphaChannel( QPixmap::fromImage( idleModeImage ) );
            icon = QIcon( pixmap );
        } else {
            icon = QIcon( QPixmap::fromImage( idleModeImage ) );
        }
        idleModeImage.save( IDLEMODEIMAGENAME );
    } else {
        QFile::remove( IDLEMODEIMAGENAME );
    }

    status->setAttribute("SimToolkit/Available", simToolkitAvailable);
    status->setAttribute("SimToolkit/IdleModeText", idleModeText.text());
    status->setAttribute("SimToolkit/IdleModeIcon", icon);
    status->setAttribute("SimToolkit/IdleModeIconSelfExplanatory",
                         idleModeText.iconSelfExplanatory());
    status->setAttribute("SimToolkit/MenuTitle", mainMenuTitle);
#ifndef QTOPIA_TEST
    MessageBoard *board = qtopiaTask<MessageBoard>();
    if ( !idleModeImage.isNull() || !idleModeText.text().isEmpty() ) {
        if ( board ) {
            // remove old message
            if ( idleModeMsgId > -1 )
                board->clearMessage( idleModeMsgId );
            idleModeMsgId = board->postMessage(IDLEMODEIMAGENAME,
                    idleModeText.iconSelfExplanatory() ? QString() : idleModeText.text(),
                    20 ); // set as very low priority
        }
    } else {
        if ( board )
            board->clearMessage( idleModeMsgId );
        idleModeMsgId = -1;
    }
#endif
}

void SimApp::sendEnvelope( const QSimEnvelope &env )
{
    if (commandOutsideMenu) {
        commandOutsideMenu = false;
        stk->sendEnvelope( env );
        hide();
    } else {
#ifndef QTOPIA_TEST
        if (waitDlg && !hasSustainedDisplayText)
            waitDlg->show();
#endif
        stk->sendEnvelope( env );
    }
}

void SimApp::sendResponse( const QSimTerminalResponse &res )
{
    if (commandOutsideMenu) {
        commandOutsideMenu = false;
        stk->sendResponse( res );
        hide();
    } else {
#ifndef QTOPIA_TEST
        if (waitDlg && !hasSustainedDisplayText)
            waitDlg->show();
#endif
        stk->sendResponse( res );
    }
}

void SimApp::cmdMenu(const QSimCommand &cmd)
{
    if (!simToolkitAvailable) {
        // First time that we have seen the toolkit menu, so this
        // SIM obviously has a SIM toolkit application on it.
        simToolkitAvailable = true;
        mainMenuTitle = cmd.title();
        updateValueSpace();
    }

    // Create QSimIconReader if the menu contains them.
    QList<QSimMenuItem> items = cmd.menuItems();
    QList<QSimMenuItem>::ConstIterator it;
    bool needIcon = false;
    for (it = items.begin(); it != items.end(); ++it) {
        if ( (int)(*it).iconId() ) {
            needIcon = true;
            if ( !iconReader )
                createIconReader();
            else
                break;
        }
    }

    // Create the menu widgetry.
    SimMenu *menu;
    if ( needIcon )
        menu = new SimMenu(cmd, iconReader, stack);
    else
        menu = new SimMenu(cmd, stack);

    // main menu send QSimEnvelope,sub menu sends QSimTerminalResponse
    if (cmd.type() == QSimCommand::SetupMenu) {
        connect(menu, SIGNAL(sendEnvelope(QSimEnvelope)),
                this, SLOT(sendEnvelope(QSimEnvelope)) );
    } else {
        connect(menu, SIGNAL(sendResponse(QSimTerminalResponse)),
                this, SLOT(sendResponse(QSimTerminalResponse)) );
    }
    setView(menu);
}

void SimApp::cmdDisplayText(const QSimCommand &cmd)
{
#ifndef QTOPIA_TEST
    // DisplayText with low priority came while screen is busy
    QWidget *home = homescreen();
    if ( !this->isVisible()
            && !(home->windowState() & Qt::WindowActive)
            && !cmd.highPriority() ) {
        QSimTerminalResponse resp;
        resp.setCommand(cmd);
        resp.setCause(QSimTerminalResponse::ScreenIsBusy);
        resp.setResult(QSimTerminalResponse::MEUnableToProcess);
        sendResponse( resp );
        return;
    }
#endif
    if ( cmd.iconId() > 0 )
        createIconReader();

    hasSustainedDisplayText = cmd.immediateResponse();

    if (view && qobject_cast<SimText*>(view)) {
        SimText *text = (SimText *)view;
        if (cmd.highPriority() || !text->hasHighPriority()) {
            if (cmd.iconId())
                text->setCommand(cmd, iconReader);
            else
                text->setCommand(cmd);
            stack->setCurrentWidget(text);
        } else {
            // send screen busy response
            QSimTerminalResponse resp;
            resp.setCommand(cmd);
            resp.setCause(QSimTerminalResponse::ScreenIsBusy);
            resp.setResult(QSimTerminalResponse::MEUnableToProcess);
            sendResponse( resp );
        }
    } else {
        SimText *text = new SimText(cmd, iconReader, stack);
        connect(text, SIGNAL(sendResponse(QSimTerminalResponse)),
                this, SLOT(sendResponse(QSimTerminalResponse)) );
        connect(text, SIGNAL(hideApp()), this, SLOT(hideApp()));
        setView(text);
    }
}

void SimApp::cmdInKey(const QSimCommand &cmd)
{
    if ( cmd.iconId() > 0 )
        createIconReader();

    SimInKey *inKey = new SimInKey(cmd, iconReader, stack);
    connect(inKey, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(inKey);
}

void SimApp::cmdInput(const QSimCommand &cmd)
{
    SimInput *input = new SimInput(cmd, iconReader, stack);
    connect(input, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(input);
}

void SimApp::cmdSetupCall(const QSimCommand &cmd)
{
    SimSetupCall *setupCall;

    if ( cmd.iconId() > 0 ) {
        createIconReader();
        setupCall = new SimSetupCall( cmd, iconReader, stack );
    } else {
        setupCall = new SimSetupCall( cmd, stack );
    }

    connect(setupCall, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(setupCall);
}

void SimApp::cmdTone(const QSimCommand & cmd )
{
    if ( cmd.iconId() > 0 )
        createIconReader();
    SimTone *tone = new SimTone(cmd, iconReader, stack);
    connect(tone, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(tone);
}

void SimApp::cmdRefresh(const QSimCommand & /*cmd*/)
{
    // we assume that the actual reset in handled for us.
    //XXX apps may need to reload data
}

void SimApp::cmdChannel(const QSimCommand& cmd)
{
    SimChannel *channel;
    if ( cmd.iconId() > 0 ) {
        createIconReader();
        channel = new SimChannel(cmd, iconReader, stack);
    } else {
        channel = new SimChannel(cmd, stack);
    }

    connect(channel, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(channel);
}

void SimApp::cmdIdleModeText(const QSimCommand &cmd)
{
    idleModeText = cmd;
    if ( cmd.iconId() != 0 ) {
        createIconReader();
        if ( iconReader->haveIcon( cmd.iconId() ) ) {
            // Icon already cached from last time.
            idleModeImage = iconReader->icon( cmd.iconId() );
        } else {
            // Request that the icon be fetched from the sim.
            iconReader->requestIcon( cmd.iconId() );
            idleModeImage = QImage();
        }
    } else {
        idleModeImage = QImage();
    }
    updateValueSpace();
}

void SimApp::cmdSetupEventList(const QSimCommand &cmd)
{
    QByteArray events = cmd.extensionField(0x99);
    int newEvents = 0;
    foreach ( char ev, events )
        newEvents |= (1 << (int)(ev & 0xFF));
    changeEventList( newEvents );
}

void SimApp::cmdLaunchBrowser(const QSimCommand &cmd)
{
    SimLaunchBrowser *browser;
    if ( cmd.iconId() > 0 ) {
        createIconReader();
        browser = new SimLaunchBrowser(cmd, iconReader, stack);
    } else {
        browser = new SimLaunchBrowser(cmd, stack);
    }

    connect(browser, SIGNAL(sendResponse(QSimTerminalResponse)),
            this, SLOT(sendResponse(QSimTerminalResponse)) );
    setView(browser);
}

void SimApp::closeEvent(QCloseEvent *ce)
{
    if (!view || view == notification) {
        ce->ignore();
    } else {
        if (!view->inherits("SimMenu") || !((SimMenu*)view)->isMainMenu())
            ce->ignore();
    }
    QMainWindow::closeEvent(ce);
}

void SimApp::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Context1) {
        if (view) {
            if (view->command().hasHelp()) {
                // help requested
                if (view->command().type() == QSimCommand::SetupMenu) {
                    QSimEnvelope env;
                    env.setType(QSimEnvelope::MenuSelection);
                    env.setSourceDevice(QSimCommand::Keypad);
                    env.setMenuItem(view->helpIdentifier());
                    env.setRequestHelp(true);
                    sendEnvelope( env );
                } else {
                    QSimTerminalResponse resp;
                    resp.setCommand(view->command());
                    resp.setResult(QSimTerminalResponse::HelpInformationRequested);
                    resp.setMenuItem(view->helpIdentifier());
                    sendResponse( resp );
                }
            }
        }
    }
    QMainWindow::keyPressEvent(ke);
}

void SimApp::showEvent(QShowEvent *event)
{
    if (!commandOutsideMenu)
        QTimer::singleShot(0, this, SLOT(activate()));
    QMainWindow::showEvent(event);
}

// Shows info to user when, e.g. SMS is sent.
void SimApp::showNotification(const QSimCommand &cmd)
{
    if (!cmd.text().isEmpty() || !cmd.suppressUserFeedback()) {
        if (notification) {
            notification->setCommand(cmd);
            stack->setCurrentWidget(notification);
            emit viewChanged(notification);
        } else {
            if ( cmd.iconId() > 0 )
                createIconReader();
            notification = new SimText(cmd, iconReader, stack);
            stack->addWidget(notification);
            stack->setCurrentWidget(notification);
            emit viewChanged(notification);
        }
    }
}

void SimApp::hideNotification()
{
    if (notification) {
        if (stack->indexOf(notification) != -1 ) {
            stack->removeWidget(notification);
        }
        delete notification;
        notification = 0;
    }
}

void SimApp::setView(SimCommandView *w)
{
    // View for other than DisplayText received. Cancel sustainedDisplay
    if ( !qobject_cast<SimText*>(w) && hasSustainedDisplayText )
       hasSustainedDisplayText = false;

#ifndef QTOPIA_TEST
    if (waitDlg) 
        waitDlg->hide();
#endif
    hideNotification();
    stack->addWidget(w);
    if (view) {
        view->removeEventFilter(this);
        removeView(view);
    }
    view = w;
    view->installEventFilter(this);
    stack->setCurrentWidget(w);
    if (!isVisible() && (!view->inherits("SimMenu") || !((SimMenu*)view)->isMainMenu())) {
        commandOutsideMenu = true;
        showMaximized();
    }
    emit viewChanged(w);
}

void SimApp::removeView(QWidget *w)
{
    qApp->processEvents();
    if (stack->indexOf(w) != -1 ) {
        stack->removeWidget(w);
        if (view == w)
            view = 0;
        w->deleteLater();
    }
}

void SimApp::terminateSession()
{
#ifndef QTOPIA_TEST
    if (waitDlg) 
        waitDlg->hide();
#endif
    QSimTerminalResponse resp;
    if ( view )
        resp.setCommand( view->command() );
    resp.setResult( QSimTerminalResponse::SessionTerminated );
    stk->sendResponse( resp );
    if ( view )
        removeView( view );
    stk->end();
    if (commandOutsideMenu) {
        commandOutsideMenu = false;
        hide();
    }
}

void SimApp::hideApp()
{
    if ( view ) {
        removeView( view );
        hasSustainedDisplayText = false;
    }
    hide();
}

bool SimApp::listViewPreferred(const QSimCommand& cmd)
{
    if (!cmd.softKeysPreferred())
        return true;
    // reserve one key for back button
    if ((QSoftMenuBar::keys().count() - 1) >= cmd.menuItems().count())
        return false;
    else
        return true;
}

void SimApp::softKeysMenu(const QSimCommand& cmd)
{
    // Request icons from the SIM if the menu contains them.
    QList<QSimMenuItem> items = cmd.menuItems();
    QList<QSimMenuItem>::ConstIterator it;
    for (it = items.begin(); it != items.end(); ++it) {
        int iconId = (int)((*it).iconId());
        if ( iconId > 0 )
            createIconReader();
    }

    // Create the menu widgetry.
    SoftKeysMenu *menu = new SoftKeysMenu(cmd, iconReader, stack);

    // main menu send QSimEnvelope,sub menu sends QSimTerminalResponse
    if (cmd.type() == QSimCommand::SetupMenu) {
        connect(menu, SIGNAL(sendEnvelope(QSimEnvelope)),
                this, SLOT(sendEnvelope(QSimEnvelope)) );
    } else {
        connect(menu, SIGNAL(sendResponse(QSimTerminalResponse)),
                this, SLOT(sendResponse(QSimTerminalResponse)) );
    }
    setView(menu);
}

void SimApp::createIconReader()
{
    if (!iconReader) {
        iconReader = new QSimIconReader( QString(), this );
        connect(iconReader, SIGNAL(iconAvailable(int)), this, SLOT(iconAvailable(int)));
    }
}

void SimApp::iconAvailable(int iconId)
{
    if (idleModeText.iconId() != 0 && iconId == (int)idleModeText.iconId()) {
        idleModeImage = iconReader->icon(iconId);
        updateValueSpace();
    }
}

static bool hasEvent(int list, QSimEnvelope::Event event)
{
    return ( ( list & (1 << (int)event) ) != 0 );
}

void SimApp::changeEventList(int newEvents)
{
    int oldEvents = eventList;
    eventList = newEvents;

    if ( hasEvent( newEvents, QSimEnvelope::UserActivity ) !=
         hasEvent( oldEvents, QSimEnvelope::UserActivity ) ) {
        changeUserActivityEvent
            ( hasEvent( newEvents, QSimEnvelope::UserActivity ) );
    }

    if ( hasEvent( newEvents, QSimEnvelope::IdleScreenAvailable ) !=
         hasEvent( oldEvents, QSimEnvelope::IdleScreenAvailable ) ) {
        changeIdleScreenEvent
            ( hasEvent( newEvents, QSimEnvelope::IdleScreenAvailable ) );
    }

    if ( hasEvent( newEvents, QSimEnvelope::BrowserTermination ) !=
         hasEvent( oldEvents, QSimEnvelope::BrowserTermination ) ) {
        changeBrowserTerminationEvent
            ( hasEvent( newEvents, QSimEnvelope::BrowserTermination ) );
    }
}

void SimApp::changeUserActivityEvent(bool value)
{
    if ( value ) {
        connect( SimKeyFilter::instance(), SIGNAL(keyPressed()),
                this, SLOT(userActivityOccurred()) );
#ifndef QTOPIA_TEST
        QtopiaInputEvents::addKeyboardFilter( SimKeyFilter::instance() );
#endif
    } else {
        disconnect( SimKeyFilter::instance(), SIGNAL(keyPressed()),
                this, SLOT(userActivityOccurred()) );
#ifndef QTOPIA_TEST
        QtopiaInputEvents::removeKeyboardFilter();
#endif
    }
}

void SimApp::userActivityOccurred()
{
    // send response
    QSimEnvelope env;
    env.setType( QSimEnvelope::EventDownload );
    env.setEvent( QSimEnvelope::UserActivity );
    sendEnvelope( env );
    // remove event from the list
    changeEventList(eventList & ~(1 << (int)(QSimEnvelope::UserActivity & 0xFF)));
}

void SimApp::changeIdleScreenEvent(bool value)
{
#ifndef QTOPIA_TEST
    QWidget *home = homescreen();
    if (!home) return;
    if ( value )
        home->installEventFilter(this);
    else
        home->removeEventFilter(this);
#endif
}

bool SimApp::eventFilter(QObject *o, QEvent *e)
{
#ifndef QTOPIA_TEST
    QWidget *home = homescreen();
    if ( home && o == home && e->type() == QEvent::WindowActivate ) {
        // send response
        QSimEnvelope env;
        env.setType( QSimEnvelope::EventDownload );
        env.setEvent( QSimEnvelope::IdleScreenAvailable );
        sendEnvelope( env );
        // remove event from the list
        changeEventList(eventList & ~(1 << (int)(QSimEnvelope::IdleScreenAvailable & 0xFF)));
    } else
#endif
    if ( o == view && e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if ( ke->key() == Qt::Key_Hangup )
            close();
    }
    return false;
}

void SimApp::changeBrowserTerminationEvent(bool value)
{
#ifndef QTOPIA_TEST
    if ( !qtopiaTask<ApplicationLauncher>() )
        return;

    if ( value ) {
        connect( qtopiaTask<ApplicationLauncher>(),
                SIGNAL(applicationTerminated
                    (QString,ApplicationTypeLauncher::TerminationReason,bool)),
                this, SLOT(applicationTerminated
                    (QString,ApplicationTypeLauncher::TerminationReason,bool)) );
    } else {
        disconnect( qtopiaTask<ApplicationLauncher>(),
                SIGNAL(applicationTerminated
                    (QString,ApplicationTypeLauncher::TerminationReason,bool)),
                this, SLOT(applicationTerminated
                    (QString,ApplicationTypeLauncher::TerminationReason,bool)) );
    }
#endif
}

QWidget* SimApp::homescreen()
{
    static QWidget* homescreen = 0;
    if (!homescreen) {
        foreach(QWidget* w, QApplication::allWidgets())
        {
            if (w->inherits("QAbstractHomeScreen")) {
                homescreen = w;
                break;
            }

        }
    }
    return homescreen;
}

#ifndef QTOPIA_TEST
void SimApp::applicationTerminated(const QString &app, ApplicationTypeLauncher::TerminationReason reason, bool)
{
    if ( app == QtopiaService::app( "WebAccess" ) &&
            hasEvent( eventList, QSimEnvelope::BrowserTermination ) ) {
        QSimEnvelope env;
        env.setType( QSimEnvelope::EventDownload );
        env.setEvent( QSimEnvelope::BrowserTermination );
        // add extension tags for termination reason
        // in GSM 11.14 section 12.51, GSM 51.010 section 27.22.7.9.1.4.2
        if ( reason == ApplicationTypeLauncher::Normal ) {
            env.addExtensionField( 0xB4, QByteArray( 1, (char)(0x00) ) );   // User termination
        } else {
            env.addExtensionField( 0xB4, QByteArray( 1, (char)(0x01) ) );   // Error termination
        }
        sendEnvelope( env );
    }
}

UIFACTORY_REGISTER_WIDGET(SimApp);

//simapp is a task as well as an in-built application
static QWidget *simapp()
{
    static QPointer<QWidget> s = 0;
    if (!s)
        s = UIFactory::createWidget("SimApp");

    SimApp::homescreen();
    return s;
}

QTOPIA_SIMPLE_BUILTIN(simapp, simapp);
QTOPIA_STATIC_TASK( SimAppTask, simapp() );
#endif

#include "simapp.moc"
