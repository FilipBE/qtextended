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

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

class QWidget;
class QLayout;
class QDesignerFormEditorInterface;
class QFormLayout;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT LayoutInfo
{
public:
    enum Type
    {
        NoLayout,
        HSplitter,
        VSplitter,
        HBox,
        VBox,
        Grid,
        Form,
        UnknownLayout // QDockWindow inside QMainWindow is inside QMainWindowLayout - it doesn't mean there is no layout
    };

    static void deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget);

    static Type layoutType(const QDesignerFormEditorInterface *core, const QWidget *w);
    static Type layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout);
    static Type layoutType(const QString &typeName);

    static QWidget *layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout);

    static Type laidoutWidgetType(const QDesignerFormEditorInterface *core, QWidget *widget, bool *isManaged = 0);
    static bool inline isWidgetLaidout(const QDesignerFormEditorInterface *core, QWidget *widget) { return laidoutWidgetType(core, widget) != NoLayout; }

    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget);
    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, QLayout *layout);
    static QLayout *internalLayout(const QWidget *widget);
};

QDESIGNER_SHARED_EXPORT void getFormLayoutItemPosition(const QFormLayout *formLayout, int index, int *rowPtr, int *columnPtr = 0, int *rowspanPtr = 0, int *colspanPtr = 0);
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LAYOUTINFO_H
