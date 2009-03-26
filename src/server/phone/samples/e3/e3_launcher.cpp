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

#include "e3_launcher.h"
#include "e3_clock.h"
#include "e3_navipane.h"
#include "qabstractheader.h"
#include "qabstractcontextlabel.h"
#include "windowmanagement.h"
#include "themecontrol.h"
#include <QApplication>
#include <QDesktopWidget>
#include "qtopiaserverapplication.h"
#include "launcherview.h"
#include <custom.h>
#include <QVBoxLayout>
#include <QContentFilter>
#include <QCategoryFilter>
#include <QListWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <QExportedBackground>
#endif
#include <QStringListModel>
#include <QVariant>
#include <QPainter>
#include <QSettings>
#include <QPen>
#include <QSoftMenuBar>
#include <QtopiaChannel>
#include <QKeyEvent>
#ifdef Q_WS_QWS
#include <qscreen_qws.h>
#endif
#include "qabstractbrowserscreen.h"
#include "qabstractdialerscreen.h"
#include "qabstractmessagebox.h"
#include "qabstractcallpolicymanager.h"
#include "qabstractcallscreen.h"
#include "dialercontrol.h"
#include "applicationlauncher.h"
#include "qabstractcallhistory.h"
#include "qabstractthemewidgetfactory.h"
#include "e3_today.h"
#include "dialerservice.h"
#include <QLabel>
#include <QPalette>
#include <QTimeString>
#include <QOccurrence>
#include <QAppointment>
#include <QTask>
#include <ThemedView>

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, E3ServerInterface);

class NonModalLauncherView : public LauncherView
{
Q_OBJECT
public:
    NonModalLauncherView(QWidget *parent);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
};

NonModalLauncherView::NonModalLauncherView(QWidget *parent)
: LauncherView(parent)
{
    setViewMode(QListView::IconMode);
    m_icons->installEventFilter(this);
    m_icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_icons->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool NonModalLauncherView::eventFilter(QObject *, QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        resetSelection();
        m_icons->setEditFocus(true);
    } else if(e->type() == QEvent::FocusOut) {
        clearSelection();
    } else if(e->type() == QEvent::KeyPress ||
              e->type() == QEvent::KeyRelease) {

        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch(ke->key()) {
            case Qt::Key_Up:
                if(QEvent::KeyPress == e->type())
                    focusPreviousChild();
                return true;
            case Qt::Key_Down:
                if(QEvent::KeyPress == e->type())
                    focusNextChild();
                return true;
            case Qt::Key_Back:
                e->ignore();
                return true;
        }

    }

    return false;
}

class NonModalListView : public QListView
{
Q_OBJECT
public:
    enum Navigation { Up = 0x0001, Down = 0x0002, Both = 0x0003 };
    NonModalListView(Navigation, QWidget *parent);

protected:
    virtual bool eventFilter(QObject *, QEvent *);

private:
    Navigation nav;
};

NonModalListView::NonModalListView(Navigation n, QWidget *parent)
: QListView(parent), nav(n)
{
    installEventFilter(this);
}

bool NonModalListView::eventFilter(QObject *, QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        setEditFocus(true);
        if(model()->rowCount())
            setCurrentIndex(model()->index(0, 0));
    } else if(e->type() == QEvent::FocusOut) {
        clearSelection();
    } else if(e->type() == QEvent::KeyPress ||
              e->type() == QEvent::KeyRelease) {

        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch(ke->key()) {
            case Qt::Key_Up:
                if(QEvent::KeyPress == e->type() &&
                   (selectedIndexes().isEmpty() ||
                    selectedIndexes().first().row() == 0)) {
                    if(Up & nav)
                        focusPreviousChild();
                    return true;
                }
                break;
            case Qt::Key_Down:
                if(QEvent::KeyPress == e->type() &&
                   (selectedIndexes().isEmpty() ||
                    selectedIndexes().first().row() == (model()->rowCount() - 1))) {
                    if(Down & nav)
                        focusNextChild();
                    return true;
                }
                
                break;
            case Qt::Key_Back:
                e->ignore();
                return true;
        }

    }

    return false;
}

class E3ListDelegate : public QAbstractItemDelegate
{
Q_OBJECT
public:
    E3ListDelegate(QObject *parent);

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


E3ListDelegate::E3ListDelegate(QObject *parent)
: QAbstractItemDelegate(parent)
{
}

QSize E3ListDelegate::sizeHint(const QStyleOptionViewItem &,
                               const QModelIndex &) const
{
    return QSize(40, 40);
}

void E3ListDelegate::paint(QPainter *painter, 
                           const QStyleOptionViewItem &option, 
                           const QModelIndex &index) const
{
    QSize s = option.rect.size();
    s.setHeight(s.height() - 4);
    s.setWidth(s.width() - 4);
    QPixmap pix = 
        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(s);

    if(option.state & QStyle::State_Selected) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawRoundRect(option.rect, 600 / option.rect.width(),
                                            600 / option.rect.height());
        painter->restore();
    }

    int x = option.rect.x() + (option.rect.width() - pix.width()) / 2;
    int y = option.rect.y() + (option.rect.height() - pix.height()) / 2;
    painter->drawPixmap(x, y, pix);
}

class E3Separator : public QWidget
{
Q_OBJECT
public:
    E3Separator(QWidget *parent);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QColor color;
};

E3Separator::E3Separator(QWidget *parent)
: QWidget(parent)
{
    setFixedHeight(1);
    color = palette().brightText().color();
    color.setAlpha(60);
}

void E3Separator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(QPen(color, 1));
    p.drawLine(0, 0, width(), 0);
}

class E3HomescreenItem : public QWidget
{
Q_OBJECT
public:
    E3HomescreenItem(QWidget *parent = 0);


protected slots:
    virtual void activated();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

protected:
    bool isFocused;
};

E3HomescreenItem::E3HomescreenItem(QWidget *parent)
: QWidget(parent), isFocused(false)
{
    setFocusPolicy(Qt::StrongFocus);
}

void E3HomescreenItem::activated()
{
}

void E3HomescreenItem::paintEvent(QPaintEvent *)
{
    if(isFocused) {
        QPainter p(this);
        QPalette pal;
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(pal.highlight());
        p.setPen(Qt::NoPen);
        p.drawRoundRect(rect(), 600 / width(), 600 / height());
    }
}

void E3HomescreenItem::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Select) {
        activated();
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void E3HomescreenItem::focusInEvent(QFocusEvent *)
{
    isFocused = true;
    update();
}

void E3HomescreenItem::focusOutEvent(QFocusEvent *)
{
    isFocused = false;
    update();
}

class E3Label : public QWidget
{
Q_OBJECT
public:
    E3Label(QWidget *parent = 0);

    void setText(const QString &);
    void setPixmap(const QPixmap &);

signals:
    void activated();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    QLabel *text;
    QLabel *pixmap;
};

class E3CalItem : public E3HomescreenItem
{
Q_OBJECT
public:
    E3CalItem(E3Today *t, QWidget *parent = 0);

private slots:
    void dataChanged();
    void activated();

private:
    E3Today *today;
    E3Label *textLabel;
    E3Label *appointmentLabel;
};

class E3TodoItem : public E3HomescreenItem
{
Q_OBJECT
public:
    E3TodoItem(E3Today *t, QWidget *parent = 0);

private slots:
    void activated();
    void dataChanged();

private:
    E3Today *today;
    E3Label *textLabel;
};

class E3CallsItem : public E3HomescreenItem
{
Q_OBJECT
public:
    E3CallsItem(QWidget *parent = 0);

signals:
    void showCallscreen();

private slots:
    void activated();
    void stateChanged();

private:
    E3Label *callsLabel;
};

E3Label::E3Label(QWidget *parent)
: QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    setLayout(layout);

    pixmap = new QLabel(this);
    layout->addWidget(pixmap);

    text = new QLabel(this);
    text->setWordWrap( true );
    layout->addWidget(text);

    layout->addStretch(10);
}

void E3Label::setText(const QString &txt)
{
    text->setText(txt);
}

void E3Label::setPixmap(const QPixmap &pix)
{
    pixmap->setPixmap(pix);
}

void E3Label::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Select) {
        emit activated();
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void E3Label::mousePressEvent(QMouseEvent *e)
{
    emit activated();
    e->accept();
}

E3CalItem::E3CalItem(E3Today *t, QWidget *parent)
: E3HomescreenItem(parent), today(t), textLabel(0), appointmentLabel(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    E3Separator *sep = new E3Separator(this);
    layout->addWidget(sep);

    textLabel = new E3Label(this);
    QPixmap pix = QIcon(":icon/datebook/DateBook").pixmap(16);
    textLabel->setPixmap(pix);
    layout->addWidget(textLabel);
    appointmentLabel = new E3Label(this);
    appointmentLabel->setPixmap(pix);
    appointmentLabel->setText("World");
    layout->addWidget(appointmentLabel);

    QObject::connect(today, SIGNAL(todayChanged()), this, SLOT(dataChanged()));
    dataChanged();
    QObject::connect(textLabel, SIGNAL(activated()), this, SLOT(activated()));
    QObject::connect(appointmentLabel, SIGNAL(activated()), this, SLOT(activated()));
}

void E3CalItem::activated()
{
    QtopiaServiceRequest req("Calendar", "raiseToday()");
    req.send();
}

void E3CalItem::dataChanged()
{
    if(today->dayStatus() == E3Today::MoreAppointments) {
        textLabel->hide();
        appointmentLabel->show();
    } else if(today->nextAppointment().isValid()) {
        textLabel->show();
        textLabel->setText(tr("No more entries today", "calendar entries"));
        appointmentLabel->show();
    } else {
        textLabel->show();
        textLabel->setText(tr("No cal. entries for today", "cal=calendar"));
        appointmentLabel->hide();
    }

    if(today->nextAppointment().isValid()) {
        QOccurrence o = 
            today->nextAppointment().nextOccurrence(QDate::currentDate());

        QString str;
        str += o.start().toString(QTimeString::currentFormat());
        str += today->nextAppointment().description();

        appointmentLabel->setText(str);
    }
}


E3CallsItem::E3CallsItem(QWidget *parent)
: E3HomescreenItem(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    E3Separator *sep = new E3Separator(this);
    layout->addWidget(sep);

    callsLabel = new E3Label(this);
    QPixmap pix = QIcon(":icon/phone/calls").pixmap(16);
    callsLabel->setPixmap(pix);
    layout->addWidget(callsLabel);

    QObject::connect(DialerControl::instance(), SIGNAL(stateChanged()), 
                     this, SLOT(stateChanged()));

    QObject::connect(callsLabel, SIGNAL(activated()), this, SLOT(activated()));

    stateChanged();
}

void E3CallsItem::activated()
{
    emit showCallscreen();
}

void E3CallsItem::stateChanged()
{
    int calls = DialerControl::instance()->allCalls().count();
    callsLabel->setText(tr("%1 call(s) in progress", "%1=a number").arg(calls));
    if(calls)
        show();
    else
        hide();
}

E3TodoItem::E3TodoItem(E3Today *t, QWidget *parent)
: E3HomescreenItem(parent), today(t), textLabel(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    E3Separator *sep = new E3Separator(this);
    layout->addWidget(sep);

    textLabel = new E3Label(this);
    QPixmap pix = QIcon(":icon/todolist/TodoList").pixmap(16);
    textLabel->setPixmap(pix);
    layout->addWidget(textLabel);

    QObject::connect(today, SIGNAL(todayChanged()), this, SLOT(dataChanged()));
    QObject::connect(textLabel, SIGNAL(activated()), this, SLOT(activated()));
    dataChanged();
}

void E3TodoItem::activated()
{
    QtopiaServiceRequest req("Application:todolist", "raise()");
    req.send();
}

void E3TodoItem::dataChanged()
{
    int tasks = today->tasks();
    if(!tasks) {
        hide();
        return;
    }

    show();
    QString str;
    if(tasks == 1) {
        str = today->task(0).description();
    } else {
        str = QString(tr("%1 to-do notes not done", "%1 = number")).arg(tasks);
    }

    textLabel->setText(str);
}



static QWidget *callHistory()
{
    QAbstractCallHistory* history = qtopiaWidget<QAbstractCallHistory>();
    if ( !history )
        qLog(Component) << "E3Launcher: No callhistory component available";
    return history;
}

class E3DialerServiceProxy : public DialerService
{
Q_OBJECT
public:
    E3DialerServiceProxy(QObject *parent) : DialerService(parent) {}

protected:
    virtual void dialVoiceMail() {}
    virtual void dial( const QString&, const QString& number ) {
        emit doDial(number);
    }
    virtual void dial( const QString&number, const QUniqueId&) {
        emit doDial(number);
    }
    virtual void showDialer( const QString& digits ) {
        emit doShowDialer(digits);
    }

    virtual void onHook() {}
    virtual void offHook() {}
    virtual void headset() {}
    virtual void speaker() {}
    virtual void setDialToneOnlyHint( const QString &/*app*/ ) {}
    virtual void redial() {}

signals:
    void doDial( const QString& number );
    void doShowDialer( const QString& digits );
};

//===========================================================================

class E3WidgetFactory : public QAbstractThemeWidgetFactory
{
public:
    virtual bool createWidget(ThemeWidgetItem *item)
    {
        if (item->itemName() == QLatin1String("clock")) {
            E3Clock *clock = new E3Clock;
            clock->showCurrentTime();
            item->setWidget(clock);
            return true;
        } else if (item->itemName() == QLatin1String("navbar")) {
            item->setWidget(new E3NaviPane);
            return true;
        }
        return false;
    }
};

//===========================================================================

E3ServerInterface::E3ServerInterface(QWidget *parent, Qt::WFlags flags)
: QAbstractServerInterface(parent, flags), m_view(0), m_header(0), m_context(0),
  m_browser(0), m_dialer(0), m_callscreen(0), m_today(0), m_theme(0), m_cell(0)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QSize desktopSize = QExportedBackground::exportedBackgroundSize();
    QExportedBackground::initExportedBackground(desktopSize.width(),
                                                desktopSize.height());
#endif
    themeCtrl = qtopiaTask<ThemeControl>();
    if ( themeCtrl )
        themeCtrl->setThemeWidgetFactory(new E3WidgetFactory);
    else 
        qLog(Component) << "E3ServerInterface: ThemeControl not available, Theme will not work properly";

    // Install call history builtin
    BuiltinApplicationLauncher::install("callhistory", callHistory);

    // Create dialer control
    DialerControl::instance();
    QObject::connect(DialerControl::instance(), SIGNAL(callIncoming(QPhoneCall)), this, SLOT(showCallscreen()));

    // Get cell policy manager
    m_cell = QAbstractCallPolicyManager::managerForCallType( "Voice" ); //no tr

    // Create header
    header();
    // Create context
    context();

    if (themeCtrl) {
        QObject::connect(this, SIGNAL(themeLoaded()),
                    m_header, SLOT(themeLoaded()));
        QObject::connect(this, SIGNAL(themeLoaded()),
                    m_context, SLOT(themeLoaded()));
    }

    m_theme = new ThemedView(this);
    m_theme->setGeometry(desktopRect);
    m_tbackground = new ThemeBackground(this);
    QVBoxLayout *layout = new QVBoxLayout(m_theme);
    m_theme->setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    if ( themeCtrl )
        themeCtrl->registerThemedView(m_theme, "Home");

    m_titleSpacer = new QWidget;
    layout->addWidget(m_titleSpacer);

    E3Separator *sep = new E3Separator(m_theme);
    layout->addWidget(sep);

    // Quick launcher bar
    m_view = new NonModalLauncherView(m_theme);
    m_view->setFixedHeight(37);
    m_view->setFixedWidth(230);
    m_view->setItemDelegate(new E3ListDelegate(m_view));

    QHBoxLayout *l2 = new QHBoxLayout(m_theme);
    l2->setMargin(3);
    l2->addSpacing(7);
    l2->addWidget(m_view);
    layout->addLayout(l2);

    QObject::connect(m_view, SIGNAL(clicked(QContent)), this, SLOT(launch(QContent)));
    quickApps();

    // Info list
    m_today = new E3Today(this);
    E3CalItem *calItem = new E3CalItem(m_today, m_theme);
    layout->addWidget(calItem);
    E3TodoItem *todoItem = new E3TodoItem(m_today, m_theme);
    layout->addWidget(todoItem);
    E3CallsItem *callsItem = new E3CallsItem(m_theme);
    QObject::connect(callsItem, SIGNAL(showCallscreen()), 
                     this, SLOT(showCallscreen()));
    layout->addWidget(callsItem);

    layout->addStretch(10);
    
    // Context buttons
    idleContext();

    QtopiaChannel *e3 = new QtopiaChannel("QPE/E3", this);
    QObject::connect(e3, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(received(QString,QByteArray)));

    // Themes
    if ( themeCtrl )
        QObject::connect(themeCtrl, SIGNAL(themeChanged()),
                     this, SLOT(loadTheme()));
    loadTheme();

    // Dialer service
    E3DialerServiceProxy *proxy = new E3DialerServiceProxy(this);
    QObject::connect(proxy, SIGNAL(doDial(QString)), this, SLOT(requestDial(QString)));
    QObject::connect(proxy, SIGNAL(doShowDialer(QString)), this, SLOT(showDialer(QString)));

#ifdef Q_WS_QWS
    // Window management
    connect(qwsServer, SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
            this, SLOT(windowEvent(QWSWindow*,QWSServer::WindowEvent)) );
#endif
}

bool E3ServerInterface::eventFilter(QObject *, QEvent *e)
{
    if(e->type() == QEvent::Resize) {
        if(m_theme) {
            m_theme->setGeometry(0, 0, width(), height());
            m_titleSpacer->setFixedSize(1,m_header->height());
        }
    }

    return false;
}

bool E3ServerInterface::event(QEvent *e)
{
    if(e->type() == QEvent::WindowActivate ||
       e->type() == QEvent::Show)
        m_today->forceUpdate();

    return QAbstractServerInterface::event(e);
}

void E3ServerInterface::received(const QString &message, const QByteArray &)
{
    if(message == "showApplications()")
        showApps();
}

void E3ServerInterface::acceptIncoming()
{
    DialerControl::instance()->accept();
}

void E3ServerInterface::showCallscreen()
{
    if ( callscreen() ) {
        dialer()->close();
        callscreen()->showMaximized();
        callscreen()->raise();
    }
}

void E3ServerInterface::showDialer(const QString &number)
{
    if(DialerControl::instance()->allCalls().isEmpty()) {

        dialer()->reset();
        dialer()->setDigits(number);
        dialer()->showMaximized();
        dialer()->raise();

    }
}

void E3ServerInterface::requestDial(const QString &number)
{
    if(!m_cell || 
            (m_cell->registrationState() != QTelephony::RegistrationHome && 
             m_cell->registrationState() != QTelephony::RegistrationRoaming )) {
        QAbstractMessageBox::information(this, tr("Not Registered"), tr("Cannot make call until network is registered."));
    } else if(!DialerControl::instance()->allCalls().isEmpty()) {
        QAbstractMessageBox::information(this, tr("Active call"), tr("Cannot make call while other call in progress."));
    } else if ( callscreen() ) {
        DialerControl::instance()->dial(number, DialerControl::instance()->callerIdNeeded(number));
        callscreen()->showMaximized();
        callscreen()->raise();
    }
}

QAbstractDialerScreen *E3ServerInterface::dialer()
{
    if(!m_dialer) {
        m_dialer = qtopiaWidget<QAbstractDialerScreen>();
        QObject::connect(m_dialer, SIGNAL(requestDial(QString,QUniqueId)),
                         this, SLOT(requestDial(QString)));
    }

    return m_dialer;
}

QAbstractCallScreen *E3ServerInterface::callscreen()
{
    if(!m_callscreen) {
        m_callscreen = qtopiaWidget<QAbstractCallScreen>(0);
        if ( !m_callscreen ) {
            qLog(Component) << "E3ServerInterface: Callscreen component unavailable";
            return 0;
        }
        QObject::connect(m_callscreen, SIGNAL(acceptIncoming()),
                         this, SLOT(acceptIncoming()));
        QObject::connect(DialerControl::instance(), SIGNAL(stateChanged()),
                         m_callscreen, SLOT(stateChanged()));
    }

    return m_callscreen;
}

void E3ServerInterface::showApps()
{
    if(!m_browser)
        m_browser = qtopiaWidget<QAbstractBrowserScreen>();

    if(m_browser) {
        m_browser->showMaximized();
        m_browser->raise();
        m_browser->resetToView("Main");
    }
}

void E3ServerInterface::header()
{
    if (!m_header) {
        m_header = qtopiaWidget<QAbstractHeader>();
        if (!m_header) {
            qLog(Component) << "E3ServerInterface: QAbstractHeader not available";
        } else {
            m_header->installEventFilter(this);
            WindowManagement::protectWindow(m_header);
            QTimer::singleShot(0, m_header, SLOT(show()));
        }
    }
}

void E3ServerInterface::context()
{
    if (!m_context) {
        m_context = qtopiaWidget<QAbstractContextLabel>();
        if(!m_context) {
            qLog(UI) << "E3 ServerInterface: QAbstractContext not available";
        } else {
            //m_context->move(QApplication::desktop()->screenGeometry().topLeft()); // move to the correct screen
            WindowManagement::protectWindow(m_context);
            m_context->setAttribute(Qt::WA_GroupLeader);
            QTimer::singleShot(0, m_context, SLOT(show()));
        }
    }
}

void E3ServerInterface::keyPressEvent(QKeyEvent *e)
{
    IdleKeys::ConstIterator iter = m_idleKeys.find(e->key());
    if(iter != m_idleKeys.end()) {
        QtopiaServiceRequest req = *iter;
        req.send();
        e->accept();
    } else if((e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9) ||
              e->key() == Qt::Key_NumberSign || e->key() == Qt::Key_Asterisk) {

        if(DialerControl::instance()->allCalls().isEmpty()) {

            dialer()->reset();
            dialer()->appendDigits(e->text());
            dialer()->showMaximized();
            dialer()->raise();
        }
        e->accept();

    } else if(e->key() == Qt::Key_Yes || e->key() == Qt::Key_Call) {
        if(DialerControl::instance()->allCalls().isEmpty()) {
            QtopiaServiceRequest req("Application:callhistory", "raise()");
            req.send();
        }
    } else {
        QAbstractServerInterface::keyPressEvent(e);
    }
}

void E3ServerInterface::loadTheme()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
    WindowManagement::dockWindow(m_header, WindowManagement::Top, 
                                 m_header->reservedSize());
    m_theme->setGeometry(0, 0, width(), height());
    m_titleSpacer->setFixedSize(1,m_header->height());

    WindowManagement::dockWindow(m_context, WindowManagement::Bottom, 
                                 m_context->reservedSize());


    m_tbackground->updateBackground();
}

void E3ServerInterface::quickApps()
{
    QSettings settings("Trolltech", "E3");
    settings.beginGroup("QuickApps");

    int count = settings.value("Count", 0).toInt();
    if(!count) return;

    int apps = 0;

    QContentSet set(QContentFilter::Role, "Applications");

    for(int ii = 0; ii < count; ++ii) {
        QString app = settings.value("Application" + QString::number(ii), QString()).toString();
        QContent c = set.findExecutable(app);
        if(c.isValid()) {
            ++apps;
            m_view->addItem(&c);
        }
    }

    m_view->setColumns(apps);
}

void E3ServerInterface::idleContext()
{
    QSettings settings("Trolltech", "E3");
    settings.beginGroup("IdleButtons");
    settings.beginGroup("Context1");

    ButtonBinding bb = buttonBinding(settings);
    if(!bb.first.isEmpty()) {
        m_idleKeys.insert(Qt::Key_Context1, bb.second);
        QSoftMenuBar::setLabel(this, Qt::Key_Context1, QString(), bb.first);
    }

    settings.endGroup();
    settings.beginGroup("Context2");

    bb = buttonBinding(settings);
    if(!bb.first.isEmpty()) {
        m_idleKeys.insert(Qt::Key_Back, bb.second);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), bb.first);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), QString());
    }
}

E3ServerInterface::ButtonBinding E3ServerInterface::buttonBinding(QSettings & settings) const
{
    QPair<QString, QtopiaServiceRequest> rv;
    QString text = settings.value("Text").toString();
    QString service = settings.value("Service").toString();
    QString message = settings.value("Message").toString();

    if(!text.isEmpty() && !service.isEmpty() && !message.isEmpty()) {
        rv.first = text;
        rv.second.setService(service);
        rv.second.setMessage(message);
    }

    return rv;
}

void E3ServerInterface::launch(QContent c)
{
    c.execute();
}

#ifdef Q_WS_QWS

void E3ServerInterface::windowEvent(QWSWindow *w, QWSServer::WindowEvent e)
{
    if (!w)
        return;

    static QWidget *shadeWindow = 0;
    static int topWindow = 0;

    switch( e ) {
        case QWSServer::Raise:
            if (!w->isVisible())
                break;
            // else FALL THROUGH
        case QWSServer::Show:
            if (w->name() != "_fullscreen_") { // bogus - Qt needs to send window flags to server.
                QRect req = w->requestedRegion().boundingRect();
                QSize s(qt_screen->deviceWidth(),
                        qt_screen->deviceHeight());
                req = qt_screen->mapFromDevice(req, s);

                QDesktopWidget *desktop = QApplication::desktop();
                QRect availRect = desktop->availableGeometry(desktop->primaryScreen());
                if (req != availRect && req.width() >= qt_screen->deviceWidth()) {
                    QRect rect(0,0,qt_screen->deviceWidth(),qt_screen->deviceHeight());
                    rect.setBottom(req.top()-1);
                    if (!shadeWindow || topWindow != w->winId()
                        || rect != shadeWindow->geometry()) {
                        if (!shadeWindow) {
                            shadeWindow = new QWidget(0, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Tool);
                            QColor col(Qt::white);
                            col.setAlpha(160);
                            QPalette pal = shadeWindow->palette();
                            pal.setBrush(QPalette::Window, col);
                            shadeWindow->setPalette(pal);
                            shadeWindow->setObjectName("_fullscreen_");
                        }
                        topWindow = w->winId();
                        shadeWindow->setGeometry(rect);
                        shadeWindow->raise();
                        shadeWindow->show();
                    }
                } else {
                    if (shadeWindow && topWindow == w->winId()) {
                        shadeWindow->hide();
                        topWindow = 0;
                    }
                }
            }
            break;
        case QWSServer::Hide:
            if (w->winId() == topWindow && shadeWindow) {
                shadeWindow->hide();
                topWindow = 0;
            }
        default:
            break;
    }
}

#endif // Q_WS_QWS

#include "e3_launcher.moc"
