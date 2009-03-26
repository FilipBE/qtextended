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

#include "pinyinim.h"
#include <QDebug>
#include <QStringList>
#include <QMessageBox>
#include <QListIterator>
#include <QFile>
#include <QMultiHash>
#include <QVariant>
#include <QTranslatableSettings>
#include <qtopianamespace.h>
#include <qtopialog.h>

PinyinMatch::PinyinMatch(const QString &path, const QDawg::Node * node) :
    currentpath(path), currentnode(node)
{
}

PinyinMatch::~PinyinMatch()
{
}

PinyinIM::PinyinIM()
    : QWSInputMethod(), isActive(false), microX(20), microY(20), pinyinPicker(0), charPicker(0), symbolPicker(0)
{
    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    cfg.beginGroup("LocaleTextButtons"); // No tr

    key2let.insert(Qt::Key_2, cfg.value("Tap2","abc").toString());
    key2let.insert(Qt::Key_3, cfg.value("Tap3","def").toString());
    key2let.insert(Qt::Key_4, cfg.value("Tap4","ghi").toString());
    key2let.insert(Qt::Key_5, cfg.value("Tap5","jkl").toString());
    key2let.insert(Qt::Key_6, cfg.value("Tap6","mno").toString());
    key2let.insert(Qt::Key_7, cfg.value("Tap7","pqrs").toString());
    key2let.insert(Qt::Key_8, cfg.value("Tap8","tuv").toString());
    key2let.insert(Qt::Key_9, cfg.value("Tap9","wxyz").toString());
    
    loadPy2Uni();
}

PinyinIM::~PinyinIM()
{
    if (pinyinPicker)
        delete pinyinPicker;
    if (charPicker)
        delete charPicker;
    if (symbolPicker)
        delete symbolPicker;
}

bool PinyinIM::loadPy2Uni()
{
    QFile data( Qtopia::qtopiaDir() + "etc/im/pyim/py2uni" );
    if (!data.open(QFile::ReadOnly))
        return false;
    
    QString line;
    QStringList tokens;   //first member is unicode, rest are matching pinyin
    QString pinyin;
    QStringList pinyinList;
    QRegExp exp = QRegExp("[\\t,]");
    
    QTextStream in(&data);
    while (!in.atEnd())
    {
        line = in.readLine();
        tokens = line.split(exp, QString::SkipEmptyParts);
        pinyin = tokens[0];
        for (int i = tokens.size()-1; i > 0; --i) {
            py2uni.insert(pinyin, tokens[i][0]);
        }
        
        pinyinList << pinyin;
    }
    data.close();
    
    pinyinDawg.createFromWords(pinyinList);
    rootNode = pinyinDawg.root();
    return true;
}

bool PinyinIM::filter(int unicode, int keycode, int modifiers, 
        bool isPress, bool autoRepeat)
{
    qLog(Input) << "Keycode" << keycode;
    /*if (!isActive)
    return false;*/
    
    bool bUsingPicker = false;

    //NOTE: pinyin picker stays visible as we type
    if (pinyinPicker && pinyinPicker->isVisible()) {
        pinyinPicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat);

        if (!isPress && Qt::Key_Back == keycode) {
            if (matchHistory.count() <= 2) {    //i.e. if no characters are left after delete
                reset();
                return true;
            }
            matchHistory.pop();
        }

        bUsingPicker = true;
    }

    if (charPicker && charPicker->isVisible()) {
        charPicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat);

        if (!isPress && Qt::Key_Back == keycode) {
            //charPicker->clearSelection();
            charPicker->hide();
        }
        
        bUsingPicker = true;
    }
    
    if (symbolPicker && symbolPicker->isVisible()) {
        if (!symbolPicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat))
            symbolPicker->hide();
        return true;
    }

    
    //TODO: a redesign of filterKey would allow for cleaner code here
    switch(keycode) {
        case Qt::Key_Select:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            return bUsingPicker;
        case Qt::Key_Back:
            if (!bUsingPicker)
                return false;
            break;
        case Qt::Key_1:
        case Qt::Key_Asterisk:
            if (!symbolPicker) {
                symbolPicker = new SymbolPicker();
                connect(symbolPicker, SIGNAL(symbolClicked(int,int)),
                        this, SLOT(symbolSelected(int,int)));
            }
            symbolPicker->setMicroFocus(microX, microY);
            symbolPicker->show();
            return true;
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            if (!isPress)
                processDigitKey(keycode);
            break;
        case Qt::Key_0:
            if (!isPress)
                sendCommitString(" ");
            return true;
        default:
            return false;
    }

    if (!isPress && !matchHistory.isEmpty()) {
        QStringList choices;
        QList<PinyinMatch> matches = matchHistory.top();
        QListIterator<PinyinMatch> it(matches);
        while (it.hasNext())
            choices << it.next().path();
        
        if (choices.count()) {
            if (!pinyinPicker) {
                pinyinPicker = new WordPicker();
                connect(pinyinPicker, SIGNAL(wordChosen(QString)),
                    this, SLOT(pinyinSelected(QString)));
            }
            pinyinPicker->setChoices(choices);
            if (!pinyinPicker->isVisible())
            {
                qwsServer->sendIMQuery ( Qt::ImMicroFocus );
                pinyinPicker->show();
            }
        }
    }
    
    return true;
}

void PinyinIM::processDigitKey(int keycode)
{   
    QString letters = key2let.value(keycode,"");
    
    bool starting = false;
    if (matchHistory.empty())
    {
        QList<PinyinMatch> match;
        match.append(PinyinMatch("",rootNode));
        matchHistory.push(match);
        starting = true;
    }
    QList<PinyinMatch> prevmatches = matchHistory.top();
    
    QList<PinyinMatch> curmatches;
    QListIterator<PinyinMatch> it(prevmatches);
    while (it.hasNext()) {
        PinyinMatch match = it.next();
        
        const QDawg::Node * currentNode = match.node();
        if (!starting)
            currentNode = currentNode->jump();
        if (currentNode == 0)
            continue;
        
        while (1) {
            if (letters.contains(currentNode->letter())) {
                PinyinMatch addmatch(match.path() + currentNode->letter(), currentNode);
                curmatches.append(addmatch);
            }
            if (currentNode->isLast())
                break;
            currentNode = currentNode->next();
        }
    }
    
    if (!curmatches.empty())
        matchHistory.push(curmatches);
}

void PinyinIM::pinyinSelected(const QString &s)
{
    qLog(Input) << "Pinyin Selected" << s;
    QList<QChar> values = py2uni.values(s);
    QStringList choices;
    for (int j = 0; j < values.size(); ++j)
        choices << values.at(j);
    
    if (choices.count()) {
        if (!charPicker) {
            charPicker = new WordPicker();
            connect(charPicker, SIGNAL(wordChosen(QString)),
                    this, SLOT(charSelected(QString)));
        }
        charPicker->setChoices(choices);
        qwsServer->sendIMQuery ( Qt::ImMicroFocus );
        charPicker->show();
    }
    else
        pinyinPicker->show();   //user has selected invalid pinyin, so we don't want to hide
}

void PinyinIM::charSelected(const QString &s)
{
    qLog(Input) << "Character Selected" << s;
    sendCommitString(s);
    reset();
}

void PinyinIM::symbolSelected(int unicode, int /*keycode*/)
{
    qLog(Input) << "Symbol Selected" << unicode;
    sendCommitString(QString(QChar(unicode)));
    reset();
}

void PinyinIM::queryResponse ( int property, const QVariant & result )
{
    if(property==Qt::ImMicroFocus){
        QRect resultRect = result.toRect();
        if(resultRect.isValid()) setMicroFocus(resultRect.x(), resultRect.y());
    }
}

void PinyinIM::reset()
{
    if (pinyinPicker)
    {
        //pinyinPicker->clearSelection();
        pinyinPicker->hide();
    }
    if (charPicker)
    {
        //charPicker->clearSelection();
        charPicker->hide();
    }
    if (symbolPicker)
        symbolPicker->hide();

    matchHistory.clear();
}

void PinyinIM::setActive(bool b)
{
    isActive = b;
}

void PinyinIM::setMicroFocus( int x, int y )
{
    microX = x;
    microY = y;
    //if (isActive) {
    if (pinyinPicker && pinyinPicker->isVisible())
        pinyinPicker->setMicroFocus(x, y);
    if (charPicker && charPicker->isVisible())
        charPicker->setMicroFocus(x, y);
    if (symbolPicker && symbolPicker->isVisible())
        symbolPicker->setMicroFocus(x, y);
    //}
}
