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

#include "qabstractpagesetupdialog.h"
#include "qabstractpagesetupdialog_p.h"

#ifndef QT_NO_PRINTDIALOG

#include <QtCore/qcoreapplication.h>
#include <QtGui/qprinter.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QAbstractPageSetupDialog

    \brief The QAbstractPageSetupDialog class provides a base for
    implementations of page setup dialogs.
*/

/*!
    Constructs the page setup dialog for the printer \a printer with
    \a parent as parent widget.
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QAbstractPageSetupDialogPrivate), parent)
{
    Q_D(QAbstractPageSetupDialog);
    d->printer = printer;
    setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));

    if (printer->outputFormat() != QPrinter::NativeFormat) {
        qWarning("QAbstractPageSetupDialog::QAbstractPageSetupDialog: Page setup dialog cannot be "
                 "used on non-native printers");
    }
}

/*!
    \internal
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QAbstractPageSetupDialogPrivate &ptr,
                                                   QPrinter *printer, QWidget *parent)
    : QDialog(ptr, parent)
{
    Q_D(QAbstractPageSetupDialog);
    d->printer = printer;
    setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));

    if (printer->outputFormat() != QPrinter::NativeFormat) {
        qWarning("QAbstractPageSetupDialog::QAbstractPageSetupDialog: Page setup dialog cannot be "
                 "used on non-native printers");
    }
}

/*!
    Returns the printer that this page setup dialog is operating on.
*/
QPrinter *QAbstractPageSetupDialog::printer()
{
    Q_D(QAbstractPageSetupDialog);
    return d->printer;
}

/*!
    \fn int QAbstractPageSetupDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
