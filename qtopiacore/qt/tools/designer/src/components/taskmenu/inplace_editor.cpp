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

#include "abstractformwindow.h"
#include "inplace_editor.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ----------------- InPlaceEditor

InPlaceEditor::InPlaceEditor(QWidget *widget,
                             TextPropertyValidationMode validationMode,
                             QDesignerFormWindowInterface *fw,
                             const QString& text,
                             const QRect& r) :
    TextPropertyEditor(widget, EmbeddingInPlace, validationMode),
    m_InPlaceWidgetHelper(this, widget, fw)
{
    setAlignment(m_InPlaceWidgetHelper.alignment());
    setObjectName(QLatin1String("__qt__passive_m_editor"));

    setText(text);
    selectAll();

    setGeometry(QRect(widget->mapTo(widget->window(), r.topLeft()), r.size()));
    setFocus();
    show();

    connect(this, SIGNAL(editingFinished()),this, SLOT(close()));
}


// -------------- TaskMenuInlineEditor

TaskMenuInlineEditor::TaskMenuInlineEditor(QWidget *w, TextPropertyValidationMode vm,
                                           const QString &property, QObject *parent) :
    QObject(parent),
    m_vm(vm),
    m_property(property),
    m_widget(w)
{
}

void TaskMenuInlineEditor::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_widget);
    if (m_formWindow.isNull())
        return;
    // Close as soon as a different widget is selected
    connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));

    // get old value
    QDesignerFormEditorInterface *core = m_formWindow->core();
    const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), m_widget);
    const int index = sheet->indexOf(m_property);
    if (index == -1)
        return;
    const QString oldValue = sheet->property(index).toString();

    m_editor = new InPlaceEditor(m_widget, m_vm, m_formWindow, oldValue, editRectangle());
    connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));
}

void TaskMenuInlineEditor::updateText(const QString &text)
{
    m_formWindow->cursor()->setProperty(m_property, QVariant(text));
}

void TaskMenuInlineEditor::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

}

QT_END_NAMESPACE
