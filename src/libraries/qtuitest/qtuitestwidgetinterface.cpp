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

#include "qtuitestwidgetinterface.h"
#include "qtuitestnamespace.h"

#include <QRect>
#include <QRegion>
#include <QApplication>
#include <QWidget>

/*!
    \preliminary
    \class QtUiTest::WidgetFactory
    \inpublicgroup QtUiTestModule
    \brief The WidgetFactory class provides a factory interface
    for QtUiTest widget wrapper classes.

    QtUiTest::WidgetFactory is an abstract base class which enables the
    creation of QtUiTest wrapper objects around Qt/Qt Extended widgets.

    Customizing QtUiTest behaviour for particular widgets is achieved by
    implementing one or more test widget classes which inherit from
    one or more QtUiTest widget interfaces,
    subclassing QtUiTest::WidgetFactory, reimplementing the pure virtual
    keys() and create() functions to create instances of the custom test
    widget classes, and exporting the factory class using the
    Q_EXPORT_PLUGIN2() macro.
*/

/*!
    \fn QtUiTest::WidgetFactory::create(QObject* object)

    Attempts to create a test widget to wrap \a object.  Returns the created
    test widget.  Returns 0 if this factory does not support wrapping
    \a object.

    The returned object is suitable for use with
    \l{QtUiTest}{qtuitest_cast}.

    This function will only be called for objects which inherit one of the
    classes returned by keys().
*/

/*!
    Returns a widget or test widget of \a type, or 0 if none can be found.

    Reimplement this function to provide custom behaviour for
    QtUiTest::findWidget().  For example, if a custom soft menu widget is
    being used rather than the shipped ContextLabel class, this function
    must be reimplemented to return a pointer to the custom widget.

    The base implementation always returns 0.
*/
QObject* QtUiTest::WidgetFactory::find(QtUiTest::WidgetType type)
{ Q_UNUSED(type); return 0; }

/*!
    \fn QtUiTest::WidgetFactory::keys() const

    Returns the list of C++ class names this factory can generate test widgets
    for.

    Note that returning a class from this function does not guarantee that the
    factory will always be able to generate a test widget for that class.
*/

/*!
    \preliminary
    \class QtUiTest::Widget
    \inpublicgroup QtUiTestModule
    \brief The Widget class provides an abstract base class
    for all test widgets.

    QtUiTest::Widget contains functionality which maps
    closely to QWidget.  It encapsulates important information
    and functionality related to two-dimensional GUI elements.

    All Qt Extended test widgets should implement the QtUiTest::Widget interface,
    using multiple inheritance to implement other QtUiTest interfaces
    where suitable.
*/

/*!
    \fn const QRect& QtUiTest::Widget::geometry() const

    Returns the geometry of this widget in parent coordinates.

    \sa QWidget::geometry()
*/

/*!
    Returns the bounding rect of this widget in widget coordinates.

    The base implementation returns geometry(), transformed to widget
    coordinates.

    \sa QWidget::rect()
*/
QRect QtUiTest::Widget::rect() const
{
    QRect ret(geometry());
    ret.moveTopLeft(QPoint(0,0));
    return ret;
}

/*!
    \fn bool QtUiTest::Widget::isVisible() const

    Returns true if this widget is currently visible.

    In this context, "visible" has the same meaning as in QWidget::isVisible().
    Therefore, this function returning \c{true} is not a guarantee that this
    widget is currently on screen and unobscured.  To test this,
    visibleRegion() can be used.

    \sa QWidget::isVisible(), visibleRegion()
*/

/*!
    \fn QRegion QtUiTest::Widget::visibleRegion() const

    Returns the currently on-screen, unobscured region of this widget,
    in widget coordinates.

    \sa QWidget::visibleRegion()
*/

/*!
    \fn bool QtUiTest::Widget::ensureVisibleRegion(const QRegion& region)

    Simulate whatever user input is necessary to ensure that \a region
    (in local coordinates) is on-screen and unobscured.

    Returns true if \a region was successfully made visible.

    \sa visibleRegion()
*/

/*!
    Simulate whatever user input is necessary to ensure that \a point
    (in local coordinates) is on-screen and unobscured.

    This convenience function calls ensureVisibleRegion() with a region
    containing only \a point.

    Returns true if \a point was successfully made visible.

    \sa visibleRegion(), ensureVisibleRegion()
*/
bool QtUiTest::Widget::ensureVisiblePoint(const QPoint& point)
{
    return ensureVisibleRegion( QRegion(point.x(), point.y(), 1, 1) );
}

/*!
    \fn QObject* QtUiTest::Widget::parent() const

    Returns the parent of this widget, or 0 if this widget has no parent.

    The returned object may be an actual QWidget, or may be a wrapping
    test widget.  Therefore, the only safe way to use the returned value
    of this function is to cast it to the desired QtUiTest interface
    using \l{QtUiTest}{qtuitest_cast}.

    \sa QObject::parent(), QWidget::parentWidget(), children()
*/

/*!
    Returns the window title (caption).

    The returned string will typically be empty for all widgets other than
    top-level widgets.

    The default implementation returns an empty string.

    \sa QWidget::windowTitle()
*/
QString QtUiTest::Widget::windowTitle() const
{ return QString(); }

/*!
    \fn const QObjectList& QtUiTest::Widget::children() const

    Returns all children of this widget.

    The returned objects may be actual QWidget instances, or may be wrapping
    test widgets.  Therefore, the only safe way to use the returned objects
    are to cast them to the desired QtUiTest interface using
    \l{QtUiTest}{qtuitest_cast}.

    Reimplementing this function allows widgets which are conceptually
    widgets but are not QObject subclasses to be wrapped.  This can be
    achieved by returning a list of test widgets which do not necessarily
    have underlying QWidget instances.

    \sa QObject::children(), parent()
*/

/*!
    Returns the currently on-screen, unobscured region of all
    child widgets of this widget, in widget coordinates.  This does not
    include the visible region of this widget.

    The base implementation calculates the visible region by calling
    visibleRegion() and childrenVisibleRegion() on all children().

    \sa QWidget::visibleRegion(), children(), visibleRegion()
*/
QRegion QtUiTest::Widget::childrenVisibleRegion() const
{
    QRegion ret;
    foreach (QObject *o, children()) {
        Widget *w = qtuitest_cast<Widget*>(o);
        if (w) ret |= ((w->childrenVisibleRegion() | w->visibleRegion())
                .translated( -w->geometry().topLeft() ) );
    }
    return ret;
}

/*!
    \fn QPoint QtUiTest::Widget::mapToGlobal(const QPoint& pos) const

    Maps \a pos from widget coordinates to global screen coordinates and
    returns the result.

    \sa QWidget::mapToGlobal()
*/

/*!
    \fn QPoint QtUiTest::Widget::mapFromGlobal(const QPoint& pos) const

    Maps \a pos from global screen coordinates to widget coordinates and
    returns the result.

    \sa QWidget::mapFromGlobal()
*/

/*!
    Simulate the user input necessary to move keyboard focus to this
    widget.

    The base implementation uses the result of hasFocus() to determine if the
    widget currently has focus.  If in keypad mode, a sequence of Up or Down
    key presses will be used to move focus until this widget has focus.
    If in mouse mode, the center of this widget will be clicked once.

    Due to the way this function works in keypad mode, it is very important
    that focusOutEvent() is correctly implemented for all widgets to dismiss
    any "grab" effects on keyboard focus.

    When reimplementing this function, it is necessary to call focusOutEvent()
    on any widget before taking any action which could cause that widget to
    lose focus.

    Returns true if focus was successfully given to this widget, false
    otherwise.

    \sa QWidget::setFocus(), focusOutEvent()
*/
bool QtUiTest::Widget::setFocus()
{
    if (hasFocus()) return true;

    // xxx Try using findWidget(Focus) here instead.
    struct Focus {
        static QWidget* focusWidget() {
            QWidget *w = QApplication::focusWidget();
            if (!w) w = QApplication::activeWindow();
            if (!w) w = QApplication::activePopupWidget();
            if (!w) w = QApplication::activeModalWidget();
            return w;
        }
    };

    using namespace QtUiTest;

    if (!mousePreferred()) {
        QWidget *w;
        Widget *tw;

        const int maxtries = 100;
        int i = 0;

        while (!hasFocus() && i < maxtries) {
            w = Focus::focusWidget();
            tw = qtuitest_cast<Widget*>(w);
            if (!tw) {
                setErrorString("Can't find currently focused widget!");
                return false;
            }

            tw->focusOutEvent();

#ifdef Q_WS_QWS
            keyClick(Qt::Key_Down);
#else
            keyClick(Qt::Key_Tab);
#endif
            if (!QtUiTest::waitForSignal(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)))) {
                setErrorString("Pressing the "
#ifdef Q_WS_QWS
                        "down"
#else
                        "tab"
#endif
                        " key didn't result in a change in focused widget!");
                return false;
            }

            //FIXME: Additional wait for Greenphone
            QtUiTest::wait(200);
            ++i;
        }

        if (!hasFocus()) {
            setErrorString(QString("Failed to give focus to widget after %1 keyclicks")
                           .arg(maxtries));
        }
    } else {
        Widget* tw = qtuitest_cast<Widget*>(Focus::focusWidget());
        if (tw) tw->focusOutEvent();

        QPoint center = rect().center();
        ensureVisiblePoint(center);

        center = mapToGlobal(center);

        // In the past, bugs have existed in Qt/Qtopia which make one click
        // be consumed somewhere.
        // Keep working in that case, but give a warning.
        int i = 0;
        do {
            mouseClick( center );
            if (hasFocus()) break;
        } while(++i < 2);

        if (hasFocus() && 1 == i)
            qWarning("QtUitest: possible bug, took more than one click to "
                     "give focus to a widget.");
    }

    return hasFocus();
}

/*!
    \fn bool QtUiTest::Widget::setEditFocus(bool enable)
    Simulate the user input necessary to \a enable or disable edit focus for
    this widget.  Enabling edit focus will implicitly call setFocus() when
    necessary.

    The concept of edit focus only exists in Qt Embedded.  This function must
    be implemented when using Qt Embedded.  On other platforms,
    this function will behave the same as setFocus() when enable is true, and
    will have no effect when enable is false.

    Returns true if edit focus was successfully enabled or disabled for this
    widget, false otherwise.

    \sa QWidget::setEditFocus(), hasEditFocus()
*/
#ifndef Q_WS_QWS
bool QtUiTest::Widget::setEditFocus(bool enable)
{
    if (!enable) return true;

    return hasFocus() || setFocus();
}
#endif

/*!
    \fn bool QtUiTest::Widget::hasEditFocus() const
    Returns true if this widget currently has edit focus.

    The concept of edit focus only exists in Qt Embedded.  This function must
    be implemented on Qt Embedded.  On other platforms, the base implementation
    will return the same as hasFocus().

    \sa QWidget::hasEditFocus(), setEditFocus()
*/
#ifndef Q_WS_QWS
bool QtUiTest::Widget::hasEditFocus() const
{ return hasFocus(); }
#endif

/*!
    This function is called when this widget is about to lose keyboard focus.
    The base implementation does nothing.

    This function should be reimplemented to put this widget in a state where
    subsequent up/down key clicks will result in non-destructive navigation
    between widgets.

    For example, if this function is called on a combo box which
    currently has a list popped up, it should dismiss the list
    so that subsequent key clicks will navigate between widgets rather
    than navigating within the list.

    \code
    void MyComboBox::focusOutEvent() {
        // Assuming q is a QComboBox...
        // If the list is currently popped up...
        if (q->view()->isVisible()) {
            // Press the Select key to commit the currently highlighted
            // item and ensure we are ready to navigate by keys.
            QtUiTest::keyClick(Qt::Key_Select);
        }
    }
    \endcode

    \sa QWidget::focusOutEvent(), setFocus(), hasFocus()
*/
void QtUiTest::Widget::focusOutEvent()
{}

/*!
    \fn bool QtUiTest::Widget::hasFocus() const

    Returns true if this widget currently has keyboard focus.

    \sa QWidget::hasFocus()
*/

/*!
    Returns the flags set on this widget.

    The default implementation returns 0, which is equivalent to a plain
    Qt::Widget with no special flags set.
*/
Qt::WindowFlags QtUiTest::Widget::windowFlags() const
{ return 0; }

/*!
    Returns true if this widget is of the given \a type.

    The base implementation always returns false.
*/
bool QtUiTest::Widget::inherits(QtUiTest::WidgetType type) const
{ Q_UNUSED(type); return false; }

/*!
    \fn void QtUiTest::Widget::gotFocus()

    This signal is emitted when this widget gains focus by any means.

    \sa QApplication::focusChanged()
*/

/*!
    \preliminary
    \class QtUiTest::ActivateWidget
    \inpublicgroup QtUiTestModule
    \brief The ActivateWidget class provides an abstract base class
    for all test widgets which can conceptually be "activated" by a user.

    "Activation" occurs when user input causes an action, possibly
    non-reversible, on a widget which exists solely for the purpose of
    causing that action.

    Examples of widgets suitable for this interface include QAbstractButton.
*/

/*!
    \fn bool QtUiTest::ActivateWidget::activate()
    Simulate the user input necessary to activate this widget.

    Returns true if this widget was successfully activated.

    For example, a button would reimplement this function to simulate
    a click on itself, or to navigate to itself and hit the "Select" key.
*/

/*!
    \fn void QtUiTest::ActivateWidget::activated()

    This signal is emitted when this widget is activated.
*/

/*!
    \preliminary
    \class QtUiTest::LabelWidget
    \inpublicgroup QtUiTestModule
    \brief The LabelWidget class provides an abstract base class
    for all test widgets which are conceptually labels.

    Some widgets may act as labels for themselves while also providing
    other functionality.  For example, a button widget labels itself
    and is also an ActivateWidget.

    Examples of widgets suitable for this interface include QLabel.
*/

/*!
    \fn QString QtUiTest::LabelWidget::labelText() const

    Returns the text displayed on this label.

    Most label widgets will also implement \l{QtUiTest::TextWidget}.
    Most commonly, labelText() returns the same value as \l{QtUiTest::TextWidget::text()}.
*/

/*!
    \preliminary
    \class QtUiTest::CheckWidget
    \inpublicgroup QtUiTestModule
    \brief The CheckWidget class provides an abstract base class
    for all test widgets which support 'checked' and 'unchecked' states.

    QtUiTest::CheckWidget exposes the current check state of widgets
    which can be checked or unchecked.

    Examples of widgets suitable for this interface include QCheckBox
    and QAbstractButton.
*/

/*!
    Returns true if this widget has three possible states, i.e. the widget
    can be in state Qt::PartiallyChecked.

    The base implementation returns false.

    \sa QCheckBox::isTristate()
*/
bool QtUiTest::CheckWidget::isTristate() const
{ return false; }

/*!
    \fn Qt::CheckState QtUiTest::CheckWidget::checkState() const

    Returns the current check state of this widget.

    \sa QCheckBox::checkState(), setCheckState()
*/

/*!
    Simulates the user input necessary to set the current check state
    to \a state, returning true on success.

    The default implementation does nothing and returns false.

    \sa QCheckBox::setCheckState(), checkState()
*/
bool QtUiTest::CheckWidget::setCheckState(Qt::CheckState state)
{ Q_UNUSED(state); return false; }

/*!
    \fn void QtUiTest::CheckWidget::stateChanged(int state)

    This signal is emitted when the check state of this widget changes
    to \a state.  \a state is compatible with Qt::CheckState.
*/

/*!
    \preliminary
    \class QtUiTest::TextWidget
    \inpublicgroup QtUiTestModule
    \brief The TextWidget class provides an abstract base class
    for all test widgets which display text to the user.

    The QtUiTest::TextWidget interface should be implemented on any widget
    which shows any text at all.  This is the primary interface QtUiTest
    uses to determine text->widget mappings, and it is used to implement
    \l{QSystemTest::}{getText()}, a heavily used verification mechanism.

    This interface is closely related to QtUiTest::InputWidget, which
    provides an interface for entering text into a widget.  Any widgets
    which contain user-editable text will typically implement both
    QtUiTest::InputWidget and QtUiTest::TextWidget.

    Examples of widgets suitable for this interface include QLabel,
    QAbstractButton, QLineEdit, QTextEdit and many more.

    \sa QtUiTest::InputWidget
*/

/*!
    Returns the text in this widget which is currently selected / highlighted.
    If the widget does not support the concept of selected text, this function
    should return the same as text().

    The base implementation calls text().

    \sa QLineEdit::selectedText()
*/
QString QtUiTest::TextWidget::selectedText() const
{ return text(); }

/*!
    \fn QString QtUiTest::TextWidget::text() const

    Returns all of the text this widget is currently presenting to the user.
*/

/*!
    \preliminary
    \class QtUiTest::ListWidget
    \inpublicgroup QtUiTestModule
    \brief The ListWidget class provides an abstract base class
    for all test widgets which display a list of items to the user.

    QtUiTest::ListWidget allows a widget which is conceptually a list to
    be enumerated.  This is closely related to QtUiTest::SelectWidget,
    which may be implemented to allow a user to select an item from a list.

    Examples of widgets suitable for this interface include QAbstractItemView,
    QComboBox and QMenu.

    \sa QtUiTest::SelectWidget
*/

/*!
    \fn QStringList QtUiTest::ListWidget::list() const

    Returns a list containing a text representation of each item in this list
    widget.
*/

/*!
    \fn QRect QtUiTest::ListWidget::visualRect(const QString& item) const

    Returns the bounding rect of \a item, in widget coordinates.
    If \a item isn't currently shown in this widget, returns a null rect.

    \sa QAbstractItemView::visualRect()
*/

/*!
    Simulates the user input necessary to navigate this widget until \a item
    is currently visible and return true on success.

    For example, in a QAbstractItemView with vertical scrollbars, if \a item
    exists further down the list than currently shown, this function might
    simulate 'Down' key clicks until it becomes visible.

    The base implementation does nothing and returns true.
*/
bool QtUiTest::ListWidget::ensureVisible(const QString& item)
{ Q_UNUSED(item); return true; }

/*!
    \preliminary
    \class QtUiTest::InputWidget
    \inpublicgroup QtUiTestModule
    \brief The InputWidget class provides an abstract base class
    for all test widgets which allow the user to input text.

    QtUiTest::InputWidget encapsulates a widget in which a user may enter
    and remove text.  This is closely related to QtUiTest::TextWidget, which
    provides an interface for retrieving text from a widget.  Any widgets
    which contain user-editable text will typically implement both
    QtUiTest::InputWidget and QtUiTest::TextWidget.

    Examples of widgets suitable for this interface include QLineEdit and
    QTextEdit.

    \sa QtUiTest::TextWidget
*/

/*!
    \fn bool QtUiTest::InputWidget::canEnter(const QVariant& item) const

    Returns true if \a item can possibly be entered into this widget.

    For example, for a QLineEdit, QLineEdit::validator() would be used to
    ensure that \l{QVariant::toString()}{item.toString()} constitutes
    valid input for this line edit.
*/

/*!
    \fn bool QtUiTest::InputWidget::enter(const QVariant& item, bool noCommit)

    Simulates the user input necessary to enter \a item into this widget.
    Returns true on success.

    \a item can potentially be any type representable in a QVariant.
    Most widgets will only be interested in text, in which case
    \l{QVariant::toString()}{item.toString()} can be called to retrieve
    a string.  Examples of widgets which may handle types other than text
    are QDateEdit and QTimeEdit, which may handle dates and times.

    If \a noCommit is true, no attempt is made to commit the input (for example,
    by clicking the Select key). Normally this value will be false, which will
    result in the input being committed. This value is not applicable to all
    widget types.

    If canEnter() returns true and this function returns false, an error
    has occurred and this widget is left in an undefined state.
*/

/*!
    \fn void QtUiTest::InputWidget::entered(const QVariant& item)
    This signal is emitted when \a item is entered into the widget
    by the user.
*/

/*!
    \preliminary
    \class QtUiTest::SelectWidget
    \inpublicgroup QtUiTestModule
    \brief The SelectWidget class provides an abstract base class
    for all test widgets which allow the user to select from a range of items.

    QtUiTest::SelectWidget encapsulates a widget which provides the user
    with a choice from a (possibly unlimited) range.  This is closely related
    to QtUiTest::ListWidget, which may be implemented to allow a user to
    enumerate all items from a list.

    Examples of widgets suitable for this interface include QAbstractItemView,
    QComboBox and QMenu.

    \sa QtUiTest::ListWidget
*/

/*!
    Returns true if this widget supports the selection of multiple items at
    the same time.

    The base implementation returns false.
*/
bool QtUiTest::SelectWidget::isMultiSelection() const
{ return false; }

/*!
    \fn bool QtUiTest::SelectWidget::canSelect(const QString& item) const

    Returns true if \a item can possibly be selected from this widget.
*/

/*!
    Returns true if all of the given \a items can be selected from this widget
    at the same time.

    The base implementation returns true if isMultiSelection() returns true
    and canSelect() returns true for every item in \a items.
*/
bool QtUiTest::SelectWidget::canSelectMulti(const QStringList& items) const
{
    if (!isMultiSelection()) return false;
    foreach (QString item, items)
        if (!canSelect(item)) return false;
    return true;
}

/*!
    \fn bool QtUiTest::SelectWidget::select(const QString& item)

    Simulates the user input necessary to select \a item from this widget.

    Returns true if \a item was successfully selected.

    If canSelect() returns true and this function returns false, an error
    has occurred and this widget's state is undefined.
*/

/*!
    Simulates the user input necessary to select all \a items from this widget
    at the same time.

    Returns true if \a items were all successfully selected.

    If canSelectMulti() returns true and this function returns false, an error
    has occurred and this widget's state is undefined.

    The base implementation calls canSelectMulti() to check if \a items can
    be selected, then calls select() on each item in \a items.
*/
bool QtUiTest::SelectWidget::selectMulti(const QStringList& items)
{
    if (!canSelectMulti(items)) return false;
    foreach (QString item, items)
        if (!select(item)) return false;
    return true;
}

/*!
    \fn void QtUiTest::SelectWidget::selected(const QString& item)
    This signal is emitted when \a item is selected from this widget.
*/

