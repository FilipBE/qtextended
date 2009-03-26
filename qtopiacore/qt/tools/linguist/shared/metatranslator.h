/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef METATRANSLATOR_H
#define METATRANSLATOR_H

#include "translator.h"
#include <QMap>
#include <QString>
#include <QList>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

class QIODevice;
class QTextCodec;
class QXmlErrorHandler;

class MetaTranslator
{
public:
    MetaTranslator();
    MetaTranslator( const MetaTranslator& tor );

    MetaTranslator& operator=( const MetaTranslator& tor );

    void clear();
    bool load( const QString& filename );
    bool save( const QString& filename ) const;
    bool release( const QString& filename, bool verbose = false,
                  bool ignoreUnfinished = false,
                  Translator::SaveMode mode = Translator::Stripped ) const;
    bool release( QIODevice *iod, bool verbose = false,
                  bool ignoreUnfinished = false,
                  Translator::SaveMode mode = Translator::Stripped ) const;

    bool contains(const QByteArray &context, const QByteArray &sourceText,
        const QByteArray &comment) const;

    TranslatorMessage find(const QByteArray &context,
        const QByteArray &sourceText, const QByteArray &comment) const;

    TranslatorMessage find(const QByteArray &context,
        const QByteArray &comment, const QString &fileName, int lineNumber) const;

    void insert( const TranslatorMessage& m );

    void stripObsoleteMessages();
    void stripEmptyContexts();
    void stripNonPluralForms();
    void stripIdenticalSourceTranslations();
    void makeFileNamesAbsolute();

    void setXmlErrorHandler(QXmlErrorHandler* handler) { xmlErrorHandler = handler; }
    void setCodec( const char *name ); // kill me
    void setCodecForTr( const char *name ) { setCodec(name); }
    QTextCodec *codecForTr() const { return codec; }
    QString toUnicode( const char *str, bool utf8 ) const;

    QString languageCode() const;
    static void languageAndCountry(const QString &languageCode, QLocale::Language *lang, QLocale::Country *country);
    void setLanguageCode(const QString &languageCode);
    QList<TranslatorMessage> messages() const;
    QList<TranslatorMessage> translatedMessages() const;
    static int grammaticalNumerus(QLocale::Language language, QLocale::Country country);
    static QStringList normalizedTranslations(const TranslatorMessage& m,
                    QLocale::Language lang, QLocale::Country country);

private:
    typedef QMap<TranslatorMessage, int> TMM;       // int stores the sequence position.
    typedef QMap<int, TranslatorMessage> TMMInv;    // Used during save operation. Seems to use the map only the get the sequence order right.
    bool saveTS( const QString& filename ) const;
    bool saveXLIFF( const QString& filename) const;

    TMM mm;
    QByteArray codecName;
    QTextCodec *codec;
    QDir m_originalPath;
    QXmlErrorHandler* xmlErrorHandler;

    // A string beginning with a 2 or 3 letter language code (ISO 639-1
    // or ISO-639-2), followed by the optional country variant to distinguish
    //  between country-specific variations of the language. The language code
    // and country code are always separated by '_'
    // Note that the language part can also be a 3-letter ISO 639-2 code.
    // Legal examples:
    // 'pt'         portuguese, assumes portuguese from portugal
    // 'pt_BR'      Brazilian portuguese (ISO 639-1 language code)
    // 'por_BR'     Brazilian portuguese (ISO 639-2 language code)
    QString m_language;
};

/*
  This is a quick hack. The proper way to handle this would be
  to extend MetaTranslator's interface.
*/
#define ContextComment "QT_LINGUIST_INTERNAL_CONTEXT_COMMENT"

bool saveXLIFF( const MetaTranslator &mt, const QString& filename);

QT_END_NAMESPACE

#endif
