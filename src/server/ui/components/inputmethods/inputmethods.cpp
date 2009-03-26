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

#include "inputmethods.h"

#include <qsettings.h>
#include <qtopiaapplication.h>
#include <qpluginmanager.h>

#include <qstyle.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qdir.h>
#include <stdlib.h>
#include <qtranslator.h>
#include <qdesktopwidget.h>
#include <qtopialog.h>

#include "windowmanagement.h"
#include "uifactory.h"

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <qtopiachannel.h>

#include <QSoftMenuBar>
#include <QMenu>
#include <QMap>
#include <QStackedWidget>
#include <QDebug>
#include <QTranslatableSettings>
#include <QtopiaInputMethod>
#include <QList>
#include <QPainter>
#include <QApplication>

/*
  Slightly hacky: We use WStyle_Tool as a flag to say "this widget
  belongs to the IM system, so clicking it should not cause a reset".
 */
class IMToolButton : public QToolButton
{
public:
    IMToolButton( QWidget *parent = 0 ) : QToolButton( parent )
    {
    }

    QSize sizeHint() const {
        int w = style()->pixelMetric(QStyle::PM_SmallIconSize);
        return QSize(w,w);
    };
};


/*!
  \class InputMethodSelector
    \inpublicgroup QtInputMethodsModule
  \ingroup QtopiaServer::InputMethods
  \brief The InputMethodSelector class Provides the user-visible aspects of the Input Method system.

    The InputMethodSelector owns the button used to toggle the current input method and to show the current input methods state when the current input method supports those functions.  It also provides the list of input methods, and the button to support them.

    These features are provided by a \l QWidget which is positioned by the theme, usually in the title bar at the top of the screen.  See \l {Theming - Known Elements} for more detail.

    The Input method selector also controls showing and hiding the current input method, and switching between the available input methods, although this functionality is accessible through the main InputMethods class as well.

    Finally, the InputMethodSelector manipulates the value space on behalf of the current input method to facilitate entries in the \l QMenu for the \l QSoftMenuBar for the current input method.

    \sa InputMethods, {Theming - Known Elements}
  */

/*!
  Constructs a new InputMethodSelector widget with \a parent.
  */
InputMethodSelector::InputMethodSelector(QWidget *parent)
    : QWidget(parent), mCurrent(0), m_IMMenuActionAdded(false), m_menuVS("UI/InputMethod"), defaultIM("")
{
    QHBoxLayout *hb = new QHBoxLayout;
    hb->setMargin(0);
    hb->setSpacing(0);

    pop = new QMenu;
    pop->setFocusPolicy( Qt::NoFocus ); //don't reset IM

    mButtonStack = new QStackedWidget;

    mButton = new IMToolButton;
    mButtonStack->addWidget(mButton);

    mStatus = 0;

    hb->addWidget(mButtonStack);

    mButton->setFocusPolicy(Qt::NoFocus);
    mButton->setCheckable( true );
    mButton->setChecked( true );
    mButton->setAutoRaise( true );
    int sz = style()->pixelMetric(QStyle::PM_SmallIconSize);
    mButton->setIconSize(QSize(sz,sz));
    mButton->setBackgroundRole(QPalette::Button);

    connect(mButton, SIGNAL(toggled(bool)), this, SLOT(activateCurrent(bool)));

    mButton->setBackgroundRole(QPalette::Button);

    mChoice = new QToolButton(this);
    hb->addWidget(mChoice);

    mChoice->setBackgroundRole(QPalette::Button);
    mChoice->setIcon( generatePixmap() );
    int dpi = QApplication::desktop()->screen()->logicalDpiY();
    mChoice->setFixedWidth( qRound(7.0 * dpi / 100.0) );    // 7 pixels on a 100dpi screen
    mChoice->setAutoRaise( true );
    connect( mChoice, SIGNAL(clicked()), this, SLOT(showList()) );
    mChoice->hide();// until more than one.

    setLayout(hb);

    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    cfg.beginGroup("InputMethods");

    defaultIM = cfg.value("DefaultIM").toString(); // No tr

    // Add sensible defaults:
    if (defaultIM.isEmpty() && !Qtopia::mousePreferred())
        defaultIM = "Phone Keys"; // No tr

    qLog(Input) << "Default IM is "<< defaultIM;
}

/*!
    Destroys and cleans up this \l InputMethodSelector
*/
InputMethodSelector::~InputMethodSelector()
{
    // doesn't own anything other than children... do nothing.
}


/*!
    Add \a im to this InputMethodSelector.

    If this is the first or the default input method, this will
    cause it to become the current input method and become active.

    If this is the second input method added, this will activate
    the drop-down input-method selection \l QToolButton, making
    the input methods available to users via the drop-down input method list.

    Subsequent input methods added with this function are also added to this list.
*/
// before, need to set libName, iface, style.
void InputMethodSelector::add(QtopiaInputMethod *im)
{
    /* should check if im has a statusWidget()  */
    list.append(im);

    if (mCurrent == 0 || im->identifier() == defaultIM) {
        if( im->identifier() == defaultIM )
            qLog(Input) << "Found defaultIM "<< defaultIM << ", activating";
        mCurrent = list[list.count()-1];
        emit activated(mCurrent);

        updateStatusIcon();
    }
    if (count() > 1)
        mChoice->show();
}

/*!
    \fn void InputMethodSelector::activated(QtopiaInputMethod *im)
    This signal is emitted to notify the \l InputMethods object that \a im should be activated.
*/


/*!
    Updates the status \l QIcon.  This \l QIcon is displayed according to the theme, and may be hidden, especially if the input methods are not currently activated.

    The status icon is set to the \l QtopiaInputMethod::statusWidget() of the current \l QtopiaInputMethod.  If this is not available, then the \l QtopiaInputMethod::icon() is used, and if this is also not available, then an empty \l QIcon is used.
*/
void InputMethodSelector::updateStatusIcon()
{
    if (mCurrent) {
        if (mCurrent->statusWidget(this)) {
            if (mStatus)
                mButtonStack->removeWidget(mStatus);
            mStatus = mCurrent->statusWidget(this);
            mButtonStack->addWidget(mStatus);
            mButtonStack->setCurrentWidget(mStatus);
        } else {
            QIcon i = mCurrent->icon();
            mButton->setIcon(i);
            mButtonStack->setCurrentWidget(mButton);
        }
    } else {
        mButton->setIcon(QIcon());
        mButtonStack->setCurrentWidget(mButton);
    }
}

QPixmap InputMethodSelector::generatePixmap() const
{
    int dpi = QApplication::desktop()->screen()->logicalDpiY();
    int w = qRound(4.5 * dpi / 100.0);
    int h = qRound(3.5 * dpi / 100.0);
    
    QPixmap triangle(w,h);
    triangle.fill(Qt::transparent);
    QPainter painter(&triangle);
    painter.setRenderHint(QPainter::Antialiasing);
    QPolygon poly;
    poly << QPoint(0,0) << QPoint(w/2,h) << QPoint(w,0);
    painter.setPen(QApplication::palette().color(QPalette::ButtonText));
    painter.setBrush(QApplication::palette().buttonText());
    painter.drawPolygon(poly);
    painter.end();
    return triangle;
}

/*!
    This function notifies the InputMethodSelector that focus has changed.  It causes the input method menus to be reformulated.
    \sa updateIMMenuAction
*/
void InputMethodSelector::focusChanged(QWidget* old, QWidget* now)
{
    Q_UNUSED(old);
    Q_UNUSED(now);
    qLog(Input) << "void InputMethodSelector::focusChanged(QWidget* old, QWidget* now)";
    updateIMMenuAction(m_IMMenuActionAdded);
}

/*!
    Shows the list of input methods modally, relative to the choice \l QToolButton, and activates the selected input method, if any.
*/
void InputMethodSelector::showList()
{
    pop->clear();
    QMap<QAction *, QtopiaInputMethod *> map;
    foreach(QtopiaInputMethod *method, list) {
        QAction *a = pop->addAction(method->icon(), method->name());
        if ( mCurrent == method )
            a->setChecked( true );
        map.insert(a, method);
    }

    QPoint pt;
    QSize s = pop->sizeHint();

#if 0 // The following doesn't work until mapToGlobal works with QGraphicsView items
    bool rtl = QApplication::isRightToLeft();
    if ( !rtl )
        pt = mapToGlobal(mChoice->geometry().topRight());
    else
        pt = mapToGlobal(mChoice->geometry().topLeft());

    if (pt.y() > s.height()) // room to pop above?
        pt.ry() -= s.height();
    if ( !rtl )
        pt.rx() -= s.width();
#else // just center for now
    QDesktopWidget *desktop = QApplication::desktop();
    pt.setX((desktop->screenGeometry(desktop->primaryScreen()).width()-s.width())/2);
    pt.setY((desktop->screenGeometry(desktop->primaryScreen()).height()-s.height())/2);
#endif

    QAction *selected = pop->exec( pt );
    if ( selected == 0 )
        return;
    setInputMethod(map[selected]);
}

/*!
    Set the current input method to \a method and activate it.
    \sa activated()
*/
void InputMethodSelector::setInputMethod(QtopiaInputMethod *method)
{
    if (mCurrent != method) {
        activateCurrent(false);
        mCurrent = method;
        emit activated(mCurrent);
        updateStatusIcon();
        mButton->setChecked( true );
    }
    activateCurrent(true);
}

/*!
    Cycle to the next input method.
*/
void InputMethodSelector::setNextInputMethod()
{
    if(list.size() < 1)
        return;

    if(list.size() == 1) {
        if (list.first() != mCurrent)
            setInputMethod(list.first());
        return;
    }

    QList<QtopiaInputMethod *>::iterator  i = list.begin();
    while(i != list.end() && *i != mCurrent)
        i++;

    if(i != list.end()) { // implies i == mCurrent
        if(++i != list.end()) {
            setInputMethod(*i);
            return;
        }
    }

    setInputMethod(list.first());
}

/*!
    Set \a hint for all input methods.  If \a restricted is true the input
    method is restricted to the specified mode.
*/
void InputMethodSelector::setHint(const QString &hint, bool restricted)
{
    QList<QtopiaInputMethod *>::iterator it;
    for (it = list.begin(); it != list.end(); ++it)
        (*it)->setHint(hint, restricted);
}

/*!
    Cycle to the next input method.
*/
void InputMethods::setNextInputMethod(){
    selector->setNextInputMethod();
}

/*!
    Cycle to the next input method.
*/
void InputMethodService::setNextInputMethod()
{
    parent->setNextInputMethod();
}

/*!
    Turns the current input method on if \a on is true, or off if it is false.
*/
void InputMethodSelector::activateCurrent( bool on )
{
    if (mCurrent) {
        QWidget *w = mCurrent->inputWidget();
        qLog(Input) << (on?"activating":"deactivating") << "input method" << (ulong)mCurrent << ", with widget" << w;
        if ( on ) {
            updateIMMenuAction(true);
            if (w) {
                mCurrent->reset();
                //  Basic sanity checks on the size of the input method 
                //  widget.
                if(w->sizeHint().height() <= 0 || w->sizeHint().width() <= 0)
                    qLog(Input) << "Input Method Widget " << w->metaObject()->className() << " has sizeHint() with null height or width, and will not be visible";
                int height = w->sizeHint().height();

                QDesktopWidget *desktop = QApplication::desktop();
                w->resize(desktop->screenGeometry(desktop->primaryScreen()).width(), height );

                if(mButton->isChecked()) {
                    if (mCurrent->properties() & QtopiaInputMethod::DockedInputWidget)
                        WindowManagement::showDockedWindow(w);
                    else
                        w->show();
                    emit inputWidgetShown( on );
                };
            };
        } else {
            updateIMMenuAction(false);
            if(w) {
                if (mCurrent->properties() & QtopiaInputMethod::DockedInputWidget)
                    WindowManagement::hideDockedWindow(w);
                else
                    w->hide();
                emit inputWidgetShown( on );
            };
        }
    }
}

/*!
    \fn void InputMethodSelector::inputWidgetShown(bool shown)
    This signal is emitted when the visible state of the inputWidget is toggled.  \a shown reflects the new state of the widget, true if the widget is being shown, false if it is being hidden.
*/
/*!
    Deactivates and hides the current input method, and then sets no current input method.
*/
void InputMethodSelector::clear()
{
    pop->clear();
    mChoice->hide();
    mCurrent = 0;
    emit activated(mCurrent);
    updateStatusIcon();
    list.clear();
}

class QtopiaInputMethodSorter
{
    public:
        bool operator()(const QtopiaInputMethod *first, const QtopiaInputMethod *second) {
            if (!first && second)
                return true;
            if (first && second)
                return (first->name() < second->name());
            return false;
        }
};

/*!
    Sorts the loaded input methods alphabetically by name.
*/
void InputMethodSelector::sort()
{
    if (list.count() > 1) {
        qSort( list.begin(), list.end(), QtopiaInputMethodSorter() );
        updateStatusIcon();
    }
}

/*!
    \fn uint InputMethodSelector::count() const
    Returns the number of input methods that have been added to the InputMethodSelector.
*/


/*!
    Returns a pointer to the current input method.
*/
QtopiaInputMethod *InputMethodSelector::current() const
{
    return mCurrent;
}

/*!
    Selects the input method named \a name if it exists.
*/
void InputMethodSelector::setInputMethod(const QString &name)
{
    foreach(QtopiaInputMethod *method, list) {
        if (method->identifier() == name) {
            setInputMethod(method);
            break;
        }
    }
}

/*!
    A helper function for supporting input method menu QActions.
    Removes the current IM's QAction from all menus.
    If \a addToMenu is true, adds the QAction to the
    current focus widgets Menu.
*/
void InputMethodSelector::updateIMMenuAction(bool addToMenu)
{
    qLog(Input) << "Updating IM Menu actions" << (addToMenu?"- adding":"- removing");
    // Could have gotten here because input method changed menu options,
    // so set them again whether or not they're already shown
    if (m_IMMenuActionAdded) {
        qLog(Input) << "Removing valuespace Entry";
        m_menuVS.removeAttribute("MenuItem");
        m_IMMenuActionAdded = false;
    };

    if( addToMenu ){
        QList<QIMActionDescription*> actionDescriptionList  = mCurrent->menuDescription();

        if(count() > 1)
        {
            actionDescriptionList.append(new QIMActionDescription(InputMethods::NextInputMethod, tr("Change Input Method"),QString(":icon/rotate")));
        }

        if(!actionDescriptionList.isEmpty()){
            QList<QVariant> imMenu;
            qLog(Input) << "Building imMenu";
            for(QList<QIMActionDescription*>::const_iterator i = actionDescriptionList.begin(); i != actionDescriptionList.end(); ++i) {
                imMenu.append(QVariant::fromValue(**i));
                delete *i;
            };

            qLog(Input) << "Adding valuespace Entry";
            m_menuVS.setAttribute("MenuItem", QVariant(imMenu));
            m_IMMenuActionAdded = true;
        };
    };
};

/*!
    Shows  the input method selector choice button if \a on is true, otherwise hides it.
*/
void InputMethodSelector::showChoice( bool on)
{
    if(on){
        mChoice->show();
    }
    else {
        mChoice->hide();
    };
};


/*!
  \class InputMethods
  \inpublicgroup QtInputMethodsModule
  \brief The InputMethods class provides an implementation of Qt Extended server input method handling.
  \ingroup QtopiaServer::InputMethods

    InputMethods is the core class for the Qt Extended servers input method handling.
    It is very closely related to the \l {InputMethodService}{InputMethod} service and \l InputMethodSelector classes.
 
    InputMethods is primarily resposible for loading input method plugins and maintaining the hints 
    set for different widgets. It also acts on the messages from the \l {InputMethodService}{InputMethod} service, either taking 
    direct action or passing them on to the  \l InputMethodSelector.
    
    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Create a new InputMethods object with \a parent and \a flags that handles input according to \a t.
*/
InputMethods::InputMethods( QWidget *parent, Qt::WFlags flags, IMType t ) :
    QWidget( parent, flags ),
    loader(0), type(t),
#ifdef Q_WS_QWS
    currentIM(0),
#endif
    lastActiveWindow(0), m_IMVisibleVS("/UI/IMVisible"), m_IMVisible(false)
{
    // Start up the input method service via \l{Qt Extended IPC Layer}{IPC}.
    new InputMethodService( this );

    //overrideWindowFlags(Qt::Tool);
    setObjectName("InputMethods");
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    setLayout(hbox);

    selector = new InputMethodSelector;
    hbox->addWidget(selector);

    connect(selector, SIGNAL(activated(QtopiaInputMethod*)),
            this, SLOT(choose(QtopiaInputMethod*)));

    connect(selector, SIGNAL(inputWidgetShown(bool)),
            this, SIGNAL(inputToggled(bool)));

    loadInputMethods();

#ifdef Q_WS_QWS
    connect( qwsServer, SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
            this, SLOT(updateHintMap(QWSWindow*,QWSServer::WindowEvent)));
#endif
}

/*!
    \fn InputMethods::inputToggled(bool on)
    Connect to this signal to recieve notifications of changes in the state of input methods. \a on describes whether the input methods have just turned on (true), or off (false).
*/

/*!
    \fn void InputMethods::visibilityChanged(bool visible)
    Connect to this signal to receive notification of changes in the visibility of input methods.  This is synonymous with activation, input methods do not need to have an input widget to be visible in this sense. \a visible is true if the input method is becoming visible/activating, false if it becoming invisible/deactivating.
*/

/*!
    Unloads all input methods.
*/
InputMethods::~InputMethods()
{
    unloadInputMethods();
}

/*!
    \enum InputMethods::IMType
    This enum describes which types of input an input method processes.
    \value Any    The input method can process both mouse and keypad input
    \value Mouse    The input method processes mouse or pointer input.
    \value Keypad   The input method processes keypad input.
*/

/*!
    \enum InputMethods::SystemMenuItemId
    This enum defines values that input method menu items can return to activate system actions based for these menu items.
    \value  NextInputMethod This value results in the input method being changed to the next input method in alphabetic order.
    \value ChangeInputMethod    This value causes the input method selection menu to be shown, and to recieve keyboard focus, facilitating changing input methods with the keyboard.
*/

/*!
    Deactivates and hides the current input method.
*/
void InputMethods::hideInputMethod()
{
    selector->activateCurrent(false);
}

/*!
    Shows and activates the current input method.
*/
void InputMethods::showInputMethod()
{
    selector->activateCurrent(true);
}

/*!
    Selects the the input method named \a name, if it exists, and activates and shows it.  If the named input method does not exist, this function activates the current input method.
*/
void InputMethods::showInputMethod(const QString& name)
{
    selector->setInputMethod(name);
    selector->activateCurrent(true);
}

/*!
    Checks whether the activated menu item belonged to the server, and either responds or passes the information on to the current input method appropriately.

    Server menu items typically have values less than 0, so \a v is generally passed to the current input method if it is greater than or equal to 0.
    \sa QtopiaInputMethod::menuActionActivated()
*/
void InputMethods::activateMenuItem(int v)
{
    // Check for server items
    if(v == ChangeInputMethod) {// client menu items
        selector->showList();
        return;
    }
    if(v == NextInputMethod) {// client menu items
        setNextInputMethod();
        return;
    }

    // pass to current input method
    selector->current()->menuActionActivated(v);
}

/*!
    Set system-wide description for allowable inputmethods to the given \a type.
    \i {Note}: Triggers a reloading of all inputmethod plugins
*/

void InputMethods::setType(IMType type)
{
    if(type != this->type){
        this->type=type;
        loadInputMethods();
    };
};

void InputMethods::resetStates()
{
    // just the current ones.
    if (selector->current())
            selector->current()->reset();
}

/*!
    Returns the geometry of the current input methods input widget, or an empty \l QRect if there is no current input method, or no input widget for the current input method.
*/
QRect InputMethods::inputRect() const
{
    if (selector->current()) {
        QWidget *w = selector->current()->inputWidget();
        if (w && w->isVisible())
            return w->geometry();
    }
    return QRect();
}

/*!
    Clears the current input method, and clears and destroys all previously loaded input methods.  Called from InputMethods destructor.
*/
void InputMethods::unloadInputMethods()
{
#ifdef Q_WS_QWS
    if (currentIM) {
        QWSServer::setCurrentInputMethod( 0 );
        currentIM = 0;
    }
#endif
    if ( loader ) {
        selector->clear();
        ifaceList.clear();
        delete loader;
        loader = 0;
    }
}

/*!
    Creates a QPluginManager for input methods to load input method plugins out of the \c{ plugins/inputmethods} directory.

    If InputMethods already has a QPluginManager, this function does nothing.
*/
void InputMethods::loadInputMethods()
{
    if(loader) {
        qLog(Input) << "Warning : loadInputMethods() erroneously called twice";
        return;
    }

    selector->blockSignals(true);
#ifndef QT_NO_COMPONENT
    hideInputMethod();
    unloadInputMethods();

    loader = new QPluginManager( "inputmethods" );

    if ( !loader ){
        qLog(Input) << "Missing inputmethods loader";
        selector->blockSignals(false);
        return;
    }

    foreach ( QString name, loader->list() ) {
        qLog(Input) << "Loading IM: "<<name;
        QObject *loaderInstance = loader->instance(name);
        if ( !loaderInstance ){
            qLog(Input) << "Missing loader loaderInstance";
            continue;
        }
        QtopiaInputMethod *plugin = qobject_cast<QtopiaInputMethod*>(loaderInstance);
        if ( plugin ) {
            bool require_keypad = plugin->testProperty(QtopiaInputMethod::RequireKeypad);
            bool require_mouse = plugin->testProperty(QtopiaInputMethod::RequireMouse);
            if (type == Keypad && require_mouse || type == Mouse && require_keypad) {
                delete loaderInstance;
            } else {
                QWidget *w = plugin->inputWidget();
                if (w) {
                    w->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );
                    w->setAttribute( Qt::WA_GroupLeader );
                    if(plugin->testProperty(QtopiaInputMethod::InputWidget) &&
                    plugin->testProperty(QtopiaInputMethod::DockedInputWidget))
                    {
                        qLog(Input) << "Docking input widget for "<<plugin->name();
                        WindowManagement::dockWindow(w, WindowManagement::Bottom);
                    }
                }
                selector->add(plugin);
                ifaceList.append(loaderInstance);
                connect(plugin, SIGNAL(stateChanged()), this, SLOT(updateIMVisibility()));
            }
        } else {
            delete loaderInstance;
        }
    }

    selector->sort();
#else

    if (type == Mouse) {
        // some check for which are avail?
        selector.add(new HandwritingInputMethod());
        selector.add(new KeyboardInputMethod());
        selector.add(new UnicodeInputMethod());
    }
    // } else if (type == Keypad) {
    // and the any?
#endif

#ifdef Q_WS_QWS
    if ( selector->current()) {
        //keyBased->show?
        QtopiaInputMethod *imethod = selector->current();
        currentIM = imethod->inputModifier();
        QWSServer::setCurrentInputMethod( currentIM );
    } else {
        currentIM = 0;
    }
#endif

    updateIMVisibility();
    selector->blockSignals(false);
}

void InputMethods::updateIMVisibility()
{
    // hide... negates chance to interact with non input widget input methods...
    // Should never be true? True for only certain states?
    bool imVisible = false;
    bool choiceVisible = selector->count() > 1;

    // Only show the selector if there are at least 2 input methods,
    // and the current window has some kind of input hint.
    // Unless the current input method has an interactive widget, in which
    // case show it anyway. (Is this valid?)

    //the logic is:
    //bool shouldHideSelector=false;
    //if(hintMap[lastActiveWindow] == "") shouldHideSelector = true;
    //if(c && c->testProperty(QtopiaInputMethod::InteractiveIcon))  shouldHideSelector = false;  //this one might be wrong
    if (( !lastActiveWindow
                || !hintMap.contains(lastActiveWindow)
                || hintMap[lastActiveWindow].isEmpty())
            && (!selector->current() || !selector->current()->testProperty(QtopiaInputMethod::InteractiveIcon))) {
        // imVisible = false;
    }
    else {
        imVisible = true;
    }

    // To avoid double paints, update the choice widgets visibility before the
    // selector if the selector is being shown, and afterwards if the selector
    // is being hidden, whether or not the selectors visibility is actually
    //changing.
    if(imVisible) {
        // If the selector is visible, or about to become so, show 
        // the choice widget immediately.
        selector->showChoice( choiceVisible );
    };

    if( imVisible != m_IMVisible ) {
        m_IMVisibleVS.setAttribute( "", QVariant(imVisible));
        m_IMVisible = imVisible;
        selector->activateCurrent(imVisible);
        emit visibilityChanged(imVisible);
    }

    if(!imVisible) {
        selector->showChoice( choiceVisible );
    }
    selector->refreshIMMenuAction();

}

void InputMethods::choose(QtopiaInputMethod* imethod)
{
#ifdef Q_WS_QWS
    if ( imethod ) {
        currentIM = imethod->inputModifier();
        QWSServer::setCurrentInputMethod( currentIM );
    } else if (currentIM) {
        QWSServer::setCurrentInputMethod( 0 );
        currentIM = 0;
    }
#else
    Q_UNUSED(imethod);
#endif
}

/*!
    Sets the input hint for the current input method for widget \a wid.  \a h is the hint for the input method, according to \l QtopiaApplication::InputMethodHint.  The \a password flag sets password mode, indicating that input should be hidden after being entered, and defaults to false.
    This value is recorded for when this widget gains focus in the future, and if \a wid refers to the last active widget, the hint is forwarded to the current input method immediately.
*/
void InputMethods::inputMethodHint( int h, int wid, bool password)
{

    if (hintMap.contains(wid)) {
        switch (h) {
            case (int)QtopiaApplication::Number:
                hintMap[wid] = "int";
                break;
            case (int)QtopiaApplication::PhoneNumber:
                hintMap[wid] = "phone";
                break;
            case (int)QtopiaApplication::Words:
                hintMap[wid] = "words";
                break;
            case (int)QtopiaApplication::Text:
                hintMap[wid] = "text";
                break;
            case (int)QtopiaApplication::ProperNouns:
                hintMap[wid] = "propernouns";
                break;
            default:
                hintMap[wid] = QString();
                break;
        }
    }
    if(password)
        hintMap[wid] += " password";
    if (wid && wid == lastActiveWindow)
    {
        updateHint(wid);
    }
}

/*!
    Sets the input hint for widget \a wid, according to the hint \a h.  This information is recorded for this widget should it gain focus, and if \a wid refers to the last active window, the hint is immediately forwarded to the current input method.
*/
void InputMethods::inputMethodHint( const QString& h, int wid )
{
    // Could easily scan this here and act on it to change inputmethods...
    bool r;
    if (h.contains("only")) {
        r = true;
    } else {
        r = false;
    }
    if (hintMap.contains(wid))
        hintMap[wid] = h;
    if (restrictMap.contains(wid))
        restrictMap[wid] = r;
    if (wid && wid == lastActiveWindow)
        updateHint(wid);
}

/*!
    Sets the password hint for \a wid to \a passwordFlag, provided an input hint has previously been set for that widget.  If no hint has been set for that widget previously, this function does nothing.
    \sa inputMethodHint()
*/
void InputMethods::inputMethodPasswordHint(bool passwordFlag, int wid)
{
    if(!hintMap.contains(wid))
    {
        // TODO: find default hint.
        // In the meantime, a normal sethint must be called before
        // setting password
        return;
    };

    int stringindex =  hintMap[wid].indexOf("password");
    if(!passwordFlag && stringindex!=-1)
    {
        hintMap[wid].remove("password");

        if(stringindex > 0 && hintMap[wid].at(stringindex-1)==' ')
            hintMap[wid].remove(stringindex-1,1);

    } else if (passwordFlag && stringindex == -1)
    {
         hintMap[wid].append(" password");
    }
    if (wid && wid == lastActiveWindow)
        updateHint(wid);
}

#ifdef Q_WS_QWS

void InputMethods::updateHintMap(QWSWindow *w, QWSServer::WindowEvent e)
{
    if (!w)
        return;
    // one signal can be multiple events.
    if ((e & QWSServer::Create) == QWSServer::Create) {
        if (!hintMap.contains(w->winId()))
            hintMap.insert(w->winId(), QString());
        if (!restrictMap.contains(w->winId()))
            restrictMap.insert(w->winId(), false);
    } else if ((e & QWSServer::Destroy) == QWSServer::Destroy) {
        if (hintMap.contains(w->winId()))
            hintMap.remove(w->winId());
        if (restrictMap.contains(w->winId()))
            restrictMap.remove(w->winId());
    }

    if ( (e & QWSServer::Active) == QWSServer::Active
         && w->winId() != lastActiveWindow)  {
        lastActiveWindow = w->winId();
        updateHint(lastActiveWindow);
    }
}

#endif // Q_WS_QWS

/* TODO: Also... if hint null, don't just set the hint, remove the IM/GM
   Not a problem now, (well behaved plugins) but should be done for
   misbehaved plugins.
 */
void InputMethods::updateHint(int wid)
{
    selector->setHint(hintMap[wid], restrictMap[wid]);
    updateIMVisibility();
}

/*!
    Returns true if the current input method is active and visible, otherwise returns false.
*/
bool InputMethods::shown() const
{
    return selector->current() && selector->current()->inputWidget() && selector->current()->inputWidget()->isVisible();
}

/*!
    Returns the name of the current input method, or an empty QString if there is no current input method.
    \sa QtopiaInputMethod::name()
*/
QString InputMethods::currentShown() const
{
    return shown() ? selector->current()->name() : QString();
}

/*!
    Returns true if the input method selector is visible, false otherwise.
    This indicates the visibility of the widget used to change input methods, not the drop-down menu itself.  In Qtopia, this widget is only visible if at least 2 input methods have been loaded, and the widget with focus accepts text input.
    \sa InputMethodSelector::isVisible(), InputMethodSelector::count()
*/
bool InputMethods::selectorShown() const
{
    return selector->isVisible();
}

/*!
    \service InputMethodService InputMethod
    \inpublicgroup QtInputMethodsModule
    \brief The InputMethodService class provides the InputMethod service.

    The \i InputMethod service enables applications to adjust the input
    method that is being used on text entry fields.

    The messages in this service are sent as ordinary QtopiaIpc messages
    on the \c{QPE/InputMethod} channel.  The service is provided by
    the Qt Extended server.

    Normally applications won't need to send these messages directly,
    as they are handled by methods in the QtopiaApplication class.

    \sa QtopiaApplication, QtopiaIpcEnvelope
*/

/*!
    \internal
*/
InputMethodService::InputMethodService( InputMethods *parent )
    : QtopiaIpcAdaptor( "QPE/InputMethod", parent )
{
    this->parent = parent;
    publishAll(Slots);
}

/*!
    \internal
*/
InputMethodService::~InputMethodService()
{
    // Nothing to do here.
}

/*!
    Set the input method \a hint for \a windowId.  The valid values
    for \a hint are specified in QtopiaApplication::InputMethodHint.
    This method should not be used if the \c Named hint is requested.

    This slot corresponds to the \l{Qt Extended IPC Layer}{IPC} message
    \c{inputMethodHint(int,int)} on the \c QPE/InputMethod channel.
*/
void InputMethodService::inputMethodHint( int hint, int windowId )
{
    parent->inputMethodHint( hint, windowId );
}

/*!
    Set the input method \a hint for \a windowId.  This method should
    be used for \c Named hints.

    This slot corresponds to the \l{Qt Extended IPC Layer}{IPC} message
    \c{inputMethodHint(QString,int)} on the \c QPE/InputMethod channel.
*/
void InputMethodService::inputMethodHint( const QString& hint, int windowId )
{
    parent->inputMethodHint( hint, windowId );
}

/*!
    Set the input method \a passwordFlag for \a windowId.
*/

void InputMethodService::inputMethodPasswordHint(bool passwordFlag, int windowId)
{
    parent->inputMethodPasswordHint(passwordFlag, windowId);
};

/*!
    Explicitly hide the current input method.

    The current input method may still indicated in the taskbar, but no
    longer takes up screen space, and can no longer be interacted with.

    This slot corresponds to the \l{Qt Extended IPC Layer}{IPC} message
    \c{hideInputMethod()} on the \c QPE/InputMethod channel.

    \sa showInputMethod()
*/
void InputMethodService::hideInputMethod()
{
    parent->hideInputMethod();
}

/*!
    Explicitly show the current input method.

    Input methods may be indicated in the taskbar by a small icon. If the
    input method is activated (shown) then it takes up some proportion
    of the bottom of the screen, to allow the user to interact (input
    characters) with it.

    This slot corresponds to the \l{Qt Extended IPC Layer}{IPC} message
    \c{showInputMethod()} on the \c QPE/InputMethod channel.

    \sa hideInputMethod()
*/
void InputMethodService::showInputMethod()
{
    parent->showInputMethod();
}

/*!
    \internal
*/
void InputMethodService::activateMenuItem(int v)
{
    parent->activateMenuItem(v);
}

/*!
    Activate the input method selector choice pop-up (a menu of available input methods), and give it keyboard focus.
*/

void InputMethodService::changeInputMethod()
{
    parent->changeInputMethod();
}

/*!
    Activate the input method selector choice pop-up (a menu of available input methods), and give it keyboard focus.
*/

void InputMethods::changeInputMethod()
{
    selector->showList();
};

/*!
    If actions have been added to the softmenu on behalf of an IM, make
    sure they are up to date.

    \sa QtopiaInputMethod::stateChanged()
*/
void InputMethodSelector::refreshIMMenuAction()
{
    updateIMMenuAction(m_IMMenuActionAdded);

};

/*!
    Change to the input method named \a inputMethodName if it exists.
*/
void InputMethodService::setInputMethod(const QString &inputMethodName)
{
    parent->showInputMethod(inputMethodName);
};

/*!
    \internal
    Loads all input method plugins.  If any input method plugins have already been loaded, this function does nothing.
    As input method plugins are loaded automatically on startup, this will generally only be of any use after calling \l unloadInputMethods, which should never be used on a device.
    Use
    \code
        $QPEDIR/src/tools/qcop/qcop send "QPE/InputMethod" "loadInputMethods()"
    \endcode
    \sa unloadinputMethods(), InputMethods::unloadInputMethods()
*/
void InputMethodService::loadInputMethods()
{
    parent->loadInputMethods();
};

/*!
    \internal
    Unloads all input method plugins.
    This function is extremely dangerous.  Unloading input methods that are in use can easily cause a segmentation fault.
    This function is only intended as a convenience while developing input method plugins, and should never be called on a device.
    You have been warned.

    Use:
    \code{$QPEDIR/src/tools/qcop/qcop send "QPE/InputMethod" "unloadInputMethods()" \endcode

    \sa loadinputMethods(), InputMethods::loadInputMethods()
*/

void InputMethodService::unloadInputMethods()
{
    parent->unloadInputMethods();
};

UIFACTORY_REGISTER_WIDGET( InputMethods );
