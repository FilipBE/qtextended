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

#include "qwizard_container.h"

#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QWizardContainer::QWizardContainer(QWizard *widget, QObject *parent) :
    QObject(parent),
    m_wizard(widget)
{
}

QWizardContainer::Pages QWizardContainer::pages() const
{
    return qFindChildren<QWizardPage *>(m_wizard);
}

int QWizardContainer::count() const
{
    return pages().size();
}

QWidget *QWizardContainer::widget(int index) const
{
    if (index < 0)
        return 0;
    QWidget *rc = pages().at(index);
    // Hack: In some modi, pages have height 0.
    if (rc->isVisible() && rc->height() == 0)
        rc->resize(m_wizard->size());
    return rc;
}

int QWizardContainer::currentIndex() const
{
    QWizardPage *currentPage = m_wizard->currentPage();
    if (!currentPage)
        return  -1;
     return pages().indexOf(currentPage);
}

void QWizardContainer::setCurrentIndex(int /* index*/)
{
    qDebug() << "** WARNING QWizardContainer::setCurrentIndex is not implemented";
}

void QWizardContainer::addWidget(QWidget *widget)
{

    QWizardPage *page = qobject_cast<QWizardPage *>(widget);
    if (!page) {
        qDebug() << "** WARNING Attempt to add oject that is not of class WizardPage to a QWizard";
        return;
    }
    m_wizard->addPage(page);
}

void QWizardContainer::insertWidget(int /* index*/, QWidget *widget)
{
    qDebug() << "** WARNING QWizardContainer::insertWidget is not implemented, defaulting to add";
    addWidget(widget);
}

void QWizardContainer::remove(int /* index */)
{
    qDebug() << "** WARNING QWizardContainer::remove is not implemented";
}
}

QT_END_NAMESPACE
