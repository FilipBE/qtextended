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

#include "formwindowsettings.h"
#include <formwindowbase_p.h>
#include <QtGui/QStyle>

QT_BEGIN_NAMESPACE

FormWindowSettings::FormWindowSettings(QDesignerFormWindowInterface *parent) :
    QDialog(parent),
    m_formWindow(qobject_cast<qdesigner_internal::FormWindowBase*>(parent))
{
    Q_ASSERT(m_formWindow);
    ui.setupUi(this);
    ui.gridPanel->setCheckable(true);
    ui.gridPanel->setResetButtonVisible(false);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    int defaultMargin = INT_MIN, defaultSpacing = INT_MIN;
    m_formWindow->layoutDefault(&defaultMargin, &defaultSpacing);

    QStyle *style = m_formWindow->style();
    ui.defaultMarginSpinBox->setValue(style->pixelMetric(QStyle::PM_DefaultChildMargin, 0));
    ui.defaultSpacingSpinBox->setValue(style->pixelMetric(QStyle::PM_DefaultLayoutSpacing, 0));

    if (defaultMargin != INT_MIN || defaultMargin != INT_MIN) {
        ui.layoutDefaultGroupBox->setChecked(true);

        if (defaultMargin != INT_MIN)
            ui.defaultMarginSpinBox->setValue(defaultMargin);

        if (defaultSpacing != INT_MIN)
            ui.defaultSpacingSpinBox->setValue(defaultSpacing);
    } else {
        ui.layoutDefaultGroupBox->setChecked(false);
    }

    QString marginFunction, spacingFunction;
    m_formWindow->layoutFunction(&marginFunction, &spacingFunction);
    if (!marginFunction.isEmpty() || !spacingFunction.isEmpty()) {
        ui.layoutFunctionGroupBox->setChecked(true);
        ui.marginFunctionLineEdit->setText(marginFunction);
        ui.spacingFunctionLineEdit->setText(spacingFunction);
    } else {
        ui.layoutFunctionGroupBox->setChecked(false);
    }

    const QString pixFunction = m_formWindow->pixmapFunction();
    ui.pixmapFunctionGroupBox->setChecked(!pixFunction.isEmpty());
    ui.pixmapFunctionLineEdit->setText(pixFunction);

    ui.authorLineEdit->setText(m_formWindow->author());

    foreach (QString includeHint, m_formWindow->includeHints()) {
        if (includeHint.isEmpty())
            continue;

        ui.includeHintsTextEdit->append(includeHint);
    }

    const bool hasFormGrid = m_formWindow->hasFormGrid();
    ui.gridPanel->setChecked(hasFormGrid);
    ui.gridPanel->setGrid(hasFormGrid ? m_formWindow->designerGrid() : qdesigner_internal::FormWindowBase::defaultDesignerGrid());
}

void FormWindowSettings::accept()
{
    m_formWindow->setAuthor(ui.authorLineEdit->text());

    if (ui.pixmapFunctionGroupBox->isChecked())
        m_formWindow->setPixmapFunction(ui.pixmapFunctionLineEdit->text());
    else
        m_formWindow->setPixmapFunction(QString());

    if (ui.layoutDefaultGroupBox->isChecked())
        m_formWindow->setLayoutDefault(ui.defaultMarginSpinBox->value(), ui.defaultSpacingSpinBox->value());
    else
        m_formWindow->setLayoutDefault(INT_MIN, INT_MIN);

    if (ui.layoutFunctionGroupBox->isChecked())
        m_formWindow->setLayoutFunction(ui.marginFunctionLineEdit->text(), ui.spacingFunctionLineEdit->text());
    else
        m_formWindow->setLayoutFunction(QString(), QString());

    m_formWindow->setIncludeHints(ui.includeHintsTextEdit->toPlainText().split(QString(QLatin1Char('\n'))));

    const bool hadFormGrid = m_formWindow->hasFormGrid();
    const bool wantsFormGrid = ui.gridPanel->isChecked();
    m_formWindow->setHasFormGrid(wantsFormGrid);
    if (wantsFormGrid || hadFormGrid != wantsFormGrid)
        m_formWindow->setDesignerGrid(wantsFormGrid ? ui.gridPanel->grid() : qdesigner_internal::FormWindowBase::defaultDesignerGrid());

    m_formWindow->setDirty(true);

    QDialog::accept();
}

QT_END_NAMESPACE
