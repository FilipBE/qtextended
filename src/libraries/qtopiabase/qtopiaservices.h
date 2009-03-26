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
#ifndef QTOPIASERVICES_H
#define QTOPIASERVICES_H

#include <qtopiaglobal.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <QVariant>
#include <qtopiaipcmarshal.h>

class QTOPIABASE_EXPORT QtopiaService {
public:
    static QStringList list();
    static QString binding(const QString& service);
    static QStringList apps(const QString& service);
    static QString app(const QString& service, const QString& appname=QString());
    static QString appConfig(const QString& service, const QString& appname=QString());
    static QString config(const QString& service);
    static QStringList channels(const QString& service);
    static QString channel(const QString& service, const QString& appname=QString());
    static QStringList services(const QString& appname);
};

class QTOPIABASE_EXPORT QtopiaServiceRequest
{
    friend bool operator==( const QtopiaServiceRequest &m1, const QtopiaServiceRequest &m2 );

public:
    QtopiaServiceRequest();
    QtopiaServiceRequest(const QString& service, const QString& message);
    QtopiaServiceRequest(const QtopiaServiceRequest& orig);
    ~QtopiaServiceRequest();

    bool send() const;
    bool isNull() const;

    void setService(const QString &service);
    QString service() const { return m_Service; }
    void setMessage(const QString& message);
    QString message() const { return m_Message; }

    const QList<QVariant> &arguments() const { return m_arguments; }
    void setArguments(const QList<QVariant> &arguments) { m_arguments = arguments; }

    QtopiaServiceRequest& operator=(const QtopiaServiceRequest& orig);
    template<typename T>
    inline QtopiaServiceRequest &operator<< (const T &var)
    {
        QVariant v = qVariantFromValue(var);
        m_arguments.append(v);
        return *this;
    }

    inline QtopiaServiceRequest &operator<< (const char *var)
    {
        QVariant v = QVariant(QString(var));
        m_arguments.append(v);
        return *this;
    }

    inline void addVariantArg(const QVariant& var)
    {
        m_arguments.append(var);
    }

    static QByteArray serializeArguments(const QtopiaServiceRequest &action);
    static void deserializeArguments(QtopiaServiceRequest &action, const QByteArray &data);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QList<QVariant> m_arguments;
    QString m_Service;
    QString m_Message;
};

inline bool operator==( const QtopiaServiceRequest &m1, const QtopiaServiceRequest &m2 )
{
    return (m1.service() == m2.service()) && (m1.message() == m2.message())
            && (m1.m_arguments == m2.m_arguments);
}

Q_DECLARE_USER_METATYPE(QtopiaServiceRequest)

#endif
