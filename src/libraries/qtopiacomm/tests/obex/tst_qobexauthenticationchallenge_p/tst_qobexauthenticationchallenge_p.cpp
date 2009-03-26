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

#include "qobexauthenticationchallenge_p.h"
#include "qobexauthentication_p.h"
#include "qobexheader_p.h"

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>

#include <inttypes.h>

//TESTED_CLASS=QObexAuthenticationChallenge
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexauthenticationchallenge.cpp

// writeRawChallenge() writes the challenge tag values in this order:
//  1. Nonce
//  2. Options
//  3. Realm
// And each tag is written with a tag-length-value encoding. e.g. the
// nonce field takes up 18 bytes because it includes:
//      Tag length (1) + Tag value length indicator (1) + Tag value (16)

static const int FULL_NONCE_SIZE = 2 + QObexAuth::NonceSize;
static const int FULL_OPTIONS_SIZE = 2 + QObexAuth::OptionsSize;

static const int FULL_DIGEST_SIZE = 2 + QObexAuth::RequestDigestSize;


class tst_qobexauthenticationchallenge_p : public QObject
{
    Q_OBJECT

private:

    static QByteArray getBytes(const uint8_t *data, uint size)
    {
        return QByteArray(reinterpret_cast<const char *>(data), size);
    }

    static QByteArray insertSomeNonce(uint8_t *data)
    {
        QByteArray nonceBytes("abcdef123456789ghijklmnop", 16); // crop
        data[0] = QObexAuth::ChallengeNonceTag;
        data[1] = QObexAuth::NonceSize;
        memcpy(&data[2], nonceBytes.constData(), QObexAuth::NonceSize);
        return nonceBytes;
    }

    static void insertOptions(QObex::AuthChallengeOptions options, uint8_t *buf)
    {
        buf[0] = QObexAuth::OptionsTag;
        buf[1] = QObexAuth::OptionsSize;
        buf[2] = options;
    }

    static void insertRealm(const QByteArray &realmBytes, uint8_t *buf, uchar charSetCode)
    {
        buf[0] = QObexAuth::RealmTag;
        buf[1] = realmBytes.size() + 1;  // 1 for charset
        buf[2] = charSetCode;
        memcpy(&buf[3], realmBytes.constData(), realmBytes.size());
    }

    static QByteArray createNonce()
    {
        QByteArray b;
        QObexAuth::generateNonce(b);
        return b;
    }

    void fillRealmData()
    {
        QTest::addColumn<QString>("realmString");
        QTest::addColumn<uchar>("charSetCode");
        QTest::addColumn<QByteArray>("realmBytes");

        // ascii realm
        QString ascii("abcdef 123456");
        QTest::newRow("ascii realm")
                << ascii
                << uchar(QObexAuth::CharSetAscii)
                << ascii.toAscii();

        // latin-1 realm
        static const QChar latin1[] = { 0x00e4, 0x00e6, 0x00ff, 0x00d5, 0x00df };
        QString latin1String(latin1, 5);
        QTest::newRow("latin1 realm")
                << latin1String
                << uchar(QObexAuth::CharSetISO8859_1)
                << latin1String.toLatin1();

        // unicode realm
        static const QChar uc[4] = { 0x0055, 0x006e, 0x10e3, 0x03a3 };
        QString ucString(uc, 4);
        QByteArray ucBytes;
        QObexHeaderPrivate::unicodeBytesFromString(ucBytes, ucString);
        QTest::newRow("unicode realm")
                << ucString
                << uchar(QObexAuth::CharSetUnicode)
                << ucBytes;
    }

private slots:

    // ------------ tests for QObexAuthenticationChallenge public methods: ------------

    // This just tests the user string is set for the object; the
    // toRawResponse() tests will test that the user string is kept when
    // the challenge is converted to an auth response, and test the
    // max user string length as well.
    void setUser()
    {
        QObexAuthenticationChallenge c;
        const QObexAuthenticationChallengePrivate *priv =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        QCOMPARE(c.user(), QString());
        QVERIFY(!priv->m_modified);

        // once you set any value, even an empty string, it is "modified"
        c.setUser("");
        QCOMPARE(c.user(), QString());
        QVERIFY(priv->m_modified);

        c.setUser("some user");
        QCOMPARE(c.user(), QString("some user"));
        QVERIFY(priv->m_modified);
    }

    // Same as the setUser() test - the toRawResponse() will test the password
    // stuff more thoroughly later.
    void setPassword()
    {
        QObexAuthenticationChallenge c;
        const QObexAuthenticationChallengePrivate *priv =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        QCOMPARE(c.password(), QString());
        QVERIFY(!priv->m_modified);

        // once you set any value, even an empty string, it is "modified"
        c.setPassword("");
        QCOMPARE(c.password(), QString());
        QVERIFY(priv->m_modified);

        c.setPassword("some password");
        QCOMPARE(c.password(), QString("some password"));
        QVERIFY(priv->m_modified);
    }

    // ------------ tests for parseRawChallenge(): ------------

    void parseRawChallenge_invalid_data()
    {
        QTest::addColumn<QByteArray>("bytes");

        QTest::newRow("empty challenge") << QByteArray();

        uint8_t buf1[3];
        buf1[0] = QObexAuth::OptionsTag;
        buf1[1] = QObexAuth::OptionsSize;
        buf1[2] = 0;
        QTest::newRow("no nonce") << getBytes(buf1, 3);

        uint8_t buf2[18];
        uint8_t nonceSize = 5;  // nonce should be 16 long
        buf2[0] = QObexAuth::ChallengeNonceTag;
        buf2[1] = nonceSize;
        memcpy(&buf2[2], "abcdef123456789ghijklmnop", nonceSize);
        QTest::newRow("nonce too short") << getBytes(buf2, nonceSize + 2);

        QByteArray realm("some realm");
        int incorrectRealmSize = realm.size() * 2;
        int buflen = FULL_NONCE_SIZE + (2 + 1 + realm.size());
        uint8_t buf3[buflen];
        insertSomeNonce(buf3);
        int offset = FULL_NONCE_SIZE;
        buf3[offset++] = QObexAuth::RealmTag;
        buf3[offset++] = incorrectRealmSize;   // specified size is bigger than actual realm size
        buf3[offset++] = QObexAuth::CharSetAscii;
        memcpy(&buf3[offset], realm.constData(), realm.size());
        QTest::newRow("incorrect realm size value") << getBytes(buf3, buflen);

    }

    void parseRawChallenge_invalid()
    {
        QFETCH(QByteArray, bytes);
        QObexAuthenticationChallenge c;
        QVERIFY(!QObexAuthenticationChallengePrivate::parseRawChallenge(bytes, c));

        QCOMPARE(QObexAuthenticationChallengePrivate::getPrivate(c)->m_nonce, QByteArray());
        QCOMPARE(c.user(), QString());
        QCOMPARE(c.password(), QString());
        QCOMPARE(c.options(), 0);
        QCOMPARE(c.realm(), QString());
    }

    void parseRawChallenge_nonce_only()
    {
        int buflen = FULL_NONCE_SIZE;
        uint8_t buf[buflen];
        QByteArray nonce = insertSomeNonce(buf);

        QObexAuthenticationChallenge c;
        QVERIFY(QObexAuthenticationChallengePrivate::parseRawChallenge(getBytes(buf, buflen), c));

        QCOMPARE(QObexAuthenticationChallengePrivate::getPrivate(c)->m_nonce, nonce);

        // spec says if no options are given, we should assume options are 0
        QCOMPARE(c.options(), 0);

        // if no realm given, return empty string
        QCOMPARE(c.realm(), QString());
    }

    void parseRawChallenge_options_data()
    {
        QTest::addColumn<int>("options");
        QTest::newRow("0 options") << 0;    // can set field explicitly to 0 even if it's optional
        QTest::newRow("1 option") << int(QObex::UserIdRequired);
        QTest::newRow("2 options") << int(QObex::UserIdRequired | QObex::ReadOnlyAccess);
    }

    void parseRawChallenge_options()
    {
        QFETCH(int, options);

        int optionsOffset = FULL_NONCE_SIZE;
        int buflen = optionsOffset + FULL_OPTIONS_SIZE;
        uint8_t buf[buflen];
        QByteArray nonce = insertSomeNonce(buf);
        insertOptions(QObex::AuthChallengeOptions(options), &buf[optionsOffset]);

        QObexAuthenticationChallenge c;
        QVERIFY(QObexAuthenticationChallengePrivate::parseRawChallenge(getBytes(buf, buflen), c));

        QCOMPARE(c.options(), options);
        QCOMPARE(QObexAuthenticationChallengePrivate::getPrivate(c)->m_nonce, nonce);
    }

    void parseRawChallenge_realm_data()
    {
        fillRealmData();
    }

    void parseRawChallenge_realm()
    {
        QFETCH(QString, realmString);
        QFETCH(uchar, charSetCode);
        QFETCH(QByteArray, realmBytes);

        int realmOffset = FULL_NONCE_SIZE;
        int buflen = FULL_NONCE_SIZE + (2 + 1 + realmBytes.size());
        uint8_t buf[buflen];
        QByteArray nonce = insertSomeNonce(buf);
        insertRealm(realmBytes, &buf[realmOffset], charSetCode);

        QObexAuthenticationChallenge c;
        QVERIFY(QObexAuthenticationChallengePrivate::parseRawChallenge(getBytes(buf, buflen), c));

        QCOMPARE(c.realm().size(), realmString.size());
        QCOMPARE(c.realm(), realmString);
        QCOMPARE(QObexAuthenticationChallengePrivate::getPrivate(c)->m_nonce, nonce);
    }

    // test that parseRawChallenge() should be successful regardless of tag
    // ordering in challenge (e.g. whether nonce is the first tag or not, etc.)
    void parseRawChallenge_order_data()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<QByteArray>("nonce");
        QTest::addColumn<int>("options");
        QTest::addColumn<QString>("realm");

        QString realm("test realm");
        QByteArray realmBytes = realm.toAscii();
        uchar charSet = QObexAuth::CharSetAscii;
        int fullRealmSize = 2 + 1 + realmBytes.size();

        QObex::AuthChallengeOptions options = QObex::UserIdRequired;
        QByteArray nonce;

        int buflen = FULL_NONCE_SIZE + FULL_OPTIONS_SIZE + fullRealmSize;

        // order by Nonce -> Options -> Realm
        uint8_t buf1[buflen];
        nonce = insertSomeNonce(buf1);
        insertOptions(options, &buf1[FULL_NONCE_SIZE]);
        insertRealm(realmBytes, &buf1[FULL_NONCE_SIZE + FULL_OPTIONS_SIZE], charSet);
        QTest::newRow("usual order") << getBytes(buf1, buflen) << nonce << int(options) << realm;

        // order by Options -> Realm -> Nonce
        uint8_t buf2[buflen];
        insertOptions(options, buf2);
        insertRealm(realmBytes, &buf2[FULL_OPTIONS_SIZE], charSet);
        nonce = insertSomeNonce(&buf2[FULL_OPTIONS_SIZE + fullRealmSize]);
        QTest::newRow("different order 1") << getBytes(buf2, buflen) << nonce << int(options) << realm;

        // order by Realm -> Options -> Nonce
        uint8_t buf3[buflen];
        insertRealm(realmBytes, buf3, charSet);
        insertOptions(options, &buf3[fullRealmSize]);
        nonce = insertSomeNonce(&buf3[FULL_OPTIONS_SIZE + fullRealmSize]);
        QTest::newRow("different order 2") << getBytes(buf3, buflen) << nonce << int(options) << realm;
    }

    void parseRawChallenge_order()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(QByteArray, nonce);
        QFETCH(int, options);
        QFETCH(QString, realm);

        QObexAuthenticationChallenge c;
        QVERIFY(QObexAuthenticationChallengePrivate::parseRawChallenge(bytes, c));

        QCOMPARE(QObexAuthenticationChallengePrivate::getPrivate(c)->m_nonce, nonce);
        QCOMPARE(c.options(), options);
        QCOMPARE(c.realm(), realm);
    }



    // ------------ tests for writeRawChallenge(): ------------

    void writeRawChallenge_nonce()
    {
        QByteArray bytes;
        QObexAuthenticationChallengePrivate::writeRawChallenge(createNonce(), 0, QString(), bytes);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        // just need to know some random nonce data was written
        int offset = 0;
        QCOMPARE(data[offset++], uchar(QObexAuth::ChallengeNonceTag));
        QCOMPARE(data[offset++], uchar(QObexAuth::NonceSize));
        QCOMPARE(bytes.size(), FULL_NONCE_SIZE);
    }

    void writeRawChallenge_options_data()
    {
        QTest::addColumn<int>("options");
        QTest::newRow("no options") << 0;
        QTest::newRow("1 option") << int(QObex::UserIdRequired);
        QTest::newRow("2 options") << int(QObex::UserIdRequired | QObex::ReadOnlyAccess);
    }

    void writeRawChallenge_options()
    {
        QFETCH(int, options);
        QByteArray bytes;
        QObexAuthenticationChallengePrivate::writeRawChallenge(createNonce(), QObex::AuthChallengeOptions(options), QString(), bytes);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        if (options > 0) {
            int offset = QObexAuth::NonceSize + 2;
            QCOMPARE(data[offset++], uchar(QObexAuth::OptionsTag));
            QCOMPARE(data[offset++], uchar(QObexAuth::OptionsSize));
            QCOMPARE(data[offset], uchar(options));
            QCOMPARE(bytes.size(), FULL_NONCE_SIZE + FULL_OPTIONS_SIZE);
        } else {
            // if options == 0, options shouldn't have been added
            QCOMPARE(bytes.size(), FULL_NONCE_SIZE);
        }
    }

    void writeRawChallenge_realm_data()
    {
        fillRealmData();

        // if no realm given, should not add realm to raw bytes
        QTest::newRow("no realm") << QString() << uchar(0) << QByteArray();
    }

    void writeRawChallenge_realm()
    {
        QFETCH(QString, realmString);
        QFETCH(uchar, charSetCode);
        QFETCH(QByteArray, realmBytes);

        QByteArray bytes;
        QObexAuthenticationChallengePrivate::writeRawChallenge(createNonce(), 0, realmString, bytes);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        // for these tests, there are no options in the challenge, so the realm should come
        // straight after the nonce
        int offset = FULL_NONCE_SIZE;

        if (realmString.size() == 0) {
            QCOMPARE(bytes.size(), offset);

        } else {
            QCOMPARE(data[offset++], uchar(QObexAuth::RealmTag));
            QCOMPARE(data[offset++], uchar(1 + realmBytes.size()));  // + 1 for charset
            QCOMPARE(data[offset++], charSetCode);

            int totalSize = offset + realmBytes.size();
            QByteArray realm = bytes.mid(offset);
            QCOMPARE(realm, realmBytes);
            QCOMPARE(bytes.size(), totalSize);
        }
    }

    void writeRawChallenge_realm_plusOptions()
    {
        // if there are options as well as the realm, then the realm should
        // come after the options
        QString realmString("some realm");
        QByteArray realmBytes = realmString.toAscii();
        QObex::AuthChallengeOptions options = QObex::UserIdRequired;

        QByteArray bytes;
        QObexAuthenticationChallengePrivate::writeRawChallenge(createNonce(), options, realmString, bytes);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        int offset = FULL_NONCE_SIZE;
        QCOMPARE(data[offset], uchar(QObexAuth::OptionsTag));
        offset += FULL_OPTIONS_SIZE;

        QCOMPARE(data[offset++], uchar(QObexAuth::RealmTag));
        QCOMPARE(data[offset++], uchar(1 + realmBytes.size()));
        QCOMPARE(data[offset++], uchar(QObexAuth::CharSetAscii));

        QByteArray insertedRealm = bytes.mid(offset);
        QCOMPARE(insertedRealm, realmBytes);
        QCOMPARE(bytes.size(), offset + realmBytes.size());
    }



    // ------------ tests for toRawResponse(): ------------

protected:
    void fillChallenge(QObexAuthenticationChallenge &challenge, QObex::AuthChallengeOptions options = 0, const QString &realm = QString())
    {
        QByteArray bytes;
        QObexAuthenticationChallengePrivate::writeRawChallenge(createNonce(), options, realm, bytes);
        bool b = QObexAuthenticationChallengePrivate::parseRawChallenge(bytes, challenge);
        Q_ASSERT(b);
    }

    QByteArray expectedDigest(const QObexAuthenticationChallenge &c)
    {
        const QObexAuthenticationChallengePrivate *d =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        return QCryptographicHash::hash(
                            d->m_nonce + ":" + c.password().toLatin1(),
                            QCryptographicHash::Md5);
    }


private slots:

    void toRawResponse_digestOnly()
    {
        QObexAuthenticationChallenge c;
        fillChallenge(c);

        const QObexAuthenticationChallengePrivate *d =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        QByteArray bytes;
        QVERIFY(d->toRawResponse(bytes));
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        // check for digest
        QCOMPARE(data[0], uchar(QObexAuth::RequestDigestTag));
        QCOMPARE(data[1], uchar(QObexAuth::RequestDigestSize));
        QCOMPARE(getBytes(&data[2], QObexAuth::RequestDigestSize),
                 expectedDigest(c));

        // digest is the only thing in there
        QCOMPARE(bytes.size(), FULL_DIGEST_SIZE);
    }

    void toRawResponse_user_data()
    {
        QTest::addColumn<QString>("user");
        QTest::addColumn<bool>("parseShouldSucceed");
        QTest::newRow("empty username") << QString() << true;
        QTest::newRow("some username") << QString("some user") << true;
        QTest::newRow("20-char username)") << QString().fill('a', 20) << true;
        QTest::newRow("bad username (over 20 chars)")
                << QString().fill('a', 21) << false;
    }

    void toRawResponse_user()
    {
        QFETCH(QString, user);
        QFETCH(bool, parseShouldSucceed);

        QObexAuthenticationChallenge c;
        fillChallenge(c);
        c.setUser(user);

        const QObexAuthenticationChallengePrivate *d =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        QByteArray bytes;
        QCOMPARE(d->toRawResponse(bytes), parseShouldSucceed);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        if (!parseShouldSucceed) {
            QCOMPARE(bytes.size(), 0);

        } else {
            if (user.isEmpty()) {
                // shouldn't add user if it's empty
                QCOMPARE(bytes.size(), FULL_DIGEST_SIZE);
            } else {
                // user should come after digest
                int offset = FULL_DIGEST_SIZE;
                int userSize = user.toLatin1().size();
                QCOMPARE(data[offset++], uchar(QObexAuth::UserIdTag));
                QCOMPARE(data[offset++], uchar(userSize));
                QCOMPARE(getBytes(&data[offset], userSize), user.toLatin1());
                QCOMPARE(bytes.size(), FULL_DIGEST_SIZE + (2 + userSize));
            }

            // check digest is still there
            QCOMPARE(getBytes(&data[2], QObexAuth::RequestDigestSize),
                    expectedDigest(c));
        }
    }

    void toRawResponse_nonce_data()
    {
        QTest::addColumn<QByteArray>("nonce");
        QTest::addColumn<bool>("parseShouldSucceed");
        QTest::addColumn<bool>("nonceIsAdded");

        QTest::newRow("empty nonce, won't be added") << QByteArray() << true << false;
        QTest::newRow("valid nonce")
                << QByteArray().fill('a', QObexAuth::NonceSize) << true << true;

        QTest::newRow("invalid nonce (too short)")
                << QByteArray().fill('a', QObexAuth::NonceSize-1) << false << false;
        QTest::newRow("invalid nonce (too long)")
                << QByteArray().fill('a', QObexAuth::NonceSize+1) << false << false;
    }

    void toRawResponse_nonce()
    {
        QFETCH(QByteArray, nonce);
        QFETCH(bool, parseShouldSucceed);
        QFETCH(bool, nonceIsAdded);

        QObexAuthenticationChallenge c;
        fillChallenge(c);

        const QObexAuthenticationChallengePrivate *d =
                QObexAuthenticationChallengePrivate::getPrivate(c);

        QByteArray bytes;
        QCOMPARE(d->toRawResponse(bytes, nonce), parseShouldSucceed);
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        if (parseShouldSucceed) {
            if (nonceIsAdded) {
                // no user, so nonce should come after digest
                int offset = FULL_DIGEST_SIZE;
                QCOMPARE(data[offset++], uchar(QObexAuth::ResponseNonceTag));
                QCOMPARE(data[offset++], uchar(QObexAuth::NonceSize));
                QCOMPARE(getBytes(&data[offset], QObexAuth::NonceSize), nonce);
                QCOMPARE(bytes.size(), FULL_DIGEST_SIZE + FULL_NONCE_SIZE);

            } else {
                QCOMPARE(bytes.size(), FULL_DIGEST_SIZE);
            }

            // check digest is still there
            QCOMPARE(getBytes(&data[2], QObexAuth::RequestDigestSize),
                    expectedDigest(c));
        }
    }

    // check data is written correctly if all fields are inserted
    void toRawResponse_insertAllFields()
    {
        QString user("some user name");
        QByteArray nonce = QByteArray().fill('z', QObexAuth::NonceSize);
        QObexAuthenticationChallenge c;
        fillChallenge(c);
        c.setUser(user);

        const QObexAuthenticationChallengePrivate *d =
                QObexAuthenticationChallengePrivate::getPrivate(c);
        QByteArray bytes;
        QVERIFY(d->toRawResponse(bytes, nonce));
        const uchar *data = reinterpret_cast<uchar *>(bytes.data());

        QCOMPARE(*data++, uchar(QObexAuth::RequestDigestTag));
        QCOMPARE(*data++, uchar(QObexAuth::RequestDigestSize));
        QCOMPARE(getBytes(data, QObexAuth::RequestDigestSize),
                 expectedDigest(c));
        data += expectedDigest(c).size();

        int userSize = user.toLatin1().size();
        QCOMPARE(*data++, uchar(QObexAuth::UserIdTag));
        QCOMPARE(*data++, uchar(userSize));
        QCOMPARE(getBytes(data, userSize), user.toLatin1());
        data += userSize;

        QCOMPARE(*data++, uchar(QObexAuth::ResponseNonceTag));
        QCOMPARE(*data++, uchar(QObexAuth::NonceSize));
        QCOMPARE(getBytes(data, QObexAuth::NonceSize), nonce);

        QCOMPARE(bytes.size(), FULL_DIGEST_SIZE + (2 + userSize) + FULL_NONCE_SIZE);
    }

};

QTEST_MAIN(tst_qobexauthenticationchallenge_p)
#include "tst_qobexauthenticationchallenge_p.moc"
