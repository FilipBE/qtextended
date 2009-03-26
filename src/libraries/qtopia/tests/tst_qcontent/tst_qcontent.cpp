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

#include <qtopiasql.h>
#include <QContent>
#include <QDebug>
#include <QContentSet>
#include <QtopiaApplication>
#include <QTemporaryFile>
#include <QMimeType>

#include <QFileSystem>
#include <QStorageMetaInfo>
#include <QTest>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=QContent
//TESTED_FILES=src/libraries/qtopia/qcontent.cpp

Q_DECLARE_METATYPE(QContent::Property);

QString tempPath_noslash()
{
    QString ret = QDir::tempPath();
    if (ret.endsWith("/")) ret = ret.left(ret.size()-1);
    return ret;
}

//
// The test case
//
class tst_QContent: public QObject
{
    Q_OBJECT
protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    // These are the test functions.  Each one should return
    // the database to the state created by initTestCase().

    // Tests for the static methods of QContent
    void install_and_uninstall();

    // Tests for the non-static methods of QContent.
    //
    void tst_emptyCtor();
    void tst_QStringCtor();
    void tst_QFileInfoCtor();
    void tst_QContentIdCtor();
    void tst_copyCtor();
    void tst_categories();
    void tst_comment();
    void tst_commit();
    void tst_copyContent();
    void tst_copyTo();
    void tst_drmState();
    void tst_executableName();
    void tst_fileKnown();
    void tst_fileName();
    void tst_icon();
    void tst_iconName();
    void tst_id();
    void tst_isDocument();
    void tst_isNull();
    void tst_isPreloaded();
    void tst_isValid();
    void tst_lastUpdated();
    void tst_media();
    void tst_mimeTypeIcons();
    void tst_mimeTypePermissions();
    void tst_mimeTypes();

    void tst_removeFiles();     // used by moveTo, rename

    void tst_moveTo();
    void tst_rename();
    void tst_name();
    void tst_permissions();
    void tst_property();
    void tst_rights();
    void tst_role_data();
    void tst_role();
    void tst_size();
    void tst_type();
    void tst_untranslatedName();
    void tst_propertyKey_data();
    void tst_propertyKey();
    void tst_propertyProperty_data();
    void tst_propertyProperty();

    void tst_open_load_save();
    void tst_serialize_deserialize();

    void tst_copyQContent();

    void tst_dashes();

private:
    QString extensionForMimeType(QString const&);

    QTemporaryFile tmpFile;
    QStringList filesToDelete;
    QString documentsPath;
    QString plainTextExtension;
    bool deleteDocumentsDir;
};

#define COMPARE_METHODS(method) \
  qCompare(t1.method(), t2.method(), \
           QString("%1.%2()").arg(actual).arg(#method).toLatin1(), \
           QString("%1.%2()").arg(expected).arg(#method).toLatin1(), file, line)

#define COMPARE_METHODS_WITH_ARG(method, argument) \
  qCompare(t1.method(argument), t2.method(argument), \
           QString("%1.%2()").arg(actual).arg(#method).toLatin1(), \
           QString("%1.%2()").arg(expected).arg(#method).toLatin1(), file, line)

namespace QTest {
    template<>
    inline bool qCompare(QContent const &t1, QContent const &t2,
                        const char *actual, const char *expected, const char *file, int line)
    {
        bool result = COMPARE_METHODS(categories)
                && COMPARE_METHODS(comment)
                && COMPARE_METHODS(drmState)
                && COMPARE_METHODS(executableName)
                && COMPARE_METHODS(fileKnown)
                && COMPARE_METHODS(fileName)
                && COMPARE_METHODS(icon)
                && COMPARE_METHODS(iconName)
                && COMPARE_METHODS(id)
                && COMPARE_METHODS(isDocument)
                && COMPARE_METHODS(isNull)
                && COMPARE_METHODS(isPreloaded)
                && COMPARE_METHODS_WITH_ARG(isValid, true)
                && COMPARE_METHODS_WITH_ARG(isValid, false)
                && COMPARE_METHODS(lastUpdated)
                && COMPARE_METHODS(linkFile)
                && COMPARE_METHODS(linkFileKnown)
                && COMPARE_METHODS(media)
                && COMPARE_METHODS(mimeTypeIcons)
                && COMPARE_METHODS(mimeTypePermissions)
                && COMPARE_METHODS(mimeTypes)
                && COMPARE_METHODS(name)
                && COMPARE_METHODS(permissions)
                && COMPARE_METHODS(role)
                && COMPARE_METHODS(size)
                && COMPARE_METHODS(type)
                && COMPARE_METHODS(untranslatedName)
                && COMPARE_METHODS(usageMode);
        if(result == false)
            return false;
        // todo property and propertyKey tests
        for(QContent::Property property=QContent::Album; property < QContent::Version; property=QContent::Property((int)property+1))
        {
            result &= COMPARE_METHODS_WITH_ARG(property, property);
            if(result == false)
                return false;
        }
        return true;
   }

} // namespace QTest


QTEST_APP_MAIN( tst_QContent, QtopiaApplication )

#include "tst_qcontent.moc"

const QString testData(QLatin1String("test"));

void tst_QContent::initTestCase()
{
    //QtopiaSql::loadConfig(QString ("QSQLITE"), QCoreApplication::applicationDirPath () + "/qtopia_db.sqlite", QString ());
    tmpFile.setFileTemplate(tempPath_noslash() + "/tst_qcontentXXXXXXX.tmp");
    QVERIFY(tmpFile.open());
    QVERIFY(tmpFile.write(testData.toLatin1()) == testData.length());
    tmpFile.flush();
    QVERIFY(QFileInfo(tmpFile.fileName()).exists());
    filesToDelete.append(tmpFile.fileName());
    deleteDocumentsDir=false;
    QString documentsPath=QFileSystem::documentsFileSystem().documentsPath();
    QVERIFY(!documentsPath.isEmpty());
    if(!QDir().exists(documentsPath))
    {
        QDir().mkpath(documentsPath);
        deleteDocumentsDir=true;
    }
    plainTextExtension = QMimeType("text/plain").extension();
}

void tst_QContent::cleanupTestCase()
{
    /*QString dbPath = QCoreApplication::applicationDirPath () + "\qtopia_db.sqlite";
    if(QFile::exists(dbPath))
        QFile::remove(dbPath);*/
    QContent::uninstall(QContent(tmpFile.fileName()).id());
    tmpFile.close();
    foreach(QString filename, filesToDelete)
    {
        QFile::remove(filename);
        if(QFile::exists(filename))
            qWarning() << "Failed to clean up temporary file:" << filename;
    }
    if(deleteDocumentsDir==true)
        QDir().rmpath(documentsPath);
}

void tst_QContent::install_and_uninstall()
{
    QFileInfo fi(tmpFile.fileName());
    QContent::install(fi);
    QContentFilter filter1(QContentFilter::Directory, fi.dir().absolutePath());
    filter1 &= QContentFilter(QContentFilter::FileName, fi.fileName());
    QContentSet contentset(filter1);
    QVERIFY(contentset.count() == 1);
    QContent::uninstall(contentset.items().first().id());
    contentset.clearFilter();
    contentset.setCriteria(filter1);
    QVERIFY(contentset.count() == 0);
}

void tst_QContent::tst_emptyCtor()
{
    QContent content;
    QCOMPARE(content.id(), QContent::InvalidId);
}

void tst_QContent::tst_QStringCtor()
{
    QContent content(tmpFile.fileName());
    QCOMPARE(content.fileName(), tmpFile.fileName());
    QVERIFY(content.id() != QContent::InvalidId);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_QFileInfoCtor()
{
    QContent content(QFileInfo(tmpFile.fileName()));
    QCOMPARE(content.fileName(), tmpFile.fileName());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_QContentIdCtor()
{
    QContent content(QFileInfo(tmpFile.fileName()));
    QContent content2(content.id());
    QCOMPARE(content2.fileName(), tmpFile.fileName());
    QCOMPARE(content.id(), content2.id());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_copyCtor()
{
    QContent content(tmpFile.fileName());
    QContent content2(content);
    QCOMPARE(content2.fileName(), tmpFile.fileName());
    QCOMPARE(content.id(), content2.id());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_categories()
{
    QLatin1String test1("test1"), test2("test2");
    QContent content(tmpFile.fileName(), false);
    content.setCategories(QStringList() << test1 << test2);
    QVERIFY(content.categories().count() == 2);
    QVERIFY(content.categories().first() == test1);
    QVERIFY(content.categories().first() == test1);
    content.commit();
    QVERIFY(content.categories().count() == 2);
    QVERIFY(content.categories().first() == test1);
    QVERIFY(content.categories().first() == test1);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_comment()
{
    QLatin1String comment("test comment"), comment2("test comment2");
    QContent content(tmpFile.fileName(), false);
    content.setComment(comment);
    QCOMPARE(content.comment(), comment);
    content.commit();
    QCOMPARE(content.comment(), comment);
    content.setComment(comment2);
    QCOMPARE(content.comment(), comment2);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_commit()
{
    QLatin1String comment("test comment"), comment2("test comment2");
    QContent content(tmpFile.fileName(), false);
    QFileInfo fi(tmpFile.fileName());
    QContentFilter filter1(QContentFilter::Directory, fi.dir().absolutePath());
    filter1 &= QContentFilter(QContentFilter::FileName, fi.fileName());
    
    QVERIFY(content.id() == QContent::InvalidId);
    QContentSet contentset(filter1);
    QCOMPARE(contentset.count(), 0);
    
    content.commit();
    
    QVERIFY(content.id() != QContent::InvalidId);
    contentset.clearFilter();
    contentset.setCriteria(filter1);
    QCOMPARE(contentset.count(), 1);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_copyContent()
{
    QTemporaryFile tmpFile2(tempPath_noslash() + "/tst_qcontentXXXXXXX.tmp");
    QVERIFY(tmpFile2.open());
    QContent content(tmpFile.fileName());
    QContent content2(tmpFile2.fileName());
    filesToDelete.append(tmpFile2.fileName());
    tmpFile2.close();
    QFile::remove(content2.fileName());
    QVERIFY(QFile(content2.fileName()).exists() == false);
    
    QVERIFY(content2.copyContent(content));
    QFile file1(content.fileName()), file2(content.fileName());
    QVERIFY(file1.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(file2.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(file1.size(), file2.size());
    QVERIFY(file2.size() == testData.length());
    QByteArray line1 = file1.readAll();
    QByteArray line2 = file2.readAll();
    QVERIFY(line1 == line2);
    QFile::remove(content2.fileName());
    QContent::uninstall(content.id());
    QContent::uninstall(content2.id());
}

void tst_QContent::tst_copyTo()
{
    QTemporaryFile tmpFile2(tempPath_noslash() + "/tst_qcontentXXXXXXX.tmp");
    QVERIFY(tmpFile2.open());
    QFileInfo fi(tmpFile.fileName()), fi2(tmpFile2.fileName());
    filesToDelete.append(tmpFile2.fileName());
    tmpFile2.close();
    QFile::remove(fi2.filePath());

    QContentFilter filter1(QContentFilter::Directory, fi2.dir().absolutePath());
    filter1 &= QContentFilter(QContentFilter::FileName, fi2.fileName());
    QVERIFY(QFile(fi2.filePath()).exists() == false);
    QContentSet contentset(filter1);
    QCOMPARE(contentset.count(), 0);
    
    QContent content(fi.filePath());
    QVERIFY(content.copyTo(fi2.filePath()));
    QVERIFY(fi2.exists() == true);

    contentset.clearFilter();
    contentset.setCriteria(filter1);
    QCOMPARE(contentset.count(), 1);
    QContent::uninstall(content.id());
    QContent::uninstall(QContent(fi2.filePath()).id());
    QFile::remove(fi2.filePath());
}

void tst_QContent::tst_drmState()
{
    QContent content(tmpFile.fileName(), false);
    //todo: further testing here
    QCOMPARE(content.drmState(), QContent::Unprotected);
    content.commit();
    QCOMPARE(content.drmState(), QContent::Unprotected);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_executableName()
{
    QMimeType mtype("text/*");
    QContent tmp=QMimeType::defaultApplicationFor(mtype);
    QVERIFY(tmp.id() != QContent::InvalidId);
    QVERIFY(tmp.executableName().isEmpty() == false);
}

void tst_QContent::tst_fileKnown()
{
    QCOMPARE(QContent().fileKnown(), false);
    QCOMPARE(QContent(tmpFile.fileName(), false).fileKnown(), true);
}

void tst_QContent::tst_fileName()
{
    QCOMPARE(QContent().fileName().isEmpty(), true);
    QCOMPARE(QContent(tmpFile.fileName(), false).fileName().isEmpty(), false);
}

void tst_QContent::tst_icon()
{
    QMimeType mtype("text/*");
    QContent tmp=QMimeType::defaultApplicationFor(mtype);
    QVERIFY(tmp.id() != QContent::InvalidId);
    QVERIFY(tmp.icon().isNull() == false);
}

void tst_QContent::tst_iconName()
{
    QMimeType mtype("text/*");
    QContent tmp=QMimeType::defaultApplicationFor(mtype);
    QVERIFY(tmp.id() != QContent::InvalidId);
    QVERIFY(tmp.iconName().isEmpty() == false);
}

void tst_QContent::tst_id()
{
    QVERIFY(QContent().id() == QContent::InvalidId);
    QContent content(tmpFile.fileName(), false);
    QVERIFY(content.id() == QContent::InvalidId);
    content.commit();
    QVERIFY(content.id() != QContent::InvalidId);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_isDocument()
{
    QContent content(tmpFile.fileName(), false);
    QVERIFY(content.isDocument());
    content.commit();
    QVERIFY(content.isDocument());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_isNull()
{
    QVERIFY(QContent().isNull());
    QContent content(tmpFile.fileName(), false);
    QVERIFY(!content.isNull());
    content.commit();
    QVERIFY(!content.isNull());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_isPreloaded()
{
    // don't know how to test this...
}

void tst_QContent::tst_isValid()
{
    QCOMPARE(QContent().isValid(), false);
    QContent content(tmpFile.fileName(), false);
    QCOMPARE(content.isValid(), true);
    content.commit();
    QCOMPARE(content.isValid(), true);
    QContent::uninstall(content.id());
}

void tst_QContent::tst_lastUpdated()
{
    QContent content(tmpFile.fileName(), false);
    QVERIFY(!content.lastUpdated().isNull());
    content.commit();
    QVERIFY(!content.lastUpdated().isNull());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_media()
{
    QTemporaryFile tmpDocFile(QFileSystem::documentsFileSystem().documentsPath()+"/tmp_fileXXXXXX.txt");
    tmpDocFile.open();
    filesToDelete.append(tmpDocFile.fileName());
    QContent content(tmpDocFile);
    QFileSystem const* fs = QStorageMetaInfo::instance()->fileSystemOf(tmpDocFile.fileName());
    QVERIFY( fs );
    QCOMPARE(content.media(), fs->path());
    QContent::uninstall(content.id());
}

void tst_QContent::tst_mimeTypeIcons()
{
    // assumes .txt is going to be supported as a text mimetype
    QContent textapp=QMimeType(".txt").application();
    QVERIFY(textapp.mimeTypeIcons().count()!=0);
    QVERIFY(!textapp.mimeTypeIcons().first().isEmpty());
}

void tst_QContent::tst_mimeTypePermissions()
{
    QContent textapp=QMimeType(".txt").application();
    QVERIFY(textapp.mimeTypePermissions().count()!=0);
}

void tst_QContent::tst_mimeTypes()
{
    QContent textapp=QMimeType(".txt").application();
    QVERIFY(textapp.mimeTypes().count()!=0);
}

void tst_QContent::tst_moveTo()
{
    QString firstfile=QFileSystem::documentsFileSystem().documentsPath()+"/move_test.txt";
    QString secondfile=QFileSystem::documentsFileSystem().documentsPath()+"/move_test2.txt";
    filesToDelete.append(firstfile);
    filesToDelete.append(secondfile);

    QFile tmpDocFile(firstfile);
    tmpDocFile.open(QIODevice::ReadWrite|QIODevice::Truncate);
    tmpDocFile.write("test");
    tmpDocFile.close();
    QVERIFY(QFile::exists(firstfile));
    QVERIFY(!QFile::exists(secondfile));
    QContent content1(tmpDocFile);
    QVERIFY(content1.moveTo(secondfile));
    QVERIFY(!QFile::exists(firstfile));
    QVERIFY(QFile::exists(secondfile));
    QContent(secondfile).removeFiles();
}

void tst_QContent::tst_rename()
{
    QString fileName1 = QFileSystem::documentsFileSystem().documentsPath() + "/rename_test1.txt";
    QString fileName2 = QFileSystem::documentsFileSystem().documentsPath() + "/rename_test2.txt";
    QString fileName3 = QFileSystem::documentsFileSystem().documentsPath() + "/rename_test3." + plainTextExtension;

    filesToDelete.append(fileName3);
    filesToDelete.append(fileName2);
    filesToDelete.append(fileName1);

    {
        QFile file(fileName1);
        file.open(QIODevice::ReadWrite | QIODevice::Truncate);
        file.write("test");
        file.close();
    }

    QVERIFY(QFile::exists(fileName1));
    QVERIFY(!QFile::exists(fileName2));
    QVERIFY(!QFile::exists(fileName3));

    QContent content(fileName1);

    QVERIFY(content.rename("rename_test2.txt"));

    QCOMPARE(content.name(), QString("rename_test2"));
    QCOMPARE(content.fileName(), fileName2);
    QVERIFY(!QFile::exists(fileName1));
    QVERIFY(QFile::exists(fileName2));

    filesToDelete.removeLast();

    QVERIFY(content.rename("rename test3"));

    QCOMPARE(content.name(), QString("rename test3"));
    QCOMPARE(content.fileName(), fileName3);
    QVERIFY(!QFile::exists(fileName2));
    QVERIFY(QFile::exists(fileName3));

    filesToDelete.removeLast();

    content.removeFiles();

    QVERIFY(!QFile::exists(fileName3));
    filesToDelete.removeLast();
}

void tst_QContent::tst_name()
{
    QContent textapp=QMimeType(".txt").application();
    QVERIFY(!textapp.name().isEmpty());
    //TODO: test some of the name->filename munging algorithms also
}

void tst_QContent::tst_permissions()
{
    //TODO: need to write some testing here.
}

void tst_QContent::tst_property()
{
    {
        QContent contentWrite(tmpFile);
        contentWrite.setProperty("testkey", "testvalue", "testgroup");
        contentWrite.setProperty("testkey", "testvalue2");
        contentWrite.commit();
    }
    {
        QContent contentRead(tmpFile);
        QVERIFY(contentRead.property("testkey", "testgroup") == "testvalue");
        QVERIFY(contentRead.property("testkey") == "testvalue2");
    }
    {
        QContent contentWrite(tmpFile);
        contentWrite.setProperty("testkey", "", "testgroup");
        contentWrite.setProperty("testkey", "");
        contentWrite.commit();
    }
    {
        QContent contentRead(tmpFile);
        QVERIFY(contentRead.property("testkey", "testgroup") == "");
        QVERIFY(contentRead.property("testkey") == "");
    }
}

void tst_QContent::tst_removeFiles()
{
    QString filename=QFileSystem::documentsFileSystem().documentsPath()+"/move_test.txt";
    filesToDelete.append(filename);
    QVERIFY(!QFile::exists(filename));
    QFile file(filename);
    file.open(QIODevice::ReadWrite|QIODevice::Truncate);
    file.write("test");
    file.close();
    QVERIFY(QFile::exists(filename));
    QContent content(filename);
    content.removeFiles();
    QVERIFY(!QFile::exists(filename));
}

void tst_QContent::tst_rights()
{
    //TODO: need to write some testing here.
}

void tst_QContent::tst_role_data()
{
    QTest::addColumn<QContent::Role>("role");
    // Can't test UnknownUsage, as it's an "unknown type", and therefor can't be set once another type has been set on the content
    // QTest::newRow("QContent::UnknownUsage") << QContent::UnknownUsage;
    QTest::newRow("QContent::Document") << QContent::Document;
    QTest::newRow("QContent::Data") << QContent::Data;
    QTest::newRow("QContent::Application") << QContent::Application;
    QTest::newRow("QContent::Folder") << QContent::Folder;
}

void tst_QContent::tst_role()
{
    QFETCH( QContent::Role, role );
    {
        QContent contentWrite(tmpFile);
        contentWrite.setRole(role);
        contentWrite.commit();
    }
    {
        QContent contentRead(tmpFile);
        QCOMPARE(contentRead.role(), role);
    }
}

void tst_QContent::tst_size()
{
    QContent content(tmpFile);
    QCOMPARE(tmpFile.size(), content.size());
}

void tst_QContent::tst_type()
{
    QContent textapp=QMimeType(".txt").application();
    QCOMPARE(textapp.type(), QLatin1String("application/x-executable"));
    QContent content(tmpFile);
    QCOMPARE(QMimeType(QFileInfo(content.fileName()).completeSuffix()).id(), content.type());
}

void tst_QContent::tst_untranslatedName()
{
    QContent textapp=QMimeType(".txt").application();
    QCOMPARE(textapp.untranslatedName(), QLatin1String("Notes"));
}

void tst_QContent::tst_propertyKey_data()
{
    QTest::addColumn<QContent::Property>("property");
    QTest::addColumn<QString>("result");
    QTest::newRow("QContent::Album") << QContent::Album << "Album";
    QTest::newRow("QContent::Artist") << QContent::Artist << "Artist";
    QTest::newRow("QContent::Author") << QContent::Author << "Author";
    QTest::newRow("QContent::Composer") << QContent::Composer << "Composer";
    QTest::newRow("QContent::ContentUrl") << QContent::ContentUrl << "ContentUrl";
    QTest::newRow("QContent::Copyright") << QContent::Copyright << "Copyright";
    QTest::newRow("QContent::CopyrightUrl") << QContent::CopyrightUrl << "CopyrightUrl";
    QTest::newRow("QContent::Description") << QContent::Description << "Description";
    QTest::newRow("QContent::Genre") << QContent::Genre << "Genre";
    QTest::newRow("QContent::InformationUrl") << QContent::InformationUrl << "InformationUrl";
    QTest::newRow("QContent::PublisherUrl") << QContent::PublisherUrl << "PublisherUrl";
    QTest::newRow("QContent::RightsIssuerUrl") << QContent::RightsIssuerUrl << "RightsIssuerUrl";
    QTest::newRow("QContent::Track") << QContent::Track << "Track";
    QTest::newRow("QContent::Version") << QContent::Version << "Version";
}

void tst_QContent::tst_propertyKey()
{
    QFETCH( QContent::Property, property );
    QFETCH( QString, result );

    QCOMPARE( QContent::propertyKey(property), result );
}

void tst_QContent::tst_propertyProperty_data()
{
    tst_propertyKey_data();
}

void tst_QContent::tst_propertyProperty()
{
    QFETCH( QContent::Property, property );
    QFETCH( QString, result );

    {
        QContent contentWrite(tmpFile);
        contentWrite.setProperty(property, result);
        contentWrite.commit();
    }
    {
        QContent contentRead(tmpFile);
        QCOMPARE(contentRead.property(property), result);
    }
}

void tst_QContent::tst_open_load_save()
{
    QByteArray localBA;
    QTemporaryFile tmpFile2(tempPath_noslash() + "/tst_qcontentXXXXXXX.tmp");
    QString filename;
    QVERIFY(tmpFile2.open());
    filename = tmpFile2.fileName();
    filesToDelete.append(filename);
    QContent content(filename);
    tmpFile2.close();
    QFile::remove(content.fileName());
    QVERIFY(QFile(content.fileName()).exists() == false);

    QIODevice *iodevice=content.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QVERIFY(iodevice != NULL);
    QVERIFY(iodevice->write(testData.toLocal8Bit()) != -1);
    iodevice->close();

    QVERIFY(content.load(localBA));
    QCOMPARE(localBA, testData.toLocal8Bit());

    QVERIFY(content.save(testData.toUpper().toLocal8Bit()));
    QVERIFY(content.load(localBA));
    QCOMPARE(localBA, testData.toUpper().toLocal8Bit());

    QContent::uninstall(QContent(filename).id());
    QFile::remove(filename);
    QVERIFY(QFile::exists(filename) == false);
}

void tst_QContent::tst_serialize_deserialize()
{
    QByteArray dumpingground;
    QDataStream outstream(&dumpingground, QIODevice::WriteOnly);
    QContent content(tmpFile), contentcopy;
    outstream << content;

    QDataStream instream(dumpingground);
    instream >> contentcopy;
    QCOMPARE(content, contentcopy);
}

void tst_QContent::tst_copyQContent()
{
    QContent content(tmpFile);
    QContent contentcopy = content;
    QCOMPARE(content, contentcopy);
}

void tst_QContent::tst_dashes()
{
    QTemporaryFile tmpDocFile(tempPath_noslash() + "/03 - tstqcontentXXXXXXX.ogg");
    tmpDocFile.open();
    filesToDelete.append(tmpDocFile.fileName());

    QContent content(tmpDocFile.fileName());
    QCOMPARE(content.fileName(), tmpDocFile.fileName());
    QCOMPARE(content.file(), tmpDocFile.fileName());
}
