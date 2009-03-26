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

#ifndef QUNDOVIEW_H
#define QUNDOVIEW_H

#include <QtGui/qlistview.h>
#include <QtCore/qstring.h>

#ifndef QT_NO_UNDOVIEW

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

QT_MODULE(Gui)

class Q_GUI_EXPORT QUndoView : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUndoView)
    Q_PROPERTY(QString emptyLabel READ emptyLabel WRITE setEmptyLabel)
    Q_PROPERTY(QIcon cleanIcon READ cleanIcon WRITE setCleanIcon)

public:
    explicit QUndoView(QWidget *parent = 0);
    explicit QUndoView(QUndoStack *stack, QWidget *parent = 0);
#ifndef QT_NO_UNDOGROUP
    explicit QUndoView(QUndoGroup *group, QWidget *parent = 0);
#endif
    ~QUndoView();

    QUndoStack *stack() const;
#ifndef QT_NO_UNDOGROUP
    QUndoGroup *group() const;
#endif

    void setEmptyLabel(const QString &label);
    QString emptyLabel() const;

    void setCleanIcon(const QIcon &icon);
    QIcon cleanIcon() const;

public Q_SLOTS:
    void setStack(QUndoStack *stack);
#ifndef QT_NO_UNDOGROUP
    void setGroup(QUndoGroup *group);
#endif

private:
    Q_DISABLE_COPY(QUndoView)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_UNDOVIEW
#endif // QUNDOVIEW_H
