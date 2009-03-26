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

#include "qdesigner_widgetitem_p.h"
#include "qdesigner_widget_p.h"

#include <QtGui/QFrame>
#include <QtGui/QBoxLayout>
#include <QtCore/QDebug>
#include <QtCore/QMetaObject>
#include <private/qlayout_p.h>

QT_BEGIN_NAMESPACE

enum { DebugWidgetItem = 0 };
enum { MinimumLength = 10 };

// Widget item creation function to be registered as factory method with QLayoutPrivate
static QWidgetItem *createDesignerWidgetItem(const QLayout *layout, QWidget *widget)
{
    const QMetaObject *mo = widget->metaObject();
    // Only for QWidgets and QFrames within designer
    if (mo == &QWidget::staticMetaObject || mo == &QFrame::staticMetaObject || mo == &QDesignerWidget::staticMetaObject) {
        Qt::Orientations o = Qt::Horizontal|Qt::Vertical;
        // If it is a box, restrict to its orientation
        if (const QBoxLayout *bl = qobject_cast<const QBoxLayout *>(layout)) {
            const QBoxLayout::Direction direction = bl->direction();
            o = direction == QBoxLayout::LeftToRight || direction == QBoxLayout::RightToLeft ? Qt::Horizontal : Qt::Vertical;
        }
        if (DebugWidgetItem)
            qDebug() << "QDesignerWidgetItem: Creating on " << widget;
        return new qdesigner_internal::QDesignerWidgetItem(widget, o);
    }
    return 0;
}

namespace qdesigner_internal {

// ------------------ QDesignerWidgetItem
QDesignerWidgetItem::QDesignerWidgetItem(QWidget *w, Qt::Orientations o) :
    QWidgetItemV2(w),
    m_orientations(o)
{
}

QSize QDesignerWidgetItem::expand(const QSize &s) const
{
    // Expand the size if its too small
    QSize rc = s;
    if (m_orientations & Qt::Horizontal && rc.width() <= 0)
        rc.setWidth(MinimumLength);
    if (m_orientations & Qt::Vertical && rc.height() <= 0)
        rc.setHeight(MinimumLength);
    return rc;
}

void QDesignerWidgetItem::install()
{
    QLayoutPrivate::widgetItemFactoryMethod = createDesignerWidgetItem;
}

void QDesignerWidgetItem::deinstall()
{
    QLayoutPrivate::widgetItemFactoryMethod = 0;
}

// ------------------   QDesignerWidgetItemInstaller

int QDesignerWidgetItemInstaller::m_instanceCount = 0;

QDesignerWidgetItemInstaller::QDesignerWidgetItemInstaller()
{
    if (m_instanceCount++ == 0) {
        if (DebugWidgetItem)
            qDebug() << "QDesignerWidgetItemInstaller: installing";
        QDesignerWidgetItem::install();
    }

}

QDesignerWidgetItemInstaller::~QDesignerWidgetItemInstaller()
{
    if (--m_instanceCount == 0) {
        if (DebugWidgetItem)
            qDebug() << "QDesignerWidgetItemInstaller: deinstalling";
        QDesignerWidgetItem::deinstall();
    }
}

}

QT_END_NAMESPACE
