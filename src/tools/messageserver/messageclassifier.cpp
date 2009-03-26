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

#include "messageclassifier.h"
#include "qtopialog.h"

#include <QMailMessage>
#include <QSettings>

MessageClassifier::MessageClassifier()
{
    QSettings settings("Trolltech", "messageserver");

    settings.beginGroup("global");

    int count = settings.beginReadArray("voicemail");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        voiceMailAddresses.append(settings.value("address").toString());
    }
    settings.endArray();

    count = settings.beginReadArray("videomail");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        videoMailAddresses.append(settings.value("address").toString());
    }
    settings.endArray();

    settings.endGroup();
}

MessageClassifier::~MessageClassifier()
{
}

static QMailMessage::ContentType fromContentType(const QMailMessageContentType& contentType)
{
    QString type(contentType.type().toLower());
    QString subtype(contentType.subType().toLower());

    QMailMessage::ContentType content = QMailMessage::UnknownContent;

    if (type == "text") {
        if (subtype == "html") {
            content = QMailMessage::HtmlContent;
        } else if (subtype == "plain") {
            content = QMailMessage::PlainTextContent;
        } else if (subtype == "x-vcard") {
            content = QMailMessage::VCardContent;
        } else if (subtype == "x-vcalendar") {
            content = QMailMessage::VCalendarContent;
        }
    } else if (contentType.type().toLower() == "image") {
        content = QMailMessage::ImageContent;
    } else if (contentType.type().toLower() == "audio") {
        content = QMailMessage::AudioContent;
    } else if (contentType.type().toLower() == "video") {
        content = QMailMessage::VideoContent;
    }

    return content;
}

void MessageClassifier::classifyMessage(QMailMessageMetaData& message)
{
    QMailMessage::ContentType content = QMailMessage::UnknownContent;

    switch (message.messageType()) {
    case QMailMessage::Email:
        // Handle voicemail emails, from pre-configured addresses
        if (voiceMailAddresses.contains(message.from().address())) {
            content = QMailMessage::VoicemailContent;
        }
        else if(videoMailAddresses.contains(message.from().address()))
            content = QMailMessage::VideomailContent;
        break;

    default:
        break;
    }

    if (content != QMailMessage::UnknownContent)
        message.setContent(content);
}

void MessageClassifier::classifyMessage(QMailMessage& message)
{
    if (message.content() == QMailMessage::UnknownContent) {
        QMailMessagePartContainer::MultipartType multipartType(message.multipartType());
        QMailMessageContentType contentType(message.contentType());

        // The content type is used to categorise the message more narrowly than 
        // its transport categorisation
        QMailMessage::ContentType content = QMailMessage::UnknownContent;

        switch (message.messageType()) {
        case QMailMessage::Sms:
            content = fromContentType(contentType);
            if (content == QMailMessage::UnknownContent) {
                if (message.hasBody()) {
                    // Assume plain text
                    content = QMailMessage::PlainTextContent;
                } else {
                    // No content in this message beside the meta data
                    content = QMailMessage::NoContent;
                }
            }
            break;

        case QMailMessage::Mms:
            if (multipartType == QMailMessagePartContainer::MultipartNone) {
                content = fromContentType(contentType);
                if (content == QMailMessage::UnknownContent) {
                    if (contentType.type().toLower() == "text") {
                        // Assume some type of richer-than-plain text 
                        content = QMailMessage::RichTextContent;
                    }
                }
            } else {
                if (multipartType == QMailMessagePartContainer::MultipartRelated) {
                    // Assume SMIL for now - we should test for 'application/smil' somewhere...
                    content = QMailMessage::SmilContent;
                } else {
                    content = QMailMessage::MultipartContent;
                }
            }
            break;

        case QMailMessage::Email:
            if (multipartType == QMailMessagePartContainer::MultipartNone) {
                content = fromContentType(contentType);
                if (content == QMailMessage::UnknownContent) {
                    if (contentType.type().toLower() == "text") {
                        // Assume some type of richer-than-plain text 
                        content = QMailMessage::RichTextContent;
                    }
                }
            } else {
                // TODO: Much more goes here...
                content = QMailMessage::MultipartContent;
            }
            break;

        case QMailMessage::System:
            content = QMailMessage::RichTextContent;
            break;

        default:
            break;
        }

        if (content != QMailMessage::UnknownContent)
            message.setContent(content);
    }
}

