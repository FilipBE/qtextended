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

#ifndef QMENUBAR_P_H
#define QMENUBAR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QMAC_Q3MENUBAR_CPP_FILE
#include "QtGui/qstyleoption.h"
#include <private/qmenu_p.h> // Mac needs what in this file!

#ifdef Q_OS_WINCE
#include "qguifunctions_wince.h"
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MENUBAR
class QMenuBarExtension;
class QMenuBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMenuBar)
public:
    QMenuBarPrivate() : itemsDirty(0), itemsWidth(0), itemsStart(-1), currentAction(0), mouseDown(0),
                         closePopupMode(0), defaultPopDown(1), popupState(0), keyboardState(0), altPressed(0)
#ifdef Q_WS_MAC
                         , mac_menubar(0)
#endif

#ifdef Q_OS_WINCE
                         , wce_menubar(0), wceClassicMenu(false)
#endif
    { }
    ~QMenuBarPrivate()
        {
#ifdef Q_WS_MAC
            delete mac_menubar;
#endif
#ifdef Q_OS_WINCE
            delete wce_menubar;
#endif
        }

    void init();
    QStyleOptionMenuItem getStyleOption(const QAction *action) const;

    //item calculations
    uint itemsDirty : 1;
    int itemsWidth, itemsStart;

    QVector<int> shortcutIndexMap;
    mutable QMap<QAction*, QRect> actionRects;
    mutable QList<QAction*> actionList;
    void calcActionRects(int max_width, int start, QMap<QAction*, QRect> &actionRects, QList<QAction*> &actionList) const;
    QRect actionRect(QAction *) const;
    void updateGeometries();

    //selection
    QPointer<QAction>currentAction;
    uint mouseDown : 1, closePopupMode : 1, defaultPopDown;
    QAction *actionAt(QPoint p) const;
    void setCurrentAction(QAction *, bool =false, bool =false);
    void popupAction(QAction *, bool);

    //active popup state
    uint popupState : 1;
    QPointer<QMenu> activeMenu;

    //keyboard mode for keyboard navigation
    void setKeyboardMode(bool);
    uint keyboardState : 1, altPressed : 1;
    QPointer<QWidget> keyboardFocusWidget;

    //firing of events
    void activateAction(QAction *, QAction::ActionEvent);

    void _q_actionTriggered();
    void _q_actionHovered();
    void _q_internalShortcutActivated(int);
    void _q_updateLayout();

#ifdef Q_OS_WINCE
    void _q_updateDefaultAction();
#endif

    //extra widgets in the menubar
    QPointer<QWidget> leftWidget, rightWidget;
    QMenuBarExtension *extension;
    bool isVisible(QAction *action);

    //menu fading/scrolling effects
    bool doChildEffects;

    QRect menuRect(bool) const;

    // reparenting
    void handleReparent();
    QWidget *oldParent;
    QWidget *oldWindow;

    QList<QAction*> hiddenActions;
    //default action
    QPointer<QAction> defaultAction;

    QBasicTimer autoReleaseTimer;
#ifdef QT3_SUPPORT
    bool doAutoResize;
#endif
#ifdef Q_WS_MAC
    //mac menubar binding
    struct QMacMenuBarPrivate {
        QList<QMacMenuAction*> actionItems;
        MenuRef menu, apple_menu;
        QMacMenuBarPrivate();
        ~QMacMenuBarPrivate();

        void addAction(QAction *, QMacMenuAction* =0);
        void addAction(QMacMenuAction *, QMacMenuAction* =0);
        void syncAction(QMacMenuAction *);
        inline void syncAction(QAction *a) { syncAction(findAction(a)); }
        void removeAction(QMacMenuAction *);
        inline void removeAction(QAction *a) { removeAction(findAction(a)); }
        inline QMacMenuAction *findAction(QAction *a) {
            for(int i = 0; i < actionItems.size(); i++) {
                QMacMenuAction *act = actionItems[i];
                if(a == act->action)
                    return act;
            }
            return 0;
        }
    } *mac_menubar;
    void macCreateMenuBar(QWidget *);
    void macDestroyMenuBar();
    MenuRef macMenu();
#endif
#ifdef Q_OS_WINCE
    void wceCreateMenuBar(QWidget *);
    void wceDestroyMenuBar();
    struct QWceMenuBarPrivate {
        QList<QWceMenuAction*> actionItems;
        QList<QWceMenuAction*> actionItemsLeftButton;
        QList<QList<QWceMenuAction*>> actionItemsClassic;
        HMENU menuHandle;
        HMENU leftButtonMenuHandle;
        HWND menubarHandle;
        HWND parentWindowHandle;
        bool leftButtonIsMenu;
        QPointer<QAction> leftButtonAction;
        QMenuBarPrivate *d;
        int leftButtonCommand;

        QWceMenuBarPrivate(QMenuBarPrivate *menubar);
        ~QWceMenuBarPrivate();
        void addAction(QAction *, QWceMenuAction* =0);
        void addAction(QWceMenuAction *, QWceMenuAction* =0);
        void syncAction(QWceMenuAction *);
        inline void syncAction(QAction *a) { syncAction(findAction(a)); }
        void removeAction(QWceMenuAction *);
        void rebuild();
        inline void removeAction(QAction *a) { removeAction(findAction(a)); }
        inline QWceMenuAction *findAction(QAction *a) {
            for(int i = 0; i < actionItems.size(); i++) {
                QWceMenuAction *act = actionItems[i];
                if(a == act->action)
                    return act;
            }
            return 0;
        }
    } *wce_menubar;
    bool wceClassicMenu;
    void wceCommands(uint command);
    void wceRefresh();
    bool wceEmitSignals(QList<QWceMenuAction*> actions, uint command);
#endif
};
#endif

#endif // QT_NO_MENUBAR

QT_END_NAMESPACE

#endif // QMENUBAR_P_H
