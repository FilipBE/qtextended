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

#ifndef EMAILDIALOGPHONE_H
#define EMAILDIALOGPHONE_H

#include <QListWidget>
#include <QDialog>
#include <QStringList>
#include <QPixmap>
#include <QLineEdit>

class QAction;
class QGroupBox;

class EmailDialogList;
class EmailDialogListItem : public QListWidgetItem
{
    friend class EmailDialogList;
public:
    EmailDialogListItem( EmailDialogList *parent, const QString&, int after );
};

class EmailDialogList : public QListWidget
{
    friend class EmailLineEdit;
    Q_OBJECT
public:
    EmailDialogList( QWidget *parent, bool readonly );

    void setEmails( const QString &def, const QStringList &em );

    QString defaultEmail() const;
    QStringList emails() const;

signals:
    void editItem();
    void newItem();

public slots:
    void addEmail(const QString& email);
    void updateEmail(const QString& email);
    void deleteEmail();

protected slots:
    void setAsDefault();
    void editItem(QListWidgetItem* i);
    void updateMenus();

    void moveUp();
    void moveDown();

private:
    QIcon mDefaultPix;
    QIcon mNormalPix;
    int mDefaultIndex;
    QAction *mSetDefaultAction, *mDeleteAction, *mNewAction;
    bool readonly;
    QListWidgetItem *newItemItem;
};

class EmailLineEdit : public QLineEdit
{
    friend class EmailDialogList;
    Q_OBJECT
public:
    EmailLineEdit( QWidget *parent, const char *name = 0 );

signals:
    void setAsDefault();
    void moveUp();
    void moveDown();

protected:
    void keyPressEvent( QKeyEvent *ke );
};

class EmailDialog : public QDialog
{
    Q_OBJECT
public:
    EmailDialog( QWidget *parent, bool readonly = false);
    ~EmailDialog();

    void setEmails(const QString &def, const QStringList &em);

    QString defaultEmail() const;
    QStringList emails() const;

    QString selectedEmail() const;
protected:
    void showEvent( QShowEvent *e );
    bool eventFilter( QObject *o, QEvent *e );
    void addEmail(const QString& email);

protected slots:
    void currentChanged( QListWidgetItem* current );
    void updateCurrentText( );
    void edit();
    void newEmail();

private:
    EmailDialogList *mList;
    QGroupBox *mEditBox;
    EmailLineEdit *mEdit;
    QString mSelected;
    bool mCreatingEmail;
};

#endif
