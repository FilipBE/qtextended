/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

/*
TRANSLATOR qdesigner_internal::WidgetEditorTool
*/

#include "tool_widgeteditor.h"
#include "formwindow.h"

// sdk
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerWidgetFactoryInterface>
#include <QtDesigner/QDesignerWidgetBoxInterface>

#include <layoutinfo_p.h>
#include <qdesigner_dnditem_p.h>

#include <QtGui/qevent.h>
#include <QtGui/QAction>
#include <QtGui/QMainWindow>
#include <QtGui/QCursor>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

WidgetEditorTool::WidgetEditorTool(FormWindow *formWindow)
    : QDesignerFormWindowToolInterface(formWindow),
      m_formWindow(formWindow),
      m_action(new QAction(tr("Edit Widgets"), this))
{
}

QAction *WidgetEditorTool::action() const
{
    return m_action;
}

WidgetEditorTool::~WidgetEditorTool()
{
}

QDesignerFormEditorInterface *WidgetEditorTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *WidgetEditorTool::formWindow() const
{
    return m_formWindow;
}

bool WidgetEditorTool::mainWindowSeparatorEvent(QWidget *widget, QEvent *event)
{
    QMainWindow *mw = qobject_cast<QMainWindow*>(widget);
    if (mw == 0)
        return false;

    if (event->type() != QEvent::MouseButtonPress
            && event->type() != QEvent::MouseMove
            && event->type() != QEvent::MouseButtonRelease)
        return false;

    QMouseEvent *e = static_cast<QMouseEvent*>(event);

    if (event->type() == QEvent::MouseButtonPress) {
        if (mw->isSeparator(e->pos())) {
            m_separator_drag_mw = mw;
            return true;
        }
        return false;
    }

    if (event->type() == QEvent::MouseMove)
        return m_separator_drag_mw == mw;

    if (event->type() == QEvent::MouseButtonRelease) {
        if (m_separator_drag_mw != mw)
            return false;
        m_separator_drag_mw = 0;
        return true;
    }

    return false;
}

bool WidgetEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    const bool passive = core()->widgetFactory()->isPassiveInteractor(widget) != 0
                    || mainWindowSeparatorEvent(widget, event); // separators in QMainWindow
                                                                // are no longer widgets
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
        m_formWindow->updateSelection(widget);
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn: // Popup cancelled over a form widget: Reset its focus frame
        return !(passive || widget == m_formWindow || widget == m_formWindow->mainContainer());

    case QEvent::Wheel: // Prevent spinboxes and combos from reacting
        return !passive;

    case QEvent::KeyPress:
        return !passive && handleKeyPressEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::KeyRelease:
        return !passive && handleKeyReleaseEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::MouseMove:
        return !passive && handleMouseMoveEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonPress:
        return !passive && handleMousePressEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonRelease:
        return !passive && handleMouseReleaseEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonDblClick:
        return !passive && handleMouseButtonDblClickEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::ContextMenu:
        return !passive && handleContextMenu(widget, managedWidget, static_cast<QContextMenuEvent*>(event));

    case QEvent::DragEnter:
        return handleDragEnterMoveEvent(widget, managedWidget, static_cast<QDragEnterEvent *>(event), true);
    case QEvent::DragMove:
        return handleDragEnterMoveEvent(widget, managedWidget, static_cast<QDragEnterEvent *>(event), false);
    case QEvent::DragLeave:
        return handleDragLeaveEvent(widget, managedWidget, static_cast<QDragLeaveEvent *>(event));
    case QEvent::Drop:
        return handleDropEvent(widget, managedWidget, static_cast<QDropEvent *>(event));
    default:
        break;

    } // end switch

    return false;
}

// ### remove me

bool WidgetEditorTool::handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e)
{
    return m_formWindow->handleContextMenu(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseButtonDblClickEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMousePressEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseMoveEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseReleaseEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    return m_formWindow->handleKeyPressEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    return m_formWindow->handleKeyReleaseEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handlePaintEvent(QWidget *widget, QWidget *managedWidget, QPaintEvent *e)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(e);

    return false;
}

bool WidgetEditorTool::handleDragEnterMoveEvent(QWidget *widget, QWidget * /*managedWidget*/, QDragMoveEvent *e, bool isEnter)
{
    const QDesignerMimeData *mimeData = qobject_cast<const QDesignerMimeData *>(e->mimeData());
    if (!mimeData)
        return false;

    if (!m_formWindow->hasFeature(QDesignerFormWindowInterface::EditFeature)) {
        e->ignore();
        return true;
    }
    // If custom widgets have acceptDrops=true, the event occurs for them
    const QPoint formPos = widget != m_formWindow ? widget->mapTo(m_formWindow, e->pos()) : e->pos();
    const QPoint globalPos = m_formWindow->mapToGlobal(formPos);
    const FormWindowBase::WidgetUnderMouseMode wum = mimeData->items().size() == 1 ? FormWindowBase::FindSingleSelectionDropTarget : FormWindowBase::FindMultiSelectionDropTarget;
    QWidget *dropTarget = m_formWindow->widgetUnderMouse(formPos, wum);
    if (m_lastDropTarget && dropTarget != m_lastDropTarget)
        m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(globalPos), FormWindow::Restore);

    m_lastDropTarget = dropTarget;

    if (m_lastDropTarget)
        m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(globalPos), FormWindow::Highlight);

    if (isEnter || m_lastDropTarget)
        mimeData->acceptEvent(e);
    else
        e->ignore();
    return true;
}

bool WidgetEditorTool::handleDropEvent(QWidget *widget, QWidget *, QDropEvent *e)
{
    const QDesignerMimeData *mimeData = qobject_cast<const QDesignerMimeData *>(e->mimeData());
    if (!mimeData)
        return false;

    if (!m_lastDropTarget ||
        !m_formWindow->hasFeature(QDesignerFormWindowInterface::EditFeature)) {
        e->ignore();
        return true;
    }
    // FormWindow determines the position from the decoration.
    const QPoint globalPos = widget->mapToGlobal(e->pos());
    mimeData->moveDecoration(globalPos);
    if (!m_formWindow->dropWidgets(mimeData->items(), m_lastDropTarget, globalPos)) {
        e->ignore();
        return true;
    }
    mimeData->acceptEvent(e);
    return true;
}

bool WidgetEditorTool::restoreDropHighlighting()
{
    if (!m_lastDropTarget)
        return false;

    m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(QCursor::pos()), FormWindow::Restore);
    m_lastDropTarget = 0;
    return true;
}

bool WidgetEditorTool::handleDragLeaveEvent(QWidget *, QWidget *, QDragLeaveEvent *event)
{
    if (restoreDropHighlighting()) {
        event->accept();
        return true;
    }
    return false;
}

QWidget *WidgetEditorTool::editor() const
{
    Q_ASSERT(formWindow() != 0);
    return formWindow()->mainContainer();
}

void WidgetEditorTool::activated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(true);

    if (m_formWindow == 0)
        return;

    QList<QWidget*> sel = m_formWindow->selectedWidgets();
    foreach (QWidget *w, sel)
        m_formWindow->raiseSelection(w);
}

void WidgetEditorTool::deactivated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(false);

    if (m_formWindow == 0)
        return;

    m_formWindow->clearSelection();
}

QT_END_NAMESPACE
