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


#include "qphonestyle.h"
#include <custom.h>
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
#include "qexportedbackground.h"
#ifdef Q_WS_QWS
#include "qscreen_qws.h"
#endif
#endif
#include <QValueSpaceItem>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QStyleOption>
#include <QPainter>
#include <qdrawutil.h>
#include <QApplication>
#include <QTextEdit>
#include <QTextBrowser>
#include <QDebug>
#include <QMenu>
#include <QScrollArea>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QComboBox>
#include <QSpinBox>
#include <QMenu>
#include <QPaintEvent>
#include <QDesktopWidget>
#include <QWSManager>
#include <QMap>
#include <QDesktopWidget>
#include <QPaintEngine>
#include <private/qwidget_p.h>
#include <QLabel>
#include <QLayout>
#include <QStylePainter>
#include <QTimer>
#include <QPushButton>
#include <QToolButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QTimeLine>
#include <QPixmapCache>
#include <values.h>
#include <QGlobalPixmapCache>
#include <QSoftMenuBar>
#include <QTableView>

static const int phoneItemFrame        =  1; // menu item frame width
static const int phoneSepHeight        =  1; // separator item height
static const int phoneItemHMargin      =  2; // menu item hor text margin
static const int phoneItemVMargin      =  1; // menu item ver text margin
static const int phoneArrowHMargin     =  6; // arrow horizontal margin
static const int phoneCheckMarkHMargin =  2; // horiz. margins of check mark
static const int phoneRightBorder      =  8; // right border on windows
static int phoneCheckMarkWidth   = 12;       // checkmarks width in menus
static int scrollbarSize = 0;                // width of the scrollbar
static int comboArrowWidth = 16;             // combobox and spinbox arrow width
static int sliderHeight = 18;
static int sliderGrooveHeight = 8;

Q_GLOBAL_STATIC_WITH_ARGS(QSettings, gConfig, ("Trolltech", "qpe"));

static bool isSingleFocusWidget(QWidget *focus)
{
    bool singleFocusWidget = false;
    if (focus) {
        QWidget *w = focus;
        singleFocusWidget = true;
        while ((w = w->nextInFocusChain()) != focus) {
            if (w->isVisible() && focus != w->focusProxy() && w->focusPolicy() & Qt::TabFocus) {
                singleFocusWidget = false;
                break;
            }
        }
    }

    return singleFocusWidget;
}

static bool isSubMenu(const QWidget *widget)
{
    const QMenu *menu = qobject_cast<const QMenu*>(widget);
    if (menu) {
        QList<QWidget*> widgets = menu->menuAction()->associatedWidgets();
        for (int i=0; i<widgets.size(); i++) {
            if (qobject_cast<const QMenu*>(widget))
                return true;
        }
    }
    return false;
}


#ifdef Q_WS_QWS

class ManagerAccessorPrivate;

class ManagerAccessor : public QWidget
{
public:
    QWSManager *manager();

    Q_DECLARE_PRIVATE(ManagerAccessor);
};

class ManagerAccessorPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(ManagerAccessor);
};

QWSManager *ManagerAccessor::manager() {
    Q_D(ManagerAccessor);
    return d->topData()->qwsManager;
}

#endif

static bool useCache = true; // (qgetenv("QPHONESTYLE_USE_CACHE").toInt() > 0);

static QString uniqueName(const QString &key, const QStyleOption *option, const QRect &rect)
{
    QString tmp;
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    tmp.sprintf("%s-%d-%d-%d-%ud-%dx%d", key.toLatin1().constData(),
                uint(option->state), option->direction,
                complexOption ? uint(complexOption->activeSubControls) : uint(0),
                option->palette.color(QPalette::Highlight).rgb(), rect.width(), rect.height());
    return tmp;
}

#define BEGIN_PHONESTYLE_PIXMAPCACHE(r, key)   \
    QRect rect = r; \
    QPixmap cache; \
    QPainter *painter = p; \
    QString unique = uniqueName((key), opt, rect); \
    if (useCache) { \
        if (!QPixmapCache::find(unique, cache)) { \
            cache = QPixmap(rect.size()); \
            cache.fill(Qt::transparent); \
            p = new QPainter(&cache); \
            p->translate(-rect.topLeft()); \
        } else { \
            p->drawPixmap(rect.topLeft(), cache); \
            break; \
        } \
    }

#define END_PHONESTYLE_PIXMAPCACHE \
    if (p != painter) { \
        p->end(); \
        delete p; \
        painter->drawPixmap(rect.topLeft(), cache); \
        QPixmapCache::insert(unique, cache); \
    }

#define BEGIN_PHONESTYLE_TEXTCACHE(rect, key)   \
    QRect r = rect; \
    QPixmap cache; \
    QPainter *painter = p; \
    QString unique = uniqueName((key), r); \
    if (useCache) { \
        if (!QPixmapCache::find(unique, cache)) { \
            cache = QPixmap(r.size()); \
            cache.fill(Qt::transparent); \
            p = new QPainter(&cache); \
            p->translate(-r.topLeft()); \
        } else { \
            p->drawPixmap(r.topLeft(), cache); \
            break; \
        } \
    }

//#define DEBUG_BUDDY_FOCUS

class BuddyFocusBox : public QWidget
{
    Q_OBJECT
public:
    BuddyFocusBox(QWidget *p=0)
        : QWidget(p), state(Hidden), timeLine(180), animate(false) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_NoChildEventsForParent, true);
        timeLine.setLoopCount(1);
        timeLine.setUpdateInterval(20);
        timeLine.setStartFrame(48);
        timeLine.setEndFrame(255);
        connect(&timeLine, SIGNAL(frameChanged(int)),
                this, SLOT(update()));
        connect(&timeLine, SIGNAL(finished()),
                this, SLOT(finished()));
    }

    ~BuddyFocusBox() {}

    void setTargets(QLabel *l, QWidget *w) {
        if (label)
            label->removeEventFilter(this);
        if (widget) {
            widget->removeEventFilter(this);
            if (widget->parentWidget())
                widget->parentWidget()->removeEventFilter(this);
        }
        if (w && w->focusPolicy() == Qt::NoFocus)
            w = 0;
        label = l;
        if (!l && w && (qobject_cast<QPushButton*>(w) || qobject_cast<QToolButton*>(w)))
            w = 0;
        widget = w;
        if (label)
            label->installEventFilter(this);
        if (widget) {
            widget->installEventFilter(this);
            if (widget->parentWidget())
                widget->parentWidget()->installEventFilter(this);
#ifdef DEBUG_BUDDY_FOCUS
            qDebug() << "is visible" << widget << widget->isVisible();
#endif
        }
        calcPosition();
        moveHighlight();
    }

    bool eventFilter(QObject *obj, QEvent *e) {
#ifndef DEBUG_BUDDY_FOCUS
        Q_UNUSED(obj);
#endif
        if (e->type() == QEvent::Move || e->type() == QEvent::Resize
            || e->type() == QEvent::LayoutRequest || e->type() == QEvent::Show) {
#ifdef DEBUG_BUDDY_FOCUS
            qDebug() << "got event for" << obj << e->type();
#endif
            if (widget) {
                calcPosition();
                moveHighlight();
            }
        }
        return false;
    }

private:
    bool focusRectAllowed(QWidget *widget) {
        if (isSingleFocusWidget(widget)) {
            // Dodgy heuristic to avoid showing a focus rect for single
            // focus widgets that really don't need one.
            if (widget->geometry().width() == widget->window()->width()
                    || widget->geometry().height() == widget->window()->height()) {
                return false;
            }
            if (widget->geometry().width() >= widget->window()->width()*7/8
                    && widget->geometry().height() >= widget->window()->height()/2) {
                return false;
            }
        }

        return true;
    }

    void calcPosition() {
        if (!widget)
            return;
        QRect rect = widget->geometry();
        if (label) {
            QRect labelgeo = label->geometry();
            if (label->parentWidget() && label->parentWidget()->layout()) {
                QRect layoutgeo = label->parentWidget()->layout()->geometry();
                if (labelgeo.width() < layoutgeo.width()) {
                    labelgeo.setLeft(layoutgeo.left());
                    labelgeo.setRight(layoutgeo.right());
                }
            }
            rect = rect.united(labelgeo);
        }
        if (widget->parentWidget() && widget->parentWidget()->layout()) {
            int sp = widget->parentWidget()->layout()->spacing();
            if (sp > 0)
                rect.adjust(-sp/2, -sp/2, sp/2, sp/2);
#ifdef DEBUG_BUDDY_FOCUS
            qDebug() << "layout rect" << widget->parentWidget()->layout()->contentsRect();
#endif
            if (widget->parentWidget()->layout()->contentsRect().width() < 0
                || widget->parentWidget()->layout()->geometry().height() < 0) {
                rect = QRect();
            } else {
                rect &= widget->parentWidget()->layout()->geometry();
            }
        }
#ifdef DEBUG_BUDDY_FOCUS
        qDebug() << "position focus for" << widget << label << rect;
#endif
        target = rect;
    }

    void moveHighlight() {
        if (animate) {
//            qDebug() << "moveHighlight";
            timeLine.stop();
            if (widget && widget->isVisible() && !isVisible()) {
//                qDebug() << "fade in from !visible";
                // not currently visibile - e.g. first application show, tab change
                // Fade in from transparent
                if (parentWidget() != widget->parentWidget())
                    setParent(widget->parentWidget());
                state = FadeIn;
                timeLine.setCurrentTime(0);
                timeLine.setDirection(QTimeLine::Forward);
                timeLine.resume();
                setGeometry(target);
                lower();
                show();
            } else if (state != Hidden) {
//                qDebug() << "fade out";
                // We're visible in some way - fade out from whatever frame we're at.
                state = FadeOut;
                timeLine.setDirection(QTimeLine::Backward);
                timeLine.resume();
            } else if (widget && widget->isVisible()) {
//                qDebug() << "fade in";
                // Currently hidden - fade in from whatever frame we're at.
                state = FadeIn;
                if (parentWidget() != widget->parentWidget())
                    setParent(widget->parentWidget());
                timeLine.setDirection(QTimeLine::Forward);
                timeLine.resume();
                setGeometry(target);
                lower();
                show();
            }
        } else if (!widget) {
            state = Hidden;
            hide();
        } else if (widget->isVisible() && target.isValid()) {
            if (!focusRectAllowed(widget)) {
#ifdef DEBUG_BUDDY_FOCUS
                qDebug() << "Hiding focus rect";
#endif
                state = Hidden;
                hide();
            } else {
                if (parentWidget() != widget->parentWidget())
                    setParent(widget->parentWidget());
#ifdef DEBUG_BUDDY_FOCUS
                qDebug() << "Show focus rect";
#endif
                state = Solid;
                lower();
                show();
                setGeometry(target);
            }
        }
    }

protected:
    void paintEvent(QPaintEvent *) {
        QStylePainter p(this);
        QStyleOption opt;
        opt.initFrom(this);
        opt.rect = rect();
        /*
        if (widget->hasEditFocus() || qobject_cast<QGroupBox*>(widget)) {
            QRegion rgn(geometry());
            rgn -= widget->contentsRect().translated(widget->geometry().topLeft());
            rgn.translate(-geometry().topLeft());
            p.setClipRegion(rgn);
        }
        p.drawPrimitive((QStyle::PrimitiveElement)QPhoneStyle::PE_ExtendedFocusHighlight, opt);
        */
        p.setPen(Qt::NoPen);
        QColor col = opt.palette.brush(QPalette::Highlight).color();
        if (animate) {
//            qDebug() << "paint frame" << timeLine.currentFrame();
            col.setAlpha(col.alpha() * timeLine.currentFrame() / 255);
        }
        p.setBrush(QBrush(col));
        p.setRenderHint(QPainter::Antialiasing);
        if (widget && (widget->hasEditFocus() || qobject_cast<QGroupBox*>(widget))) {
            if (widget->width() < width() && widget->height() < height()) {
                QRect wgeom = widget->geometry();
                QPainterPath rectPath;
                QPainterPath clipPath;
                QGroupBox *gb = qobject_cast<QGroupBox*>(widget);
                if (gb && gb->isCheckable()) {
                    //put bounds around frame rather than frame and title
                    QStyleOptionGroupBox opt;
                    opt.initFrom(gb);
                    opt.text = gb->title();
                    if (gb->isChecked())
                        opt.subControls |= QStyle::SC_GroupBoxCheckBox;
                    QRect frame = style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxFrame, gb);
                    wgeom.setTop(wgeom.top() + frame.y());

                    int sp = 0;
                    if (widget->parentWidget() && widget->parentWidget()->layout())
                        sp = widget->parentWidget()->layout()->spacing();
                    frame.adjust(0, 0, sp, sp);
                    rectPath.addRoundRect(frame, 800/frame.width(), 800/frame.height());

                    //put highlight around checkbox and title as well
                    QRect checkRect = style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, gb);
                    checkRect = checkRect.united(style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxCheckBox, gb));
                    checkRect.adjust(0,0,sp,sp);
                    p.drawRoundRect(checkRect, 800/checkRect.width(), 800/checkRect.height());

                    QRegion region(this->rect());
                    region -= QRegion(checkRect);
                    p.setClipRegion(region);
                } else {
                    rectPath.addRoundRect(opt.rect, 800/opt.rect.width(), 800/opt.rect.height());
                }
                clipPath.addRoundRect(wgeom.translated(-geometry().topLeft()), 800/wgeom.width(), 800/wgeom.height());
                p.drawPath(rectPath.subtracted(clipPath));
            }
        } else {
//            p.drawRoundRect(opt.rect,800/opt.rect.width(), 800/opt.rect.height());  //8 pixel corners
            QtopiaStyle::drawRoundRect(&p, opt.rect, 8, 8);
        }
    }

private slots:
    void finished() {
        if (state == FadeIn) {
            state = Solid;
        } else if (state == FadeOut) {
            if (widget) {
                // finished fading out.  If there is a new target widget
                // begin fade in.
//                qDebug() << "done fadeout - do fade in";
                if (parentWidget() != widget->parentWidget())
                    setParent(widget->parentWidget());
                state = FadeIn;
                timeLine.setCurrentTime(0);
                timeLine.setDirection(QTimeLine::Forward);
                timeLine.start();
                setGeometry(target);
                if (!isVisible()) {
                    lower();
                    show();
                }
            } else {
//                qDebug() << "No widget, hiding";
                // No new target - hide completely.
                hide();
                state = Hidden;
            }
        }
    }

private:
    enum ShowState { Hidden, FadeIn, FadeOut, Solid };
    QPointer<QLabel> label;
    QPointer<QWidget> widget;
    QRect target;
    ShowState state;
    QTimeLine timeLine;
    bool animate;
};

class QPhoneStylePrivate : public QObject
{
    Q_OBJECT
public:
    QPhoneStylePrivate(QPhoneStyle *style) : QObject(0), useExported(false) {
        q = style;
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
        bgExport = bgForScreen(QApplication::desktop()->primaryScreen());
#endif
        connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
                SLOT(focusChangedSlot(QWidget*,QWidget*)));

        connect(QApplication::desktop(), SIGNAL(workAreaResized(int)),
                this, SLOT(bgUpdated()));
    }

    void updateDecoration() {
#ifdef Q_WS_QWS
        QWidget *active = QApplication::activeWindow();
        if (active) {
            QWSManager *manager = ((ManagerAccessor*)active)->manager();
            if (manager) {
                QDesktopWidget *desktop = QApplication::desktop();
                QApplication::postEvent(manager,
                    new QPaintEvent(desktop->screenGeometry(desktop->primaryScreen())));
            }
        }
#endif
    }

#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    QExportedBackground *bgForScreen(int screen) {
        if (!bgExportMap.contains(screen)) {
            QExportedBackground *bge = new QExportedBackground(screen, this);
            bgExportMap[screen] = bge;
            connect(bge, SIGNAL(changed()), this, SLOT(bgUpdated()));
        }

        return bgExportMap.value(screen);
    }
#endif

    void setLabelFocus(QLabel *l, QWidget *w);

    void setTextColor(QWidget *w) {
        if (w) {
            bool focus = w->hasFocus();
            bool editfocus = w->hasEditFocus();
            if (!focus && !editfocus) {
                //want original color
                if (paletteManaged.contains(w)) {
                    w->setPalette(paletteManaged[w]);
                }
            }
            if (focus) {
                if (editfocus) {
                    //want original color
                    if (paletteManaged.contains(w)) {
                        w->setPalette(paletteManaged[w]);
                    }
                } else {
                    //want switched color
                    if (paletteManaged.contains(w)) {
                        QPalette pal = paletteManaged[w];
                        QColor temptext = pal.color(QPalette::Normal, QPalette::Text);
                        pal.setColor(QPalette::Normal, QPalette::Text, pal.color(QPalette::Normal, QPalette::HighlightedText));
                        pal.setColor(QPalette::Normal, QPalette::HighlightedText, temptext);
                        w->setPalette(pal);
                    }
                }
            }
        }
    }

    void drawStretch(QPainter *p, const QRect &r, const QPixmap &pm,
                    int off1, int off2, Qt::Orientation orient) const;

    bool eventFilter(QObject *, QEvent *);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    QExportedBackground *bgExport;
    QMap<int,QExportedBackground*> bgExportMap;
#endif
    QPalette bgPal;
    QPalette windowPal;
    QPalette origPal;
    bool useExported;
    QMap<QWidget*,int> bgManaged;
    QMap<QWidget*,QPalette> paletteManaged;
    QPointer<BuddyFocusBox> focusBox;

private:
    QPhoneStyle *q;
    QBasicTimer focusVisibilityTimer; //Ensure focus widget is visible when QScrollAreas resize

protected:
    void timerEvent(QTimerEvent *);

private slots:
    void bgUpdated();
    void focusChangedSlot(QWidget *, QWidget *);
    void updateLabelFocus();
};

QTOPIA_EXPORT int qtopia_background_natural_rotation(int screen)
{
#if defined(QTOPIA_ENABLE_GLOBAL_BACKGROUNDS) && defined(Q_WS_QWS)
    // XXX - we only do this for the primary screen at present and we
    // assume that the primary screen is specified first.
    if (screen < 0 || screen == qApp->desktop()->primaryScreen()) {
        static int naturalRotation = -1;
        if (naturalRotation != -1)
            return naturalRotation;
        QString spec = QString::fromLatin1(qgetenv("QWS_DISPLAY")).trimmed();
        if (spec.startsWith(QLatin1String("Multi:")))
            spec = spec.mid(6).trimmed();
        int space = spec.indexOf(QChar(' '));
        if (space >= 0)
            spec = spec.left(space);
        QRegExp regexp(QLatin1String("\\bRot(\\d+):?\\b"), Qt::CaseInsensitive);
        if (regexp.indexIn(spec) != -1)
            naturalRotation = regexp.cap(1).toInt();
        else
            naturalRotation = 0;
        return naturalRotation;
    } else {
        return 0;
    }
#else
    return 0;
#endif
}

QTOPIA_EXPORT int qtopia_background_brush_rotation(int screen)
{
#if defined(QTOPIA_ENABLE_GLOBAL_BACKGROUNDS) && defined(Q_WS_QWS)
    QScreen *scr = qt_screen;
    if (screen < 0)
        screen = qApp->desktop()->primaryScreen();
    if (scr->classId() == QScreen::MultiClass)
        scr = scr->subScreens()[screen];
    int rot = scr->transformOrientation() * 90 - qtopia_background_natural_rotation(screen);
    if (rot < 0)
        rot += 360;
    return rot;
#else
    return 0;
#endif
}

bool QPhoneStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    QWidget *wgt = qobject_cast<QWidget*>(o);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (wgt && bgManaged.contains(wgt)) {
        if (e->type() == QEvent::Paint) {
            if (wgt && wgt->isWindow()) {
                QPaintEvent *pe = (QPaintEvent*)e;
                QBrush b = wgt->palette().brush(QPalette::Window);
                QPainter p(wgt);
                p.setBrushOrigin(-wgt->geometry().topLeft());
                // We know we have a solid brush, so we can gain a little speed
                // using QPainter::CompositionMode_Source
                if (p.paintEngine()->hasFeature(QPaintEngine::PorterDuff))
                    p.setCompositionMode(QPainter::CompositionMode_Source);
                int screen = QApplication::desktop()->screenNumber(wgt);
                int rotation = qtopia_background_brush_rotation(screen);
                if (rotation != 0) {
                    QTransform transform = b.transform();
                    transform.rotate(rotation);
                    b.setTransform(transform);
                }
                p.fillRect(pe->rect(), b);
            }
        } else if (e->type() == QEvent::ParentChange) {
            if (!wgt->isWindow()) {
                //NOTE: ordering important (no reason to trigger below palette change)
                wgt->removeEventFilter(this);
                bgManaged.remove(wgt);
                wgt->setPalette(QApplication::palette());
                wgt->setAttribute(Qt::WA_OpaquePaintEvent, false);
            }
        } else if (e->type() == QEvent::ApplicationPaletteChange) {
            QApplication::style()->polish(wgt);
            foreach (QObject *o, wgt->children())
                if (QWidget *sw = qobject_cast<QWidget*>(o)) {
                    QApplication::style()->polish(sw);
                }
        } else if (e->type() == QEvent::Move) {
            int screen = QApplication::desktop()->screenNumber(wgt);
            if (screen >= 0 && bgManaged.contains(wgt) && bgManaged[wgt] != screen) {
                bgManaged[wgt] = screen;
                QExportedBackground *bge = bgForScreen(screen);
                if (bge->isAvailable()) {
                    QPalette pal = bgPal;
                    QColor windowCol = pal.color(QPalette::Window);
                    windowCol.setAlpha(255);
                    pal.setBrush(QPalette::Window, QBrush(windowCol, bge->background()));
                    wgt->setPalette(pal);
                } else {
                    QPalette pal = bgPal;
                    QColor windowCol = pal.color(QPalette::Window);
                    windowCol.setAlpha(255);
                    pal.setBrush(QPalette::Window, QBrush(windowCol));
                    wgt->setPalette(pal);
                }
            }
    //        w->repaint();  //XXX maybe nice if we allow windows to be moved by user
        } else if (e->type() == QEvent::Destroy) {
            bgManaged.remove(wgt);
        } else if (e->type() == QEvent::PaletteChange) {
            // XXX hackery to fix a problem with category selectors created
            // from .ui files ending up with transparent popup combo boxes...
            // also ensures qtimezoneselector popup is painted correctly
            if (o->inherits("QComboBoxPrivateContainer") && bgExport->isAvailable() ) {
                wgt->setPalette(windowPal);
                return true;
            }
        }
    } else if (wgt && e->type() == QEvent::ShowToParent) {
        QScrollArea *sa = qobject_cast<QScrollArea*>(wgt);
        if (sa && sa->widget()) {
            sa->widget()->setAutoFillBackground(false);
            if (!sa->viewport()->testAttribute(Qt::WA_SetPalette))
                sa->viewport()->setAutoFillBackground(false);
            wgt->removeEventFilter(this);   //should we be removing this?
        }
    }
#endif
    if (wgt && QApplication::keypadNavigationEnabled()) {
        if (e->type() == QEvent::Paint) {   //handle background of textedit
            if (QTextEdit *te = qobject_cast<QTextEdit*>(wgt)) {
                bool extendedFocus = q->styleHint((QStyle::StyleHint)QPhoneStyle::SH_ExtendedFocusHighlight, 0, 0);
                if (te->hasFocus() && !te->hasEditFocus() && !extendedFocus) {
                    QPainter p(te);
                    QRect r = te->rect();
                    r.adjust(3,3,-3,-3);
                    p.setRenderHint(QPainter::Antialiasing);
                    QColor color = te->palette().highlight().color();
                    p.setPen(color);
                    p.setBrush(te->palette().highlight());
                    QtopiaStyle::drawRoundRect(&p, r, 8, 8);
                }
            }
        } else if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
            QComboBox *cb = qobject_cast<QComboBox*>(wgt);
            if (wgt->inherits("QLineEdit") || wgt->inherits("QAbstractSpinBox") || (cb && cb->isEditable())) {
                setTextColor(wgt);
            } else if (QTextEdit *te = qobject_cast<QTextEdit*>(wgt)) {
                te->viewport()->setBackgroundRole(e->type() == QEvent::EnterEditFocus
                                                   ? QPalette::Base : QPalette::Background);
                setTextColor(wgt);
            }
        }
    }

    if(e->type() == QEvent::Resize)
    {
        if (qobject_cast<QMenu*>(wgt)) {
            //TODO: can we do a better check and make a more accurate mask?
            QRegion reg(wgt->rect());
            if (wgt->logicalDpiY() < 240) {
                reg -= QRegion(QRect(0, 0, 1, 1));
                reg -= QRegion(QRect(wgt->rect().right(), 0, 1, 1));
                reg -= QRegion(QRect(0, wgt->rect().bottom(), 1, 1));
                reg -= QRegion(QRect(wgt->rect().right(), wgt->rect().bottom(), 1, 1));
            } else {
                reg -= QRegion(QRect(0, 0, 2, 1));
                reg -= QRegion(QRect(wgt->rect().right()-1, 0, 2, 1));
                reg -= QRegion(QRect(0, wgt->rect().bottom(), 2, 1));
                reg -= QRegion(QRect(wgt->rect().right()-1, wgt->rect().bottom(), 2, 1));
                reg -= QRegion(QRect(0, 1, 1, 1));
                reg -= QRegion(QRect(wgt->rect().right(), 1, 1, 1));
                reg -= QRegion(QRect(0, wgt->rect().bottom()-1, 1, 1));
                reg -= QRegion(QRect(wgt->rect().right(), wgt->rect().bottom()-1, 1, 1));
            }
            wgt->setMask(reg);
        } else if (qobject_cast<QScrollArea*>(o) && QApplication::focusWidget())
            focusVisibilityTimer.start(0, this);
    }

    return false;
}

void QPhoneStylePrivate::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == focusVisibilityTimer.timerId()) {
        focusVisibilityTimer.stop();
        QWidget *w = QApplication::focusWidget();
        if (w && w->visibleRegion().boundingRect().size() != w->size()) {
            QScrollArea *sa = 0;
            while (w->parentWidget()) {
                if (w->objectName() == QLatin1String("qt_scrollarea_viewport")) {
                    sa = qobject_cast<QScrollArea*>(w->parentWidget());
                    break;
                }
                w = w->parentWidget();
            }
            if (sa)
                sa->ensureWidgetVisible(QApplication::focusWidget(), 0, 10);
        }
    }
}

void QPhoneStylePrivate::bgUpdated()
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (bgExport->isAvailable()) {
        if (!useExported)
            QApplication::setPalette(QApplication::palette());
        foreach (QWidget *w, QApplication::topLevelWidgets())
            w->update();
    }
#endif
}

void QPhoneStylePrivate::focusChangedSlot(QWidget *old, QWidget *now)
{
    if (q != qApp->style()) {
        disconnect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this,
                SLOT(focusChangedSlot(QWidget*,QWidget*)));
        return;
    }

    if (QApplication::keypadNavigationEnabled()) {
        QTextEdit *te = qobject_cast<QTextEdit*>(old);
        if (te) {
            te->viewport()->setBackgroundRole(QPalette::Background);
            setTextColor(old);
            te->removeEventFilter(this);
            paletteManaged.remove(old);
        }

        te = qobject_cast<QTextEdit*>(now);
        if (te) {
            paletteManaged.insert(now, now->palette());
            te->viewport()->setBackgroundRole(QPalette::Background);
            setTextColor(now);
            if (te->hasEditFocus()) //isSingleFocusWidget can cause us to get edit focus before we get here
                te->viewport()->setBackgroundRole(QPalette::Base);
            te->installEventFilter(this);
        }

        QComboBox *cb = qobject_cast<QComboBox*>(old);
        if (old && (old->inherits("QLineEdit") || old->inherits("QAbstractSpinBox")
                    || (cb && cb->isEditable()))) {
            setTextColor(old);
            old->removeEventFilter(this);
            paletteManaged.remove(now);
        }

        cb = qobject_cast<QComboBox*>(now);
        if (now && (now->inherits("QLineEdit") || now->inherits("QAbstractSpinBox")
                    || (cb && cb->isEditable()))) {
            paletteManaged.insert(now, now->palette());
            setTextColor(now);
            now->installEventFilter(this);
        }
    }

    QWidget *widget = (QWidget*)old;
    QScrollArea *sa;
    if(widget) {
        while (widget->parentWidget()) {
            //if (widget->objectName() == QLatin1String("qt_scrollarea_viewport"))

            if ((sa = qobject_cast<QScrollArea*>(widget->parentWidget()))) {
                sa->removeEventFilter(this);
            }
            widget = widget->parentWidget();
        }
    }

    widget = (QWidget*)now;
    if(widget) {
        while (widget->parentWidget()) {
            //if (widget->objectName() == QLatin1String("qt_scrollarea_viewport"))
            if ((sa = qobject_cast<QScrollArea*>(widget->parentWidget()))) {
                sa->installEventFilter(this);
            }
            widget = widget->parentWidget();
        }
    }

    if (!now || (now->window()->windowFlags() & Qt::Popup) != Qt::Popup) {
        if (qApp->style()->styleHint((QStyle::StyleHint)QPhoneStyle::SH_ExtendedFocusHighlight))
            updateLabelFocus();
    }
}

void QPhoneStylePrivate::updateLabelFocus()
{
    QWidget *focus = QApplication::focusWidget();
    QLabel *hl = QtopiaStyle::buddyForWidget(focus);
    if (hl)
        focus = hl->buddy();
    /*
    if (focus) {
        qDebug() << "Focus for" << focus
            << focus->testAttribute(Qt::WA_PendingMoveEvent)
            << focus->testAttribute(Qt::WA_PendingResizeEvent)
            << focus->testAttribute(Qt::WA_Resized);
    } else {
        qDebug() << "Remove focus";
    }
    */
    setLabelFocus(hl, focus);
}

void QPhoneStylePrivate::setLabelFocus(QLabel *l, QWidget *w)
{
    if (!q->styleHint((QStyle::StyleHint)QPhoneStyle::SH_ExtendedFocusHighlight, 0, w))
        return;

    static QPointer<QLabel> prevLabel;
    if (prevLabel) {
        prevLabel->setForegroundRole(QPalette::Foreground);
        prevLabel = 0;
    }

    if (!w || !w->parentWidget()) {
        if (focusBox)
            focusBox->setTargets(0, 0);
    } else {
        QRect rect = w->geometry();
        if (!focusBox)
            focusBox = new BuddyFocusBox(w->parentWidget());
        focusBox->setTargets(l, w);
        if (l) {
            l->setForegroundRole(QPalette::HighlightedText);
            prevLabel = l;
        }
    }
}

void QPhoneStylePrivate::drawStretch(QPainter *p, const QRect &r,
                                const QPixmap &pm, int off1, int off2,
                                Qt::Orientation orient) const
{
    if (pm.isNull())
        return;

    int ss = off2-off1;
    if (orient == Qt::Horizontal) {
        int h = pm.height();
        p->drawPixmap(r.x(), r.y(), pm, 0, 0, off1, h);
        int w = r.width() - off1 - (pm.width()-off2);
        int x = 0;
        if (ss) {
            for (; x < w-ss; x+=ss)
                p->drawPixmap(r.x()+off1+x, r.y(), pm, off1, 0, ss, h);
        }
        if (w > x)
            p->drawPixmap(r.x()+off1+x, r.y(), pm, off1, 0, w-x, h);
        p->drawPixmap(r.x()+r.width()-(pm.width()-off2), r.y(), pm,
                    off2, 0, pm.width()-off2, h);
    } else {
        int w = pm.width();
        p->drawPixmap(r.x(), r.y(), pm, 0, 0, w, off1);
        int h = r.height() - off1 - (pm.height()-off2);
        int y = 0;
        if (ss) {
            for (; y < h-ss; y+=ss) {
                p->drawPixmap(r.x(), r.y()+off1+y, pm, 0, off1, w, ss);
            }
        }
        if (h > y)
            p->drawPixmap(r.x(), r.y()+off1+y, pm, 0, off1, w, h-y);
        p->drawPixmap(r.x(), r.y()+r.height()-(pm.height()-off2), pm,
                    0, off2, w, pm.height()-off2);
    }
}

/*!
    \class QPhoneStyle
    \inpublicgroup QtBaseModule

    \brief The QPhoneStyle class provides a phone look and feel.

    \ingroup appearance

    This style is the default GUI style for Qt Extended Phone Edition.
    To write a customized style for Qt Extended Phone it is recommended that
    QPhoneStyle be subclassed.  QPhoneStyle provides improved display
    for widgets in edit and navigation mode.  It also applies the
    global background to Qt Extended widgets.

    \image qphonestyle.png
    \sa QWindowsStyle
*/

/*!
    \enum QPhoneStyle::PhoneStyleHint

    This enum describes the extra style hints available for QPhoneStyle and derived classes.
    A style hint is a general look and/or feel hint.

    \value SH_ExtendedFocusHighlight A boolean indicating whether the widget with focus should have an extended highlight.
           The extended highlight will highlight any labels associated with that widget along with the widget itself.
    \value SH_PopupShadows A boolean indicating whether popups (widgets with flag Qt::Popup) should have a shadow.
    \value SH_HideMenuIcons A boolean indicating whether menus should hide any icons associated with menu items.
    \value SH_FullWidthMenu A boolean indicating whether the context menu should extend to the full width of the screen.
    \value SH_ScrollbarLineStepButtons A boolean indicating whether scrollbars should display scroll arrows at the ends.

    \value SH_FormLayoutWrapPolicy Provides a default for how rows are wrapped in a QFormLayout. This enum value has been deprecated in favor of QStyle::SH_FormLayoutWrapPolicy in Qt/4.4. Returns a QFormLayout::RowWrapPolicy enum.
    \value SH_FormLayoutFieldGrowthPolicy Provides a default for how fields can grow in a QFormLayout. This enum value has been deprecated in favor of QStyle::SH_FormLayoutFieldGrowthPolicy in Qt/4.4.  Returns a QFormLayout::FieldGrowthPolicy enum.
    \value SH_FormLayoutFormAlignment Provides a default for how a QFormLayout aligns its contents within the available space. This enum value has been deprecated in favor of QStyle::SH_FormLayoutFormAlignment in Qt/4.4.  Returns a Qt::Alignment enum.
    \value SH_FormLayoutLabelAlignment Provides a default for how a QFormLayout aligns labels within the available space. This enum value has been deprecated in favor of QStyle::SH_FormLayoutLabelAlignment in Qt/4.4.  Returns a Qt::Alignment enum.


    \sa QStyle::StyleHint
*/

/*!
    Constructs a QPhoneStyle object.
*/
QPhoneStyle::QPhoneStyle() : QtopiaStyle()
{
    int dpi = QApplication::desktop()->screen()->logicalDpiY();
    comboArrowWidth = qRound(9.0 * dpi / 100.0);
    phoneCheckMarkWidth = qRound(6.0 * dpi / 100.0);
    sliderHeight = qRound(9.5 * dpi / 100.0);
    sliderGrooveHeight = qRound(4.25 * dpi / 100.0);

    if(!scrollbarSize) {
        int w = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen()).width();
        if(w > 240)
            scrollbarSize = qRound(8.0 * QApplication::desktop()->screen()->logicalDpiX() /100.0);
        else if(w > 180)
            scrollbarSize = 13;
        else
            scrollbarSize = 8;
    }
    d = new QPhoneStylePrivate(this);
}

/*!
    Destroys the QPhoneStyle object.
*/
QPhoneStyle::~QPhoneStyle()
{
    delete d;
}

#include <sys/types.h>
#include <unistd.h>

/*!
    \reimp
*/
void QPhoneStyle::polish(QPalette &pal)
{
    QtopiaStyle::polish(pal);

    // Reasonable time to sync the settings.
    gConfig()->sync();

#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        //"unset" palette of running widgets (prepare for repolish)
        foreach (QWidget *w, QApplication::topLevelWidgets()) {
            foreach (QObject *o, w->children()) {
                if (QWidget *sw = qobject_cast<QWidget*>(o))
                    if (sw->palette() == d->bgPal)
                        sw->setPalette(QPalette());
            }
        }

        //set palette
        d->origPal = pal;
        d->bgPal = pal;
        QColor col = pal.color(QPalette::Window);
        col.setAlpha(0);
        d->bgPal.setColor(QPalette::Window, col);
        /*
        col = pal.color(QPalette::Base);
        col.setAlpha(64);
        d->bgPal.setColor(QPalette::Base, col);
        col = pal.color(QPalette::AlternateBase);
        col.setAlpha(176);
        d->bgPal.setColor(QPalette::AlternateBase, col);
        col = pal.color(QPalette::Button);
        col.setAlpha(176);
        d->bgPal.setColor(QPalette::Button, col);
        */
        /*
        col = pal.color(QPalette::Highlight);

        // Make a nice horizontal gradient for highlight.
        QDesktopWidget *desktop = QApplication::desktop();
        int desktopWidth = desktop->screenGeometry(desktop->primaryScreen()).width();
        QLinearGradient g(0, 0, desktopWidth-1, 0);
        QColor gradCol(col);
        gradCol = gradCol.dark(110);
//        gradCol.setAlpha(176);
        g.setColorAt(0, gradCol);
        gradCol = col.light(130);
        gradCol.setAlpha(col.alpha()/2);
        g.setColorAt(1, gradCol);
//        col.setAlpha(176);
        QPixmap pm(desktopWidth, 1);
        pm.fill(QColor(0,0,0,0));
        QPainter p(&pm);
        p.fillRect(0, 0, pm.width(), pm.height(), g);
        d->bgPal.setBrush(QPalette::Highlight, QBrush(col, pm));
*/
        pal = d->bgPal;
        d->windowPal = d->bgPal;
        QColor windowCol = pal.color(QPalette::Window);
        windowCol.setAlpha(255);
        d->windowPal.setBrush(QPalette::Window, QBrush(windowCol, d->bgExport->background()));
        d->useExported = true;
    } else if (d->useExported) {
        d->useExported = false;
        pal = d->origPal;
    }
#endif
}

/*!
    \reimp
*/
void QPhoneStyle::polish(QWidget *widget)
{
    QtopiaStyle::polish(widget);

    QTextEdit *te = qobject_cast<QTextEdit*>(widget);
    if (te)
        te->viewport()->setBackgroundRole(QPalette::Window);
    if (te && te->document()) {
        QString sheet = "a { color: palette(link) }; a:visited { color: palette(link-visited) };";
        te->document()->setDefaultStyleSheet(sheet);
    }
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(widget);
    if (aiv)
        aiv->viewport()->setBackgroundRole(QPalette::Window);
    QScrollBar *sb = qobject_cast<QScrollBar*>(widget);
    if (sb)
        sb->setAttribute(Qt::WA_OpaquePaintEvent, false); // For transparency
    QFrame *frame = qobject_cast<QFrame*>(widget);
    if (frame && !frame->parentWidget()) {
        if (frame->frameShape() != QFrame::NoFrame) {
            // All parentless frames get the rounded corner treatment
            frame->setFrameStyle(styleHint(QStyle::SH_ComboBox_PopupFrameStyle));
        }
    }
    if (Qtopia::mousePreferred()) {
        // We never want tabs to get focus, even without keypad navigation
        QTabBar *tabbar = qobject_cast<QTabBar*>(widget);
        if (tabbar) {
            tabbar->setFocusPolicy(Qt::NoFocus);
            QList<QToolButton*> scrollButtons = tabbar->findChildren<QToolButton*>();
            foreach (QToolButton *tb, scrollButtons)
                tb->setFocusPolicy(Qt::NoFocus);
        }
    }
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        bool isManaged = false;
        if (d->bgManaged.contains(widget)) {
            if (!widget->isWindow()) {
                // We were created without a parent, but now we have one.
                widget->setPalette(QApplication::palette());
                widget->removeEventFilter(d);
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                d->bgManaged.remove(widget);
            } else {
                isManaged = true;
            }
        }
        QScrollArea *sa = qobject_cast<QScrollArea*>(widget);
        if (sa && sa->widget()) {
            sa->widget()->setAutoFillBackground(false);
            if (!sa->viewport()->testAttribute(Qt::WA_SetPalette))
                sa->viewport()->setAutoFillBackground(false);
        } else if (sa && !sa->widget()) {
            sa->installEventFilter(d);
        }

        if (widget->isWindow()) {
            bool isTransparent = widget->palette().color(QPalette::Window) == QColor(0,0,0,0);
            if ((!widget->testAttribute(Qt::WA_SetPalette)
                || widget->palette() == d->bgPal
                || d->bgManaged.contains(widget))
                && !widget->testAttribute(Qt::WA_NoSystemBackground)
                && !isTransparent
                && (widget->windowFlags() & Qt::Desktop) != Qt::Desktop) {
                widget->setPalette(d->windowPal);
                widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
                if (!isManaged) {
                    d->bgManaged.insert(widget, QApplication::desktop()->primaryScreen());
                    widget->installEventFilter(d);
                }
            } else if(isTransparent && d->bgManaged.contains(widget)) {
                // unmanage transparent window
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                widget->removeEventFilter(this);
                d->bgManaged.remove(widget);
            }
        } else if (widget->parentWidget() && widget->parentWidget()->isWindow() && !widget->testAttribute(Qt::WA_SetPalette) ) {
            widget->setPalette(QApplication::palette());
        }
    } else {
        if (QMenu *menu = qobject_cast<QMenu*>(widget))
            menu->installEventFilter(d);
    }
#else
    if (QMenu *menu = qobject_cast<QMenu*>(widget))
        menu->installEventFilter(d);
#endif
}

/*!
    \reimp
*/
void QPhoneStyle::unpolish(QWidget *widget)
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        if (d->bgManaged.contains(widget)) {
            widget->setPalette(QApplication::palette());
            widget->removeEventFilter(d);
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        } else if (widget->parentWidget() && widget->parentWidget()->isWindow()) {
            widget->setPalette(QApplication::palette());
        }
    }
#endif

    QtopiaStyle::unpolish(widget);
}

/*!
    \reimp
*/
void QPhoneStyle::unpolish(QApplication *app)
{
    app->setPalette(d->origPal);

    QtopiaStyle::unpolish(app);
}

/*!
    \reimp
*/
int QPhoneStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ScrollBarExtent:
        ret = scrollbarSize;
        break;
    case PM_ScrollBarSliderMin:
        ret = scrollbarSize * 2;
        break;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        ret = 4;
        break;
    case PM_MenuPanelWidth:
        ret = 2; // to allow for rounded corners.
        break;
    case PM_TabBarTabHSpace:
        ret = 14;
        break;
    case PM_TabBarTabOverlap:
        ret = 10;
        break;
    case PM_TabBarIconSize:
        ret = QtopiaStyle::pixelMetric(PM_SmallIconSize, option, widget);
        break;
    case PM_TabBarTabShiftVertical:
        ret = 2;
        break;
    case PM_SliderLength:
    case PM_SliderControlThickness:
        ret = sliderHeight;
        break;
    case PM_SliderThickness:
        ret = sliderHeight;
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            if (slider->tickPosition & QSlider::TicksAbove)
                ret-= 5; //5 = TickSpace from QSlider
            if (slider->tickPosition & QSlider::TicksBelow)
                ret-= 5; //5 = TickSpace from QSlider;
        }
        break;

    default:
        ret = QtopiaStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}


/*!
    \reimp
*/
QSize QPhoneStyle::sizeFromContents(ContentsType type, const QStyleOption* opt,
                                const QSize &csz, const QWidget *widget ) const
{
    QSize sz(csz);
    switch (type) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            const QMenu *menu = qobject_cast<const QMenu*>(widget);
            bool fullWidthMenus =
                    qApp->style()->styleHint((QStyle::StyleHint)QPhoneStyle::SH_FullWidthMenu);
            if (fullWidthMenus) {
                if (QSoftMenuBar::widgetsFor(menu).isEmpty())
                    fullWidthMenus = false;
            }
            int w = 0;
            if (!fullWidthMenus || isSubMenu(widget)) {
                // make item only as wide as necessary, as in 4.2
                if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                    w = 10;
                } else {
                    w = sz.width();
                }
                int maxpmw = mi->maxIconWidth;
                if (mi->text.contains('\t'))
                    w += 12;
            	if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                    w += 2 * phoneArrowHMargin;
                if (mi->menuHasCheckableItems)
                    w += qMax(maxpmw, phoneCheckMarkWidth);
                if (!qApp->style()->styleHint((QStyle::StyleHint)QPhoneStyle::SH_HideMenuIcons))
                    w += pixelMetric(PM_SmallIconSize);
                w += phoneRightBorder + phoneItemHMargin*2;
            } else {
                w = QApplication::desktop()->availableGeometry().width()
                    - 2*pixelMetric(PM_MenuHMargin, opt, widget)
                    - 2*pixelMetric(QStyle::PM_MenuPanelWidth, opt, widget);
            }
            int h = 0;
            if (opt->state & State_Enabled) {
                h = sz.height();
                if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                    h = 3;
                } else {
                    h = qMax(h, mi->fontMetrics.height() + 4);
                    if (!mi->icon.isNull())
                        h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height() + 2);
                }
            }
            sz.setWidth(w);
            sz.setHeight(h);
        }
        break;
    case CT_Menu:
        {
            bool fullWidthMenus =
                    qApp->style()->styleHint((QStyle::StyleHint)QPhoneStyle::SH_FullWidthMenu);
            if (fullWidthMenus) {
                const QMenu *menu = qobject_cast<const QMenu*>(widget);
                if (QSoftMenuBar::widgetsFor(menu).isEmpty())
                    fullWidthMenus = false;
            }
            if (!fullWidthMenus || isSubMenu(widget)) {
                sz = QtopiaStyle::sizeFromContents(type, opt, csz, widget);
            } else {
                sz = QSize(QApplication::desktop()->availableGeometry().width(),
                           csz.height());
            }
        }
        break;
    case CT_TabBarTab:
        //sz = QtopiaStyle::sizeFromContents(type, opt, csz, widget);
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            int w = 0;
            int h = 0;
            if ( (tab->direction == Qt::LeftToRight &&
                    tab->position == QStyleOptionTab::End) ||
                  (tab->direction == Qt::RightToLeft &&
                    tab->position == QStyleOptionTab::Beginning) ) {
                w += pixelMetric(PM_TabBarTabOverlap, opt, widget);
            }
            if (!tab->icon.isNull() && !tab->text.isEmpty()) {
                w -= tab->fontMetrics.width(tab->text) - 4;
            }
            sz += QSize(w,h);
        }
        break;
    case CT_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            int fw = cmb->frame ? pixelMetric(PM_ComboBoxFrameWidth, opt, widget) * 2 : 0;
            sz = QSize(sz.width() + fw + comboArrowWidth + 7, sz.height() + fw);
        }
        break;
    default:
        sz = QtopiaStyle::sizeFromContents(type, opt, csz, widget);
        break;
    }

    return sz;
}

/*!
    \reimp
*/
QRect QPhoneStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const
{
    QRect ret;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QSize bs;
            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, w) : 0;
            bs.setHeight(spinbox->rect.height());
            bs.setWidth(comboArrowWidth);
            bs = bs.expandedTo(QApplication::globalStrut());
            //int y = fw;
            int x, lx, rx;
            x = spinbox->rect.width() - bs.width();
            lx = fw;
            rx = x - fw;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(x, 0, bs.width(), bs.height()/2);
                break;
            case SC_SpinBoxDown:
                ret = QRect(x, bs.height()/2, bs.width(), bs.height()/2);
                break;
            case SC_SpinBoxEditField:
                ret = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxFrame:
                ret = spinbox->rect;
                ret.setRight(ret.right()-bs.width());
            default:
                break;
            }
            ret = visualRect(spinbox->direction, spinbox->rect, ret);
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            int x = 0,
                y = 0,
                wi = cb->rect.width(),
                he = cb->rect.height();
            int xpos = x;
            int margin = cb->frame ? 2 : 0;
            int bmarg = cb->frame ? 2 : 0;
            xpos += wi - bmarg - comboArrowWidth;

            switch (sc) {
            case SC_ComboBoxFrame:
                ret = cb->rect;
                ret.setRight(ret.right()-comboArrowWidth);
                break;
            case SC_ComboBoxArrow:
                ret.setRect(xpos, y + bmarg, comboArrowWidth, he - bmarg*2);
                break;
            case SC_ComboBoxEditField:
                ret.setRect(x + margin, y + margin, wi - 2 * margin - comboArrowWidth, he - 2 * margin);
                break;
            case SC_ComboBoxListBoxPopup:
                ret = cb->rect;
                break;
            default:
                break;
            }
            ret = visualRect(cb->direction, cb->rect, ret);
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int sbextent = pixelMetric(PM_ScrollBarExtent, scrollbar, w);
            int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                          scrollbar->rect.width() : scrollbar->rect.height());
            bool lineStepButtons = styleHint((QStyle::StyleHint)SH_ScrollbarLineStepButtons, opt, w);
            if (lineStepButtons)
                maxlen -= sbextent * 2;
            int btnextent = lineStepButtons ? sbextent : 0;
            int sliderlen;

            // calculate slider length
            if (scrollbar->maximum != scrollbar->minimum) {
                uint range = scrollbar->maximum - scrollbar->minimum;
                sliderlen = (qint64(scrollbar->pageStep) * maxlen) / (range + scrollbar->pageStep);

                int slidermin = pixelMetric(PM_ScrollBarSliderMin, scrollbar, w);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sliderPositionFromValue(scrollbar->minimum,
                                                         scrollbar->maximum,
                                                         scrollbar->sliderPosition,
                                                         maxlen - sliderlen,
                                                         scrollbar->upsideDown);
            if (lineStepButtons)
                sliderstart += sbextent;
            switch (sc) {
            case SC_ScrollBarSubLine:            // top/left button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width() / 2, btnextent);
                    ret.setRect(0, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height() / 2, btnextent);
                    ret.setRect(0, 0, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarAddLine:            // bottom/right button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width()/2, btnextent);
                    ret.setRect(scrollbar->rect.width() - buttonWidth, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height()/2, btnextent);
                    ret.setRect(0, scrollbar->rect.height() - buttonHeight, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarSubPage:            // between top/left button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(btnextent, 0, sliderstart - btnextent, sbextent);
                else
                    ret.setRect(0, btnextent, sbextent, sliderstart - btnextent);
                break;
            case SC_ScrollBarAddPage:            // between bottom/right button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart + sliderlen, 0,
                                maxlen - sliderstart - sliderlen + btnextent, sbextent);
                else
                    ret.setRect(0, sliderstart + sliderlen, sbextent,
                                maxlen - sliderstart - sliderlen + btnextent);
                break;
            case SC_ScrollBarGroove:
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(btnextent, 0, scrollbar->rect.width() - btnextent * 2,
                                scrollbar->rect.height());
                else
                    ret.setRect(0, btnextent, scrollbar->rect.width(),
                                scrollbar->rect.height() - btnextent * 2);
                break;
            case SC_ScrollBarSlider: {
                int w = (sbextent == 0) ? sbextent : qMax(5, sbextent*7/8);
                int dif = (sbextent == w) ? 0 : (sbextent-w)/2;
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart, dif, sliderlen, w);
                else
                    ret.setRect(dif, sliderstart, w, sliderlen);
                break;}
            default:
                break;
            }
            ret = visualRect(scrollbar->direction, scrollbar->rect, ret);
        }
        break;
    case CC_GroupBox: {
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
            if (groupBox->features & QStyleOptionFrameV2::Flat) {
                QStyleOptionGroupBox myOpt(*groupBox);
                myOpt.features &= (~QStyleOptionFrameV2::Flat);
                ret = QtopiaStyle::subControlRect(cc, &myOpt, sc, w);
            } else {
                ret = QtopiaStyle::subControlRect(cc, opt, sc, w);
            }
        }
        break; }
    default:
        ret = QtopiaStyle::subControlRect(cc, opt, sc, w);
    }

    return ret;
}

/*!
    \reimp
*/
QRect QPhoneStyle::subElementRect(SubElement sr, const QStyleOption *opt,
                                   const QWidget *widget) const
{
    QRect r;
    switch (sr) {
    case SE_PushButtonContents:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int dx1, dx2;
            dx1 = pixelMetric(PM_DefaultFrameWidth, btn, widget)+2;
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                dx1 += pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            dx2 = dx1 * 2;
            r.setRect(opt->rect.x() + dx1, opt->rect.y() + dx1, opt->rect.width() - dx2,
                      opt->rect.height() - dx2);
            r = visualRect(opt->direction, opt->rect, r);
        }
        break;
    case QStyle::SE_TabWidgetTabBar:
    {
        r = QtopiaStyle::subElementRect(sr, opt, widget);

        // extend bar so tab labels in two-line mode are not cropped
        // when longer than total length of tabs
        if (const QStyleOptionTabWidgetFrame *tab = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            bool verticalTabs = tab->shape == QTabBar::RoundedEast
                                || tab->shape == QTabBar::RoundedWest
                                || tab->shape == QTabBar::TriangularEast
                                || tab->shape == QTabBar::TriangularWest;
            if (verticalTabs) {
                r.setTop(opt->rect.top());
                r.setBottom(opt->rect.bottom());
            } else {
                r.setLeft(opt->rect.left());
                r.setRight(opt->rect.right());
            }
        }   //QStyleOptionTabBarBase?
        break;
    }
    default:
        r = QtopiaStyle::subElementRect(sr, opt, widget);
    }

    if (sr == SE_TabWidgetTabBar || sr == SE_TabWidgetTabPane || sr == SE_TabWidgetTabContents) {
        const QStyleOptionTabWidgetFrame *tab = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt);
        const QTabWidget *tw = qobject_cast<const QTabWidget *>(widget);
        if (tab && tw) {
            bool twoLine = false;
            for (int i=0; i<tw->count(); i++) {
                if (!tw->tabIcon(i).isNull() && !tw->tabText(i).isEmpty()) {
                    twoLine = true;
                    break;
                }
            }

            // in two-line tab mode, make tabbar bigger to fit in the line
            // of text, and move & shrink tab pane & contents to compensate
            // (r has already been set, in the default case above)
            if (twoLine) {
                int textHeight = tab->fontMetrics.height() + 4;
                if (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::TriangularNorth) {
                    if (sr == SE_TabWidgetTabBar) {
                        r.adjust(0, 0, 0, textHeight);
                    } else {
                        r.adjust(0, textHeight, 0, 0);
                    }
                } else if (tab->shape == QTabBar::RoundedSouth || tab->shape == QTabBar::TriangularSouth) {
                    if (sr == SE_TabWidgetTabBar) {
                        r.adjust(0, -textHeight, 0, 0);
                    } else {
                        r.adjust(0, -textHeight, 0, -textHeight);
                    }
                } else if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    if (sr == SE_TabWidgetTabBar) {
                        r.adjust(-textHeight, 0, 0, 0);
                    } else {
                        r.adjust(-textHeight, 0, -textHeight, 0);
                    }
                } else if (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::TriangularWest) {
                    if (sr == SE_TabWidgetTabBar) {
                        r.adjust(textHeight, 0, 0, 0);
                    } else {
                        r.adjust(textHeight, 0, textHeight, 0);
                    }
                }
            }
        }
    }

    return r;
}

/*!
    \reimp
*/
void QPhoneStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt,
                                  QPainter *p, const QWidget *widget) const
{
    switch (pe) {
    case PE_FrameButtonBevel:
    case PE_FrameButtonTool:
        //qDrawShadeRect(p, opt->rect, opt->palette,
        //               opt->state & (State_Sunken | State_On), 1, 0);
        break;
    case PE_IndicatorButtonDropDown:
        break;
    case PE_PanelButtonTool:
    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel: {
        QRect r = opt->rect;
        r.adjust(2,2,-3,-3);

        bool enabled = opt->state & State_Enabled;
        bool focus = opt->state & State_HasFocus;
        bool pressed = opt->state & (State_On | State_Sunken);

        if (!focus) {
            QRect shadowRect = r;
            shadowRect.adjust(1,1,1,1);
            p->setBrush(Qt::NoBrush);
            p->setPen(opt->palette.dark().color());
            p->setRenderHint(QPainter::Antialiasing);
            drawRoundRect(p, shadowRect, 10, 10);
        }

        QColor bg = opt->palette.color(!enabled ? QPalette::Base :
                (focus && !Qtopia::mousePreferred()) ? QPalette::Highlight : QPalette::Button);
        if (widget && qobject_cast<QTabBar*>(widget->parentWidget())) {
            if (enabled)
                bg.setAlpha(255);
            else {
                QColor windowCol = opt->palette.color(QPalette::Window);
                windowCol.setAlpha(255);
                QBrush brush(windowCol);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
                brush = QBrush(windowCol, d->bgExport->background());
                p->setBrushOrigin(-widget->mapToGlobal(QPoint(0,0)));
#endif
                p->setPen(Qt::NoPen);
                p->setBrush(brush);
                drawRoundRect(p, r, 10, 10);
            }
        }
        if (pressed)
            bg = bg.darker(130);
        QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
        bgg.setColorAt(0.4f, bg);
        bgg.setColorAt(1.0f, bg.lighter(120));
        p->setBrush(bgg);
        //TODO: cache gradient

        p->setPen(focus ? bg.lighter(120) : bg.darker(120));
        p->setRenderHint(QPainter::Antialiasing, false);
        drawRoundRect(p, r, 10, 10);

        //shine
        p->setRenderHint(QPainter::Antialiasing);
        int shineVal = pressed ? 32 : 255;
        p->setBrush(QColor(shineVal,shineVal,shineVal, 60));
        p->setPen(Qt::NoPen);

        QPainterPath path;
        path.moveTo(r.right()+1, r.y()+r.height()*1/6);
        path.lineTo(r.right()+1, r.y()+5);
        path.arcTo(r.right()-9, r.y(), 10, 10,  0, 90);
        path.lineTo(r.left()+5, r.y());
        path.arcTo(r.left(), r.y(), 10, 10,  90, 90);
        path.lineTo(r.left(), r.y()+r.height()*5/6);
        path.quadTo(r.left()+r.width()*1/8, r.y()+r.height()*1/6, r.left()+r.width()*1/4, r.y()+r.height()*1/6);
        path.closeSubpath();
        p->drawPath(path);

        if (focus && !Qtopia::mousePreferred()) {
            p->setBrush(Qt::NoBrush);
            r.adjust(-2,-2, 3,3);
            p->setPen(QPen(opt->palette.color(QPalette::Highlight), 2));
            drawRoundRect(p, r, 10, 10);
        }

        break; }
    case PE_FrameDefaultButton:
        break;
    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            if (Qtopia::mousePreferred() && !(fropt->state & State_KeyboardFocusChange))
                return;
            p->setRenderHint(QPainter::Antialiasing);
            QRect r = opt->rect;
            r.adjust(1, 1, -1, -1);
            QPen oldPen = p->pen();
            QColor color = fropt->palette.highlight().color().darker(120);
            p->setPen(color);
            drawRoundRect(p, r, 6, 6);
            p->setPen(oldPen);
        }
        break;
    case PE_FilledFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            if (Qtopia::mousePreferred() && !(fropt->state & State_KeyboardFocusChange))
                return;
            QRect r = opt->rect;
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            QColor color = fropt->palette.highlight().color();
            p->setPen(color);
            p->setBrush(fropt->palette.highlight());
            drawRoundRect(p, r, 6, 6);
            p->restore();
        }
        break;
    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            p->setPen(opt->palette.light().color());
            p->drawLine(twf->rect.left(),twf->rect.top(),twf->rect.right(),twf->rect.top());
            if (const QTabWidget *tw = qobject_cast<const QTabWidget*>(widget)) {
                int current = tw->currentIndex();
                if (!tw->tabIcon(current).isNull() && !tw->tabText(current).isEmpty()) {
                    QRect r(twf->rect);
                    r.adjust(0,-(twf->fontMetrics.height() + 4),0,0);
                    p->drawLine(r.left(), r.top(), r.right(), r.top());
                }
            }
        }
        break;
    case PE_Frame:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if ((frame->state & State_Sunken) || (frame->state & State_Raised)) {
                if (widget && widget->inherits("QTextEdit")) {
                    bool extendedFocus = styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, opt, widget);
                    if (widget->hasFocus() && !extendedFocus) {
                        QStyleOptionFocusRect fropt;
                        fropt.state = State_KeyboardFocusChange;
                        fropt.rect = opt->rect;
                        fropt.palette = opt->palette;
                        drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                    } else if (!(opt->state & State_HasEditFocus)) {
                        QPen oldPen = p->pen();
                        QPen pen;
                        QColor c = opt->palette.color(QPalette::Text);
                        c.setAlpha(48);
                        pen.setColor(c);
                        p->setPen(pen);
                        QRect r = frame->rect;
                        r.adjust(0,0,-1,-1);
                        drawRoundRect(p, r, 8, 8);
                        p->setPen(oldPen);
                    }
                } else {
                    qDrawShadePanel(p, frame->rect, frame->palette, frame->state & State_Sunken,
                            frame->lineWidth);
                }
            } else {
                QRectF rect = frame->rect;
                rect.adjust(0.5, 0.5, -0.5, -0.5);
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(QPen(opt->palette.color(QPalette::WindowText).darker(130), 1));
                p->setBrush(Qt::NoBrush);
                drawRoundRect(p, rect, 8, 8);
            }
        }
        break;
    case PE_FrameMenu:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            QRectF rect = frame->rect;
            rect.adjust(0.5, 0.5, -0.5, -0.5);
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(QPen(opt->palette.color(QPalette::ButtonText).darker(130), 1));
            p->setBrush(Qt::NoBrush);
            drawRoundRect(p, rect, 8, 8);
        }
        break;
    case PE_FrameLineEdit: {
        bool extendedFocus = styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, opt, widget);
        if (opt->state & State_HasFocus && !extendedFocus) {
            QStyleOptionFocusRect fropt;
            fropt.state = State_KeyboardFocusChange;
            fropt.rect = opt->rect;
            fropt.palette = opt->palette;
            drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
        } else if (!(opt->state & State_HasEditFocus)) {
            QPen oldPen = p->pen();
            const QRect &r = opt->rect;
            //p->setPen(QPen(opt->palette.background(), 1));
            //p->drawLine(r.x(), r.bottom(), r.x(), r.top());
            //p->drawLine(r.x(), r.top(), r.right(), r.top());
            //p->drawLine(r.right(), r.top(), r.right(), r.bottom());
            QPen pen;
            QColor c = opt->palette.color(QPalette::Text);
            c.setAlpha(48);
            pen.setColor(c);
            p->setPen(pen);
            p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
            p->setPen(oldPen);
        }
        break; }
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (panel->state & State_Enabled && panel->state & State_HasEditFocus) {
                QBrush bg = panel->palette.brush(QPalette::Base);
                p->fillRect(panel->rect.adjusted(panel->lineWidth, panel->lineWidth, -panel->lineWidth, -panel->lineWidth), bg);
            }

            if (panel->lineWidth > 0)
                drawPrimitive(PE_FrameLineEdit, panel, p, widget);

            if (opt->state & State_HasFocus && !(panel->state & State_HasEditFocus)
                && !styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, panel, widget)) {

                QRect r = opt->rect;
                int m = 3;
                if (widget) {
                    if (qobject_cast<const QComboBox *>(widget->parentWidget()))
                        m = 1;
                    else if (const QDateTimeEdit *dte = qobject_cast<const QDateTimeEdit *>(widget->parentWidget()))
                        m = dte->calendarPopup() ? 1 : 2;
                    else if (qobject_cast<const QSpinBox *>(widget->parentWidget()))
                        m = 2;
                }
                r.adjust(m,m,-m,-m);

                QStyleOptionFocusRect focus;
                focus.QStyleOption::operator=(*panel);
                focus.rect = r;
                focus.state |= State_FocusAtBorder;
                focus.backgroundColor = panel->palette.highlight().color();
                drawPrimitive((QStyle::PrimitiveElement)PE_FilledFocusRect, &focus, p, widget);
            }
        }
        break;
    case PE_IndicatorCheckBox:
    case PE_IndicatorViewItemCheck: {
        p->save();
        QRect r(opt->rect);
        //bool down = opt->state & State_Sunken;
        bool enabled = opt->state & State_Enabled;
        //bool on = opt->state & State_On;
        bool focus = opt->state & State_HasFocus;
        bool small = r.width() < 13;
        bool full = (pe == PE_IndicatorCheckBox) && !small;

        //border
        if (full) {
            p->setRenderHint(QPainter::Antialiasing);
            QColor c1 = opt->palette.color(QPalette::Shadow);
            QColor c2 = opt->palette.color(QPalette::Button);
            c1.setAlpha(120);
            c2.setAlpha(120);
            QLinearGradient penGrad(0, 0, 1, 1);
            penGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
            penGrad.setColorAt(0.0f, c2);
            penGrad.setColorAt(0.65f, c1);
            p->setPen(Qt::NoPen);
            p->setBrush(penGrad);
            r.adjust(0,0,1,1);
            QPainterPath path;
            path.addRect(r);
            p->drawPath(path);
            r.adjust(0,0,-1,-1);
        }

        //inside
        p->setRenderHint(QPainter::Antialiasing, false);
        if (!small)
            r.adjust(2,2,-2,-2);
        QColor c = opt->palette.color(!enabled ? QPalette::Base :
                (focus && !Qtopia::mousePreferred()) ? QPalette::Highlight : QPalette::Button);   //||down
        QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
        bgg.setColorAt(0.4f, c);
        bgg.setColorAt(1.0f, c.lighter(120));
        p->setBrush(bgg);
        p->setPen(c.darker(120));
        p->drawRect(r);
        p->setRenderHint(QPainter::Antialiasing);

        //shine
        p->setBrush(QColor(255,255,255,110));
        p->setPen(Qt::NoPen);
        int shinew = (r.width() > 12) ? r.width()/3 : 4;
        int shineh = shinew/2;

        QRect shine(r.left()+2, r.top()+2, shinew, shineh);
        p->drawRect(shine);

        if (!small)
            r.adjust(-1,-1,1,1);
        if (opt->state & State_On) {
            small ? r.adjust(2,-1,2,-1) : r.adjust(2,-2,2,-1);
            QVector<QPoint> points;
            points.append(QPoint(r.left(),r.center().y()));
            if (!small)
                points.append(QPoint(r.left()+3,r.center().y()));
            points.append(QPoint(r.center().x(),r.bottom()-3));
            if (!small)
                points.append(QPoint(r.right()-3,r.top()));
            points.append(QPoint(r.right(),r.top()));
            points.append(QPoint(r.center().x(),r.bottom()));
            QPolygon check(points);

            p->setPen(opt->palette.color((focus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText));
            p->setBrush(opt->palette.brush((focus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText));
            p->setRenderHint(QPainter::Antialiasing);
            p->drawPolygon(check);
        } else if (opt->state & State_NoChange) {
            r.adjust(4,1,-2,1);
            QPen pen(enabled ? opt->palette.buttonText().color() : opt->palette.dark().color());
            pen.setWidth(2);
            p->setPen(pen);
            p->setBrush(enabled ? opt->palette.buttonText() : opt->palette.dark());
            p->drawLine(r.left(),r.center().y(),r.right(),r.center().y());
        }
        p->restore();
        break; }
    case PE_IndicatorRadioButton: {
        p->setRenderHint(QPainter::Antialiasing);
        //bool down = opt->state & State_Sunken;
        bool enabled = opt->state & State_Enabled;
        bool on = opt->state & State_On;
        bool focus = opt->state & State_HasFocus;
        QRect r(opt->rect);

        //border
        QColor c1 = opt->palette.color(QPalette::Shadow);
        QColor c2 = opt->palette.color(QPalette::Button);
        c1.setAlpha(120);
        c2.setAlpha(120);
        QLinearGradient penGrad(0, 0, 1, 1);
        penGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
        penGrad.setColorAt(0.0f, c2);
        penGrad.setColorAt(0.65f, c1);
        p->setPen(Qt::NoPen);
        p->setBrush(penGrad);
        r.adjust(0,0,1,1);
        QPainterPath path;
        path.addEllipse(r);
        p->drawPath(path);
        r.adjust(0,0,-1,-1);

        //ball
        r.adjust(2,2,-1,-1);
        QColor c = opt->palette.color(!enabled ? QPalette::Base : focus ? QPalette::Highlight : QPalette::Button);   //||down
        QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
        bgg.setColorAt(0.4f, c);
        bgg.setColorAt(1.0f, c.lighter(120));
        p->setBrush(bgg);
        p->setPen(c.darker(120));
        p->drawEllipse(r);

        //shine
        p->setBrush(QColor(255,255,255,110));
        p->setPen(Qt::NoPen);
        QRect shine1(0, 0, 4, 2);
        p->save();
        p->translate(r.left()+2, r.top()+4);
        p->rotate(-45);
        p->drawEllipse(shine1);
        p->restore();

        if (on) {
            //p->setBrush(c.darker(200));
            //p->setPen(c.darker(200));
            p->setPen(opt->palette.color((focus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText));
            p->setBrush(opt->palette.brush((focus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText));
            r.adjust(3,3,-3,-3);
            p->drawEllipse(r);
        }
        }
        break;
    case PE_IndicatorSpinUp:
        drawPrimitive(PE_IndicatorArrowUp, opt, p, widget);
        break;
    case PE_IndicatorSpinDown:
        drawPrimitive(PE_IndicatorArrowDown, opt, p, widget);
        break;
    case PE_IndicatorMenuCheckMark: {
        int maxwidth = phoneCheckMarkWidth > 10 ? phoneCheckMarkWidth - 4 : 7;
        const int markW = opt->rect.width() > maxwidth ? maxwidth : opt->rect.width();
        const int markH = markW;
        int posX = opt->rect.x() + (opt->rect.width() - markW)/2 + 1;
        int posY = opt->rect.y() + (opt->rect.height() - markH)/2;

        QVector<QLineF> a;
        a.reserve(markH);

        int i, xx, yy;
        xx = posX;
        yy = markH/2 + posY;
        for (i = 0; i < markW/2; ++i) {
            a << QLineF(xx, yy, xx, yy + 2);
            ++xx;
            ++yy;
        }
        yy -= 2;
        for (; i < markH; ++i) {
            a << QLineF(xx, yy, xx, yy + 2);
            ++xx;
            --yy;
        }
        if (!(opt->state & State_Enabled) && !(opt->state & State_On)) {
            int pnt;
            p->setPen(opt->palette.highlightedText().color());
            QPoint offset(1, 1);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt].translate(offset.x(), offset.y());
            p->drawLines(a);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt].translate(offset.x(), offset.y());
        }
        p->setPen((opt->state & State_On) ? opt->palette.highlightedText().color() : opt->palette.buttonText().color());
        p->drawLines(a);
        break; }
    case PE_FrameGroupBox: {
        p->setBrush(opt->palette.base());
        p->setPen(opt->palette.mid().color());
        if (opt->state & State_HasFocus && styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, opt, widget)) {
            //don't fill area under highlight
            const QGroupBox *gb = qobject_cast<const QGroupBox*>(widget);
            if (gb && gb->isCheckable()) {
                QStyleOptionGroupBox gbopt;
                gbopt.initFrom(gb);
                gbopt.text = gb->title();
                if (gb->isChecked())
                    gbopt.subControls |= QStyle::SC_GroupBoxCheckBox;

                int sp = 0;
                if (widget->parentWidget() && widget->parentWidget()->layout())
                    sp = widget->parentWidget()->layout()->spacing();

                QRect checkRect = subControlRect(QStyle::CC_GroupBox, &gbopt, QStyle::SC_GroupBoxLabel, gb);
                checkRect = checkRect.united(subControlRect(QStyle::CC_GroupBox, &gbopt, QStyle::SC_GroupBoxCheckBox, gb));
                checkRect.adjust(-sp/2,0,sp/2,sp/2);

                //TODO: fix noise in corners (use modified painter path rather than clip region?)
                QRegion region(opt->rect);
                region -= QRegion(checkRect);
                p->setClipRegion(region);
            }
        }
        QRect r = opt->rect;
        r.adjust(0,0,-1,-1);
        drawRoundRect(p, r, 8, 8);
        break; }
    case PE_ExtendedFocusHighlight: {
        p->setPen(opt->palette.brush(QPalette::Button).color());
        p->setBrush(opt->palette.brush(QPalette::Highlight));
        p->setRenderHint(QPainter::Antialiasing);
        drawRoundRect(p, opt->rect, 8, 8);
        break; }
    default:
        QtopiaStyle::drawPrimitive(pe, opt, p, widget);
    }
}

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
        case Qt::LeftArrow:
            pe = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            pe = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            pe = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            pe = QStyle::PE_IndicatorArrowDown;
            break;
        default:
            return;
    }
    QStyleOption arrowOpt;
    arrowOpt.rect = rect;
    arrowOpt.palette = toolbutton->palette;
    arrowOpt.state = toolbutton->state;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

/*!
    \reimp
*/
void QPhoneStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    switch (ce) {
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable
                            ? menuitem->checked : false;
            bool act = menuitem->state & State_Selected;
            bool drawicons = !styleHint((QStyle::StyleHint)SH_HideMenuIcons, menuitem, widget);

            int iconcol = qMax(menuitem->maxIconWidth, 18);
            int checkcol = 18;

            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.button().color());
                p->drawLine(x, y, x + w, y);
                p->drawLine(x, y + 2, x + w, y + 2);
                QColor color = menuitem->palette.color(QPalette::Active, QPalette::ButtonText);
                color.setAlpha(125);
                p->setPen(color);
                p->drawLine(x, y + 1, x + w, y + 1);
                return;
            }

            p->setPen(Qt::NoPen);
            p->fillRect(menuitem->rect, menuitem->palette.brush(QPalette::Button)); //not optimal (we paint the same space twice if act)
            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->setBrush(fill);
            if (act) {
                p->setRenderHint(QPainter::Antialiasing);
                drawRoundRect(p, menuitem->rect, 6, 6);
                p->setRenderHint(QPainter::Antialiasing, false);
            }

            if (!menuitem->icon.isNull() && drawicons) {    //draw icon
                QRect vIconRect = visualRect(opt->direction, menuitem->rect, QRect(x, y, iconcol, h));
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap = menuitem->icon.pixmap(
                        pixelMetric(PM_SmallIconSize), mode, checked ? QIcon::On : QIcon::Off);
                QRect pmr(0, 0, pixmap.width(), pixmap.height());
                pmr.moveCenter(vIconRect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);
            }

            p->setPen(act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color());
            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }

            int xm = phoneItemFrame + phoneItemHMargin + (drawicons ? iconcol : 0);
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + phoneItemVMargin,
                           w - xm - phoneRightBorder - tab + 1 - (checked ? checkcol : 0 ), h - 2 * phoneItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect, QRect(textRect.topRight(), menuitem->rect.bottomRight()));
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                p->drawText(vTextRect, text_flags, s.left(t));
                p->restore();
            }

            if (menuitem->checkType == QStyleOptionMenuItem::Exclusive) {   //draw exclusive indicators
                QRect vCheckRect = visualRect(opt->direction, menuitem->rect, QRect(x+w-checkcol, y, checkcol, h));
                //size calculations same as those in PE_IndicatorMenuCheckMark
                int maxwidth = phoneCheckMarkWidth > 10 ? phoneCheckMarkWidth - 4 : 7;
                const int markW = vCheckRect.width() > maxwidth ? maxwidth : vCheckRect.width();
                const int markH = markW;
                int posX = vCheckRect.x() + (vCheckRect.width() - markW)/2 + 1;
                int posY = vCheckRect.y() + (vCheckRect.height() - markH)/2;
                QRect radioRect(posX, posY, markH, markH);
                p->setPen(act ? opt->palette.highlightedText().color() : opt->palette.buttonText().color());
                p->drawEllipse(radioRect);
                if (checked) {
                    p->setBrush(act ? opt->palette.highlightedText() : opt->palette.buttonText());
                    p->drawEllipse(radioRect.adjusted(2,2,-2,-2));
                }
            } else if (checked) {  //draw check
                QRect vCheckRect = visualRect(opt->direction, menuitem->rect, QRect(x+w-checkcol, y, checkcol, h));
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = vCheckRect;
                drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }

            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * phoneItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - phoneArrowHMargin/2 - phoneItemFrame - dim;
                QRect  vSubMenuRect = visualRect(opt->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText,
                                           newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, p, widget);
            }
        }
        break;
    case CE_MenuEmptyArea:
        p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        break;
    case CE_ScrollBarAddPage:
        if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            QRect r(opt->rect);
            bool horz = (sb->orientation == Qt::Horizontal);
            int origextent = horz ? r.height() : r.width();
            int extent = (origextent == 0) ? 0 : qMax(3, origextent/5);
            int dif = (extent == 0) ? 0 : ((origextent-extent)%2 == 1) ? (origextent-extent)/2+1 : (origextent-extent)/2;
            horz ? r.setRect(r.x(),dif,r.width(),extent) : r.setRect(dif,r.y(),extent,r.height());
            horz ? r.adjust(-1,0,-1,0) : r.adjust(0,-1,0,-1);

            p->setPen(opt->palette.shadow().color());
            p->setBrush(opt->palette.shadow());

            QPainterPath path;
            if (horz) {
                path.moveTo(r.left(), r.top());
                path.lineTo(r.right()-extent, r.top());
                path.arcTo(r.right()-extent, r.top(), extent, extent-1, 90, -180);
                path.lineTo(r.left(), r.bottom());
                path.closeSubpath();
            } else {
                path.moveTo(r.left(), r.top());
                path.lineTo(r.left(), r.bottom()-extent);
                path.arcTo(r.left(), r.bottom()-extent, extent-1, extent, 180, 180);
                path.lineTo(r.right(), r.top());
                path.closeSubpath();
            }
            p->drawPath(path);
        }
        break;
    case CE_ScrollBarSubPage:
        if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            QRect r(opt->rect);
            bool horz = (sb->orientation == Qt::Horizontal);
            int origextent = horz ? r.height() : r.width();
            int extent = (origextent == 0) ? 0 : qMax(3, origextent/5);
            int dif = (extent == 0) ? 0 : ((origextent-extent)%2 == 1) ? (origextent-extent)/2+1 : (origextent-extent)/2;
            horz ? r.setRect(r.x(),dif,r.width(),extent) : r.setRect(dif,r.y(),extent,r.height());
            horz ? r.adjust(0,0,1,0) : r.adjust(0,0,0,2);

            p->setPen(opt->palette.shadow().color());
            p->setBrush(opt->palette.shadow());

            QPainterPath path;
            if (horz) {
                path.moveTo(r.right(), r.top());
                path.lineTo(r.left()+extent, r.top());
                path.arcTo(r.left(), r.top(), extent, extent-1, 90, 180);
                path.lineTo(r.right(), r.bottom());
                path.closeSubpath();
            } else {
                path.moveTo(r.left(), r.bottom());
                path.lineTo(r.left(), r.top()+extent);
                path.arcTo(r.left(), r.top(), extent-1, extent, 180, -180);
                path.lineTo(r.right(), r.bottom());
                path.closeSubpath();
            }
            p->drawPath(path);
        }
        break;
    case CE_ScrollBarSlider:
        if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            QRect r(opt->rect);
            bool horz = (sb->orientation == Qt::Horizontal);

            static const int bodySize = 8;
            QSize cachedSize;
            int minSlider = pixelMetric(PM_ScrollBarSliderMin, opt);
            QString key;
            if (horz) {
                cachedSize = QSize(minSlider + bodySize, opt->rect.height());
                r.setWidth(cachedSize.width());
                key = uniqueName("sb-h", opt, QRect(QPoint(0,0),cachedSize));
            } else {
                cachedSize = QSize(opt->rect.width(), minSlider + bodySize);
                r.setHeight(cachedSize.height());
                key = uniqueName("sb-v", opt, QRect(QPoint(0,0),cachedSize));
            }

            QPixmap pm;
            if (!QGlobalPixmapCache::find(key, pm)) {
                pm = QPixmap(cachedSize);
                pm.fill(Qt::transparent);
                r.translate(-opt->rect.topLeft());
                horz ? r.adjust(1,0,-2,-2) : r.adjust(0,1,-2,-2);
                QPainter ppm(&pm);

                //shadow    //TODO: change to path (because of transparency issues)
                QRect shadowRect = r;
                shadowRect.adjust(1,1,1,1);
                ppm.setBrush(Qt::NoBrush);
                ppm.setPen(opt->palette.dark().color());
                ppm.setRenderHint(QPainter::Antialiasing);
                drawRoundRect(&ppm, shadowRect, 5, 10);
                ppm.setRenderHint(QPainter::Antialiasing, false);

                //main portion
                QColor c = opt->palette.color(QPalette::Highlight);
                QLinearGradient bgg(r.x(), r.y(), horz ? r.x() : r.right(),
                                horz ? r.bottom() : r.y());
                bgg.setColorAt(0.3f, c.lighter(115));
                bgg.setColorAt(1.0f, c);
                ppm.setBrush(bgg);

                ppm.setPen(c.darker(120));
                drawRoundRect(&ppm, r, 5, 10);

                //shine
                ppm.setRenderHint(QPainter::Antialiasing);
                ppm.setPen(Qt::NoPen);
                ppm.setBrush(QColor(255,255,255,60));
                drawRoundRect(&ppm, QRect(r.left()+2,r.top()+2,5,2), 3, 1);

                //extra highlights
                if (r.height() < 50)
                    ppm.setRenderHint(QPainter::Antialiasing, false);
                /*
                QLinearGradient penGrad(r.left(), r.y(), r.left(), r.bottom());
                penGrad.setColorAt(0.5f, c);
                penGrad.setColorAt(0.9f, c.lighter(120));

                ppm.setPen(QPen(penGrad,0));
                */
                ppm.setPen(c.darker(110));
                ppm.setBrush(Qt::NoBrush);

                QPainterPath path;
                path.moveTo(r.right(), r.top()+5);
                path.lineTo(r.right(), r.bottom()-5);
                path.arcTo(r.right()-5, r.bottom()-5, 5, 5, 0, -90);
                path.lineTo(r.left()+2, r.bottom());
                ppm.drawPath(path);

                ppm.setPen(c.lighter(105));
                QPainterPath path2;
                path2.moveTo(r.right()-2, r.top()+1);
                path2.lineTo(r.left()+6, r.top()+1);
                path2.arcTo(r.left()+1, r.top()+1, 5, 5, 90, 90);
                path2.lineTo(r.left()+1, r.bottom()-6);
                ppm.drawPath(path2);
                ppm.end();
                QGlobalPixmapCache::insert(key, pm);
            }
            d->drawStretch(p, opt->rect, pm, minSlider/2, minSlider/2+bodySize, sb->orientation);
        }
        break;
    case CE_RadioButton:
    case CE_CheckBox:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            bool isRadio = (ce == CE_RadioButton);
            if (btn->state & State_HasFocus && !styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, btn, widget)) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = subElementRect(isRadio ? SE_RadioButtonFocusRect
                    : SE_CheckBoxFocusRect, btn, widget);
                drawPrimitive((QStyle::PrimitiveElement)PE_FilledFocusRect, &fropt, p, widget);
            }
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(isRadio ? SE_RadioButtonIndicator
                : SE_CheckBoxIndicator, btn, widget);
            drawPrimitive(isRadio ? PE_IndicatorRadioButton : PE_IndicatorCheckBox,
                          &subopt, p, widget);
            subopt.rect = subElementRect(isRadio ? SE_RadioButtonContents
                : SE_CheckBoxContents, btn, widget);
            drawControl(isRadio ? CE_RadioButtonLabel : CE_CheckBoxLabel, &subopt, p, widget);
        }
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            uint alignment = visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);

            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            QRect textRect = btn->rect;
            if (!btn->icon.isNull()) {
                pix = btn->icon.pixmap(btn->iconSize, btn->state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
                drawItemPixmap(p, btn->rect, alignment, pix);
                if (btn->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                else
                    textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
            }
            if (!btn->text.isEmpty()){
                drawItemText(p, textRect, alignment | Qt::TextShowMnemonic,
                             btn->palette, btn->state & State_Enabled, btn->text,
                             (btn->state & State_HasFocus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::WindowText);
            }
        }
        break;
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            drawControl(CE_PushButtonBevel, btn, p, widget);
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            drawControl(CE_PushButtonLabel, &subopt, p, widget);
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QRect ir = btn->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                tf |= Qt::TextHideMnemonic;

            if (btn->state & (State_On | State_Sunken))
                ir.translate(pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                                pixelMetric(PM_ButtonShiftVertical, opt, widget));
            if (!btn->icon.isNull()) {
                QIcon::Mode mode = btn->state & State_Enabled ? QIcon::Normal
                    : QIcon::Disabled;
                if (mode == QIcon::Normal && btn->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (btn->state & State_On)
                    state = QIcon::On;
                QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //Center the icon if there is no text

                QPoint point;
                if (btn->text.isEmpty()) {
                    point = QPoint(ir.x() + ir.width() / 2 - pixw / 2,
                                    ir.y() + ir.height() / 2 - pixh / 2);
                } else {
                    point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);
                }
                if (btn->direction == Qt::RightToLeft)
                    point.rx() += pixw;

                if ((btn->state & (State_On | State_Sunken)) && btn->direction == Qt::RightToLeft)
                    point.rx() -= pixelMetric(PM_ButtonShiftHorizontal, opt, widget) * 2;

                p->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);

                if (btn->direction == Qt::RightToLeft)
                    ir.translate(-4, 0);
                else
                    ir.translate(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!btn->text.isEmpty())
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            drawItemText(p, ir, tf, btn->palette, (btn->state & State_Enabled), btn->text,
                         (btn->state & State_HasFocus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText);
        }
        break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if ((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty()
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                drawItemText(p, rect, alignment, toolbutton->palette,
                                opt->state & State_Enabled, toolbutton->text,
                                (opt->state & State_HasFocus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText);
                } else {
                    QPixmap pm;
                    QSize pmSize = toolbutton->iconSize;
                    if (!toolbutton->icon.isNull()) {
                        QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                        QIcon::Mode mode;
                        if (!(toolbutton->state & State_Enabled))
                            mode = QIcon::Disabled;
                        else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                            mode = QIcon::Active;
                        else
                            mode = QIcon::Normal;
                        pm = toolbutton->icon.pixmap(toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                mode, state);
                        pmSize = pm.size();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    p->setFont(toolbutton->font);
                    QRect pr = rect,
                    tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!styleHint(SH_UnderlineShortcut, opt, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 6);

                        tr.adjust(0, pr.bottom(), 0, -3);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(this, toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 8);
                        tr.adjust(pr.right(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(this, toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    drawItemText(p, tr, alignment, toolbutton->palette,
                                toolbutton->state & State_Enabled, toolbutton->text,
                                (toolbutton->state & State_HasFocus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText : QPalette::ButtonText);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(this, toolbutton, rect, p, widget);
                    } else {
                        drawItemPixmap(p, rect, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        break;
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTabV2 tabV2(*tab);
            QRect r = tabV2.rect;
            bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                || tabV2.shape == QTabBar::RoundedWest
                                || tabV2.shape == QTabBar::TriangularEast
                                || tabV2.shape == QTabBar::TriangularWest;
            bool bottomTabs = tabV2.shape == QTabBar::RoundedSouth
                                || tabV2.shape == QTabBar::TriangularSouth;
            bool selected = tabV2.state & State_Selected;
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            bool lastTab = (tab->direction == Qt::LeftToRight &&
                            tab->position == QStyleOptionTab::End) ||
                           (tab->direction == Qt::RightToLeft &&
                            tab->position == QStyleOptionTab::Beginning);
            int overlap = (onlyOne || lastTab) ? 0 : pixelMetric(PM_TabBarTabOverlap, opt, widget);
            int verticalOffset = selected ? 0 : pixelMetric(PM_TabBarTabShiftVertical, opt, widget);

            if (verticalTabs || bottomTabs) {   //use Qtopia style for "non-standard" tabs
                QtopiaStyle::drawControl(ce, opt, p, widget);
                break;
            }

            r.adjust(0,verticalOffset,overlap,0);

            //shadow (TODO: simple shadow doesn't look good. investigate complex shadow)
            /*{
                p->setBrush(Qt::NoBrush);
                p->setPen(opt->palette.dark().color());
                p->setRenderHint(QPainter::Antialiasing);

                QPainterPath path;
                path.moveTo(r.right()-19, r.y());
                path.quadTo(r.right()-4, r.y(), r.right()+1, selected ? r.bottom() + 1 : r.bottom());
                p->drawPath(path);
            }*/

            BEGIN_PHONESTYLE_PIXMAPCACHE(r, QString::fromLatin1("tab-%1-%2").arg(selected).arg(r.left()))

            //tab
            {
                QColor bg = opt->palette.color(selected ? QPalette::Mid : QPalette::Button);

                if (selected) {
                    QPalette pal(opt->palette);
                    QColor windowCol = pal.color(QPalette::Window);
                    windowCol.setAlpha(255);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
                    pal.setBrush(QPalette::Window, QBrush(windowCol, d->bgExport->background()));
#endif
                    if (widget)
                        p->setBrushOrigin(-widget->mapToGlobal(QPoint(0,0)));
                    p->setBrush(pal.window());
                    p->setPen(opt->palette.color(QPalette::Light));
                } else {
                    bg.setAlpha(204);
                    QLinearGradient bgg(r.x(), r.y(), r.right(), r.bottom());
                    bgg.setColorAt(0.0f, bg.darker(120));
                    bgg.setColorAt(0.4f, bg);
                    p->setBrush(bgg);
                    bg.setAlpha(255);
                    p->setPen(bg.darker(120));
                }

                //TODO: cache gradient
                p->setRenderHint(QPainter::Antialiasing);

                QPainterPath path;
                path.moveTo(r.left(), selected ? r.bottom() + 1 : r.bottom()-1);
                path.lineTo(r.left(), r.top()+5);
                path.arcTo(r.left(), r.y(), 10, 10,  180, -90);
                path.lineTo(r.right()-20, r.y());
                path.quadTo(r.right()-5, r.y(), r.right(), selected ? r.bottom() + 1 : r.bottom()-1);
                p->drawPath(path);
            }

            //shine
            if (1) {
                p->setRenderHint(QPainter::Antialiasing);
                p->setBrush(QColor(255,255,255,40));
                p->setPen(Qt::NoPen);
                QPainterPath path;
                path.moveTo(r.right()-9, r.y()+r.height()*1/4);
                path.lineTo(r.right()-9, r.y()+5);
                path.arcTo(r.right()-19, r.y(), 10, 10,  0, 90);
                path.lineTo(r.left()+5, r.y());
                path.arcTo(r.left(), r.y(), 10, 10,  90, 90);
                path.lineTo(r.left(), r.bottom());
                path.lineTo(r.left()+r.width()*1/4, r.bottom());
                path.cubicTo(r.left()+r.width()*1/2, r.bottom(), r.left()+r.width()*5/8, r.y()+r.height()*1/4, r.right()-9, r.y()+r.height()*1/4);
                p->drawPath(path);
            }

            END_PHONESTYLE_PIXMAPCACHE
        }
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (const QTabBar *tb = qobject_cast<const QTabBar*>(widget)) {
                QStyleOptionTabV2 tabV2(*tab);
                QRect tr = tabV2.rect;
                bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                    || tabV2.shape == QTabBar::RoundedWest
                                    || tabV2.shape == QTabBar::TriangularEast
                                    || tabV2.shape == QTabBar::TriangularWest;
                bool lastTab = (tab->direction == Qt::LeftToRight &&
                                tab->position == QStyleOptionTab::End) ||
                              (tab->direction == Qt::RightToLeft &&
                                tab->position == QStyleOptionTab::Beginning);
                bool selected = tabV2.state & State_Selected;
                bool twoline = (!tabV2.icon.isNull() && !tabV2.text.isEmpty()) ? true : false;

                // consider width added in CT_TabBarTab case for end tabs
                if (lastTab)
                    tr.adjust(0, 0, -pixelMetric(PM_TabBarTabOverlap, opt, widget), 0);

                if (verticalTabs) {
                    p->save();
                    int newX, newY, newRot;
                    if (tabV2.shape == QTabBar::RoundedEast || tabV2.shape == QTabBar::TriangularEast) {
                        newX = tr.width();
                        newY = tr.y();
                        newRot = 90;
                    } else {
                        newX = 0;
                        newY = tr.y() + tr.height();
                        newRot = -90;
                    }
                    tr.setRect(0, 0, tr.height(), tr.width());
                    QMatrix m;
                    m.translate(newX, newY);
                    m.rotate(newRot);
                    p->setMatrix(m, true);
                }
                tr.adjust(0, 0, pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget),
                                pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget));

                if (selected)
                {
                    tr.setBottom(tr.bottom() - pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab,
                                                        widget));
                    tr.setRight(tr.right() - pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab,
                                                        widget));
                }

                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;
                if (!tabV2.icon.isNull()) {
                    QSize iconSize = tabV2.iconSize;
                    if (!iconSize.isValid()) {
                        int iconExtent = pixelMetric(PM_SmallIconSize);
                        iconSize = QSize(iconExtent, iconExtent);
                    }
                    QPixmap tabIcon = tabV2.icon.pixmap(iconSize,
                                                        (tabV2.state & State_Enabled) ? QIcon::Normal
                                                                                    : QIcon::Disabled);
                    int x = (twoline ? (tr.center().x() - tabIcon.width() / 2) : (tr.left() + 6));
                    p->drawPixmap(x, tr.center().y() - tabIcon.height() / 2, tabIcon);
                    if (twoline) {
                        tr.setLeft(0);
                        tr.setRight(tb->geometry().width());
                        tr.adjust(0, tr.height(), 0, tabV2.fontMetrics.height() + 4);
                    } else
                        tr.setLeft(tr.left() + iconSize.width() + 4);
                }

                if (!twoline || selected) {
                    // for tab text in twoline mode, need to elide text to full
                    // tabbar width (tab->text is only elided to width of this tab)
                    QString text = (twoline) ?
                        (tabV2.fontMetrics.elidedText(tb->tabText(tb->currentIndex()), tb->elideMode(), tr.width())) :
                        tab->text;
                    drawItemText(p, tr, alignment, tab->palette, tab->state & State_Enabled, text,
                                tab->state & State_Selected ? QPalette::WindowText : QPalette::WindowText);
                }
                if (verticalTabs)
                    p->restore();
            }
        }
        break;
    case CE_ProgressBarGroove: {
        QBrush oldBrush = p->brush();
        QPen oldPen = p->pen();
        p->setBrush(opt->palette.base());
        p->setPen(opt->palette.mid().color());
        QRect r = opt->rect;
        r.adjust(0,0,-1,-1);
        drawRoundRect(p, r, 8, 8);
        p->setBrush(oldBrush);
        p->setPen(oldPen);
        break; }
    case CE_MenuScroller: {
        // Left & right margins of scroll menu aren't painted by
        // CE_MenuEmptyArea, so paint them here, or else they show up under
        // a transparent scroll menu.
        // If scroll menus are used a lot, should probably optimize to not use
        // QPainterPath every time.
        int menuHMargin = pixelMetric(PM_MenuHMargin);
        QPainterPath scrollerWithoutHMargins;
        scrollerWithoutHMargins.addRect(opt->rect.adjusted(menuHMargin, 0, -menuHMargin, 0));
        QPainterPath scrollerPath;
        scrollerPath.addRect(opt->rect); // if menu corners were more rounded, this might have to be round rect instead
        p->fillPath(scrollerPath.subtracted(scrollerWithoutHMargins), opt->palette.brush(QPalette::Button));

        p->fillRect(opt->rect, opt->palette.button());
        QStyleOption arrowOpt = *opt;
        arrowOpt.state |= State_Enabled;
        drawPrimitive(((opt->state & State_DownArrow) ? PE_IndicatorArrowDown : PE_IndicatorArrowUp),
                    &arrowOpt, p, widget);
        break; }
    default:
        QtopiaStyle::drawControl(ce, opt, p, widget);
        break;
    }
}

/*!
    \reimp
*/
void QPhoneStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                      QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            bool extendedFocus = styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, cmb, widget);
            if (cmb->subControls & SC_ComboBoxArrow) {
                State flags = State_None;

                QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxFrame, widget);
                QBrush editBrush;
                if (cmb->state & State_Enabled && cmb->state & State_HasEditFocus) {
                    editBrush = cmb->palette.brush(QPalette::Base);
                } else {
                    editBrush = cmb->palette.brush(QPalette::Window);
                    QColor col = editBrush.color();
                    col.setAlpha(0);
                    editBrush.setColor(col);
                }
                p->fillRect(re, editBrush);
                if (cmb->frame && cmb->state & State_HasFocus && !extendedFocus && !Qtopia::mousePreferred()) {
                    QStyleOptionFocusRect fropt;
                    fropt.state = State_KeyboardFocusChange;
                    fropt.rect = opt->rect;
                    fropt.palette = opt->palette;
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                } else if (!(cmb->state & State_HasEditFocus)) {
                    QPen oldPen = p->pen();
                    QPen pen;
                    QColor c = opt->palette.color((cmb->state & State_HasFocus && extendedFocus && !cmb->editable && !Qtopia::mousePreferred())
                                                  ? QPalette::HighlightedText : QPalette::WindowText);
                    c.setAlpha(48);
                    pen.setColor(c);
                    p->setPen(pen);
                    p->drawLine(re.x(), re.bottom(), re.right(), re.bottom());
                    p->setPen(oldPen);
                }

                QRect ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                /*if (cmb->activeSubControls == SC_ComboBoxArrow) {
                    p->setPen(cmb->palette.dark().color());
                    p->setBrush(cmb->palette.brush(QPalette::Button));
                    p->drawRect(ar.adjusted(0,0,-1,-1));

                    flags |= State_Sunken;
                }*/   //never draw 'pressed' state for arrow

                if (opt->state & State_Enabled)
                    flags |= State_Enabled;

                if (const QDateTimeEdit *dte = qobject_cast<const QDateTimeEdit*>(widget)) {    //draw a calendar instead of a drop-down arrow
                    int dim = ar.width();
                    int x = ar.left();
                    int y = ar.top() + (ar.height() - dim)/2;
                    if (ar.height() < ar.width()) {
                        dim = ar.height();
                        x = ar.left() + (ar.width() - dim)/2;
                        y = ar.top();
                    }
                    QPixmap monthIcon = QIcon(":icon/month").pixmap(dim,dim, !(opt->state & State_Enabled) ? QIcon::Disabled : QIcon::Normal);
                    if (dte->currentSection() == QDateTimeEdit::NoSection) {
                        QImage img = monthIcon.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
                        QPainter painter(&img);
                        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
                        painter.fillRect(0, 0, img.width(), img.height(), opt->palette.brush(QPalette::Highlight));
                        painter.end();
                        monthIcon = QPixmap::fromImage(img);
                    }
                    p->drawPixmap(x, y, monthIcon);
                } else {
                    ar.adjust(2, 2, -2, -2);
                    QPalette pal = cmb->palette;
                    if (extendedFocus && cmb->state & State_HasFocus)
                        pal.setColor(QPalette::Normal, QPalette::ButtonText,
                                     pal.color(QPalette::Normal, cmb->editable ? QPalette::Text : QPalette::HighlightedText));  //TODO: edit focus for editable combos should be different?
                    QStyleOption arrowOpt(0);
                    arrowOpt.rect = ar;
                    arrowOpt.palette = pal;
                    arrowOpt.state = flags;
                    drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);
                }
            }
            if (cmb->subControls & SC_ComboBoxEditField) {
                QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxEditField, widget);

                if (cmb->state & State_HasFocus && !Qtopia::mousePreferred()) {
                    p->setPen(cmb->palette.highlightedText().color());
                    p->setBackground(cmb->palette.highlight());
                } else {
                    p->setPen(cmb->palette.text().color());
                    p->setBackground(cmb->palette.background());
                }

                if (cmb->state & State_HasFocus && !cmb->editable && !extendedFocus) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*cmb);
                    focus.rect = subElementRect(SE_ComboBoxFocusRect, cmb, widget);
                    focus.state |= State_FocusAtBorder;
                    focus.backgroundColor = cmb->palette.highlight().color();
                    drawPrimitive((QStyle::PrimitiveElement)PE_FilledFocusRect, &focus, p, widget);
                }
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;

            bool extendedFocus = styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, sb, widget);

            if (sb->frame && (sb->subControls & SC_SpinBoxFrame)) {
                QRect r = subControlRect(CC_SpinBox, sb, SC_SpinBoxFrame, widget);
                if (sb->state & State_HasFocus && !extendedFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.state = State_KeyboardFocusChange;
                    fropt.rect = opt->rect;
                    fropt.palette = opt->palette;
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                } else if (!(sb->state & State_HasEditFocus)) {
                    QPen oldPen = p->pen();
                    QPen pen;
                    QColor c = opt->palette.color(QPalette::WindowText);
                    c.setAlpha(48);
                    pen.setColor(c);
                    p->setPen(pen);
                    p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
                    p->setPen(oldPen);
                }
            }

            if (sb->subControls & SC_SpinBoxUp && (sb->state & State_HasFocus || Qtopia::mousePreferred())) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) && (sb->state & State_HasEditFocus || Qtopia::mousePreferred())) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                if (!(sb->state & State_HasEditFocus) && !Qtopia::mousePreferred()) {
                    pal2.setBrush(pal2.currentColorGroup(), QPalette::ButtonText, pal2.highlight());
                }
                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);

                copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
//                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.adjust(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->subControls & SC_SpinBoxDown && (sb->state & State_HasFocus || Qtopia::mousePreferred())) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) && (sb->state & State_HasEditFocus || Qtopia::mousePreferred())) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                if (!(sb->state & State_HasEditFocus) && !Qtopia::mousePreferred()) {
                    pal2.setBrush(pal2.currentColorGroup(), QPalette::ButtonText, pal2.highlight());
                }
                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);

                copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
//                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.adjust(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }
        }
        break;
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            //int thickness  = pixelMetric(PM_SliderControlThickness, slider, widget);
            int len = pixelMetric(PM_SliderLength, slider, widget);
            QRect groove = subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
            QRect handle = subControlRect(CC_Slider, slider, SC_SliderHandle, widget);
            bool focus = opt->state & State_HasFocus;
            bool editfocus = opt->state & State_HasEditFocus;

            if (focus && !editfocus && !styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, slider, widget)
                && !Qtopia::mousePreferred()) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*slider);
                fropt.rect = subElementRect(SE_SliderFocusRect, slider, widget);
                drawPrimitive((QStyle::PrimitiveElement)PE_FilledFocusRect, &fropt, p, widget);
            }

            p->setRenderHint(QPainter::Antialiasing);

            if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
                if (slider->orientation == Qt::Horizontal) {
                    int adjust = qRound((groove.height() - sliderGrooveHeight)/2.0);
                    groove.adjust(0,adjust,-1,-adjust);

                    //outside
                    QLinearGradient bgg(groove.x(), groove.y(), groove.x(), groove.bottom());
                    QColor c1 = opt->palette.color(QPalette::Shadow);
                    QColor c2 = opt->palette.color(QPalette::Button);
                    c1.setAlpha(100);
                    c2.setAlpha(100);
                    bgg.setColorAt(0.45f, c1);
                    bgg.setColorAt(1.0f, c2);
                    p->setBrush(bgg);
                    p->setPen(Qt::NoPen);
                    drawRoundRect(p, groove, 10, 10);

                    //inside
                    int insideadjust = qRound(groove.height()/2.5);
                    QRect r = groove;
                    r.adjust(4,insideadjust,-4,-insideadjust);
                    QColor c = opt->palette.color(QPalette::Dark);
                    c.setAlpha(200);
                    p->setBrush(c);
                    p->setPen(Qt::NoPen);
                    drawRoundRect(p, r, 20, 2);

                    /*p->setBrush(Qt::NoBrush);
                    c = opt->palette.color(QPalette::Shadow);
                    c.setAlpha(200);
                    p->setPen(c);
                    QPainterPath path;
                    path.moveTo(r.left(), r.top()+1);
                    path.arcTo(r.left(), r.top()+1, 10, 1, 180, 90);
                    path.lineTo(r.right()-10, r.bottom()+1);
                    path.arcTo(r.right()-10, r.top()+1, 10, 1, 270, 90);
                    p->drawPath(path);*/    //extra shadow, still useful for larger sizes
                } else {
                    //TODO: vertical
                }
            }

            if (slider->subControls & SC_SliderTickmarks) {
                p->setRenderHint(QPainter::Antialiasing, false);
                //int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
                int ticks = slider->tickPosition;
                int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (!interval)
                    interval = 1;
                int fudge = len / 2;
                int pos;
                p->setPen(slider->palette.shadow().color());
                int v = slider->minimum;
                int adjust = qRound(groove.height()/2.5) > 3 ? 2 : 1; //similar to insideadjust above. merge?
                while (v <= slider->maximum) {
                    pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          v, available) + fudge;
                    if (slider->orientation == Qt::Horizontal) {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(pos, groove.top(), pos, groove.top()+adjust);
                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(pos, groove.bottom(), pos,
                                        groove.bottom()-adjust);
                    } else {
                        //TODO: vertical
                        /*if (ticks & QSlider::TicksAbove)
                            p->drawLine(0, pos, tickOffset - 2, pos);
                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(tickOffset + thickness + 1, pos,
                                        tickOffset + thickness + 1 + available - 2, pos);*/
                    }
                    // in the case where maximum is max int
                    int nextInterval = v + interval;
                    if (nextInterval < v)
                        break;
                    v = nextInterval;
                }
            }

            if (slider->subControls & SC_SliderHandle && handle.isValid()) {
                p->setRenderHint(QPainter::Antialiasing);
                QRect r = handle;
                r.adjust(1,1,-2,-2);

                //shadow
                QRect shadowRect = r;
                shadowRect.adjust(1,1,1,1);
                p->setBrush(Qt::NoBrush);
                p->setPen(opt->palette.dark().color());
                QPainterPath path;
                path.moveTo(shadowRect.center().x(), shadowRect.top());
                path.arcTo(shadowRect.x(), shadowRect.y(), shadowRect.width(), shadowRect.height(), 90, -270);
                p->drawPath(path);
                //p->drawEllipse(shadowRect);   //faster, but not as good since ball is transparent

                //ball
                QColor c = opt->palette.color(editfocus && !Qtopia::mousePreferred() ? QPalette::Highlight : QPalette::Button);
                QLinearGradient bgg(r.x(), r.y(), r.right(), r.bottom());
                bgg.setColorAt(0.4f, c);
                bgg.setColorAt(1.0f, c.lighter(120));
                p->setBrush(bgg);
                p->setPen(c.darker(120));
                p->drawEllipse(r);

                //shine TODO: make this scale better
                p->setBrush(QColor(255,255,255,110));
                p->setPen(Qt::NoPen);
                QRect shine1(0, 0, 4, 2);
                p->save();
                p->translate(r.left() + (r.width() > 10 ? 3 : 1), r.top()+4);
                p->rotate(-45);
                p->drawEllipse(shine1);
                p->restore();
            }
        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect button, menuarea;
            button = subControlRect(cc, toolbutton, SC_ToolButton, widget);
            menuarea = subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget);

            State bflags = toolbutton->state;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;

            if (toolbutton->activeSubControls & SC_ToolButton)
                bflags |= State_Sunken;
            if (toolbutton->activeSubControls & SC_ToolButtonMenu)
                mflags |= State_Sunken;

            QStyleOption tool(0);
            tool.palette = toolbutton->palette;
            if (toolbutton->subControls & SC_ToolButton) {
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                }
            }

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (State_Sunken | State_On | State_Raised))
                    drawPrimitive(PE_IndicatorButtonDropDown, &tool, p, widget);
                drawPrimitive(PE_IndicatorArrowDown, &tool, p, widget);
            }

            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            drawControl(CE_ToolButtonLabel, &label, p, widget);
        }
        break;
    case CC_GroupBox:
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
            // Draw frame
            QRect textRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxCheckBox, widget);
            if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                QStyleOptionFrameV2 frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = subControlRect(CC_GroupBox, opt, SC_GroupBoxFrame, widget);
                p->save();
                QRegion region(groupBox->rect);
                if (!groupBox->text.isEmpty()) {
                    bool ltr = groupBox->direction == Qt::LeftToRight;
                    QRect finalRect;
                    if (groupBox->subControls & QStyle::SC_GroupBoxCheckBox) {
                        finalRect = checkBoxRect.united(textRect);
                        finalRect.adjust(ltr ? -4 : 0, 0, ltr ? 0 : 4, 0);
                    } else {
                        finalRect = textRect;
                    }
                    region -= QRect(finalRect.x(), frame.rect.y(), finalRect.width(), 1);   //only clip border (since we have background)
                }
                p->setClipRegion(region);
                drawPrimitive(PE_FrameGroupBox, &frame, p, widget);
                p->restore();
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                QColor textColor = groupBox->textColor;
                if (textColor.isValid())
                    p->setPen(textColor);
                int alignment = int(groupBox->textAlignment);
                if (!styleHint(QStyle::SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;

                if (groupBox->state & State_HasFocus && !styleHint((QStyle::StyleHint)SH_ExtendedFocusHighlight, groupBox, widget)) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect;
                    drawPrimitive((QStyle::PrimitiveElement)PE_FilledFocusRect, &fropt, p, widget);
                }

                drawItemText(p, textRect,  Qt::TextShowMnemonic | Qt::AlignHCenter | alignment,
                             groupBox->palette, groupBox->state & State_Enabled, groupBox->text,
                             (groupBox->state & State_HasFocus && !Qtopia::mousePreferred()) ? QPalette::HighlightedText
                             : textColor.isValid() ? QPalette::NoRole : QPalette::WindowText);
            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                drawPrimitive(PE_IndicatorCheckBox, &box, p, widget);
            }
        }
        break;
    default:
        QtopiaStyle::drawComplexControl(cc, opt, p, widget);
    }
}

/*!
    \reimp
*/
bool QPhoneStyle::event(QEvent *e)
{
    if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
        QWidget *focusWidget = QApplication::focusWidget();
        if (focusWidget && !QApplication::activePopupWidget()) {
            QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(focusWidget);
            if (aiv) {
                QPalette::ColorRole role = aiv->viewport()->backgroundRole();
                if (e->type() == QEvent::EnterEditFocus) {
                    if (role != QPalette::Base)
                        aiv->viewport()->setBackgroundRole(QPalette::Base);
                } else if (!isSingleFocusWidget(focusWidget)) {
                    // If a single focus widget is losing edit focus
                    // then the window is probably being closed, otherwise
                    // change the role.
                    if (role != QPalette::Window)
                        aiv->viewport()->setBackgroundRole(QPalette::Window);
                }
            }

            if ( qobject_cast<QLineEdit*>(focusWidget) ||
                  qobject_cast<QAbstractSpinBox*>(focusWidget) ||
                  qobject_cast<QSlider*>(focusWidget) ||
                  qobject_cast<QComboBox*>(focusWidget) ||
                  qobject_cast<QTableView*>(focusWidget) ) {
                focusWidget->update();
            }

            QPoint pos = focusWidget->mapTo(focusWidget->window(), QPoint(0,0));
            if (pos.y() == 0 && pos.x() < 20)
                d->updateDecoration();
        }
    }
    return QtopiaStyle::event(e);
}

/*!
    \reimp
*/
void QPhoneStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    QtopiaStyle::drawItemText(painter, rect, alignment, pal, enabled, text, textRole);
}

/*!
    \reimp
*/
int QPhoneStyle::styleHint(StyleHint stylehint, const QStyleOption *option,
                           const QWidget *widget, QStyleHintReturn* returnData) const
{
    int ret = 0;
    switch (stylehint) {
        case SH_ComboBox_PopupFrameStyle:
            ret = QFrame::StyledPanel | QFrame::Plain;
            break;
        case SH_TabBar_Alignment:
            ret = Qt::AlignCenter;
            break;
        case SH_TextControl_FocusIndicatorTextCharFormat:
            ret = true;
            if (QStyleHintReturnVariant *vret = qstyleoption_cast<QStyleHintReturnVariant*>(returnData)) {
                QTextCharFormat fmt;
                fmt.setBackground(option->palette.highlight());
                fmt.setForeground(option->palette.highlightedText());
                vret->variant = fmt;
            }
            break;
        case SH_ExtendedFocusHighlight:
            ret = gConfig()->value( "Style/ExtendedFocusHighlight", 0 ).toInt();
            break;
        case SH_FormLayoutWrapPolicy: {
            QString str = gConfig()->value( "Style/FormStyle", "QtopiaDefaultStyle" ).toString();
            if (str == "QtopiaTwoLineStyle")
                ret = QFormLayout::WrapAllRows;
            else
                ret = QFormLayout::WrapLongRows;
            break; }
        case SH_FormLayoutFieldGrowthPolicy:
            return QFormLayout::AllNonFixedFieldsGrow;
        case SH_FormLayoutFormAlignment:
            return Qt::AlignLeft | Qt::AlignTop;
        case SH_FormLayoutLabelAlignment:
            return Qt::AlignRight;
        case SH_PopupShadows:
            ret = gConfig()->value( "Style/PopupShadows", 0 ).toInt();
            break;
        case SH_HideMenuIcons:
            ret = gConfig()->value( "Style/HideMenuIcons", 0 ).toInt();
            break;
        case SH_FullWidthMenu:
            ret = gConfig()->value( "Style/FullWidthMenu", 0 ).toInt();
            break;
        case SH_ScrollbarLineStepButtons:
            ret = 0;
            break;
        case SH_EtchDisabledText:
            ret = 0;
            break;
        default:
            ret = QtopiaStyle::styleHint(stylehint, option, widget, returnData);
    }

    return ret;
}

#include "qphonestyle.moc"
