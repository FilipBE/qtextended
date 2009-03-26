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

#include "qsoftkeylabelhelper.h"
#include "qsoftkeylabelhelper_p.h"
#include <QObject>
#include <QWidget>
#include <QInputContext>
#include <QEvent>
#include <QInputMethodEvent>
#include <QTextEdit>

#define QSOFTKEYLABELHELPER_WARNINGS

/*!
    \class QAbstractSoftKeyLabelHelper
    \inpublicgroup QtBaseModule

    \internal

    \brief The QAbstractSoftKeyLabelHelper class provides an abstract base class for the softkey helpers, assisting in ensuring that the qsoftkey labels show the correct icons or words.

    This class handles most of the interactions with the system, including notifications of changes to the widget being "helped".

    QAbstractSoftkeyLabelHelper provides support for all other SoftKeyLabel helpers.  It automatically registers itself and its widget or class with the ContextKeyManager, as well as ensuring that it is destroyed in a timely fashion.

    It also connects several standard messages for use by its child classes, allowing these classes to simply set the correct buttons when the widget focuses in or out, or is asked to.

    There are two different ways to create any member of the QSoftkeyHelper family, and two corresponding modes of use.  QSoftkeyHelpers in general can be created on either a widget, taking a pointer to that widget, or on a class, taking a QString of the name of the class.

    In either case, QAbstractSoftKeyLabelHelper ensures that the the helper automatically connects up typical messages, automatically registers with the server, and automatically destorys itself at an appropriate time.  As a consequence, objects of this class can typically be instatiated on the heap and then forgotten about.

    For example, to instantiate a MySoftkeyLabelHelper derived from QAbstractSoftKeyLabelHelper on a specific widget use:
    \code
    MyWidget* myWidget = new MyWidget();
    new MySoftkeyLabelHelper(myWidget);
    \endcode

    To instantiate the same helper on all widgets of the MyWidget class use:

    \code
    new MySoftkeyLabelHelper("MyWidget");
    \endcode

    Whenever any MyWidget gains focus, the sofkey helper is notified as appropriate.

    Simple classes derived from QAbstractSoftKeyLabelHelper will typically need to implement only their constructor/destructor, \l focusIn() and \l updateAllLabels(), and will simply set appropriate keys according to the state of the widget being "helped". See \l QSoftKeyLabelHelper for an example of this sort of class used in Qtopia.

    More complex classes may also need to implement \l focusOut(), \l enterEditFocus() and \l leaveEditFocus(), as well as currentWidgetChangeNotification().  For example \l QLineEditSoftKeyLabelHelper installs an event filter during enterEditocus() in order to react appropriately to \l{QInputMethodEvent}s, and removes the event filter in \l leaveEditFocus(), and also clears its internal state when recieving \l currentWidgetChangeNotification().  For more information on this more complex example see \l QLineEditSoftKeyLabelHelper.
*/

/*!
    Construct a QAbstractSoftKeyLabelHelper on \a widget and set up connections appropriately.  Also connect the \l destroyed() signal of \a widget to the deleteLater() slot of this QAbstractSoftKeyLabelHelper, so that it is destroyed at the right time.
*/
QAbstractSoftKeyLabelHelper::QAbstractSoftKeyLabelHelper(QWidget* widget) : m_widget(0), m_className()
{
    d = new QSoftKeyLabelHelperPrivate(this);
    if(widget){
        connect(widget, SIGNAL(destroyed()), this, SLOT(deleteLater()));
        setCurrentWidget(widget);
    };
};

/*!
    Construct a QAbstractSoftKeyLabelHelper on behalf of all classes implementing or derived from \a className.
*/

QAbstractSoftKeyLabelHelper::QAbstractSoftKeyLabelHelper(const QString &className) : m_widget(0)
{
    d = new QSoftKeyLabelHelperPrivate(this);
    m_className = className;

    ContextKeyManager::instance()->setContextKeyHelper(className, this);
};

/*!
    Destroy the QAbstractSoftKeyLabelHelper and clean up.  Also deregister any widgets or classes with the server.
*/
QAbstractSoftKeyLabelHelper::~QAbstractSoftKeyLabelHelper()
{
    if(d){
        // event filters are automatically removed
        delete d;
        d =0;
    }
    else {
        qWarning("QSoftKeyLabelHelper private data deleted more than once");
    }
    if(m_widget) {
        ContextKeyManager::instance()->clearContextKeyHelper(qobject_cast<QWidget*>(m_widget));
    };
    if(!m_className.isEmpty()) {
        ContextKeyManager::instance()->clearContextKeyHelper(m_className);
    };
};


/*!
    Returns the className that this helper is for, or an empty string if this helper is acting on behalf of a specific widget.
*/
QString QAbstractSoftKeyLabelHelper::className() const
{
    return m_className;
};

/*!
    Set the class for a softkey helper to \a className.

    This is intended as a once-off function used once after creating a Softkey Helper.

    Changing classes and reusing helpers should work in theory, but it is recommended that a new one is instantiated for each class and old ones are destroyed when no longer needed.
*/
void QAbstractSoftKeyLabelHelper::setClass(const QString &className)
{
    if(!m_className.isEmpty()) {
        ContextKeyManager::instance()->clearContextKeyHelper(m_className);
    };

    if(m_widget)
    {
        ContextKeyManager::instance()->clearContextKeyHelper(m_widget);
        disconnect(m_widget, SIGNAL(destroyed()), this, SLOT(deleteLater()));
        m_widget = 0;
    }

    m_className = className;
    ContextKeyManager::instance()->setContextKeyHelper(m_className,this);
    // CHECK: Should this be connected to the app instead? If so,
    // the ContextKeyManager needs to be notified to remove this helper too
    connect(ContextKeyManager::instance(), SIGNAL(destroyed()), this, SLOT(deleteLater()));

    return;
};

/*!
    \fn void QAbstractSoftKeyLabelHelper::updateAllLabels(QWidget* widget)
    Implement this function to update all SoftKey labels for \a widget.

    Use \l setStandardLabel() to set the labels themselves.

    See \l QSoftKeyLabelHelper for a simple example, and QLineEditSoftKeyHelper::updateAllLabels() for a more complex example.

    \sa setStandardLabel(), setStandardLabel(), setLabelText(), setLabelPixmap(), clearLabel(), QSoftKeyLabelHelper::updateAllLabels(), QLineEditSoftKeyLabelHelper::updateAllLabels()
*/

/*!
    This function is called by the server when focus moves to a widget that implements this helpers className (or inherits from that class).  QAbstractSoftKeyLabelHelper sets up appropriate notifications for \a newWidget.
*/
void QAbstractSoftKeyLabelHelper::setCurrentWidget(QWidget* newWidget)
{
    if(newWidget == m_widget) {
        return;
    };

    // widget actually changing, so break old connections.
    QWidget* oldWidget = m_widget;
    if(oldWidget) {
        m_widget->removeEventFilter(d);
        ContextKeyManager::instance()->clearContextKeyHelper(oldWidget);
    };

    // check for destruction links.

    m_widget = newWidget;
    if(m_widget) {
        m_widget->installEventFilter(d);
        if(m_widget->isWidgetType()){
            ContextKeyManager::instance()->setContextKeyHelper(qobject_cast<QWidget*>(m_widget), this);
        } else {
#ifdef QSOFTKEYLABELHELPER_WARNINGS
            qWarning() << "QAbstractSoftKeyLabelHelper incorrectly instantiated on non-widget QObject";
#endif //QSOFTKEYLABELHELPER_WARNINGS
        };
    };

    this->currentWidgetChangeNotification(m_widget, oldWidget);

    // Simulate missed signals if any:
    if(m_widget->hasEditFocus()) {
        enterEditFocus(m_widget);
    } else if(m_widget->hasFocus()) {
            focusIn(m_widget);
    };

}

/*!
    This function is called when focus changes to a widget under this QAbstractSoftKeyLabelHelpers juristiction. \a newWidget is the widget gaining focus, and \a oldWidget is the widget that just lost focus.

    Implement this function to perform any initialisation/state reset necessary when changing widgets. Connections and event filters should be removed on focus-out, so this will generally only be necessary if the helper keeps state for some reason.

    This function is called after the QAbstractSoftKeyLabelHelper registers various notifications, but before the the helper recieves any pending focusIn() or enterEditFocus() notifications.

    \sa focusIn(), enterEditFocus()
*/
void QAbstractSoftKeyLabelHelper::currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget)
{
    Q_UNUSED(newWidget);
    Q_UNUSED(oldWidget);
};

/*!
    Returns the widget the helper is currently acting for.  If the helper was instatiated on a widget, this will always be that widget.  If the helper was instatiated on a class, this will be the widget implementing or derived from that class that most recently had focus.

    \bold{Note:} A non-zero value does not imply that that widget currently has focus.
*/
QWidget* QAbstractSoftKeyLabelHelper::currentWidget() {
    return m_widget;
};

/*!
    \fn QAbstractSoftKeyLabelHelper::focusIn(QWidget* widget)

    Implement this function in a derived class to respond to gaining focus on the current widget \a widget.  This is typically where any necessary connections should be made or eventFilters installed

    The return value is not used by the system, and derived functions are free to utilise it as convenient.

    \sa QObject::connect(), QObject::installEventFilter(), focusOut(), enterEditFocus(), leaveEditFocus()
*/
/*!
    Implement this function in a derived class to respond to losing focus on the current widget \a widget.  The Default implementation returns false.
    \sa focusIn(), enterEditFocus(), leaveEditFocus()
*/
bool QAbstractSoftKeyLabelHelper::focusOut(QWidget* widget) {
    Q_UNUSED(widget);
    return false;
};

/*!
    Implement this function in a derived class to respond to gaining edit focus on the current widget \a widget.  The Default implementation returns false.
    \sa focusIn(), focusOut(), leaveEditFocus()
*/
bool QAbstractSoftKeyLabelHelper::enterEditFocus(QWidget* widget) {
    Q_UNUSED(widget);
    return false;
};

/*!
    Implement this function in a derived class to respond to losing edit focus on the current widget \a widget.  The Default implementation returns false.
    \sa focusIn(), focusOut(), enterEditFocus()
*/
bool QAbstractSoftKeyLabelHelper::leaveEditFocus(QWidget* widget) {
    Q_UNUSED(widget);
    return false;
};

// Label manipulation functions
/*!
    Set the label for \key to the standard Qt Extended label \a label.

    Qt::Key_Select and Qt::Key_Back are the most common values for \a key.  Qt::Key_Menu is used to change the third softkey, but this is uncommon in Qtopia, as this key is usually used for the soft menu and left unchanged.
    \sa setLabelText(), setLabelPixmap(), clearLabel()
*/
void QAbstractSoftKeyLabelHelper::setStandardLabel(int key,  QSoftMenuBar::StandardLabel label) {
    ContextKeyManager* instance = ContextKeyManager::instance();
    if(instance) {
        instance->setStandard(m_widget, key, label);
    };
};

/*!
    Set the label for \key to string \a text.

    Qt::Key_Select and Qt::Key_Back are the most common values for \a key.  Qt::Key_Menu is used to change the third softkey, but this is uncommon in Qtopia, as this key is usually used for the soft menu and left unchanged.

    \sa setStandardLabel(), setLabelPixmap(), clearLabel()
*/
void QAbstractSoftKeyLabelHelper::setLabelText(int key, const QString &text) {
    ContextKeyManager* instance = ContextKeyManager::instance();
    if(instance) {
        instance->setText(m_widget, key, text);
    };
};

/*!
    Set the label for \key to the pixmap named by \a pm.

    Qt::Key_Select and Qt::Key_Back are the most common values for \a key.  Qt::Key_Menu is used to change the third softkey, but this is uncommon in Qtopia, as this key is usually used for the soft menu and left unchanged.

    \sa setStandardLabel(), setLabelText(), clearLabel()
*/
void QAbstractSoftKeyLabelHelper::setLabelPixmap(int key, const QString &pm) {
    ContextKeyManager* instance = ContextKeyManager::instance();
    if(instance) {
        instance->setPixmap(m_widget, key, pm);
    };
};

/*!
    Clear the label for \key.  This is the same as setting the label to QSoftMenuBar::NoLabel
    Qt::Key_Select and Qt::Key_Back are the most common values for \a key.  Qt::Key_Menu is used to change the third softkey, but this is uncommon in Qtopia, as this key is usually used for the soft menu and left unchanged.

    \sa setStandardLabel(), setLabelText(), setLabelPixmap()
*/
void QAbstractSoftKeyLabelHelper::clearLabel(int key) {
    ContextKeyManager* instance = ContextKeyManager::instance();
    if(instance) {
        instance->clearLabel(m_widget, key);
    };
};


/*!
    \internal
*/
bool QSoftKeyLabelHelperPrivate::eventFilter(QObject* watched, QEvent *event)
{
    if(watched != q->m_widget)
#ifdef QSOFTKEYLABELHELPER_WARNINGS
        qWarning() << "Warning: QSoftKeyLabelHelperPrivate event filter on "<<watched<<", not on m_widget "<<q->m_widget;
#endif //QSOFTKEYLABELHELPER_WARNINGS
    QWidget* watchedWidget = qobject_cast<QWidget*>(watched);
    if(watchedWidget)
    {
        switch (event->type())
        {
            case QEvent::FocusIn:
                q->focusIn(watchedWidget);
                break;
            case QEvent::FocusOut:
                q->focusOut(watchedWidget);
                break;
            case QEvent::EnterEditFocus:
                q->enterEditFocus(watchedWidget);
                break;
            case QEvent::LeaveEditFocus:
                q->leaveEditFocus(watchedWidget);
                break;
            default:
                break;
        }
    }
    // never consume events
    return false;
}

/*!
    \class QSoftKeyLabelHelper
    \inpublicgroup QtBaseModule
    \internal
    \brief The QSoftKeyLabelHelper class is a softkey helper suitable for most widgets that do not use edit focus.

    When a widget it is helping gains focus, this helper sets the select key to the generic select label, and the back key to the generic back label.

    \sa QAbstractSoftKeyLabelHelper()
*/

/*!
    Instantiate a QSoftKeyLabelHelper on a specific widget \a widget.
*/
QSoftKeyLabelHelper::QSoftKeyLabelHelper(QWidget* widget) : QAbstractSoftKeyLabelHelper(widget)
{
};

/*!
    Instatiate a QSoftKeyLabelHelper on all widgets that implement or inherit the class \a className.
*/
QSoftKeyLabelHelper::QSoftKeyLabelHelper(const QString &className) : QAbstractSoftKeyLabelHelper(className)
{
    QSoftMenuBar::StandardLabel lbl = QSoftMenuBar::EndEdit;
    ContextKeyManager::instance()->setClassStandardLabel(QByteArray().append(className), Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
    ContextKeyManager::instance()->setClassStandardLabel(QByteArray().append(className), Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
};

/*!
    Destory and clean up the QSoftKeyLabelHelper.
*/
QSoftKeyLabelHelper::~QSoftKeyLabelHelper()
{
};

/*!
    Sets the softkey labels according to \l updateAllLabels(), and always returns true.
    \a widget is the widget that is gaining focus.
*/
bool QSoftKeyLabelHelper::focusIn(QWidget *widget)
{
    Q_UNUSED(widget);
    updateAllLabels(widget);
    return true;
}

/*!
    For this helper, this function does nothing, and returns false.
    \a widget is ignored.
*/
bool QSoftKeyLabelHelper::focusOut(QWidget *widget)
{
    Q_UNUSED(widget);
    return false;
}

/*!
    For this helper, this function does nothing, and returns false.
    \a widget is ignored.
*/
bool QSoftKeyLabelHelper::enterEditFocus(QWidget *widget)
{
    Q_UNUSED(widget);
    return false;
};

/*!
    For this helper, this function does nothing, and returns false.
    \a widget is ignored.
*/
bool QSoftKeyLabelHelper::leaveEditFocus(QWidget *widget)
{
    Q_UNUSED(widget);
    return false;
};

/*!
    Sets the select key to the generic select label, and the back key to the generic back label.  \a widget is the widget that currently has focus.
 */
void QSoftKeyLabelHelper::updateAllLabels(QWidget *widget){
    ContextKeyManager* keyManager = ContextKeyManager::instance();
    keyManager->setStandard(widget, Qt::Key_Select,QSoftMenuBar::Select);
    keyManager->setStandard(widget, Qt::Key_Back,QSoftMenuBar::Back);
}

//////////////////////////////////////////////////////////////////////////////
// QLineEditSoftKeyLabelHelper

/*!
    \class QLineEditSoftKeyLabelHelper
    \inpublicgroup QtBaseModule
    \internal
    \brief The QLineEditSoftKeyLabelHelper class is a handler for the softkeys for the \l QLineEdit class.

    The QLineEditSoftKeyLabelHelper is installed on the \l QLineEdit class by Qtopia, and will therefor act on all \l{QLineEdit}s, as well as all classes derived from QLineEdit, unless those derived classes have their own SoftKeyHelper.

    \sa QAbstractSoftKeyLabelHelper
*/

/*!
    Constructs a QLineEditSoftKeyLabelHelper for all QWidgets that inherit the class \a className, or derive from it.  Generally this will already have been done automatically by the server.
*/
QLineEditSoftKeyLabelHelper::QLineEditSoftKeyLabelHelper(QString className) : QAbstractSoftKeyLabelHelper(className), m_l(0), m_preeditTextFlag(false)
{
};

/*!
    Constructs a QLineEditSoftKeyLabelHelper for \l QLineEdit \a l
*/
QLineEditSoftKeyLabelHelper::QLineEditSoftKeyLabelHelper(QLineEdit* l) : QAbstractSoftKeyLabelHelper(qobject_cast<QWidget*>(l)), m_preeditTextFlag(false)
{
    m_l = l;
};

/*!
    Destroys this QLineEditSoftKeyLabelHelper and cleans up.
*/
QLineEditSoftKeyLabelHelper::~QLineEditSoftKeyLabelHelper()
{
};

/*!
    This function responds to the QLineEdit gaining focus.  It updates all labels, and installs an event filter on \a widget to recieve input method events.
    \sa enterEditFocus(), updateAllLabels(), QObject::installEventFilter()
*/
bool QLineEditSoftKeyLabelHelper::focusIn(QWidget *widget){

    updateAllLabels(widget);
    widget->installEventFilter(this);

    return true;
}

/*!
    Responds to cursor movements to ensure that the softkeys are always relevant.
    This is necessary because moving to or from the beginning of the lineedit should supress or activate the \l QSoftMenuBar::BackSpace label respectively.

    \a old and \a current are ignored by QLineEditSoftKeyLabelHelper.
*/
void QLineEditSoftKeyLabelHelper::cursorPositionChanged(int old, int current) {
    Q_UNUSED(old);
    Q_UNUSED(current);
    updateAllLabels();
}

/*!
    This function responds to the QLineEdit losing focus completely.  It disconnects the various signals from \a widget, and removes the eventFilter.
    \sa focusIn(), leaveEditFocus(), QObject::disconnect(), QObject::removeEventFilter()
*/
bool QLineEditSoftKeyLabelHelper::focusOut(QWidget *widget){
    QLineEdit* lineedit = qobject_cast<QLineEdit*>(widget);
    if(lineedit){
        lineedit->disconnect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
        lineedit->disconnect(lineedit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
    };
    widget->removeEventFilter(this);
    return true;
}

/*!
    This function responds to the QLineEdit gaining edit focus.  It sets the softkey labels depending on the content of the \l QLineEdit.
    It also connects signals for \l QLineEdit::textChanged() and  \l QLineEdit::cursorPosition(), and returns true if this is successful.
    If \a widget is not a valid \l QLineEdit then the connections are not made, and this function returns false.

    \sa updateAllLabels(), QObject::connect(), QLineEdit::textChanged(), QLineEdit::cursorPosition()
*/
bool QLineEditSoftKeyLabelHelper::enterEditFocus(QWidget *widget)
{
    updateAllLabels();

    QLineEdit* lineedit = qobject_cast<QLineEdit*>(widget);

    if(lineedit){
        lineedit->connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
        lineedit->connect(lineedit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));

        return true;
    };
    return false;

};

/*!
    This function responds to the helper's \l QLineEdit losing edit focus.  It updates the Softkey labels and disconnects the \l QLineEdit::textChanged() and \l QLineEdit::cursorPositionChanged() signals from \a widget.
    \sa focusOut(), enterEditFocus(), QObject::disconnect(), QLineEdit::textChanged(), QLineEdit::cursorPositionChanged()
*/
bool QLineEditSoftKeyLabelHelper::leaveEditFocus(QWidget *widget)
{
    QLineEdit* lineedit = qobject_cast<QLineEdit*>(widget);
    if(lineedit)
    {
        disconnect(lineedit,SIGNAL(textChanged(QString)),this,SLOT(textChanged(QString)));
        disconnect(lineedit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(cursorPositionChanged(int,int)));
    };

    if(widget->hasFocus()) {
        updateAllLabels();
    };
    return false;
};

/*!
    This function polls the given \a widget and updates the softkey labels accordingly.  The behaviour of the softkeys relative to the QLineEdit with focus is as follows:
    \table
    \header \o Focus State \o Text content \o Select Button \o Back Button
    \row \o Navigation Focus \o Any \o QSoftMenuBar::Edit \o QSoftMenuBar::Back
    \row \o Edit Focus \o Empty \o QSoftMenuBar::EndEdit \o QSoftMenuBar::RevertEdit
    \row \o Edit Focus \o Text or Preedit Text \o QSoftMenuBar::EndEdit \o QSoftMenuBar::BackSpace
    \row \o Edit Focus \o Text or Preedit Text and cursor at beginning of line \o QSoftMenuBar::EndEdit \o QSoftMenuBar::NoLabel
    \endtable

    If there is text or preedit text to the left of the cursor, the back button acts like as a backspace key to delete that text.
    When the cursor reaches the left edge of the \l QLineEdit, its behaviour depends on the contents of the \l QLineEdit - if it's empty, the back button becomes QSoftMenuBar::RevertEdit, and if there is text to the right of the cursor it becomes blank.  Both of these behaviours are to prevent accidental loss of data.

    At all times the enter button toggles edit focus.  when in navigation focus the enter button enters edit focus.  When in edit focus, the enter key leaves edit focus, and enters navigation focus, saving the contents of the \l QLineEdit.

    The changing of the labels themselves is handled through the ContextKeyManager.  This should not normally be relevant for the use of this class.

    \sa enterEditFocus(), leaveEditFocus(), QSoftMenuBar::StandardLabel, QLineEdit
*/
void QLineEditSoftKeyLabelHelper::updateAllLabels(QWidget* widget)
{
    QLineEdit* lineedit = qobject_cast<QLineEdit*>(widget);

    if(!lineedit && m_l) {
        lineedit = m_l;
    };

    if(lineedit != m_l && m_l) {
#ifdef QSOFTKEYLABELHELPER_WARNINGS
        qWarning() << "QLineEditSoftKeyLabelHelper setting labels for widgets not matching it's current widget (m_l is "<<m_l<<", and widget is "<<widget<<")";
#endif //QSOFTKEYLABELHELPER_WARNINGS
        lineedit = qobject_cast<QLineEdit*>(widget);
    };

    if(!lineedit)
    {
#ifdef QSOFTKEYLABELHELPER_WARNINGS
        qWarning() << "aborting QLineEditSoftKeyLabelHelper::updateAllLabels("<<widget<<") - unable to determine appropriate QLineEdit";
#endif //QSOFTKEYLABELHELPER_WARNINGS
        return;
    }

    ContextKeyManager* keyManager = ContextKeyManager::instance();
    if(lineedit->hasFocus() && !lineedit->hasEditFocus()){
        keyManager->setStandard(lineedit, Qt::Key_Select,QSoftMenuBar::Edit);
        keyManager->setStandard(lineedit, Qt::Key_Back,QSoftMenuBar::Back);
    };

    if(lineedit->hasEditFocus()) {
        keyManager->setStandard(lineedit, Qt::Key_Select,QSoftMenuBar::EndEdit);
        if(lineedit->cursorPosition() || m_preeditTextFlag)
        {
            keyManager->setStandard(lineedit, Qt::Key_Back,QSoftMenuBar::BackSpace);
        } else if(lineedit->text().isEmpty())
        {
            keyManager->setStandard(lineedit, Qt::Key_Back,QSoftMenuBar::RevertEdit);
        } else {
            // no text, not blank, and cursorPosition is 0 so blank label.
            keyManager->setStandard(lineedit, Qt::Key_Back,QSoftMenuBar::NoLabel);
        };
    }
}

/*!
    This function responds to changing widgets, and is specific to the "class" mode.  A QLineEditSoftKeyLabelHelper that is instantiated on a specific widget should never recieve this signal.

    In response to changing widgets, the Softkey Helper only takes note of the new widgets and resets it's internal state, specifically clearing any preedit text it has been tracking.

    If \a newWidget is non-null, it is set as the current widget for this helper.

    \a oldWidget is ignored.
    \sa enterEditFocus(), leaveEditFocus()
*/
void QLineEditSoftKeyLabelHelper::currentWidgetChangeNotification(QWidget* newWidget, QWidget* oldWidget)
{
    Q_UNUSED(oldWidget);

    if(newWidget && newWidget != m_l) {
        m_l = qobject_cast<QLineEdit*>(newWidget);
    };

    m_preeditTextFlag = false;
}

/*!
    This eventFilter is to keep track of preedit text for the QObject \a watched.  If \a watched is not a valid QWidget, and the QLineEditSoftKeyLabelHelper uses its own currently watched QWidget instead.  If it cannot find a valid QWidget, the QLineEditSoftKeyLabelHelper aborts and returns false.

    If \a event is a preedit event, the QLineEditSoftKeyLabelHelper keeps track of its contents to be able
    so that the helper can know whether or not the preedit text is empty.

    It never consumes any events, so it always returns false.

    \sa QObject::installEventFilter(), QInputMethodEvent, QWSEvent::IMEvent
*/
bool QLineEditSoftKeyLabelHelper::eventFilter(QObject* watched, QEvent *event)
{
    // eventfilter is needed on lineedit to make sure the softkeys show
    // backspace if there is preedit text
    if(event->type() == QEvent::InputMethod)
    {
        QInputMethodEvent *imevent = static_cast<QInputMethodEvent*>(event);
        if(imevent){
            QWidget* widget = qobject_cast<QWidget*>(watched);
            if(!widget)
            {
                if(m_l)
                {
                    widget=qobject_cast<QWidget*>(m_l);
                };

            }
            if(!widget){
                return false;
            };

            // If the preedit text is not empty, then the softkey will be a
            // backspace.
            // If the preedit is empty, then the existing logic is sufficient.
            // either way, store the preedit so that future updates are correct
            m_preeditTextFlag = !imevent->preeditString().isEmpty();
            updateAllLabels();

        } else {
            qWarning("IM event not proper QInputMethodEvent");
        };
    }

    // never consume event
    return false;
};

//////////////////////////////////////////////////////
class SoftKeyLabelHelperQSpinBoxLineEditAccessor : public QAbstractSpinBox
{
    Q_OBJECT
public:
    QLineEdit *getLineEdit() { return lineEdit(); }
};

/*!
    \class QDateTimeEditSoftKeyLabelHelper
    \inpublicgroup QtBaseModule
    \internal
    \brief The QDateTimeEditSoftKeyLabelHelper class is a handler for the softkeys for the \l QDateTimeEdit class.
*/

QDateTimeEditSoftKeyLabelHelper::QDateTimeEditSoftKeyLabelHelper(QString className) : QAbstractSoftKeyLabelHelper(className), m_dte(0)
{
    
}

QDateTimeEditSoftKeyLabelHelper::QDateTimeEditSoftKeyLabelHelper(QDateTimeEdit* dte) : QAbstractSoftKeyLabelHelper(dte)
{
    m_dte = dte;
}

QDateTimeEditSoftKeyLabelHelper::~QDateTimeEditSoftKeyLabelHelper()
{
    
}

bool QDateTimeEditSoftKeyLabelHelper::focusIn(QWidget *widget)
{
    updateAllLabels(widget);
    widget->installEventFilter(this);
    return true;
}

bool QDateTimeEditSoftKeyLabelHelper::focusOut(QWidget *widget)
{
    widget->removeEventFilter(this);
    return true;
}

bool QDateTimeEditSoftKeyLabelHelper::enterEditFocus(QWidget *widget)
{
    updateAllLabels(widget);
    return true;
}

bool QDateTimeEditSoftKeyLabelHelper::leaveEditFocus(QWidget *widget)
{
    updateAllLabels(widget);
    return true;
}

void QDateTimeEditSoftKeyLabelHelper::updateAllLabels(QWidget *widget)
{
    if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit*>(widget)) {
        ContextKeyManager* keyManager = ContextKeyManager::instance();
        
        if(dte->hasFocus() && !dte->hasEditFocus()){
            keyManager->setStandard(dte, Qt::Key_Select, QSoftMenuBar::Edit);
            keyManager->setStandard(dte, Qt::Key_Back, QSoftMenuBar::Back);
        }
        
        if (dte->hasEditFocus()) {
            if (dte->currentSection() == QDateTimeEdit::NoSection) {
                keyManager->setStandard(dte, Qt::Key_Select, QSoftMenuBar::Select);
                keyManager->setStandard(dte, Qt::Key_Back, QSoftMenuBar::NoLabel);
            } else {
                keyManager->setStandard(dte, Qt::Key_Select, QSoftMenuBar::EndEdit);
                keyManager->setStandard(dte, Qt::Key_Back, QSoftMenuBar::BackSpace);
            }
        }
    }
}

bool QDateTimeEditSoftKeyLabelHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit*>(watched)) {
            updateAllLabels(dte);
            return true;
        }
    }
    return false;
}
//////////////////////////////////////////////////////

/*!
    The function responds to changes in the text of the current target \l QLineEdit by calling \l updateAllLabels().
    \a newText is not currently used, as information about the text content is available as needed.
    \sa updateAllLabels()
*/
void QLineEditSoftKeyLabelHelper::textChanged(QString newText)
{
    Q_UNUSED(newText);
    updateAllLabels();
};

/*!
    \internal
*/
QSoftKeyLabelHelperPrivate::QSoftKeyLabelHelperPrivate(QAbstractSoftKeyLabelHelper* parent)
{
    q = parent;
};


