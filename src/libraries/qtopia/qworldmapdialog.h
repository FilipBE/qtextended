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

#ifndef QWORLDMAPDIALOG_H
#define QWORLDMAPDIALOG_H

#include <QWorldmap>
#include <QDialog>
#include <QTimeZone>

class QTOPIA_EXPORT QWorldmapDialog : public QDialog
{
    Q_OBJECT
public:
    QWorldmapDialog( QWidget* parent = 0 );
    ~QWorldmapDialog();

    void setZone( const QTimeZone& zone );
    QTimeZone selectedZone() const;
    int realResult;

signals:
    void zoneSelected(const QString& zone);

protected:
    void showEvent(QShowEvent *);

private:
    QWorldmap* mMap;
    QTimeZone mZone;
    bool reallyDone;
    void doneIt(int);

private slots:
    void selected();
    void selected( const QTimeZone& zone );
    void cancelled();
    void keyPressEvent( QKeyEvent *ke );
};

#endif
