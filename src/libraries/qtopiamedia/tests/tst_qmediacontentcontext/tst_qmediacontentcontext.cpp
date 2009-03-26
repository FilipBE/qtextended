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

#include <QMediaContentContext>
#include <QObject>
#include <QTest>
#include <QSignalSpy>

Q_DECLARE_METATYPE(QMediaContent*)

class tst_QMediaContentContext : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();

    void setMediaContent();
    void setMediaContent_addObject();

    void contentChanged();
};

class TestPlayer : public QObject
{
Q_OBJECT
public:
    QMediaContent* content;

public slots:
    void setMediaContent(QMediaContent* _content)
    { content = _content; }
};

void tst_QMediaContentContext::initTestCase()
{
    qRegisterMetaType<QMediaContent*>();
}

void tst_QMediaContentContext::setMediaContent()
{
    TestPlayer p1;
    TestPlayer p2;

    QMediaContent c1(QUrl("something"));
    QMediaContent c2(QUrl("another thing"));

    QMediaContentContext ctx;
    ctx.addObject(&p1);
    ctx.addObject(&p2);
    ctx.setMediaContent(&c1);

    // Verify setMediaContent worked on all added objects
    QCOMPARE( p1.content, &c1 );
    QCOMPARE( p1.content, p2.content ); // invariant

    // Verify that removing an object causes it to no longer be updated
    ctx.removeObject(&p2);
    ctx.setMediaContent(&c2);
    QCOMPARE( p1.content, &c2 );
    QCOMPARE( p2.content, &c1 );
}

void tst_QMediaContentContext::setMediaContent_addObject()
{
    TestPlayer p1;
    QMediaContent c1(QUrl("something"));

    QMediaContentContext ctx;
    ctx.setMediaContent(&c1);
    ctx.addObject(&p1);

    QCOMPARE( p1.content, &c1 );
}

void tst_QMediaContentContext::contentChanged()
{
    QMediaContent c1(QUrl("something"));
    QMediaContent c2(QUrl("something2"));
    QMediaContentContext ctx;

    {
        QSignalSpy spy(&ctx, SIGNAL(contentChanged(QMediaContent*)));
        ctx.setMediaContent(&c1);
        QCOMPARE( spy.count(), 1 );
        QMediaContent* c = spy.takeAt(0).at(0).value<QMediaContent*>();
        QCOMPARE(c, &c1);
    }

    // Verify signal isn't emitted when setting to the same content
    QSignalSpy spy(&ctx, SIGNAL(contentChanged(QMediaContent*)));
    ctx.setMediaContent(&c1);
    QCOMPARE( spy.count(), 0 );
}

QTEST_MAIN(tst_QMediaContentContext)

#include "tst_qmediacontentcontext.moc"

