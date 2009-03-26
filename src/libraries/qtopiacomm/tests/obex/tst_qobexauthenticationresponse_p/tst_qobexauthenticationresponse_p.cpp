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

#include "qobexauthenticationresponse_p.h"
#include "qobexauthenticationchallenge_p.h"
#include "qobexauthentication_p.h"
#include "qobexheader_p.h"

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QByteArray>

#include <inttypes.h>

const int FULL_DIGEST_SIZE = 2 + QObexAuth::RequestDigestSize;
const int FULL_NONCE_SIZE = 2 + QObexAuth::NonceSize;

//TESTED_CLASS=QObexAuthenticationResponsePrivate
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexauthenticationresponse.cpp


class tst_qobexauthenticationresponse_p : public QObject
{
    Q_OBJECT

private:
    static QByteArray getBytes(const uint8_t *data, uint size)
    {
        return QByteArray((const char*)data, size);
    }

    static QByteArray insertSomeDigest(uint8_t *data)
    {
        QByteArray bytes("abcdef123456789ghijklmnop", 16); // crop
        data[0] = QObexAuth::RequestDigestTag;
        data[1] = QObexAuth::RequestDigestSize;
        memcpy(&data[2], bytes.constData(), QObexAuth::RequestDigestSize);
        return bytes;
    }

    static void insertUser(const QByteArray &user, uint8_t *data)
    {
        data[0] = QObexAuth::UserIdTag;
        data[1] = user.size();
        memcpy(&data[2], user.constData(), user.size());
    }

    static QByteArray insertSomeNonce(uint8_t *data)
    {
        QByteArray nonce("abcdef123456789ghijklmnop", 16); // crop
        data[0] = QObexAuth::ResponseNonceTag;
        data[1] = nonce.size();
        memcpy(&data[2], nonce.constData(), nonce.size());
        return nonce;
    }

private slots:

    // ------------ tests for QObexAuthenticationResponse::match(): ----------

    void match_data()
    {
        QTest::addColumn<QString>("attemptedPassword");
        QTest::addColumn<QString>("correctPassword");

        QString thePassword = "the correct password";
        QTest::newRow("right password") << thePassword << thePassword;
        QTest::newRow("wrong password") << "lkaaeoijflsdfj" << thePassword;
        QTest::newRow("wrong password (empty)") << "" << thePassword;

        QTest::newRow("right password (is empty)") << "" << "";
        QTest::newRow("wrong password (is not empty)") << "afojwefoi" << "";
    }

    void match()
    {
        QFETCH(QString, attemptedPassword);
        QFETCH(QString, correctPassword);

        // create a challenge
        QByteArray challengeBytes;
        QObexAuthenticationChallenge challenge;
        QByteArray nonce;
        QObexAuth::generateNonce(nonce);
        QObexAuthenticationChallengePrivate::writeRawChallenge(nonce, 0, QString(), challengeBytes);
        QVERIFY(QObexAuthenticationChallengePrivate::parseRawChallenge(challengeBytes, challenge));
        challenge.setPassword(attemptedPassword);

        // get the response for the challenge
        QByteArray responseBytes;
        const QObexAuthenticationChallengePrivate *privateChallenge =
                QObexAuthenticationChallengePrivate::getPrivate(challenge);
        QVERIFY(privateChallenge->toRawResponse(responseBytes));

        // create a response that knows the nonce sent in the challenge
        QObexAuthenticationResponse response =
                QObexAuthenticationResponsePrivate::createResponse(
                        privateChallenge->m_nonce);
        QVERIFY(QObexAuthenticationResponsePrivate::parseRawResponse(responseBytes, response));

        // check match() result
        QCOMPARE(response.match(correctPassword),
                 attemptedPassword == correctPassword);
    }

    // ------------ tests for createResponse(): ------------

    void createResponse()
    {
        QByteArray bytes = QByteArray().fill('c', QObexAuth::NonceSize);
        QObexAuthenticationResponse r =
                QObexAuthenticationResponsePrivate::createResponse(bytes);
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_nonce,
                 bytes);
    }

    // ------------ tests for parseRawResponse(): ------------

    void parseRawResponse_invalid_data()
    {
        QTest::addColumn<QByteArray>("bytes");

        QTest::newRow("empty response") << QByteArray();

        QString user("a username");
        QByteArray userBytes = user.toLatin1();

        int buflen = 2 + userBytes.size();
        uint8_t buf1[buflen];
        insertUser(user.toLatin1(), buf1);
        QTest::newRow("no digest") << getBytes(buf1, buflen);

        buflen = FULL_DIGEST_SIZE;
        uint8_t buf2[buflen];
        uint8_t digestSize = 5;  // digest should be 16 long
        buf2[0] = QObexAuth::RequestDigestTag;
        buf2[1] = digestSize;
        memcpy(&buf2[2], "abcdef123456789ghijklmnop", digestSize);
        QTest::newRow("digest too short") << getBytes(buf2, buflen);

        buflen = FULL_DIGEST_SIZE + (2 + userBytes.size());
        uint8_t buf3[buflen];
        insertSomeDigest(buf3);
        int offset = FULL_DIGEST_SIZE;
        buf3[offset++] = QObexAuth::UserIdTag;
        buf3[offset++] = userBytes.size() * 2;    // bad size, goes over total size
        memcpy(&buf3[offset], userBytes.constData(), userBytes.size());
        QTest::newRow("bad user length value") << getBytes(buf3, buflen);
    }

    void parseRawResponse_invalid()
    {
        QFETCH(QByteArray, bytes);
        QObexAuthenticationResponse r =
                QObexAuthenticationResponsePrivate::createResponse(QByteArray());
        QVERIFY(!QObexAuthenticationResponsePrivate::parseRawResponse(bytes, r));

        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_requestDigest, QByteArray());
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_nonce, QByteArray());
        QCOMPARE(r.user(), QString());
    }

    void readRawAuthResponse_digest_only()
    {
        int buflen = FULL_DIGEST_SIZE;
        uint8_t buf[buflen];
        QByteArray digest = insertSomeDigest(buf);

        QObexAuthenticationResponse r =
                QObexAuthenticationResponsePrivate::createResponse(QByteArray());
        QVERIFY(QObexAuthenticationResponsePrivate::parseRawResponse(getBytes(buf, buflen), r));

        // check digest is correct
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_requestDigest, digest);

        // check default values for user and nonce
        QCOMPARE(r.user(), QString());
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_nonce, QByteArray());
    }

    void readRawAuthResponse_user_data()
    {
        // we don't know the encoding for user names, so right now 
        // readRawAuthResponse() converts them using toLatin1().

        QTest::addColumn<QString>("userString");
        QTest::addColumn<QByteArray>("userBytes");

        QTest::newRow("empty user") << QString() << QByteArray();

        QString ascii("ascii user name");
        QTest::newRow("ascii user") << ascii << ascii.toAscii();

        static const QChar latin1[] = { 0x00e4, 0x00e6, 0x00ff, 0x00d5, 0x00df };
        QString latin1String(latin1, 5);
        QTest::newRow("ascii user") << latin1String << latin1String.toLatin1();
    }

    void readRawAuthResponse_user()
    {
        QFETCH(QString, userString);
        QFETCH(QByteArray, userBytes);

        int buflen = FULL_DIGEST_SIZE + (2 + userBytes.size());
        uint8_t buf[buflen];
        QByteArray digest = insertSomeDigest(buf);
        insertUser(userBytes, &buf[FULL_DIGEST_SIZE]);

        QObexAuthenticationResponse r =
                QObexAuthenticationResponsePrivate::createResponse(QByteArray());
        QVERIFY(QObexAuthenticationResponsePrivate::parseRawResponse(getBytes(buf, buflen), r));

        QCOMPARE(r.user(), userString);
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_requestDigest, digest);
    }

    /*
    void readRawAuthResponse_nonce_data()
    {
        QTest::addColumn<QByteArray>("insertedNonce");
        QTest::addColumn<QByteArray>("expectedNonce");

        QByteArray invalid("too small");
        QTest::newRow("invalid nonce") << invalid << QByteArray();

        QByteArray ok = QByteArray().fill('a', QObexAuth::NonceSize);
        QTest::newRow("valid nonce") << ok << ok;
    }

    void readRawAuthResponse_nonce()
    {
        QFETCH(QByteArray, insertedNonce);
        QFETCH(QByteArray, expectedNonce);

        int buflen = FULL_DIGEST_SIZE + (2 + insertedNonce.size());
        uint8_t buf[buflen];
        QByteArray digest = insertSomeDigest(buf);

        int offset = FULL_DIGEST_SIZE;
        buf[offset++] = QObexAuth::ResponseNonceTag;
        buf[offset++] = insertedNonce.size();
        memcpy(&buf[offset], insertedNonce.constData(), insertedNonce.size());

        QObexAuthenticationResponse r;
        QVERIFY(QObexAuthenticationResponsePrivate::parseRawResponse(getBytes(buf, buflen), r));

        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_nonce, expectedNonce);
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_requestDigest, digest);
    }
*/
    void parseRawResponse_order_data()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<QByteArray>("digest");
        QTest::addColumn<QString>("user");
        QTest::addColumn<QByteArray>("nonce");

        QString user("test user");
        QByteArray userBytes = user.toAscii();
        int fullUserSize = 2 + userBytes.size();

        QByteArray digest;
        QByteArray nonce;

        int buflen = FULL_DIGEST_SIZE + fullUserSize + FULL_NONCE_SIZE;

        // order by Digest -> User -> Nonce
        uint8_t buf1[buflen];
        digest = insertSomeDigest(buf1);
        insertUser(userBytes, &buf1[FULL_DIGEST_SIZE]);
        nonce = insertSomeNonce(&buf1[FULL_DIGEST_SIZE + fullUserSize]);
        QTest::newRow("usual order") << getBytes(buf1, buflen) << digest << user << nonce;

        // order by User -> Nonce -> Digest
        uint8_t buf2[buflen];
        insertUser(userBytes, buf2);
        nonce = insertSomeNonce(&buf2[fullUserSize]);
        digest = insertSomeDigest(&buf2[fullUserSize + FULL_NONCE_SIZE]);
        QTest::newRow("different order 1") << getBytes(buf2, buflen) << digest << user << nonce;

        // order by Nonce -> User -> Digest
        uint8_t buf3[buflen];
        nonce = insertSomeNonce(buf3);
        insertUser(userBytes, &buf3[FULL_NONCE_SIZE]);
        digest = insertSomeDigest(&buf3[FULL_NONCE_SIZE + fullUserSize]);
        QTest::newRow("different order 2") << getBytes(buf3, buflen) << digest << user << nonce;
    }

    void parseRawResponse_order()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(QByteArray, digest);
        QFETCH(QString, user);
        QFETCH(QByteArray, nonce);

        QObexAuthenticationResponse r =
                QObexAuthenticationResponsePrivate::createResponse(QByteArray());
        QVERIFY(QObexAuthenticationResponsePrivate::parseRawResponse(bytes, r));

        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_requestDigest, digest);
        QCOMPARE(r.user(), user);

        // Nonce is not read in the current implementation.
        QCOMPARE(QObexAuthenticationResponsePrivate::getPrivate(r)->m_nonce,
                 QByteArray());
    }

};

QTEST_MAIN(tst_qobexauthenticationresponse_p)
#include "tst_qobexauthenticationresponse_p.moc"
