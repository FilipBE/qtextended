/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QTREEWIDGETITEMITERATOR_H
#define QTREEWIDGETITEMITERATOR_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_TREEWIDGET

class QTreeWidget;
class QTreeWidgetItem;
class QTreeModel;

class QTreeWidgetItemIteratorPrivate;
class Q_GUI_EXPORT QTreeWidgetItemIterator
{
    friend class QTreeModel;

public:
    enum IteratorFlag {
        All           = 0x00000000,
        Hidden        = 0x00000001,
        NotHidden     = 0x00000002,
        Selected      = 0x00000004,
        Unselected    = 0x00000008,
        Selectable    = 0x00000010,
        NotSelectable = 0x00000020,
        DragEnabled   = 0x00000040,
        DragDisabled  = 0x00000080,
        DropEnabled   = 0x00000100,
        DropDisabled  = 0x00000200,
        HasChildren   = 0x00000400,
        NoChildren    = 0x00000800,
        Checked       = 0x00001000,
        NotChecked    = 0x00002000,
        Enabled       = 0x00004000,
        Disabled      = 0x00008000,
        Editable      = 0x00010000,
        NotEditable   = 0x00020000,
        UserFlag      = 0x01000000 // The first flag that can be used by the user.
    };
    Q_DECLARE_FLAGS(IteratorFlags, IteratorFlag)

    QTreeWidgetItemIterator(const QTreeWidgetItemIterator &it);
    explicit QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags = All);
    explicit QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags = All);
    ~QTreeWidgetItemIterator();

    QTreeWidgetItemIterator &operator=(const QTreeWidgetItemIterator &it);

    QTreeWidgetItemIterator &operator++();
    inline const QTreeWidgetItemIterator operator++(int);
    inline QTreeWidgetItemIterator &operator+=(int n);

    QTreeWidgetItemIterator &operator--();
    inline const QTreeWidgetItemIterator operator--(int);
    inline QTreeWidgetItemIterator &operator-=(int n);

    inline QTreeWidgetItem *operator*() const;

private:
    bool matchesFlags(const QTreeWidgetItem *item) const;
    QTreeWidgetItemIteratorPrivate *d_ptr;
    QTreeWidgetItem *current;
    IteratorFlags flags;
    Q_DECLARE_PRIVATE(QTreeWidgetItemIterator)
};

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator++(int)
{
    QTreeWidgetItemIterator it = *this;
    ++(*this);
    return it;
}

inline const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator--(int)
{
    QTreeWidgetItemIterator it = *this;
    --(*this);
    return it;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator+=(int n)
{
    if (n < 0)
        return (*this) -= (-n);
    while (current && n--)
        ++(*this);
    return *this;
}

inline QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator-=(int n)
{
    if (n < 0)
        return (*this) += (-n);
    while (current && n--)
        --(*this);
    return *this;
}

inline QTreeWidgetItem *QTreeWidgetItemIterator::operator*() const
{
    return current;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QTreeWidgetItemIterator::IteratorFlags)


QT_END_NAMESPACE
#endif // QT_NO_TREEWIDGET
QT_END_HEADER

#endif // QTREEWIDGETITEMITERATOR_H
