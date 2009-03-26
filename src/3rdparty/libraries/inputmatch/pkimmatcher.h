/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** WARNING: Use of this file may require additional third party patent licensing.
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

#ifndef PKIMMATCHER_H
#define PKIMMATCHER_H

#include <qdawg.h>
#include <qmap.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qvector.h>
#include <limits.h>

#include "inputmatchglobal.h"

struct IMIGuess {
    IMIGuess() : c(0x0000ffff), length(0), error(0) {}
    uint c;
    int length;
    uint error;
};

class QTranslatableSettings;

QChar convertToUpper(const QChar& letter);

enum InputMatcherFunc {
    noFunction = 0,
    changeShift,
    changeMode,
    modifyText,
    insertSpace,
    insertSymbol,
    insertText,
    changeInputMethod
};

struct InputMatcherChar {
    QChar id;
    InputMatcherFunc tapfunc;
    QString taparg;
    InputMatcherFunc holdfunc; 
    QString holdarg;
    bool showList;
};

class InputMatcherGuessList : public QList<IMIGuess>
{
public:
    InputMatcherGuessList() : QList<IMIGuess>(), shift(FALSE) {}
    InputMatcherGuessList(const QList<IMIGuess> &o) : QList<IMIGuess>(o), shift(FALSE) { }
    InputMatcherGuessList(const InputMatcherGuessList &o) : QList<IMIGuess>(), shift(FALSE)
    {
	InputMatcherGuessList::ConstIterator it;
	for (it = o.begin(); it != o.end(); ++it)
	    append(*it);
	shift = o.shift;
    }

    InputMatcherGuessList(const QString &s, bool c) : QList<IMIGuess>(), shift(c)
    {
	for (int i = 0; i < s.length(); ++i) {
	    IMIGuess g;
	    g.c = s[i].unicode();
	    g.length = 1;
	    g.error = 0; // equal with all others in this set.
	    append(g);
	}
    }

    InputMatcherGuessList &operator=(const QList<IMIGuess> &o) {
	QList<IMIGuess>::operator=(o);
	shift = FALSE;
	return *this;
    }

    InputMatcherGuessList &operator=(const InputMatcherGuessList &o)
    {
	InputMatcherGuessList::ConstIterator it;
	for (it = o.begin(); it != o.end(); ++it)
	    append(*it);
	shift = o.shift;
	return *this;
    }

    bool contains(const QChar &c) const
    {
	InputMatcherGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).c == c.unicode() || (*it).c == c.toLower().unicode())
		return TRUE;
	}
	return FALSE;
    }

    IMIGuess find(const QChar &c) const
    {
	InputMatcherGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).c == c.unicode() || (*it).c == c.toLower().unicode())
		return (*it);
	}
	IMIGuess g;
	g.c = 0xffff;
	g.length = 0;
	g.error = UINT_MAX;
	return g;
    }

    int longest() const {
	int min = 0;
	InputMatcherGuessList::ConstIterator it;
	for (it = begin(); it != end(); ++it) {
	    if ((*it).length > min)
		min = (*it).length;
	}
	return min;
    }

    bool shift;
};

struct InputMatcherWordError {
    QString text;
    uint error;
};

class InputMatcherWordErrorList : public QList<InputMatcherWordError>
{
public:
    InputMatcherWordErrorList() : QList<InputMatcherWordError>() {}
    ~InputMatcherWordErrorList() {}
    QStringList asStringList() const {
	QStringList r;
	QList<InputMatcherWordError>::ConstIterator it;
	for (it = QList<InputMatcherWordError>::begin();
		it != QList<InputMatcherWordError>::end(); ++it) {
	    r.append((*it).text);
	}

	return r;
    }

    bool contains(const QString &item) const {
	InputMatcherWordErrorList::ConstIterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).text == item)
		return TRUE;
	}
	return FALSE;
    }

    void insert(const InputMatcherWordError &item)
    {
	InputMatcherWordErrorList::Iterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).error > item.error) {
		QList<InputMatcherWordError>::insert(i, item);
		return;
	    }
	}
	append(item);
    }

    void remove(const QString &item)
    {
	InputMatcherWordErrorList::Iterator i;
	for ( i=begin(); i!=end(); ++i) {
	    if ((*i).text == item) {
		QList<InputMatcherWordError>::erase(i);
		return;
	    }
	}
    }

    int merge(const InputMatcherWordErrorList &o) {
	uint min = UINT_MAX;
	InputMatcherWordErrorList::ConstIterator i;
	for ( i=o.begin(); i!=o.end(); ++i) {
	    InputMatcherWordError we = *i;
	    if ( !contains(we.text) ) {
		min = qMin(min, we.error);
		insert(we);
	    }
	}
	return min;
    }
};

/* what does it take to turn matcher into mode. */
class QTOPIA_INPUTMATCH_EXPORT InputMatcher {
public:
    InputMatcher( );
    InputMatcher( const QString & );

    QString collate(const QString &) const;

    QString id() const {return mId;}
    bool isValid() const { return !mId.isNull(); }

    bool isNamedMode() const { return mIndex < 0; }
    QString replaces() const;
    bool guessCompatible() const;

    bool lookup() const { return mLookup; }

    QPixmap pixmap(bool ucase=FALSE) const; // depends on name

    void reset();

    // hide later?
    QMap<QChar, InputMatcherChar> map() const;

    QStringList allChoices() const;
    QStringList choices() const;
    QStringList choices(bool allowPartial, bool allowPredicted) const;
    QStringList choices(bool allowPartial, bool allowPredicted, const QString &start, const QString &end) const;

    QStringList findAll() const;
    QStringList findAll(const QString &prefix, const QString &suffix) const;

    QString numberWord(const QString &prefix = QString(), const QString &suffix = QString()) const;
    QString writtenWord(const QString &prefix = QString(), const QString &suffix = QString()) const;

    static QStringList languages();
    static QStringList chosenLanguages();

    int count() const;

    QString at(int) const;
    QString atReverse(int) const;

    bool push(const QChar&, bool = FALSE); // digit

    // reset, then multi push...
    bool match(const QString&); // num string

    // push set
    void pushSet(const QString&, bool = FALSE);
    // reset, then multi push set., makes more sense if QStringList. 
    void matchSet(const QStringList&);
    void matchSet(const QString&);

    void pushGuessSet( const InputMatcherGuessList &);
   // search for pattern in word, starting at offset in pattern for
   // length in pattern. 
    int match(const QString& word, int offset, const QString& pattern, int length);
    // how is this different to revert?
    InputMatcherGuessList pop();

    static void readConfig(const QTranslatableSettings& cfg, QMap<QChar,InputMatcherChar>& set);
private:

    int prefixLength(const QString &, int f = 0) const;
    int suffixLength(const QString &, int f = 0) const;
    QStringList searchDict(bool allowprefix, bool predict, const QString& lang, const QStringList& extradict, int start, int len , uint *merror = 0) const;

    InputMatcherWordErrorList findWords(const QDawg::Node* node, int set, int maxset, const QString& str, uint error, bool allowprefix, bool predict) const;

    QStringList findAll(int set, int maxset, const QString& str) const;
    void init();

    QString findBest(const QString &prefix, const QString &suffix, bool digit) const;
    QString findBest(int set, int maxset, bool digit, const QString & = QString()) const;

    QString mId;
    int mIndex;
    int mReplaces;
    bool mLookup;
    QString dict;
    QStringList extradict;
    bool lowerdict;

    QList<InputMatcherGuessList*> nsets;
    int longestEnd;
    bool qualifiedSets;

    QMap<QChar, InputMatcherChar> extendedset;

    static QStringList langs;
};

// tasks... 
class QTOPIA_INPUTMATCH_EXPORT InputMatcherSet : public QObject
{
    Q_OBJECT
public:
    // also loads up all known modes... or
    // maybe just caches them.
    InputMatcherSet(QObject *parent);
    ~InputMatcherSet();

    // if named, will repace named mode.
    InputMatcher *setCurrentMode(const QString &);
    InputMatcher *setHintedMode(const QString &);
    void clearHintedMode();
    InputMatcher *namedMode() const { return named; }
    void clearNamedMode();

    InputMatcher *currentMode() const;

    void nextMode();
    void toggleHinted();

    QStringList modes();
    QStringList guessCompatibleModes();
    QPixmap pixmapFor(const QString &, bool ucase=FALSE);

signals:
    void modeChanged(InputMatcher *);

private:
    void checkDictionaries();
    void populate();

    InputMatcher *mode(const QString &);
    // this has...
    QList<InputMatcher*> mModes;
    InputMatcher *named;
    InputMatcher *current;
    InputMatcher *hintedMode;
};

#endif
