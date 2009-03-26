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

#ifndef QLAYOUT_P_H
#define QLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout*.cpp, and qabstractlayout.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "private/qobject_p.h"
#include "qstyle.h"
#include "qsizepolicy.h"

QT_BEGIN_NAMESPACE

class QWidgetItem;
class QSpacerItem;

class Q_GUI_EXPORT QLayoutPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QLayout)

public:
    typedef QWidgetItem * (*QWidgetItemFactoryMethod)(const QLayout *layout, QWidget *widget);
    typedef QSpacerItem * (*QSpacerItemFactoryMethod)(const QLayout *layout, int w, int h, QSizePolicy::Policy hPolicy, QSizePolicy::Policy);

    QLayoutPrivate();

    void getMargin(int *result, int userMargin, QStyle::PixelMetric pm) const;
    void doResize(const QSize &);
    void reparentChildWidgets(QWidget *mw);

    static QWidgetItem *createWidgetItem(const QLayout *layout, QWidget *widget);
    static QSpacerItem *createSpacerItem(const QLayout *layout, int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum, QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

    static QWidgetItemFactoryMethod widgetItemFactoryMethod;
    static QSpacerItemFactoryMethod spacerItemFactoryMethod;

    int insideSpacing;
    int userLeftMargin;
    int userTopMargin;
    int userRightMargin;
    int userBottomMargin;
    uint topLevel : 1;
    uint enabled : 1;
    uint activated : 1;
    uint autoNewChild : 1;
    QLayout::SizeConstraint constraint;
    QRect rect;
    QWidget *menubar;
};

QT_END_NAMESPACE

#endif // QLAYOUT_P_H
