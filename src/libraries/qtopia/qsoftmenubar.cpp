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

#include "qsoftmenubar.h"
#include "contextkeymanager_p.h"
#include <QMenu>
#include <QKeyEvent>
#include <QDebug>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QStyle>
#include <QClipboard>
#ifndef QT_NO_WIZARD
# include <QWizard>
# include <QAbstractButton>
#endif

#include <QtopiaInputMethod>

#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <QValueSpaceItem>

/*!
  \class QSoftMenuBar
    \inpublicgroup QtBaseModule

  \brief The QSoftMenuBar class allows the labels in the phone
  soft menu bar to be set.

  QSoftMenuBar is
  used to set the icon/text in the soft menu bar to describe the action
  performed when pressing the associated soft button.  Keep in
  mind that not all phones have soft keys and therefore may not have
  a visible soft menu bar.  The application should be useable without
  soft key shortcuts.

  The labels are updated whenever a widget gains or loses focus
  or their navigation focus state changes.  Therefore, the labels should
  be set for each widget that may gain focus, and will respond to a
  soft key.  The standard Qt and Qt Extended widgets
  set the labels appropriately, so this is usually only necessary for
  custom widgets.

  The rule for determining what label is displayed on the soft menu bar and
  where the key events are delivered is:

\list
  \o If the current focus widget has claimed the key, then the corresponding
  context label is shown.
  \o Otherwise, the widget's parent, grandparent and so on may set the label.
  \o If the focus widget has not claimed the key and an ancestor has, then
  the key event will be sent directly to the ancestor.
\endlist

  QSoftMenuBar labels specify both a pixmap and a text label.  The user
  can choose whether the text or pixmap labels are shown.

  QSoftMenuBar is only available in the Qt Extended Phone Edition.

  \ingroup userinput
*/

/*!
  \enum QSoftMenuBar::FocusState

  \value EditFocus apply the label setting when the widget has edit focus.
  \value NavigationFocus apply the label setting when the widget does not have edit focus.
  \value AnyFocus apply the label setting regardless of focus state.
*/

/*!
  \enum QSoftMenuBar::StandardLabel

  \value NoLabel
  \value Options
  \value Ok
  \value Edit
  \value Select
  \value View
  \value Cancel
  \value Back
  \value BackSpace
  \value Next
  \value Previous
  \value EndEdit
  \value RevertEdit
  \value Deselect
  \value Finish
*/

/*!
  \enum QSoftMenuBar::LabelType
  \value IconLabel
  \value TextLabel
*/


QSoftMenuBar::QSoftMenuBar()
{
}


/*!
  \fn void QSoftMenuBar::setLabel(QWidget *widget, int key, const QString &pixmap, const QString &text, FocusState state)

  When \a widget gains the specified focus \a state, the label for \a key
  is set to \a text and \a pixmap.
*/
void QSoftMenuBar::setLabel(QWidget *w, int key, const QString &pm, const QString &t, FocusState state)
{
    ContextKeyManager::instance()->setContextText(w, key, t, state);
    ContextKeyManager::instance()->setContextPixmap(w, key, pm, state);
}

/*!
  When \a widget gains the specified focus \a state, the label for
  \a key is set to the standard \a label.
*/
void QSoftMenuBar::setLabel(QWidget *widget, int key, StandardLabel label, FocusState state)
{
    ContextKeyManager::instance()->setContextStandardLabel(widget, key, label, state);
}

/*!
  Clears any label set for \a widget and \a key in focus \a state.

  \sa setLabel()
*/
void QSoftMenuBar::clearLabel(QWidget *widget, int key, FocusState state)
{
    ContextKeyManager::instance()->clearContextLabel(widget, key, state);
}

/*!
  Returns the list of soft keys (i.e. keys with a corresponding label
  in the soft menu bar).
*/
const QList<int> &QSoftMenuBar::keys()
{
    return ContextKeyManager::instance()->keys();
}

#ifndef QT_NO_CLIPBOARD
class EditMenu : public QMenu
{
    Q_OBJECT
public:
    EditMenu(QWidget *parent)
        : QMenu(parent)
    {
        selectAction = addAction(QIcon(":icon/selecttext"), tr("Select text..."));
        copyAction = addAction(QIcon(":icon/copy"), tr("Copy text"));
        pasteAction = addAction(QIcon(":icon/paste"), tr("Paste text"));
        connect(this, SIGNAL(triggered(QAction*)),
                this, SLOT(editMenuActivated(QAction*)));

        // use aboutToShow() instead of overriding showEvent() to customise the
        // menu, or else the menu flickers if screen repainting is slow
        connect(this, SIGNAL(aboutToShow()),
                this, SLOT(menuAboutToShow()));
    }

    static bool keyEventFilter( QObject *o, QEvent *e )
    {
        if (highlighting) {
            if ( e->type() == QEvent::KeyPress) {
                QKeyEvent *ke = (QKeyEvent *)e;
                if ( ke->key() >= Qt::Key_Left && ke->key() <= Qt::Key_Down ) {
                    if ( !(ke->modifiers()&Qt::ShiftModifier) ) {
                        if ( highlighting ) {
                            qApp->postEvent(o,new QKeyEvent(ke->type(),ke->key(),ke->modifiers()|Qt::ShiftModifier,ke->text(),ke->isAutoRepeat(),ke->count()));
                            return true;
                        }
                    }
                } else {
                    highlighting = 0;
                }
            }
        }
        return false;
    }

private slots:
    void menuAboutToShow()
    {
        // actions might have been removed
        if (!selectAction && !copyAction && !pasteAction)
            return;

        QWidget *w = QApplication::focusWidget();
        QTextEdit *te = qobject_cast<QTextEdit*>(w);
        QLineEdit *le = qobject_cast<QLineEdit*>(w);

        bool readOnly = (te && te->isReadOnly() || le && le->isReadOnly());
        bool showPaste = false;
        bool hasText = (te && te->document() && !te->document()->isEmpty()) ||
                       (le && !le->text().isEmpty());

        if (!readOnly && pasteAction) {
            if (te) {
                showPaste = te->canPaste();
            } else if (le) {
                const QMimeData *mime = QApplication::clipboard()->mimeData();
                if (mime && mime->hasText())
                    showPaste = true;
            }
        }

        if (te && te->hasEditFocus() || le && le->hasEditFocus()) {
            if ( (te && te->textCursor().hasSelection()) ||
                    (le && le->hasSelectedText()) ) {
                if (selectAction)
                    selectAction->setVisible(false);
                if (copyAction)
                    copyAction->setVisible(true);
                if (pasteAction)
                    pasteAction->setVisible(showPaste);
            } else {
                if (selectAction)
                    selectAction->setVisible(hasText);
                if (copyAction)
                    copyAction->setVisible(false);
                if (pasteAction)
                    pasteAction->setVisible(showPaste);
            }
        } else {
            // in navigation mode
            if (selectAction)
                selectAction->setVisible(false);
            if (copyAction)
                copyAction->setVisible(hasText);
            if (pasteAction)
                pasteAction->setVisible(showPaste);
        }
    }

    void editMenuActivated(QAction *action)
    {
        QWidget* w = qApp->focusWidget();
        QTextEdit* te = qobject_cast<QTextEdit*>(w);
        QLineEdit* le = qobject_cast<QLineEdit*>(w);

        if (action == selectAction) {
            highlighting = this;
            return;
        }
        highlighting = 0;

        if (action == copyAction) {
            if (te) {
                if (te->hasEditFocus()) {
                    te->copy();
                } else {
                    QTextCursor c = te->textCursor();
                    te->selectAll();
                    te->copy();
                    te->setTextCursor(c);   // reset selection
                }
            } else if (le) {
                if (le->hasEditFocus()) {
                    le->copy();
                } else {
                    qApp->clipboard()->setText(le->text());
                }
            }
        } else if (action == pasteAction) {
            // assumes clipboard is not empty if 'Paste' is able to be
            // activated, otherwise the line/textedit might be cleared
            // without pasting anything back into it
            if (te) {
                if (!te->hasEditFocus())
                    te->clear();
                te->paste();
                if (!te->hasEditFocus()) {
                    te->moveCursor(QTextCursor::Start);
                    te->ensureCursorVisible();
                }
            } else if (le) {
                if (!le->hasEditFocus())
                    le->clear();
                le->paste();
                if (!le->hasEditFocus())
                    le->home(false);
            }
        }
    }

private:
    static QPointer<EditMenu> highlighting;

    QPointer<QAction> selectAction;
    QPointer<QAction> copyAction;
    QPointer<QAction> pasteAction;
};
QPointer<EditMenu> EditMenu::highlighting;

#endif // QT_NO_CLIPBOARD

class MenuManager : public QObject
{
    Q_OBJECT
public:
    MenuManager();

    QMenu *menuFor(QWidget *w, QSoftMenuBar::FocusState state);
    bool hasMenu(QWidget *w, QSoftMenuBar::FocusState state);
    void addMenuTo(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state);
    void removeMenuFrom(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state);
    QWidgetList widgetsFor(const QMenu *menu, QSoftMenuBar::FocusState state);
    void setHelpEnabled(QWidget *widget, bool enable);
    void setCancelEnabled(QWidget *widget, bool enable);
    void setInputMethodEnabled(QWidget *widget, bool enable);
    int key();

    static MenuManager *instance() {
        if (!mmgr)
            mmgr = new MenuManager;
        return mmgr;
    }

    QMenu *getActiveMenu() const;

protected:
    bool eventFilter(QObject *, QEvent *e);

private:
    void popup(QWidget *w, QMenu *menu);
    QMenu *internalMenuFor(QWidget *w, QSoftMenuBar::FocusState state);
    bool helpExists(QWidget *w);
    QList<QWidget*> widgetsWithMenu(QMenu *menu, QSoftMenuBar::FocusState state);
    QString findHelp(const QWidget* w);
    bool triggerMenuItem( QMenu *menu, int keyNum );
    QPoint positionForMenu(const QMenu *menu);

private slots:
    void widgetDestroyed();
    void menuDestroyed();
    void help();
    void inputMethod();
    void rotationChanged();

private:
    struct WidgetData {
        QString helpFile;
        int shown : 1;
        int helpexists : 1;
        int helpEnabled : 1;
        int cancelEnabled : 1;
        int hasHelp : 1;
        int inputMethodEnabled : 1;
    };
    QWidget *focusWidget;
    QMap<QWidget*, QWidget*> focusWidgetMap;
    QMap<QWidget*, QMenu*> modalMenuMap;
    QMap<QWidget*, QMenu*> nonModalMenuMap;
    QMap<QWidget*, WidgetData> widgetDataMap;
    QPointer<QAction> sepAction;
    QPointer<QAction> helpAction;
    QPointer<QAction> inputMethodAction;
    QPointer<QAction> cancelAction;
    QPointer<QAction> previousAction;
    QPointer<QAction> nextAction;
    static QPointer<MenuManager> mmgr;
    QPointer<QMenu> activeMenu;
    QSettings gConfig;
};

QPointer<MenuManager> MenuManager::mmgr = 0;

/*!
  Returns the QMenu assigned to \a widget in focus
  \a state.  If a QMenu does not yet exist for the widget,
  an empty menu will be created with \a widget as its parent.

  \sa addMenuTo()
*/
QMenu *QSoftMenuBar::menuFor(QWidget *widget, FocusState state)
{
    return MenuManager::instance()->menuFor(widget, state);
}

/*!
  Returns true if the \a widget has a menu assigned to it
  in focus \a state.

  \sa menuFor()
*/
bool QSoftMenuBar::hasMenu(QWidget *widget, FocusState state)
{
    return MenuManager::instance()->hasMenu(widget, state);
}

/*!
  Adds \a menu to \a widget for focus \a state.

  \sa removeMenuFrom()
*/
void QSoftMenuBar::addMenuTo(QWidget *widget, QMenu *menu, FocusState state)
{
    MenuManager::instance()->addMenuTo(widget, menu, state);
}

/*!
  Removes the \a menu from \a widget for focus \a state.

  \sa addMenuTo()
*/
void QSoftMenuBar::removeMenuFrom(QWidget *widget, QMenu *menu, FocusState state)
{
    MenuManager::instance()->removeMenuFrom(widget, menu, state);
}

/*!
  Returns all widgets that have added \a menu to the menu bar for focus
  \a state.

  \sa addMenuTo(), removeMenuFrom(), menuFor(), hasMenu()
*/
QWidgetList QSoftMenuBar::widgetsFor(const QMenu *menu, QSoftMenuBar::FocusState state)
{
    return MenuManager::instance()->widgetsFor(menu, state);
}

/*!
  Sets whether the Help menu option is available for \a widget when a menu
  associated with it is shown.  The Help option will only be shown if there
  is a \l {File Name Standards}{help file} available.
  If \a enable is true the help menu option may be displayed for the widget.

  The default is true.
*/
void QSoftMenuBar::setHelpEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setHelpEnabled(widget, enable);
}

/*!
  Sets whether the Input Method menu option is available for \a widget when a
  menu associated with it is shown.  If \a enable is true an Input Method
  menu action may be displayed for the widget.

  The default is true.
*/
void QSoftMenuBar::setInputMethodEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setInputMethodEnabled(widget, enable);
}

/*!
  Sets whether Cancel is available for \a widget when a menu
  associated with it is shown. The Cancel option is only shown if the
  widget's top level window is a QDialog.
  If \a enable is true a Cancel menu action will be displayed for a dialog.

  The default is true.
*/
void QSoftMenuBar::setCancelEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setCancelEnabled(widget, enable);
}

/*!
  Returns the key that activates menus managed by the soft menu bar.
*/
int QSoftMenuBar::menuKey()
{
    return MenuManager::instance()->key();
}

/*!
  Creates and returns a standard "Edit" menu used for QLineEdit and QTextEdit.

  Returns 0 if the clipboard functionality has been disabled in Qt for Embedded Linux.
*/
QMenu *QSoftMenuBar::createEditMenu()
{
#ifndef QT_NO_CLIPBOARD
    return new EditMenu(0);
#else
    return 0;
#endif
}

/*!
  \internal
*/
QMenu* QSoftMenuBar::activeMenu()
{
    return MenuManager::instance()->getActiveMenu();
}

MenuManager::MenuManager()
    : QObject(qApp)
    , focusWidget(0)
    , sepAction(0)
    , helpAction(0)
    , inputMethodAction(0)
    , cancelAction(0)
    , previousAction(0)
    , nextAction(0)
    , activeMenu(0)
    , gConfig("Trolltech", "qpe")
{
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    QValueSpaceItem *rotation = new QValueSpaceItem("/UI/Rotation/Current");
    QObject::connect(rotation, SIGNAL(contentsChanged()),
                     this, SLOT(rotationChanged()));
#endif
}

QMenu *MenuManager::menuFor(QWidget *w, QSoftMenuBar::FocusState state)
{
    if ( !w )
        return 0;

    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QMenu *m = internalMenuFor(w, state);
#ifndef QT_NO_CLIPBOARD
    if (!m && (w->inherits("QLineEdit") ||
            w->inherits("QTextEdit") && !w->inherits("QTextBrowser"))) {
        m = new EditMenu(w);
        addMenuTo(w, m, state);
    } else
#endif
    if (!m) {
        m = new QMenu(w);
        addMenuTo(w, m, state);
    }
    return m;
}

bool MenuManager::hasMenu(QWidget *w, QSoftMenuBar::FocusState state)
{
    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QMenu *m = internalMenuFor(w, state);
    return ( m != 0 );
}

void MenuManager::addMenuTo(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state)
{
    if (!w || !menu)
        return;

    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QWidget *fw = w;
    while (fw->focusProxy())
        fw = fw->focusProxy();

    bool haveModal = internalMenuFor(w, QSoftMenuBar::EditFocus) == menu;
    bool haveNonModal = internalMenuFor(w, QSoftMenuBar::NavigationFocus) == menu;

    if (!focusWidgetMap.contains(fw)) {
        fw->installEventFilter(this);
        w->installEventFilter(this);
        focusWidgetMap[fw] = w;
    }
    if (widgetsWithMenu(menu, QSoftMenuBar::AnyFocus).isEmpty()) {
        menu->installEventFilter(this);
        connect(menu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));
    }
    QSoftMenuBar::setLabel(fw, key(), QSoftMenuBar::Options, state);
    QSoftMenuBar::setLabel(w, key(), QSoftMenuBar::Options, state);
    QSoftMenuBar::setLabel(menu, key(), "options-hide", tr("Hide"), QSoftMenuBar::AnyFocus);
    if (QApplication::style()->inherits("Series60Style"))
        QSoftMenuBar::setLabel(menu, key(), "select", tr("Select"), QSoftMenuBar::AnyFocus);

    if (state & QSoftMenuBar::EditFocus)
        modalMenuMap.insert(w, menu);
    if (state & QSoftMenuBar::NavigationFocus)
        nonModalMenuMap.insert(w, menu);

    WidgetData d;
    d.shown = 0;
    d.helpexists = 0;
    d.helpEnabled = 1;
    d.inputMethodEnabled = 1;
    d.cancelEnabled = 1;
    d.hasHelp = 0;
    widgetDataMap.insert(w, d);

    if (!haveModal && !haveNonModal)
        connect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
}

void MenuManager::removeMenuFrom(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state)
{
    if ( !w )
        return;

    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QWidget *fw = w;
    while (fw->focusProxy())
        fw = fw->focusProxy();

    bool haveModal = internalMenuFor(w, QSoftMenuBar::EditFocus) == menu;
    bool haveNonModal = internalMenuFor(w, QSoftMenuBar::NavigationFocus) == menu;

    if (state & QSoftMenuBar::EditFocus && haveModal) {
        QSoftMenuBar::clearLabel(fw, key(), QSoftMenuBar::EditFocus);
        QSoftMenuBar::clearLabel(w, key(), QSoftMenuBar::EditFocus);
        modalMenuMap.remove(w);
        haveModal = false;
    }
    if (state & QSoftMenuBar::NavigationFocus && haveNonModal) {
        QSoftMenuBar::clearLabel(fw, key(), QSoftMenuBar::NavigationFocus);
        QSoftMenuBar::clearLabel(w, key(), QSoftMenuBar::NavigationFocus);
        nonModalMenuMap.remove(w);
        haveNonModal = false;
    }

    if (!haveModal && !haveNonModal) {
        fw->removeEventFilter(fw);
        w->removeEventFilter(fw);
        focusWidgetMap.remove(fw);
        disconnect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
    }
    if (widgetsWithMenu(menu, QSoftMenuBar::AnyFocus).isEmpty()) {
        menu->removeEventFilter(this);
        disconnect(menu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));
    }
    widgetDataMap.remove(w);
}

QWidgetList MenuManager::widgetsFor(const QMenu *menu, QSoftMenuBar::FocusState state)
{
    QWidgetList wl;

    QMap<QWidget*, QMenu*>::iterator it;
    if (state & QSoftMenuBar::EditFocus) {
        for (it = modalMenuMap.begin(); it != modalMenuMap.end(); ++it) {
            if (*it == menu)
                wl.append(it.key());
        }
    }

    if (state & QSoftMenuBar::NavigationFocus) {
        for (it = nonModalMenuMap.begin(); it != nonModalMenuMap.end(); ++it) {
            if (*it == menu)
                wl.append(it.key());
        }
    }

    return wl;
}

void MenuManager::setHelpEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].helpEnabled = enable;
}

void MenuManager::setInputMethodEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].inputMethodEnabled = enable;
}

void MenuManager::setCancelEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].cancelEnabled = enable;
}

int MenuManager::key()
{
    static int k = -1;
    if (k < 0) {
        if (!Qtopia::hasKey(Qt::Key_Menu)) {
            if (Qtopia::hasKey(Qt::Key_Context1))
                k = Qt::Key_Context1;
            else
                qWarning("Cannot map key to QMenu");
        } else {
            k = Qt::Key_Menu;
        }
    }

    return k;
}

QMenu *MenuManager::getActiveMenu() const
{
    return activeMenu;
}

bool MenuManager::triggerMenuItem( QMenu *menu, int keyNum )
{
    QList<QAction *> list = menu->actions();
    int count = list.count();
    keyNum = keyNum - 48;
    int index = 1;
    if (count) {
        for (int i = 0 ; i < count ; i++) {
            if ( !list.at(i)->isSeparator() && list.at(i)->isVisible() ) {
                if (keyNum == index && list.at(i)->isEnabled()) {
                    emit list.at(i)->trigger();
                    return true;
                } else {
                    index++;
                    if ( index == 10 )
                        index = 0;
                }
            }
        }
    }
    return false;
}

bool MenuManager::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Hide) {
        // If the window hiding has a visible menu, hide the menu too.
        QMap<QWidget*, QWidget*>::ConstIterator it;
        QWidget *w = 0;
        it = focusWidgetMap.find((QWidget*)o);
        if (it == focusWidgetMap.end())
            w = qobject_cast<QWidget*>(o);
        else
            w = *it;
        if (w) {
            QMenu *menu = internalMenuFor(w, w->hasEditFocus() ? QSoftMenuBar::EditFocus : QSoftMenuBar::NavigationFocus);
            if (menu && menu->isVisible()) {
                menu->close();
            }
        }
        return false;
    }

    if (e->type() == QEvent::Resize) {
        QMenu *menu = qobject_cast<QMenu*>(o);
        if (menu && menu->isVisible()) {    //menu needs to be moved when resized (since it is bottom-anchored)
            menu->move(positionForMenu(menu));
        }
    }

    if (!o || !e || e->type() != QEvent::KeyPress )
        return false;

    if (!o->isWidgetType())
        return false;

    QKeyEvent *k = (QKeyEvent *) e;
    int keyNum = k->key();

    QMenu *menu = qobject_cast<QMenu*>(o);
    if (menu && keyNum >= Qt::Key_0 && keyNum <= Qt::Key_9 ) {
        // trigger menuitem and close menu
        if (triggerMenuItem(menu, keyNum)) {
            activeMenu = 0;
            menu->close();
        }
    }

    if (k->key() != key()) {
#ifndef QT_NO_CLIPBOARD
        return EditMenu::keyEventFilter(o,e);
#else
        return false;
#endif
    } else if (QApplication::style()->inherits("Series60Style") && menu) {
        QAction *a = menu->activeAction();
        if (a) emit a->trigger();
    }

    focusWidget = (QWidget*)o;

    if (e->type() == QEvent::KeyPress && !((QKeyEvent*)e)->isAutoRepeat()) {
        if (menu) {
            menu->close();
        } else {
            QMap<QWidget*, QWidget*>::ConstIterator it;
            QWidget *w = 0;
            it = focusWidgetMap.find(focusWidget);
            if (it == focusWidgetMap.end()) {
                w = qobject_cast<QWidget*>(o);
            } else {
                w = *it;
            }
            if (w) {
                QMenu *menu = internalMenuFor(w, w->hasEditFocus() ? QSoftMenuBar::EditFocus : QSoftMenuBar::NavigationFocus);
                if (menu) {
                    activeMenu = menu;
                    popup(w, menu);
                    return true;
                }
            }
        }
    }
    return false;
}

QMenu *MenuManager::internalMenuFor(QWidget *w, QSoftMenuBar::FocusState state)
{
    QMap<QWidget*, QMenu*>::ConstIterator it;
    if (state & QSoftMenuBar::EditFocus) {
        it = modalMenuMap.find(w);
        if (it != modalMenuMap.end())
            return *it;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        it = nonModalMenuMap.find(w);
        if (it != nonModalMenuMap.end())
            return *it;
    }

    return 0;
}

QList<QWidget*> MenuManager::widgetsWithMenu(QMenu *menu, QSoftMenuBar::FocusState state)
{
    QList<QWidget*> list;
    QMap<QWidget*, QMenu*>::ConstIterator it;
    if (state & QSoftMenuBar::EditFocus) {
        for (it = modalMenuMap.begin(); it != modalMenuMap.end(); ++it) {
            if (*it == menu)
                list += it.key();
        }
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        for (it = nonModalMenuMap.begin(); it != nonModalMenuMap.end(); ++it) {
            if (*it == menu && !list.contains(it.key()))
                list += it.key();
        }
    }

    return list;
}

void MenuManager::widgetDestroyed()
{
    QWidget *w = (QWidget*)sender();
    QMenu *menu = internalMenuFor(w, QSoftMenuBar::AnyFocus);
    if (menu) {
        removeMenuFrom(w, menu, QSoftMenuBar::AnyFocus);
    }
}

void MenuManager::menuDestroyed()
{
    QMenu *menu = (QMenu*)sender();
    QList<QWidget*> widgets = widgetsWithMenu(menu, QSoftMenuBar::AnyFocus);
    QList<QWidget*>::Iterator it;
    for (it = widgets.begin(); it != widgets.end(); ++it)
        removeMenuFrom(*it, menu, QSoftMenuBar::AnyFocus);
}

void MenuManager::help()
{
    QWidget* w = focusWidget;
    if (!w)
        w = qApp->activeWindow();
    if (!w)
        return;
    QWidget* focus = w;
    w = w->topLevelWidget();
    if (helpExists(w)) {
        QString hf = findHelp(focus);
        if ( hf.isEmpty() )
            hf = findHelp( w );
        if (hf.isEmpty()) {
            WidgetData &d = widgetDataMap[w];
            hf = d.helpFile;
        }
        qLog(Help) << ">>> Using help " << hf << "<<<";
        QtopiaServiceRequest env("Help", "setDocument(QString)");
        env << hf;
        env.send();
    }
}

void MenuManager::inputMethod()
{
    // Input method is likely to be in different process than server
    // so send notification via ipc
    QtopiaIpcEnvelope envelope("QPE/InputMethod", "activateMenuItem(int)");
    QAction *action =qobject_cast<QAction *>(sender());
    if(action){
        qLog(Input) << "Notifying IM that menu id " << action->data().toInt() << " has been activated";
        envelope << action->data().toInt();
    }
    else
    {
        envelope << 0;
        qLog(Input) << "MenuManager not able to determine which input method menu action was selected";
    };
}

void MenuManager::popup(QWidget *w, QMenu *menu)
{
    if (!widgetDataMap.contains(w))
        return;

    WidgetData &d = widgetDataMap[w];

    if (!d.shown ) {
        d.hasHelp = helpExists(w);
        d.shown = true;
    }

    if (sepAction) {
        delete sepAction;
        sepAction = 0;
    }

    if (helpAction) {
        delete helpAction;
        helpAction = 0;
    }

    if (cancelAction ) {
        delete cancelAction;
        cancelAction = 0;
    }

    if (previousAction ) {
        delete previousAction;
        previousAction = 0;
    }

    if (nextAction ) {
        delete nextAction;
        nextAction = 0;
    }

    // for now, refresh every time, in case input method has changed it's action
    // TODO: InputMethods should let us know when the IMMenu changes
    if(inputMethodAction)
    {
        delete inputMethodAction;
        inputMethodAction=0;
    }

    int visibleActionCount = 0;
    foreach ( QAction *action, menu->actions() )
        if ( action->isVisible() )
            visibleActionCount++;

    if (visibleActionCount)
        sepAction = menu->addSeparator();

    QList<QVariant> descriptionList = (QValueSpaceItem("UI/InputMethod").value( "MenuItem" )).toList();

    if(d.inputMethodEnabled && !descriptionList.isEmpty()){
        qLog(Input) << "Adding IM action to QSoftMenuBar menu";
        QIMActionDescription desc = descriptionList.first().value<QIMActionDescription>();
        inputMethodAction = menu->addAction(QIcon(desc.iconFileName()),desc.label());
        inputMethodAction->setData(desc.id());
        connect(inputMethodAction, SIGNAL(triggered()), this, SLOT(inputMethod()));

        if(descriptionList.size() > 1)
        {
            QMenu* IMMenu = new QMenu();
            inputMethodAction->setMenu(IMMenu);
            connect(IMMenu, SIGNAL(triggered(QAction*)), this, SLOT(inputMethod()));
            qLog(Input) << "QSoftMenuBar is building IM menu";
            QList<QVariant>::iterator i = descriptionList.begin();
            ++i;
            for(; i != descriptionList.end(); ++i) {
                qLog(Input) << "Appending QAction from QIMActionDescription("<< i->value<QIMActionDescription>().id() << ","<< i->value<QIMActionDescription>().label() << ","<< i->value<QIMActionDescription>().iconFileName() <<")";

                QAction* action = IMMenu->addAction(QIcon(i->value<QIMActionDescription>().iconFileName()), i->value<QIMActionDescription>().label());
                action->setData(i->value<QIMActionDescription>().id());
                connect(action, SIGNAL(triggered()), this, SLOT(inputMethod()));
            }
        }
    }

    if (d.helpEnabled && d.hasHelp )
        helpAction = menu->addAction(QIcon( ":icon/help" ),
            tr("Help"), this, SLOT(help()));


    if (w) {
        QWidget* tlw = w->topLevelWidget();
        if (d.cancelEnabled && tlw->inherits("QDialog") && !Qtopia::hasKey(Qt::Key_No)) {
            if (!QtopiaApplication::isMenuLike((QDialog*)tlw))
                cancelAction = menu->addAction(QIcon(":icon/cancel"), tr("Cancel"), tlw, SLOT(reject()));
        }
#ifndef QT_NO_WIZARD
        if (QWizard *wizard = qobject_cast<QWizard*>(tlw)) {
            QAbstractButton *button = wizard->button(QWizard::BackButton);
            if (button && button->isEnabled())
                previousAction = menu->addAction(QIcon(":icon/previous"), tr("Previous"), tlw, SLOT(back()));

            button = wizard->button(QWizard::NextButton);
            if (button && button->isEnabled())
                nextAction = menu->addAction(QIcon(":icon/next"), tr("Next"), tlw, SLOT(next()));

            button = wizard->button(QWizard::FinishButton);
            if (button && button->isEnabled())
                nextAction = menu->addAction(QIcon(":icon/done"), tr("Finish"), tlw, SLOT(accept()));
        }
#endif
    }

    if( !helpAction && !cancelAction && !inputMethodAction && !previousAction && !nextAction && sepAction ) {
        delete sepAction;
        sepAction = 0;
    }

    if (menu->actions().count()) {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect r = desktop->availableGeometry(desktop->primaryScreen());
        int x = 0;
        gConfig.beginGroup(QLatin1String("Style"));
        QString mpos = gConfig.value("MenuAlignment", "left").toString();
        gConfig.endGroup();
        if (mpos.contains(QLatin1String("hcenter"), Qt::CaseInsensitive)) {
            x = r.left() + (r.width()-menu->sizeHint().width())/2;
        } else {
            bool rtl = !( QApplication::layoutDirection() == Qt::LeftToRight );
            if (mpos.contains(QLatin1String("right"), Qt::CaseInsensitive))
                rtl = !rtl;
            x = rtl ? r.right() - menu->sizeHint().width()+1 : r.left();
        }
        menu->popup(QPoint(x, r.bottom()), menu->actions().last());  //last item at bottom of screen

        if (!Qtopia::mousePreferred()) {
            //select first action by default
            foreach (QAction *a, menu->actions()) {
                if (a->isEnabled() && a->isVisible() && !a->isSeparator()) {
                    if ( a->menu() == 0 ) {
                        menu->setActiveAction(a);
                    } else {
                        // if first item points to submenu, remove the submenu
                        // before setActiveAction() or else the submenu will
                        // be shown when the menu pops up
                        QMenu *m = a->menu();
                        a->setMenu(0);
                        menu->setActiveAction(a);
                        a->setMenu(m);
                    }
                    break;
                }
            }
        }
    }
}

QString MenuManager::findHelp(const QWidget* w)
{
    // seems like we sometimes start an app with just the name and sometimes with a full path.
    // So make sure we only use the basename here
    QFileInfo fi( QString(qApp->argv()[0]) );
    QString hf;
    if( w==NULL ) {
        hf = fi.baseName() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
            if ( QFile::exists( *it + "/" + hf ) ) {
                qLog(Help) << "Using help " << hf;
                return hf;
                break;
            }
        }
    }

    const QObject *widget=w;
    while(widget) {
        for (int qualified=1; qualified>=0; --qualified) {
            qLog(Help) << "checking object with widget->metaObject()->className():" << widget->metaObject()->className() << "widget->objectName():" << widget->objectName().toLower();
            if ( qualified ) {
                hf = fi.baseName() + "-";
            } else {
                hf.clear();
            }
            hf += widget->objectName().toLower() + ".html";
            QStringList helpPath = Qtopia::helpPaths();
            for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
                if ( QFile::exists( *it + "/" + hf ) ) {
                    qLog(Help) << "Using help " << hf;
                    return hf;
                    break;
                }
            }
        }
        widget=widget->parent();
    }

    if ( qLogEnabled(Help) ) {
        hf = fi.baseName();
        if ( w && w->objectName().isEmpty() ) {
            QString parents;
            parents = QString("(%1)").arg(w->metaObject()->className());
            QObject *p = w->parent();
            while (p) {
                parents = QString("(%1)%2").arg(p->metaObject()->className()).arg(parents);
                p = p->parent();
            }
            qLog(Help) << "No help for" << hf << parents;
        } else {
            qLog(Help) << "No help for " << hf;
        }
    }

    return "";
}

bool MenuManager::helpExists(QWidget *w)
{
    WidgetData &d = widgetDataMap[w];
    if (d.helpFile.isNull()) {
        QString hf = findHelp(0);
        bool he = !hf.isEmpty();
        if (!he)
            qWarning() << ">>> NO help exists for" << QString(qApp->argv()[0]);
        if ( QtopiaService::app("Help").isNull() )
            he = false;
        d.helpFile = hf;
        d.helpexists = he;
        return he;
    }
    if (!d.helpexists)
        qWarning() << ">>> NO help exists for" << QString(qApp->argv()[0]);
    return d.helpexists;
}

QPoint MenuManager::positionForMenu(const QMenu *menu)
{
    if (!menu)
        return QPoint(0,0);
    QDesktopWidget *desktop = QApplication::desktop();
    QRect r = desktop->availableGeometry(desktop->primaryScreen());
    QPoint pos;
    if ( QApplication::layoutDirection() == Qt::LeftToRight )
        pos = QPoint(r.left(), r.bottom() - menu->sizeHint().height() + 1);
    else {
        int x = r.right() - menu->sizeHint().width()+1;
        if ( x < 0 ) x=1;
        pos = QPoint(x, r.bottom() - menu->sizeHint().height() + 1);
    }
    return pos;
}

void MenuManager::rotationChanged()
{
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    QMenu *menu = getActiveMenu();
    if (menu && menu->isVisible() ) {
        menu->hide();
    }
#endif
}

#include "qsoftmenubar.moc"
