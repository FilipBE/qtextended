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

#include "e1_phonebrowser.h"
#include <QPainter>
#include "launcherview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPixmap>
#include <QIcon>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QMouseEvent>
#include <QMenu>
#include "e1_bar.h"
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif
#include <QSignalMapper>
#include "e1_popup.h"
#include "e1_battery.h"
#include "colortint.h"

// declare E1PhoneBrowserBar
class E1PhoneBrowserBar : public E1Bar
{
    Q_OBJECT
public:
    E1PhoneBrowserBar(QWidget *);

signals:
    void toListView();
    void toIconView();

private slots:
    void operatorChanged();
    void profileChanged();
    void profileSelected(int);
    void changeToList();
    void changeToIcon();

private:
    void createProfilesMenu();

    E1Popup *m_toIcon;
    E1Popup *m_toList;

    QValueSpaceItem profile;
    QValueSpaceItem currentOperator;
    E1Button * operatorText;
    E1Menu * profiles;
    E1Menu * context;
    QString m_profileName;
    QList<int> m_profileList;
};

// define E1PhoneTelephonyBar
E1PhoneTelephonyBar::E1PhoneTelephonyBar(QWidget *parent)
: E1Bar(parent),
  time("/UI/DisplayTime/Time"),
  signal("/Hardware/Accessories/QSignalSource/DefaultSignal/SignalStrength"),
  batteryPixmap(":image/samples/e1_bat"),
  signalPixmap(":image/samples/e1_signal")
{
    addItem(new E1Spacer(3));
    signalBut = new E1Button;
    signalBut->clearFlag(E1Button::Clickable);
    addItem(signalBut);
    addItem(new E1BarItem);
    batteryBut = new E1Button;
    batteryBut->clearFlag(E1Button::Clickable);
    addItem(batteryBut);
    addItem(new E1Spacer(3));
    timeBut = new E1Button;
    batteryBut->clearFlag(E1Button::Clickable);
    addItem(timeBut);
    addItem(new E1Spacer(3));

    E1Battery *bat = new E1Battery(this);

    QObject::connect(&time, SIGNAL(contentsChanged()),
                     this, SLOT(timeChanged()));
    QObject::connect(&signal, SIGNAL(contentsChanged()),
                     this, SLOT(signalChanged()));
    QObject::connect(bat, SIGNAL(chargeChanged(int)), 
                     this, SLOT(batteryChanged(int)));

    batteryChanged(bat->charge());
    signalChanged();
    timeChanged();
}

void E1PhoneTelephonyBar::batteryChanged(int percent)
{
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;

    // Clamp to image
    int image = 0;
    if(percent < 25)
        image = 0;
    else if(percent < 50)
        image = 1;
    else if(percent < 75)
        image = 2;
    else
        image = 3;

    // Display image
    QPixmap pix = batteryPixmap.copy(QRect(image * batteryPixmap.width() / 4, 0,
                                      batteryPixmap.width() / 4,
                                      batteryPixmap.height()));

    batteryBut->setPixmap(pix, false, false);
}

void E1PhoneTelephonyBar::signalChanged()
{
    int percent = signal.value().toInt();
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;

    int image = 0;
    if(percent < 16)
        image = 0;
    else if(percent < 32)
        image = 1;
    else if(percent < 48)
        image = 2;
    else if(percent < 64)
        image = 3;
    else if(percent < 80)
        image = 4;
    else
        image = 5;

    // Display image
    QRect cp(image * signalPixmap.width() / 6, 0,
             signalPixmap.width() / 6, signalPixmap.height());
    QPixmap pix = signalPixmap.copy(cp);

    signalBut->setPixmap(pix, false, false);
}

void E1PhoneTelephonyBar::timeChanged()
{
    QString ctime = time.value().toString();

    timeBut->setText(ctime, false);
}

// define E1PhoneBrowserBar
E1PhoneBrowserBar::E1PhoneBrowserBar(QWidget *parent)
: E1Bar(parent),
  profile("/UI/Profile/Name"),
  currentOperator("/Telephony/Status/OperatorName"),
  operatorText(0),
  profiles(0)
{
    E1CloseButton * but = new E1CloseButton;
    but->setMargin(3);
    addItem(but);

    addSeparator();

    operatorText = new E1Button;
    operatorText->clearFlag(E1Button::Clickable);
    operatorText->setFlag(E1Button::Expanding);
    addItem(operatorText);

    addSeparator();

    profiles = new E1Menu;
    profiles->setMargin(3);
    createProfilesMenu();
    addItem(profiles);

    addSeparator();

    m_toList = new E1Popup;
    m_toList->addItem("List View");
    QObject::connect(m_toList, SIGNAL(selected(int)),
                     this, SLOT(changeToList()));
    m_toIcon = new E1Popup;
    m_toIcon->addItem("Thumbnail View");
    QObject::connect(m_toIcon, SIGNAL(selected(int)),
                     this, SLOT(changeToIcon()));

    context = new E1Menu;
    context->setMargin(3);
    context->setPixmap(QPixmap(":image/samples/e1_context"));
    addItem(context);
    context->setMenu(m_toList);

    QObject::connect(&currentOperator, SIGNAL(contentsChanged()),
                     this, SLOT(operatorChanged()));
    QObject::connect(&profile, SIGNAL(contentsChanged()),
                     this, SLOT(profileChanged()));

    operatorChanged();
    profileChanged();
}

void E1PhoneBrowserBar::changeToList()
{
    context->setMenu(m_toIcon);
    emit toListView();
}

void E1PhoneBrowserBar::changeToIcon()
{
    context->setMenu(m_toList);
    emit toIconView();
}

void E1PhoneBrowserBar::operatorChanged()
{
    QString oper = currentOperator.value().toString();
    if(oper.isEmpty()) {
        operatorText->setText("No Operator", false);
    } else {
        operatorText->setText(oper, false);
    }
}

void E1PhoneBrowserBar::profileChanged()
{
    QPhoneProfileManager manager;
    QPhoneProfile prof = manager.activeProfile();
    profiles->setPixmap(QPixmap(prof.icon()));
}

void E1PhoneBrowserBar::profileSelected(int p)
{
    Q_ASSERT(p >= 0 && p < m_profileList.count());
    QPhoneProfileManager manager;
    manager.activateProfile(m_profileList.at(p));
}

void E1PhoneBrowserBar::createProfilesMenu()
{
    // This is a silly way to create the icons, but whatever
    E1Bar * bar = new E1Bar;
    bar->setBorder(E1Bar::ButtonBorder);
    bar->setFixedHeight(32);
    bar->setFixedWidth(36);
    E1Button * button = new E1Button;
    bar->addItem(button);

    QObject::connect(this->profiles->menu(), SIGNAL(selected(int)),
                     this, SLOT(profileSelected(int)));

    QPhoneProfileManager manager;
    QList<QPhoneProfile> profiles = manager.profiles();

    foreach(QPhoneProfile profile, profiles) {
        if(-1 != profile.id() &&
           !profile.icon().isEmpty()) {

            QPixmap pix(profile.icon());
            if(pix.isNull())
                continue;

            button->setPixmap(pix);
            QPixmap wid = QPixmap::grabWidget(bar);
            this->profiles->menu()->addItem(wid);
            m_profileList.append(profile.id());
        }
    }

    delete bar;
}

// declare E1PhoneBrowserTabs
class E1PhoneBrowserTabs : public QWidget
{
Q_OBJECT
public:
    E1PhoneBrowserTabs(QWidget *parent);

    void addTab(const QString &, const QIcon &);

    QString currentTab() const;
    int currentTabIndex() const;
    int tabCount() const;
    void setTab(int);
    void setTab(const QString &);


    virtual QSize sizeHint() const;

signals:
    void tabChanged(const QString &);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    void drawTabLine(QPainter *);
    void drawTab(QPainter *, int);
    QRect visualTabRect(int) const;

    static const unsigned int tabWidth = 18;
    static const unsigned int spacing = 3;
    static const unsigned int topSpacing = 2;
    static const unsigned int leftMargin = 1;
    static const unsigned int curvRadius = 3;
    static const unsigned int tabSize = 43;

    int selectedTab;
    QStringList tabs;
    QList<QIcon> tabIcons;
};

// define E1PhoneBrowserTabs

E1PhoneBrowserTabs::E1PhoneBrowserTabs(QWidget *parent)
: QWidget(parent), selectedTab(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void E1PhoneBrowserTabs::addTab(const QString &name, const QIcon &icon)
{
    tabs.append(name);
    tabIcons.append(icon);
    update();
}

QString E1PhoneBrowserTabs::currentTab() const
{
    if(tabs.isEmpty())
        return QString();
    return tabs.at(selectedTab);
}

int E1PhoneBrowserTabs::currentTabIndex() const
{
    return selectedTab;
}

int E1PhoneBrowserTabs::tabCount() const
{
    return tabs.count();
}

void E1PhoneBrowserTabs::setTab(int tab)
{
    Q_ASSERT(tab < tabCount());
    if(selectedTab != tab) {
        selectedTab = tab;
        update();
        emit tabChanged(tabs.at(tab));
    }
}

void E1PhoneBrowserTabs::setTab(const QString &tab)
{
    for(int ii = 0; ii < tabs.count(); ++ii) {
        if(tabs.at(ii) == tab) {
            if(selectedTab != ii) {
                selectedTab = ii;
                update();
                emit tabChanged(tabs.at(ii));
            }
            return;
        }
    }
}

void E1PhoneBrowserTabs::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    drawTabLine(&painter);

    for(int ii = 0; ii < tabs.count(); ++ii)
        drawTab(&painter, ii);
}

QSize E1PhoneBrowserTabs::sizeHint() const
{
    return QSize(leftMargin + tabWidth + 1, height());
}

void E1PhoneBrowserTabs::mousePressEvent(QMouseEvent *e)
{
    for(int ii = 0; ii < tabs.count(); ii++) {
        QRect tabRect = visualTabRect(ii);
        if(tabRect.contains(e->pos())) {
            selectedTab = ii;
            update();
            emit tabChanged(tabs.at(ii));
            e->accept();
            return;
        }
    }

    QWidget::mousePressEvent(e);
}

void E1PhoneBrowserTabs::drawTabLine(QPainter *painter)
{
    for(int ii = 0; ii < 2; ++ii) {
        if(0 == ii)
            painter->setPen(palette().color(QPalette::Dark));
        else
            painter->setPen(palette().color(QPalette::Mid));

        int currentY = 0;
        int currentX = tabWidth - ii + leftMargin;

        // Draw the top lead in
        painter->drawLine(currentX, currentY,
                          currentX, currentY + topSpacing);
        currentY += topSpacing;

        // Draw each of the tabs
        for(int jj = 0; jj < tabs.count(); ++jj) {
            // Draw space
            painter->drawLine(currentX, currentY,
                              currentX, currentY + spacing);
            currentY += spacing;

            // Draw tab
            if(jj != selectedTab)
                painter->drawLine(currentX, currentY,
                                  currentX, currentY + tabSize);
            currentY += tabSize;
        }

        // Draw end
        painter->drawLine(currentX, currentY,
                          currentX, height());
    }
}

QRect E1PhoneBrowserTabs::visualTabRect(int tab) const
{
    int tabPos = (tab + 1) * spacing + tab * tabSize + topSpacing;
    QRect tabRect(leftMargin, tabPos,
                  tabWidth, tabSize + 1);
    return tabRect;
}

void E1PhoneBrowserTabs::drawTab(QPainter *painter, int tab)
{
    QColor light = palette().color(QPalette::Light);
    QColor dark = palette().color(QPalette::Dark);
    QColor highlight = ColorTint::lighten(palette().color(QPalette::Highlight), 30);
    QColor darkHighlight = palette().color(QPalette::Highlight);


    QRect tabRect = visualTabRect(tab);

    QPainterPath path;
    path.moveTo(tabRect.topRight());
    path.lineTo(tabRect.topLeft().x() + curvRadius, tabRect.topLeft().y());
    path.arcTo(tabRect.topLeft().x(), tabRect.topLeft().y(),
               curvRadius * 2, curvRadius * 2, 90, 90);
    path.lineTo(tabRect.bottomLeft().x(),
                tabRect.bottomLeft().y() - curvRadius);
    path.arcTo(tabRect.bottomLeft().x(),
               tabRect.bottomLeft().y() - 2 * curvRadius,
               2 * curvRadius, 2 * curvRadius, 180, 90);
    path.lineTo(tabRect.bottomRight());

    if(tab != selectedTab) {
        path.closeSubpath();

        // Fill
        QLinearGradient grad(tabRect.topLeft().x(),
                             tabRect.topLeft().y(),
                             tabRect.bottomLeft().x(),
                             tabRect.bottomLeft().y());
        grad.setColorAt(0, light);
        grad.setColorAt(0.2, highlight);
        grad.setColorAt(0.4, darkHighlight);
        grad.setColorAt(1, darkHighlight);

        painter->setPen(dark);
        painter->setBrush(grad);
        painter->drawPath(path);

    } else {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(palette().color(QPalette::Dark));
        painter->drawPath(path);
    }

    QPixmap pix = tabIcons.at(tab).pixmap(tabWidth - 3, tabWidth - 3);
    painter->drawPixmap(tabRect.x() + (tabRect.width() - pix.width()) / 2,
                        tabRect.y() + (tabRect.height() - pix.height()) / 2,
                        pix);
}

// define E1PhoneBrowser
E1PhoneBrowser::E1PhoneBrowser(QWidget *parent, Qt::WFlags wflags)
: LazyContentStack(NoStack, parent, wflags), m_tabs(0),
  appCategories(QContentFilter( QContent::Folder ) & QContentFilter::category(QLatin1String("MainApplications"))),
  m_mode(QListView::IconMode)
{
    QVBoxLayout * vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    E1PhoneTelephonyBar * tbar = new E1PhoneTelephonyBar(this);
    tbar->setFixedHeight(26);
    vlayout->addWidget(tbar);

    QHBoxLayout * layout = new QHBoxLayout;
    layout->setMargin(0);

    vlayout->addLayout(layout);

    m_tabs = new E1PhoneBrowserTabs(this);
    layout->addWidget(m_tabs);
    QObject::connect(m_tabs, SIGNAL(tabChanged(QString)),
                     this, SLOT(tabChanged(QString)));

    QContentSetModel model(&appCategories);
    for(int ii = 0; ii < model.rowCount(); ++ii) {
        QContent c = model.content( ii );
        m_tabs->addTab(c.type().mid( 7 ), c.icon());
    }

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);
    m_stack->setFocusPolicy(Qt::NoFocus);

    E1PhoneBrowserBar * bar = new E1PhoneBrowserBar(this);
    bar->setFixedHeight(32);
    vlayout->addWidget(bar);
    QObject::connect(bar, SIGNAL(toListView()), this, SLOT(toList()));
    QObject::connect(bar, SIGNAL(toIconView()), this, SLOT(toIcon()));

    tabChanged(m_tabs->currentTab());

    // Listen to header channel
    QCopChannel* channel = new QCopChannel( "QPE/E1", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(message(QString,QByteArray)) );
}

QObject* E1PhoneBrowser::createView(const QString &category)
{
    if(category == "Main")
        return 0;

    LauncherView *alv = LauncherView::createLauncherView( "ApplicationLauncherView", m_stack );
    if ( !alv ) {
        qLog(Component) << "E1PhoneBrowser: ApplicationLauncherView not available";
        return 0;
    }
    QContentFilter filters = (QContentFilter( QContent::Application ) | QContentFilter( QContent::Folder ))
                & QContentFilter( QContentFilter::Category, category );
    alv->showCategory( filters );
    alv->setColumns(2);
    alv->setViewMode(m_mode);
    stopFocus(alv);
    m_views.insert(category, alv);
    m_stack->addWidget(alv);
    return alv;
}

QObject* E1PhoneBrowser::currentViewObject()
{
    return m_stack->currentWidget();
}


void E1PhoneBrowser::stopFocus(QObject *obj)
{
    foreach(QObject * cobj, obj->children())
        stopFocus(cobj);

    if(obj->isWidgetType()) {
        static_cast<QWidget *>(obj)->setFocusPolicy(Qt::NoFocus);
    }
}

void E1PhoneBrowser::raiseView(const QString &category, bool)
{
    Q_ASSERT(m_views.contains(category));
    QWidget * wid = m_views[category];
    m_stack->setCurrentWidget(wid);
    m_tabs->setTab(category);
}

void E1PhoneBrowser::tabChanged(const QString &tab)
{
    resetToView(tab);
}

void E1PhoneBrowser::display()
{
    showMaximized();
    raise();
    activateWindow();
}

void E1PhoneBrowser::message(const QString &message, const QByteArray &)
{
    if(message == "showHome()") {
        display();
    }
}

void E1PhoneBrowser::toList()
{
    m_mode = QListView::ListMode;
    foreach(QWidget *v, m_views)
        static_cast<LauncherView *>(v)->setViewMode(m_mode);
}

void E1PhoneBrowser::toIcon()
{
    m_mode = QListView::IconMode;
    foreach(QWidget *v, m_views)
        static_cast<LauncherView *>(v)->setViewMode(m_mode);
}

void E1PhoneBrowser::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Down:
            {
                int newTab = m_tabs->currentTabIndex() + 1;
                newTab = newTab % m_tabs->tabCount();
                m_tabs->setTab(newTab);
            }
            break;
        case Qt::Key_Up:
            {
                int newTab = m_tabs->currentTabIndex() - 1;
                if(newTab < 0)
                    newTab = m_tabs->tabCount() - 1;
                m_tabs->setTab(newTab);
            }
            break;
        default:
            break;
    };

    e->accept();
}

void E1PhoneBrowser::resetToView(const QString &view)
{
    if("Main" == view)
        LazyContentStack::resetToView("Applications");
    else
        LazyContentStack::resetToView(view);
}

void E1PhoneBrowser::showView(const QString &view)
{
    if("Main" == view)
        LazyContentStack::showView("Applications");
    else
        LazyContentStack::showView(view);
}

#include "e1_phonebrowser.moc"
