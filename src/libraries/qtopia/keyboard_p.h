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

#ifndef KEYBOARD_P_H
#define KEYBOARD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QWidget>
#include <QTimeLine>
// These are copied from the predictive keyboard input method so we need
// to put it in a namespace to avoid symbol clashes
namespace HomeUiKeyboard {
class OptionsWidget;
class Board;
class AcceptWindow;
}
using namespace HomeUiKeyboard;
class EmbeddableKeyboardWidget;
class KeyMagnifier;
class WordPredict;

class KeyboardInputWidget : public QWidget
{
    Q_OBJECT
public:
    KeyboardInputWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KeyboardInputWidget();
    EmbeddableKeyboardWidget *keyboard;
    OptionsWidget *options;
};

class EmbeddableKeyboardWidget : public QWidget
{
Q_OBJECT
public:
    enum BoardType { NonAlphabet, Numeric, UpperCase, LowerCase };

    struct Config
    {
        int minimumStrokeMotionPerPeriod;
        int strokeMotionPeriod;
        int maximumClickStutter;
        int maximumClickTime;
        qreal minimumStrokeLength;
        qreal minimumStrokeDirectionRatio;

        QSize keyAreaSize;

        int selectCircleDiameter;
        int selectCircleOffset;

        int boardChangeTime;

        int leftSquishPoint;
        qreal leftSquishScale;
        int rightSquishPoint;
        qreal rightSquishScale;

        QSize keySize;
        int keyMargin;
        int bottomMargin;

        int maxGuesses;

        int optionWordSpacing;
        int optionsWidgetHeight;

        int reallyNoMoveSensitivity;
        int moveSensitivity;
        int excludeDistance;
    };

    EmbeddableKeyboardWidget(const Config &, OptionsWidget *ow, QWidget *parent = 0);
    virtual ~EmbeddableKeyboardWidget();

    void setSelectionHeight(int);

    void addBoard(const QStringList &, BoardType);

    void autoCapitalizeNextWord(bool);

    virtual QSize sizeHint() const;
    bool hasText();
    void reset();

    void setAcceptDest(const QPoint &);
    virtual bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);

signals:
    void backspace();
    void preedit(const QString &);
    void commit(const QString &);
    void pressedAndHeld();

protected:
    virtual void paintEvent(QPaintEvent *);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void timerEvent(QTimerEvent *);

public slots:
    void acceptWord();
    virtual void setHint(const QString& hint);
    void doBackspace();
    QRect rectForCharacter(const QChar &) const;
    QRect rectForWord(const QString &);
    QStringList words() const;

private:
    bool m_predictive;
    Config m_config;
    void clear();
    void resetToHistory();
    QStringList m_words;

    void dumpPossibleMotion();

    void startMouseTimer();
    void speedMouseTimer();
    void stopMouseTimer();
    int m_mouseTimer;
    bool m_speedMouseTimer;

    void mouseClick(const QPoint &);
    enum Stroke { NoStroke, StrokeLeft, StrokeRight, StrokeUp, StrokeDown };
    void stroke(Stroke);
    void pressAndHold();
    void pressAndHoldChar(const QChar &);

    QChar closestCharacter(const QPoint &, Board * = 0) const;

    QList<Board *> m_boards;
    int m_currentBoard;
    QSize m_boardSize;
    QRect m_boardRect;
    QPoint toBoardPoint(const QPoint &) const;

    QRectF m_modeRect;
    QRectF m_backspaceRect;
    QRectF m_spaceRect;
    QRectF m_shiftRect;
    QRectF m_periodRect;
    QRectF m_atRect;
    QRectF m_returnRect;
    void layoutBoard();

    enum Motion { Left = 0x01, Right = 0x02, Up = 0x04, Down = 0x08 };
    Motion m_possibleMotion;
    QPoint m_mouseMovePoint;
    QPoint m_lastSamplePoint;
    QPoint m_mousePressPoint;
    bool m_mouseClick;
    bool m_pressAndHold;
    QChar m_pressAndHoldChar;
    bool m_animate_accept;

    QPoint windowPosForChar() const;
    KeyMagnifier *m_charWindow;

    void setBoardByType(BoardType newBoard);
    QTimeLine m_boardChangeTimeline;
    int m_oldBoard;
    bool m_boardUp;

    bool m_specialDelete;

    bool m_ignore;

    OptionsWidget *m_options;

    bool m_notWord;
    bool m_alphabetSet;
    WordPredict *m_predict;
    void updateWords();
    QString closestWord();
    bool m_autoCap;
    bool m_autoCapitaliseEveryWord;
    bool m_preeditSpace;
    bool m_dontAddPreeditSpace;

    QStringList fixupCase(const QStringList &) const;

    struct KeyOccurance {
        enum Type { MousePress, CharSelect };
        Type type;
        QPoint widPoint;
        QChar explicitChar;
        int board;

        QString freezeWord;
    };
    QList<KeyOccurance> m_occuranceHistory;
};

#endif
