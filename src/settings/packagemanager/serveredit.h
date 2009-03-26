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

#ifndef SERVEREDIT_H
#define SERVEREDIT_H

#include <QDialog>
#include <QHash>
#include <QLineEdit>
#include <QTextEdit>

#include "ui_serveredit.h"

class ServerItem;

class ServerEdit : public QDialog, private Ui_ServerEditBase
{
    Q_OBJECT
public:
    ServerEdit( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ServerEdit();
    bool wasModified() const;
    QHash<QString,QString> serverList() const;

public slots:
    void accept();

private slots:
    void init();
    void addNewServer();
    void editServer();
    void removeServer();
    void contextMenuShow();

private:
    bool m_modified;
    QAction *editServerAction;
    QStringList serversToRemove;
};

class QLabel;

class ServerEditor : public QDialog
{

Q_OBJECT

public:
    enum Mode {New, ViewEdit};
    enum DialogCode{Modified=QDialog::Accepted + 1,Removed};

    ServerEditor( Mode mode, ServerEdit *parent, const QString &name = "",
                  const QString &url = "" );
    QString name() const;
    QString url() const;
    bool wasModified() const;

public slots:
    virtual void accept();

private slots:
    void removeServer();

private:
    Mode m_mode;
    ServerEdit *m_parent;
    bool m_modified;
    QLabel *m_nameLabel;
    QLabel *m_urlLabel;
    QLineEdit *m_nameLineEdit;
    QTextEdit *m_urlTextEdit;
    QString m_initialName;
    QString m_initialUrl;
};

////////////////////////////////////////////////////////////////////////
/////
///// inline ServerEdit implementations
/////
inline bool ServerEdit::wasModified() const
{
    return m_modified;
}

////////////////////////////////////////////////////////////////////////
/////
///// inline ServerEditor implementations
/////
inline QString ServerEditor::name() const
{
    return m_nameLineEdit->text();
}

inline QString ServerEditor::url() const
{
    return m_urlTextEdit->toPlainText();
}

#endif
