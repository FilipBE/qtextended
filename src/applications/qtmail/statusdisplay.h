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

#ifndef STATUSDISPLAY_H
#define STATUSDISPLAY_H

#include <QProgressBar>


// A QProgressBar and status label combined. No percentage is shown, as
// that's represented by the bar alone.
//
class StatusProgressBar : public QProgressBar 
{
    Q_OBJECT

public:
    StatusProgressBar( QWidget* parent = 0 );
    virtual ~StatusProgressBar();

    QSize sizeHint() const;

    void setText(const QString& s);

    QString text() const;

private:
    QString txt;
    mutable bool txtchanged;
};

// Implements some policy for the display of status and progress
class StatusDisplay : public StatusProgressBar
{
    Q_OBJECT

public:
    StatusDisplay(QWidget* parent = 0);

public slots:
    void showStatus(bool visible);
    void displayStatus(const QString& txt);
    void displayProgress(uint value, uint range);
    void clearStatus();

private:
    bool suppressed;
};

#endif
