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

#include "propertylineedit_p.h"

#include <QtGui/QContextMenuEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    PropertyLineEdit::PropertyLineEdit(QWidget *parent) :
        QLineEdit(parent), m_wantNewLine(false)
    {
    }

    bool PropertyLineEdit::event(QEvent *e)
    {
        // handle 'Select all' here as it is not done in the QLineEdit
        if (e->type() == QEvent::ShortcutOverride && !isReadOnly()) {
            QKeyEvent* ke = static_cast<QKeyEvent*> (e);
            if (ke->modifiers() & Qt::ControlModifier) {
                if(ke->key() == Qt::Key_A) {
                    ke->accept();
                    return true;
                }
            }
        }
        return QLineEdit::event(e);
    }

    void PropertyLineEdit::insertNewLine() {
        insertText(QLatin1String("\\n"));
    }

    void PropertyLineEdit::insertText(const QString &text) {
        // position cursor after new text and grab focus
        const int oldCursorPosition = cursorPosition ();
        insert(text);
        setCursorPosition (oldCursorPosition + text.length());
        setFocus(Qt::OtherFocusReason);
    }

    void PropertyLineEdit::contextMenuEvent(QContextMenuEvent *event) {
        QMenu  *menu = createStandardContextMenu ();

        if (m_wantNewLine) {
            menu->addSeparator();
            QAction* nlAction = menu->addAction(tr("Insert line break"));
            connect(nlAction, SIGNAL(triggered()), this, SLOT(insertNewLine()));
        }

        menu->exec(event->globalPos());
    }
}

QT_END_NAMESPACE
