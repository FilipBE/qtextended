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

#ifndef QSTACKEDLAYOUT_H
#define QSTACKEDLAYOUT_H

#include <QtGui/qlayout.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QStackedLayoutPrivate;

class Q_GUI_EXPORT QStackedLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QStackedLayout)
    Q_ENUMS(StackingMode)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(StackingMode stackingMode READ stackingMode WRITE setStackingMode)
    QDOC_PROPERTY(int count READ count)

public:
    enum StackingMode {
        StackOne,
        StackAll
    };

    QStackedLayout();
    explicit QStackedLayout(QWidget *parent);
    explicit QStackedLayout(QLayout *parentLayout);
    ~QStackedLayout();

    int addWidget(QWidget *w);
    int insertWidget(int index, QWidget *w);

    QWidget *currentWidget() const;
    int currentIndex() const;
#ifdef Q_NO_USING_KEYWORD
    inline QWidget *widget() { return QLayout::widget(); }
#else
    using QLayout::widget;
#endif
    QWidget *widget(int) const;
    int count() const;

    StackingMode stackingMode() const;
    void setStackingMode(StackingMode stackingMode);

    // abstract virtual functions:
    void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt(int) const;
    QLayoutItem *takeAt(int);
    void setGeometry(const QRect &rect);

Q_SIGNALS:
    void widgetRemoved(int index);
    void currentChanged(int index);

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget *w);

private:
    Q_DISABLE_COPY(QStackedLayout)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSTACKEDLAYOUT_H
