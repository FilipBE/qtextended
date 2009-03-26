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


#include <QContentSet>
#include <QContent>
#include <QDebug>
#include <QtopiaApplication>
#include <QMetaType>
#include <stdlib.h>
#include <locale.h>
#include <shared/util.h>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=QContentSet
//TESTED_FILES=src/libraries/qtopia/qcontentset.cpp

QString tempPath_noslash()
{
    QString ret(QDir::tempPath());
    if (ret.endsWith('/')) ret = ret.left(ret.size()-1);
    return ret;
}

class tst_QContentSet: public QObject
{
    Q_OBJECT
public:
    typedef QPair<int, QString> Data;

protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    // These are the test functions.  Each one should return
    // the database to the state created by initTestCase().
    void tst_Constructors();
    void tst_SortOrder_data();
    void tst_SortOrder();
    void tst_Accessors();
    void tst_AddRemoveSynchronous();
    void tst_AddRemoveAsynchronous();

private:
    QList<Data> Hyphenated;
    QList<Data> Korean;
    QList<Data> German;
    QList<Data> Chinese;
    QList<Data> English;

    QString tempdir;
};

const QString testData(QLatin1String("test"));

QTEST_APP_MAIN( tst_QContentSet, QtopiaApplication )

Q_DECLARE_METATYPE( QList<tst_QContentSet::Data> );

#include "tst_qcontentset.moc"

void tst_QContentSet::initTestCase()
{
    

    Hyphenated.append(Data(1, "Cal\xad\x63u\xadla\xadtor"));
    Hyphenated.append(Data(2, "Cal\xad\x65n\xad\x64\x61r"));
    Hyphenated.append(Data(3, "Call Op\xadtions"));
    Hyphenated.append(Data(4, "Se\xad\x63u\xadri\xadty"));
    Hyphenated.append(Data(5, "Ser\xadver Wid\xadgets"));

    Chinese.append(Data(1, QString::fromLocal8Bit("安静")));
    Chinese.append(Data(2, QString::fromLocal8Bit("安全")));
    Chinese.append(Data(3, QString::fromLocal8Bit("岸边")));
    Chinese.append(Data(4, QString::fromLocal8Bit("米饭")));
    Chinese.append(Data(5, QString::fromLocal8Bit("眼泪")));
    Chinese.append(Data(6, QString::fromLocal8Bit("眼泪洗面")));

    Korean.append(Data(1, QString::fromLocal8Bit("가나다라")));
    Korean.append(Data(2, QString::fromLocal8Bit("나다라마")));
    Korean.append(Data(3, QString::fromLocal8Bit("다라마바")));
    Korean.append(Data(4, QString::fromLocal8Bit("라마바사")));
    Korean.append(Data(5, QString::fromLocal8Bit("마바사아")));

    German.append(Data(1, QString::fromLocal8Bit("ausnehmen")));
    German.append(Data(2, QString::fromLocal8Bit("aussehen")));
    German.append(Data(3, QString::fromLocal8Bit("außerdem")));
    German.append(Data(4, QString::fromLocal8Bit("aussuchen")));
    German.append(Data(5, QString::fromLocal8Bit("Göbel")));
    German.append(Data(6, QString::fromLocal8Bit("Goethe")));
    German.append(Data(7, QString::fromLocal8Bit("Goldmann")));
    German.append(Data(8, QString::fromLocal8Bit("Götz")));
    German.append(Data(9, QString::fromLocal8Bit("Haus")));
    German.append(Data(10, QString::fromLocal8Bit("Häuser")));
    German.append(Data(11, QString::fromLocal8Bit("Hüne")));
    German.append(Data(12, QString::fromLocal8Bit("Hunnen")));
    German.append(Data(13, QString::fromLocal8Bit("Mann")));
    German.append(Data(14, QString::fromLocal8Bit("Männer")));
    German.append(Data(15, QString::fromLocal8Bit("spät")));
    German.append(Data(16, QString::fromLocal8Bit("spatz")));

    {
        QTemporaryFile tf(tempPath_noslash() + "/tst_qcontent_dataXXXXXX");
        QVERIFY( tf.open() );
        tempdir = tf.fileName();
    }
    QVERIFY( QDir("/").mkpath(tempdir) );

    foreach(Data data, Hyphenated)
    {
        QString filename = tempdir + '/' + Qtopia::dehyphenate(data.second) + ".txt";
        QContent content;
        content.setName(data.second);
        content.setFile(filename);
        content.save(testData.toLocal8Bit());
        content.setProperty("language", "Hyphenated");
        content.setProperty("order", QString::number(data.first));
        content.commit();
        QVERIFY2(QFile::exists(filename), ("Failed to create file: "+filename).toLocal8Bit());
    }

    foreach(Data data, Chinese)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        QContent content;
        content.setName(data.second);
        content.setFile(filename);
        content.save(testData.toLocal8Bit());
        content.setProperty("language", "Chinese");
        content.setProperty("order", QString::number(data.first));
        content.commit();
        QVERIFY2(QFile::exists(filename), ("Failed to create file: "+filename).toLocal8Bit());
    }
    foreach(Data data, Korean)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        QContent content;
        content.setName(data.second);
        content.setFile(filename);
        content.save(testData.toLocal8Bit());
        content.setProperty("language", "Korean");
        content.setProperty("order", QString::number(data.first));
        content.commit();
        QVERIFY2(QFile::exists(filename), ("Failed to create file: "+filename).toLocal8Bit());
    }
    foreach(Data data, German)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        QContent content;
        content.setName(data.second);
        content.setFile(filename);
        content.save(testData.toLocal8Bit());
        content.setProperty("language", "German");
        content.setProperty("order", QString::number(data.first));
        content.commit();
        QVERIFY2(QFile::exists(filename), ("Failed to create file: "+filename).toLocal8Bit());
    }
}

void tst_QContentSet::cleanupTestCase()
{
    foreach(Data data, Hyphenated)
    {
        QString filename = tempdir + '/' + Qtopia::dehyphenate(data.second) + ".txt";
        if(QFile::exists(filename))
        {
            QFile::remove(filename);
            QContent::uninstall(QContent(filename).id());
        }
    }
    foreach(Data data, Chinese)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        if(QFile::exists(filename))
        {
            QFile::remove(filename);
            QContent::uninstall(QContent(filename).id());
        }
    }
    foreach(Data data, Korean)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        if(QFile::exists(filename))
        {
            QFile::remove(filename);
            QContent::uninstall(QContent(filename).id());
        }
    }
    foreach(Data data, German)
    {
        QString filename = tempdir + '/' + data.second + ".txt";
        if(QFile::exists(filename))
        {
            QFile::remove(filename);
            QContent::uninstall(QContent(filename).id());
        }
    }

    QDir("/").rmpath(tempdir);
}

void tst_QContentSet::tst_Constructors()
{
    QContentSet set(QContentFilter::Location, tempdir);
    QVERIFY(set.count() != 0);
    QVERIFY(!set.isEmpty());
    QContentSet set2(QContentFilter(QContentFilter::Location, tempdir));
    QVERIFY(set2.count() != 0);
    QCOMPARE(set.count(), set2.count());
    QContentSet set3(QContentFilter(QContentFilter::Location, tempdir), QStringList()<<"name");
    QVERIFY(set3.count() != 0);
    QCOMPARE(set.count(), set3.count());
    QCOMPARE(set.itemIds(), set3.itemIds());
    QContentSet set4(QContentFilter::Location, tempdir, QStringList()<<"name");
    QVERIFY(set4.count() != 0);
    QCOMPARE(set.count(), set4.count());
    QCOMPARE(set.itemIds(), set4.itemIds());
    QContentSet set5(set);
    QVERIFY(set5.count() != 0);
    QCOMPARE(set.count(), set5.count());
    QCOMPARE(set.itemIds(), set5.itemIds());
    QContentSet set6;
    QVERIFY(set6.count() == 0);
    QVERIFY(set6.isEmpty());
}

void tst_QContentSet::tst_SortOrder_data()
{
    QTest::addColumn<QString>("language");
    QTest::addColumn< QList<Data> >("list");
    QTest::addColumn<QString>("locale");
    QTest::newRow("Hyphenated") << "Hyphenated" << Hyphenated << "en_US.UTF-8";
    QTest::newRow("Chinese") << "Chinese" << Chinese << "zh_CN.UTF-8";
    QTest::newRow("Korean") << "Korean" << Korean << "ko_KR.UTF-8";
    QTest::newRow("German") << "German" << German << "de_DE.UTF-8";
}

void tst_QContentSet::tst_SortOrder()
{
    QFETCH( QString, language );
    QFETCH( QList<Data>, list );
    QFETCH( QString, locale );

    qLog(Autotest);   // to show which language we're testing.

    QVERIFY((setlocale(LC_ALL, locale.toLocal8Bit().constData()) != NULL)
            || (setenv("LANG", locale.toLocal8Bit().constData(), 1) == 0));
    //QCOMPARE(QString(getenv("LANG")), locale);
    QVERIFY((setlocale(LC_COLLATE, locale.toLocal8Bit().constData()) != NULL)
            || (setenv("LC_COLLATE", locale.toLocal8Bit().constData(), 1) == 0));
    //QCOMPARE(QString(getenv("LC_COLLATE")), locale);

    QContentSet set(QContentFilter::Synthetic, "none/language/"+language, QStringList() << "name");
    QVERIFY(set.count() != 0);
    QContentList clist=set.items();
    for(int i=0;i<clist.count();i++)
    {
        if(language == "Chinese")
            qDebug() << "expected position" << clist.at(i).property("order").toInt() << "actual position" << i+1;
        if(language == "Chinese" && clist.at(i).property("order").toInt() != i+1)
            QEXPECT_FAIL("Chinese", "Linux sort support is broken/wrong for chinese languages", Continue);
        QCOMPARE(clist.at(i).property("order").toInt(), i+1);
        if(language == "Chinese" && clist.at(i).property("order").toInt() != i+1)
            QEXPECT_FAIL("Chinese", "Linux sort support is broken/wrong for chinese languages", Continue);
        QCOMPARE(clist.at(i).name(), list.at(i).second);
    }
}

void tst_QContentSet::tst_Accessors()
{
    QContentSet set(QContentFilter::Location, tempdir);
    QContentSetModel model(&set);
    QVERIFY(set.count() != 0);
    QCOMPARE(set.count(), model.rowCount());
    QContentList clist=set.items();
    QContentIdList cidlist=set.itemIds();
    QCOMPARE(set.count(), clist.count());
    QCOMPARE(set.count(), cidlist.count());
    for(int i=0;i<set.count();i++)
    {
        QCOMPARE(clist.at(i), set.content(i));
        QCOMPARE(cidlist.at(i), set.contentId(i));
        QCOMPARE(clist.at(i).id(), set.contentId(i));
        QCOMPARE(model.content(i), set.content(i));
        QCOMPARE(model.contentId(i), set.contentId(i));
    }
}

void tst_QContentSet::tst_AddRemoveSynchronous()
{
    QContentSet set(QContentSet::Synchronous);
    QContentSetModel model(&set);
    // add from hyphenated set in sorted order.

    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), i);
        set.add(QContent(filename));
        QCOMPARE(set.count(), i+1);
        QCOMPARE(set.content(i).fileName(), filename);
        QCOMPARE(model.content(i).fileName(), filename);
        QCOMPARE(model.content(model.index(i)).fileName(), filename);
    }
    
    // remove from hyphenated set from last to first.
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.content(i).fileName(), filename);
        QCOMPARE(model.content(i).fileName(), filename);
        QCOMPARE(model.content(model.index(i)).fileName(), filename);
        QCOMPARE(set.count(), i+1);
        set.remove(QContent(filename));
        QCOMPARE(set.count(), i);
    }

    QCOMPARE(set.count(), 0);

    // add from hyphenated set from last to first (reverse ordered)
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), Hyphenated.count()-(i+1));
        set.add(QContent(filename));
        QCOMPARE(set.count(), Hyphenated.count()-i);
        QCOMPARE(set.content(0).name(), Hyphenated[i].second);
        QCOMPARE(model.content(0).name(), Hyphenated[i].second);
        QCOMPARE(model.content(model.index(0)).name(), Hyphenated[i].second);
    }

    // remove from hyphenated set from first to last.
    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.content(0).fileName(), filename);
        QCOMPARE(model.content(0).fileName(), filename);
        QCOMPARE(model.content(model.index(0)).fileName(), filename);
        QCOMPARE(set.count(), Hyphenated.count()-(i));
        set.remove(QContent(filename));
        QCOMPARE(set.count(), Hyphenated.count()-(i+1));
        QVERIFY(set.content(0).name() != Hyphenated[i].second);
    }

    QCOMPARE(set.count(), 0);
    
    set.add(QContent(tempdir + '/' + Qtopia::dehyphenate(Hyphenated[0].second) + ".txt"));
    QCOMPARE(set.count(), 1);
    set.clear();
    QCOMPARE(set.count(), 0);

    set.setCriteria(QContentFilter::Synthetic, "none/language/Korean");
    int priorcount=set.count();
    QVERIFY(set.count() != 0);
    // add from hyphenated set from last to first (reverse ordered)
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-(i+1));
        set.add(QContent(filename));
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-i);
    }
    // remove from hyphenated set from first to last.
    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-(i));
        set.remove(QContent(filename));
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-(i+1));
    }
    QCOMPARE(set.count(), priorcount);
    set.clear();
    QCOMPARE(set.count(), 0);
}

void tst_QContentSet::tst_AddRemoveAsynchronous()
{
    QContentSet set(QContentSet::Asynchronous);
    QContentSetModel model(&set);
    QSignalSpy insertspy(&set, SIGNAL(contentInserted()));
    QSignalSpy removespy(&set, SIGNAL(contentRemoved()));
    QSignalSpy updatefinishedspy(&model, SIGNAL(updateFinished()));
    // add from hyphenated set in sorted order.

    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), i);
        insertspy.clear();
        set.add(QContent(filename));
        QTRY_VERIFY( insertspy.count() == 1 );
        QCOMPARE(set.count(), i+1);
        QCOMPARE(set.content(i).fileName(), filename);
        QCOMPARE(model.content(i).fileName(), filename);
        QCOMPARE(model.content(model.index(i)).fileName(), filename);
    }
    
    // remove from hyphenated set from last to first.
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.content(i).fileName(), filename);
        QCOMPARE(model.content(i).fileName(), filename);
        QCOMPARE(model.content(model.index(i)).fileName(), filename);
        QCOMPARE(set.count(), i+1);
        removespy.clear();
        set.remove(QContent(filename));
        QTRY_VERIFY( removespy.count() == 1 );
        QCOMPARE(set.count(), i);
    }

    QCOMPARE(set.count(), 0);

    // add from hyphenated set from last to first (reverse ordered)
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), Hyphenated.count()-(i+1));
        insertspy.clear();
        set.add(QContent(filename));
        QTRY_VERIFY( insertspy.count() == 1 );
        QCOMPARE(set.count(), Hyphenated.count()-i);
        QCOMPARE(set.content(0).name(), Hyphenated[i].second);
        QCOMPARE(model.content(0).name(), Hyphenated[i].second);
        QCOMPARE(model.content(model.index(0)).name(), Hyphenated[i].second);
    }

    // remove from hyphenated set from first to last.
    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.content(0).fileName(), filename);
        QCOMPARE(model.content(0).fileName(), filename);
        QCOMPARE(model.content(model.index(0)).fileName(), filename);
        QCOMPARE(set.count(), Hyphenated.count()-i);
        removespy.clear();
        set.remove(QContent(filename));
        QTRY_VERIFY( removespy.count() == 1 );
        QCOMPARE(set.count(), Hyphenated.count()-(i+1));
        QVERIFY(set.content(0).name() != Hyphenated[i].second);
    }

    QCOMPARE(set.count(), 0);

    
    insertspy.clear();
    set.add(QContent(tempdir + '/' + Qtopia::dehyphenate(Hyphenated[0].second) + ".txt"));
    QTRY_VERIFY( insertspy.count() == 1 );
    QCOMPARE(set.count(), 1);
    set.clear();
    QEXPECT_FAIL("", "Should fail in asynchronous mode. No way to block on this as yet", Continue);
    QCOMPARE(set.count(), 0);

    updatefinishedspy.clear();
    set.setCriteria(QContentFilter::Synthetic, "none/language/Korean");
    QTRY_VERIFY(updatefinishedspy.count() == 1);
    QVERIFY(set.count() != 0);
    int priorcount=set.count();
    // add from hyphenated set from last to first (reverse ordered)
    for(int i=Hyphenated.count()-1;i>=0;i--)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-(i+1));
        insertspy.clear();
        set.add(QContent(filename));
        QTRY_VERIFY( insertspy.count() == 1 );
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-i);
    }

    // remove from hyphenated set from first to last.
    for(int i=0;i<Hyphenated.count();i++)
    {
        QString filename=tempdir + '/' + Qtopia::dehyphenate(Hyphenated[i].second) + ".txt";
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-i);
        removespy.clear();
        set.remove(QContent(filename));
        QTRY_VERIFY( removespy.count() == 1 );
        QCOMPARE(set.count(), priorcount+Hyphenated.count()-(i+1));
    }
    QCOMPARE(set.count(), priorcount);

    set.clear();
    QEXPECT_FAIL("", "Should fail in asynchronous mode. No way to block on this as yet", Continue);
    QCOMPARE(set.count(), 0);
}
