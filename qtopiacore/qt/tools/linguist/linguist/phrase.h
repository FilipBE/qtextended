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

#ifndef PHRASE_H
#define PHRASE_H

#include <QObject>
#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE

class PhraseBook;

class Phrase
{
public:
    Phrase();
    Phrase(const QString &source, const QString &target,
            const QString &definition, int sc = -1);
    Phrase(const QString &source, const QString &target,
            const QString &definition, PhraseBook *phraseBook);

    QString source() const { return s; }
    void setSource(const QString &ns);
    QString target() const {return t;}
    void setTarget(const QString &nt);
    QString definition() const {return d;}
    void setDefinition (const QString &nd);
    int shortcut() const { return shrtc; }
    PhraseBook *phraseBook() const { return m_phraseBook; }
    void setPhraseBook(PhraseBook *book) { m_phraseBook = book; }

private:
    int shrtc;
    QString s;
    QString t;
    QString d;
    PhraseBook *m_phraseBook;
};

bool operator==(const Phrase &p, const Phrase &q);
inline bool operator!=(const Phrase &p, const Phrase &q) {
    return !(p == q);
}

class QphHandler;

class PhraseBook : public QObject
{
    Q_OBJECT

public:
    PhraseBook();
    ~PhraseBook();
    bool load(const QString &fileName);
    bool save(const QString &fileName);
    const QList<Phrase *> phrases() { return m_phrases; }
    void append(Phrase *phrase);
    void remove(Phrase *phrase);
    const QString fileName() const { return m_fileName; }
    const QString friendlyPhraseBookName() const;
    bool isModified() { return m_changed; }

signals:
    void modifiedChanged(bool changed);
    void listChanged();

private:
    // Prevent copying
    PhraseBook(const PhraseBook &);
    PhraseBook& operator=(const PhraseBook &);

    void setModified(bool modified);
    void phraseChanged(Phrase *phrase);

    QList<Phrase *> m_phrases;
    QString m_fileName;
    bool m_changed;

    friend class QphHandler;
    friend class Phrase;
};

QT_END_NAMESPACE

#endif
