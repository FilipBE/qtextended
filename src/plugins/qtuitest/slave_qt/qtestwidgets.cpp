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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include <qtestwidgets.h>
#include <qtestslaveglobal.h>

#include <QtGui>

#ifdef QTOPIA_TARGET
# include <QIconSelector>
# include <qtopialog.h>
#else
# define qLog(A) if (1); else qDebug() << #A
#endif

bool active_test_widget_valid = false;
QActiveTestWidgetData *active_data = 0;
QString const QTestWidget::GetListRegExp = QString("QTUITEST_REGEX_");

uint qHash(QPointer<QObject> o)
{ return qHash((QObject*)o); }

inline QString dehyphenate(QString const& in)
{ return QString(in).remove(QChar(0x00AD)); }

//***********************************************************

QTestWidget::QTestWidget( const QString &signature, QObject *w )
    : QObject()
    , _instance(w)
    , _unsafe_instance(w)
    , _signature(signature)
{
    connect( w, SIGNAL(destroyed()), this, SLOT(onDestroyed()), Qt::DirectConnection );
}

QTestWidget::~QTestWidget()
{


    if (active_data) active_data->removeTestWidget( this );
    QTestWidgets::removeWidget(this);
    active_test_widget_valid = false;
}

QString QTestWidget::signature() const
{
    return _signature;
}

QString QTestWidget::uniqueName() const
{
    if (_instance == 0) return "[]";
    return QString("%1[%2]").arg(_instance->metaObject()->className()).arg((long)(void*)_instance,0,32);
}

QObject* QTestWidget::instance() const
{
    return _instance;
}

void QTestWidget::onDestroyed()
{
    delete this;
}

QWidget* QTestWidget::parentWidget( const QWidget *child ) const
{
    if (!child) return 0;
    QObject *parent = child->parent();
    while (parent) {
        if (parent->inherits("QWidget")) return qobject_cast<QWidget*>(parent);
        parent = parent->parent();
    }
    return 0;
}

int QTestWidget::x() const
{
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(_instance);
    return w ? w->mapToGlobal(QPoint()).x() : 0;
}

int QTestWidget::y() const
{
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(_instance);
    return w ? w->mapToGlobal(QPoint()).y() : 0;
}

int QTestWidget::width() const
{
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(_instance);
    return w ? w->geometry().width() : 0;
}

int QTestWidget::height() const
{
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(_instance);
    return w ? w->geometry().height() : 0;
}

QRegion _q_totalVisibleRegion(QWidget *widget)
{
    QRegion ret;
    if (widget) {
        ret |= widget->visibleRegion();
        foreach (QObject *o, widget->children()) {
            QWidget *w;
            QAction *a;
            if ( (w = qobject_cast<QWidget*>(o)) != 0 ) {
                QRegion r = _q_totalVisibleRegion(w);
                /* w->mapTo(widget, QPoint(0,0)) does not always work as expected. */
                r.translate(widget->mapFromGlobal(w->mapToGlobal(QPoint(0,0))));
                ret |= r;
            } else if ( (a = qobject_cast<QAction*>(o)) != 0 ) {
                if ( a->menu() != widget && a->menu() ) {
                    QRegion r = _q_totalVisibleRegion( a->menu() );
                    r.translate(widget->mapFromGlobal(a->menu()->mapToGlobal(QPoint(0,0))));
                    ret |= r;
                }
            }
        }
        /* Another special case since menus aren't children*/
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        if (button && button->menu())
            ret |= _q_totalVisibleRegion(button->menu());
    }
    return ret;
}

QRegion QTestWidget::totalVisibleRegion() const
{
    QRegion visibleRegion;
    QWidget *f = qobject_cast<QWidget*>(_instance);
    /* FIXME get rid of special case */
    if (f->inherits("QCalendarView")) f = qobject_cast<QWidget*>(f->parent());
    while (f) {
        visibleRegion |= _q_totalVisibleRegion(f);
        f = f->focusProxy();
    }
    if (_instance->inherits("QCalendarView")) {
        visibleRegion.translate(-qobject_cast<QWidget*>(_instance)->x(), -qobject_cast<QWidget*>(_instance)->y());
    }

    return visibleRegion;
}

QRect mapRectToGlobal(QWidget *widget, QRect const &rect)
{
    return QRect(widget->mapToGlobal(rect.topLeft()), widget->mapToGlobal(rect.bottomRight()));
}

bool getScrollPoints(QWidget *widget, QString const &item, QPoint const &pos, bool &needsHorizontal, QPoint &horizontal, bool &needsVertical, QPoint &vertical)
{
    // Find the scroll area containing relevant scrollbars, which is:
    //  - if we're selecting an item from a widget, it must be the widget itself or a child;
    //  - if we're selecting a widget itself, it must be a parent of the selected widget
    QAbstractScrollArea *sa = (!item.isEmpty() ? qobject_cast<QAbstractScrollArea*>(widget) : 0);
    if (item.isEmpty()) {
        QWidget *p = widget;
        while (!sa && p) {
            p = p->parentWidget();
            sa = qobject_cast<QAbstractScrollArea*>(p);
        }
    } else {
        if (!sa)
            sa = widget->findChild<QAbstractScrollArea*>();
    }
    if (!sa) {
        if (item.isEmpty()) {
            qLog(QtUitest) << "widget" << widget << "isn't in scroll area, can't scroll to it";
        } else {
            qLog(QtUitest) << "widget" << widget << "doesn't contain scroll area, can't scroll to item" << item;
        }
        return false;
    }

    int horiz_scroll = 0;
    int vert_scroll = 0;
    QPoint horizScrollPoint;
    QPoint vertScrollPoint;
    QRect visibleScrollRect = _q_totalVisibleRegion(sa).boundingRect();
    if (visibleScrollRect.isNull()) {
        qLog(QtUitest) << "widget" << widget << "is in scroll area, but the scroll area isn't visible";
        return false;
    }

    qLog(QtUitest) << "widget" << widget << "pos" << pos << "visible scroll rect" << mapRectToGlobal(sa, visibleScrollRect);

    if (pos.x() >= sa->mapToGlobal(QPoint(visibleScrollRect.right(),0)).x()) {
        horiz_scroll = 1;
    } else if (pos.x() <= sa->mapToGlobal(QPoint(visibleScrollRect.left(),0)).x()) {
        horiz_scroll = -1;
    }
    needsHorizontal = !!horiz_scroll;

    if (pos.y() >= sa->mapToGlobal(QPoint(0,visibleScrollRect.bottom())).y()) {
        vert_scroll = 1;
    } else if (pos.y() <= sa->mapToGlobal(QPoint(0,visibleScrollRect.top())).y()) {
        vert_scroll = -1;
    }
    needsVertical = !!vert_scroll;

    do {
        if (!horiz_scroll) break;

        QScrollBar *sb = sa->horizontalScrollBar();
        if (!sb) break;

        QRect r(sb->visibleRegion().boundingRect());
        if (r.isNull()) break;

        horizontal = r.center();
        horizontal.setX((horiz_scroll < 0) ? (r.left()+5) : (r.right()-5));
        horizontal = sb->mapToGlobal(horizontal);
    } while(0);

    do {
        if (!vert_scroll) break;

        QScrollBar *sb = sa->verticalScrollBar();
        if (!sb) break;

        QRect r(sb->visibleRegion().boundingRect());
        if (r.isNull()) break;

        vertical = r.center();
        vertical.setY((vert_scroll < 0) ? (r.top()+5) : (r.bottom()-5));
        vertical = sb->mapToGlobal(vertical);
    } while(0);

    qLog(QtUitest) << "widget" << widget << "scroll info: horiz_scroll" << horiz_scroll << horizontal
             << "vert_scroll" << vert_scroll << vertical;

    return true;
}

QList<QPoint> QTestWidget::navigateByMouse( QString const &item, QPoint const &pos, int *wait, bool *ok )
{
    /* Base implementation: look for a scroll area to manipulate. */
    bool needH, needV;
    QList<QPoint> ret;
    ret << QPoint() << QPoint();
    bool _ok = getScrollPoints(qobject_cast<QWidget*>(_instance), item, pos, needH, ret[0], needV, ret[1]);
    if (ok) *ok = _ok;
    if (wait) *wait = 0;
    ret.removeAll(QPoint());
    return ret;
}

//***********************************************************

/*!
   \internal

   Distinguished test widget to handle unsupported widget classes.  This class
   is deliberately not registered.

   To make the handling of custom widgets easier, if a QTestWidget function is
   called on an unsupported widget, the function will attempt to
*/
class QTestUnsupportedClass : public QTestWidget
{
public:
    QTestUnsupportedClass( const QString &signature, QObject *w )
        : QTestWidget( signature, w ) { }
    virtual ~QTestUnsupportedClass()
        { }
    virtual QString isChecked(bool *ok) {
        bool _ok; QString ret = QTestWidget::isChecked(&_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::isChecked(QTestWidgets::signature(visibleChild()), ok); }
        if (ok) *ok = false; return QString("ERROR: '%1', of class '%2', doesn't support isChecked").arg(signature()).arg(_instance->metaObject()->className());
    }
    virtual QString getSelectedText(bool *ok) {
        bool _ok; QString ret = QTestWidget::getSelectedText(&_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::getSelectedText(QTestWidgets::signature(visibleChild()), ok); }
        if (ok) *ok = false; return QString("ERROR: '%1', of class '%2', doesn't support getSelectedText").arg(signature()).arg(_instance->metaObject()->className());
    }
    virtual QString getText(bool *ok) {
        bool _ok; QString ret = QTestWidget::getText(&_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::getText(QTestWidgets::signature(visibleChild()), ok); }
        if (ok) *ok = false; return QString("ERROR: '%1', of class '%2', doesn't support getText").arg(signature()).arg(_instance->metaObject()->className());
    }
    virtual QStringList getList(bool *ok) {
        bool _ok; QStringList ret = QTestWidget::getList(&_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::getList(QTestWidgets::signature(visibleChild()), ok); }
        if (ok) *ok = false; return QStringList(QString("ERROR: '%1', of class '%2', doesn't support getList").arg(signature()).arg(_instance->metaObject()->className()));
    }
    virtual QPoint getCenter(QString const &item, bool *ok) {
        bool _ok; QPoint ret = QTestWidget::getCenter(item, &_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::testWidget(visibleChild())->getCenter(item, ok); }
        if (ok) *ok = false; return QPoint();
    }
    virtual QList<QPoint> navigateByMouse(QString const &item, QPoint const &pos, int *wait, bool *ok) {
        bool _ok; QList<QPoint> ret = QTestWidget::navigateByMouse(item, pos, wait, &_ok); if (_ok) { if (ok) *ok = true; return ret; }
        if (visibleChild()) { return QTestWidgets::testWidget(visibleChild())->navigateByMouse(item, pos, wait, ok); }
        if (ok) *ok = false; return QList<QPoint>();
    }
    virtual bool isEditableType() {return false;}
    virtual bool isCheckboxType() {return false;}

    static QTestWidget* factory( const QString &signature, QObject* w )
        { return new QTestUnsupportedClass( signature, w ); }

    static bool proxyToChild;
private:
    static bool isRegistered;

    QWidget* visibleChild() const {
        if (!proxyToChild) return 0;
        if (!_instance) return 0;
        QWidget *ret = 0;
        foreach(QObject* c, _instance->children()) {
            QWidget *w = qobject_cast<QWidget*>(c);
            if (!w) continue;
            if (QTestWidgets::widgetVisible(w)) {
                if (ret) return 0; // only return non-zero if there is just 1 visible child
                ret = w;
            }
        }
        return ret;
    }

};

bool QTestUnsupportedClass::isRegistered = false;
bool QTestUnsupportedClass::proxyToChild = true;

//***********************************************************

const char nameDelimiterChar = '/';
QHash<QString, QTestWidget *> QTestWidgets::testWidgetsBySignature;
QHash<QObject*, QTestWidget *> QTestWidgets::testWidgetsByPtr;

QObject *QTestWidgets::_findWidget( const QString &signature )
{
    if (signature.isEmpty()) {
        qLog(QtUitest) <<  "QTestWidgets::findWidget() ... no signature or text specified. Returning active widget." ;
        return QTestWidgets::activeWidget();
    }

    QTestWidget *rec = QTestWidgets::testWidgetsBySignature.value(signature, 0);
    if (rec != 0) {
        qLog(QtUitest) <<  QString("QTestWidgets::findWidget() ... using DB: " + signature).toLatin1() ;
        return rec->instance(); //FIXME: can we always assume that the instance is still valid/existing?
    }

    QWidget *w = qobject_cast<QWidget *>(QTestWidgets::findObject(signature));
    if (w != 0) QTestWidgets::testWidget( w ); // make a testwidget instance
        qLog(QtUitest) <<  QString("QTestWidgets::findWidget() ... added to DB: " + signature).toLatin1() ;
    return w;
}

QObject* QTestWidgets::findObject( const QString &signature )
{
    return signature.isEmpty() ? qApp->focusWidget() : findObjectBySignature(signature, allObjects());
}

QString QTestWidgets::isChecked( const QString &signature, bool *ok )
{
    QTestWidget *rec = testWidget(signature);
    if (rec) return rec->isChecked(ok);

    if (ok) *ok = false;
    return "ERROR_NO_FOCUS_WIDGET: " + signature;
}

QString QTestWidgets::getSelectedText( const QString &signature, bool *ok )
{
    QTestWidget *rec = testWidget(signature);
    if (rec) return rec->getSelectedText(ok).replace(char(-83), "");

    if (ok) *ok = false;
    return "ERROR_NO_FOCUS_WIDGET: " + signature;
}

QString QTestWidgets::getText( const QString &signature, bool *ok )
{
    QTestWidget *rec = testWidget(signature);
    if (rec) return rec->getText(ok).replace(char(-83), "");

    if (ok) *ok = false;
    return "ERROR_NO_FOCUS_WIDGET: " + signature;
}

QStringList QTestWidgets::getList( const QString &signature, bool *ok )
{
    QTestWidget *rec = testWidget(signature);
    if (rec) return rec->getList(ok);

    if (ok) *ok = false;
    return QStringList() << "ERROR_NO_FOCUS_WIDGET: " + signature;
}

/*!
  Returns a list of signatures for all widgets that contain \a widgetText in
  their text and are optionally of a type specified in \a typeList.
*/
QStringList QTestWidgets::locateWidgetByText( const QString &widgetText, const QStringList &typeList )
{
    return locateWidgetByText( widgetText, typeList, allObjects());
}

/*!
   Returns a list of signatures for all objects with a type listed in \a typeList.
*/
QStringList QTestWidgets::locateObjectByType( const QStringList &typeList )
{
    return locateObjectByType( typeList, allObjects());
}

/*!
   Returns a comma-separated list of signatures for all widgets with \a widgetName and optionally with
   a type listed in \a typeList.
*/
/*
QString QTestWidgets::locateWidget( const QString &widgetName, const QString &typeList )
{
    QStringList widgets = locateWidget( widgetName, typeList.split(','), allObjects());
    QString reply;

    if (!widgets.isEmpty())
        reply = widgets.join(",");
    else
        reply = "ERROR_NO_WIDGETS_FOUND";

    return reply;
}
*/

/*!
    Returns a fully qualified path of the \a object and all it's parents.
    For example: /mainWidget/groupbox1/myWidget or an empty string in case the object doesn't exist.
*/
QString QTestWidgets::signature( const QObject *object )
{
    if (!object) return "ERROR_NO_OBJECT";

    QString sig = uniqueName(object);
    QObject *pobject = object->parent();

    while (pobject != 0) {
        sig = uniqueName(pobject) + nameDelimiterChar + sig;
        pobject = pobject->parent();
    }

    return sig;
}

/*
    \internal

    This helper function compares two Factory records and is called from QTestWidgets::registerTestWidget()\
    to keep the list of registered test widgets sorted after each insertion.
    We need to sort so that the most-derived classes are first.
    Returns true if class1 is derived (either directly or indirectly) from class2.
*/
bool QTestWidgets::derivedFrom(FactoryRec class1, FactoryRec class2)
{
    const QMetaObject * m = class1.metaObject->superClass();
    while (m != 0)
    {
        if (m->className() == class2.className) {
            return true;
        }
        m = m->superClass();
    }

    return false;
}

/*!
    \internal

    If \a o has an objectName() return it, otherwise use the class name.
*/
static QString defaultObjectName(const QObject *o)
{
#ifdef USE_ADRESS_FOR_SIGNATURE
    if (o != 0) {
        if (o->metaObject() != 0) {
            return QString("%1[%2]").arg(o->metaObject()->className()).arg((long)(void*)o,0,32);
        }
    }
    return "";
#else
    if (o != 0) {
        QString name = o->objectName();
        if (name.isNull() || name == "unnamed" ) {
            name = QString("class_%1").arg(o->metaObject()->className());
        }
        return name;
    }
    return "";
#endif
}

/*!
    Returns a name for the object that is unique within the same peer level.
*/
QString QTestWidgets::uniqueName( const QObject *item )
{
    if (item == 0)
        return "err_uniqueName";

    // itemName is the object name, if any, otherwise we use the class name.
    QString itemName = defaultObjectName(item);

#ifndef USE_ADRESS_FOR_SIGNATURE
    QObjectList siblings;
    QObject *parent = item->parent();
    // Make sure that all objects, even toplevel objects, are uniquely named.
    // For instance, there can be two QWidget classes both having a QListWidget as their child (real example).
    if (parent) siblings = parent->children();
    else siblings = allObjects();

    bool first_time;
    if (siblings.count() > 0) {
        // loop until we find item, counting how many other items have the same name.
        uint count;
        do {
            count = 0;
            for (int i = 0; i < siblings.count(); ++i) {
                QObject *tmp = siblings.at(i);
                if (tmp == item) {
                    if (first_time) break;
                } else if (defaultObjectName(tmp) == itemName)
                    count++;
            }
            // if we have multiple objects with the same name, we use an indexed naming scheme.
            if (count > 0) {
                if (first_time) {
                    first_time = false;
                    itemName += "_";
                }
                itemName += QString( "%1" ).arg( count );
            }
            // repeat the loop at least once to make sure that our new found name isn't used already
        } while (count > 0);
    }
#endif

    return itemName.replace(' ', '_');
}

QObject* QTestWidgets::findObjectBySignature( const QString &signature, const QObjectList &search_list )
{
    QObjectList list = search_list;
    QStringList s = signature.split(nameDelimiterChar, QString::SkipEmptyParts);
    QObject *obj = 0;

    for (int i = 0; i < s.count(); ++i) {
        QObjectList::const_iterator it = list.begin();
        while (it != list.end() && uniqueName(*it) != s[i]) {
            ++it;
        }
        if (it == list.end()) {
            // reached the end of the list -- widget does not exist.
            last_error() = "ERROR_UNKNOWN_OBJECT_" + signature + "'";
            return 0;
        }
        list = (*it)->children();
        obj = *it;
    }

    return obj;
}

/*
    Returns the best matching buddy widget for the \a label by scanning through \a search_list. The match is done based on the position information.
*/
QWidget* QTestWidgets::locateBuddy( const QTestWidget *testlabel, const QList< QPointer<QTestWidget> > &search_list )
{
    if (testlabel == 0) return false;
    QWidget *label = qobject_cast<QWidget*>(testlabel->instance());
    if (label == 0) return false;

    int start = testlabel->y();
    int end = start + testlabel->height();
    int x_end = testlabel->x() + testlabel->width();
    qLog(QtUitest) <<  QString("*** locate buddy for %1[%2] y: %3 y+: %4").arg(label->metaObject()->className()).arg((long)(void*)label,0,32).arg(start).arg(end).toLatin1();

    // These widgets are their own buddies.
    if (qobject_cast<QCheckBox*>(label) ||
        qobject_cast<QRadioButton*>(label)) return label;

    // A group box is considered a buddy of itself if and only if it is
    // checkable.
    QGroupBox* groupbox = qobject_cast<QGroupBox*>(label);
    if (groupbox && groupbox->isCheckable()) {
        return label;
    }

    QWidget *buddy = 0;
    foreach(QTestWidget *tw, search_list) {
        QWidget* w = qobject_cast<QWidget*>(tw->instance());
        if ( w && w != label && QTestWidgets::widgetVisible(w)) {
            int w_start = tw->y();
            int w_end = w_start + tw->height();
    qLog(QtUitest) <<  QString("y %1 y+ %2 %3").arg(w_start).arg(w_end).arg(tw->signature()).toLatin1();
            if ((start == w_start ||
                (start < w_start && end > w_start) ||
                (start > w_start && start < w_end)) &&
                (x_end < tw->x()) && // FIXME: This assumes Left-to-right languages
                (!w->inherits("QGroupBox")) &&
                (!w->inherits("BuddyFocusBox")) &&
                (!w->inherits("QAbstractScrollArea"))) {
                if (buddy == 0) {
    qLog(QtUitest) <<  "gotcha" ;
                    buddy = w;
                } else {
    qLog(QtUitest) <<  "** Problem: Multiple buddy solutions found for one label**" ;
                    return 0; // multiple possible solutions found
                }
            }
        }

        // If the widget is a QGroupBox, and it has exactly one focusable child widget,
        // consider that child widget the buddy.
        if (!buddy && groupbox && w && groupbox->isAncestorOf(w)) {
            QList<QWidget*> focusableWidgets;
            foreach (QWidget* fw, groupbox->findChildren<QWidget*>()) {
                if (fw->focusPolicy() != Qt::NoFocus)
                    focusableWidgets << fw;
            }
            if (focusableWidgets.count() == 1) {
                buddy = w;
            }
        }
    }

    return buddy;
}

/*
    Returns true if the widget is truly visible, i.e. the widget claims it is visible, all parents are visible and a parent is the activeWindow.
*/
bool QTestWidgets::widgetVisible( QWidget *widget )
{
    if (widget == 0) return false;
    QWidget *aw = activeWidget();
    QWidget *w = widget;
    bool is_active = (aw == 0); // if there's no activeWindow then we solely depend on isVisible and isHidden
    do {
        if (w != 0) {
            if (w->isHidden() || !w->isVisible()) return false;
            // check if a parent is the activeWindow.
            if (w == aw) is_active = true;
        }
        w = w->parentWidget();
    } while (w != 0);
    return is_active;
}

QStringList QTestWidgets::locateObjectByType( const QStringList &typeList, const QObjectList &searchList )
{
    QStringList objects;

    QObjectList::const_iterator it = searchList.begin();
    while (it != searchList.end()) {
        QObject *o = qobject_cast<QObject*>(*it);
        if (o) {
            QString sig = signature(o);
            if (
            (typeList.count() == 0 ||
            (typeList.count() == 1 && typeList[0].isEmpty()) ||
             typeList.contains(o->metaObject()->className())) &&
            !objects.contains( sig )) {
                objects.append(sig);
            }
        }
        QStringList tmp_list = locateObjectByType( typeList, (*it)->children());
        // prevent duplicates
        for (int k=0; k<tmp_list.count(); k++) {
            if (!objects.contains( tmp_list[k] ))
                objects.append( tmp_list[k] );
        }
        ++it;
    }
    return objects;
}

QStringList QTestWidgets::locateWidget( const QString &widgetName, const QStringList &typeList, const QObjectList &searchList )
{
    QStringList widgets;

    QObjectList::const_iterator it = searchList.begin();
    while (it != searchList.end()) {
        QWidget *w = qobject_cast<QWidget*>(*it);
        if (w) {
            QString sig = signature(w);
            if (("*" == widgetName || widgetName.isEmpty() || uniqueName(w) == widgetName) &&
            (typeList.count() == 0 || (typeList.count() == 1 && typeList[0].isEmpty()) ||      typeList.contains(w->metaObject()->className())) &&
            !widgets.contains( sig ))
            widgets.append(sig);
        }
        QStringList tmp_list = locateWidget(widgetName, typeList, (*it)->children());
        // prevent duplicates
        for (int k=0; k<tmp_list.count(); k++) {
            if (!widgets.contains( tmp_list[k] ))
                widgets.append( tmp_list[k] );
        }
        ++it;
    }
    return widgets;
}

QStringList QTestWidgets::locateWidgetByText( const QString &widgetText, const QStringList &typeList, const QObjectList &searchList, bool checkChildren )
{
    if (searchList.count() == 0) return QStringList();

    QStringList widgets;
    QStringList exclude;
    QStringList include;
    foreach(QString t, typeList) {
        if (!t.startsWith("!")) include << t;
        else                    exclude << t.mid(1);
    }

    QObjectList::const_iterator it = searchList.begin();
    while (it != searchList.end()) {
        QWidget *w = qobject_cast<QWidget*>(*it);
        if (w && QTestWidgets::widgetVisible(w)) {
            QString cn = w->metaObject()->className();
            if (cn != "QWidget" &&
                cn != "QStackedWidget" &&
                cn != "QDelayedScrollArea" &&
                cn != "QFrame" &&
                cn != "BuddyFocusBox") {

                QTestWidget *rec = testWidget(w);
                if (rec) {
                    // remove soft returns
                    bool ok;
                    QTestUnsupportedClass::proxyToChild = false;
                    QString widget_txt = rec->getSelectedText(&ok).replace(char(-83), "");

                    QTestUnsupportedClass::proxyToChild = true;
                    if (ok) {
                        if (widgetText.contains("/") && widgetText != widget_txt && widgetText.contains(widget_txt)) {
                            // maybe we're dealing with a groupbox sub-field?
                            QWidget *parent = qobject_cast<QWidget*>(w->parent());
                            if (parent && parent->inherits("QGroupBox")) {
                                QGroupBox *gb = qobject_cast<QGroupBox*>(parent);
                                if (gb && !gb->title().isEmpty()) widget_txt = gb->title() + "/" + widget_txt;
                            }
                        }
                        if (widgetText == widget_txt &&
                            (
                                (include.count() == 0) ||
                                (include.count() == 1 && include[0].isEmpty()) || (include.contains(w->metaObject()->className()))
                            ) &&
                            !exclude.contains(w->metaObject()->className()) &&
                            !widgets.contains(rec->signature())
                            ) {
                            widgets.append(rec->signature());
                        }
                    }
                }
            }
            if (checkChildren) {
                QObjectList child_list = (*it)->children();
                if (child_list.count() > 0) {
                    QStringList tmp_list = locateWidgetByText(widgetText, typeList, (*it)->children());
                    // prevent duplicates
                    for (int k=0; k<tmp_list.count(); k++) {
                        if (!widgets.contains( tmp_list[k] ))
                            widgets.append( tmp_list[k] );
                    }
                }
            }
        }

        ++it;
    }

    return widgets;
}

void QTestWidgets::clear()
{
    testWidgetsBySignature.clear();
    testWidgetsByPtr.clear();
}

uint QTestWidgets::count()
{
    return testWidgetsBySignature.count();
}

/*!
    Returns the testWidget that is associated with the given \a widget.
*/
QTestWidget *QTestWidgets::testWidget( QObject *widget )
{
    if (!widget) return 0; // not much we can do

    QTestWidget *rec = testWidgetsByPtr.value( widget, 0 );
    if (rec != 0) return rec;

    QString new_signature = QTestWidgets::signature( widget );
    rec = createInstance( widget, new_signature );

    if (rec != 0) {
        testWidgetsBySignature[new_signature] = rec;
        testWidgetsByPtr[ widget ] = rec;
    }

    return rec;
}

/*!
    Returns the testWidget that is associated with the given \a signature.
*/
QTestWidget *QTestWidgets::testWidget( const QString signature )
{
    if (signature.isEmpty()) {
        return testWidget( qApp->focusWidget() );
    }

    QTestWidget *rec = testWidgetsBySignature.value( signature, 0 );
    if (rec != 0) return rec;

    QWidget *w = qobject_cast<QWidget *>(findObject(signature));
    if (w == 0) return 0; // not much we can do

    return testWidget( w ); // make a testwidget instance
}

/*!
    Factory that creates an instance of one of the QTestWidget sub-classes that knows best
    how to deal with \a o. The created instance can be identified with \a new_signature and
    will be owned by \a owner.
*/
QTestWidget* QTestWidgets::createInstance( QObject *w, const QString &signature )
{
    if (w->inherits( "QTestWidget" ) || w->inherits( "QTestWidgets" )) {
        return 0;
    }

    // Attempt to find a factory with which to make a QTestWidget for the object.
    QList< FactoryRec >::iterator it = factories().begin();
    while (it != factories().end()) {
        if (w->inherits( it->className.toLatin1() )) {
            return it->factory( signature, w );
        }
        ++it;
    }

    // Last resort -- we don't know what kind of widget we have.
    return QTestUnsupportedClass::factory( signature, w );
}

/*!
    Removes test widget \a t from the cache and destroys it.
*/
void QTestWidgets::removeWidget( QTestWidget *t )
{
    if (t != 0) {
        testWidgetsByPtr.remove( t->_unsafe_instance );
/*
        QHash<QWidget*, QTestWidget*>::iterator i = testWidgetsByPtr.begin();
        while (i != testWidgetsByPtr.end()) {
            if (i.value() == t) testWidgetsByPtr.erase(i);
            else ++i;
        }
*/
        testWidgetsBySignature.remove( t->signature() );
    }
}

/*!
    \internal

    Returns a list of all top-level QObjects in the current application.
*/
QObjectList QTestWidgets::allObjects()
{
    QWidgetList wl = qApp->topLevelWidgets();
    QObjectList ol = qApp->children();
    foreach(QWidget *w, wl) {
        if (!ol.contains(w)) ol << w;
    }
    return ol;
}

/*!
    Returns the current activeWindow, activePopupWidget or activeModalWidget.
*/
QWidget* QTestWidgets::activeWidget()
{
    QWidget *fw = qApp->focusWidget();
    QWidget *w = 0;
    if (qApp) {
        if (w == 0) w = qApp->activeModalWidget();
        if (w == 0) w = qApp->activePopupWidget();
        if (w == 0) w = qApp->activeWindow();
    }

    if (w != 0 && fw != 0) {
        QObject *parent = fw->parent();
        while (parent != 0) {
            if (parent == w) return w;
            parent = parent->parent();
        }
//        showVisibleChildren(fw);
        if (!QTestWidgets::hasVisibleChildren(fw) && fw->parent()) {
            parent = fw->parent();
            while (parent != 0 && !parent->inherits("QWidget")) parent = parent->parent();
            if (parent) return (QWidget*)parent;
        }
        return fw;
    }
    if (!w) w = fw;
    return w;
}

bool QTestWidgets::hasVisibleChildren( QObject *w )
{
    if (w == 0 || w->children().count() == 0) return false;

    foreach(QObject* c, w->children()) {
        if (c) {
            if (hasVisibleChildren(c)) return true;
            QWidget *cw = qobject_cast<QWidget*>(c);
            if (cw && !cw->isHidden() && cw->isVisible()) {

                return true;
            }
        }
    }
    return false;
}

// **********************************************************************

QActiveTestWidgetData::QActiveTestWidgetData()
{
}

QActiveTestWidgetData::~QActiveTestWidgetData()
{
}

void QActiveTestWidgetData::clear()
{
    resolved_buddy_pairs.clear();
    visible_tw_buddies.clear();
    unresolved_tw_buddies.clear();
    visible_tw_labels.clear();
    unresolved_tw_labels.clear();
    active_scrollbars.clear();
    active_tabbars.clear();
}

void QActiveTestWidgetData::removeTestWidget( QTestWidget *w )
{
//    resolved_buddy_pairs.removeAll(w);
    visible_tw_buddies.removeAll(w);
    unresolved_tw_buddies.removeAll(w);
    visible_tw_labels.removeAll(w);
    unresolved_tw_labels.removeAll(w);
    active_scrollbars.removeAll(w);
    active_tabbars.removeAll(w);
}

bool QActiveTestWidgetData::scan( QObject *ao )
{

    if (qobject_cast<QAbstractSpinBox*>(ao)) return false;

    QtUiTest::Widget* aw = qtuitest_cast<QtUiTest::Widget*>(ao);
    if (aw == 0) return false;

    bool any_appended = false;
    foreach(QObject *o, aw->children()) {
        QWidget *w = qobject_cast<QWidget*>(o);
        if (w && !QTestWidgets::widgetVisible(w)) continue;

        // Recursively scan child widgets
        any_appended |= scan(o);

        do {
            if (qobject_cast<QScrollBar*>(w)) {
                QTestWidget *rec = QTestWidgets::testWidget(w);
                if (rec) active_scrollbars.append(rec);
                break;
            }

            if (qobject_cast<QTabWidget*>(w) ||
                qobject_cast<QTabBar*>(w)) {
                QTestWidget *rec = QTestWidgets::testWidget(w);
                if (rec) active_tabbars.append(rec);
                break;
            }

            QLatin1String cn(o->metaObject()->className());

            // not interested in these.
            if (cn == "QDelayedScrollArea" ||
                cn == "QStackedWidget" ||
                cn == "BuddyFocusBox" ||
                cn == "QFrame" ||
                cn == "QWidget") {
                break;
            }

            QTestWidget *rec = QTestWidgets::testWidget(o);
            if (!rec) {
                break;
            }

            if (qobject_cast<QGroupBox*>(w)) {
                visible_tw_labels.append(rec);
                visible_tw_buddies.append(rec);
                break;
            }

            if (qobject_cast<QLabel*>(w) ||
#ifdef QTOPIA_TARGET
                qobject_cast<QIconSelector*>(w) ||
#endif
                qobject_cast<QAbstractButton*>(w) ||
                qobject_cast<QRadioButton*>(w) ||
                qobject_cast<QCheckBox*>(w) ||
                qobject_cast<QComboBox*>(w)) {
                if (visible_tw_labels.contains(rec))
                    qWarning( "adding a duplicate to visible labels" );
                else {
                    any_appended = true;
                    visible_tw_labels.append(rec);

                }
                break;
            }

            QtUiTest::LabelWidget* lw = qtuitest_cast<QtUiTest::LabelWidget*>(o);
            if (lw) {
                visible_tw_labels.append(rec);
                break;
            }

            // OK, the widget doesn't satisfy any special cases, but it is
            // potentially interesting, so keep it.
            if (visible_tw_buddies.contains(rec))
               qWarning( "adding a duplicate to visible buddies" );
            else {
                any_appended = true;

                visible_tw_buddies.append(rec);
            }
        } while(0);
    }

    return any_appended;
}

void QActiveTestWidgetData::resolveLabels()
{
    // resolve relationship between label and buddy fields
    unresolved_tw_labels = visible_tw_labels;
    unresolved_tw_buddies = visible_tw_buddies;
    foreach (QTestWidget *lbl_rec, visible_tw_labels) {
        Q_ASSERT(lbl_rec);

        QWidget *buddy = 0;
        QObject *w = lbl_rec->instance();

        do {
            QLabel *label(qobject_cast<QLabel*>(w));
            if (!label) break;

            buddy = label->buddy();
            break;
        } while(0);

        if (!buddy)
            buddy = QTestWidgets::locateBuddy( lbl_rec, visible_tw_buddies );

        // If there's "definitely" no buddy... well, some widgets can be both
        // interactive and self-labelling, so if it's one of those, it's
        // its own buddy.
        if (!buddy && (qobject_cast<QAbstractButton*>(w) || w->inherits("QComboBox"))) {
            buddy = qobject_cast<QWidget*>(lbl_rec->instance());
            // A button is both a label and a field. Some buttons however have an icon as 'text' which means we can't access them through
            // their label text. So buttons also need to go into the buddy-list and be sorted on their physical location.
            QTestWidget *buddy_rec(QTestWidgets::testWidget(buddy));
            if (buddy_rec) visible_tw_buddies.append(buddy_rec);
        }

        if (!buddy) continue;

        // If the buddy has a focus proxy, set that as the actual buddy,
        // as that's the widget that will always receive focus.
        QWidget *focusProxy = buddy->focusProxy();
        while (focusProxy) {
            buddy = focusProxy;
            focusProxy = buddy->focusProxy();
        }

        QTestWidget *buddy_rec(QTestWidgets::testWidget(buddy));
        if (!buddy_rec) {
            qLog(QtUitest) <<  "*** Sorry, no testwidget available for x ***" ;
            continue;
        }

        if (resolved_buddy_pairs.values().contains(buddy_rec)) {
          qLog(QtUitest) <<  QString("*** duplicate Label: %1[%2] -- Buddy: %3[%4]").arg(w->metaObject()->className()).arg((long)(void*)w,0,32).arg(buddy->metaObject()->className()).arg((long)(void*)buddy,0,32).toLatin1();
        } else {
            resolved_buddy_pairs.insert( w, buddy_rec );
            qLog(QtUitest) <<  QString("*** resolved  Label: %1[%2] -- Buddy: %3[%4]").arg(w->metaObject()->className()).arg((long)(void*)w,0,32).arg(buddy->metaObject()->className()).arg((long)(void*)buddy,0,32).toLatin1();
            unresolved_tw_buddies.removeAll( buddy_rec );
            unresolved_tw_labels.removeAll( buddy_rec );
            if (buddy_rec != lbl_rec) visible_tw_labels.removeAll( buddy_rec );
            unresolved_tw_buddies.removeAll( lbl_rec );
            unresolved_tw_labels.removeAll( lbl_rec );
        }
    }


}

void QActiveTestWidgetData::sort()
{
    // sort the buddy list based on y position;
    // if two widgets have same y position, sort based on x position
    class TestWidgetSort {
        public:
        static bool lessThan(QTestWidget *a, QTestWidget *b) {
            Q_ASSERT(a && b);
            if (a->y() == b->y())
                return a->x() < b->x();
            else
                return a->y() < b->y();
        }
    };

    qStableSort(visible_tw_buddies.begin(), visible_tw_buddies.end(), TestWidgetSort::lessThan);
}

QString textForLabel(QTestWidget *label, bool &ok)
{
    QString ret;

    // If the testwidget is a LabelWidget, that takes highest priority.
    // Else just treat its displayed text as its label.
    QtUiTest::LabelWidget* lw = qtuitest_cast<QtUiTest::LabelWidget*>(label->instance());
    if (lw) {
        ret = lw->labelText();
    } else {
        ret = label->getSelectedText(&ok);
        if (!ok) return ret;
    }

    QWidget *w = qobject_cast<QWidget*>(label->instance());
    if (w) w = w->parentWidget();
    QGroupBox *gb = 0;
    while (w) {
        if ((gb = qobject_cast<QGroupBox*>(w))) {
            if (!gb->title().isEmpty()) ret.prepend(gb->title() + "/");
        }
        w = w->parentWidget();
    }
    return ret;
}

bool QActiveTestWidgetData::findLabel( const QString &labelText, QTestWidget *&label, QString &error )
{
    error = "";
    bool ok;
    QList<QTestWidget*> possibleMatches;
    QList<QTestWidget*> definiteMatches;
    for (int i=0; i<visible_tw_labels.count(); i++)
    {
        label = qobject_cast<QTestWidget*>(visible_tw_labels.at(i));
        if (label && textForLabel(label, ok).trimmed() == labelText.trimmed() && ok) {
            definiteMatches << label;
        } else if (label && label->getSelectedText(&ok).trimmed() == labelText.trimmed() && ok) {
            possibleMatches << label;
        }
    }
    if (definiteMatches.count() == 1) {
        label = definiteMatches[0];
        return true;
    }
    if (definiteMatches.count() > 1) {
        error = "ERROR: '" + labelText + "' is ambiguous.\n Available labels: " + allLabels().join(",");
        label = 0;
        return false;
    }

    /* No definite matches. */
    if (possibleMatches.count() == 1) {
        label = possibleMatches[0];
        return true;
    }
    if (possibleMatches.count() > 1) {
        error = "ERROR: '" + labelText + "' is ambiguous.\n Available labels: " + allLabels().join(",");
        label = 0;
        return false;
    }

    error = "ERROR: No label with text '" + labelText + "' found.\n Available labels: " + allLabels().join(",");
    label = 0;
    return false;
}

bool QActiveTestWidgetData::findWidget( const QString &labelOrSignature, QTestWidget *&buddy, QString &error, int offset )
{
    error = "";
    buddy = 0;

   qLog(QtUitest) <<  QString("QActiveTestWidgetData::findWidget(%1,%2)").arg(labelOrSignature).arg(offset).toLatin1() ;

    if (labelOrSignature == TAB_BAR_ALIAS) return findTab( labelOrSignature, buddy, error );

    if (labelOrSignature.contains("[") && labelOrSignature.contains("]")) {
        // it's probably signature. Check the db first
        buddy = QTestWidgets::testWidget(labelOrSignature);
    }

    if (buddy == 0) {
        // it's probably a label text, so do a findLabel first.
        QTestWidget *label = 0;
        if (!findLabel(labelOrSignature,label,error)) return false;
        QHash< QPointer<QObject>,QTestWidget*>::iterator i = resolved_buddy_pairs.begin();
//        QHash< QPointer<QWidget>,QPointer<QTestWidget> >::iterator i = resolved_buddy_pairs.begin();
        while (i != resolved_buddy_pairs.end()) {
            if (i.key() == label->instance()) {
                buddy = i.value();
                break;
            }
            ++i;
        }
    }

    if (buddy != 0 && offset == 0) {
            qLog(QtUitest) <<  QString("QActiveTestWidgetData::findWidget(offset=0) ... found: " + buddy->signature()).toLatin1() ;
        return true;
    }

    int index = 0;
    int pos = visible_tw_buddies.indexOf(buddy);
    if (pos < 0) {
        error = "ERROR: Buddy widget for '" + labelOrSignature + "' not found.\nAvailable labels: " + allLabels().join(",");
        return false;
    }
    index = pos + offset;

    qLog(QtUitest) <<  QString("QActiveTestWidgetData::findWidget(offset=%1) index=%2 ").arg(offset).arg(index).toLatin1() ;

    if ((index >= 0) && (index < visible_tw_buddies.count())) {
        buddy = qobject_cast<QTestWidget*>(visible_tw_buddies.at(index));
        if (buddy != 0 && buddy->instance() != 0)
            return true;
    }

    return findTab( labelOrSignature, buddy, error );
}

bool QActiveTestWidgetData::findTab( const QString &signature, QTestWidget *&tab, QString &error )
{
    tab = 0;
    error = "";

    foreach(QTestWidget *tabbar, active_tabbars) {
        if (signature == TAB_BAR_ALIAS) {
            if (qobject_cast<QTabBar*>(tabbar->instance())) {
                if (tab == 0)
                    tab = tabbar;
                else {
                    error = "ERROR: Multiple Tabs found. Don't know which one to choose.";
                    return false; // there should be only ONE tab, or else we have a conflict
                }
            }
        } else if (tabbar->signature() == signature) {
            tab = tabbar;
            break;
        }
    }

    if (tab == 0) error = "ERROR: Tab widget not found.";
    return tab != 0;
}

QStringList QActiveTestWidgetData::allLabels()
{
    bool ok = true;
    QStringList ret;
    foreach (QTestWidget *label, visible_tw_labels) {
        if (label) ret << textForLabel(label, ok);
    }

    return ret;
}

// ****************************************************************
// ****************************************************************

#include <QTime>
QActiveTestWidget::QActiveTestWidget()
{
    active_widget = 0;
    d = 0;
    scan_busy = false;
    active_test_widget_valid = false;
}

QActiveTestWidget::~QActiveTestWidget()
{

    delete d;
    active_data = 0;
}

QActiveTestWidget* QActiveTestWidget::instance()
{
    static QActiveTestWidget qatw;
    return &qatw;
}

void QActiveTestWidget::clear()
{
    if (active_widget) active_widget->removeEventFilter(this);
    active_widget = 0;
    scan_time = 0;
    if (d == 0) {
        d = new QActiveTestWidgetData();
        active_data = d;
    } else
        d->clear();
}

const QString QActiveTestWidget::NoActiveWidgetError("ERROR: No active widget available");

bool QActiveTestWidget::rescan( QString &error, int timeout )
{
    if (scan_busy) return true;
    scan_busy = true;

    if (d == 0) d = new QActiveTestWidgetData();

    // FIXME: should we use a QElapsedTimer?
    QTime t;
    t.start();
    QWidget *aw = 0;
    while (aw == 0) {
        if (!QApplication::startingUp()) {
            aw = QTestWidgets::activeWidget();
            if (aw != 0) break;
        }
        if (t.elapsed() >= timeout) {
            error = QString("%1 (timeout %2 ms)").arg(NoActiveWidgetError).arg(t.elapsed());
            scan_busy = false;
            return false;
        }
        QtUiTest::wait(3);
    }

    clear();
    if (aw == 0) {
        error = "ERROR: No active widget available";
        scan_busy = false;
        return false;
    }
    active_widget = aw;
    active_widget->installEventFilter(this);

    d->scan(active_widget);
    d->resolveLabels();
    d->sort();
    scan_time = t.elapsed();
    active_tab = currentTab();
    active_title = active_widget->windowTitle();





    scan_busy = false;
    active_test_widget_valid = true;
    return true;
}

QString QActiveTestWidget::toString()
{
    bool ok;
    QString ret;
    if (!d) rescan(ret, 1000);
    ret =    QString("Application  : %1").arg(qApp->applicationName());
    ret +=         "\nTitle        : " + (active_widget ? active_widget->windowTitle() : "(no active widget)");
    ret +=         "\nActive Widget: " + (active_widget ? QTestWidgets::signature(active_widget) : "(no active widget)");
    ret +=         "\nFocus Widget : " + QTestWidgets::signature(qApp->focusWidget());
    ret +=         "\nActive Tab   : " + active_tab;
//    ret += QString("\nScan time    : %1 ms").arg(scan_time);

    bool first = true;
    foreach(QTestWidget *tw, d->active_scrollbars) {
        if (first) {
            ret += "\nActive scrollbars:";
            first = false;
        }
        ret += QString("\n  %1  x:%2 y:%3 h:%4 w:%5").arg(tw->uniqueName()).arg(tw->x()).arg(tw->y()).arg(tw->height()).arg(tw->width());
    }

    first = true;
    QHash< QPointer<QObject>,QTestWidget*>::iterator i = d->resolved_buddy_pairs.begin();
    while (i != d->resolved_buddy_pairs.end()) {
        QTestWidget *buddy_rec = i.value();
        QObject *lbl = i.key(); // must be label
        if (buddy_rec != 0 && lbl != 0) {
            QTestWidget *lbl_rec = QTestWidgets::testWidget(lbl);
            QString lbl_txt;
            if (lbl_rec) lbl_txt = textForLabel(lbl_rec, ok);

            QString buddy_txt = textForLabel(buddy_rec, ok);
            if (buddy_txt.length() > 50)
                buddy_txt = buddy_txt.left(50) + " [...]";

            QObject *buddy(buddy_rec->instance());
            Q_ASSERT(buddy);

            QLatin1String cn(buddy->metaObject()->className());
            if (first) {
                ret += "\nBuddypairs:";
                first = false;
            }
            ret += QString("\n  Label: %1[%2] '%3' -- Buddy: %4[%5] '%6' x:%7 y:%8 h:%9 w:%10").
                arg(lbl->metaObject()->className()).
                arg((long)(void*)lbl,0,32).
                arg(lbl_txt).
                arg(cn).
                arg((long)(void*)buddy,0,32).
                arg(buddy_txt).
                arg(buddy_rec->x()).
                arg(buddy_rec->y()).
                arg(buddy_rec->height()).
                arg(buddy_rec->width());
        }
        ++i;
    }

    first = true;
    foreach(QTestWidget* buddy_rec, d->unresolved_tw_buddies) {
        if (buddy_rec) {
            QObject *vw(buddy_rec->instance());
            if (vw && !vw->inherits("BuddyFocusBox") && vw->isWidgetType() && qobject_cast<QWidget*>(vw)->isEnabled()) {
                if (first) {
                    ret += "\nWidgets without a buddy Label:";
                    first = false;
                }
                ret += QString("\n  %1 x:%2 y:%3 h:%4 w:%5").arg(buddy_rec->signature()).arg(buddy_rec->x()).arg(buddy_rec->y()).arg(buddy_rec->height()).arg(buddy_rec->width()).toLatin1();
            }
        }
    }

    first = true;
    foreach(QTestWidget* buddy_rec, d->unresolved_tw_buddies) {
        if (buddy_rec) {
            QObject *vw(buddy_rec->instance());
            if (vw && !vw->inherits("BuddyFocusBox") && vw->isWidgetType() && !qobject_cast<QWidget*>(vw)->isEnabled()) {
                if (first) {
                    ret += "\nDisabled widgets without a buddy Label:";
                    first = false;
                }
//            ret += QString("\n  %1").arg(buddy_rec->signature()).toLatin1();
                ret += QString("\n  %1 x:%2 y:%3 h:%4 w:%5").arg(buddy_rec->signature()).arg(buddy_rec->x()).arg(buddy_rec->y()).arg(buddy_rec->height()).arg(buddy_rec->width()).toLatin1();
            }
        }
    }

    first = true;
    foreach(QTestWidget *lbl_rec, d->unresolved_tw_labels) {
        if (lbl_rec) {
            QObject *vw(lbl_rec->instance());
            if (!vw) continue;
            QString txt = textForLabel(lbl_rec, ok);
            if (first) {
                ret += "\nLabels without a buddy widget:";
                first = false;
            }
            ret += QString("\n  %1 Text: %2 x:%3 y:%4 h:%5 w:%6").arg(lbl_rec->signature()).arg(txt).arg(lbl_rec->x()).arg(lbl_rec->y()).arg(lbl_rec->height()).arg(lbl_rec->width()).toLatin1();
        }
    }

    first = true;
    foreach(QTestWidget *tw, d->visible_tw_buddies) {
        if (tw) {
            if (first) {
                ret += "\nBuddy widgets sorted on position:";
                first = false;
            }
            ret += QString("\n  %1  x:%2 y:%3 h:%4 w:%5").arg(tw->uniqueName()).arg(tw->x()).arg(tw->y()).arg(tw->height()).arg(tw->width());
        }
    }

    return ret;
}

QStringList QActiveTestWidget::allLabels()
{
    QString error;
    if (!rescan(error)) return QStringList(error);
    if (d) return d->allLabels();
    return QStringList("ERROR: No labels found");
}

bool QActiveTestWidget::isList()
{
    QString error;
    if (!rescan(error)) return false;

    // we assume that something is a list if we couldn't find any known label types as children
    return d->visible_tw_labels.count() > 0;
}

bool QActiveTestWidget::findLabel( const QString &labelText, QTestWidget *&label, QString &error )
{
    if (!rescan(error)) return false;

    return d->findLabel( labelText, label, error );
}

/*!
    Returns the friendliest possible, unambiguous name for \a o.
*/
QString QActiveTestWidget::friendlyName( QObject* o )
{
    QtUiTest::Widget* w = qtuitest_cast<QtUiTest::Widget*>(o);

    if (!w)
        return QTestWidgets::signature(o);

#define SPECIAL(Alias,Type)              \
    if (w->inherits(QtUiTest::Type)) { \
        return Alias;                    \
    }
    SPECIAL(SOFT_MENU_ALIAS, SoftMenu);
    SPECIAL(TAB_BAR_ALIAS, TabBar);
    SPECIAL(LAUNCHER_MENU_ALIAS, Launcher);
    SPECIAL(OPTIONS_MENU_ALIAS, OptionsMenu);
#undef SPECIAL

    QString ret;
    rescan(ret, 50);

    // Look through all of the buddy/label pairs.
    // If this widget is in the list, then return its label as the friendly name.
    QObject* label = 0;
    QHash< QPointer<QObject>,QTestWidget*>::iterator i;
    int focusableBuddyCount = 0;
    for ( i = d->resolved_buddy_pairs.begin(); i != d->resolved_buddy_pairs.end(); ++i) {
        QTestWidget* buddy = i.value();
        if (!buddy) continue;

        // Determine if this widget can ever have focus.
        QObject* object = buddy->instance();
        if (QWidget* widget = qobject_cast<QWidget*>(object)) {
            if (widget->focusPolicy() != Qt::NoFocus) {
                ++focusableBuddyCount;
            }
        } else {
            // Oops, it wasn't a QWidget.
            // Better just assume it can have focus.
            ++focusableBuddyCount;
        }

        label = i.key();
        if (!label) continue;

        QtUiTest::Widget* otherW = qtuitest_cast<QtUiTest::Widget*>(buddy->instance());
        if (otherW == w)
            break;

        QTestWidget* qtw = qobject_cast<QTestWidget*>(label);
        otherW = qtuitest_cast<QtUiTest::Widget*>(qtw ? qtw->instance() : label);
        if (otherW == w)
            break;
        label = 0;
    }

    if (!label) {
        // If this is the _only_ widget that can have focus,
        // an empty string is good enough.
        if (!focusableBuddyCount || ((1 == focusableBuddyCount) && w->hasFocus()))
            return QString();
        return QTestWidgets::signature(o);
    }

    // If we get here, then the widget was in the list of buddy/label pairs,
    // so we can return the label text.
    bool ok;
    ret = textForLabel(QTestWidgets::testWidget(label), ok);
    ret.replace(":", "\\:");
    if (!ok) {
        ret = QTestWidgets::signature(o);
    }
    return ret;
}

bool QActiveTestWidget::findWidget( const QString &labelOrSignature, QTestWidget *&buddy, QString &error, int offset )
{
#define SPECIAL(Alias,Type) \
    if (labelOrSignature == Alias) { \
        QObject *o = QtUiTest::findWidget(QtUiTest::Type); \
        if (!o) error = "Could not find widget of type " #Type; \
        buddy = QTestWidgets::testWidget(o); \
        if (!buddy) error = "Could not construct testwidget wrapper for " #Type; \
        if (buddy && !buddy->instance()) error = "Testwidget instance returned null!"; \
        return buddy && buddy->instance(); \
    }

    SPECIAL(SOFT_MENU_ALIAS, SoftMenu);
    SPECIAL(TAB_BAR_ALIAS, TabBar);
    SPECIAL(LAUNCHER_MENU_ALIAS, Launcher);
    SPECIAL(OPTIONS_MENU_ALIAS, OptionsMenu);

#undef SPECIAL

    bool was_valid = active_test_widget_valid;
    if (!rescan(error)) return false;
    bool ok = findWidget_impl(labelOrSignature,buddy,error,offset);
    if (!ok && was_valid) {

        active_test_widget_valid = false;
        if (!rescan(error)) return false;
        ok = findWidget_impl(labelOrSignature,buddy,error,offset);
    }
    if (!ok) error += "\n" + toString();
    return ok;
}

bool QActiveTestWidget::findWidget_impl( const QString &labelOrSignature, QTestWidget *&buddy, QString &error, int offset )
{
    if (labelOrSignature.isEmpty()) {
        buddy = QTestWidgets::testWidget(qApp->focusWidget());
        if (buddy == 0) {
            error = "ERROR: No focus Widget available\n";
        }
        return buddy != 0;
    }

    return d->findWidget( labelOrSignature, buddy, error, offset );
}

bool QActiveTestWidget::findTab( const QString &signature, QTestWidget *&tab, QString &error )
{
    if (!rescan(error)) return false;
    return d->findTab(signature, tab, error);
}

QString QActiveTestWidget::currentLabelText()
{
    bool ok;
    QString error;
    if (!rescan(error)) return false;
    QWidget *w = qApp->focusWidget();
    if (!w) return "";
    QTestWidget *tw = QTestWidgets::testWidget(w);

    QHash< QPointer<QObject>,QTestWidget*>::iterator i = d->resolved_buddy_pairs.begin();
    while (i != d->resolved_buddy_pairs.end()) {
        if (i.value() == tw) {
            QTestWidget *ltw = QTestWidgets::testWidget(i.key());
            if (ltw) return ltw->getSelectedText(&ok);
        }
        ++i;
    }

    return "";
}

QString QActiveTestWidget::currentFieldText()
{
    QString error;
    if (!rescan(error)) return false;
    QWidget *w = qApp->focusWidget();
    if (!w) return "";
    QTestWidget *tw = QTestWidgets::testWidget(w);
    bool ok;
    return tw->getSelectedText(&ok);
}

QString QActiveTestWidget::currentTab()
{
    QString error;
    if (!rescan(error)) return false;
    bool ok;
    QTestWidget *tw;
    if (d->findTab(TAB_BAR_ALIAS, tw, error)) {
        return tw->getSelectedText(&ok);
    }
    return "";
}

bool QActiveTestWidget::getSelectedText( QString &selectedText, QString &error )
{
    if (!rescan(error)) return false;
    bool ok;
    selectedText = QTestWidgets::getSelectedText("",&ok);
    if (ok && !selectedText.startsWith("ERROR"))
        return true;
    error = selectedText;
    selectedText = "";
    return false;
}

bool QActiveTestWidget::eventFilter(QObject * /*obj*/, QEvent *event)
{
    if (event->type() == QEvent::Hide ||
        event->type() == QEvent::Show ||
        event->type() == QEvent::EnabledChange ||
#ifdef Q_WS_QWS
        event->type() == QEvent::EnterEditFocus ||
        event->type() == QEvent::LeaveEditFocus ||
#endif
        event->type() == QEvent::ParentChange ||
        event->type() == QEvent::FocusIn ||
        event->type() == QEvent::FocusOut ||
        event->type() == QEvent::HideToParent ||
        event->type() == QEvent::ShowToParent ||
        event->type() == QEvent::WindowTitleChange ||
        event->type() == QEvent::ChildAdded ||
        event->type() == QEvent::ChildRemoved ) active_test_widget_valid = false;
    return false;
}

QWidget* QActiveTestWidget::focusWidget()
{
    return qApp->focusWidget();
}

