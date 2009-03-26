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

#ifndef OPENOBEXHEADERPROCESSOR_H
#define OPENOBEXHEADERPROCESSOR_H

#include <QObject>

#include <openobex/obex.h>


class OpenObexClient;
class QTcpServer;
template <class T> class QList;


/*?
    The HeaderValue class represents a single OBEX header value. It provides
    access to the header's ID and data value.

    The data value type can be stored as one of the following:
        - 4 byte
        - 1 byte
        - byte sequence
 */

class HeaderValue
{
private:
    int m_id;
    unsigned int m_size;
    uint8_t *m_bs;

public:
    HeaderValue(int id, unsigned int size);
    ~HeaderValue();

    inline int id() const { return m_id; }
    inline int size() const { return m_size; }

    /* byte sequence data or unicode text */
    void setBuf(const uint8_t *bs);
    void readBuf(uint8_t **buf);

    uint32_t m_bq4; /* 4 byte data */
    uint8_t m_bq1;  /* 1 byte data */


    static void clearList(QList<HeaderValue *> &headerValues);
};


//==========================================


/*?
    The OpenObexHeaderProcessor class allows you to send an OBEX request
    from an OBEX client to an OBEX server, and examine the request that
    is received by the OBEX server.

    This is useful for getting the header values that are inserted into an
    obex_object_t for a client request. That is, there is no way to get the
    inserted header values once you have added them to an obex_object_t in
    a client request, so if you want to examine what was actually inserted,
    you will just have to send them through to an OBEX server and then look
    at the header values that come out at the server end.

    This isn't the ideal solution, since it relies on OpenOBEX to parse the
    headers correctly when it receives them on the server side, but it's
    really not worth writing another OBEX header parser just to do these tests.

    If it was possible to read *and* write headers on the same obex_object_t*
    object, this class would be completely unnecessary (at least for the
    header tests, anyway -- perhaps there are other uses for this class).

    To run an individual test:
    1. Call startNewRequest()
    2. Call one of the add...Header() functions
    3. Call run().
       To see the results (i.e. the headers that are received by the OBEX
       server), either get the results from the receivedHeaders() signal,
       or pass a list pointer to the run() function. If you pass the pointer,
       make sure you delete contents of the list when you are finished;
       HeaderValue::clearList() can do this for you.
*/

class OpenObexHeaderProcessor : public QObject
{
    Q_OBJECT

public:
    OpenObexHeaderProcessor(QObject *parent = 0);
    ~OpenObexHeaderProcessor();

    void startNewRequest();
    void add4ByteHeader(uint8_t hi, uint32_t value);
    void add1ByteHeader(uint8_t hi, uint8_t value);
    void addBytesHeader(uint8_t hi, const uint8_t *bytes, uint size);

    // If results is given, this sets results to the received headers;
    // otherwise, receivedHeaders() is emitted.
    void run(QList<HeaderValue *> *results = 0);

    obex_object_t *currentRequest() const;
    obex_t *clientHandle() const;

    void serviceReceivedRequest(obex_t *handle, obex_object_t *obj);
    static QList<HeaderValue *> listFromObject(obex_t* handle, obex_object_t *obj);

signals:
    // The next request cannot be started until this is emitted.
    void done();

    // If the obj here needs to be parsed (read) more than once,
    // OBEX_ObjectReParseHeaders() must be called to reset the read pointer
    // on the obj.
    void receivedHeaders(obex_t *handle, obex_object_t *obj);

private slots:
    void newConnection();

private:
    OpenObexClient *m_client;
    QTcpServer *m_server;
    QList<HeaderValue *> *m_userResultsStorage;
};


#endif
