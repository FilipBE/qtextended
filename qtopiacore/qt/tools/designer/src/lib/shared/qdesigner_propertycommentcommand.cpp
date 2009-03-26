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

#include "qdesigner_propertycommentcommand_p.h"
#include "metadatabase_p.h"
#include "qdesigner_propertyeditor_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QApplication>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

SetPropertyCommentCommand::Entry::Entry(QObject* object, const QString &oldCommentValue) :
    m_object(object),
    m_oldCommentValue(oldCommentValue)
{
}

SetPropertyCommentCommand::SetPropertyCommentCommand(QDesignerFormWindowInterface *formWindow) :
     QDesignerFormWindowCommand(QString(), formWindow),
     m_propertyType(QVariant::Invalid)
{
}

bool SetPropertyCommentCommand::init(QObject *object, const QString &propertyName, const QString &newCommentValue)
{
    m_propertyName = propertyName;
    m_newCommentValue = newCommentValue;

    m_Entries.clear();
    if (!add(object))
        return false;

    setDescription();
    return true;
}

void SetPropertyCommentCommand::setDescription()
{
    if (m_Entries.size() == 1) {
        setText(QApplication::translate("Command", "changed comment of '%1' of '%2'").arg(m_propertyName).arg(m_Entries[0].m_object->objectName()));
    } else {
        int count = m_Entries.size();
        setText(QApplication::translate("Command", "changed comment of '%1' of %2 objects", "", QCoreApplication::UnicodeUTF8, count).arg(m_propertyName).arg(count));
    }
}

bool SetPropertyCommentCommand::init(const ObjectList &list, const QString &propertyName, const QString &newCommentValue)
{
    m_propertyName = propertyName;
    m_newCommentValue = newCommentValue;

    m_Entries.clear();
    foreach (QObject *o, list) {
        add(o);
    }

    if (m_Entries.empty())
        return false;

    setDescription();
    return true;
}


bool SetPropertyCommentCommand::add(QObject *object)
{
    QDesignerFormEditorInterface *core = formWindow()->core();
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    if (!sheet)
        return false;

    const int index = sheet->indexOf(m_propertyName);
    if (index == -1 || !sheet->isVisible(index))
        return false;

    // Set or check type
    const QVariant::Type propertyType = sheet->property(index).type();
    if (m_Entries.empty()) {
        m_propertyType = propertyType;
    } else {
        if ( propertyType != m_propertyType)
            return false;
    }

    const QString oldCommentValue = propertyComment(core, object, m_propertyName);

    m_Entries.push_back(Entry(object, oldCommentValue));
    return true;
}


int SetPropertyCommentCommand::id() const
{
    return 1968;
}

bool SetPropertyCommentCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    // check property name and list of objects
    const SetPropertyCommentCommand *cmd = static_cast<const SetPropertyCommentCommand*>(other);

    if (cmd->m_propertyName != m_propertyName)
         return false;

    const int numEntries = m_Entries.size();
    if (numEntries != cmd->m_Entries.size()) {
        return false;
    }

    for (int i = 0; i < numEntries; i++) {
        if (m_Entries[i].m_object != cmd->m_Entries[i].m_object)
            return false;
    }

    m_newCommentValue = cmd->m_newCommentValue;
    return true;

}

void SetPropertyCommentCommand::redo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QDesignerPropertyEditor *designerPropertyEditor = qobject_cast<QDesignerPropertyEditor *>(core->propertyEditor());
    Q_ASSERT(designerPropertyEditor);
    QObject* propertyEditorObject = designerPropertyEditor->object();
    // Set m_newCommentValue and update property editor
    const EntryList::const_iterator cend = m_Entries.end();
    for (EntryList::const_iterator it = m_Entries.begin(); it != cend; ++it) {
        if (QObject *object = it->m_object) { // might have been deleted
            setPropertyComment(core, object, m_propertyName, m_newCommentValue);
            if (object == propertyEditorObject)
                designerPropertyEditor->setPropertyComment(m_propertyName, m_newCommentValue);
        }
    }
}

void SetPropertyCommentCommand::undo()
{
    QDesignerFormEditorInterface *core = formWindow()->core();

    QDesignerPropertyEditor *designerPropertyEditor = qobject_cast<QDesignerPropertyEditor *>(core->propertyEditor());
    Q_ASSERT(designerPropertyEditor);
    QObject* propertyEditorObject = designerPropertyEditor->object();

    // Set stored old value and update property editor
    const EntryList::const_iterator cend = m_Entries.end();
    for (EntryList::const_iterator it = m_Entries.begin(); it != cend; ++it) {
        if (QObject *object = it->m_object) {
            setPropertyComment(core, object, m_propertyName, it->m_oldCommentValue);
            if (object == propertyEditorObject)
                designerPropertyEditor->setPropertyComment(m_propertyName, it->m_oldCommentValue);
        }
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
