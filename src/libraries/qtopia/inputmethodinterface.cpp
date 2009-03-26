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

#include "inputmethodinterface.h"
#include <qtopialog.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/*!
    \class QIMActionDescription
    \inpublicgroup QtBaseModule

    \preliminary
    \brief The QIMActionDescription class provides a way to define a QAction for a QtopiaInputMethod in the QSoftMenuBar QMenu.

    The menu in which QtopiaInputMethod actions will eventually displayed will
    typically be in a different process from the input method itself
    (which runs in the server).
    In order to avoid confusion, the simple QIMActionDescription is used
    rather than QActions on this side.

    The \a label attribute is used as the label of the final \l QAction.

    the \a iconFileName argument is a QString, and is used to find the icon
    for the action.  Note that the file name must be fully qualified, as it
    will not necessarily be called with the same environment as the
    inputmethod.

    The \a id attribute will be passed back to the input method via
    \c menuActionActivated(), in order to differentiate different actions,
    so the id field must be unique for each action in an input-method.

    \bold {Note:} 0 is reserved for an unknown or unrecognised menu actions,
    and negative id values are reserved for system menus.  Using these values
    is not recommended.

    IMActionDescriptions should be created on the heap.
    The server expects lists of pointers to QIMActionDescriptions, and will
    delete them when it is finished with them.  If constructing the
    QIMActionDescription is expensive, the copy constructor
    is very fast, and can be used safely.

    \sa QtopiaInputMethod, QIMActionDescription()

  \ingroup userinput
*/


class QIMActionDescriptionPrivate : public QSharedData
{
public:
    QIMActionDescriptionPrivate(int id=0, QString label=QString(),QString iconFileName= QString());

    int m_id;
    QString m_label;
    QString m_iconFileName;
};

QIMActionDescriptionPrivate::QIMActionDescriptionPrivate(int id, 
                                        QString label, QString iconFileName) 
    : QSharedData(), m_id(id), m_label(label), m_iconFileName(iconFileName)
{
}
/*!
    The QIMActionDescription takes an \a id, a unique identifier used in 
    notification to differentiate the menu icons.  The \a iconFileName argument 
    is a QString used to find the picture for the action.  
    Note that the file name must be fully 
    qualified, as it will not necessarily be called with the same 
    environment as the inputmethod.

    The default constructor creates an QIMActionDescription with \a id = 0 (), 
    and empty QStrings for \a label and \a iconFileName    
*/


QIMActionDescription::QIMActionDescription(int id, QString label, QString iconFileName)
{
    d = new QIMActionDescriptionPrivate;
    d->m_id=id;
    d->m_label=label;
    d->m_iconFileName=iconFileName;
};

/*!
Constructs a shallow copy of the given QIMActionDescription \a original.
For more information about shallow copies, see the Implicitly Shared Classes Qt documentation.
*/

QIMActionDescription::QIMActionDescription(const QIMActionDescription& original){
    d = original.d;
}
/*!
\internal
Constructs a QIMActionDescription from the private data object \a dd.
*/

QIMActionDescription::QIMActionDescription(QIMActionDescriptionPrivate &dd){
    d = &dd;
}

/*!
  Destroys the action description and cleans up
*/
QIMActionDescription::~QIMActionDescription()
{
};

/*!
    Returns the identifier for this menu action, used to differentiate 
    actions in the one input method menu.
    \sa setId()
*/
int QIMActionDescription::id() const{return d->m_id;};

/*!
    Sets the identifier for this menu action to the given \a id.
    This value is used to differentiate the actions in the input method 
    menu.
    \sa id()
*/
void QIMActionDescription::setId(const int id){d->m_id = id;};

/*! 
    Returns the user-visible label to be displayed by this menu action. 
    \sa setLabel()
*/
QString QIMActionDescription::label() const {return d->m_label;};

/*! 
    Sets the user-visible label to be displayed by this menu action to the given \a string.
    \sa label()
*/
void QIMActionDescription::setLabel(const QString &string) {d->m_label = string;};

/*! 
    Returns the fully-qualified file name used to create the icon for 
    this menu item.

*/
QString QIMActionDescription::iconFileName() const {return d->m_iconFileName;};

/*! 
    Sets the icon file name for the input method action to the given \a string.
    \bold {Note:} the icon file name must be fully qualified, as the menu is 
    likely to be created by a seperate process that will not necessarily 
    share the same local address as the input method.
*/
void QIMActionDescription::setIconFileName(const QString &string) {d->m_iconFileName = string;};

/*!
  \class QtopiaInputMethod
    \inpublicgroup QtBaseModule

    \brief The QtopiaInputMethod class describes the minimum interface that an input method plug-in must provide.

    Input Methods may be added to Qt Extended via plug-ins.  In
    order to write an input method plug-in you must create an
    interface for your plug-in by deriving from the
    QtopiaInputMethod class.

    At a minimum you will need to implement the methods name(),
    identifier(), version(), state(), properties(), icon()
    and reset().

    Pop-up input methods that need to show a widget on the
    screen can do so by overriding inputWidget().

    Filtering input methods that need to filter application pen
    or keyboard events can do so by overriding inputModifier()

    \sa {Tutorial: Create an Input method Plug-in that Modifies Keyboard Input}, QWSInputMethod()

  \ingroup userinput
  \ingroup plugins
*/

/*!
  \fn QtopiaInputMethod::QtopiaInputMethod(QObject *parent)

  Constructs an input method with the parent set to \a parent.
 */

/*!
  \fn QtopiaInputMethod::~QtopiaInputMethod()

  Destroys the input method.
*/


/*!
  \enum QtopiaInputMethod::Properties

  The Properties flags describe how the input method behaves and under
  what conditions it should be loaded.

  \value RequireMouse
    The input method should not be loaded if no pen input is
    available.
  \value RequireKeypad
    The input method should not be loaded if no phone keypad
    is available.  A phone keypad consists of 0-9, * and # keys.
  \value InputModifier
    The input method can filter device pen and key events.
  \value InputWidget
    The input method provides a popup widget that receives input
  \value DockedInputWidget
    The input method's popup widget should be docked to the bottom of the screen, and other windows should be resized to allow for it's space when it is visible.  Note that DockedInputWidget implies InputWidget, and that the input method's input widget should be defined if this property is set (Setting the InputWidget property as well will not do any harm, but is not necessary)
  \value InteractiveIcon
    The icon representing the input method is interactive.  It
    can be clicked or will animate to show the mode of the
    input method.
  \value MenuItem
    The input method menu adds an item to the softkey context menu.
    If this value is set, the input method must also implement the menuActionToDuplicate() function that returns a QAction that is to copied for the menu.
    Additionally, the input method must respond to the menuActionActivated, as the supplied
    action will not receive events (it is only used as a model to copy).
    The input method can respond by either by implementing
    menuActionActivated in a subclass, or by connecting it somewhere else for processing.
*/

/*!
  \enum QtopiaInputMethod::State

  This enum describe the state of the input method.

  \value Sleeping
      The input method is not able to provide input
      for the current input method hint.
  \value Ready
      The input method is able to provide input
      for the current input method hint.
*/

/*!
  \fn QString QtopiaInputMethod::name() const

  Returns the name of the input method.  This is a end user
  visible string and should be translated to the users locale.
*/

/*!
  \fn void QtopiaInputMethod::updateMenuAction(bool)
  This signal is not currently used, but reserved for future use.
  It is anticipated that this signal will be used to signal to the server 
  that the input method menu has changed,  with \a showMenuAction being used
  to indicate whether or not the new menu should be displayed.
  \bold{Note:} Currently the server updates the menu on a \l stateChanged()
  signal.

*/
void updateMenuAction(bool showMenuAction){Q_UNUSED(showMenuAction);};
/*!
  \fn QString QtopiaInputMethod::identifier() const

  Returns the indentifier of the input method.  This is used
  to identify the input method in code.
*/

/*!
  \fn QString QtopiaInputMethod::version() const

  Returns the version string of the input method.
*/

/*!
  \fn QtopiaInputMethod::State QtopiaInputMethod::state() const

  Returns the current state of the input method.
*/

/*!
  \fn int QtopiaInputMethod::properties() const

  Returns the capability property flags for the input method.

  \sa Properties
*/

/*!
  \fn QIcon QtopiaInputMethod::icon() const

  Returns the icon associated with this input method.
*/

/*!
  \fn void QtopiaInputMethod::reset()

  Resets the input method to its initial state.
*/

/*!
 \fn void QtopiaInputMethod::stateChanged()

 This signal is emitted when the input method state changes.
 Causes the server to update the menus for the input method.

 \sa state()
*/

/*!
   Returns true if the current input method is restricted
   to providing input suitable for the current hint.
*/
bool QtopiaInputMethod::restrictedToHint() const
{
    return false;
}

/*!
   Returns true if the current input method is restricted
   to providing input suitable for a password field.
*/
bool QtopiaInputMethod::passwordHint() const
{
    return false;
}


/*!
  Returns the status widget for the input method.  On the
  first call of this function it should create the widget
  with the given \a parent.

  The status widget is displayed when the input method is
  selected and not in the Sleeping state.  If no status
  widget is provided a label with the icon returned from
  icon() will be used instead.

  The base class returns 0.
*/
QWidget *QtopiaInputMethod::statusWidget( QWidget * parent )
{
    Q_UNUSED(parent);
    return 0;
}

/*!
  Returns the input widget for the input method.  On the
  first call of this function it should create the widget
  with the given \a parent.

  The widget is used for input methods that have the InputWidget
  property.  It is shown when the user requests it to provide
  input or even simply provide additional keys.  Returning
  0 indicates no input widget is provided and that the
  input method uses some other method for the user to generate
  text.

  The base class returns 0.
*/
QWidget *QtopiaInputMethod::inputWidget( QWidget * parent )
{
    Q_UNUSED(parent);
    return 0;
}

/*!
  Returns the input modifier for the input method.

  The input modifer is installed as the current input method for
  the QWSServer when selected.  This allows the input method
  to work by filtering pen movements on the screen or filtering
  device keys.  Returning 0 indicates no input modifier is
  provided and that the input method uses some other method
  for the user to generated text.

  The base class returns 0.
*/
QWSInputMethod *QtopiaInputMethod::inputModifier()
{
    return 0;
}

/*!
  This function is called when the user clicks on the status
  icon of the input method.  It is only called doesn't provide
  its own status widget.

  \sa statusWidget()
*/
void QtopiaInputMethod::clicked() {}

/*!
    \fn void QtopiaInputMethod::setHint( const QString &hint, bool restricted)

  This is the primary communication channel from the server to the
  Input Method, and sets the hint describing the sort of input the current
  input widget requires to \a hint.  It must be implemented in Input
  Method plugins.

  If \a restricted is true
  the widget only accepts one kind of input and input method
  should disable any mode switching.  For example, the
  home screen Qt Extended might set the hint to be restricted
  to phone numbers.

  an empty hint is used to indicate that input is not needed for the
  current focused widget- the Input Method should deactivate.

  Common hints are
  words  - Dictonary words such as some notes
  text - Names, passwords etc.
  extended - text with extensions for additional character sets
  number - 0-9
  phone - Phone numbers, 0-9 plus phone control codes such as 'p' 'x' or 'w'.

  Application may also define custom hints, such as email to
  describe an email address.  Input methods should provide
  reasonable default behavior for hints they do not recognize.
*/

//void QtopiaInputMethod::setHint( const QString &hint, bool restricted)
//{
//    Q_UNUSED(hint);
//    Q_UNUSED(restricted);
//}

/*!
  Returns true if the property flag \a p is set to true for
  this input method.  Otherwise returns false.
*/
bool QtopiaInputMethod::testProperty(int p) const
{
    return (properties() & p) == p;
}

/*!
    This function is called by the system to notify it that focus has changed.
    Focus changes will usually trigger a new hint as well.

    \sa setHint()
*/
void QtopiaInputMethod::focusChanged(){
}

/*!
    This slot is triggered when the input methods action has been
    activated from the softkey menu.  Implement in a subclass, or connect the
    slot to react to menu activation.

    \bold{Note:} This is the only way to receive notification of menu action
    activations, as the Softkey Menu has separate copies of the QAction,
    often in a separate process to the input method.  However, connecting this
    slot to other slots in the input method or server will work normally.

    The \a data is the id from the id field of the \l QIMActionDescription used
    to create the QAction that was triggered.  Be sure to assign a unique id
    to each menu action, so as to be able to tell them apart here.

    0 is the default value returned if the server was not able to determine
    which action was triggered for some reason.

    See also: \l QIMActionDescription
*/

void QtopiaInputMethod::menuActionActivated(int data){Q_UNUSED(data)};


/*!
    Implement this function in a derived class to return a description of
    the menu that should be installed on the input methods behalf.

    This function is called by Qt Extended when the input methods menu is shown,
    or when the inputmethod emits stateChanged(), in order to construct the
    softkey menu entries for the input method.

    If the list contains only a single item, the server interprets this as a
    single menu item, and softkey puts it in its top-level menu, and returns
    its id when it is activated.

    If the list contains more than one item, the server interprests the first
    as the icon and label for a menu item in the top level softkey menu, and
    subsequent items as members of that submenu.  Note that in this case the
    first item will become a menu, and will not be triggerable, so its id will
    never be sent back to the IM.

    Each IMAction description includes an id to identify the action, a
    user-visible label, and the filename of the icon used for the action.
    Note that the icon will usually be instantiated from a different process
    that could have a different environment from the inputmethod, so the file
    name must be fully qualified.

    The default implementation returns an empty list, which will result in
    no menu actions being added on behalf of the input method.

   see also: QIMActionDescription
*/
QList<QIMActionDescription*> QtopiaInputMethod::menuDescription() {
    return QList<QIMActionDescription*>();
};

template <typename T>
/*!
  Convert data from  an action description to \a stream.
  Used in converting QIMActionDescription to QVariant.
*/
void QIMActionDescription::serialize(T &stream) const
{
    stream<< d->m_id; 
    stream<< d->m_label;
    stream<< d->m_iconFileName; 
};

template <typename T>
/*! 
  Convert data from \a stream to an action description.  
  Used in converting QIMActionDescription from QVariant.
*/
void QIMActionDescription::deserialize(T& stream)
{
    Q_ASSERT(d);
    stream >> d->m_id;
    stream >> d->m_label;
    stream >> d->m_iconFileName;
};

Q_IMPLEMENT_USER_METATYPE( QIMActionDescription );
