/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QTOPIAINPUTDIALOG_P_H
#define QTOPIAINPUTDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QDialog>
#include <QLineEdit>
#include <QtopiaApplication>

#include <qtopiaglobal.h>

class QtopiaInputDialogPrivate;
class QDate;
class QTime;

class QTOPIA_EXPORT QtopiaInputDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QtopiaInputDialog)
private:
    QtopiaInputDialog(QWidget *parent, const QString &title, const QString &label, QWidget *input);

public:
    // TODO: these should be removed with the new API
    QtopiaInputDialog(QWidget *parent, const QString &title, QWidget* label, QWidget *input);
    ~QtopiaInputDialog();

    static QString getText(QWidget *parent, const QString &title, const QString &label, QLineEdit::EchoMode mode = QLineEdit::Normal,
                           QtopiaApplication::InputMethodHint hint = QtopiaApplication::Words, const QString &hintParam = QString(),
                           const QString &text = QString(), bool *ok = 0);

    static QString getMultiLineText(QWidget *parent, const QString &title, const QString &label,
                           QtopiaApplication::InputMethodHint hint = QtopiaApplication::Words, const QString &hintParam = QString(),
                           const QString &text = QString(), bool *ok = 0);

    //possible options: editable, multiselect
    static QString getItem(QWidget *parent, const QString &title, const QString &label, const QStringList &list,
                           int current = 0, bool *ok = 0);

    //possible options: calendar vs. spinbox (use near/far distinction?)
    static QDate getDate(QWidget *parent, const QString &title, const QString &label, const QDate &date,
                         const QDate &minDate, const QDate &maxDate, bool *ok = 0);

    static QTime getTime(QWidget *parent, const QString &title, const QString &label, const QTime &time,
                         const QTime &minTime, const QTime &maxTime, bool *ok = 0);

    static int getInteger(QWidget *parent, const QString &title, const QString &label, int value = 0,
                          int minValue = -2147483647, int maxValue = 2147483647, int step = 1, bool *ok = 0);

private:
    Q_DISABLE_COPY(QtopiaInputDialog)

    Q_PRIVATE_SLOT(d_func(), void _q_preedit(QString text))
    Q_PRIVATE_SLOT(d_func(), void _q_submitWord(QString word))
    Q_PRIVATE_SLOT(d_func(), void _q_erase())
};

#endif
