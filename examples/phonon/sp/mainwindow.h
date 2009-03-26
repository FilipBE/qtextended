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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMultiMap>
#include <QMainWindow>

#include <phonon/phononnamespace.h>


class QContent;
namespace Phonon
{
class MediaSource;
}

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();

private slots:
    void documentSelected(QContent const& content);

    void currentSourceChanged(const Phonon::MediaSource &newSource);
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);
    void tick(qint64 time);
    void metaDataChanged(QMultiMap<QString, QString>);
    void seekableChanged(bool);
    void hasVideoChanged(bool);

    void finished();
    void prefinishMarkReached(qint32);
    void aboutToFinish();
    void totalTimeChanged(qint64 length);
    void bufferStatus(int percentFilled);

private:
    MainWindowPrivate* d;
};

#endif  // MAINWINDOW_H
