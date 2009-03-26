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

#include "webviewer.h"
#include "navigationbar.h"
#include "softnavigationbar.h"
#include "bindings.h"
#include "cookiejar.h"
#include <QMailMessage>
#include <QMimeType>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QtopiaAbstractService>
#include <QtopiaServiceDescription>
#include <QtopiaApplication>
#include <QSpeedDial>
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebHistory>
#include <QtWebKit/QWebPluginFactory>
#include <QSoftMenuBar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAction>
#include <QBoxLayout>
#include <QMenu>
#include <QWaitWidget>
#include <QContent>
#include <QDocumentSelector>
#include <QProgressBar>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QPhoneStyle>
#include <QPainter>
#include <QKeyEvent>

// XXX until there's a real plugin system
#include "plugins/s60wrt/systeminfo.h"

class WebAccessService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    WebAccessService(QObject* parent) : QtopiaAbstractService("WebAccess",parent)
    {
        publishAll();
    }

signals:
    void openUrl(const QUrl&, bool);

public slots:
    void openURL(QString);
    void openSecureURL(QString);
};

void WebAccessService::openURL(QString url)
{
    emit openUrl(QUrl(url), false);
}

void WebAccessService::openSecureURL(QString url)
{
    // XXX make sure this is a secure url
    emit openUrl(QUrl(url), true);
}

class ProgressWidget : public QProgressBar {
    Q_OBJECT
public:
    ProgressWidget(QWidget *parent) : QProgressBar(parent)
    {
        setMinimum(0);
        setMaximum(100);
    }

public slots:
    void start()
    {
        setValue(0);
        show();
    }
    void end()
    {
        QTimer::singleShot(500, this, SLOT(hide()));
    }
};

class QWebStyle : public QPhoneStyle
{
    Q_OBJECT
public:
    QWebStyle() : QPhoneStyle() {};
    ~QWebStyle() {};

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt,
                       QPainter *p, const QWidget *widget) const
    {
        switch (pe) {
        case PE_FrameLineEdit: {
            QPen oldPen = p->pen();
            QPen pen;
            QColor c = opt->palette.color(QPalette::Text);
            c.setAlpha(100);
            pen.setColor(c);
            p->setPen(pen);

            p->setBrush(Qt::NoBrush);
            QRectF r(opt->rect);
            r.adjust(.5,.5,-.5,-.5);
            p->drawRect(r);
            //p->drawRoundedRect(r, 4, 4);
            p->setPen(oldPen);
            break; }
        case PE_PanelLineEdit:
            if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                QBrush bg = panel->palette.brush(QPalette::Base);
                p->fillRect(panel->rect.adjusted(panel->lineWidth, panel->lineWidth, -panel->lineWidth, -panel->lineWidth), bg);
            }
            break;
        default:
            QPhoneStyle::drawPrimitive(pe, opt, p, widget);
        }
    }

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                            QPainter *p, const QWidget *widget) const
    {
        switch (cc) {
        case CC_ComboBox:
            if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (cmb->frame && cmb->state & State_HasFocus && !Qtopia::mousePreferred()) {
                    QStyleOptionFocusRect fropt;
                    fropt.state = State_KeyboardFocusChange;
                    fropt.rect = opt->rect;
                    fropt.palette = opt->palette;
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                } else {
                    QPen oldPen = p->pen();
                    QPen pen;
                    QColor c = opt->palette.color(QPalette::Text);
                    c.setAlpha(48);
                    pen.setColor(c);
                    p->setPen(pen);
                    QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxFrame, widget);
                    p->setRenderHint(QPainter::Antialiasing, false);
                    p->drawLine(re.x(), re.bottom(), re.right(), re.bottom());
                    p->setRenderHint(QPainter::Antialiasing);
                    p->setPen(oldPen);
                }

                if (cmb->subControls & SC_ComboBoxArrow) {
                    State flags = State_None;

                    QRect ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                    /*if (cmb->activeSubControls == SC_ComboBoxArrow) {
                        p->setPen(cmb->palette.dark().color());
                        p->setBrush(cmb->palette.brush(QPalette::Button));
                        p->drawRect(ar.adjusted(0,0,-1,-1));

                        flags |= State_Sunken;
                    }*/   //never draw 'pressed' state for arrow

                    if (opt->state & State_Enabled)
                        flags |= State_Enabled;

                    ar.adjust(2, 2, -2, -2);
                    QStyleOption arrowOpt(0);
                    arrowOpt.rect = ar;
                    arrowOpt.palette = cmb->palette;
                    arrowOpt.state = flags;
                    drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);
                }
            }
            break;
        default:
            QPhoneStyle::drawComplexControl(cc, opt, p, widget);
        }
    }
};

class URLDialog : public QDialog
{
    Q_OBJECT
public:
    URLDialog( QWidget * parent = 0, Qt::WindowFlags flags = 0) : QDialog(parent, flags)
    {
        QPalette pal = palette();
        pal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,200)));
        setPalette(pal);

        setWindowTitle("Enter URL");
        setWindowState(Qt::WindowMaximized);

        QVBoxLayout *vbl = new QVBoxLayout(this);
        urlEdit = new QLineEdit("http://");
        vbl->addWidget(urlEdit);
        vbl->addStretch(1);

        connect(urlEdit, SIGNAL(returnPressed()), this, SLOT(accept()));

        QSoftMenuBar::setLabel(urlEdit, Qt::Key_Back, "go", tr("Go"), QSoftMenuBar::NavigationFocus);  //TODO: image
    }

    void setText(QString str)
    {
        urlEdit->setText(str);
    }

    QString text()
    {
        return urlEdit->text();
    }

private:
    QLineEdit *urlEdit;
};

class QtopiaWebPage : public QWebPage {
    Q_OBJECT
public:
    QtopiaWebPage(QObject* parent) : QWebPage(parent) {}
    QObject *createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
    QWebPage *createModalDialog();
    QWebPage *createWindow(QWebPage::WebWindowType);
    QString chooseFile(QWebFrame *parentFrame, const QString& oldFile);
    QString userAgentFor(const QUrl& url) const;

    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, QWebPage::NavigationType type);
};

QObject* QtopiaWebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    // XXX embed a widget here
    return QWebPage::createPlugin(classid,url,paramNames,paramValues);
}

QWebPage *QtopiaWebPage::createWindow(WebWindowType type)
{
    // XXX how to present multiple windows?
    // XXX QtLauncher would do "new WebView", but need to integrate them

    if (type == WebBrowserWindow)
        return this;
    else
        return QWebPage::createWindow(type);
}

QString QtopiaWebPage::chooseFile(QWebFrame * /*parentFrame*/, const QString& oldFile)
{
    QContent content(oldFile);
    QContentFilter filter(QContent::Document);
    QDocumentSelector::Selection sel
        = QDocumentSelector::select(view(),&content,QDrmRights::Permission(),tr("Choose File"), filter);
    if (sel==QDocumentSelector::DocumentSelected)
        return content.fileName();
    else
        return QString::null;
}

QString QtopiaWebPage::userAgentFor(const QUrl& url) const
{
    Q_UNUSED(url)
    return "Mozilla/5.0 (compatible; U; Linux; "+Qtopia::languageList().first()+") "
            "AppleWebKit/418.9.1 (KHTML, like Gecko) "
            "QtopiaWebViewer/4.4.0 (like iPhone) "
            "Safari/419.3 "
            "Qt";
}

static QMailMessage convertQUrlToQMailMessageRfc2368(const QUrl& url)
{
    QMailMessage r;
    r.setMessageType(QMailMessage::Email);
    QString to = url.toString(QUrl::RemoveQuery|QUrl::RemoveScheme);
    r.setTo(QMailAddress(to));
    QList<QPair<QString, QString> > values = url.queryItems();
    for (int i=0; i<values.count(); ++i) {
        QPair<QString,QString> kv=values[i];
        QString key = kv.first.toLower();
        if ( key == "to" ) {
            r.setTo(r.to() << QMailAddress(kv.second));
        } else if ( key == "subject" || key == "cc" || key == "in-reply-to" ) {
            r.setHeaderField(kv.first,kv.second);
        } else if ( key == "body" ) {
            // XXX content-type not passed through for webkit formdata anyway,
            // XXX so assume text/plain.
            // XXX need to compare with other browsers
            r.setBody(QMailMessageBody::fromData(kv.second,QMailMessageContentType("text/plain; charset=UTF-8"),QMailMessageBody::QuotedPrintable));
        }
        // could setHeaderField all fields, but the above
        // are considered "safe" by RFC2368.
    }
    return r;
}

static QString convertQUrlToPhoneNumberRfc2806(const QUrl& url)
{
    QString r;
    QString s = url.toString(QUrl::RemoveQuery|QUrl::RemoveScheme);
    int semi = s.indexOf(';');
    r = s.left(semi);
    if ( semi >= 0 ) {
        QRegExp postd(";postd=([0-9*#ABCDpw]*)");
        if (postd.indexIn(s)) {
            r += "w";
            r += postd.cap();
        }
        s.remove(postd);
        if ( s != r ) {
            // RFC says MUST NOT use the URL
            return QString("ERROR");
        }
    }
    r.remove(QRegExp("[-.()]")); // XXX or pass to dialer to look nicer?
    return r;
}

bool QtopiaWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, QWebPage::NavigationType type)
{
    if ( request.url().scheme() == "mailto" ) {
        QMailMessage mail = convertQUrlToQMailMessageRfc2368(request.url());
        QtopiaServiceRequest req("Messages", "composeMessage(QMailMessage)");
        req << mail;
        req.send();
        return false;
    } else if ( request.url().scheme() == "tel" ) {
        QString name; // XXX not defined
        QString number = convertQUrlToPhoneNumberRfc2806(request.url());
        if (number.length()>=11 && number[0]!='+')
            number = "+"+number;
        // XXX Dialer should confirm (with Key::Call) if this URL not known to be user activated
        QtopiaServiceRequest req("Dialer", "dial(QString,QString)");
        req << name << number;
        req.send();
        return false;
    } else {
        return QWebPage::acceptNavigationRequest(frame,request,type);
    }
}


class QtopiaPluginFactory : public QWebPluginFactory {
    Q_OBJECT
public:
    QtopiaPluginFactory(QWebPage *owner);
    QObject* create(const QString& mimetype, const QUrl& url, const QStringList& argumentNames, const QStringList& argumentValues) const;
    QList<QWebPluginFactory::Plugin> plugins() const;
    void refreshPlugins();

private:
    QWebPage *mPage;
    QList<QWebPluginFactory::Plugin> mPlugins;
};

QtopiaPluginFactory::QtopiaPluginFactory(QWebPage *page)
    : QWebPluginFactory(), mPage(page)
{

}

void QtopiaPluginFactory::refreshPlugins()
{
    // XXX This should actually enumerate stuff
    mPlugins.clear();
    Plugin p;
    MimeType m;
    m.description = "S60 System Info Widget";
    m.name = "application/x-systeminfo-widget";
    p.description = "S60 WRT";
    p.name = "s60wrt";
    p.mimeTypes << m;
    mPlugins << p;
}

QList<QtopiaPluginFactory::Plugin> QtopiaPluginFactory::plugins() const
{
    return mPlugins;
}

QObject* QtopiaPluginFactory::create(const QString& mimetype, const QUrl& url, const QStringList& args, const QStringList& values) const
{
    Q_UNUSED(url);
    Q_UNUSED(args);
    Q_UNUSED(values);
    QObject *ret = 0;
    // XXX again, should be dynamic
    if (mimetype == "application/x-systeminfo-widget") {
        ret = new S60SystemInfo(mPage);
    }

    return ret;
}

class QtopiaWebView : public QWebView {
    Q_OBJECT
public:
    QtopiaWebView(QWidget* parent) :
        QWebView(parent)
    {
        if (!Qtopia::mousePreferred()) {
            QImage vcursorimg(":image/vcursor");
            vcursorpm = QPixmap::fromImage(vcursorimg);
            vcursorhotspot = vcursorimg.offset();
            vcursorimg = vcursorimg.convertToFormat(QImage::Format_ARGB32);
            QRgb *rgb = (QRgb*)vcursorimg.bits();
            for (int i=vcursorimg.numBytes()/4; i-->0; )
                *rgb++ &= 0x1fffffff;
            vcursorpm_faded = QPixmap::fromImage(vcursorimg);
            vcursorfade.start(1000, this);
        }

        QPalette pal = palette();
        pal.setBrush(QPalette::Background, Qt::white);
        pal.setBrush(QPalette::Base, QColor(255,255,255,64));
        pal.setBrush(QPalette::Foreground, Qt::black);
        pal.setBrush(QPalette::Text, Qt::black);
        pal.setBrush(QPalette::Button, QColor(220,220,220));
        setPalette(pal);

        setAttribute(Qt::WA_OpaquePaintEvent, false);

        prog = new ProgressWidget(this);

        connect(this, SIGNAL(loadStarted()),
                prog, SLOT(start()));
        connect(this, SIGNAL(loadStarted()),
                this, SLOT(updateSoftMenuBar()));
        connect(this, SIGNAL(loadProgress(int)),
                prog, SLOT(setValue(int)));
        connect(this, SIGNAL(loadProgress(int)),
                this, SLOT(updateSoftMenuBar()));
        connect(this, SIGNAL(loadFinished(bool)),
                prog, SLOT(end()));
        target = 0;
        filterPress = false;
        pressed = false;
        moveThreshold = QApplication::desktop()->screenGeometry().width()/40;

        QtopiaApplication::setInputMethodHint(this,QtopiaApplication::Text);
    }

    void setProgress(int p) { prog->setValue(p); if (p==100) prog->end(); else prog->show(); } // for external progress

protected:
    void paintEvent(QPaintEvent* e);

    void resizeEvent(QResizeEvent* e)
    {
        if (vcursorpos.isNull() || !rect().contains(vcursorpos))
            vcursorpos = QPoint(width()/2,height()/2);
        QWebView::resizeEvent(e);
        QSize sh = prog->sizeHint();
        const int marg=20;
        prog->setGeometry(marg,height()-marg-sh.height(),width()/3,sh.height());
    }

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
    void inputMethodEvent(QInputMethodEvent *);

private slots:
    void updateSoftMenuBar();

private:
    ProgressWidget *prog;

    QPoint vcursorpos;
    QPoint vcursorhotspot;
    QPixmap vcursorpm;
    QPixmap vcursorpm_faded;
    QBasicTimer vcursorfade;

    QPoint mousePos;
    Qt::MouseButtons buttons;
    QWidget *target;
    bool filterPress;
    bool pressed;
    int moveThreshold;
    QBasicTimer ptimer;
    void timerEvent(QTimerEvent*);
};

static const int vcursorjump=5;
static const int vcursorjumprepeat=20;
static const int vcursoredgemin=3;


void QtopiaWebView::paintEvent(QPaintEvent* e)
{
    QWebView::paintEvent(e);
    if (!Qtopia::mousePreferred()) {
        QPainter p(this);
        p.drawPixmap(vcursorpos-vcursorhotspot,vcursorfade.isActive() ? vcursorpm : vcursorpm_faded);
    }
}

void QtopiaWebView::updateSoftMenuBar()
{
    if (testAttribute(Qt::WA_InputMethodEnabled)) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    } else if (page()->history()->canGoBack()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
        //QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Previous);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "close", tr("Close"));
        //QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
    }
}

void QtopiaWebView::inputMethodEvent(QInputMethodEvent *e)
{
    updateSoftMenuBar();
    QWebView::inputMethodEvent(e);
}

void QtopiaWebView::keyPressEvent(QKeyEvent* e)
{
    if (!Qtopia::mousePreferred()) {
        QPoint movecursor;
        switch (e->key()) {
        case Qt::Key_Select:
            {
                QApplication::postEvent(this,new QMouseEvent(QEvent::MouseButtonPress,vcursorpos,Qt::LeftButton,Qt::LeftButton,0));
                QApplication::postEvent(this,new QMouseEvent(QEvent::MouseButtonRelease,vcursorpos,Qt::LeftButton,0,0));
                return;
            }
            break;
        case Qt::Key_Left:
            movecursor.setX(-1);
            break;
        case Qt::Key_Right:
            movecursor.setX(+1);
            break;
        case Qt::Key_Up:
            movecursor.setY(-1);
            break;
        case Qt::Key_Down:
            movecursor.setY(+1);
            break;
        }
        if (!movecursor.isNull()) {
            vcursorfade.start(1000,this);
            if (!rect().contains(vcursorpos+movecursor*vcursoredgemin)) {
                // at edge - just scroll
                QWebView::keyPressEvent(e);
            } else {
                // "sticky" cursor movement:
                //  - jump by "vcursorjump" normally
                //  - if move onto link, look vcursorjump ahead and try to land in middle of
                //      link (at least as far as visible that far)
                QWebFrame *frame = page()->mainFrame();
                QRect rfrom(vcursorpos,vcursorpm.size());
                bool havenewhit=false;
                QWebHitTestResult fromhit = frame->hitTestContent(vcursorpos);
                QWebHitTestResult newhit;
                QWebHitTestResult firstnewhit;
                QPoint newhitpos;
                QPoint firstnewhitpos;
                int jumpsize = e->isAutoRepeat() ? vcursorjumprepeat : vcursorjump;
                for (int i=0; i<jumpsize; ++i) {
                    if (!rect().contains(vcursorpos+movecursor*vcursoredgemin))
                        break; // close to edge - stop
                    vcursorpos += movecursor;
                    QWebHitTestResult tohit = frame->hitTestContent(vcursorpos);
                    if (!havenewhit &&
                            (tohit.frame() != fromhit.frame() ||
                            tohit.linkUrl() != fromhit.linkUrl() ||
                            tohit.imageUrl() != fromhit.imageUrl()))
                    {
                        // first different - store
                        havenewhit=true;
                        newhit = tohit;
                        newhitpos = vcursorpos;
                        firstnewhitpos = vcursorpos;
                        if (!newhit.linkUrl().isEmpty() || !newhit.imageUrl().isEmpty())
                            i -= vcursorjumprepeat; // look further (so we can center on new hit better)
                    } else if (havenewhit) {
                        if (newhit.frame() == tohit.frame() &&
                        newhit.linkUrl() == tohit.linkUrl() &&
                        newhit.imageUrl() == tohit.imageUrl())
                        {
                            // same different - update
                            newhit = tohit;
                            newhitpos = vcursorpos;
                        } else {
                            break; // changed again - stop
                        }
                    }
                }
                if (havenewhit)
                    vcursorpos = (newhitpos+firstnewhitpos)/2;
                QApplication::postEvent(this,new QMouseEvent(QEvent::MouseMove,vcursorpos,Qt::NoButton,Qt::MouseButtons(),0));
                QRect r(vcursorpos,vcursorpm.size());
                r.translate(-vcursorhotspot);
                rfrom.translate(-vcursorhotspot);
                r |= rfrom;
                update(r);
            }
            return;
        }
    }
    updateSoftMenuBar();
    if (testAttribute(Qt::WA_InputMethodEnabled)) {
        if (e->key() == Qt::Key_Back) {
            QKeyEvent kp(QEvent::KeyPress,Qt::Key_Backspace,e->modifiers());
            QWebView::keyPressEvent(&kp);
            return;
        }
    }
    QWebView::keyPressEvent(e);
}

void QtopiaWebView::keyReleaseEvent(QKeyEvent* e)
{
    if (testAttribute(Qt::WA_InputMethodEnabled)) {
        QKeyEvent kr(QEvent::KeyRelease,Qt::Key_Backspace,e->modifiers());
        QWebView::keyPressEvent(&kr);
    } else {
        QWebView::keyReleaseEvent(e);
    }
}

void QtopiaWebView::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == ptimer.timerId()) {
        ptimer.stop();
        QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonPress, target->mapFromGlobal(mousePos), mousePos, Qt::LeftButton, buttons, QApplication::keyboardModifiers()));
        if (!pressed)
            QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonRelease, target->mapFromGlobal(mousePos), mousePos, Qt::LeftButton, buttons, QApplication::keyboardModifiers()));
    } else if (ev->timerId() == vcursorfade.timerId()) {
        QRect r(vcursorpos,vcursorpm.size());
        r.translate(-vcursorhotspot);
        vcursorfade.stop();
        update(r);
    }
}

void QtopiaWebView::mousePressEvent(QMouseEvent* ev)
{
    if (!ev->spontaneous()) {
        QWebView::mousePressEvent(ev);
        updateSoftMenuBar();
        return;
    }
    if (ev->button() == Qt::LeftButton) {
        target = this;
        mousePos = ev->globalPos();
        buttons = ev->buttons();
        filterPress = false;
        ptimer.start(250, this);
        pressed = true;
        return;
    } else {
        target = 0;
        filterPress = false;
    }

    QWebView::mousePressEvent(ev);
    updateSoftMenuBar();
}

void QtopiaWebView::mouseMoveEvent(QMouseEvent* ev)
{
    if (!ev->spontaneous()) {
        //QWebView::mouseMoveEvent(ev);
        return;
    }
    if (pressed) {
        QPoint diff = mousePos - ev->globalPos();
        if (!filterPress
            && (qAbs(diff.y()) > moveThreshold
            || qAbs(diff.x()) > moveThreshold)) {
            filterPress = true;
            if (!ptimer.isActive()) {
                // TODO: unpress buttons, etc
                // QThumbStyle uses bogus move to get e.g. QPushButtons unpressed.
                // but can cause selection to happen in webkit
                /*QPoint bogusPos(-1,-1);
                QApplication::postEvent(target,
                        new QMouseEvent(QEvent::MouseMove,
                                        target->mapFromGlobal(bogusPos),
                                        bogusPos, Qt::LeftButton, buttons,
                                        QApplication::keyboardModifiers()));*/
            }
            ptimer.stop();
            diff = QPoint(0,0); // avoid jump
        }
        if (filterPress) {
            int maxh = page()->mainFrame()->scrollBarMaximum(Qt::Horizontal);
            int maxv = page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
            if (diff.y() && maxv > 0) {
                int moveY = diff.y();
                page()->mainFrame()->setScrollBarValue(Qt::Vertical, page()->mainFrame()->scrollBarValue(Qt::Vertical) + moveY);
            }

            if (diff.x() && maxh > 0) {
                int moveX = diff.x();
                page()->mainFrame()->setScrollBarValue(Qt::Horizontal, page()->mainFrame()->scrollBarValue(Qt::Horizontal) + moveX);
            }

            mousePos = ev->globalPos();
            return;
        }
        if (ptimer.isActive())
            return;
    }

    QWebView::mouseMoveEvent(ev);
}

void QtopiaWebView::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!ev->spontaneous()) {
        QWebView::mouseReleaseEvent(ev);
        updateSoftMenuBar();
        return;
    }
    if (ev->button() == Qt::LeftButton) {
        pressed = false;
        if (target) {
            //scrollArea = 0;
            if (filterPress) {
                // Don't send any release
                target = 0;
                ptimer.stop();
                ev->accept();
                filterPress = false;
                return;
            } /*else {
                QWidget *fw = target;
                while (fw) {
                    if (fw->isEnabled() && fw->focusPolicy() != Qt::NoFocus) {
                        fw->setFocus(Qt::MouseFocusReason);
                        break;
                    }
                    if (fw->isWindow())
                        break;
                    fw = fw->parentWidget();
                }
            }*/
            if (ptimer.isActive()) {
                ptimer.start(0, this);
                return;
            }
        }
    }

    QWebView::mouseReleaseEvent(ev);
    updateSoftMenuBar();
}

/*
 *  Constructs a WebViewer which is a child of 'parent', with the
 *  name 'name'.
 */
WebViewer::WebViewer(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f),
        downloading(false)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    if (Qtopia::hasKey(Qt::Key_Tab) && !Qtopia::mousePreferred()) {
        // Handle tab-based link selection for non-touchscreen devices.
        QWebSettings::globalSettings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    }
    QString icondir = Qtopia::applicationFileName("WebAccess","iconDatabase");
    QDir().mkdir(icondir);
    QWebSettings::setIconDatabasePath(icondir);

    QWidget *box = new QWidget(this);
    QBoxLayout *boxLayout = new QVBoxLayout(box);

    view = new QtopiaWebView(box);
    unsecurePage = new QtopiaWebPage(this);
    view->setPage(unsecurePage);

    unsecurePage->setForwardUnsupportedContent(true);
    connect(unsecurePage,SIGNAL(unsupportedContent(QNetworkReply*)),
            this,SLOT(handleUnsupportedContent(QNetworkReply*)));

    view->setStyle(new QWebStyle);
    securePage = 0;
    bindings = 0;

    if (!qobject_cast<CookieJar*>(unsecurePage->networkAccessManager()->cookieJar()))
        unsecurePage->networkAccessManager()->setCookieJar(new CookieJar(this));

    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);
    boxLayout->addWidget(view);

    if (Qtopia::hasKey(Qt::Key_Context1) && Qtopia::hasKey(Qt::Key_Context2)) {
        softNavigationBar = new SoftNavigationBar(this);
        softNavigationBar->setBack(view->pageAction(QWebPage::Back));
        softNavigationBar->setForward(view->pageAction(QWebPage::Forward));

#ifdef NAVIGATION_BAR
        navigationBar = 0;
#endif
    } else {
        softNavigationBar = 0;
#ifdef NAVIGATION_BAR
        navigationBar = new NavigationBar(box);
        navigationBar->setBack(view->pageAction(QWebPage::Back));
        navigationBar->setForward(view->pageAction(QWebPage::Forward));
        navigationBar->hide(); // initial state

        boxLayout->addWidget(navigationBar);
#endif
    }

    setCentralWidget(box);

    QMenu *menu = QSoftMenuBar::menuFor(view);

    menu->addAction(tr("Go to..."), this, SLOT(goTo()));

    QAction *actionBack = view->pageAction(QWebPage::Back);
    menu->addAction(actionBack);

    QAction *actionStop = view->pageAction(QWebPage::Stop);
    menu->addAction(actionStop);

    QAction *actionForward = view->pageAction(QWebPage::Forward);
    menu->addAction(actionForward);

    QAction *actionClose = new QAction(QIcon("close"),tr("Close"),this);
    connect(actionClose,SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(actionClose);

    menu->addSeparator();

    actionSpeedDial = new QAction(QIcon(":icon/favorite"),tr("Add to Favorites..."),this);
    connect(actionSpeedDial,SIGNAL(triggered()), this, SLOT(addToSpeedDial()));
    menu->addAction(actionSpeedDial);

    connect(view, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    connect(view, SIGNAL(titleChanged(QString)),
            this, SLOT(setWindowTitle(QString)));

    QObject *service = new WebAccessService(this);
    connect(service,SIGNAL(openUrl(QUrl,bool)),this,SLOT(load(QUrl,bool)));

    QSettings conf("Trolltech", "WebAccess");
    conf.beginGroup("Cache");
    QWebSettings::setMaximumPagesInCache( conf.value("MaximumPages", 5).toInt() );
    int maxdead = conf.value("MaximumDeadBytes",-1).toInt();
    int maxsize = conf.value("MaximumBytes",-1).toInt();
    if ( maxdead < 0 ) maxdead = maxsize;
    if ( maxdead < 0 ) maxdead = 500000;
    if ( maxsize < 0 ) maxsize = maxdead;
    QWebSettings::globalSettings()->setObjectCacheCapacities(
        conf.value("MinimumDeadBytes",0).toInt(), maxdead, maxsize);
    conf.endGroup();

    QUrl home(standardPage("Home"));

    if ( home.isValid() )
        view->load(home);
}

QUrl WebViewer::standardPage(const QString& id) const
{
    QSettings conf("Trolltech", "WebAccess");
    conf.beginGroup("Locations");
    QString urlstr = conf.value(id).toString();
    urlstr.replace("$QPEDIR",Qtopia::qtopiaDir());
    return QUrl(urlstr);
}

/*
 *  Destroys the object and frees any allocated resources
 */
WebViewer::~WebViewer()
{
}

void WebViewer::handleUnsupportedContent(QNetworkReply* reply)
{
    if (downloading) {
        // XXX
        qWarning("Can only download one at a time");
        return;
    }

    if (reply->error()) {
        reply->abort();
        QUrl error = standardPage("Error");
        if (error != reply->url() /* i.e. infinite recursion if no error page */) {
            error.addQueryItem("code",QString::number(int(reply->error())));
            error.addQueryItem("url",reply->url().toString());
            error.addQueryItem("hostname",reply->url().host());
            error.addQueryItem("reason",reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
            view->load(error);
        }
        return;
    }

    QMimeType mimetype(reply->header(QNetworkRequest::ContentTypeHeader).toString());
    QContent handler = mimetype.application();

    if (handler.isNull()) {
        reply->abort();
        QUrl error = standardPage("UnsupportedContent");
        if (error != reply->url() /* i.e. infinite recursion if no error page */) {
            error.addQueryItem("url",reply->url().toString());
            view->load(error);
        }
        return;
    }

    // Can handler handle http itself?
    if (QtopiaService::apps("PlayMedia").contains(handler.executableName())) {
        if (mimetype.id().startsWith("audio")) { // XXX Helix doesn't do http video properly
            // Yes
            QtopiaIpcEnvelope env("QPE/Application/"+handler.executableName(),"PlayMedia::openURL(QString)");
            env << reply->url().toString();
            reply->abort(); // let the app download it
            return;
        }
    }

    // Load the media and pass as document
    // XXX always download to Document storage (i.e. keep all downloads)
    downloadcontent = QContent();
    QString name = reply->url().path();
    if (int slash = name.lastIndexOf('/'))
        name = name.mid(slash+1);
    if (int dot = name.lastIndexOf('.'))
        name = name.left(dot);
    if (name.isEmpty())
        name = tr("%1 Download","eg. Media Player Download").arg(handler.name());
    downloadcontent.setName(name);
    downloadcontent.setType(mimetype.id());
    downloadto.setFileName(downloadcontent.file());
    if (!downloadto.open(QIODevice::WriteOnly)) {
        // XXX error
        downloadcontent.removeFiles();
        reply->abort();
        return;
    }

    downloading = true;
    connect(reply, SIGNAL(readyRead()), this, SLOT(downloadMore()));
    connect(reply, SIGNAL(finished()), this, SLOT(finishedDownload()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(view, SIGNAL(loadStarted()), this, SLOT(abortDownload()));
}

void WebViewer::downloadMore()
{
    QIODevice* io = qobject_cast<QIODevice*>(sender());
    while (1) {
        QByteArray r = io->read(65536);
        if (r.isEmpty())
            break;
        downloadto.write(r);
    }
}

void WebViewer::finishedDownload()
{
    downloadto.close();
    downloadcontent.execute();
    downloading = false;
}

void WebViewer::downloadProgress(qint64 p, qint64 total)
{
    view->setProgress(p/(total/100));
}

void WebViewer::addToSpeedDial()
{
    QtopiaServiceRequest req = QtopiaServiceRequest("WebAccess", "openURL(QString)");
    req << view->url().toString();
    QString label = view->title();
    if (label.isEmpty()) {
        label = view->url().host();
        if (label.isEmpty()) {
            label = view->url().path();
            label.remove(QRegExp(".*/"));
        }
        if (label.isEmpty())
            label = view->url().toString();
    }
    QIcon icon = view->icon();
    QString iconData;
    if ( icon.isNull() ) {
        QUrl appleiconurl = view->url();
        appleiconurl.setPath("/apple-touch-icon.png");
        // XXX get it...
    }
    if ( !icon.isNull() ) {
        // we don't want to maintain references to files,
        // so we store the image in the name.
        // (could instead have a ":webkiticon/url" file engine)
        int iconsize = style()->pixelMetric(QStyle::PM_LargeIconSize);
        QPixmap pm = icon.pixmap(iconsize);
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        pm.save(&buf,"PNG");
        buf.close();
        iconData = ":data/" + buf.buffer().toBase64();
    } else {
        iconData = "webviewer/WebViewer";
    }
    QtopiaServiceDescription desc(req,label,iconData);
    QtopiaServiceRequest freq("Favorites","addAndEdit(QtopiaServiceDescription)");
    freq << desc;
    freq.send();
}

void WebViewer::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Back) {
        if (view->page()->history()->canGoBack()) {
            view->triggerPageAction(QWebPage::Back);
        } else {
            QMainWindow::keyPressEvent(ke);
        }

#ifdef DEBUG
    // Debugging aids for qvfb
    } else if (ke->key() == Qt::Key_R && ke->modifiers() & Qt::ControlModifier) {
        view->reload();
    } else if (ke->key() == Qt::Key_S && ke->modifiers() & Qt::ControlModifier) {
        load(view->url(), true);
    } else if (ke->key() == Qt::Key_U && ke->modifiers() & Qt::ControlModifier) {
        load(view->url(), false);
#endif
    } else if (ke->key() == Qt::Key_Select) {
        // Handle tab-based link selection for non-touchscreen devices.
        if (Qtopia::hasKey(Qt::Key_Tab) && !Qtopia::mousePreferred()) {
            QKeyEvent nke(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
            QApplication::sendEvent(view->page(), &nke);
        } else {
            QMainWindow::keyPressEvent(ke);
        }
    } else
        QMainWindow::keyPressEvent(ke);
}

void WebViewer::load(const QUrl& url, bool secure)
{
    if (secure) {
        if (!bindings)
            bindings = createBindingHierarchy(this, this);
        if (!securePage) {
            securePage = new QWebPage(this);
            securePage->setPluginFactory(new QtopiaPluginFactory(securePage));
            connect(securePage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addBindings()));
        }
        view->setPage(securePage);
    } else {
        view->setPage(unsecurePage);
    }
    view->load(url);
    QtopiaApplication::instance()->showMainWidget();
}

void WebViewer::addBindings()
{
    // Have to make sure our bindings still exist
    if (securePage && view->page() == securePage) {
        // Bindings should have been created by WebViewer::load
        securePage->mainFrame()->addToJavaScriptWindowObject(bindings->objectName(), bindings);
    }
}

void WebViewer::loadFinished()
{
#ifdef NAVIGATION_BAR
    if (navigationBar)
        navigationBar->setVisible(view->page()->history()->canGoBack() || view->page()->history()->canGoForward());
    /* looks poor
    navigationBar->labelsChanged(
        page->history()->canGoBack() ? page->history()->backItem().title() : QString(""),
        page->history()->canGoForward() ? page->history()->forwardItem().title() : QString(""));
    */
#endif
    actionSpeedDial->setVisible(!view->url().isEmpty());
}

void WebViewer::setDocument( const QString &doc )
{
    if ( !doc.isEmpty() ) {
        view->load(QUrl(doc));
        QtopiaApplication::instance()->showMainWidget();
    }
}

void WebViewer::goTo()
{
    URLDialog dlg(this);
    if (!view->url().isEmpty())
        dlg.setText(view->url().toString());
    if (dlg.exec() == QDialog::Accepted) {
        QUrl url(dlg.text());
        view->load(url);
    }
}

#include "webviewer.moc"
