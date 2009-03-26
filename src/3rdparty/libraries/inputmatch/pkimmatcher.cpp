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
#include "pkimmatcher.h"
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtranslatablesettings.h>
#include <QPixmapCache>
#include <QPainter>

static QDebug operator<<(QDebug s, const InputMatcherWordError& e)
{
    s << e.text << "(" << e.error << ")";
    return s;
}

// could later move all this info into conf file?
struct MatcherConfig {
    char *name;
    char *buttonDiff;
    bool lookup;
    bool guessCompatible;
    bool hasCase;
    bool extendTextButtons;
};

// only means 'no 0..9 + lookup mode.
// really means.. no 'textbuttons' if in lookup mode.
//#define NO_DICT_MODE

// special characters are not properly converted by QChar::upper()
// of course still misses a lot.
QChar convertToUpper(const QChar& letter)
{
    const short offset = 32; //offset between upper and lower case letter
    if ( letter >= QChar(0x00e0) && letter <= QChar(0x00fe)) // special chars range
	return QChar(letter.unicode() - offset);
    else if ( letter == QChar(0x00ff) )
	return QChar(0x0179);
    return letter.toUpper();
}

MatcherConfig validNames[] =
{
    { "dict", 0, TRUE, TRUE, TRUE, FALSE },
    { "abc", "LocaleTextButtons", FALSE, TRUE, TRUE, TRUE },
    { "ext", 0, FALSE, FALSE, TRUE, FALSE },
    { "phone", "PhoneTextButtons", FALSE, FALSE, FALSE, FALSE },
    { 0, 0, FALSE, FALSE, FALSE, FALSE }
};

static const int null_index = -1;
static const int dict_index = 0;
static const int abc_index = 1;
static const int ext_index = 2;
static const int phone_index = 3;

struct MatcherAlias {
    char *name;
    char *lookup;
};

MatcherAlias aliasLookup[] =
{
    { "extended", "ext" },
    { "words", "dict" },
    { "propernouns", "dict" },
    { "text", "abc" },
    { "int", "phone" },
    { 0, 0 }
};

InputMatcher::InputMatcher() : mIndex(null_index), mReplaces(null_index), mLookup(TRUE), lowerdict(FALSE)
{
    mId = "abc";
    mIndex = abc_index;
    init();
}

InputMatcher::InputMatcher(const QString &n) : mIndex(null_index), mReplaces(null_index), mLookup(TRUE), lowerdict(FALSE)
{
    int index = 0;
    QString tmpName = n;
    while(aliasLookup[index].name != 0) {
	if (tmpName == aliasLookup[index].name) {
	    tmpName = aliasLookup[index].lookup;
	    break;
	}
	index++;
    }
    index = 0;
    while(validNames[index].name != 0) {
	if (tmpName == validNames[index].name) {
	    mId = tmpName;
	    mIndex = index;
	    break;
	}
	index++;
    }

    QStringList cl = chosenLanguages();
    // named dictionary.  mIndex is still dict, but name isn't
    if (tmpName.left(5) == "dict-" && cl.indexOf(tmpName.mid(5)) >= 0) {
	mId = tmpName;
	mIndex = dict_index;
    }

    if (mId.isNull())
	mId = tmpName;
    // init will set mId to null if there is no valid mode of that name
    init();
}

QString InputMatcher::collate(const QString &s) const
{
    QString result;
    for (int i = 0; i < s.length(); i++) {
	QChar c = s[i].toLower();
	QMapIterator<QChar, InputMatcherChar> it(extendedset);
	bool found = false;
	while(it.hasNext()) {
	    it.next();
	    InputMatcherChar ic = it.value();
	    if (ic.tapfunc == insertText && ic.taparg.toLower().contains(c)) {
		found = true;
		result += ic.id;
		break;
	    }
	}
	if (!found)
	    result+= c;
    }
    return result;

}

QString InputMatcher::replaces() const
{
    return validNames[mReplaces].name;
}

QPixmap InputMatcher::pixmap(bool shift) const
{
    QString key;
    QString ul;
    if (mIndex != null_index) {
	if (validNames[mIndex].hasCase)
	    ul = shift ? "-uc" : "-lc";
    } else {
	if (validNames[mReplaces].hasCase)
	    ul = shift ? "-uc" : "-lc";
    }

    QPixmap pm;

    // should be icons for named modes, both lc and uc
    if (isNamedMode()) {
	key = "named_"+mId;
    } else {
        int index = mId.indexOf('-');
        QString modifiedId = mId;
        if (index != -1)
            //We cannot use translatable image lookup because we want
            //to load a translatable image independently from the current locale
            //see difference between :image/pkim/i18n and :image/i18n/pkim
            modifiedId = "i18n/" + mId.mid(index+1) + '/' + mId.left(index);
	key = modifiedId;
    }

    if ( !key.isEmpty() )
	pm = QPixmap(":image/pkim/"+key+ul);

    if ( pm.isNull() && mIndex == dict_index) {
	// was a dictionary but no pixmap found, so make one.
	key = "qpe_"+key;
	if ( !QPixmapCache::find(key,pm) ) {
	    pm = QPixmap(":image/pkim/dict-lang");
	    QPainter p(&pm);
	    QString l = dict.toLower().left(2);
	    if ( shift )
		l[0] = l[0].toUpper();
	    p.drawText(pm.rect(),Qt::AlignCenter,l);
	    QPixmapCache::insert(key,pm);
	}
    }
    if ( pm.isNull()) {
	// try from base type?
	if (mReplaces != null_index) {
	    key = validNames[mReplaces].name;
	    if (!key.isEmpty())
		pm = QPixmap(":image/pkim/"+key+ul);
	}
    }
    return pm;
}

QStringList InputMatcher::findAll() const
{
    return findAll(0, nsets.size(), QString() );
}

QStringList InputMatcher::findAll(const QString &prefix, const QString &suffix) const
{
    int pl = prefixLength(prefix);
    int sl = suffixLength(suffix); 
    QStringList ch;
    if (pl != -1 && sl != -1 && (pl + sl < (int)nsets.size())) {
	ch = findAll(pl, nsets.size()-sl, QString());
    }
    QStringList::Iterator it;
    for (it = ch.begin(); it != ch.end(); ++it) {
	*it = prefix + *it + suffix;
    }
    return ch;
}

QStringList InputMatcher::choices() const
{
    return choices(TRUE, FALSE, QString(), QString());
}

QStringList InputMatcher::allChoices() const
{
    struct WordWraps
    {
	const char *prefix;
	const char *postfix;
    };

    // TODO should get from config file.
    WordWraps wraps[] = {
	{ 0, "'s"},
	{ "\"", "'s\""},
	{ "\"", "\""},
	{ "\"", "'s"},
	{ "\"", 0},
	{ 0, 0},
    };

    QStringList result;
    bool added_number = FALSE;
    if ( count() > 0 )  {
	result = choices(TRUE, FALSE, QString(), QString());
	if (count() == 1) {
	    QStringList all = findAll();
	    QStringList::Iterator it;
	    for (it = all.begin(); it != all.end(); ++it)
		if (!result.contains(*it))
		    result.append(*it);
	}

	WordWraps *w = wraps;
	// test if should add with gramar prefixes and post fixes.
	while(result.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
	    result = choices(TRUE, FALSE, w->prefix, w->postfix);
	    w += 1;
	}
	// test if should add with gramar prefixes and post fixes for number
	w = wraps;
	while(result.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
	    QString nword = numberWord(w->prefix, w->postfix);
	    if (!nword.isEmpty()) {
		added_number = TRUE;
		result += nword;
	    }
	    w += 1;
	}

	// test if anyting that may fit in prefix/postfix
	w = wraps;
	while(result.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
	    // only returns a word if sets have error values...
	    QString wword = writtenWord(w->prefix, w->postfix);
	    if (!wword.isEmpty())
		result += wword;
	    w += 1;
	}
	// always append the number word as a choice.
	if (!added_number) {
	    QString nword = numberWord(QString(), QString());
	    if (!nword.isEmpty()) {
		added_number = TRUE;
		result += nword;
	    }
	}
    }
    return result;
}

QStringList InputMatcher::choices(bool allowpartial, bool allowpredicted) const
{
    return choices(allowpartial, allowpredicted, QString(), QString());
}


int InputMatcher::prefixLength(const QString &p, int from) const
{
    if (from >= (int)nsets.size())
	return -1;
    if (p.isEmpty())
	return from;

    QChar c = p[0];

    const InputMatcherGuessList *gl = nsets[from];
    if (gl->contains(c)) {
	IMIGuess guess = gl->find(c);
	//ASSERT(guess.length != 0);
	return prefixLength(p.mid(1), from + guess.length);
    } else {
	return -1;
    }
}

int InputMatcher::suffixLength(const QString &p, int from) const
{
    if (from >= (int)nsets.size())
	return -1;
    if (p.isEmpty())
	return from;

    QChar c = p[p.length()-1];

    int i;
    for (i = (int)nsets.size()-1; i >= 0; --i) {
	const InputMatcherGuessList *g = nsets[i];
	InputMatcherGuessList::ConstIterator it = g->begin();
	while(it != g->end()) {
	    IMIGuess guess = *it;
	    if ((guess.length + i == (int)nsets.size() - from  )
		    && QChar(guess.c) == c && guess.length != 0)
		return suffixLength(p.left(p.length()-1),
			from + guess.length);
	    ++it;
	}
    }
    return -1;
}

QStringList InputMatcher::choices(bool allowpartial, bool allowpredicted, const QString &prefix, const QString &suffix) const
{
    if (nsets.size() == 0)
	return QStringList();

    if (mLookup) {
	QStringList ch;
	uint minError;
	int pl = prefixLength(prefix);
	int sl = suffixLength(suffix);
	if (pl == -1 || sl == -1 || (pl + sl >= (int)nsets.size()))
	    return QStringList();
	ch = searchDict(allowpartial, allowpredicted, dict, extradict,
		pl, nsets.size()-sl, &minError);
	if ( lowerdict ) {
	    for (QStringList::Iterator i=ch.begin(); i!=ch.end(); ++i)
		*i = (*i).toLower();
	}
	QStringList::Iterator it;
	for (it = ch.begin(); it != ch.end(); ++it) {
	    *it = prefix + *it + suffix;
	}
	return ch;
    } else {
	return findAll(nsets.size()-longestEnd, nsets.size(), QString() );
    }
}

QString InputMatcher::numberWord(const QString &prefix, const QString &suffix) const
{
    return findBest(prefix, suffix, TRUE);
}

QString InputMatcher::writtenWord(const QString &prefix, const QString &suffix) const
{
    return findBest(prefix, suffix, FALSE);
}

QString InputMatcher::findBest(const QString &prefix, const QString &suffix, bool isDigit) const
{
    int pl = prefixLength(prefix);
    int sl = suffixLength(suffix);
    if (pl == -1 || sl == -1 || (pl + sl >= (int)nsets.size()))
	return QString();
    QString nword = findBest(pl, nsets.size()-sl, isDigit);
    if (nword.isEmpty())
	return QString();
    else
	return prefix+nword+suffix;
}

QString InputMatcher::findBest(int set, int maxset, bool isDigit, const QString &str) const
{
    // only return if error values given.
    if (!qualifiedSets)
	return QString();
    if (set == maxset)
	return str;
    else if (set >maxset)
	return QString();

    // add word as spelt.
    const InputMatcherGuessList *gl = nsets[set];
    InputMatcherGuessList::const_iterator it = gl->begin();
    QList<int> avoidLength;
    avoidLength.append(0);
    while(it != gl->end()) {
	IMIGuess guess = *it;
	QChar ch(guess.c);
	if (gl->shift)
	    ch = convertToUpper(ch);
	if (!avoidLength.contains(guess.length) && ch.isDigit() == isDigit) {
	    QString r = findBest(set+guess.length, maxset, isDigit, str+ch);
	    if (!r.isEmpty())
		return r;
	    avoidLength.append(guess.length);
	}
	++it;
    }
    return QString();
}

static InputMatcherFunc funcNameToEnum(const QString &s, QString& arg, bool* showlist)
{
    if (s[0] == '\'' || s[0] == '"' ) {
	arg = s.mid(1);
	if ( showlist )
	    *showlist = s[0] == '"';
	return insertText;
    }
    if (s == "shift")
	return changeShift;
    if (s == "mode")
	return changeMode;
    if (s == "modify")
	return modifyText;
    if (s == "space")
	return insertSpace;
    if (s == "symbol")
	return insertSymbol;
    if (s == "changeim")
        return changeInputMethod;
    return noFunction;
}


QStringList InputMatcher::searchDict(bool allowprefix,
	bool predict, const QString& language,
	const QStringList& extradict, int start, int end, uint *me) const
{
    // Search dictionaries. Return list of words matching
    // this->sets, with the most likely word first. If no
    // words match, prefixes of the most likely words are
    // returned.

    InputMatcherWordErrorList r;
    QString lang = language;
    QString lang2;
    if ( lang.isNull() )
	lang = chosenLanguages()[0];
    if ( lang[2] == '_' ) {
	// Non country-specific words
	lang2 = lang.left(2);
    }

    bool prefixequal=true; // XXX could be parameterized if needed

    uint minError = UINT_MAX;
    for (int prefix=0; (r.isEmpty() || prefixequal) && prefix<=1; ++prefix) {
	InputMatcherWordErrorList preferred, added, extra;
	InputMatcherWordErrorList common, fixed, common2, fixed2;

	if(!qualifiedSets)
	    preferred = findWords(Qtopia::dawg("preferred").root(),start,end,
		    "", 0, prefix, predict);
	added = findWords(Qtopia::addedDawg().root(),start,end,
		    "",0, prefix, predict);
	if(!qualifiedSets)
	    common = findWords(Qtopia::dawg("_common",lang).root(),start,end,
		    "",0, prefix, predict);
	fixed = findWords(Qtopia::dawg("_words",lang).root(),start,end,
		    "",0, prefix, predict);

	if ( !lang2.isEmpty() ) {
	    common2 = findWords(Qtopia::dawg("_common",lang2).root(),
		    start,end,"",0, prefix, predict);
	    fixed2 = findWords(Qtopia::dawg("_words",lang2).root(),
		    start,end,"",0, prefix, predict);
	}

	uint nerror;
	for (QStringList::ConstIterator xdit=extradict.begin(); xdit!=extradict.end(); ++xdit) {
	    extra = findWords(Qtopia::dawg(*xdit).root(),start,end,"",0, prefix, predict);
	    nerror = r.merge(extra);
	    if (nerror < minError)
		minError = nerror;
	}

	nerror = r.merge(preferred);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(common);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(fixed);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(added);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(common2);
	if (nerror < minError)
	    minError = nerror;
	nerror = r.merge(fixed2);
	if (nerror < minError)
	    minError = nerror;

        if ( !allowprefix )
            break;
    }
    QStringList deleted = findWords(Qtopia::dawg("deleted").root(),start,end,"",0, false, predict).asStringList();
    for ( QStringList::ConstIterator i=deleted.begin(); i!=deleted.end(); ++i) {
        r.remove(*i);
    };

    qLog(Input) << "searchDict" <<
        (allowprefix ? " with prefixes" : "") <<
        (predict ? " with prediction" : "") <<
        " in" << language << ": " << r;

    if (me)
	*me = minError;
    // if error larger than X add extra word? not here.
    return r.asStringList();
}

QStringList InputMatcher::findAll(int set, int maxset, const QString& str) const
{
    if ( set == maxset ) {
	// fits in asked for set
	return QStringList(str);
    } else if (set > maxset ) {
	// does not fit in asked for set
	return QStringList();
    }

    QStringList r;
    const InputMatcherGuessList *g = nsets[set];
    InputMatcherGuessList::const_iterator it = g->begin();
    while(it != g->end()) {
	IMIGuess guess = *it;
	QChar c(guess.c);
	if (g->shift)
	    c = convertToUpper(c);
	r += findAll(set+guess.length, maxset, str+c);
	++it;
    }
    return r;

}

InputMatcherWordErrorList InputMatcher::findWords(const QDawg::Node* node,
	int set, int maxset,
	const QString& str, uint error, bool allowprefix, bool predict) const
{
    if ( !node || (set >= maxset && !predict) )
	return InputMatcherWordErrorList();

    InputMatcherWordErrorList r;
    const InputMatcherGuessList *g = 0;
    if ( set < maxset ) {
	g = nsets[set];
	// no letters to follow, don't try and make word, invalid path.
	if (g->count() == 0) {
	    return InputMatcherWordErrorList();
	}
    }
    while (node) {
	QChar ch = node->letter();
	IMIGuess guess;
	if (g && g->contains(ch)) {
	    guess = g->find(ch);
	} else {
	    guess.length = 0;
	    guess.c = 0xffff;
	    guess.error = UINT_MAX;
	}
	if ( (predict && (g == 0 || g->isEmpty())) || guess.length != 0) {
	    if (g && g->shift)
		ch = convertToUpper(ch);
	    if ( set >= maxset-(guess.length > 0 ? guess.length : 1) ) {
		InputMatcherWordError we;
		we.text = str+ch;
		we.error = error;
		if (guess.error != UINT_MAX)
		    we.error += guess.error;
		if ( node->isWord() || allowprefix )
		    r.append(we);
	    }
	    // is >> 10 so can up 2^10 = 1024 char words.
	    if (guess.length) {
		r += findWords(node->jump(),
			set+guess.length, maxset,
			str+ch, error+(guess.error >> 10),
			allowprefix, predict);
	    } else if (predict) {
		r += findWords(node->jump(),
			set+1, maxset,
			str+ch, error,
			allowprefix, predict);
	    }
	}
	node = node->next();
    }
    return r;
}

void InputMatcher::reset()
{
    for (int nsetsI = 0; nsetsI < nsets.size(); ++nsetsI)
	delete nsets[nsetsI];
    nsets.clear();
    longestEnd = 0;
    qualifiedSets = FALSE;
}

void InputMatcher::matchSet(const QString& s)
{
    reset();
    for (int i=0; i<(int)s.length(); i++)
	pushSet(QString(s[i]));
}

void InputMatcher::matchSet(const QStringList& s)
{
    reset();
    for (int i=0; i<(int)s.count(); i++)
	pushSet(QString(s[i]));
}

// reset then do input...
bool InputMatcher::match(const QString& s)
{
    reset();
    for (int i=0; i<(int)s.length(); i++) {
	if ( !push(s[i]) ) {
	    reset();
	    return FALSE;
	}
    }
    return TRUE;
}

bool InputMatcher::push(const QChar& ch, bool shift)
{
    if (extendedset.contains(ch)) {
	InputMatcherChar item = extendedset[ch];
	if (item.tapfunc == insertText) {
	    pushSet(item.taparg, shift);
	    return TRUE;
	}
    }
    return FALSE;
}

void InputMatcher::pushSet(const QString& s, bool shift)
{
    nsets.append(new InputMatcherGuessList(s, shift));
    longestEnd = 1;
}

void InputMatcher::pushGuessSet( const InputMatcherGuessList &nl)
{
    longestEnd = 0;

    InputMatcherGuessList *set = new InputMatcherGuessList();
    set->shift = nl.shift;
    nsets.append(set);

    InputMatcherGuessList::ConstIterator it = nl.begin();
    while(it != nl.end()) {
	int l = (*it).length;
	int index = nsets.size()-l;
	if (index < 0 || index >=(int)nsets.size()) {
	    // drop, wouldn't fit into word list
	    ++it;
	    continue;
	}
	if (l > longestEnd)
	    longestEnd = l;
	if ((*it).error != 0)
	    qualifiedSets = TRUE;

	InputMatcherGuessList *gl = nsets[nsets.size()-l];

	// need to insert in order of error
	InputMatcherGuessList::Iterator git;
	for (git = gl->begin(); git != gl->end(); ++git) {
	    if ((*git).error > (*it).error) {
		gl->insert(git, *it);
		break;
	    }
	}
	if (git == gl->end()) {
	    // end of list
	    gl->append(*it);
	}
	++it;
    }
}

InputMatcherGuessList InputMatcher::pop()
{
    InputMatcherGuessList result;
    if (nsets.size() > 0) {
	longestEnd = 0;
	for (int i = 0; i < nsets.size(); i++) {
	    InputMatcherGuessList *gl = nsets[i];
	    InputMatcherGuessList::Iterator it = gl->begin();
	    while(it != gl->end()) {
		if ((*it).length + i == nsets.size()) {
		    result.append(*it);
		    it = gl->erase(it);
		} else {
		    if (i+1 == nsets.size() && (*it).length > longestEnd)
			longestEnd = (*it).length;
		    ++it;
		}
	    }
	}
	delete nsets[nsets.size()-1];
	nsets.removeLast();
    }
    if (nsets.size() == 0)
	qualifiedSets = FALSE;
    return result;
}

int InputMatcher::count() const
{
    return nsets.size();
}

QString InputMatcher::at(int ind) const
{
    QString result;
    InputMatcherGuessList *gl = nsets[ind];
    InputMatcherGuessList::Iterator it = gl->begin();
    while(it != gl->end()) {
	result += (*it).c;
	++it;
    }
    return result;
}

QString InputMatcher::atReverse(int ind) const
{
    QString result;
    ind = nsets.size()-ind;
    for (int i = 0; i < (int)nsets.size(); i++) {
	InputMatcherGuessList *gl = nsets[i];
	InputMatcherGuessList::Iterator it = gl->begin();
	while(it != gl->end()) {
	    if ((*it).length + i == ind) {
		result += (*it).c;
	    }
	    ++it;
	}
    }
    return result;
}


QMap<QChar, InputMatcherChar> InputMatcher::map() const
{
    return extendedset;
}

void InputMatcher::readConfig(const QTranslatableSettings& cfg, QMap<QChar,InputMatcherChar>& set)
{
    QString buttons = cfg.value("Buttons").toString(); // No tr

    for (int i = 0; i < (int)buttons.length(); i++) {
	QChar ch = buttons[i];

	QString tapfunc("Tap"); // No tr
	QString holdfunc("Hold"); // No tr
	tapfunc += ch;
	holdfunc += ch;

	// lookup existing setting
	QMap<QChar,InputMatcherChar>::iterator existingSetting = set.find(ch);
	InputMatcherChar c;
	if (existingSetting != set.end())
	    c = *existingSetting;

	c.id = ch; // (if not found)

	QString fn;
	fn = cfg.value(tapfunc).toString();
	if ( !fn.isNull() )
	    c.tapfunc = funcNameToEnum(fn,c.taparg,&c.showList);
	fn = cfg.value(holdfunc).toString();
	if ( !fn.isNull() )
	    c.holdfunc = funcNameToEnum(fn,c.holdarg,0); // no showlist for Hold

	set.remove(c.id);
	set.insert(c.id, c);
    }
}

/*
   Read the config of button to behaviour map
   */
void InputMatcher::init()
{
    longestEnd = 0;
    qualifiedSets = 0;

    QSettings locale_cfg("Trolltech","locale");
    locale_cfg.beginGroup("Language");
    QString current_System_Language = locale_cfg.value("Language").toString();

    if (isNamedMode()) {
        qLog(Input) << "Initializing named mode: "<<mId;
	QTranslatableSettings cfg(Qtopia::qtopiaDir()+"etc/im/pkim/named_"+mId+".conf", QSettings::IniFormat);
	if ( cfg.status()==QSettings::NoError ) {
	    cfg.beginGroup("Buttons");
	    extradict = cfg.value("ExtraDict").toString().split(' ', QString::SkipEmptyParts);
	    QString d = cfg.value("Dict").toString();
	    if ( !d.isEmpty() )
		dict = d;
	    else {
                // no dictionary found for named mode - default to system setting
                if(chosenLanguages().contains(current_System_Language))
                {
                    dict = current_System_Language;
                } else {
                    // no dictionary specified, and no dictionary found for
                    // current system language, so default to first language
                    dict = chosenLanguages()[0];// InputLanguages marker
                }
            }
	    QString m = cfg.value("Mode").toString();
	    if ( m.isEmpty() ) {
		mReplaces = abc_index; // "abc"
	    } else switch (m[0].toLatin1()) {
		default:
		    mReplaces=abc_index; // "abc"
		    break;
		case 'D':
		    mReplaces=dict_index; // "dict"
		    break;
		case 'P':
		    mReplaces=phone_index; // Phone;
		    break;
	    }
	    QString cs = cfg.value("Case").toString();
	    if ( !cs.isEmpty() ) {
		if (cs[0] == 'l')
		    lowerdict = TRUE;
	    }

	    QString base = cfg.value("Base").toString();
#ifdef NO_DICT_MODE
	    if (!validNames[mReplaces].lookup || base != "TextButtons") { // no tr
#endif
		if (!base.isNull()) {
		    QTranslatableSettings globalcfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
		    globalcfg.beginGroup(base);
		    readConfig(globalcfg,extendedset);
		}
		// overrides?
		readConfig(cfg,extendedset);
#ifdef NO_DICT_MODE
	    }
#endif
	} else {
	    mId = QString(); // make invalid.
	}
    } else {
        // not named mode
	if (mId.left(5) == "dict-") {
	    dict = mId.mid(5);
	} else {
            if(chosenLanguages().contains(current_System_Language)) {
                dict = current_System_Language;
            } else {
                dict = chosenLanguages()[0];// InputLanguages marker
            };
        }

	mLookup = validNames[mIndex].lookup;

	QTranslatableSettings globalcfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
#ifdef NO_DICT_MODE
	if (!validNames[mIndex].lookup) {
#endif
	    if (validNames[mIndex].buttonDiff == 0 || validNames[mIndex].extendTextButtons) {
		globalcfg.beginGroup("TextButtons"); // No tr
		readConfig(globalcfg,extendedset);
                globalcfg.endGroup();
	    }
	    if (validNames[mIndex].buttonDiff) {
		globalcfg.beginGroup(validNames[mIndex].buttonDiff);
		readConfig(globalcfg,extendedset);
                globalcfg.endGroup();
	    }
#ifdef NO_DICT_MODE
	}
#endif
    }
}

/*
   Matches phone keypad input \a needle to string \a haystack, starting
   at \a offset, for n characters,
   returning how many matched consecutively.

   // this is more of a compare
*/
int InputMatcher::match(const QString& haystack, int offset, const QString& needle, int n)
{
    for (int i=0; i<n; i++)
    {
        if(i + offset >= haystack.length())
            return i;

	QChar ch = haystack[i+offset].toLower();
	QChar pk = needle[i];
	if( pk < '0' || pk > '9' ) {
	    if ( ch != pk )
		return i;
	} else {
	    if (extendedset.contains(pk)) {
		InputMatcherChar item = extendedset[pk];
		if (item.tapfunc != insertText || !item.taparg.contains(ch))
		    return i;
	    } else
		return i;
	}
    }

    return n; // match!
}

QStringList InputMatcher::langs;

QStringList InputMatcher::languages()
{
    if ( langs.isEmpty() ) {
        QString basename = Qtopia::qtopiaDir() + "/etc/dict/";
        QDir dir(basename);
        QStringList dftLangs = dir.entryList(QDir::Dirs);
        foreach (QString lang, dftLangs){
            if (QFile::exists(basename+lang+"/words.dawg")) {
                langs.append(lang);
            }
        }
    }

    return langs;
}

QStringList InputMatcher::chosenLanguages()
{
    // Rely on QSettings's caching, so we just re-read every time
    // rather than needing a change message.
    QSettings cfg("Trolltech","locale");
    cfg.beginGroup("Language");
    QStringList r = cfg.value("InputLanguages").toStringList();
    if ( r.isEmpty() )
	r.append(cfg.value("Language","en_US").toString());
    return r;
}

bool InputMatcher::guessCompatible() const
{
    if (mIndex != null_index)
	return validNames[mIndex].guessCompatible;
    else
	return validNames[mReplaces].guessCompatible;
}

/*******************
  * InputMatcherSet
  ********************/

InputMatcherSet::InputMatcherSet(QObject *parent)
    : QObject(parent), named(0), current(0), hintedMode(0)
{
    populate();
}

void InputMatcherSet::populate()
{
    int index = 0;
    while(validNames[index].name != 0) {
	mModes.append(new InputMatcher(validNames[index].name));
	index++;
    }

    QSettings cfg("Trolltech","locale");
    cfg.beginGroup("Language");
    QString current_System_Language = cfg.value("Language").toString();

    // load up dictionaries.
    QStringList cl = InputMatcher::chosenLanguages();
    QStringList::Iterator it = cl.begin();

    while(it != cl.end()) {
        // skip the current system language
        if(*it == current_System_Language){
            ++it;
            continue;
        }

	mModes.append(new InputMatcher("dict-"+(*it)));
        // check if this mode matches the system language, and if so set it as default
        if(*it == current_System_Language)
            current = mModes.last();
	++it;
    }

    if(!current){
        qLog(Input) << "Didn't find dictionary for system language. Defaulting to first found.";
        current = mModes.first();
    }
}

InputMatcherSet::~InputMatcherSet()
{
    if (named)
	delete named;
    named = 0; // pointless, but meaningful
    current = 0;

    while (mModes.count()) delete mModes.takeLast();
}

void InputMatcherSet::checkDictionaries()
{
    // load up dictionaries.
    QStringList cl = InputMatcher::chosenLanguages();
    QStringList::Iterator it;

    // Get the current system language so we can avoid creating a redundant mode
    QSettings cfg("Trolltech","locale");
    cfg.beginGroup("Language");
    QString current_System_Language = cfg.value("Language").toString();

    // first remove ones from mMode that are not chosen.
    QListIterator<InputMatcher*> mit(mModes);
    while ( mit.hasNext()) {
	if (mit.peekNext()->id().left(5) == "dict-") {
	    it = cl.begin();
	    bool keep = FALSE;

	    while(it != cl.end()) {
                // skip the current system language
                if(*it == current_System_Language){
                    ++it;
                    continue;
                }
		if (mit.peekNext()->id() == "dict-"+(*it)) {
		    // remove from list so not added later.
		    cl.erase(it);
		    keep = TRUE;
		    break;
		}
		++it;
	    }

	    if (keep) {
		mit.next();
	    } else {
		if (current == mit.peekNext())
		    current = 0;
		if (hintedMode == mit.peekNext())
		    hintedMode = 0;
		if (named == mit.peekNext())
		    named = 0;
		delete mit.peekNext();
		if (mModes.removeAll(mit.peekNext()) != 1)
                    qWarning() << "InputMatcherSet::checkDictionaries() - Warning! Duplicate dictionary entries detected";
                mit.next();
	    }
	} else {
	    mit.next();
	}
    }

    // then put the ones chosen back in. Only ones not already in
    // list are in cl, others removed above.
    // Also, don't add a mode that matches the current system language,
    // because that's already covered by plain "dict" mode
    it = cl.begin();
    while(it != cl.end()) {
        if(*it != current_System_Language) mModes.append(new InputMatcher("dict-"+(*it)));
	++it;
    }
    if (!current) {
	if (hintedMode)
	    current = hintedMode;
	else
	    current = mModes.first();
	emit modeChanged(current);
    }
}

void InputMatcherSet::clearHintedMode()
{
    hintedMode = 0;
}

InputMatcher *InputMatcherSet::setHintedMode(const QString &name)
{
    hintedMode = setCurrentMode(name);
    return hintedMode;
}

InputMatcher *InputMatcherSet::setCurrentMode(const QString &name)
{
    InputMatcher *target = mode(name);
    if (!target) {
	target = new InputMatcher(name);
	if (target->isValid()) {
	    if (target->isNamedMode()) {
		if (named)
		    delete named;
		named = target;
	    } else {
		mModes.append(target);
	    }
	} else {
	    delete target;
	    target = 0;
	}
    }

    if (target) {
	current = target;
	emit modeChanged(current);
	return target;
    } else {
	return 0;
    }
}

void InputMatcherSet::clearNamedMode()
{
    if (named) {
	if (hintedMode == named)
	    hintedMode = 0;
	if (current == named) {
	    current = mModes.first();
	    emit modeChanged(current);
	}
	delete named;
	named = 0;
    }
}

InputMatcher *InputMatcherSet::currentMode() const
{
    // ASSERT current != 0.  current should be set to non-0
    // in constructor, and only set to non-0 thereafter.
    return current;
}

/* if not first, sets to first.  otherwise, toggles

   There is a slight assumption in this code that if there
   is a named and a hintedMode, that they are equal.
   */
void InputMatcherSet::toggleHinted()
{
    // if there is a hinted mode.. then that
    // else if there is a first mode, then that,
    // if already 'first' then next.
    InputMatcher *target = 0;
    if (hintedMode)
	target = hintedMode;
    else
	target = mModes.first();

    if (current != target) {
	current = target;
	emit modeChanged(current);
    } else {
	nextMode();
    }
}

/* order is
   hinted
   as loaded in mModes (without hinted)
    but with named mode replacing one of them
*/
void InputMatcherSet::nextMode()
{
    checkDictionaries();
    if (hintedMode && current == hintedMode) {
	current = mModes.first();
	// skip any mode named replaces.
	if (current == hintedMode || (named && named->replaces() == current->id()))
	    // not sure about this
	    current = mModes[1];
    } else {
	// if current in mModes, then next in mModes,
	// if current !in mModes, first mModes
	QListIterator<InputMatcher*> it(mModes);
	InputMatcher *target = 0;
	for ( ; it.hasNext(); it.next() ) {
	    if (it.peekNext() == current) {
		it.next();
		if (it.hasNext()) {
		    target = it.peekNext();
		    // skip any mode named replaces.
		    if (named && named->replaces() == target->id()) {
			it.next();
			if (it.hasNext())
			    target = it.peekNext();
			else
			    target = hintedMode;
		    }
		} else {
		    target = hintedMode;//it.toFirst();
		}
		break;
	    }
	}
	if (!target) {
	    current = mModes.first();
	} else {
	    current = target;
	}
    }
    emit modeChanged(current);
}

QStringList InputMatcherSet::guessCompatibleModes()
{
    checkDictionaries();
    QStringList res;
    QListIterator<InputMatcher*> it(mModes);
    for ( ; it.hasNext(); it.next() ) {
	InputMatcher *match = it.peekNext();
	if (match->guessCompatible()) {
	    if (named && named->replaces() == match->id())
		res += named->id();
	    else
		res += match->id();
	}
    }
    return res;
}

QStringList InputMatcherSet::modes()
{
    checkDictionaries();
    QStringList res;
    QListIterator<InputMatcher*> it(mModes);
    for ( ; it.hasNext(); it.next() ) {
	InputMatcher *match = it.peekNext();
	if (named && named->replaces() == match->id())
	    res += named->id();
	else
	    res += match->id();
    }
    return res;
}


QPixmap InputMatcherSet::pixmapFor(const QString &name, bool ucase)
{
    if (mode(name))
	return mode(name)->pixmap(ucase);
    return QPixmap();
}

InputMatcher *InputMatcherSet::mode(const QString &name)
{
    checkDictionaries();
    // first see if name has an alias.
    QString tmpName = name;
    uint index=0;
    while(aliasLookup[index].name != 0) {
	if (tmpName == aliasLookup[index].name) {
	    tmpName = aliasLookup[index].lookup;
	    break;
	}
	index++;
    }

    // ?? yeah.. this is what I mean.
    if (named && (named->id() == tmpName || named->replaces() == tmpName))
	return named;

    QListIterator<InputMatcher*> it(mModes);
    for ( ; it.hasNext(); it.next() ) {
	InputMatcher *match = it.peekNext();
	if (match->id() == tmpName)
	    return match;
    }
    return 0;
}
