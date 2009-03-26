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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QStack>

#include <QtopiaApplication>

#include "dirdeleterdialog.h"

#include <QObexFtpClient>
#include <QObexFolderListingEntryInfo>

#define SHOW_CANCEL_BUTTON 1

class DirDeleter : public QObject
{
    Q_OBJECT

public:
    DirDeleter(QObexFtpClient *client, QObject *parent = 0);

    void removeDirectory(const QString &dir);
    void cancel();
    DirDeleterDialog::Result result() const;

signals:
    void finished(bool error);

private slots:
    void commandFinished(int id, bool error);
    void listInfo(const QObexFolderListingEntryInfo &info);
    void clientDone(bool error);

private:
    void cdOrRemoveFiles();

    QObexFtpClient *m_client;
    QStack<QObexFolderListingEntryInfo> m_files;
    QStack<QString> m_directories;
    bool m_bailing;
    DirDeleterDialog::Result m_result;
};

DirDeleter::DirDeleter(QObexFtpClient *client, QObject *parent) : QObject(parent)
{
    m_client = client;
    connect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
            this, SLOT(listInfo(QObexFolderListingEntryInfo)));
    connect(m_client, SIGNAL(commandFinished(int,bool)),
            this, SLOT(commandFinished(int,bool)));

    m_bailing = false;
    m_result = DirDeleterDialog::Success;
}

void DirDeleter::clientDone(bool)
{
    emit finished(m_bailing);
}

void DirDeleter::cdOrRemoveFiles()
{
    if (m_files.size() == 0) {
        m_client->cdUp();
        return;
    }

    QObexFolderListingEntryInfo info = m_files.pop();
    if (info.isFolder()) {
        m_client->cd(info.name());
        m_directories.push(info.name());
    }
    else {
        m_client->remove(info.name());
    }
}

void DirDeleter::commandFinished(int, bool error)
{
    QObexFtpClient::Command command = m_client->currentCommand();

    if (error) {
        m_result = DirDeleterDialog::Failed;
    }

    if (error || m_bailing) {
        disconnect(m_client, SIGNAL(listInfo(QObexFolderListingEntryInfo)),
            this, SLOT(listInfo(QObexFolderListingEntryInfo)));
        disconnect(m_client, SIGNAL(commandFinished(int,bool)),
            this, SLOT(commandFinished(int,bool)));

        int depth = (command == QObexFtpClient::Cd ? m_directories.size() - 1 : m_directories.size());

        if (depth == 0) {
            clientDone(false);
        }
        else {
            m_bailing = true;
            connect(m_client, SIGNAL(done(bool)),
                this, SLOT(clientDone(bool)));

            for (int i = 0; i < depth; i++) {
                m_client->cdUp();
            }
        }

        return;
    }

    if (command == QObexFtpClient::Cd) {
        m_files.clear();
        m_client->list();
    }
    else if (command == QObexFtpClient::List) {
        cdOrRemoveFiles();
    }
    else if (command == QObexFtpClient::CdUp) {
        QString dirToRemove = m_directories.pop();
        m_client->rmdir(dirToRemove);
    }
    else if (command == QObexFtpClient::Remove) {
        cdOrRemoveFiles();
    }
    else if (command == QObexFtpClient::Rmdir) {
        if (m_directories.size())
            m_client->list();
        else
            emit finished(false);
    }
}

void DirDeleter::listInfo(const QObexFolderListingEntryInfo &info)
{
    // Assume that the parent always exists
    if (info.isParent())
        return;

    m_files.push(info);
}

void DirDeleter::cancel()
{
    m_client->clearPendingCommands();
    m_bailing = true;
    m_result = DirDeleterDialog::Canceled;
}

DirDeleterDialog::Result DirDeleter::result() const
{
    return m_result;
}

void DirDeleter::removeDirectory(const QString &dir)
{
    m_client->cd(dir);
    m_directories.push(dir);
}

DirDeleterDialog::DirDeleterDialog(const QString &dir, QObexFtpClient *client, QWidget *parent)
    : QDialog(parent)
{
    m_deleter = new DirDeleter(client, this);

    QVBoxLayout *widgetLayout = new QVBoxLayout;

    QString caption(tr("Removing Folder: "));
    caption.append(dir);
    setWindowTitle(caption);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    m_removingLabel = new QLabel(this);

    m_currentLabel = new QLabel(this);
    labelLayout->addWidget(m_removingLabel);
    labelLayout->addWidget(m_currentLabel);

#ifdef SHOW_CANCEL_BUTTON
    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(tr("Cancel"));
    connect(m_cancelButton, SIGNAL(clicked()),
            this, SLOT(cancel()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
#endif

    widgetLayout->addLayout(labelLayout);

#ifdef SHOW_CANCEL_BUTTON
    widgetLayout->addLayout(buttonLayout);
#endif

    setLayout(widgetLayout);

    m_deleter->removeDirectory(dir);

    connect(m_deleter, SIGNAL(finished(bool)),
            this, SLOT(deleteDone(bool)));
}

DirDeleterDialog::~DirDeleterDialog()
{
    delete m_deleter;
}

void DirDeleterDialog::cancel()
{
    m_deleter->cancel();
}

void DirDeleterDialog::deleteDone(bool error)
{
    if (error)
        reject();
    else
        accept();
}

void DirDeleterDialog::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Back ) {
        cancel();
        event->accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}

DirDeleterDialog::Result
DirDeleterDialog::deleteDirectory(const QString &dir, QObexFtpClient *client, QWidget *parent)
{
    DirDeleterDialog *dialog = new DirDeleterDialog(dir, client, parent);

    QtopiaApplication::execDialog(dialog);

    DirDeleterDialog::Result result = dialog->m_deleter->result();
    delete dialog;

    return result;
}

#include "dirdeleterdialog.moc"
