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

#ifndef DESIGNERWIDGETITEM_H
#define DESIGNERWIDGETITEM_H

#include "shared_global_p.h"

#include <QtGui/QLayoutItem>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// QDesignerWidgetItem: A Layout Item that prevents its widget item from being slammed to size 0
// in the directions specified if the widget returns an empty sizeHint()/ minimumSizeHint().
// Used for form editor Layout Items containing a QFrame or a QWidget for which this
// will happen if they are non-laid out

class QDESIGNER_SHARED_EXPORT QDesignerWidgetItem : public QWidgetItemV2 {
    Q_DISABLE_COPY(QDesignerWidgetItem)

public:
    QDesignerWidgetItem(QWidget *w, Qt::Orientations o = Qt::Horizontal|Qt::Vertical);

    virtual QSize minimumSize() const { return  expand(QWidgetItemV2::minimumSize()); }
    virtual QSize sizeHint()    const { return  expand(QWidgetItemV2::sizeHint());    }

    // Register itself using QLayoutPrivate's widget item factory method hook
    static void install();
    static void deinstall();

private:
    QSize expand(const QSize &s) const;
    const Qt::Orientations m_orientations;
};

// Helper class that ensures QDesignerWidgetItem is installed while an instance is in scope.

class QDESIGNER_SHARED_EXPORT QDesignerWidgetItemInstaller {
    Q_DISABLE_COPY(QDesignerWidgetItemInstaller)

public:
    QDesignerWidgetItemInstaller();
    ~QDesignerWidgetItemInstaller();

private:
    static int m_instanceCount;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
