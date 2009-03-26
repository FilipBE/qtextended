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

#include "gridpanel_p.h"
#include "ui_gridpanel.h"
#include "grid_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

GridPanel::GridPanel(QWidget *parentWidget) :
    QWidget(parentWidget)
{
    m_ui = new Ui::GridPanel;
    m_ui->setupUi(this);

    connect(m_ui->m_resetButton, SIGNAL(clicked()), this, SLOT(reset()));
}

GridPanel::~GridPanel()
{
    delete m_ui;
}

void GridPanel::setGrid(const Grid &g)
{
    m_ui->m_deltaXSpinBox->setValue(g.deltaX());
    m_ui->m_deltaYSpinBox->setValue(g.deltaY());
    m_ui->m_visibleCheckBox->setCheckState(g.visible() ? Qt::Checked : Qt::Unchecked);
    m_ui->m_snapXCheckBox->setCheckState(g.snapX()  ? Qt::Checked : Qt::Unchecked);
    m_ui->m_snapYCheckBox->setCheckState(g.snapY()  ? Qt::Checked : Qt::Unchecked);
}

void GridPanel::setTitle(const QString &title)
{
    m_ui->m_gridGroupBox->setTitle(title);
}

Grid GridPanel::grid() const
{
    Grid rc;
    rc.setDeltaX(m_ui->m_deltaXSpinBox->value());
    rc.setDeltaY(m_ui->m_deltaYSpinBox->value());
    rc.setSnapX(m_ui->m_snapXCheckBox->checkState() == Qt::Checked);
    rc.setSnapY(m_ui->m_snapYCheckBox->checkState() == Qt::Checked);
    rc.setVisible(m_ui->m_visibleCheckBox->checkState() == Qt::Checked);
    return rc;
}

void GridPanel::reset()
{
    setGrid(Grid());
}

void GridPanel::setCheckable (bool c)
{
    m_ui->m_gridGroupBox->setCheckable(c);
}

bool GridPanel::isCheckable () const
{
    return m_ui->m_gridGroupBox->isCheckable ();
}

bool GridPanel::isChecked () const
{
    return m_ui->m_gridGroupBox->isChecked ();
}

void GridPanel::setChecked(bool c)
{
    m_ui->m_gridGroupBox->setChecked(c);
}

void GridPanel::setResetButtonVisible(bool v)
{
    m_ui->m_resetButton->setVisible(v);
}

}

QT_END_NAMESPACE
