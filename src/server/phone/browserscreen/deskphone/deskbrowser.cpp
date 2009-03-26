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

#include "deskbrowser.h"
#include "windowmanagement.h"
#include "qtopiaserverapplication.h"
#include "lazycontentstack.h"
#include "serverthemeview.h"
#include "qabstracthomescreen.h"

#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QContentFilter>
#include <QCategoryManager>
#include <QtopiaItemDelegate>
#include <QKeyEvent>
#include <QSoftMenuBar>
#include <QStackedWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QLinearGradient>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>

#include "applicationlauncher.h"
#include "launcherview.h"

static QWidget *dialer()
{
    QtopiaServiceRequest e( "Dialer", "showDialer(QString)" );
    e << QString();
    e.send();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(dialer, dialer);

class DeskphoneBrowserStack : public LazyContentStack
{
    Q_OBJECT
public:
    DeskphoneBrowserStack(QWidget *parent = 0);
    void showView(const QString &view);
    void resetToView(const QString &view);

    void setViewMode(QListView::ViewMode);

    QString currentName() const;

signals:
    void launched(const QString &);

protected:
    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool reset);
    virtual void busy(const QContent &);
    virtual QObject* currentViewObject();

private:
    QStackedWidget *m_stack;
    QMap<QString, QWidget *> m_views;
    QString m_currentView;
};

//===========================================================================

class CaptionedLauncherView : public QWidget
{
    Q_OBJECT
public:
    CaptionedLauncherView(LauncherView *view, QWidget *parent=0) : QWidget(parent)
    {
        QVBoxLayout *vbl = new QVBoxLayout;
        QToolButton *backBtn = new QToolButton;
        backBtn->setText(tr("Back"));
        m_caption = new QLabel;
        m_caption->setAlignment(Qt::AlignCenter);
        QPalette pal = m_caption->palette();
        pal.setBrush(QPalette::Window, pal.brush(QPalette::Button));
        m_caption->setPalette(pal);
        m_caption->setAutoFillBackground(true);
        QHBoxLayout *hbl = new QHBoxLayout;
        vbl->addLayout(hbl);
        hbl->addWidget(m_caption);
        hbl->addWidget(backBtn);
        vbl->setMargin(0);
        vbl->setSpacing(0);
        vbl->addWidget(view);
        setLayout(vbl);
        connect(view, SIGNAL(clicked(QContent)), this, SIGNAL(clicked(QContent)));
        connect(backBtn, SIGNAL(clicked()), this, SIGNAL(back()));
    }

    void setCaption(const QString &caption) {
        m_caption->setText(caption);
    }

signals:
    void clicked(const QContent &);
    void back();

private:
    QLabel *m_caption;
};

//===========================================================================

class HomeBrowserDelegate : public QtopiaItemDelegate
{
public:
    HomeBrowserDelegate(QObject *parent=0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QRect &rect, const QVariant &decoration) const;
    using QtopiaItemDelegate::drawDecoration;
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    inline QIcon::Mode iconMode(QStyle::State state) const {
        if (!(state & QStyle::State_Enabled))
            return QIcon::Disabled;
        return QIcon::Normal;
    }

    inline QIcon::State iconState(QStyle::State state) const {
        return state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    }

private:
    int margin;
    QSize maxSize;
    QPixmap bg;
};

HomeBrowserDelegate::HomeBrowserDelegate(QObject *parent)
    : QtopiaItemDelegate(parent)
{

    maxSize = QSize(QApplication::desktop()->availableGeometry().width()/5,QApplication::desktop()->availableGeometry().height()/4);
#ifndef QTOPIA_HOMEUI_WIDE
    bg = QPixmap(":image/home/launcher_icon_bg");
#endif
    margin = 4;
}

void HomeBrowserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV3 opt = setOptions(index, option);
    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features
                    : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);
    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale();
    opt.widget = v3 ? v3->widget : 0;

    QRect rect = opt.rect;
    if (!bg.isNull()) {
        rect.setX(rect.x() + (rect.width() - bg.width())/2);
        rect.setWidth(bg.width());
        rect.setY(rect.y() + (rect.height() - bg.height())/2);
        rect.setHeight(bg.height());
    }

    painter->save();
    if (hasClipping())
        painter->setClipRect(opt.rect);

    drawBackground(painter, opt, index);

    QString text;
    QVariant value = index.data(Qt::DisplayRole);
    if (value.isValid())
        text = value.toString();
    QRect textRect = rect;
    textRect.setY(textRect.bottom() - painter->fontMetrics().height());
    drawDisplay(painter, opt, textRect, text);

    QRect decRect = rect;
    drawDecoration(painter, opt, decRect, index.data(Qt::DecorationRole));

    painter->restore();
}

void HomeBrowserDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    QtopiaItemDelegate::drawDisplay(painter, option, rect, text);
}

void HomeBrowserDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QRect &rect, const QVariant &decoration) const {
    if (decoration.type() == QVariant::Icon) {
        QIcon icon = qvariant_cast<QIcon>(decoration);
        QIcon::Mode mode = iconMode(option.state);
        QIcon::State state = iconState(option.state);
        if (bg.isNull()) {
            QSize s = maxSize;
            if (rect.width() < maxSize.width())
                s.setWidth(rect.width());
            if (rect.height() < maxSize.height())
                s.setHeight(rect.height());    
            QRect bgRect(rect.x() + (rect.width() - s.width())/2,
                    rect.y() + (rect.height() -s.height())/2,
                    s.width(), s.height());
//            int height = bgRect.height() - painter->fontMetrics().height() - 3*margin;
//            int adj = (bgRect.width()-width)/2;
            QRect myRect = bgRect.adjusted(margin, margin, -margin, -painter->fontMetrics().height()-margin);
//            QSize actualSize = icon.actualSize (myRect.size(),mode,state);
            if (myRect.width() > myRect.height()+1) {
                int adj = (myRect.width() - myRect.height()) >> 1;
                myRect.adjust(adj,0,-adj,0);
            }

            icon.paint(painter, myRect, Qt::AlignVCenter|Qt::AlignHCenter, mode, state);
        } else {
            QRect bgRect(rect.x() + (rect.width() - bg.width())/2,
                    rect.y() + (rect.height() -bg.height())/2,
                    bg.width(), bg.height());
            int height = bgRect.height() - painter->fontMetrics().height() - 3*margin;
            int adj = (bgRect.width()-height)/2;
            QRect myRect = bgRect.adjusted(adj, margin, -adj, -painter->fontMetrics().height()-margin);
            icon.paint(painter, myRect, option.decorationAlignment, mode, state);
        }

    } else {
        QtopiaItemDelegate::drawDecoration(painter, option, rect, decoration);
    }
}

void HomeBrowserDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    if (!bg.isNull()) {
        QRect r(option.rect);
        painter->drawPixmap(r.x() + (r.width() - bg.width())/2,
                r.y() + (r.height() -bg.height())/2, bg);
    }
}


//===========================================================================

/*!
    \class DeskphoneBrowserScreen
    \inpublicgroup QtUiModule
    \brief The DeskphoneBrowserScreen class provides an application
    launcher view suitable for a desk phone.
    \ingroup QtopiaServer::PhoneUI

    An image of this browser screen can be found in the \l {Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractBrowserScreen

*/

DeskphoneBrowserStack::DeskphoneBrowserStack(QWidget *parent)
    : LazyContentStack(NoFlags, parent), m_stack(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);
}

QObject* DeskphoneBrowserStack::currentViewObject()
{
    return m_stack->currentWidget();
}

QObject* DeskphoneBrowserStack::createView(const QString &category)
{
    LauncherView *alv = 0;
    QWidget *view = 0;
    if (category == "Main") {
        alv = LauncherView::createLauncherView("ApplicationLauncherView", this);
        alv->setColumns(3);
        alv->setViewMode(QListView::IconMode);
        alv->setFlow(QListView::TopToBottom);
        QContentFilter filter = QContentFilter(QContent::Application)
            & ~QContentFilter(QContentFilter::Category, "DeskphoneLauncher")
            & ~QContentFilter(QContentFilter::Category, "Settings");
        QContentSet set(filter);
        QContentList list = set.items();
        for(int ii = 0; ii < list.count(); ++ii)
            alv->addItem(new QContent(list.at(ii)));

        const QString SettingsFolder("Settings");
        QCategoryManager catman("Applications");
        if (catman.contains(SettingsFolder)) {
            QContent *settings = new QContent();
            settings->setName(catman.label(SettingsFolder));
            settings->setIcon(catman.iconFile(SettingsFolder));
            settings->setType(QLatin1String("Folder/")+SettingsFolder);
            alv->addItem(settings);
        }
        alv->setItemDelegate(new HomeBrowserDelegate);

        QAbstractScrollArea *sa = alv->findChild<QAbstractScrollArea*>();
        if (sa) {
            QPalette pal(sa->viewport()->palette());
#ifdef QTOPIA_HOMEUI_WIDE
            pal.setBrush(QPalette::Base, QColor(0x231f20));
            pal.setColor(QPalette::Text, Qt::white);    
            sa->viewport()->setPalette(pal);
#else
           QLinearGradient grad(0, 0, 0, QApplication::desktop()->availableGeometry().height());
            grad.setColorAt(0.0, Qt::black);
            grad.setColorAt(1.0, Qt::gray);
        pal.setBrush(QPalette::Base, grad);
#endif
        }
        view = alv;
    } else if("All" == category) {
        alv = LauncherView::createLauncherView("ApplicationLauncherView", this);
        if ( !alv ) {
            qLog(Component) << "DeskPhoneBrowserStack: ApplicationLauncherView not available";
            return 0;
        }
        QContentFilter filter = QContentFilter(QContent::Application)
            & ~QContentFilter(QContentFilter::Category, "DeskphoneLauncher");
        QContentSet set(filter);
        QContentList list = set.items();
        for(int ii = 0; ii < list.count(); ++ii)
            alv->addItem(new QContent(list.at(ii)));
        view = alv;
    } else if("Folder/Documents" == category) {
        QSettings cfg( "Trolltech","UIFactory"); // No tr
        QByteArray launcherName = cfg.value("Mappings/DocumentLauncherView", "DocumentLauncherView" ).toByteArray();
        alv = LauncherView::createLauncherView( launcherName, this, 0 );
        if ( !alv ) {
            qLog(Component) << "DeskphoneBrowserStack:"<< launcherName << "not available";
            return 0;
        }
        alv->setObjectName("Documents");
        alv->setViewMode(QListView::ListMode);
        view = alv;
    } else {
        alv = LauncherView::createLauncherView("ApplicationLauncherView");
        if (!alv) {
            qLog(Component) << "DeskPhoneBrowserStack: ApplicationLauncherView not available";
            return 0;
        }

        QString cat(category);
        if (cat.startsWith("Folder/"))
            cat = cat.mid(7);
        QContentFilter filters = (QContentFilter( QContent::Application ) | QContentFilter( QContent::Folder ))
                    & QContentFilter( QContentFilter::Category, cat );
        alv->showCategory( filters );

        QString label;
        QCategoryManager catman("Applications");
        label = catman.label(cat);

        CaptionedLauncherView *clv = new CaptionedLauncherView(alv);
        clv->setCaption(label);
        connect(clv, SIGNAL(back()), this, SLOT(back()));
        view = clv;
    }
    m_views.insert(category, view);
    m_stack->addWidget(view);
    return view;
}

void DeskphoneBrowserStack::busy(const QContent &content)
{
    emit launched(content.fileName());
}

void DeskphoneBrowserStack::raiseView(const QString &category, bool)
{
    Q_ASSERT(m_views.contains(category));
    QWidget * wid = m_views[category];
    m_stack->setCurrentWidget(wid);
    m_currentView = category;
}

void DeskphoneBrowserStack::resetToView(const QString &view)
{
    if("Main" == view) {
        LazyContentStack::resetToView("Main");
    } else {
        LazyContentStack::resetToView(view);
    }
}

void DeskphoneBrowserStack::showView(const QString &view)
{
    if("Main" == view)
        LazyContentStack::showView("Main");
    else
        LazyContentStack::showView(view);
}

QString DeskphoneBrowserStack::currentName() const
{
    return m_currentView;
}

//===========================================================================
/*!
    Constructs a new DeskphoneBrowserScreen instance with the specified \a parent
    and widget \a flags.
*/
DeskphoneBrowserScreen::DeskphoneBrowserScreen(QWidget *parent, Qt::WFlags flags)
    : QAbstractBrowserScreen(parent, flags)
{
    QAbstractHomeScreen *hs = qtopiaWidget<QAbstractHomeScreen>();
    if ( hs ) {
        hs->installEventFilter( this );
        QSoftMenuBar::setLabel(hs, Qt::Key_Back, QString(), "Launcher");
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_stack = new DeskphoneBrowserStack(this);
    QObject::connect(m_stack, SIGNAL(launched(QString)),
            this, SIGNAL(applicationLaunched(QString)));
    layout->addWidget(m_stack);
    setFocusPolicy(Qt::NoFocus);

    // Show the launcher when we're visible.
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), "Launcher");
}

/*!
    \reimp
*/
QString DeskphoneBrowserScreen::currentView() const
{
    return m_stack->currentName();
}

/*!
    \reimp
*/
bool DeskphoneBrowserScreen::viewAvailable(const QString &) const
{
    return true;
}

/*!
    \reimp
*/
void DeskphoneBrowserScreen::resetToView(const QString &v)
{
    m_stack->resetToView(v);
}

/*!
    \reimp
*/
void DeskphoneBrowserScreen::moveToView(const QString &v)
{
    m_stack->showView(v);
}

//XXX Hardcoded homescreen actions.
bool DeskphoneBrowserScreen::homescreenKeyPress(QKeyEvent *e)
{
    if (Qtopia::mousePreferred())
        return false;

    bool ret = false;

    switch (e->key()) {
        case Qt::Key_Context2: {
            QtopiaIpcEnvelope e("QPE/Application/addressbook", "raise()");
            ret = true;
            break; }
        case Qt::Key_Context1: {
            QtopiaServiceRequest e( "CallHistory", "showCallHistory(QCallList::ListType,QString)");
            e << 0 /*QCallList::All*/;
            e << QString();
            e.send();
            ret = true;
            break; }
        case Qt::Key_Select:
            // Messages
            ret = true;
            break;
        case Qt::Key_Back:
            ret = true;
            break;
        default:
            break;
    }

    return ret;
}

/*!
    \reimp
*/
bool DeskphoneBrowserScreen::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
        return homescreenKeyPress((QKeyEvent*)e);

    return QAbstractBrowserScreen::eventFilter(o, e);
}

/*!
    \reimp
*/
void DeskphoneBrowserScreen::keyPressEvent(QKeyEvent *ke)
{
    if(ke->key() == Qt::Key_Back && m_stack->currentName() != "All")
    {
        m_stack->back();
        ke->accept();
    }
    else
        ke->ignore();
}

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, DeskphoneBrowserScreen);

#include "deskbrowser.moc"
