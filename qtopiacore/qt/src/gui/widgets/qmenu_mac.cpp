/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qmenu.h"
#include "qhash.h"
#include <qdebug.h>
#include "qapplication.h"
#include <private/qt_mac_p.h>
#include "qregexp.h"
#include "qmainwindow.h"
#include "qdockwidget.h"
#include "qtoolbar.h"
#include "qevent.h"
#include "qstyle.h"
#include "qdebug.h"
#include "qwidgetaction.h"

#include <private/qapplication_p.h>
#include <private/qmenu_p.h>
#include <private/qmenubar_p.h>
#include <private/qhiviewwidget_mac_p.h>

QT_BEGIN_NAMESPACE

/*****************************************************************************
  QMenu debug facilities
 *****************************************************************************/

/*****************************************************************************
  QMenu globals
 *****************************************************************************/
bool qt_mac_no_native_menubar = false;
bool qt_mac_no_menubar_merge = false;
bool qt_mac_quit_menu_item_enabled = true;
int qt_mac_menus_open_count = 0;

static uint qt_mac_menu_static_cmd_id = 'QT00';
struct QMenuMergeItem
{
    inline QMenuMergeItem(MenuCommand c, QMacMenuAction *a) : command(c), action(a) { }
    MenuCommand command;
    QMacMenuAction *action;
};
typedef QList<QMenuMergeItem> QMenuMergeList;

const UInt32 kMenuCreatorQt = 'cute';
enum {
    kMenuPropertyQAction = 'QAcT',
    kMenuPropertyQWidget = 'QWId',
    kMenuPropertyCausedQWidget = 'QCAU',
    kMenuPropertyMergeMenu = 'QApP',
    kMenuPropertyMergeList = 'QAmL',
    kMenuPropertyWidgetActionWidget = 'QWid',
    kMenuPropertyWidgetMenu = 'QWMe',

    kHICommandAboutQt = 'AOQT',
    kHICommandCustomMerge = 'AQt0'
};

static struct {
    QPointer<QMenuBar> qmenubar;
    bool modal;
} qt_mac_current_menubar = { 0, false };

/*****************************************************************************
  Externals
 *****************************************************************************/
extern HIViewRef qt_mac_hiview_for(const QWidget *w); //qwidget_mac.cpp
extern HIViewRef qt_mac_hiview_for(WindowPtr w); //qwidget_mac.cpp
extern IconRef qt_mac_create_iconref(const QPixmap &px); //qpixmap_mac.cpp
extern QWidget * mac_keyboard_grabber; //qwidget_mac.cpp
extern bool qt_sendSpontaneousEvent(QObject*, QEvent*); //qapplication_xxx.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

/*****************************************************************************
  QMenu utility functions
 *****************************************************************************/
QString qt_mac_no_ampersands(QString str) {
    for(int w = -1; (w=str.indexOf(QLatin1Char('&'), w+1)) != -1;) {
        if (w < (int)str.length())
            str.remove(w, 1);
    }
    return str;
}

void qt_mac_get_accel(quint32 accel_key, quint32 *modif, quint32 *key) {
    if (modif) {
        *modif = 0;
        if ((accel_key & Qt::CTRL) != Qt::CTRL)
            *modif |= kMenuNoCommandModifier;
        if ((accel_key & Qt::META) == Qt::META)
            *modif |= kMenuControlModifier;
        if ((accel_key & Qt::ALT) == Qt::ALT)
            *modif |= kMenuOptionModifier;
        if ((accel_key & Qt::SHIFT) == Qt::SHIFT)
            *modif |= kMenuShiftModifier;
    }

    accel_key &= ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL);
    if (key) {
        *key = 0;
        if (accel_key == Qt::Key_Return)
            *key = kMenuReturnGlyph;
        else if (accel_key == Qt::Key_Enter)
            *key = kMenuEnterGlyph;
        else if (accel_key == Qt::Key_Tab)
            *key = kMenuTabRightGlyph;
        else if (accel_key == Qt::Key_Backspace)
            *key = kMenuDeleteLeftGlyph;
        else if (accel_key == Qt::Key_Delete)
            *key = kMenuDeleteRightGlyph;
        else if (accel_key == Qt::Key_Escape)
            *key = kMenuEscapeGlyph;
        else if (accel_key == Qt::Key_PageUp)
            *key = kMenuPageUpGlyph;
        else if (accel_key == Qt::Key_PageDown)
            *key = kMenuPageDownGlyph;
        else if (accel_key == Qt::Key_Up)
            *key = kMenuUpArrowGlyph;
        else if (accel_key == Qt::Key_Down)
            *key = kMenuDownArrowGlyph;
        else if (accel_key == Qt::Key_Left)
            *key = kMenuLeftArrowGlyph;
        else if (accel_key == Qt::Key_Right)
            *key = kMenuRightArrowGlyph;
        else if (accel_key == Qt::Key_CapsLock)
            *key = kMenuCapsLockGlyph;
        else if (accel_key >= Qt::Key_F1 && accel_key <= Qt::Key_F15)
            *key = (accel_key - Qt::Key_F1) + kMenuF1Glyph;
        else if (accel_key == Qt::Key_Home)
            *key = kMenuNorthwestArrowGlyph;
        else if (accel_key == Qt::Key_End)
            *key = kMenuSoutheastArrowGlyph;
    }
}

bool qt_mac_watchingAboutToShow(QMenu *menu)
{
    return menu && menu->receivers(SIGNAL(aboutToShow()));
}

static int qt_mac_CountMenuItems(MenuRef menu)
{
    if (menu) {
        int ret = 0;
        const int items = CountMenuItems(menu);
        for(int i = 0; i < items; i++) {
            MenuItemAttributes attr;
            if (GetMenuItemAttributes(menu, i+1, &attr) == noErr &&
               attr & kMenuItemAttrHidden)
                continue;
            ++ret;
        }
        return ret;
    }
    return 0;
}

bool qt_mac_menubar_is_open()
{
    return qt_mac_menus_open_count > 0;
}

//lookup a QMacMenuAction in a menu
static int qt_mac_menu_find_action(MenuRef menu, MenuCommand cmd)
{
    MenuItemIndex ret_idx;
    MenuRef ret_menu;
    if (GetIndMenuItemWithCommandID(menu, cmd, 1, &ret_menu, &ret_idx) == noErr) {
        if (ret_menu == menu)
            return (int)ret_idx;
    }
    return -1;
}
static int qt_mac_menu_find_action(MenuRef menu, QMacMenuAction *action)
{
    return qt_mac_menu_find_action(menu, action->command);
}

//enabling of commands
void qt_mac_command_set_enabled(MenuRef menu, UInt32 cmd, bool b)
{
    if (cmd == kHICommandQuit)
        qt_mac_quit_menu_item_enabled = b;

    if (b) {
        EnableMenuCommand(menu, cmd);
        if (MenuRef dock_menu = GetApplicationDockTileMenu())
            EnableMenuCommand(dock_menu, cmd);
    } else {
        DisableMenuCommand(menu, cmd);
        if (MenuRef dock_menu = GetApplicationDockTileMenu())
            DisableMenuCommand(dock_menu, cmd);
    }
}

//toggling of modal state
void qt_mac_set_modal_state(MenuRef menu, bool on)
{
    MenuRef merge = 0;
    GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu,
                        sizeof(merge), 0, &merge);

    for(int i = 0; i < CountMenuItems(menu); i++) {
        MenuRef submenu;
        GetMenuItemHierarchicalMenu(menu, i+1, &submenu);
        if(submenu != merge) {
            if (on)
                DisableMenuItem(submenu, 0);
            else
                EnableMenuItem(submenu, 0);
        }
    }

    UInt32 commands[] = { kHICommandQuit, kHICommandPreferences, kHICommandAbout, kHICommandAboutQt, 0 };
    for(int c = 0; commands[c]; c++) {
        bool enabled = !on;
        if (enabled) {
            QMacMenuAction *action = 0;
            GetMenuCommandProperty(menu, commands[c], kMenuCreatorQt, kMenuPropertyQAction,
                                   sizeof(action), 0, &action);
            if (!action && merge) {
                GetMenuCommandProperty(merge, commands[c], kMenuCreatorQt, kMenuPropertyQAction,
                                       sizeof(action), 0, &action);
                if (!action) {
                    QMenuMergeList *list = 0;
                    GetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                                        sizeof(list), 0, &list);
                    for(int i = 0; list && i < list->size(); ++i) {
                        QMenuMergeItem item = list->at(i);
                        if (item.command == commands[c] && item.action) {
                            action = item.action;
                            break;
                        }
                    }
                }
            }

            if (!action) {
                if (commands[c] != kHICommandQuit)
                    enabled = false;
            } else {
                enabled = action->action ? action->action->isEnabled() : 0;
            }
        }
        qt_mac_command_set_enabled(menu, commands[c], enabled);
    }
}

void qt_mac_clear_menubar()
{
    MenuRef clear_menu = 0;
    if (CreateNewMenu(0, 0, &clear_menu) == noErr) {
        SetRootMenu(clear_menu);
        ReleaseMenu(clear_menu);
    } else {
        qWarning("QMenu: Internal error at %s:%d", __FILE__, __LINE__);
    }
    ClearMenuBar();
    qt_mac_command_set_enabled(0, kHICommandPreferences, false);
    InvalMenuBar();
}

static MenuCommand qt_mac_menu_merge_action(MenuRef merge, QMacMenuAction *action)
{
    if (qt_mac_no_menubar_merge || action->action->menu() || action->action->isSeparator()
            || action->action->menuRole() == QAction::NoRole)
        return 0;

    QString t = qt_mac_no_ampersands(action->action->text().toLower());
    int st = t.lastIndexOf(QLatin1Char('\t'));
    if (st != -1)
        t.remove(st, t.length()-st);
    t.replace(QRegExp(QString::fromLatin1("\\.*$")), QLatin1String("")); //no ellipses
    //now the fun part
    MenuCommand ret = 0;
    switch (action->action->menuRole()) {
    case QAction::NoRole: {
        ret = 0;
        break; }
    case QAction::ApplicationSpecificRole: {
        QMenuMergeList *list = 0;
        if (GetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                    sizeof(list), 0, &list) == noErr && list) {
            MenuCommand lastCustom = kHICommandCustomMerge;
            for(int i = 0; i < list->size(); ++i) {
                QMenuMergeItem item = list->at(i);
                if (item.command == lastCustom)
                    ++lastCustom;
            }
            ret = lastCustom;
        } else {
            // The list hasn't been created, so, must be the first one.
            ret = kHICommandCustomMerge;
        }
        break;
    }
    case QAction::AboutRole:
        ret = kHICommandAbout;
        break;
    case QAction::AboutQtRole:
        ret = kHICommandAboutQt;
        break;
    case QAction::QuitRole:
        ret = kHICommandQuit;
        break;
    case QAction::PreferencesRole:
        ret = kHICommandPreferences;
        break;
    case QAction::TextHeuristicRole: {
        QString aboutString = QMenuBar::tr("About").toLower();
        if (t.startsWith(aboutString) || t.endsWith(aboutString)) {
            if (t.indexOf(QRegExp(QString::fromLatin1("qt$"), Qt::CaseInsensitive)) == -1)
                ret = kHICommandAbout;
            else
                ret = kHICommandAboutQt;
        } else if (t.startsWith(QMenuBar::tr("Config").toLower())
                   || t.startsWith(QMenuBar::tr("Preference").toLower())
                   || t.startsWith(QMenuBar::tr("Options").toLower())
                   || t.startsWith(QMenuBar::tr("Setting").toLower())
                   || t.startsWith(QMenuBar::tr("Setup").toLower())) {
            ret = kHICommandPreferences;
        } else if (t.startsWith(QMenuBar::tr("Quit").toLower())
                   || t.startsWith(QMenuBar::tr("Exit").toLower())) {
            ret = kHICommandQuit;
        }
    }
        break;
    }

    QMenuMergeList *list = 0;
    if (GetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                           sizeof(list), 0, &list) == noErr && list) {
        for(int i = 0; i < list->size(); ++i) {
            QMenuMergeItem item = list->at(i);
            if (item.command == ret && item.action)
                return 0;
        }
    }

    QAction *cmd_action = 0;
    if (GetMenuCommandProperty(merge, ret, kMenuCreatorQt, kMenuPropertyQAction,
                              sizeof(cmd_action), 0, &cmd_action) == noErr && cmd_action)
        return 0; //already taken
    return ret;
}

static bool qt_mac_auto_apple_menu(MenuCommand cmd)
{
    return (cmd == kHICommandPreferences || cmd == kHICommandQuit);
}

static QString qt_mac_menu_merge_text(QMacMenuAction *action)
{
    QString ret;
    if (action->action->menuRole() == QAction::ApplicationSpecificRole)
        ret = action->action->text();
    else if (action->command == kHICommandAbout)
        ret = QMenuBar::tr("About %1").arg(qAppName());
    else if (action->command == kHICommandAboutQt)
        ret = QMenuBar::tr("About Qt");
    else if (action->command == kHICommandPreferences)
        ret = QMenuBar::tr("Preferences");
    else if (action->command == kHICommandQuit)
        ret = QMenuBar::tr("Quit %1").arg(qAppName());
    return ret;
}

static QKeySequence qt_mac_menu_merge_accel(QMacMenuAction *action)
{
    QKeySequence ret;
    if (action->action->menuRole() == QAction::ApplicationSpecificRole)
        ret = action->action->shortcut();
    else if (action->command == kHICommandPreferences)
        ret = QKeySequence(Qt::CTRL+Qt::Key_Comma);
    else if (action->command == kHICommandQuit)
        ret = QKeySequence(Qt::CTRL+Qt::Key_Q);
    return ret;
}

void Q_GUI_EXPORT qt_mac_set_menubar_icons(bool b)
{ QApplication::instance()->setAttribute(Qt::AA_DontShowIconsInMenus, !b); }
void Q_GUI_EXPORT qt_mac_set_native_menubar(bool b) { qt_mac_no_native_menubar = !b; }
void Q_GUI_EXPORT qt_mac_set_menubar_merge(bool b) { qt_mac_no_menubar_merge = !b; }

bool qt_mac_activate_action(MenuRef menu, uint command, QAction::ActionEvent action_e, bool by_accel)
{
    //fire event
    QMacMenuAction *action = 0;
    if (GetMenuCommandProperty(menu, command, kMenuCreatorQt, kMenuPropertyQAction, sizeof(action), 0, &action) != noErr) {
        QMenuMergeList *list = 0;
        GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                            sizeof(list), 0, &list);
        if (!list && qt_mac_current_menubar.qmenubar) {
            MenuRef apple_menu = qt_mac_current_menubar.qmenubar->d_func()->mac_menubar->apple_menu;
            GetMenuItemProperty(apple_menu, 0, kMenuCreatorQt, kMenuPropertyMergeList, sizeof(list), 0, &list);
            if (list)
                menu = apple_menu;
        }
        if (list) {
            for(int i = 0; i < list->size(); ++i) {
                QMenuMergeItem item = list->at(i);
                if (item.command == command && item.action) {
                    action = item.action;
                    break;
                }
            }
        }
        if (!action)
            return false;
    }

    if (action_e == QAction::Trigger && by_accel && action->ignore_accel) //no, not a real accel (ie tab)
        return false;

    // Unhighlight the highlighted menu item before triggering the action to
    // prevent items from staying highlighted while a modal dialog is shown.
    // This also fixed the problem that parentless modal dialogs leave
    // the menu item highlighted (since the menu bar is cleared for these types of dialogs).
    if (action_e == QAction::Trigger)
        HiliteMenu(0);

    action->action->activate(action_e);

    //now walk up firing for each "caused" widget (like in the platform independent menu)
    QWidget *caused = 0;
    if (GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), 0, &caused) == noErr) {
        MenuRef caused_menu = 0;
        if (QMenu *qmenu2 = qobject_cast<QMenu*>(caused))
            caused_menu = qmenu2->macMenu();
        else if (QMenuBar *qmenubar2 = qobject_cast<QMenuBar*>(caused))
            caused_menu = qmenubar2->macMenu();
        else
            caused_menu = 0;
        while(caused_menu) {
            //fire
            QWidget *widget = 0;
            GetMenuItemProperty(caused_menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(widget), 0, &widget);
            if (QMenu *qmenu = qobject_cast<QMenu*>(widget)) {
                if (action_e == QAction::Trigger) {
                    emit qmenu->triggered(action->action);
                } else if (action_e == QAction::Hover) {
                    emit qmenu->hovered(action->action);
                }
            } else if (QMenuBar *qmenubar = qobject_cast<QMenuBar*>(widget)) {
                if (action_e == QAction::Trigger) {
                    emit qmenubar->triggered(action->action);
                } else if (action_e == QAction::Hover) {
                    emit qmenubar->hovered(action->action);
                }
                break; //nothing more..
            }

            //walk up
            if (GetMenuItemProperty(caused_menu, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget,
                                   sizeof(caused), 0, &caused) != noErr)
                break;
            if (QMenu *qmenu2 = qobject_cast<QMenu*>(caused))
                caused_menu = qmenu2->macMenu();
            else if (QMenuBar *qmenubar2 = qobject_cast<QMenuBar*>(caused))
                caused_menu = qmenubar2->macMenu();
            else
                caused_menu = 0;
        }
    }
    return true;
}

typedef QMultiHash<MenuRef, EventHandlerRef> EventHandlerHash;
Q_GLOBAL_STATIC(EventHandlerHash, menu_eventHandlers_hash)

static EventTypeSpec widget_in_menu_events[] = {
    { kEventClassMenu, kEventMenuMeasureItemWidth },
    { kEventClassMenu, kEventMenuMeasureItemHeight },
    { kEventClassMenu, kEventMenuDrawItem },
    { kEventClassMenu, kEventMenuCalculateSize }
};

static OSStatus qt_mac_widget_in_menu_eventHandler(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event);
    UInt32 eclass = GetEventClass(event);
    OSStatus result = eventNotHandledErr;
    switch (eclass) {
    case kEventClassMenu:
        switch (ekind) {
        default:
            break;
        case kEventMenuMeasureItemWidth: {
            MenuItemIndex item;
            GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex,
                              0, sizeof(item), 0, &item);
            MenuRef menu;
            GetEventParameter(event, kEventParamDirectObject, typeMenuRef, 0, sizeof(menu), 0, &menu);
            QWidget *widget;
            if (GetMenuItemProperty(menu, item, kMenuCreatorQt, kMenuPropertyWidgetActionWidget,
                                 sizeof(widget), 0, &widget) == noErr) {
                short width = short(widget->sizeHint().width());
                SetEventParameter(event, kEventParamMenuItemWidth, typeSInt16,
                                  sizeof(short), &width);
                result = noErr;
            }
            break; }
        case kEventMenuMeasureItemHeight: {
            MenuItemIndex item;
            GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex,
                              0, sizeof(item), 0, &item);
            MenuRef menu;
            GetEventParameter(event, kEventParamDirectObject, typeMenuRef, 0, sizeof(menu), 0, &menu);
            QWidget *widget;
            if (GetMenuItemProperty(menu, item, kMenuCreatorQt, kMenuPropertyWidgetActionWidget,
                                     sizeof(widget), 0, &widget) == noErr && widget) {
                short height = short(widget->sizeHint().height());
                SetEventParameter(event, kEventParamMenuItemHeight, typeSInt16,
                                  sizeof(short), &height);
                result = noErr;
            }
            break; }
        case kEventMenuDrawItem:
            result = noErr;
            break;
        case kEventMenuCalculateSize: {
            result = CallNextEventHandler(er, event);
            if (result == noErr) {
                MenuRef menu;
                GetEventParameter(event, kEventParamDirectObject, typeMenuRef, 0, sizeof(menu), 0, &menu);
                HIViewRef content;
                HIMenuGetContentView(menu, kThemeMenuTypePullDown, &content);
                UInt16 count = CountMenuItems(menu);
                for (MenuItemIndex i = 1; i <= count; ++i) {
                    QWidget *widget;
                    if (GetMenuItemProperty(menu, i, kMenuCreatorQt, kMenuPropertyWidgetActionWidget,
                            sizeof(widget), 0, &widget) == noErr && widget) {
                        RgnHandle itemRgn = qt_mac_get_rgn();
                        GetControlRegion(content, i, itemRgn);

                        Rect bounds;
                        GetRegionBounds( itemRgn, &bounds );
                        qt_mac_dispose_rgn(itemRgn);
                        widget->setGeometry(bounds.left, bounds.top,
                                            bounds.right - bounds.left, bounds.bottom - bounds.top);
                    }
                }
            }
            break; }
        }
    }
    return result;
}

//handling of events for menurefs created by Qt..
static EventTypeSpec menu_events[] = {
    { kEventClassCommand, kEventCommandProcess },
    { kEventClassMenu, kEventMenuTargetItem },
    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuClosed }
};

// Special case for kEventMenuMatchKey, see qt_mac_create_menu below.
static EventTypeSpec menu_menu_events[] = {
    { kEventClassMenu, kEventMenuMatchKey }
};

OSStatus qt_mac_menu_event(EventHandlerCallRef er, EventRef event, void *)
{
    QScopedLoopLevelCounter loopLevelCounter(QApplicationPrivate::instance()->threadData);

    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassCommand:
        if (ekind == kEventCommandProcess) {
            UInt32 context;
            GetEventParameter(event, kEventParamMenuContext, typeUInt32,
                              0, sizeof(context), 0, &context);
            HICommand cmd;
            GetEventParameter(event, kEventParamDirectObject, typeHICommand,
                              0, sizeof(cmd), 0, &cmd);
            if (!mac_keyboard_grabber && (context & kMenuContextKeyMatching)) {
                QMacMenuAction *action = 0;
                if (GetMenuCommandProperty(cmd.menu.menuRef, cmd.commandID, kMenuCreatorQt,
                                          kMenuPropertyQAction, sizeof(action), 0, &action) == noErr) {
                    QWidget *widget = 0;
                    if (qApp->activePopupWidget())
                        widget = (qApp->activePopupWidget()->focusWidget() ?
                                  qApp->activePopupWidget()->focusWidget() : qApp->activePopupWidget());
                    else if (QApplicationPrivate::focus_widget)
                        widget = QApplicationPrivate::focus_widget;
                    if (widget) {
                        int key = action->action->shortcut();
                        QKeyEvent accel_ev(QEvent::ShortcutOverride, (key & (~Qt::KeyboardModifierMask)),
                                           Qt::KeyboardModifiers(key & Qt::KeyboardModifierMask));
                        accel_ev.ignore();
                        qt_sendSpontaneousEvent(widget, &accel_ev);
                        if (accel_ev.isAccepted()) {
                            handled_event = false;
                            break;
                        }
                    }
                }
            }
            handled_event = qt_mac_activate_action(cmd.menu.menuRef, cmd.commandID,
                                                   QAction::Trigger, context & kMenuContextKeyMatching);
        }
        break;
    case kEventClassMenu: {
        MenuRef menu;
        GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(menu), NULL, &menu);
        if (ekind == kEventMenuMatchKey) {
            // Don't activate any actions if we are showing a native modal dialog,
            // the key events should go to the dialog in this case.
            if (QApplicationPrivate::native_modal_dialog_active)
                return menuItemNotFoundErr;

             handled_event = false;
        } else if (ekind == kEventMenuTargetItem) {
            MenuCommand command;
            GetEventParameter(event, kEventParamMenuCommand, typeMenuCommand,
                              0, sizeof(command), 0, &command);
            handled_event = qt_mac_activate_action(menu, command, QAction::Hover, false);
        } else if (ekind == kEventMenuOpening || ekind == kEventMenuClosed) {
            qt_mac_menus_open_count += (ekind == kEventMenuOpening) ? 1 : -1;
            MenuRef mr;
            GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
                              0, sizeof(mr), 0, &mr);

            QWidget *widget = 0;
            if (GetMenuItemProperty(mr, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(widget), 0, &widget) == noErr) {
                if (QMenu *qmenu = qobject_cast<QMenu*>(widget)) {
                    handled_event = true;
                    if (ekind == kEventMenuOpening) {
                        emit qmenu->aboutToShow();

                        int merged = 0;
                        const QMenuPrivate::QMacMenuPrivate *mac_menu = qmenu->d_func()->mac_menu;
                        for(int i = 0; i < mac_menu->actionItems.size(); ++i) {
                            QMacMenuAction *action = mac_menu->actionItems.at(i);
                            if (action->action->isSeparator()) {
                                bool hide = false;
                                if(!action->action->isVisible()) {
                                    hide = true;
                                } else if (merged && merged == i) {
                                    hide = true;
                                } else {
                                    for(int l = i+1; l < mac_menu->actionItems.size(); ++l) {
                                        QMacMenuAction *action = mac_menu->actionItems.at(l);
                                        if (action->merged) {
                                            hide = true;
                                        } else if (action->action->isSeparator()) {
                                            if (hide)
                                                break;
                                        } else if (!action->merged) {
                                            hide = false;
                                            break;
                                        }
                                    }
                                }

                                const int index = qt_mac_menu_find_action(mr, action);
                                if (hide) {
                                    ++merged;
                                    ChangeMenuItemAttributes(mr, index, kMenuItemAttrHidden, 0);
                                } else {
                                    ChangeMenuItemAttributes(mr, index, 0, kMenuItemAttrHidden);
                                }
                            } else if (action->merged) {
                                ++merged;
                            }
                        }
                    } else {
                        emit qmenu->aboutToHide();
                    }
                }
            }
        } else {
            handled_event = false;
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    if (!handled_event) //let the event go through
        return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}
static EventHandlerRef mac_menu_event_handler = 0;
static EventHandlerUPP mac_menu_eventUPP = 0;
static void qt_mac_cleanup_menu_event()
{
    if (mac_menu_event_handler) {
        RemoveEventHandler(mac_menu_event_handler);
        mac_menu_event_handler = 0;
    }
    if (mac_menu_eventUPP) {
        DisposeEventHandlerUPP(mac_menu_eventUPP);
        mac_menu_eventUPP = 0;
    }
}
static inline void qt_mac_create_menu_event_handler()
{
    if (!mac_menu_event_handler) {
        mac_menu_eventUPP = NewEventHandlerUPP(qt_mac_menu_event);
        InstallEventHandler(GetApplicationEventTarget(), mac_menu_eventUPP,
                            GetEventTypeCount(menu_events), menu_events, 0,
                            &mac_menu_event_handler);
        qAddPostRoutine(qt_mac_cleanup_menu_event);
    }
}

//creation of the MenuRef
static MenuRef qt_mac_create_menu(QWidget *w)
{
    MenuRef ret = 0;
    if (CreateNewMenu(0, 0, &ret) == noErr) {
        qt_mac_create_menu_event_handler();
        SetMenuItemProperty(ret, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(w), &w);

        // kEventMenuMatchKey is only sent to the menu itself and not to
        // the application, install a separate handler for that event.
        EventHandlerRef eventHandlerRef;
        InstallMenuEventHandler(ret, qt_mac_menu_event,
                                GetEventTypeCount(menu_menu_events),
                                menu_menu_events, 0, &eventHandlerRef);
        menu_eventHandlers_hash()->insert(ret, eventHandlerRef);
    } else {
        qWarning("QMenu: Internal error");
    }
    return ret;
}


/*****************************************************************************
  QMenu bindings
 *****************************************************************************/
QMenuPrivate::QMacMenuPrivate::QMacMenuPrivate() : menu(0)
{
}

QMenuPrivate::QMacMenuPrivate::~QMacMenuPrivate()
{
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it) {
        QMacMenuAction *action = (*it);
        RemoveMenuCommandProperty(action->menu, action->command, kMenuCreatorQt, kMenuPropertyQAction);
        if (action->merged) {
            QMenuMergeList *list = 0;
            GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                                sizeof(list), 0, &list);
            for(int i = 0; list && i < list->size(); ) {
                QMenuMergeItem item = list->at(i);
                if (item.action == action)
                    list->removeAt(i);
                else
                    ++i;
            }
        }
        delete action;
    }
    if (menu) {
        EventHandlerHash::iterator it = menu_eventHandlers_hash()->find(menu);
        while (it != menu_eventHandlers_hash()->end() && it.key() == menu) {
            RemoveEventHandler(it.value());
            ++it;
        }
        menu_eventHandlers_hash()->remove(menu);
        ReleaseMenu(menu);
    }
}

void
QMenuPrivate::QMacMenuPrivate::addAction(QAction *a, QMacMenuAction *before, QMenuPrivate *qmenu)
{
    QMacMenuAction *action = new QMacMenuAction;
    action->action = a;
    action->command = qt_mac_menu_static_cmd_id++;
    action->ignore_accel = 0;
    action->merged = 0;
    action->menu = 0;
    addAction(action, before, qmenu);
}

void
QMenuPrivate::QMacMenuPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before, QMenuPrivate *qmenu)
{
    if (!action)
        return;
    int before_index = actionItems.indexOf(before);
    if (before_index < 0) {
        before = 0;
        before_index = actionItems.size();
    }
    actionItems.insert(before_index, action);

    int index = qt_mac_menu_find_action(menu, action);
    action->menu = menu;
    /* I don't know if this is a bug or a feature, but when the action is considered a mergable action it
       will stay that way, until removed.. */
    if (!qt_mac_no_menubar_merge) {
        MenuRef merge = 0;
        GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu,
                            sizeof(merge), 0, &merge);
        if (merge) {
            if (MenuCommand cmd = qt_mac_menu_merge_action(merge, action)) {
                action->merged = 1;
                action->menu = merge;
                action->command = cmd;
                if (qt_mac_auto_apple_menu(cmd))
                    index = 0; //no need

                QMenuMergeList *list = 0;
                if (GetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                                       sizeof(list), 0, &list) != noErr || !list) {
                    list = new QMenuMergeList;
                    SetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                                        sizeof(list), &list);
                }
                list->append(QMenuMergeItem(cmd, action));
            }
        }
    }

    if (index == -1) {
        index = before_index;
        MenuItemAttributes attr = kMenuItemAttrAutoRepeat;
        QWidget *widget = qmenu ? qmenu->widgetItems.value(action->action) : 0;
        if (widget) {
            ChangeMenuAttributes(action->menu, kMenuAttrDoNotCacheImage, 0);
            attr = kMenuItemAttrCustomDraw;
        }

        if (before) {
            InsertMenuItemTextWithCFString(action->menu, 0, qMax(before_index, 0), attr, action->command);
        } else {
            // Append the menu item to the menu. If it is a kHICommandAbout or a kHICommandAboutQt append
            // a separator also (to get a separator above "Preferences"), but make sure that we don't
            // add separators between two "about" items.

            // Build a set of all commands that could possibly be before the separator.
            QSet<MenuCommand> mergedItems;
            mergedItems.insert(kHICommandAbout);
            mergedItems.insert(kHICommandAboutQt);
            mergedItems.insert(kHICommandCustomMerge);

            QMenuMergeList *list = 0;
            if (GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                        sizeof(list), 0, &list) == noErr && list) {
                for (int i = 0; i < list->size(); ++i) {
                    MenuCommand command = list->at(i).command;
                    if (command > kHICommandCustomMerge) {
                        mergedItems.insert(command);
                    }
                }
            }

            const int itemCount = CountMenuItems(action->menu);
            MenuItemAttributes testattr;
            GetMenuItemAttributes(action->menu, itemCount , &testattr);
            if (mergedItems.contains(action->command)
                 && (testattr & kMenuItemAttrSeparator)) {
                    InsertMenuItemTextWithCFString(action->menu, 0, qMax(itemCount - 1, 0), attr, action->command);
                    index = itemCount;
                 } else {
                    MenuItemIndex tmpIndex;
                    AppendMenuItemTextWithCFString(action->menu, 0, attr, action->command, &tmpIndex);
                    index = tmpIndex;
                    if (mergedItems.contains(action->command))
                        AppendMenuItemTextWithCFString(action->menu, 0, kMenuItemAttrSeparator, 0, &tmpIndex);
                 }
        } // ! before

        if (widget) {
            SetMenuItemProperty(action->menu, index, kMenuCreatorQt, kMenuPropertyWidgetActionWidget,
                                sizeof(QWidget *), &widget);
            HIViewRef content;
            HIMenuGetContentView(action->menu, kThemeMenuTypePullDown, &content);

            EventHandlerRef eventHandlerRef;
            InstallMenuEventHandler(action->menu, qt_mac_widget_in_menu_eventHandler,
                                    GetEventTypeCount(widget_in_menu_events),
                                    widget_in_menu_events, 0, &eventHandlerRef);
            menu_eventHandlers_hash()->insert(action->menu, eventHandlerRef);

            QWidget *menuWidget = 0;
            GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyWidgetMenu,
                                sizeof(menuWidget), 0, &menuWidget);
            if(!menuWidget) {
                menuWidget = new QHIViewWidget(content, false, 0, Qt::ToolTip);
                SetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyWidgetMenu,
                                    sizeof(menuWidget), &menuWidget);
                menuWidget->show();
            }
            widget->setParent(menuWidget);
            widget->show();
        }
    } else {
        qt_mac_command_set_enabled(action->menu, action->command, !QApplicationPrivate::modalState());
    }
    SetMenuCommandProperty(action->menu, action->command, kMenuCreatorQt, kMenuPropertyQAction,
                           sizeof(action), &action);
    syncAction(action);
}

void
QMenuPrivate::QMacMenuPrivate::syncAction(QMacMenuAction *action)
{
    if (!action)
        return;
    const int index = qt_mac_menu_find_action(action->menu, action);
    if (index == -1)
        return;

    if (!action->action->isVisible()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrHidden, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrHidden);

    if (action->action->isSeparator()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrSeparator, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrSeparator);

    //find text (and accel)
    action->ignore_accel = 0;
    QString text = action->action->text();
    QKeySequence accel = action->action->shortcut();
    {
        int st = text.lastIndexOf(QLatin1Char('\t'));
        if (st != -1) {
            action->ignore_accel = 1;
            accel = QKeySequence(text.right(text.length()-(st+1)));
            text.remove(st, text.length()-st);
        }
    }
    {
        QString cmd_text = qt_mac_menu_merge_text(action);
        if (!cmd_text.isNull()) {
            text = cmd_text;
            accel = qt_mac_menu_merge_accel(action);
        }
    }
    if (accel.count() > 1)
        text += QLatin1String(" (****)"); //just to denote a multi stroke shortcut

    MenuItemDataRec data;
    memset(&data, '\0', sizeof(data));

    //text
    data.whichData |= kMenuItemDataCFString;
    QCFString cfstr(qt_mac_no_ampersands(text));
    data.cfText = cfstr;

    //enabled
    data.whichData |= kMenuItemDataEnabled;
    data.enabled = action->action->isEnabled();

    //icon
    data.whichData |= kMenuItemDataIconHandle;
    if (!action->action->icon().isNull()
            && action->action->isIconVisibleInMenu()) {
        data.iconType = kMenuIconRefType;
        data.iconHandle = (Handle)qt_mac_create_iconref(action->action->icon().pixmap(22, QIcon::Normal));
    } else {
        data.iconType = kMenuNoIcon;
    }

    if (action->action->font().resolve()) { //font
        if (action->action->font().bold())
            data.style |= bold;
        if (action->action->font().underline())
            data.style |= underline;
        if (action->action->font().italic())
            data.style |= italic;
        if (data.style)
            data.whichData |= kMenuItemDataStyle;
        data.whichData |= kMenuItemDataFontID;
        data.fontID = action->action->font().macFontID();
    }

    if (action->action->menu()) { //submenu
        data.whichData |= kMenuItemDataSubmenuHandle;
        data.submenuHandle = action->action->menu()->macMenu();
        QWidget *caused = 0;
        GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(caused), 0, &caused);
        SetMenuItemProperty(data.submenuHandle, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), &caused);
    } else { //respect some other items
        //shortcuts (say we are setting them all so that we can also clear them).
        data.whichData |= kMenuItemDataCmdKey;
        data.whichData |= kMenuItemDataCmdKeyModifiers;
        data.whichData |= kMenuItemDataCmdKeyGlyph;
        if (!accel.isEmpty()) {
            qt_mac_get_accel(accel[0], (quint32*)&data.cmdKeyModifiers, (quint32*)&data.cmdKeyGlyph);
            if (data.cmdKeyGlyph == 0)
                data.cmdKey = (UniChar)accel[0];
        }
    }

    //mark glyph
    data.whichData |= kMenuItemDataMark;
    if (action->action->isChecked()) {
#if 0
        if (action->action->actionGroup() &&
           action->action->actionGroup()->isExclusive())
            data.mark = diamondMark;
        else
#endif
            data.mark = checkMark;
    } else {
        data.mark = noMark;
    }

    //actually set it
    SetMenuItemData(action->menu, action->command, true, &data);

    // Free up memory
    if (data.iconHandle)
        ReleaseIconRef(IconRef(data.iconHandle));
}

void
QMenuPrivate::QMacMenuPrivate::removeAction(QMacMenuAction *action)
{
    if (!action)
        return;
    if (action->command == kHICommandQuit || action->command == kHICommandPreferences)
        qt_mac_command_set_enabled(action->menu, action->command, false);
    else
        DeleteMenuItem(action->menu, qt_mac_menu_find_action(action->menu, action));
    actionItems.removeAll(action);
}

MenuRef
QMenuPrivate::macMenu(MenuRef merge)
{
    Q_Q(QMenu);
    if (mac_menu && mac_menu->menu)
        return mac_menu->menu;
    if (!mac_menu)
        mac_menu = new QMacMenuPrivate;
    mac_menu->menu = qt_mac_create_menu(q);
    if (merge)
        SetMenuItemProperty(mac_menu->menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu, sizeof(merge), &merge);

    QList<QAction*> items = q->actions();
    for(int i = 0; i < items.count(); i++)
        mac_menu->addAction(items[i], 0, this);
    return mac_menu->menu;
}

/*!
  \internal
*/
void QMenuPrivate::setMacMenuEnabled(bool enable)
{
    if (!macMenu(0))
        return;

    if (enable) {
        // Only enable those items which contains an enabled QAction.
        // i == 0 -> the menu itself, hence i + 1 for items.
        for (int i = 0; i < mac_menu->actionItems.count(); ++i) {
            QMacMenuAction *menuItem = mac_menu->actionItems.at(i);
            if (menuItem && menuItem->action && menuItem->action->isEnabled())
                EnableMenuItem(mac_menu->menu, i + 1);
        }
    } else {
        DisableAllMenuItems(mac_menu->menu);
    }
}

/*!
    \internal

    This function will return the MenuRef used to create the native menu bar
    bindings. This MenuRef may be referenced in the Menu Manager, or this
    can be used to create native dock menus.

    \warning This function is not portable.

    \sa QMenuBar::macMenu()
*/
MenuRef QMenu::macMenu(MenuRef merge) { return d_func()->macMenu(merge); }

/*****************************************************************************
  QMenuBar bindings
 *****************************************************************************/
typedef QHash<QWidget *, QMenuBar *> MenuBarHash;
Q_GLOBAL_STATIC(MenuBarHash, menubars)
static QMenuBar *fallback = 0;

QMenuBarPrivate::QMacMenuBarPrivate::QMacMenuBarPrivate() : menu(0), apple_menu(0)
{
}

QMenuBarPrivate::QMacMenuBarPrivate::~QMacMenuBarPrivate()
{
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
        delete (*it);
    if (apple_menu) {
        QMenuMergeList *list = 0;
        GetMenuItemProperty(apple_menu, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                            sizeof(list), 0, &list);
        if (list) {
            RemoveMenuItemProperty(apple_menu, 0, kMenuCreatorQt, kMenuPropertyMergeList);
            delete list;
        }
        ReleaseMenu(apple_menu);
    }
    if (menu)
        ReleaseMenu(menu);
}

void
QMenuBarPrivate::QMacMenuBarPrivate::addAction(QAction *a, QMacMenuAction *before)
{
    if (a->isSeparator() || !menu)
        return;
    QMacMenuAction *action = new QMacMenuAction;
    action->action = a;
    action->ignore_accel = 1;
    action->merged = 0;
    action->menu = 0;
    action->command = qt_mac_menu_static_cmd_id++;
    addAction(action, before);
}

void
QMenuBarPrivate::QMacMenuBarPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
    if (!action || !menu)
        return;

    int before_index = actionItems.indexOf(before);
    if (before_index < 0) {
        before = 0;
        before_index = actionItems.size();
    }
    actionItems.insert(before_index, action);

    MenuItemIndex index = actionItems.size()-1;

    action->menu = menu;
    if (before) {
        InsertMenuItemTextWithCFString(action->menu, 0, qMax(1, before_index+1), 0, action->command);
        index = before_index;
    } else {
        AppendMenuItemTextWithCFString(action->menu, 0, 0, action->command, &index);
    }
    SetMenuItemProperty(action->menu, index, kMenuCreatorQt, kMenuPropertyQAction, sizeof(action),
                        &action);
    syncAction(action);
}

void
QMenuBarPrivate::QMacMenuBarPrivate::syncAction(QMacMenuAction *action)
{
    if (!action || !menu)
        return;
    const int index = qt_mac_menu_find_action(action->menu, action);

    MenuRef submenu = 0;
    bool release_submenu = false;
    if (action->action->menu()) {
        if ((submenu = action->action->menu()->macMenu(apple_menu))) {
            QWidget *caused = 0;
            GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(caused), 0, &caused);
            SetMenuItemProperty(submenu, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), &caused);
        }
    } else if (!submenu) {
        release_submenu = true;
        CreateNewMenu(0, 0, &submenu);
    }
    if (submenu) {
        SetMenuItemHierarchicalMenu(action->menu, index, submenu);
        SetMenuTitleWithCFString(submenu, QCFString(qt_mac_no_ampersands(action->action->text())));
        bool visible = action->action->isVisible();
        if (visible && action->action->text() == QString(QChar(0x14)))
            visible = false;
        if (visible && action->action->menu() && !action->action->menu()->actions().isEmpty() &&
           !qt_mac_CountMenuItems(action->action->menu()->macMenu(apple_menu)) &&
           !qt_mac_watchingAboutToShow(action->action->menu()))
            visible = false;
        if (visible)
            ChangeMenuAttributes(submenu, 0, kMenuAttrHidden);
        else
            ChangeMenuAttributes(submenu, kMenuAttrHidden, 0);

        if (release_submenu) //no pointers to it
            ReleaseMenu(submenu);
    } else {
        qWarning("QMenu: No MenuRef created for popup menu");
    }
}

void
QMenuBarPrivate::QMacMenuBarPrivate::removeAction(QMacMenuAction *action)
{
    if (!action || !menu)
        return;
    DeleteMenuItem(action->menu, qt_mac_menu_find_action(action->menu, action));
    actionItems.removeAll(action);
}

void
QMenuBarPrivate::macCreateMenuBar(QWidget *parent)
{
    Q_Q(QMenuBar);
    static int checkEnv = -1;
    if (qt_mac_no_native_menubar == false && checkEnv < 0) {
        checkEnv = !qgetenv("QT_MAC_NO_NATIVE_MENUBAR").isEmpty();
        qt_mac_no_native_menubar = checkEnv;
    }
    if (!qt_mac_no_native_menubar) {
        extern void qt_event_request_menubarupdate(); //qapplication_mac.cpp
        qt_event_request_menubarupdate();
        if (!parent && !fallback) {
            fallback = q;
            mac_menubar = new QMacMenuBarPrivate;
        } else if (parent && parent->isWindow()) {
            menubars()->insert(q->window(), q);
            mac_menubar = new QMacMenuBarPrivate;
        }
    }
}

void QMenuBarPrivate::macDestroyMenuBar()
{
    Q_Q(QMenuBar);
    if (fallback == q)
        fallback = 0;
    delete mac_menubar;
    QWidget *tlw = q->window();
    menubars()->remove(tlw);
    mac_menubar = 0;

    if (qt_mac_current_menubar.qmenubar == q) {
        extern void qt_event_request_menubarupdate(); //qapplication_mac.cpp
        qt_event_request_menubarupdate();
    }
}

MenuRef QMenuBarPrivate::macMenu()
{
    Q_Q(QMenuBar);
    if (!mac_menubar) {
        return 0;
    } else if (!mac_menubar->menu) {
        ProcessSerialNumber mine, front;
        if (GetCurrentProcess(&mine) == noErr && GetFrontProcess(&front) == noErr) {
            mac_menubar->menu = qt_mac_create_menu(q);
            if (!qt_mac_no_menubar_merge && !mac_menubar->apple_menu) { //handle the apple menu
                MenuItemIndex index;
                mac_menubar->apple_menu = qt_mac_create_menu(q);
                AppendMenuItemTextWithCFString(mac_menubar->menu, 0, 0, 0, &index);

                SetMenuTitleWithCFString(mac_menubar->apple_menu, QCFString(QString(QChar(0x14))));
                SetMenuItemHierarchicalMenu(mac_menubar->menu, index, mac_menubar->apple_menu);
                SetMenuItemProperty(mac_menubar->apple_menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(q), &q);
            }
            if (mac_menubar->apple_menu)
                SetMenuItemProperty(mac_menubar->menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu,
                                    sizeof(mac_menubar->apple_menu), &mac_menubar->apple_menu);

            QList<QAction*> items = q->actions();
            for(int i = 0; i < items.count(); i++)
                mac_menubar->addAction(items[i]);
        }
    }
    return mac_menubar->menu;
}

/*!
    \internal

    This function will return the MenuRef used to create the native menu bar
    bindings. This MenuRef is then set as the root menu for the Menu
    Manager.

    \warning This function is not portable.

    \sa QMenu::macMenu()
*/
MenuRef QMenuBar::macMenu() {  return d_func()->macMenu(); }

/* !
    \internal
    Ancestor function that crosses windows (QWidget::isAncestorOf
    only considers widgets within the same window).
*/
static bool qt_mac_is_ancestor(QWidget* possibleAncestor, QWidget *child)
{
    QWidget * current = child->parentWidget();
    while (current != 0) {
        if (current == possibleAncestor)
            return true;
        current = current->parentWidget();
    }
    return false;
}

/* !
    \internal
    Returns true if the entries of menuBar should be disabled,
    based on the modality type of modalWidget.
*/
static bool qt_mac_should_disable_menu(QMenuBar *menuBar, QWidget *modalWidget)
{
    if (modalWidget == 0 || menuBar == 0)
        return false;
    const Qt::WindowModality modality = modalWidget->windowModality();
    if (modality == Qt::ApplicationModal) {
        return true;
    } else if (modality == Qt::WindowModal) {
        QWidget * parent = menuBar->parentWidget();

        // Special case for the global menu bar: It's not associated
        // with a window so don't disable it.
        if (parent == 0)
            return false;

        // Disable menu entries in menu bars that belong to ancestors of
        // the modal widget, leave entries in unrelated menu bars enabled.
        return qt_mac_is_ancestor(parent, modalWidget);
    }
    return false; // modality == NonModal
}

/*!
  \internal

  This function will update the current menu bar and set it as the
  active menu bar in the Menu Manager.

  \warning This function is not portable.

  \sa QMenu::macMenu(), QMenuBar::macMenu()
*/
bool QMenuBar::macUpdateMenuBar()
{
    if (qt_mac_no_native_menubar) //nothing to be done..
        return true;

    QMenuBar *mb = 0;
    //find a menu bar
    QWidget *w = qApp->activeWindow();
    if (!w) {
        WindowClass c;
        for(WindowPtr wp = FrontWindow(); wp; wp = GetNextWindow(wp)) {
            if (GetWindowClass(wp, &c))
                break;
            if (c == kOverlayWindowClass)
                continue;
            w = QWidget::find((WId)wp);
            break;
        }
    }
    if (!w) {
        QWidgetList tlws = QApplication::topLevelWidgets();
        for(int i = 0; i < tlws.size(); ++i) {
            QWidget *tlw = tlws.at(i);
            if ((tlw->isVisible() && tlw->windowType() != Qt::Tool &&
                tlw->windowType() != Qt::Popup)) {
                w = tlw;
                break;
            }
        }
    }
    if (w) {
        mb = menubars()->value(w);
#ifndef QT_NO_MAINWINDOW
        QDockWidget *dw = qobject_cast<QDockWidget *>(w);
        if (!mb && dw) {
            QMainWindow *mw = qobject_cast<QMainWindow *>(dw->parentWidget());
            if (mw && (mb = menubars()->value(mw)))
                w = mw;
        }
#endif
        while(w && !mb)
            mb = menubars()->value((w = w->parentWidget()));
    }
    if (!mb)
        mb = fallback;
    //now set it
    bool ret = false;
    if (mb) {
        if (MenuRef menu = mb->macMenu()) {
            SetRootMenu(menu);
            QWidget *modalWidget = qApp->activeModalWidget();
            if (mb != menubars()->value(modalWidget)) {
                qt_mac_set_modal_state(menu, qt_mac_should_disable_menu(mb, modalWidget));
            }
        }
        qt_mac_current_menubar.qmenubar = mb;
        qt_mac_current_menubar.modal = QApplicationPrivate::modalState();
        ret = true;
    } else if (qt_mac_current_menubar.qmenubar) {
        const bool modal = QApplicationPrivate::modalState();
        if (modal != qt_mac_current_menubar.modal) {
            ret = true;
            if (MenuRef menu = qt_mac_current_menubar.qmenubar->macMenu()) {
                SetRootMenu(menu);
                QWidget *modalWidget = qApp->activeModalWidget();
                if (qt_mac_current_menubar.qmenubar != menubars()->value(modalWidget)) {
                    qt_mac_set_modal_state(menu, qt_mac_should_disable_menu(mb, modalWidget));
                }
            }
            qt_mac_current_menubar.modal = modal;
        }
    }
    if(!ret)
        qt_mac_clear_menubar();
    return ret;
}

bool QMenuPrivate::QMacMenuPrivate::merged(const QAction *action) const
{
    MenuRef merge = 0;
    GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu,
            sizeof(merge), 0, &merge);
    if (merge) {
        QMenuMergeList *list = 0;
        if (GetMenuItemProperty(merge, 0, kMenuCreatorQt, kMenuPropertyMergeList,
                    sizeof(list), 0, &list) == noErr && list) {
            for(int i = 0; i < list->size(); ++i) {
                QMenuMergeItem item = list->at(i);
                if (item.action->action == action)
                    return true;
            }
        }
    }
    return false;
}

QT_END_NAMESPACE
