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

Words = {

/*
    Translates a word into a sequence of key presses eg. word = 9673
*/
    alphaToKeys: function(word)
    {
        var keyMap = "22233344455566677778889999";
        var alpha  = "abcdefghijklmnopqrstuvwxyz";
        var wordLC = word.toLowerCase();
        var wordKeys="";
        for (var i=0; i<wordLC.length; i++) {
            idx = alpha.indexOf(wordLC[i]);
            if (idx != -1)
                wordKeys += keyMap[idx];
        }
        return wordKeys;
    },

/*
    Press a sequence of keys
*/
    enterKeyClicks: function(keyClicks)
    {
        for (var i=0; i<keyClicks.length; i++) {
            wait(50);
            eval("keyClick(Qt.Key_" + keyClicks[i] + ")");
        }
    },

/*
    Check a predictive word by pressing the numbered keys
*/
    checkPredictiveWord: function(word)
    {
        // This assumes the inputmethod is set to PhoneKeys.

        // Open the 'Notes' application and create a new text document.
        startApplication( "Notes" );
        select( "New", optionsMenu() );

        keyClicks = Words.alphaToKeys(word);
        Words.enterKeyClicks(keyClicks);
        keyClick(Qt.Key_Select);
        var txt = getText().toLowerCase();
        select( "Cancel", optionsMenu() );
        return txt == word;
    },

    predictiveWordExists: function(word)
    {
        // checkPredictiveWord doesn't work reliably, so return
        checkWords = checkPredictiveWord(word);
        return (checkWords.contains(word.toLowerCase()));
    },

    preferredWord: function(word)
    {
    print("checking preferred word: " +word + "\n");
        checkWords = checkPredictiveWord(word);
    print("matching words: " +checkWords + "\n");
        return (checkWords[0] == word.toLowerCase());
    }
}
