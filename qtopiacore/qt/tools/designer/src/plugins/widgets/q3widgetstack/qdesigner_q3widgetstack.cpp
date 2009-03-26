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

#include "qdesigner_q3widgetstack_p.h"
#include "../../../lib/shared/qdesigner_propertycommand_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QEvent>
#include <QtGui/QToolButton>

QT_BEGIN_NAMESPACE

namespace {
    QToolButton *createToolButton(QWidget *parent, Qt::ArrowType at, const QString &name) {
         QToolButton *rc =  new QToolButton();
         rc->setAttribute(Qt::WA_NoChildEventsForParent, true);
         rc->setParent(parent);
         rc->setObjectName(name);
         rc->setArrowType(at);
         rc->setAutoRaise(true);
         rc->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
         rc->setFixedSize(QSize(15, 15));
         return rc;
     }
}

QDesignerQ3WidgetStack::QDesignerQ3WidgetStack(QWidget *parent) : 
    Q3WidgetStack(parent), 
    m_prev(createToolButton(this, Qt::LeftArrow,  QLatin1String("__qt__passive_prev"))),
    m_next(createToolButton(this, Qt::RightArrow, QLatin1String("__qt__passive_next")))
{
    connect(m_prev, SIGNAL(clicked()), this, SLOT(prevPage()));
    connect(m_next, SIGNAL(clicked()), this, SLOT(nextPage()));
    updateButtons();

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

QDesignerFormWindowInterface *QDesignerQ3WidgetStack::formWindow()
{
    return QDesignerFormWindowInterface::findFormWindow(this);
}

QDesignerContainerExtension *QDesignerQ3WidgetStack::container()
{
    if (formWindow()) {
        QDesignerFormEditorInterface *core = formWindow()->core();
        return qt_extension<QDesignerContainerExtension*>(core->extensionManager(), this);
    }
    return 0;
}

int QDesignerQ3WidgetStack::count()
{
    return container() ? container()->count() : 0;
}

int QDesignerQ3WidgetStack::currentIndex()
{
    return container() ? container()->currentIndex() : -1;
}

void QDesignerQ3WidgetStack::setCurrentIndex(int index)
{
    if (container() && (index >= 0) && (index < count())) {
        container()->setCurrentIndex(index);
        emit currentChanged(index);
    }
}

QWidget *QDesignerQ3WidgetStack::widget(int index)
{
    return container() ? container()->widget(index) : 0;
}

void QDesignerQ3WidgetStack::updateButtons()
{
    if (m_prev) {
        m_prev->move(width() - 31, 1);
        m_prev->show();
        m_prev->raise();
    }

    if (m_next) {
        m_next->move(width() - 16, 1);
        m_next->show();
        m_next->raise();
    }
}

void QDesignerQ3WidgetStack::gotoPage(int page) {
    // Are we on a form or in a preview?
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        qdesigner_internal::SetPropertyCommand *cmd = new qdesigner_internal::SetPropertyCommand(fw);
        cmd->init(this, QLatin1String("currentIndex"), page);
        fw->commandHistory()->push(cmd);
        fw->emitSelectionChanged(); // Magically prevent an endless loop triggered by auto-repeat.
    } else {
        setCurrentIndex(page);
    }
    updateButtons();
}


void QDesignerQ3WidgetStack::prevPage()
{
    if (count() > 1) {
        int newIndex = currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count() - 1;
        gotoPage(newIndex);
    }
}

void QDesignerQ3WidgetStack::nextPage()
{
    if (count() > 1)
        gotoPage((currentIndex() + 1) % count());
}

QString QDesignerQ3WidgetStack::currentPageName()
{
    if (currentIndex() == -1)
        return QString();

    return widget(currentIndex())->objectName();
}

void QDesignerQ3WidgetStack::setCurrentPageName(const QString &pageName)
{
    if (currentIndex() == -1)
        return;

    if (QWidget *w = widget(currentIndex()))
        w->setObjectName(pageName);
}

bool QDesignerQ3WidgetStack::event(QEvent *e)
{
    if (e->type() == QEvent::LayoutRequest) {
        updateButtons();
    }

    return Q3WidgetStack::event(e);
}

void QDesignerQ3WidgetStack::childEvent(QChildEvent *e)
{
    Q3WidgetStack::childEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::resizeEvent(QResizeEvent *e)
{
    Q3WidgetStack::resizeEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::showEvent(QShowEvent *e)
{
    Q3WidgetStack::showEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = formWindow()) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}

QT_END_NAMESPACE
