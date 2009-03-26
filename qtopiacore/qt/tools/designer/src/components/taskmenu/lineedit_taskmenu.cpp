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
TRANSLATOR qdesigner_internal::LineEditTaskMenu
*/

#include "lineedit_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// -------- LineEditTaskMenuInlineEditor
class LineEditTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    LineEditTaskMenuInlineEditor(QLineEdit *button, QObject *parent);

protected:
    virtual QRect editRectangle() const;
};

LineEditTaskMenuInlineEditor::LineEditTaskMenuInlineEditor(QLineEdit *w, QObject *parent) :
      TaskMenuInlineEditor(w, ValidationSingleLine, QLatin1String("text"), parent)
{
}

QRect LineEditTaskMenuInlineEditor::editRectangle() const
{
    QStyleOption opt;
    opt.init(widget());
    return opt.rect;
}

// --------------- LineEditTaskMenu
LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent) :
    QDesignerTaskMenu(lineEdit, parent),
    m_editTextAction(new QAction(tr("Change text..."), this))
{
    TaskMenuInlineEditor *editor = new LineEditTaskMenuInlineEditor(lineEdit, this);
    connect(m_editTextAction, SIGNAL(triggered()), editor, SLOT(editText()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QAction *LineEditTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LineEditTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

}

QT_END_NAMESPACE
