/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "contentwindow.h"
#include "mainwindow.h"
#include "centralwidget.h"

#include <QtGui/QLayout>
#include <QtGui/QFocusEvent>
#include <QtGui/QMenu>
#include <QtGui/QTreeView>

#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpContentWidget>

QT_BEGIN_NAMESPACE

ContentWindow::ContentWindow(QHelpEngine *helpEngine)
{
	m_helpEngine = helpEngine;
    m_expandDepth = -2;

    QVBoxLayout *layout = new QVBoxLayout(this);
	m_contentWidget = m_helpEngine->contentWidget();
	connect(m_contentWidget, SIGNAL(linkActivated(const QUrl&)),
		this, SIGNAL(linkActivated(const QUrl&)));
    layout->setMargin(4);
	layout->addWidget(m_contentWidget);

    m_contentWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_contentWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));
    QHelpContentModel *contentModel = 
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    connect(contentModel, SIGNAL(contentsCreated()),
        this, SLOT(expandTOC()));

    m_contentWidget->viewport()->installEventFilter(this);
}

ContentWindow::~ContentWindow()
{
}

bool ContentWindow::syncToContent(const QUrl& url)
{
    QModelIndex idx = m_contentWidget->indexOf(url);
    if (!idx.isValid())
        return false;
    m_contentWidget->setCurrentIndex(idx);
    return true;
}

void ContentWindow::expandTOC()
{
    if (m_expandDepth > -2) {
        expandToDepth(m_expandDepth);
        m_expandDepth = -2;
    }    
}

void ContentWindow::expandToDepth(int depth)
{
    m_expandDepth = depth;
    if (depth == -1)
        m_contentWidget->expandAll();
    else
        m_contentWidget->expandToDepth(depth);
}

void ContentWindow::focusInEvent(QFocusEvent *e)
{
    if (e->reason() != Qt::MouseFocusReason)
        m_contentWidget->setFocus();
}

void ContentWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        MainWindow::activateCurrentCentralWidgetTab();
}

bool ContentWindow::eventFilter(QObject* o, QEvent *e)
{
    if (m_contentWidget && o == m_contentWidget->viewport() && e->type()
        == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (m_contentWidget->indexAt(me->pos()).isValid()
            && me->button() == Qt::LeftButton) {
                itemClicked(m_contentWidget->currentIndex());
        }
    }
    return QWidget::eventFilter(o,e);
}        


void ContentWindow::showContextMenu(const QPoint &pos)
{
    if (!m_contentWidget->indexAt(pos).isValid())
        return;

    QMenu menu;
    QAction *curTab = menu.addAction(tr("Open Link"));
    QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
    menu.move(m_contentWidget->mapToGlobal(pos));

    QHelpContentModel *contentModel = 
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    QHelpContentItem *itm = 
        contentModel->contentItemAt(m_contentWidget->currentIndex());

    QAction *action = menu.exec();
    if (curTab == action)
        emit linkActivated(itm->url());
    else if (newTab == action)
        CentralWidget::instance()->setSourceInNewTab(itm->url());    
}

void ContentWindow::itemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    QHelpContentModel *contentModel = 
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    QHelpContentItem *itm = 
        contentModel->contentItemAt(index);
    if (itm)
        emit linkActivated(itm->url());
}

QT_END_NAMESPACE
