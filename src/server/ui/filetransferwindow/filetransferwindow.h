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

#ifndef FILETRANSFERWINDOW_H
#define FILETRANSFERWINDOW_H

#include <QMainWindow>
#include <QHash>

class QCloseEvent;
class QListView;
class FileTransfer;
class FileTransferListModel;
class QTabWidget;
class TaskManagerEntry;
class QModelIndex;
class QAction;

class FileTransferWindow : public QMainWindow
{
    Q_OBJECT
public:
    FileTransferWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

signals:
    void abortTransfer(int id);

private slots:
    void showWindow();
    void activated(const QModelIndex &index);
    void currentChanged(const QModelIndex &current,
                        const QModelIndex &previous);
    void stopCurrentTransfer();
    void incomingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void outgoingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void transferProgress(int id, qint64 done, qint64 total);
    void transferFinished(int id, bool error, bool aborted);

protected:
    void closeEvent(QCloseEvent *event);

private:
    enum ObjectType {
        VCardObject,
        VCalendarObject,
        OtherObjectType
    };

    void setUpView(QListView *view);
    void setUpTaskConnections();
    void addTransfer(const FileTransfer &transfer);
    void transferStarted(bool incoming, int id, const QString &name, const QString &mimeType, const QString &description);
    ObjectType objectType(const QString &mimeType);

    FileTransferListModel *m_model;
    QTabWidget *m_tabs;
    QListView *m_incomingView;
    QListView *m_outgoingView;
    TaskManagerEntry *m_taskManagerEntry;
    QAction *m_stopAction;
    QHash<int, ObjectType> m_vObjects;
};

#endif
