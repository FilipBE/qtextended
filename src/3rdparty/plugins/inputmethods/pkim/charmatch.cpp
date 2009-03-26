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

#include "charmatch.h"

#include <qdawg.h>
#include <qtopialog.h>

#include <QApplication>
#include <QTimer>

#include <limits.h>

/*!
   \class QFSPenMatch
   \internal
*/

/*!
   \class QFSPenMatch::Guess
   \internal
*/

QFSPenMatch::QFSPenMatch( QObject *parent )
    : QObject( parent ), mCharSet(0), mCanvasHeight(0)
{
    qLog(Input) << "QFSPenMatch: instantiated";
}

QFSPenMatch::~QFSPenMatch()
{
}

void QFSPenMatch::setCharSet( QIMPenCharSet *cs )
{
    mCharSet = cs;
}

bool contains(const QList<QFSPenMatch::Guess> &l, uint c, QChar text) {
    QListIterator<QFSPenMatch::Guess> it(l);
    while(it.hasNext()) {
	QFSPenMatch::Guess m = it.next();
	if (m.key == c && m.text == text)
	    return true;
    }
    return false;
}

void appendNew(QList<QFSPenMatch::Guess> &list, const QIMPenCharMatchList &matched, int length)
{
    QIMPenCharMatchList::ConstIterator it;
    for (it = matched.begin(); it != matched.end(); ++it) {
	if ((*it).error <= 200000 && (int)(*it).penChar->strokeCount() >= length) {
	    QFSPenMatch::Guess g;
	    g.length = length;
	    g.error = length > 1 ? (*it).error / 3 : (*it).error;
	    g.text = (*it).penChar->repCharacter();
	    g.key = (*it).penChar->key();
	    if (!contains(list, g.key, g.text))
		list.append(g);
	}
    }
}

void QFSPenMatch::addStroke( QIMPenStroke *st )
{
    qLog(Input) << "QFSPenMatch:: addStroke";
    if(!mCharSet){
        qWarning("CharSet not set in QFSPenMatch::addStroke( QIMPenStroke *st )");
        qWarning("QFSPenMatch::setCharSet() must be called before addStroke()");
        return;
    }


    mCanvasHeight = 0; // reset
    mMatchList.clear();
    mMatchCount = 0;

    /* want to test for current longest multi stroke
       and for each shorter..  then for low 
       thresholds, drop them from the list of max lenght
       matches.
       */

    // add 1 stroke test char to test chars that matched last time.
    QIMPenChar c;
    mTestChars.append(c);
    // and test current test chars for matches.
    uint minerror = UINT_MAX;
    int matched_height = 0;
    int input_height = 0;
    QMutableListIterator<QIMPenChar> it(mTestChars);
    while (it.hasNext()) {
	QIMPenChar c = it.next();
	c.addStroke(st);
	it.setValue(c);
	QIMPenCharMatchList ml = mCharSet->match( &c );
	// if ml is 0, drop this char...
	if (ml.count() > 0) {
	    // later just append, for now, bias towards more stroke and
	    // compare errors.
	    appendNew(mMatchList, ml, c.strokeCount());

	    // if lowest error, up date height of matched char and height
	    // of test char.
	    QIMPenChar *ch = ml.first().penChar;
	    if (ch && (uint)ml.first().error < minerror) {
		matched_height = ch->boundingRect().height();
		input_height = c.boundingRect().height();
	    }
	} else {
	    it.remove();
	}
    }

    if (input_height && matched_height)
	mCanvasHeight = int(float(input_height * 75) / float(matched_height));
}

void QFSPenMatch::clear()
{
    mTestChars.clear();
    mMatchList.clear();
}
