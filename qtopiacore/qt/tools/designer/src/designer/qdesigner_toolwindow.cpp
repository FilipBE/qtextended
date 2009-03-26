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

#include "qdesigner.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"

#include <QtCore/QEvent>
#include <QtCore/qdebug.h>
#include <QtGui/QAction>
#include <QtGui/QCloseEvent>

QT_BEGIN_NAMESPACE

QDesignerToolWindow::QDesignerToolWindow(QDesignerWorkbench *workbench, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_workbench(workbench),
      m_saveSettings(false)
{
    Q_ASSERT(workbench != 0);

#ifndef Q_WS_MAC
    setWindowIcon(qDesigner->windowIcon());
#endif    

    m_action = new QAction(this);
    m_action->setShortcutContext(Qt::ApplicationShortcut);
    m_action->setText(windowTitle());
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}

QDesignerToolWindow::~QDesignerToolWindow()
{
    if (workbench())
        workbench()->removeToolWindow(this);
}

void QDesignerToolWindow::showMe(bool v)
{
    QWidget *target = parentWidget() == 0 ? this : parentWidget();

    if (v)
        target->setWindowState(target->windowState() & ~Qt::WindowMinimized);

    target->setVisible(v);
}

void QDesignerToolWindow::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);

    bool blocked = m_action->blockSignals(true);
    m_action->setChecked(true);
    m_action->blockSignals(blocked);
}

void QDesignerToolWindow::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);

    bool blocked = m_action->blockSignals(true);
    m_action->setChecked(false);
    m_action->blockSignals(blocked);
}

QAction *QDesignerToolWindow::action() const
{
    return m_action;
}

void QDesignerToolWindow::changeEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::WindowTitleChange:
            m_action->setText(windowTitle());
            break;
        case QEvent::WindowIconChange:
            m_action->setIcon(windowIcon());
            break;
        default:
            break;
    }
    QMainWindow::changeEvent(e);
}

QDesignerWorkbench *QDesignerToolWindow::workbench() const
{
    return m_workbench;
}

QRect QDesignerToolWindow::geometryHint() const
{
    return QRect();
}

void QDesignerToolWindow::closeEvent(QCloseEvent *ev)
{
    if (m_saveSettings) {
        ev->setAccepted(workbench()->handleClose());
        if (ev->isAccepted() && qDesigner->mainWindow() == this)
            QMetaObject::invokeMethod(qDesigner, "quit", Qt::QueuedConnection);  // We're going down!
    } else {
        QMainWindow::closeEvent(ev);
    }
}

bool QDesignerToolWindow::saveSettingsOnClose() const
{
    return m_saveSettings;
}

void QDesignerToolWindow::setSaveSettingsOnClose(bool save)
{
    m_saveSettings = save;
}

Qt::DockWidgetArea QDesignerToolWindow::dockWidgetAreaHint() const
{
    return Qt::RightDockWidgetArea;
}


QT_END_NAMESPACE
