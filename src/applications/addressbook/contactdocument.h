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
#ifndef CONTACTDOCUMENT_H
#define CONTACTDOCUMENT_H

#include <QObject>
#ifdef QTOPIA_VOIP
class QCollectivePresence;
#endif
#include <qtopiaservices.h>
#include <QTextCharFormat>
#include <QContact>
#include <QContactModel>
#include "qtopiaapplication.h"
#include <QHash>

class QTextDocument;
class QTextCursor;
class QWidget;
class ContactAnchorData;
class QContactFieldDefinition;
class QStringList;
#if defined(QTOPIA_TELEPHONY)
class QContent;
#endif


class ContactDocument : public QObject
{
    Q_OBJECT

public:
    ContactDocument(QObject *parent);
    virtual ~ContactDocument();

    typedef enum {Details} ContactDocumentType;
    typedef enum {None, DialLink, EmailLink, QdlLink, CustomLink} ContactAnchorType;

    void init(QWidget *widget, const QContact& contact, ContactDocumentType docType);

    QTextDocument* textDocument() const { return mDocument; }
    QContact contact() const { return mContact; }

    ContactAnchorType getAnchorType(const QString& href);
    QString getAnchorTarget(const QString &href);
    QString getAnchorField(const QString &href);

signals:
    void externalLinkActivated();

protected:
    QContact mContact;
    QTextDocument *mDocument;
    bool bDialer;
    bool voipDialer;
    bool mRtl;

    // member formats
    QTextCharFormat cfNormal;
    QTextCharFormat cfItalic;
    QTextCharFormat cfBold;
    QTextCharFormat cfBoldUnderline;
    QTextCharFormat cfSmall;
    QTextCharFormat cfSmallBold;

    QTextBlockFormat bfNormal;
    QTextBlockFormat bfCenter;

    QMap<QString,ContactAnchorData *> mFields;

    typedef enum {NoLink = 0, Dialer, Email} LinkType;

    int mIconHeight;

    // Document helpers
    void createContactDetailsDocument();

    // Fragment helpers
    void addFieldFragments(QTextCursor &curs, const QString &tags);
    void addFieldFragment( QTextCursor& outCurs, const QContactFieldDefinition &, const QString &);

    QString nameFragment();
    void addBusinessFragment( QTextCursor &outCurs );
    void addPersonalFragment( QTextCursor &outCurs );
    void addEmailFragment( QTextCursor &outCurs );

    void addCachedPixmap(const QString& url, const QString& path);

    // QTextCursor helpers
    void addTextBreak( QTextCursor &curs);
    void addTextLine( QTextCursor& curs, const QString& text, const QTextCharFormat& cf,
                      const QTextBlockFormat &bf, const QTextCharFormat& bcf);
    void addImageAndTextLine ( QTextCursor& curs, const QTextImageFormat& imf,
                          const QString& text, const QTextCharFormat& cf, const QTextBlockFormat& bf,
                          const QTextCharFormat& bcf);
    void addTextNameValue( QTextCursor& curs, const QString& name,
                      const QTextCharFormat &ncf, const QString& value, const QTextCharFormat &vcf,
                      const QTextBlockFormat& bf, const QTextCharFormat& bcf);

#ifdef QTOPIA_VOIP
    QCollectivePresence *mPresenceProvider;
#endif

    QHash<QString, QPixmap> mCachedPixmaps;

    QtopiaServiceRequest anchorService(ContactAnchorData *cfd) const;

    QContactModel *mModel;

    private slots:
#ifdef QTOPIA_VOIP
    void peerPresencesChanged(const QStringList &uri);
#endif

};

#endif
