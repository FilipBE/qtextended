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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_PROPERTYCOMMENTCOMMAND_H
#define QDESIGNER_PROPERTYCOMMENTCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT SetPropertyCommentCommand: public QDesignerFormWindowCommand
{
    SetPropertyCommentCommand(const SetPropertyCommentCommand &);
    SetPropertyCommentCommand& operator=(const SetPropertyCommentCommand &);

public:
    explicit SetPropertyCommentCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QObject *object, const QString &propertyName, const QString &newCommentValue);

    typedef QList<QObject *> ObjectList;
    bool init(const ObjectList &list, const QString &propertyName, const QString &newCommentValue);

    int id() const;
    bool mergeWith(const QUndoCommand *other);

    virtual void redo();
    virtual void undo();

private:
    bool add(QObject *object);
    void setDescription();

    struct Entry {
        Entry(QObject* object, const QString &oldCommentValue);
        QPointer<QObject> m_object;
        QString m_oldCommentValue;
    };

    typedef QList<Entry> EntryList;
    EntryList m_Entries;

    QVariant::Type m_propertyType;
    QString m_propertyName;
    QString m_newCommentValue;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_PROPERTYCOMMENTCOMMAND_H
