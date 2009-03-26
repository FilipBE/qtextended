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

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <QtGui/qabstractprintdialog.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_PRINTDIALOG

class QPrintDialogPrivate;
class QPushButton;
class QPrinter;


#if defined (Q_OS_UNIX) && !defined(QTOPIA_PRINTDIALOG) && !defined(Q_WS_MAC)
class QUnixPrintWidgetPrivate;

class Q_GUI_EXPORT QUnixPrintWidget : public QWidget
{
    Q_OBJECT
public:
    QUnixPrintWidget(QPrinter *printer, QWidget *parent = 0);
    ~QUnixPrintWidget();
    void updatePrinter();

private:
    friend class QPrintDialogPrivate;
    friend class QUnixPrintWidgetPrivate;
    QUnixPrintWidgetPrivate *d;
    Q_PRIVATE_SLOT(d, void _q_printerChanged(int))
    Q_PRIVATE_SLOT(d, void _q_btnBrowseClicked())
    Q_PRIVATE_SLOT(d, void _q_btnPropertiesClicked())
};
#endif

class Q_GUI_EXPORT QPrintDialog : public QAbstractPrintDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPrintDialog)
public:
    explicit QPrintDialog(QPrinter *printer, QWidget *parent = 0);
    ~QPrintDialog();

    int exec();
#if defined (Q_OS_UNIX) && !defined(QTOPIA_PRINTDIALOG) && !defined(Q_WS_MAC)
    virtual void accept();
#endif

#if defined (Q_OS_UNIX) && defined (QT3_SUPPORT)
    void setPrinter(QPrinter *, bool = false);
    QPrinter *printer() const;
    void addButton(QPushButton *button);
#endif

#ifdef QTOPIA_PRINTDIALOG
public:
    bool eventFilter(QObject *, QEvent *);
#endif

private:
#ifndef QTOPIA_PRINTDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_chbPrintLastFirstToggled(bool))
#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
    Q_PRIVATE_SLOT(d_func(), void _q_collapseOrExpandDialog())
#endif
# if defined(Q_OS_UNIX) && !defined(QT_NO_MESSAGEBOX)
    Q_PRIVATE_SLOT(d_func(), void _q_checkFields())
# endif
#else // QTOPIA_PRINTDIALOG
    Q_PRIVATE_SLOT(d_func(), void _q_okClicked())
    Q_PRIVATE_SLOT(d_func(),void _q_printerOrFileSelected(QAbstractButton *b))
    Q_PRIVATE_SLOT(d_func(),void _q_paperSizeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_orientSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_pageOrderSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_colorModeSelected(QAbstractButton *))
    Q_PRIVATE_SLOT(d_func(), void _q_setNumCopies(int))
    Q_PRIVATE_SLOT(d_func(), void _q_printRangeSelected(int))
    Q_PRIVATE_SLOT(d_func(), void _q_setFirstPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_setLastPage(int))
    Q_PRIVATE_SLOT(d_func(), void _q_fileNameEditChanged(const QString &text))
#endif // QTOPIA_PRINTDIALOG
    friend class QUnixPrintWidget;
};

#endif // QT_NO_PRINTDIALOG

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPRINTDIALOG_H
