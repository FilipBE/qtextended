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

#include <QMediaPlaylist>
#include <QObject>
#include <QTest>
#include <QSignalSpy>


class tst_QMediaPlaylist : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void playingItemInitialized();
    void setPlayingItem();
    void equalityOperatorsTest();
    void uriSchemeTest();
};


void tst_QMediaPlaylist::initTestCase()
{
}

void tst_QMediaPlaylist::playingItemInitialized()
{
    QMediaPlaylist  playlist;

    QCOMPARE(playlist.playing(), QModelIndex());  // Invalid index
}

void tst_QMediaPlaylist::setPlayingItem()
{
    QUrl dummyPath = QUrl::fromLocalFile("/dummy/path");
    QMediaPlaylist  playlist(QStringList() << dummyPath.toString());

    playlist.setPlaying(playlist.firstIndex());

    QCOMPARE(qvariant_cast<QUrl>(playlist.data(playlist.playing(), QMediaPlaylist::Url)), dummyPath);
}

void tst_QMediaPlaylist::equalityOperatorsTest()
{
    QMediaPlaylist  playlist1(QStringList() << "/dummy1" << "/dummy2");
    QMediaPlaylist  playlist2(QStringList() << "/dummy3" << "/dummy4");
    QMediaPlaylist  playlist3(QStringList() << "/dummy1" << "/dummy2");

    //
    QVERIFY(playlist1 == playlist3);
    QVERIFY(!(playlist1 != playlist3));

    QVERIFY(playlist1 != playlist2);
    QVERIFY(!(playlist1 == playlist2));

    //
    playlist1.setPlaying(playlist1.index(0));
    playlist3.setPlaying(playlist3.index(0));
    QVERIFY(playlist1 == playlist3);

    //
    playlist1.setPlaying(playlist1.index(0));
    playlist3.setPlaying(playlist3.index(1));
    QVERIFY(playlist1 != playlist3);
}

void tst_QMediaPlaylist::uriSchemeTest()
{
    QMediaPlaylist  playList1(QStringList() << "http://some.example.com");
    QMediaPlaylist  playList2(QStringList() << "file:///path/to/a/file");

    QCOMPARE(qvariant_cast<QUrl>(playList1.data(playList1.firstIndex(), QMediaPlaylist::Url)).scheme(), QString::fromLatin1("http"));
    QCOMPARE(qvariant_cast<QUrl>(playList2.data(playList2.firstIndex(), QMediaPlaylist::Url)).scheme(), QString::fromLatin1("file"));
}

QTEST_MAIN(tst_QMediaPlaylist)

#include "tst_qmediaplaylist.moc"

