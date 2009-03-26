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

#ifndef _QIMPENMATCH_H_
#define _QIMPENMATCH_H_

#include <char.h>
#include <qlist.h>

class QTimer;
class QIMPenWidget;
class QIMPenSetup;

class QFSPenMatch : public QObject
{
    Q_OBJECT
public:
    struct Guess {
	Guess() : length(0), error(0), key(0) {}
	int length;
	uint error;
	uint key;
	QChar text;
    };

    QFSPenMatch( QObject *parent=0);
    virtual ~QFSPenMatch();

    void clear();
    void addStroke( QIMPenStroke * );
    QList<Guess> currentMatches() const { return mMatchList; }

    // the adjusted canvas height of the last stroke
    int lastCanvasHeight() const { return mCanvasHeight; }

    void setCharSet( QIMPenCharSet * );
    const QIMPenCharSet *charSet() const { return mCharSet; }

private:
    QList<QIMPenChar> mTestChars;

    QIMPenCharSet *mCharSet;
    QList<Guess> mMatchList;
    int mMatchCount;

    int mCanvasHeight;
};

#endif // _QIMPENINPUT_H_
