/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "qtgradientviewdialog.h"
#include "qtgradientmanager.h"
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

QtGradientViewDialog::QtGradientViewDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(m_ui.gradientView, SIGNAL(currentGradientChanged(const QString &)),
            this, SLOT(slotGradientSelected(const QString &)));
    connect(m_ui.gradientView, SIGNAL(gradientActivated(const QString &)),
            this, SLOT(slotGradientActivated(const QString &)));
}

void QtGradientViewDialog::setGradientManager(QtGradientManager *manager)
{
    m_ui.gradientView->setGradientManager(manager);
}

QGradient QtGradientViewDialog::getGradient(bool *ok, QtGradientManager *manager, QWidget *parent, const QString &caption)
{
    QtGradientViewDialog dlg(parent);
    dlg.setGradientManager(manager);
    dlg.setWindowTitle(caption);
    QGradient grad = QLinearGradient();
    const int res = dlg.exec();
    if (res == QDialog::Accepted)
        grad = dlg.m_ui.gradientView->gradientManager()->gradients().value(dlg.m_ui.gradientView->currentGradient());
    if (ok)
        *ok = res == QDialog::Accepted;
    return grad;
}

void QtGradientViewDialog::slotGradientSelected(const QString &id)
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!id.isEmpty());
}

void QtGradientViewDialog::slotGradientActivated(const QString &id)
{
    Q_UNUSED(id)
    accept();
}

QT_END_NAMESPACE
