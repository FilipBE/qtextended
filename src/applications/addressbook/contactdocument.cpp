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

#include "contactdocument.h"
#include "contactbrowser.h"
#include "qfielddefinition.h"

#include <QApplication>
#include <QPalette>
#include <QFont>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QUrl>

#include <QTimeString>
#include <qtopialog.h>
#include <QLineEdit>
#include "qcontactmodel.h"
#include "qcontact.h"
#include "qtopiaapplication.h"
#include "qthumbnail.h"
#include <QDL>

#if defined(QTOPIA_TELEPHONY)
#include "qcontent.h"
#endif

#if defined(QTOPIA_VOIP)
#include <QCollectivePresence>
#include <QCollectivePresenceInfo>
#endif

class ContactAnchorData {
    public:
        ContactDocument::ContactAnchorType type;
        QContactModel::Field field;
        QString fieldId;
        QString number;
};

/* ContactDocument */
ContactDocument::ContactDocument(QObject* parent)
    : QObject(parent)
#ifdef QTOPIA_VOIP
    , mPresenceProvider(0)
#endif
{
#if defined(QTOPIA_TELEPHONY)
    bDialer = true;
#else
    bDialer = false;
#endif

#ifdef QTOPIA_VOIP
    voipDialer = bDialer;
#else
    voipDialer = false;
#endif
    mRtl = qApp->layoutDirection() == Qt::RightToLeft;
    mDocument = new QTextDocument(this);
    mDocument->setUndoRedoEnabled(false);
    mModel = new QContactModel(this);
}

ContactDocument::~ContactDocument()
{
#ifdef QTOPIA_VOIP
    delete mPresenceProvider;
#endif
}

void ContactDocument::init( QWidget *widget, const QContact& contact, ContactDocumentType docType)
{
    mContact = contact;

    QPalette thisPalette = qApp->palette(widget);
    QFont defaultFont = QApplication::font(widget);
    cfNormal.setFont(defaultFont);

    cfBold = cfNormal;
    cfItalic = cfNormal;
    cfSmall = cfNormal;
    cfSmallBold = cfNormal;
    cfBoldUnderline = cfNormal;

    cfItalic.setFontItalic(true);

    cfBold.setFontWeight(80);

    cfBoldUnderline.setFontWeight(80);
    cfBoldUnderline.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    cfSmall.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    cfSmallBold.setFontWeight(80);
    cfSmallBold.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    bfCenter.setAlignment(Qt::AlignHCenter);
    bfNormal.setAlignment(Qt::AlignLeft);

    mIconHeight = (int) (0.75 * widget->style()->pixelMetric(QStyle::PM_SmallIconSize));

    mDocument->clear();
    mFields.clear();

    switch(docType) {
        case Details:
            createContactDetailsDocument();
            break;
    }
}

void ContactDocument::createContactDetailsDocument()
{
    QTextCursor curs(mDocument);

    QString value;

    /* add thumbnail and name */
    curs.setBlockFormat(bfCenter);

    /* check if we need to have a thumbnail */
    QPixmap thumb = mContact.thumbnail();

    QString nameHtml = "<table border=0 padding=4 spacing=0 align=center><tr>%1<td valign=middle>" + nameFragment() + "</td>%2</table>";
    QString thumbHtml;
    QString meHtml;

    if( !thumb.isNull() ) {
        QVariant thumbV = thumb;
        QTextImageFormat img;
        mDocument->addResource(QTextDocument::ImageResource, QUrl("addressbookdetailthumbnail"), thumbV);
        img.setName("addressbookdetailthumbnail"); // No tr

        thumbHtml = "<td><img src='addressbookdetailthumbnail'></td>";
    }

    if (mModel->isPersonalDetails(mContact.uid())) {
        QVariant detailsV = QIcon(":icon/addressbook/personaldetails").pixmap(QContact::thumbnailSize());
        QTextImageFormat img;
        mDocument->addResource(QTextDocument::ImageResource, QUrl("addressbookpersonaldetails"), detailsV);
        img.setName("addressbookpersonaldetails"); // No tr

        meHtml = "<td><img src='addressbookpersonaldetails'></td>";
    }

    if (mRtl)
        curs.insertHtml(nameHtml.arg(meHtml, thumbHtml));
    else
        curs.insertHtml(nameHtml.arg(thumbHtml, meHtml));

    //  Job (if this is a business contact)
    bool isBus = mContact.categories().contains( "Business" ); // no tr

    if( isBus )
    {
        curs.insertBlock(bfCenter);
        value = mContact.jobTitle();
        if ( !value.isEmpty() )
            curs.insertText(value + " ", cfSmall);

        value = mContact.company();
        if( !value.isEmpty() ) {
            curs.insertText(value, cfSmallBold);
        }
        curs.movePosition(QTextCursor::NextBlock);
    }

    curs.insertHtml("<br>");

    if (isBus) {
        addFieldFragments(curs, "phone business -fax");
        addFieldFragments(curs, "phone -business -fax");
    } else {
        addFieldFragments(curs, "phone -business -fax");
        addFieldFragments(curs, "phone business -fax");
    }

    addEmailFragment( curs );

    addFieldFragments(curs, "chat -phone");

    if (isBus) {
        addFieldFragments(curs, "phone business fax");
        addFieldFragments(curs, "phone -business fax");
    } else {
        addFieldFragments(curs, "phone -business fax");
        addFieldFragments(curs, "phone business fax");
    }

    // XXX For now, no 'Messaging' support for faxes,
    // XXX but it's completely feasible to add.

    if( isBus ) {
        addBusinessFragment( curs );
        addPersonalFragment( curs );
    } else {
        addPersonalFragment( curs );
        addBusinessFragment( curs );
    }

    // Add groups
    QStringList cats = mContact.categories();

    if (cats.count() > 0) {
        addTextBreak(curs);
        curs.insertBlock(bfCenter);
        curs.insertText(qApp->translate("AbFullEditor", "Groups"), cfBoldUnderline);
        addTextBreak(curs);
        QStringList catLabels;
        QCategoryManager cm("Address Book"); // no tr
        foreach (QString cat, cats) {
            catLabels << Qt::escape(cm.label(cat));
        }
        curs.insertHtml(catLabels.join(", ")); // ### tr needed?
    }

    // Notes
    value = mContact.notes();
    if ( !value.isEmpty() )
    {
        // add a Notes: header
        addTextBreak(curs);
        curs.insertBlock(bfCenter);
        curs.insertText(qApp->translate("AbFullEditor", "Notes"), cfBoldUnderline);
        curs.insertBlock(bfNormal);
        curs.insertHtml(value);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addFieldFragments(QTextCursor &curs, const QString &tags)
{
    QStringList fields = QContactFieldDefinition::fields(tags);

    foreach (QString field, fields) {
        QContactFieldDefinition def(field);
        QString value = def.value(mContact).toString();
        if (!value.isEmpty())
            addFieldFragment(curs, def, value);
    }
}

void ContactDocument::addFieldFragment(QTextCursor &curs, const QContactFieldDefinition &def, const QString &fieldValue)
{
    QString value(fieldValue);
    QString img = def.iconString();

    QString escvalue = Qt::escape(value);
    curs.insertBlock(bfCenter);

    QString innerHtml = QString(mRtl ? "%2<img src='cached%1'>" : "<img src='cached%1'>%2").arg(img, value);
    /* Use HTML for anchors to pick up stylesheet stuff */

    QString fieldKey = QString("contactdocument:") + QString::number(mFields.count()); // no tr
    ContactAnchorData *cfd = 0;
    cfd = new ContactAnchorData;
    cfd->type = CustomLink;
    cfd->fieldId = def.id();
    cfd->number = escvalue;
    mFields.insert(fieldKey, cfd);

    QString text("<a href='%1'>%2</a>");
    curs.insertHtml(text.arg(fieldKey).arg(innerHtml));

    curs.movePosition(QTextCursor::NextBlock);
    addCachedPixmap(QString("cached") + img, img);
#if defined(QTOPIA_VOIP)
    bool isVoip = def.hasTag("voip");

    if (isVoip) {
        if ( !mPresenceProvider ) {
            mPresenceProvider = new QCollectivePresence( "voip", this );
            connect(mPresenceProvider, SIGNAL(peerPresencesChanged(QStringList)), this, SLOT(peerPresencesChanged(QStringList)));
        }

        QCollectivePresenceInfo::PresenceType presence = QCollectivePresenceInfo::None;
        if (mPresenceProvider->available()) {
            QCollectivePresenceInfo info = mPresenceProvider->peerInfo(fieldValue);
            if (!info.isNull())
                presence = mPresenceProvider->statusTypes()[info.presence()];
        }

        QString path;
        if ( presence == QCollectivePresenceInfo::None )
            path = ":icon/presence-none";
        else if ( presence == QCollectivePresenceInfo::Online )
            path = ":icon/presence-online";
        else if ( presence == QCollectivePresenceInfo::Offline )
            path = ":icon/presence-offline";
        else
            path = ":icon/presence-away";
        addCachedPixmap( QString::number( presence ) + "addressbookvoipicon", path );
        curs.insertHtml( QString("<img src='%1'>").arg( QString::number( presence ) + "addressbookvoipicon" ) );
    }
#endif
}

void ContactDocument::addCachedPixmap(const QString& url, const QString& path)
{
    QPixmap pm = mCachedPixmaps.value(url);
    if (pm.isNull()) {
        QIcon icon(path);
        pm = icon.pixmap(mIconHeight);
        if (!pm.isNull()) {
            mCachedPixmaps.insert(url, pm);
        }
    }

    if (!pm.isNull() && mDocument->resource(QTextDocument::ImageResource, url).isNull())
        mDocument->addResource(QTextDocument::ImageResource, url, pm);
}

QString ContactDocument::nameFragment()
{
    QString value;
    QString ret;

    //  Name
    // attempt full name first.
    QStringList nameList;
    if (!mContact.nameTitle().isEmpty())
        nameList.append(mContact.nameTitle());
    if (!mContact.firstName().isEmpty())
        nameList.append(mContact.firstName());
    if (!mContact.middleName().isEmpty())
        nameList.append(mContact.middleName());
    if (!mContact.lastName().isEmpty())
        nameList.append(mContact.lastName());
    if (!mContact.suffix().isEmpty())
        nameList.append(mContact.suffix());

    if (!nameList.isEmpty()) {
        value = nameList.join(QChar(' '));
        ret = "<b>" + value + "</b>";
    } else if ( !(value = mContact.label()).isEmpty() ) {
        ret = "<b>" + value + "</b>";
    }

    //  Also, name pronunciation
    if ( !mContact.firstNamePronunciation().isEmpty() || !mContact.lastNamePronunciation().isEmpty()) {
        value += "<br><i>\"" + mContact.firstNamePronunciation();
        if (!mContact.firstNamePronunciation().isEmpty() && !mContact.lastNamePronunciation().isEmpty())
            value += ' ';
        value += mContact.lastNamePronunciation() + "\"</i>";
    }
    return value;
}

void ContactDocument::addEmailFragment( QTextCursor &curs )
{
    QStringList emails = mContact.emailList();
    QStringList::Iterator it;
    if (emails.count() > 0)
        addCachedPixmap("cachediconemail", ":icon/email");
    for( it = emails.begin() ; it != emails.end() ; ++it )
    {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty())
        {
            QString fieldKey = QString("contactdocument:") + QString::number(mFields.count()); // no tr
            ContactAnchorData *cfd = new ContactAnchorData;
            cfd->type = EmailLink;
            cfd->number = Qt::escape(trimmed);
            cfd->field = QContactModel::Emails;
            mFields.insert(fieldKey, cfd);

            curs.insertBlock(bfCenter);
            QString innerHtml = QString(mRtl ? "%1<img src='cachediconemail'>" : "<img src='cachediconemail'>%1").arg(trimmed);
            QString text("<a href='" + fieldKey + "'>" + innerHtml+ "</a>");
            curs.insertHtml(text);
            curs.movePosition(QTextCursor::NextBlock);
        }
    }
}


#if defined(QTOPIA_VOIP)
void ContactDocument::peerPresencesChanged(const QStringList &uris)
{
    if (uris.contains(mContact.businessVOIP()) || uris.contains(mContact.homeVOIP())) {
        mDocument->clear();
        createContactDetailsDocument();
    }
}
#endif

void ContactDocument::addBusinessFragment( QTextCursor &curs )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    if (mRtl) {
        curs.insertText(qApp->translate( "QtopiaPim", "Business Details"), cfBoldUnderline);
        curs.insertImage(":icon/addressbook/business");
    } else {
        curs.insertImage(":icon/addressbook/business");
        curs.insertText(qApp->translate( "QtopiaPim", "Business Details"), cfBoldUnderline);
    }
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    /* And add stuff */
    addTextLine(curs, mContact.jobTitle(), cfNormal, bfCenter, cfNormal);
    addTextLine(curs, mContact.department(), cfItalic, bfCenter, cfNormal);

    value = mContact.company();
    value2 = mContact.companyPronunciation();

    if ( !value.isEmpty() || !value2.isEmpty()) {
        curs.insertBlock(bfCenter);
        if ( !value.isEmpty()) {
            curs.insertText(value, cfBold);
            curs.insertText(" ", cfNormal);
        }
        if ( !value2.isEmpty() )
            curs.insertText("\"" + value2 + "\"", cfItalic);
        curs.movePosition(QTextCursor::NextBlock);
    }

    if ( !mContact.businessStreet().isEmpty() || !mContact.businessCity().isEmpty() ||
         !mContact.businessZip().isEmpty() || !mContact.businessCountry().isEmpty() ) {

        // glom the city/state together
        value = mContact.businessCity();
        value2 = mContact.businessState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextBreak(curs);
        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.businessCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
        mContact.businessWebpage(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Office: "), cfBold,
        mContact.office(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Profession: "), cfBold,
        mContact.profession(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Assistant: "), cfBold,
        mContact.assistant(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Manager: "), cfBold,
        mContact.manager(), cfNormal, bfNormal, cfNormal);

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

void ContactDocument::addPersonalFragment( QTextCursor& curs )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    if (mRtl) {
        curs.insertText(qApp->translate( "QtopiaPim", "Personal Details"), cfBoldUnderline);
        curs.insertImage(":icon/home");
    } else {
        curs.insertImage(":icon/home");
        curs.insertText(qApp->translate( "QtopiaPim", "Personal Details"), cfBoldUnderline);
    }
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    // home address
    if ( !mContact.homeStreet().isEmpty() || !mContact.homeCity().isEmpty() ||
         !mContact.homeZip().isEmpty() || !mContact.homeCountry().isEmpty() ) {
        // glom the city/state together
        value = mContact.homeCity();
        value2 = mContact.homeState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, mContact.homeCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
            mContact.homeWebpage(), cfNormal, bfNormal, cfNormal);

    if ( mContact.gender() != QContact::UnspecifiedGender ) {
        if ( mContact.gender() == QContact::Male )
            str = qApp->translate( "QtopiaPim", "Male" );
        else if ( mContact.gender() == QContact::Female)
            str = qApp->translate( "QtopiaPim", "Female" );
        else
            str = QString();
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Gender: "), cfBold,
                str, cfNormal, bfNormal, cfNormal);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Spouse: "), cfBold,
            mContact.spouse(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Children:") + " ", cfBold,
            mContact.children(), cfNormal, bfNormal, cfNormal);

    if ( mContact.birthday().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Birthday: "), cfBold,
            QTimeString::localYMD( mContact.birthday() ), cfNormal, bfNormal, cfNormal);
    }
    if ( mContact.anniversary().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Anniversary: "), cfBold,
            QTimeString::localYMD( mContact.anniversary()), cfNormal, bfNormal, cfNormal);
    }

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

void ContactDocument::addTextBreak( QTextCursor &curs)
{
    curs.insertBlock();
    curs.movePosition(QTextCursor::NextBlock);
}

void ContactDocument::addTextLine( QTextCursor& curs, const QString& text, const QTextCharFormat& cf,
                                   const QTextBlockFormat &bf, const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(text, cf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addImageAndTextLine ( QTextCursor& curs, const QTextImageFormat& imf,
                                            const QString& text, const QTextCharFormat& cf, const QTextBlockFormat& bf,
                                            const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        if (mRtl) {
            curs.insertText(text, cf);
            curs.insertImage(imf);
        } else {
            curs.insertImage(imf);
            curs.insertText(text, cf);
        }
        curs.movePosition(QTextCursor::NextBlock);
    }
}

void ContactDocument::addTextNameValue( QTextCursor& curs, const QString& name,
                                        const QTextCharFormat &ncf, const QString& value, const QTextCharFormat &vcf,
                                        const QTextBlockFormat& bf, const QTextCharFormat& bcf)
{
    if (! value.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(name, ncf);
        curs.insertText(value, vcf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

ContactDocument::ContactAnchorType ContactDocument::getAnchorType(const QString& href)
{
    ContactAnchorType cType = None;

    if (href.startsWith("contactdocument:")) {
        ContactAnchorData *cfd = mFields.value(href);
        if (cfd)
            cType = cfd->type;
    } else if (href.startsWith(QDL::ANCHOR_HREF_PREFIX)) {
        cType = QdlLink;
    }

    return cType;
}

QString ContactDocument::getAnchorTarget(const QString &href)
{
    if (href.startsWith("contactdocument:")) {
        ContactAnchorData *cfd = mFields.value(href);
        if (cfd)
            return cfd->number;
    } else if (href.startsWith(QDL::ANCHOR_HREF_PREFIX)) {
        return href;
    }
    return QString();
}

QString ContactDocument::getAnchorField(const QString &href)
{
    if (href.startsWith("contactdocument:")) {
        ContactAnchorData *cfd = mFields.value(href);
        if (cfd)
        {
            if (!cfd->fieldId.isEmpty())
                return cfd->fieldId;
            return QContactModel::fieldIdentifier(cfd->field);
        }
    }
    return QString();
}
