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
TRANSLATOR qdesigner_internal::ButtonTaskMenu
*/

#include "button_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// -------- Text area editor
class ButtonTextTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    ButtonTextTaskMenuInlineEditor(QAbstractButton *button, QObject *parent);

protected:
    virtual QRect editRectangle() const;
};

ButtonTextTaskMenuInlineEditor::ButtonTextTaskMenuInlineEditor(QAbstractButton *button, QObject *parent) :
      TaskMenuInlineEditor(button, ValidationMultiLine, QLatin1String("text"), parent)
{
}

QRect ButtonTextTaskMenuInlineEditor::editRectangle() const
{
    QWidget *w = widget();
    QStyleOptionButton opt;
    opt.init(w);
    return w->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, w);
}

// -------- Command link button description editor
class LinkDescriptionTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    LinkDescriptionTaskMenuInlineEditor(QAbstractButton *button, QObject *parent);

protected:
    virtual QRect editRectangle() const;
};

LinkDescriptionTaskMenuInlineEditor::LinkDescriptionTaskMenuInlineEditor(QAbstractButton *button, QObject *parent) :
      TaskMenuInlineEditor(button, ValidationMultiLine, QLatin1String("description"), parent)
{
}

QRect LinkDescriptionTaskMenuInlineEditor::editRectangle() const
{
    QWidget *w = widget(); // TODO: What is the exact description area?
    QStyleOptionButton opt;
    opt.init(w);
    return w->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, w);
}

// ----------- ButtonTaskMenu:

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)  :
    QDesignerTaskMenu(button, parent),
    m_preferredEditAction(new QAction(tr("Change text..."), this))
{
    TaskMenuInlineEditor *textEditor = new ButtonTextTaskMenuInlineEditor(button, this);
    connect(m_preferredEditAction, SIGNAL(triggered()), textEditor, SLOT(editText()));
    m_taskActions.append(m_preferredEditAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QAction *ButtonTaskMenu::preferredEditAction() const
{
    return m_preferredEditAction;
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ButtonTaskMenu::insertAction(int index, QAction *a)
{
    m_taskActions.insert(index, a);
}

// -------------- CommandLinkButtonTaskMenu

CommandLinkButtonTaskMenu::CommandLinkButtonTaskMenu(QCommandLinkButton *button, QObject *parent) :
    ButtonTaskMenu(button, parent)
{
    TaskMenuInlineEditor *descriptonEditor = new LinkDescriptionTaskMenuInlineEditor(button, this);
    QAction *descriptionAction = new QAction(tr("Change description..."), this);
    connect(descriptionAction, SIGNAL(triggered()), descriptonEditor, SLOT(editText()));
    insertAction(1, descriptionAction);
}

}

QT_END_NAMESPACE
