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
#ifndef QPAGESETUPWIDGET_H
#define QPAGESETUPWIDGET_H

#include "qglobal.h"

#ifndef QT_NO_PRINTDIALOG

#include <ui_qpagesetupwidget.h>

QT_BEGIN_NAMESPACE

class QPrinter;
class QPagePreview;
class QCUPSSupport;

class QPageSetupWidget : public QWidget {
    Q_OBJECT
public:
    QPageSetupWidget(QWidget *parent = 0);
    QPageSetupWidget(QPrinter *printer, QWidget *parent = 0);
    void setPrinter(QPrinter *printer);
    /// copy information from the widget and apply that to the printer.
    void setupPrinter() const;
    void selectPrinter(QCUPSSupport *m_cups);
    void selectPdfPsPrinter(const QPrinter *p);

private slots:
    void _q_pageOrientationChanged();
    void _q_paperSizeChanged();
    void unitChanged(int item);
    void setTopMargin(double newValue);
    void setBottomMargin(double newValue);
    void setLeftMargin(double newValue);
    void setRightMargin(double newValue);

private:
    Ui::QPageSetupWidget widget;
    QPagePreview *m_pagePreview;
    QPrinter *m_printer;
    qreal m_leftMargin;
    qreal m_topMargin;
    qreal m_rightMargin;
    qreal m_bottomMargin;
    QSizeF m_paperSize;
    qreal m_currentMultiplier;
    bool m_blockSignals;
    QCUPSSupport *m_cups;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
#endif
