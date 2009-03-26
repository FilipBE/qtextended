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


#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qdatetime.h>
#include <qdialog.h>

class FindWidget;

class FindDialogPrivate;
class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog( const QString &appName, QWidget *parent = 0 );
    ~FindDialog();

    QString findText() const;
    void setUseDate( bool show );
    void setDate( const QDate &dt );

public slots:
    void slotNotFound();
    void slotWrapAround();

signals:
    void signalFindClicked( const QString &txt, bool caseSensitive,
                            bool backwards, int category );
    void signalFindClicked( const QString &txt, const QDate &dt,
                            bool caseSensitive, bool backwards, int category );

private:
    FindWidget *fw;
    FindDialogPrivate *d;
};

#endif
