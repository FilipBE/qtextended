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

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaSql>
#include <QSqlQuery>
#define private public
#include <QMailFolderId>
#undef private
#include <QMailStore>
#include <QMailFolder>


//TESTED_CLASS=QMailStore
//TESTED_FILES=src/libraries/qtopiamail/qmailstore.cpp

/*
    Unit test for QMailStore class.
    This class tests that QMailStore correctly handles addition, updates, removal, counting and 
    querying of QMailMessages and QMailFolders.
*/
class tst_QMailStore : public QObject
{
    Q_OBJECT

public:
    tst_QMailStore();
    virtual ~tst_QMailStore();

private slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();
    void addFolder();
    void addMessage();
    void updateFolder();
    void updateMessage();
    void removeFolder();
    void removeMessage();
    void queryFolders();
    void queryMessages();
    void countFolders();
    void countMessages();
    void folder();
    void message();
    void messageHeader();

private:
    bool folderExists(unsigned int id, const QString& name, unsigned int parentId);
    void removeFolder(unsigned int id);
    void recursivelyRemovePath(QString const&);
};

QTEST_APP_MAIN( tst_QMailStore, QtopiaApplication )
#include "tst_qmailstore.moc"

tst_QMailStore::tst_QMailStore()
{

}

tst_QMailStore::~tst_QMailStore()
{
}

void tst_QMailStore::initTestCase()
{
}

void tst_QMailStore::cleanup()
{
    //delete all database entries

    QSqlDatabase db = QtopiaSql::instance()->applicationSpecificDatabase("qtopiamail");
    QSqlQuery sql(db);
    sql.exec("DELETE * FROM mailfolders");
    sql.exec("DELETE * FROM mailmessages");

    //delete all mail files

    QString mailpath = Qtopia::applicationFileName("qtopiamail","mail");
    recursivelyRemovePath(mailpath);
}

void tst_QMailStore::cleanupTestCase()
{
    //remove everything from the qtopiamail data directory
    //as this is where the store holds all mails

    QString mailpath = Qtopia::applicationFileName("qtopiamail","");
    recursivelyRemovePath(mailpath);
}

void tst_QMailStore::addFolder()
{
    //root folder

    QMailFolder newFolder("new folder 1");
    QVERIFY(QMailStore::instance()->addFolder(&newFolder));
    QVERIFY(newFolder.id().isValid());
    QVERIFY(folderExists(newFolder.id().toULongLong(),"new folder 1",0));

    //root folder with no name

    QMailFolder newFolder2("");
    QVERIFY(QMailStore::instance()->addFolder(&newFolder2));
    QVERIFY(newFolder2.id().isValid());
    QVERIFY(folderExists(newFolder2.id().toULongLong(),"",0));

    //root folder with valid parent

    QMailFolder newFolder3("new folder 3",newFolder2.id());
    QVERIFY(QMailStore::instance()->addFolder(&newFolder3));
    QVERIFY(newFolder3.id().isValid());
    QVERIFY(folderExists(newFolder3.id().toULongLong(),"new folder 3",newFolder2.id().toULongLong()));

    //delete root folder 

    removeFolder(newFolder3.id().toULongLong());
    QVERIFY(!folderExists(newFolder3.id().toULongLong(),"new folder 3",newFolder2.id().toULongLong()));

    //root folder with invalid parent

    QMailFolder newFolder4("new folder 4",newFolder3.id());
    QVERIFY(!QMailStore::instance()->addFolder(&newFolder4));
    QVERIFY(!newFolder4.id().isValid());
    QVERIFY(!folderExists(newFolder4.id().toULongLong(),"new folder 4", newFolder3.id().toULongLong()));

    //root folder with no name and invalid parent

    QMailFolder newFolder5("new folder 5",newFolder3.id());
    QVERIFY(!QMailStore::instance()->addFolder(&newFolder5));
    QVERIFY(!newFolder5.id().isValid());
    QVERIFY(!folderExists(newFolder4.id().toULongLong(),"new folder 5",newFolder3.id().toULongLong()));
}

void tst_QMailStore::addMessage()
{
   
}

void tst_QMailStore::updateFolder()
{
    //update an existing folder with a new name

    QMailFolder newFolder("new folder 1");
    QVERIFY(QMailStore::instance()->addFolder(&newFolder));
    newFolder.setName("newer folder!!");

    QVERIFY(QMailStore::instance()->updateFolder(&newFolder));
    QVERIFY(folderExists(newFolder.id().toULongLong(),"newer folder!!",0));
    QVERIFY(!folderExists(newFolder.id().toULongLong(),"new folder 1",0));

    //update existing folder with empty name

    newFolder.setName("");

    QVERIFY(QMailStore::instance()->updateFolder(&newFolder));
    QVERIFY(folderExists(newFolder.id().toULongLong(),"",0));
    QVERIFY(!folderExists(newFolder.id().toULongLong(),"new folder!!",0));
    QVERIFY(!folderExists(newFolder.id().toULongLong(),"new folder 1",0));

    //update a folder that does not exist in the db

    QMailFolder bogusFolder("does not exist");
    QVERIFY(!QMailStore::instance()->updateFolder(&bogusFolder)); 
    QVERIFY(!folderExists(bogusFolder.id().toULongLong(),"does not exist",0));

    //update a folder with an invalid parent

    QMailFolder newFolder2("new folder 2");
    QVERIFY(QMailStore::instance()->addFolder(&newFolder2));
    QMailFolder newFolder3("new folder 3",newFolder2.id());
    QVERIFY(QMailStore::instance()->addFolder(&newFolder3));
    QMailFolder newFolder4("new folder 4");
    QVERIFY(QMailStore::instance()->addFolder(&newFolder4));

    removeFolder(newFolder3.id().toULongLong());
    
    newFolder4.setParentId(newFolder3.id());
    QVERIFY(!QMailStore::instance()->updateFolder(&newFolder4));
    QVERIFY(!folderExists(newFolder4.id().toULongLong(),"new folder 4",newFolder3.id().toULongLong()));
    QVERIFY(folderExists(newFolder4.id().toULongLong(),"new folder 4",0));

    //update a folder to valid parent

    newFolder4.setParentId(newFolder2.id());
    QVERIFY(QMailStore::instance()->updateFolder(&newFolder4));
    QVERIFY(!folderExists(newFolder4.id().toULongLong(),"new folder 4",0));
    QVERIFY(folderExists(newFolder4.id().toULongLong(),"new folder 4",newFolder2.id().toULongLong()));

    //updata a folder to a root folder

    newFolder4.setParentId(QMailFolderId());
    QVERIFY(QMailStore::instance()->updateFolder(&newFolder4));
    QVERIFY(folderExists(newFolder4.id().toULongLong(),"new folder 4",0));
    QVERIFY(!folderExists(newFolder4.id().toULongLong(),"new folder 4",newFolder2.id().toULongLong()));

    //update a folder with a reference to itself

    newFolder2.setParentId(newFolder2.id());
    QVERIFY(!QMailStore::instance()->updateFolder(&newFolder2));

}

void tst_QMailStore::updateMessage()
{

}

void tst_QMailStore::removeFolder()
{
    //remove a folder that does not exist

    QVERIFY(QMailStore::instance()->removeFolder(QMailFolderId()));

    //remove a root folder with some mails in it
    //remove a child folder with mails in it
    //remove a folder that has mails and child folders with mails

}

void tst_QMailStore::removeMessage()
{

}

void tst_QMailStore::queryFolders()
{

}

void tst_QMailStore::queryMessages()
{

}

void tst_QMailStore::countFolders()
{

}

void tst_QMailStore::countMessages()
{

}

void tst_QMailStore::folder()
{
   

}

void tst_QMailStore::message()
{

}

void tst_QMailStore::messageHeader()
{

}

//these functions should not fail

bool tst_QMailStore::folderExists(unsigned int id, const QString& name, unsigned int parentId)
{
    //check existance of folder record

    QSqlDatabase db = QtopiaSql::instance()->applicationSpecificDatabase("qtopiamail");
    QSqlQuery sql(db);

    sql.prepare("SELECT id FROM mailfolders WHERE id=\? AND name=\? AND parentid=\?");

    sql.addBindValue(id);
    sql.addBindValue(name);
    sql.addBindValue(parentId);
    sql.exec();
    return sql.first();

}

void tst_QMailStore::removeFolder(unsigned int id)
{
    //remove a folder record

    QSqlDatabase db = QtopiaSql::instance()->applicationSpecificDatabase("qtopiamail");
    QSqlQuery sql(db);
    sql.prepare("DELETE FROM mailfolders WHERE id=\?");
    sql.addBindValue(id);
    sql.exec();
}

void tst_QMailStore::recursivelyRemovePath(QString const &path)
{
    QFileInfo fi(path);
    if (!fi.isDir()) {
        QFile::remove(path);
        return;
    }

    QDir dir(path);
    foreach (QString file, dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot)) {
        recursivelyRemovePath(path + "/" + file);
    }
    dir.setPath("/");
    dir.rmpath(path);
}

