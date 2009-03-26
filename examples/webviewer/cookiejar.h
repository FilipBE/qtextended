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

#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QtNetwork/QNetworkCookieJar>
#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QDialog>
#include <QtGui/QTableView>

class QSortFilterProxyModel;
class QKeyEvent;
class AutoSave;

class CookieJar : public QNetworkCookieJar
{
    friend class CookieModel;
    Q_OBJECT
    Q_PROPERTY(AcceptPolicy acceptPolicy READ acceptPolicy WRITE setAcceptPolicy)
    Q_PROPERTY(KeepPolicy keepPolicy READ keepPolicy WRITE setKeepPolicy)
    Q_PROPERTY(QStringList blockedCookies READ blockedCookies WRITE setBlockedCookies)
    Q_PROPERTY(QStringList allowedCookies READ allowedCookies WRITE setAllowedCookies)
    Q_PROPERTY(QStringList allowForSessionCookies READ allowForSessionCookies WRITE setAllowForSessionCookies)
    Q_ENUMS(KeepPolicy)
    Q_ENUMS(AcceptPolicy)

signals:
    void cookiesChanged();

public:
    enum AcceptPolicy {
        AcceptAlways,
        AcceptNever,
        AcceptOnlyFromSitesNavigatedTo
    };

    enum KeepPolicy {
        Expire,
        Exit,
        TimeLimit
    };

    CookieJar(QObject *parent = 0);
    ~CookieJar();

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    AcceptPolicy acceptPolicy() const;
    void setAcceptPolicy(AcceptPolicy policy);

    KeepPolicy keepPolicy() const;
    void setKeepPolicy(KeepPolicy policy);

    QStringList blockedCookies() const;
    QStringList allowedCookies() const;
    QStringList allowForSessionCookies() const;

    void setBlockedCookies(const QStringList &list);
    void setAllowedCookies(const QStringList &list);
    void setAllowForSessionCookies(const QStringList &list);

public slots:
    void clear();

private slots:
    void save();

private:
    void purgeOldCookies();
    void load();
    bool m_loaded;
    AutoSave *m_saveTimer;

    AcceptPolicy m_acceptCookies;
    KeepPolicy m_keepCookies;

    QStringList m_exceptions_block;
    QStringList m_exceptions_allow;
    QStringList m_exceptions_allowForSession;
};

class CookieModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CookieModel(CookieJar *jar, QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private slots:
    void cookiesChanged();

private:
    CookieJar *m_jar;
};

#include "ui_cookies.h"
#include "ui_cookiesexceptions.h"

class CookiesDialog : public QDialog, public Ui_CookiesDialog
{
    Q_OBJECT

public:
    CookiesDialog(CookieJar *cookieJar, QWidget *parent = 0);

private:
    QSortFilterProxyModel *m_proxyModel;
};

class CookieExceptionsModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class CookiesExceptionsDialog;

public:
    CookieExceptionsModel(CookieJar *cookieJar, QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    CookieJar *m_jar;

    // Domains we allow, Domains we block, Domains we allow for this session
    QStringList m_allowedCookies;
    QStringList m_blockedCookies;
    QStringList m_sessionCookies;
};

class CookiesExceptionsDialog : public QDialog, public Ui_CookiesExceptionsDialog
{
    Q_OBJECT

public:
    CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent = 0);

private slots:
    void block();
    void allow();
    void allowForSession();

private:
    CookieExceptionsModel *m_model;
    QSortFilterProxyModel *m_proxyModel;
    CookieJar *m_jar;

};

#endif
