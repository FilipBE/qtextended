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

#include <private/qabstractpagesetupdialog_p.h>

#ifndef QT_NO_PRINTDIALOG

QT_BEGIN_NAMESPACE

/*!
    \enum QPageSetupDialog::PageSetupDialogOption
    \since 4.4

    Used to specify options to the page setup dialog

    \value None None of the options are enabled.
    \value DontUseSheet Do not make the native print dialog a sheet. By default
    on Mac OS X, the native dialog is made a sheet if it has a parent that can
    accept sheets and is visible. Internally, Mac OS X tracks whether
    a printing <em>session</em> and not which particular dialog should have a sheet or not.
    Therefore, make sure this value matches between the page setup dialog and
    the print dialog or you can potentially end up in a modal loop that you can't break.
*/

/*!
    \since 4.4

    Adds the option \a option to the set of enabled options in this dialog.
*/
void QPageSetupDialog::addEnabledOption(PageSetupDialogOption option)
{
    reinterpret_cast<QAbstractPageSetupDialogPrivate *>(d_func())->addEnabledOption(option);
}

/*!
    \since 4.4

    Sets the set of options that should be enabled for the page setup dialog
    to \a options.
*/
void QPageSetupDialog::setEnabledOptions(PageSetupDialogOptions options)
{
    reinterpret_cast<QAbstractPageSetupDialogPrivate *>(d_func())->setEnabledOptions(options);
}

/*!
    \since 4.4

    Returns the set of enabled options in this dialog.
*/
QPageSetupDialog::PageSetupDialogOptions QPageSetupDialog::enabledOptions() const
{
    return reinterpret_cast<const QAbstractPageSetupDialogPrivate *>(d_func())->enabledOptions();
}

/*!
    \since 4.4

    Returns true if the specified \a option is enabled; otherwise returns false
*/
bool QPageSetupDialog::isOptionEnabled(PageSetupDialogOption option) const
{
    return reinterpret_cast<const QAbstractPageSetupDialogPrivate *>(d_func())->isOptionEnabled(option);
}

QT_END_NAMESPACE

#endif
