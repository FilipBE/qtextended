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

#include "default_layoutdecoration.h"
#include "qlayout_widget_p.h"

#include <QtDesigner/QDesignerMetaDataBaseItemInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ---- QDesignerLayoutDecorationFactory ----
QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (!object->isWidgetType() || iid != Q_TYPEID(QDesignerLayoutDecorationExtension))
        return 0;

    QDesignerFormWindowInterface *fw = 0;
    QWidget *widget = qobject_cast<QWidget*>(object);
    bool match = false;
    do {
        if (const QLayoutWidget *layoutWidget = qobject_cast<const QLayoutWidget*>(widget)) {
            fw = layoutWidget->formWindow();
            match = true;
            break;
        }

        QLayout *layout = widget->layout();
        if (!layout)
            break;

        fw = QDesignerFormWindowInterface::findFormWindow(widget);
        if (!fw)
            break;

        if (fw->core()->metaDataBase()->item(layout))
            match = true;
    } while (false);
    if (match)
        return QLayoutSupport::createLayoutSupport(fw, widget, parent);
    return 0;
}
}

QT_END_NAMESPACE
