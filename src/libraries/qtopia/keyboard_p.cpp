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

#include <QApplication>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLabel>
#include <QDesktopWidget>
#include <QListWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QDebug>
#include <QTimeLine>
#include <QPixmap>
#include <QPainter>
#include <QPaintEngine>
#include <QTextEdit>
#include <QWidget>
#include <QTimer>
#include <QCoreApplication>
#include <QInputContext>
#include <QExportedBackground>
#include <ThemedView>
#include <QSettings>
#include <qtopialog.h>
#include "keyboard_p.h"
#include "pred_p.h"

#define PREFIX_PREEDIT
#define QTOPIA_INTERNAL_QDAWG_TRIE
#include "qdawg.h"

namespace HomeUiKeyboard {

class Board
{
public:
    Board(const QStringList &,
          const QSize &,
          EmbeddableKeyboardWidget::BoardType alphabet);

    int lines() const;
    QString characters(int line) const;
    QString characters() const;

    QRect rect(const QChar &) const;

    QSize size() const;

    bool isAlphabet() const;
    bool isNumeric() const;
    EmbeddableKeyboardWidget::BoardType type() const;

private:
    QSize m_size;
    QStringList m_lines;
    QString m_characters;
    QHash<QChar, QRect> m_rects;
    EmbeddableKeyboardWidget::BoardType m_isAlphabet;
};

} // namespace

Board::Board(const QStringList &lines,
             const QSize &boardSize,
             EmbeddableKeyboardWidget::BoardType isAlphabet)
: m_size(boardSize), m_lines(lines), m_isAlphabet(isAlphabet)
{
    int maxline = -1;
    int maxline_len = 0;

    for(int ii = 0; ii < m_lines.count(); ++ii) {
        m_characters.append(m_lines.at(ii));
        if(maxline == -1 || m_lines.at(ii).length() > maxline_len) {
            maxline = ii;
            maxline_len = m_lines.at(ii).length();
        }
    }

    if(maxline == -1 || !maxline_len || lines.isEmpty())
        return;

    int width = m_size.width() / maxline_len;
    int height = m_size.height() / lines.count();

    for(int ii = 0; ii < m_lines.count(); ++ii) {
        const QString &line = m_lines.at(ii);

        int border = (m_size.width() - line.length() * width) / 2;

        for(int jj = 0; jj < line.length(); ++jj) {
            QRect r(border + jj * width, ii * height, width, height);
            m_rects.insert(line.at(jj), r);
        }
    }
}

int Board::lines() const
{
    return m_lines.count();
}

QString Board::characters(int line) const
{
    return m_lines.at(line);
}

QRect Board::rect(const QChar &c) const
{
    return m_rects[c];
}

QSize Board::size() const
{
    return m_size;
}

bool Board::isAlphabet() const
{
    return m_isAlphabet == EmbeddableKeyboardWidget::UpperCase ||
           m_isAlphabet == EmbeddableKeyboardWidget::LowerCase;
}

bool Board::isNumeric() const
{
    return m_isAlphabet == EmbeddableKeyboardWidget::Numeric;
}

EmbeddableKeyboardWidget::BoardType Board::type() const
{
    return m_isAlphabet;
}

QString Board::characters() const
{
    return m_characters;
}

namespace HomeUiKeyboard {

class AcceptWindow : public QWidget
{
Q_OBJECT
public:
    AcceptWindow(int time);

    void accept(const QString &word, const QRect &from, const QPoint &to);
    void setToPoint(const QPoint &);

protected:
    virtual void paintEvent(QPaintEvent *);

private slots:
    void valueChanged(qreal);

private:
    QTimeLine m_anim;

    QPoint m_from;
    QPoint m_to;
    QString m_word;
};

} // namespace

AcceptWindow::AcceptWindow(int time)
: QWidget(0, (Qt::WindowFlags)(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)), m_anim(time)
{
#ifdef Q_WS_QWS
    QPalette p = palette();
    p.setBrush(backgroundRole(), QColor(0,0,0,0));
    setPalette(p);
#endif
    setAttribute(Qt::WA_InputMethodTransparent, true);

    m_anim.setCurveShape(QTimeLine::EaseInCurve);
    QObject::connect(&m_anim, SIGNAL(valueChanged(qreal)),
                     this, SLOT(valueChanged(qreal)));

    setWindowTitle("_allow_on_top_"); // Use window manager back door.
    setWindowModality (Qt::NonModal);
    setFocusPolicy(Qt::NoFocus);
}

void AcceptWindow::valueChanged(qreal v)
{
    if(v == 1.0f) {
        hide();
        deleteLater();
    } else {
        QPoint p = m_from + (m_to - m_from) * v;
        move(p);
        update();
    }
}

void AcceptWindow::accept(const QString &word, const QRect &from, const QPoint &to)
{
    setFixedSize(from.size());
    m_from = from.topLeft();
    m_to = to - QPoint(from.width() / 2, from.height() / 2);
    move(m_from);
    m_word = word;

    show();
    m_anim.start();
}

void AcceptWindow::setToPoint(const QPoint &p)
{
    if(m_anim.state() != QTimeLine::NotRunning) {

        QPoint currentPoint = m_from + (m_to - m_from) * m_anim.currentValue();

        m_to = p - QPoint(width() / 2, height() / 2);

        QPoint newStart;
        if(m_anim.currentValue() == 1.0f) {
            newStart = m_to;
        } else {
            newStart = (currentPoint - m_anim.currentValue() * m_to) / (1.0f - m_anim.currentValue());
        }

        m_from = newStart;
    }

}

void AcceptWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QColor color = palette().text().color();
    color.setAlpha((int)(255 * (1.0f - m_anim.currentValue())));
    p.setPen(color);
    p.drawText(rect(), m_word, Qt::AlignHCenter | Qt::AlignVCenter);
}

namespace HomeUiKeyboard {

class OptionsWidget : public QWidget
{
Q_OBJECT
public:
    OptionsWidget(int wordSpacing);

    void setWords(const QStringList &);

    enum ClearType { ClearImmediate, ClearSoon, ClearEventually };
    void clear(ClearType = ClearImmediate);

    QString acceptWord(const QPoint &, bool animate = true);
    void setAcceptDest(const QPoint &);
    QString selectedWord() const;

    QRect wordRect(int ii) const;

signals:
    void wordAccepted();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void moveEvent(QMoveEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void timerEvent(QTimerEvent *);

private slots:
    void valueChanged(qreal);
    void finished();

private:
    void layoutWords(const QStringList &);

    bool m_specialDelete;
    bool m_ignore;

    QList<QPair<QString, QRect> > m_words;
    int m_selectedWord;

    void slideTo(int word);
    int optionsOffset() const;
    QTimeLine m_slideTimeline;
    int m_slideStart;
    int m_slideEnd;

    QTimeLine m_clearTimeline;
    int m_wordSpacing;

    void startClearTimer(ClearType);
    void stopClearTimer();
    int m_clearTimer;
    ClearType m_clearType;

    QPointer<AcceptWindow> m_lastAccept;
};

} // namespace

OptionsWidget::OptionsWidget(int wordSpacing)
: QWidget(0, 0), m_specialDelete(false), m_ignore(false),
  m_selectedWord(0), m_slideTimeline(300),
  m_wordSpacing(wordSpacing), m_clearTimer(0)
{
    //setAttribute(Qt::WA_InputMethodTransparent, true);

    m_clearTimeline.setCurveShape(QTimeLine::LinearCurve);
    QObject::connect(&m_clearTimeline, SIGNAL(valueChanged(qreal)),
                     this, SLOT(valueChanged(qreal)));
    QObject::connect(&m_clearTimeline, SIGNAL(finished()),
                     this, SLOT(finished()));
    QObject::connect(&m_slideTimeline, SIGNAL(valueChanged(qreal)),
                     this, SLOT(update()));

    setFocusPolicy(Qt::NoFocus);

    setFixedSize(320, 20);
}

void OptionsWidget::setWords(const QStringList &words)
{
    stopClearTimer();

    if(isHidden())
        show();

    if(m_clearTimeline.state() != QTimeLine::NotRunning) {
        m_ignore = true;
        m_clearTimeline.stop();
        m_clearTimeline.setCurrentTime(0);
        m_ignore = false;
    }

    if(m_slideTimeline.state() != QTimeLine::NotRunning) {
        m_ignore = true;
        m_slideTimeline.stop();
        m_slideTimeline.setCurrentTime(0);
        m_ignore = false;
    }

    m_words.clear();
    layoutWords(words);

    update();
}

void OptionsWidget::layoutWords(const QStringList &words)
{
    m_words.clear();
    m_selectedWord = 0;

    if(words.isEmpty())
        return;

    QFont font;
    font.setPointSize((font.pointSize() * 15) / 10);
    QFontMetrics fm(font);

    int leftEdge = 0;
    int rightEdge = 0;
    for(int ii = 0; ii < words.count(); ++ii) {

        QRect r;
        QString w = words.at(ii);
        int ww = fm.width(w);

        if(ii == 0) {
            r = QRect(-ww/2, 0, ww, 1);
        } else if(ii % 2) {
            r = QRect(leftEdge - ww, 0, ww, 1);
        } else {
            r = QRect(rightEdge, 0, ww, 1);
        }

        if(r.left() < leftEdge)
            leftEdge = r.left() - m_wordSpacing;

        if(r.right() > rightEdge)
            rightEdge = r.right() + m_wordSpacing;

        m_words << qMakePair(w, r);
    }
}

void OptionsWidget::finished()
{
    if(m_clearTimeline.state() == QTimeLine::NotRunning) {
        //hide();
    }
}

void OptionsWidget::moveEvent(QMoveEvent *)
{
}

void OptionsWidget::startClearTimer(ClearType type)
{
    stopClearTimer();
    if(type == ClearSoon) {
        m_clearTimer = startTimer(1000);
    } else if(type == ClearEventually) {
        m_clearTimer = startTimer(2500);
    }
}

void OptionsWidget::stopClearTimer()
{
    if(m_clearTimer) {
        killTimer(m_clearTimer);
        m_clearTimer = 0;
    }
}

void OptionsWidget::timerEvent(QTimerEvent *)
{
    stopClearTimer();
    m_ignore = true;
    m_ignore = false;
    m_clearTimeline.start();
    update();
}

void OptionsWidget::clear(ClearType type)
{
    bool special = false;
    for(int ii = 0; !special && ii < m_words.count(); ++ii) {
        if(m_words.at(ii).first.toLower() == "pacman")
            special = true;
    }

    if(special) {
        m_clearTimeline.setDuration(2000);
        m_specialDelete = true;
    } else {
        m_clearTimeline.setDuration(300);
        m_specialDelete = false;
    }

    m_clearTimeline.start();
    update();

    stopClearTimer();
    if(type == ClearImmediate) {
        m_ignore = true;
        m_ignore = false;
    } else {
        startClearTimer(type);
    }
}

void OptionsWidget::setAcceptDest(const QPoint &p)
{
    if(m_lastAccept)
        m_lastAccept->setToPoint(p);
}

QString OptionsWidget::selectedWord() const
{
    if(m_words.isEmpty())
        return QString();
    else
        return m_words.at(m_selectedWord).first;
}

QString OptionsWidget::acceptWord(const QPoint &p, bool animate)
{
    QString word = m_words.at(m_selectedWord).first;
    QRect startRect = wordRect(m_selectedWord);

    if(animate) {
        AcceptWindow *win = new AcceptWindow(500 /* XXX */);
        win->accept(word,
                QRect(mapToGlobal(startRect.topLeft()), startRect.size()), p);
        m_lastAccept = win;
    }
    m_words.clear();
    update();
    return word;
}

void OptionsWidget::valueChanged(qreal v)
{
    if(m_ignore)
        return;

    if(1.0f == v)
        m_words.clear();

    update();
}

void OptionsWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_words.isEmpty()) {
        // Hide now so that the user can get to the menubar.
        clear(ClearImmediate);
    } else {
        for(int ii = 0; ii < m_words.count(); ++ii) {
            QRect r = wordRect(ii);
            if(r.contains(e->pos())) {
                if(ii == m_selectedWord) {
                    emit wordAccepted();
                } else {
                    slideTo(ii);
                    update();
                }

                return;
            }
        }
    }
}

QRect OptionsWidget::wordRect(int ii) const
{
    return QRect(m_words.at(ii).second.x() + optionsOffset(), 0,
                 m_words.at(ii).second.width(), height());
}

void OptionsWidget::slideTo(int word)
{
    int offset = optionsOffset();
    m_ignore = true;
    m_slideTimeline.stop();
    m_slideTimeline.setCurrentTime(0);
    m_ignore = false;
    m_selectedWord = word;
    m_slideStart = offset;
    m_slideEnd = optionsOffset();
    m_slideTimeline.start();
}

int OptionsWidget::optionsOffset() const
{
    if(m_words.isEmpty())
        return 0;

    if(m_slideTimeline.state() == QTimeLine::NotRunning) {
        return width() / 2 - m_words.at(m_selectedWord).second.center().x();
    } else {
        return (int)(m_slideStart + (m_slideEnd - m_slideStart) * m_slideTimeline.currentValue());
    }
}

void OptionsWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    //p.drawRect(rect());

    QPixmap pix(size());
    pix.fill(Qt::transparent);
    QRect totalWordRect;
    {
        QPainter pixp(&pix);
        QFont font;
        QFont bigfont = font;
        bigfont.setPointSize((bigfont.pointSize() * 15) / 10);
        if(!m_words.isEmpty()) {

            for(int ii = 0; ii < m_words.count(); ++ii) {
                const QString &word = m_words.at(ii).first;
                QRect rect = wordRect(ii);
                totalWordRect = totalWordRect.united(rect);

                if(ii == m_selectedWord) {
                    pixp.setFont(bigfont);
                }
                pixp.setPen(palette().windowText().color());
                pixp.drawText(rect, word, Qt::AlignVCenter | Qt::AlignHCenter);
                if(ii == m_selectedWord) {
                    pixp.setFont(font);
                }
            }
        }
    }

    if(m_clearTimeline.state() != QTimeLine::NotRunning) {

        QPainter pixp(&pix);

        int fadeWidth = 60;
        int fadeStart;
        if(m_specialDelete) {
            fadeStart = (int)((1.0f - m_clearTimeline.currentValue()) * (width() + fadeWidth) - fadeWidth);
        } else {
            fadeStart = (int)((1.0f - m_clearTimeline.currentValue()) * (totalWordRect.width() + fadeWidth) - fadeWidth) + totalWordRect.x();
        }
        int fadeEnd = fadeStart + fadeWidth;

        QLinearGradient grad(QPoint(fadeStart, 0), QPoint(fadeEnd, 0));
        grad.setColorAt(0, QColor(0,0,0,255));
        grad.setColorAt(1.0f, QColor(0,0,0,0));
        if (pixp.paintEngine()->hasFeature(QPaintEngine::PorterDuff))
            pixp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pixp.fillRect(pix.rect(), grad);
//        pixp.setCompositionMode(QPainter::CompositionMode_Plus);
 //       pixp.fillRect(pix.rect(), QColor(255, 0, 0, 0));


        if(m_specialDelete) {
            QPainterPath pacman;
            int pac_width = (height() * 12) / 10;
            int pac_height = height();

            qreal pacmouth = m_clearTimeline.currentValue() / 0.2f;
            pacmouth = pacmouth - (int)(pacmouth);
            pacmouth *= 2.0f;
            pacmouth -= 1.0f;
            pacmouth = qAbs(pacmouth);

            pacman.moveTo(pac_width / 2, pac_height / 2);

            pacman.arcTo(0, 0, pac_width, pac_height, 180 - pacmouth * 50.0f,
                               -(360 - pacmouth * 100.0f));
            pacman.closeSubpath();
            p.setPen(Qt::black);
            p.setBrush(Qt::yellow);
            int pacStart = (int)((1.0f - m_clearTimeline.currentValue()) * (width() + pac_width) - pac_width);

            p.save();
            p.setClipRect(0, 0, (pac_width / 2) + pacStart, height());
            p.drawPixmap(0,0,pix);
            p.restore();

            p.translate(pacStart, 0);
            p.setRenderHint(QPainter::Antialiasing);
            p.drawPath(pacman);
            p.setBrush(Qt::black);
            p.drawEllipse(pac_width / 2, pac_height / 5, 2, 2);
            p.setRenderHint(QPainter::Antialiasing, false);
            p.translate(-pacStart, 0);
        } else {
            p.drawPixmap(0,0,pix);
        }
    } else {
        p.drawPixmap(0,0,pix);
    }
}

/////////////////////////////////////////////////////////////////////////
static EmbeddableKeyboardWidget::Config createKeyboardConfig()
{
    EmbeddableKeyboardWidget::Config config;

    int swidth = QApplication::desktop()->width();
    int sheight = QApplication::desktop()->width();

    config.minimumStrokeMotionPerPeriod = 50;
    config.strokeMotionPeriod = 200;
    config.maximumClickStutter = swidth / 12;
    config.maximumClickTime = 400;
    config.minimumStrokeLength = 0.3f;
    config.minimumStrokeDirectionRatio = 2.0f;
    config.selectCircleDiameter = swidth / 4;
    config.selectCircleOffset = -swidth / 4;
    config.boardChangeTime = 400;
    config.keySize.setWidth(QApplication::desktop()->width());
    config.keySize.setHeight(config.keySize.height() / 4);  //280 for tests
    config.keyMargin = swidth / 10;
    config.bottomMargin = sheight / 24;
    config.maxGuesses = 5;
    config.optionsWidgetHeight = -1;
    config.optionWordSpacing = swidth / 24;
    config.reallyNoMoveSensitivity = swidth / 48;
    config.moveSensitivity = config.maximumClickStutter;
    config.excludeDistance = (swidth * 10) / 48;

    QSettings cfg("Trolltech", "PredictiveKeyboard");
    cfg.beginGroup("Settings");

    config.minimumStrokeMotionPerPeriod =
        cfg.value("MinimumStrokeMotionPerPeriod", 50).toInt();
    config.strokeMotionPeriod =
        cfg.value("StrokeMotionPeriod", 200).toInt();
    config.maximumClickStutter =
        cfg.value("MaximumClickStutter", config.maximumClickStutter).toInt();
    config.maximumClickTime =
        cfg.value("MaximumClickTime", 400).toInt();
    config.minimumStrokeLength =
        cfg.value("MinimumStrokeLength", 0.3f).toInt();
    config.minimumStrokeDirectionRatio =
        cfg.value("MinimumStrokeDirectionRatio", 2.0f).toDouble();
    config.selectCircleDiameter =
        cfg.value("SelectCircleDiameter", config.selectCircleDiameter).toInt();
    config.selectCircleOffset =
        cfg.value("SelectCircleOffset", config.selectCircleOffset).toInt();
    config.boardChangeTime =
        cfg.value("BoardChangeTime", 400).toInt();
    config.keySize.setWidth(cfg.value("KeySizeWidth", config.keySize.width()).toInt());
    if(!cfg.contains("KeySizeHeight")) {
        config.keySize.setHeight(config.keySize.width() / 3);
    } else {
        config.keySize.setHeight(cfg.value("KeySizeHeight", config.keySize.height()).toInt());
    }       //comment out for tests
    config.keyMargin =
        cfg.value("KeyMargin", config.keyMargin).toInt();
    config.bottomMargin =
        cfg.value("BottomMargin", config.bottomMargin).toInt();
    config.maxGuesses =
        cfg.value("MaxGuesses", 5).toInt();
    config.optionWordSpacing =
        cfg.value("OptionWordSpacing", config.optionWordSpacing).toInt();
    config.optionsWidgetHeight =
        cfg.value("OptionsWindowHeight", -1).toInt();
    config.reallyNoMoveSensitivity =
        cfg.value("ReallyNoMoveSensitivity", config.reallyNoMoveSensitivity).toInt();
    config.moveSensitivity =
        cfg.value("MoveSensitivity", config.moveSensitivity).toInt();
    config.excludeDistance =
        cfg.value("ExcludeDistance", config.excludeDistance).toInt();

    return config;
}

KeyboardInputWidget::KeyboardInputWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f),
    keyboard(0), options(0)
{
    //create keyboard and options widgets
    EmbeddableKeyboardWidget::Config config = createKeyboardConfig();

    options = new OptionsWidget(config.optionWordSpacing);

    keyboard = new EmbeddableKeyboardWidget(config, options);
    keyboard->addBoard(QStringList() << "QWERTYUIOP" << "ASDFGHJKL" << "ZXCVBNM", EmbeddableKeyboardWidget::UpperCase);
    keyboard->addBoard(QStringList() << "qwertyuiop" << "asdfghjkl" << "zxcvbnm", EmbeddableKeyboardWidget::LowerCase);
    keyboard->addBoard(QStringList() << "\{}[]`^%=" << (QString("|<>") + QChar(0x00A3) + QChar(0x20AC) + QChar(0x00A5) + QChar('~')) << "", EmbeddableKeyboardWidget::NonAlphabet);
    keyboard->addBoard(QStringList() << "!+#$:;&*()" << "1234567890" << "_-\"',/?", EmbeddableKeyboardWidget::Numeric);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(options);
    l->addWidget(keyboard);
}

KeyboardInputWidget::~KeyboardInputWidget()
{
}
/////////////////////////////////////////////////////////////////////////


class KeyMagnifier : public QWidget
{
Q_OBJECT
public:
    KeyMagnifier(int raise, QWidget * = 0);

    void setChar(const QChar &, const QPoint &, Board *board);
    bool charOk() const;
    QChar getChar() const;
    void dismiss();
    void setImmediate(bool b) { m_immediate = b; }

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

private slots:
    void valueChanged(qreal);

private:
    void moveTo(const QPoint &newPos);
    bool m_ignore;
    bool m_immediate;
    QPoint m_startPoint;
    QPoint m_endPoint;
    QTimeLine m_timeline;
    QChar m_char;
    Board *m_board;

    int m_offset;
    bool m_dismissing;
    qreal m_dismissValue;
    QPoint m_point;
};

KeyMagnifier::KeyMagnifier(int raise, QWidget *parent)
: QWidget(parent, (Qt::WindowFlags)(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)),
  m_ignore(false), m_immediate(false), m_timeline(300), m_board(0),
  m_offset(raise), m_dismissing(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_QWS
    QPalette p = palette();
    p.setBrush(backgroundRole(), QColor(0,0,0,0));
    setPalette(p);
#endif
    QObject::connect(&m_timeline, SIGNAL(valueChanged(qreal)), this, SLOT(valueChanged(qreal)));

    setWindowTitle("_allow_on_top_"); // Use window manager back door.
    setWindowModality (Qt::NonModal);
    setFocusPolicy(Qt::NoFocus);
}

void KeyMagnifier::moveTo(const QPoint &newPos)
{
    m_ignore = true;
    m_timeline.stop();
    m_startPoint = pos();
    m_endPoint = newPos;
    m_ignore = false;
    m_timeline.start();
}

void KeyMagnifier::valueChanged(qreal v)
{
    if(m_ignore)
        return;
    QPoint newPos = m_startPoint + (m_endPoint - m_startPoint) * v;
    update();
    move(newPos);
    if(1.0f == v &&  m_dismissing)
        hide();
}

void KeyMagnifier::showEvent(QShowEvent *)
{
    if (m_immediate)
        move(m_endPoint);
    else
        moveTo(m_endPoint);
}

void KeyMagnifier::hideEvent(QHideEvent *)
{
    m_ignore = true;
    m_timeline.stop();
    m_timeline.setCurrentTime(0);
    m_ignore = false;
    m_dismissing = false;
}

void KeyMagnifier::dismiss()
{
    if (m_immediate) {
        hide();
        return;
    }

    m_ignore = true;
    if(m_timeline.state() == QTimeLine::Running)
        m_dismissValue = m_timeline.currentValue();
    else
        m_dismissValue = 1.0f;
    m_timeline.stop();
    m_timeline.setCurrentTime(0);
    m_ignore = false;

    m_dismissing = true;
    moveTo(m_point);
}

bool KeyMagnifier::charOk() const
{
    //if (m_immediate)
    //    return true;    //###
    return (m_timeline.state() == QTimeLine::NotRunning ||
            m_timeline.currentValue() > 0.66f);
}

QChar KeyMagnifier::getChar() const
{
    return m_char;
}

void KeyMagnifier::setChar(const QChar &c, const QPoint &p, Board *board)
{
    m_char = c;
    m_board = board;

    QPoint pos = p - QPoint(width() / 2, height() / 2);
    if(isHidden()) {
        move(pos);
        m_endPoint = pos + QPoint(0, m_offset);
        show();
    } else {
        moveTo(pos + QPoint(0, m_offset));
        update();
    }
    m_point = pos;

    raise();
}

void KeyMagnifier::paintEvent(QPaintEvent *)
{
    if(!m_board)
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient grad(QPoint(0, 0), QPoint(0, height()));
    QLinearGradient grad2(QPoint(2, 0), QPoint(0, height() - 4));

    grad.setColorAt(0.0f, QColor(127, 127, 127));
    grad.setColorAt(0.8f, QColor(200, 200, 200));
    grad.setColorAt(1.0f, QColor(230, 230, 230));
    grad2.setColorAt(0.0f, QColor(200, 200, 200));
    grad2.setColorAt(0.2f, QColor(180, 180, 180));
    grad2.setColorAt(1.0f, QColor(50, 50, 50));

    int ewidth;
    if (m_immediate)
        ewidth = width() - 2;
    else if(!m_dismissing)
        ewidth = (int)(m_timeline.currentValue() * (width() - 2));
    else /* if (!m_showing) */
        ewidth = (int)(m_dismissValue * (1.0f - m_timeline.currentValue()) * (width() - 2));

    if(ewidth == 0)
        return;

    p.setBrush(grad);
    p.setPen(QPen(grad2, 0));
    p.drawEllipse((width() - ewidth) / 2, (width() - ewidth) / 2, ewidth, ewidth);

    QPainterPath path;
    path.addEllipse((width() - ewidth) / 2, (width() - ewidth) / 2, ewidth, ewidth);
    p.setClipPath(path);

    p.setRenderHint(QPainter::Antialiasing, false);

    p.setPen(Qt::black);

    QString characters = m_board->characters();
    QPoint boardCenter = m_board->rect(m_char).center();
    QPoint widgetPosTrans = pos() - (m_point + QPoint(0, m_offset));
    QPoint widgetCenter = rect().center() - widgetPosTrans;

    QPoint transform = widgetCenter - boardCenter;

    QFont mainFont = QApplication::font();
    QFont bigFont = mainFont;
    bigFont.setPointSize((bigFont.pointSize() * 16) / 10);

    for(int ii = 0; ii < characters.count(); ++ii) {
        const QChar &c = characters.at(ii);
        QRect crect = m_board->rect(c).translated(transform.x(), transform.y());
        if(rect().intersects(crect)) {
            if(c == m_char) {
                QRect r = rect();
                r.moveCenter(crect.center());
                p.setFont(bigFont);
                p.drawText(r, c, Qt::AlignHCenter | Qt::AlignVCenter);
                p.setFont(mainFont);
            } else if (!m_immediate) {
                //TODO: do a blur for all other text?
                p.drawText(crect, c, Qt::AlignHCenter | Qt::AlignVCenter);
            }
        }
    }
}

QString EmbeddableKeyboardWidget::closestWord()
{
    QString rv;
    for(int ii = 0; ii < m_occuranceHistory.count(); ++ii)  {
        const KeyOccurance &o = m_occuranceHistory.at(ii);
        if(!o.freezeWord.isEmpty())
            rv = o.freezeWord;
        if(o.type == KeyOccurance::CharSelect)
            rv.append(o.explicitChar);
        else if(o.type == KeyOccurance::MousePress)
            rv.append(closestCharacter(o.widPoint, m_boards.at(o.board)));
    }
    return rv;
}

void EmbeddableKeyboardWidget::dumpPossibleMotion()
{

    QString res;
    if(m_possibleMotion & Right) res += "Right ";
    if(m_possibleMotion & Left) res += "Left ";
    if(m_possibleMotion & Up) res += "Up ";
    if(m_possibleMotion & Down) res += "Down ";
    qWarning() << res;
}

void EmbeddableKeyboardWidget::layoutBoard()
{
    if (!m_predictive) {
        m_config.keySize.rheight() += 20;   //### magic number: height of optionswidget
    }
    m_config.keySize.rheight() += 10;   //### magic number: space for spacebar, etc
    setFixedSize(m_config.keySize.width(), m_config.keySize.height());

    int x = m_config.keyMargin;
    int y = 0;
    int w = m_config.keySize.width() - 2 * m_config.keyMargin;
    int h = m_config.keySize.height() - 2 * m_config.bottomMargin - 10;
    m_boardRect = QRect(x, y, w, h);
    m_boardSize = m_boardRect.size();
    
    const int spaceWidth = static_cast<int>(w/2.5);
    m_spaceRect = QRectF(x+(w - spaceWidth)/2, y+h+5, spaceWidth, m_config.bottomMargin*2);
    int sideWidth = static_cast<int>(m_spaceRect.left());
    m_periodRect = QRectF(m_spaceRect.right()+5,y+h+5,(sideWidth-20)/4,m_config.bottomMargin*2);
    m_atRect = QRectF(m_periodRect.right()+5,y+h+5,(sideWidth-20)/4,m_config.bottomMargin*2);
    m_returnRect = QRectF(m_atRect.right()+5,y+h+5,(sideWidth-20)/4 * 2,m_config.bottomMargin*2);
    m_modeRect = QRectF(5, y+h+5, sideWidth-10, m_config.bottomMargin*2);
    m_shiftRect = QRectF(5,0,x-5,h);
    m_backspaceRect = QRectF(x+w,0,x-5,h);

    m_options->setVisible(m_predictive);
}

EmbeddableKeyboardWidget::EmbeddableKeyboardWidget(const Config &config, OptionsWidget *ow,
                               QWidget *parent)
: QWidget(parent, (Qt::WindowFlags)(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)),
  m_predictive(false), m_config(config), m_mouseTimer(0),
  m_currentBoard(0),
  m_pressAndHold(false),m_animate_accept(true), m_charWindow(0),
  m_boardChangeTimeline(config.boardChangeTime),
  m_ignore(false),
  m_notWord(false), m_alphabetSet(false),
  m_predict(0), m_autoCap(false), m_autoCapitaliseEveryWord(false),
  m_preeditSpace(false), m_dontAddPreeditSpace(false)
{
    setAttribute(Qt::WA_InputMethodTransparent, true);

    WordPredict::Config wconfig;
    wconfig.reallyNoMoveSensitivity = m_config.reallyNoMoveSensitivity;
    wconfig.moveSensitivity = m_config.moveSensitivity;
    wconfig.excludeDistance = m_config.excludeDistance;

    m_predict = new WordPredict(wconfig, m_config.maxGuesses);
    
    QObject::connect(&m_boardChangeTimeline, SIGNAL(valueChanged(qreal)), this, SLOT(update()));

    m_options = ow;
    QObject::connect(m_options, SIGNAL(wordAccepted()),
                     this, SLOT(acceptWord()));
    
    layoutBoard();

    setFocusPolicy(Qt::NoFocus);
}

EmbeddableKeyboardWidget::~EmbeddableKeyboardWidget()
{
    delete m_predict;
}

void EmbeddableKeyboardWidget::autoCapitalizeNextWord(bool autocap)
{
    m_autoCap = autocap;
}

void EmbeddableKeyboardWidget::setAcceptDest(const QPoint &p)
{
    m_options->setAcceptDest(p);
}

void EmbeddableKeyboardWidget::reset()
{
    if(!m_words.isEmpty()) {
        m_animate_accept = false;
        m_dontAddPreeditSpace = true;
        acceptWord();
        m_dontAddPreeditSpace = false;
        m_animate_accept = true;
    };

    clear();
    m_options->clear(OptionsWidget::ClearSoon);
}

/*void EmbeddableKeyboardWidget::focusOut()
{
    return;
    if(!m_words.isEmpty()) {
        m_animate_accept = false;
        acceptWord();
        m_animate_accept = true;
    } else {
        emit commit(QString());
    }
}*/

QSize EmbeddableKeyboardWidget::sizeHint() const
{
    return QSize(m_config.keySize.width(), m_config.keySize.height());
}

void EmbeddableKeyboardWidget::addBoard(const QStringList &chars, BoardType type)
{
    Board *board = new Board(chars, m_boardSize, type);
    m_boards << board;

    if(NonAlphabet != type && !m_alphabetSet) {
        m_alphabetSet = true;

        QString chars = board->characters();
        for(int ii = 0; ii < chars.length(); ++ii) {
            m_predict->setLetter(chars.at(ii).toLower().toLatin1(), board->rect(chars.at(ii)).center());
        }

    }

    if(LowerCase == type)
        m_currentBoard = m_boards.count() - 1;
}

void EmbeddableKeyboardWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setBrush(palette().highlight());
    p.setPen(palette().highlightedText().color());
    p.setRenderHint(QPainter::Antialiasing);

    //p.drawRect(rect());
    //p.drawRect(m_boardRect);

    p.drawRoundedRect(m_spaceRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    p.drawText(m_spaceRect, tr("space"), Qt::AlignHCenter | Qt::AlignVCenter);
    
    p.drawRoundedRect(m_periodRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    p.drawText(m_periodRect, ".", Qt::AlignHCenter | Qt::AlignVCenter);
    
    p.drawRoundedRect(m_atRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    p.drawText(m_atRect, "@", Qt::AlignHCenter | Qt::AlignVCenter);

    p.setBrush(QColor(Qt::black));
    p.drawRoundedRect(m_returnRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    p.drawText(m_returnRect, tr("return"), Qt::AlignHCenter | Qt::AlignVCenter);

    p.drawRoundedRect(m_modeRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    if (m_boards.at(m_currentBoard)->isAlphabet())
        p.drawText(m_modeRect, "123", Qt::AlignHCenter | Qt::AlignVCenter);
    else
        p.drawText(m_modeRect, "abc", Qt::AlignHCenter | Qt::AlignVCenter);

    p.drawRoundedRect(m_shiftRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::LowerCase)
        p.drawText(m_shiftRect, "Aa", Qt::AlignHCenter | Qt::AlignVCenter);
    else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::UpperCase)
        p.drawText(m_shiftRect, "Aa", Qt::AlignHCenter | Qt::AlignVCenter);
    else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::NonAlphabet)
        p.drawText(m_shiftRect, "123", Qt::AlignHCenter | Qt::AlignVCenter);
    else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::Numeric)
        p.drawText(m_shiftRect, QString("_#") + QChar(0x00A3), Qt::AlignHCenter | Qt::AlignVCenter);

    // XXX Dependency issue!
#if 0 // def QTOPIA_HOMEUI
    p.setBrush(QtopiaHome::standardColor(QtopiaHome::Red));
#else
    p.setBrush(QColor(Qt::red));
#endif
    p.drawRoundedRect(m_backspaceRect.adjusted(.5,.5,-.5,-.5), 2, 2);
    p.drawText(m_backspaceRect, "<-", Qt::AlignHCenter | Qt::AlignVCenter);
    
    p.setBrush(palette().highlight());

    p.setClipRect(m_boardRect);

    if(!m_boards.isEmpty()) {
        if(m_boardChangeTimeline.state() == QTimeLine::Running) {
            if(m_boardUp) {
                p.translate(0, m_boardRect.height() * (1.0f - m_boardChangeTimeline.currentValue()));
            } else {
                p.translate(0, -m_boardRect.height() + m_boardChangeTimeline.currentValue() * m_boardRect.height());
            }
        }
        Board *board = m_boards.at(m_currentBoard);

        QString str = board->characters();
        for(int ii = 0; ii < str.length(); ++ii) {
            QChar c = str.at(ii);
            QRectF r = board->rect(c);
            r.translate(m_boardRect.x(), m_boardRect.y());

            p.drawRoundedRect(r.adjusted(2.5,2.5,-2.5,-2.5), 2, 2);

            p.drawText(r, c, Qt::AlignHCenter | Qt::AlignVCenter);
        }

        if(m_boardChangeTimeline.state() == QTimeLine::Running) {
            if(m_boardUp) {
                p.translate(0, -m_boardRect.height());
            } else {
                p.translate(0, m_boardRect.height());
            }
            Board *board = m_boards.at(m_oldBoard);
            QString str = board->characters();
            for(int ii = 0; ii < str.length(); ++ii) {
                QChar c = str.at(ii);
                QRect r = board->rect(c);

                p.drawText(r.translated(m_boardRect.x(), m_boardRect.y()), c, Qt::AlignHCenter | Qt::AlignVCenter);
            }
        }
    }
}

void EmbeddableKeyboardWidget::mousePressEvent(QMouseEvent *e)
{
    m_mousePressPoint = e->pos();
    m_lastSamplePoint = m_mousePressPoint;
    m_mouseMovePoint = m_mousePressPoint;
    m_possibleMotion = (Motion)(Left | Right | Up | Down);

    m_pressAndHold = false;
    m_mouseClick = false;

    if (m_backspaceRect.contains(m_mousePressPoint))
        doBackspace();  //TODO: timer for press-and-hold deletion
    else if (m_spaceRect.contains(m_mousePressPoint))
        acceptWord();
    else if (m_shiftRect.contains(m_mousePressPoint)) {
        if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::UpperCase) {
            m_oldBoard = m_currentBoard;
            m_currentBoard = (m_currentBoard + 1) % m_boards.count();
            update();
        } else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::LowerCase) {
            m_oldBoard = m_currentBoard;
            m_currentBoard--;
            if(m_currentBoard < 0)
                m_currentBoard = m_boards.count() - 1;
            update();
        } else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::NonAlphabet) {
            m_oldBoard = m_currentBoard;
            m_currentBoard++;
            update();
        } else if (m_boards.at(m_currentBoard)->type() == EmbeddableKeyboardWidget::Numeric) {
            m_oldBoard = m_currentBoard;
            m_currentBoard--;
            update();
        }
    } else if (m_modeRect.contains(m_mousePressPoint)) {
        m_oldBoard = m_currentBoard;
        if (m_boards.at(m_currentBoard)->isAlphabet()) {
            m_currentBoard = m_boards.count() - 1;
        } else
            m_currentBoard = 1;
        update();
    } else if (m_periodRect.contains(m_mousePressPoint)) {
        pressAndHoldChar('.');
        /*if (m_predictive) {
            m_dontAddPreeditSpace = true;
            acceptWord();
            m_dontAddPreeditSpace = false;
        }*/
    } else if (m_atRect.contains(m_mousePressPoint)) {
        pressAndHoldChar('@');
        /*if (m_predictive) {
            m_dontAddPreeditSpace = true;
            acceptWord();
            m_dontAddPreeditSpace = false;
        }*/
    } else if (m_returnRect.contains(m_mousePressPoint)) {
        m_dontAddPreeditSpace = true;
        acceptWord();
        m_dontAddPreeditSpace = false;
        emit commit("\n");
    } else if (m_boardRect.contains(m_mousePressPoint)) {
        if (!m_predictive)
            pressAndHold();
        else {
            m_mouseClick = true;
            startMouseTimer();
        }
    }
}

/*
   Interesting motions are:
        Click
            + Limited time
            + Limited motion

        Stroke up
        Stroke down
        Stroke left
        Stroke right
            + Minimum sampled velocity
            + Minimum single direction motion
            + Minimum motion ratio

        Hold and move
            + Everything else
*/
void EmbeddableKeyboardWidget::mouseReleaseEvent(QMouseEvent *e)
{
    bool m_charWindowOk = false;
    if(m_charWindow) {
        m_charWindowOk = m_charWindow->charOk();
        m_charWindow->dismiss();
    }

    //dumpPossibleMotion();
    //update();

    stopMouseTimer();

    if(m_boards.isEmpty())
        return;

    QPoint rp = e->pos();

    if(m_mouseClick) {
        QPoint p((rp.x() + m_mousePressPoint.x()) / 2,
                 (rp.y() + m_mousePressPoint.y()) / 2);
        mouseClick(p);
        return;
    }

    if(m_pressAndHold) {
        if(m_boardRect.contains(e->pos()) && (m_charWindowOk || !m_predictive))
            pressAndHoldChar(m_pressAndHoldChar);
        m_pressAndHold = false;
        return;
    }


    Stroke updown = NoStroke;
    int y_delta = e->pos().y() - m_mousePressPoint.y();
    int x_delta = e->pos().x() - m_mousePressPoint.x();

    Stroke leftright = NoStroke;

    if(m_possibleMotion & Down && y_delta > (m_boardSize.height() / 3))
        updown = StrokeDown;
    else if(m_possibleMotion & Up && -y_delta > (m_boardSize.height() / 3))
        updown = StrokeUp;

    if(m_possibleMotion & Right && x_delta > (m_boardSize.width() / 3))
        leftright = StrokeRight;
    else if(m_possibleMotion & Left && -x_delta > (m_boardSize.width() / 3))
        leftright = StrokeLeft;

    if(updown != NoStroke && leftright != NoStroke) {
        if((qAbs(y_delta) / qAbs(x_delta)) > m_config.minimumStrokeDirectionRatio) {
            leftright = NoStroke;
        } else if((qAbs(x_delta) / qAbs(y_delta)) > m_config.minimumStrokeDirectionRatio) {
            updown = NoStroke;
        } else {
            leftright = NoStroke;
            updown = NoStroke;
        }
    }

    if(updown != NoStroke) {
        stroke(updown);
    } else if(leftright != NoStroke) {
        stroke(leftright);
    }

}

void EmbeddableKeyboardWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_mouseMovePoint = e->pos();

    if(qMax(qAbs(e->pos().x() - m_mousePressPoint.x()), qAbs(e->pos().y() - m_mousePressPoint.y())) > m_config.maximumClickStutter)
        m_mouseClick = false;

    if(m_pressAndHold || (m_charWindow && m_charWindow->isVisible())) {

        QChar c = closestCharacter(m_mouseMovePoint);
        if(c != m_pressAndHoldChar) {
            QPoint cp = m_boards.at(m_currentBoard)->rect(c).translated(m_boardRect.x(), m_boardRect.y()).center();
            QPoint cp_cur = m_boards.at(m_currentBoard)->rect(m_pressAndHoldChar).translated(m_boardRect.x(), m_boardRect.y()).center();

            int delta_x = cp.x() - m_mouseMovePoint.x();
            int delta_y = cp.y() - m_mouseMovePoint.y();
            int delta_x_cur = cp_cur.x() - m_mouseMovePoint.x();
            int delta_y_cur = cp_cur.y() - m_mouseMovePoint.y();

            int distance = delta_x * delta_x + delta_y * delta_y;
            int distance_cur = delta_x_cur * delta_x_cur + delta_y_cur * delta_y_cur;
            bool closer = distance < ((distance_cur * 4) / 9);

            if(closer && m_charWindow)  {
                m_pressAndHoldChar = c;
                m_charWindow->setChar(c, windowPosForChar(), m_boards.at(m_currentBoard));
                update();
            }
        }
    }
}

QPoint EmbeddableKeyboardWidget::windowPosForChar() const
{
    QPoint c = m_boards.at(m_currentBoard)->rect(m_pressAndHoldChar).translated(m_boardRect.x(), m_boardRect.y()).center();

    return mapToGlobal(c);
}

void EmbeddableKeyboardWidget::timerEvent(QTimerEvent *)
{
    m_mouseClick = false;

    if(m_possibleMotion & Right) {
        if((m_mouseMovePoint.x() - m_lastSamplePoint.x()) < m_config.minimumStrokeMotionPerPeriod)
            m_possibleMotion = (Motion)(m_possibleMotion & ~Right);
    }

    if(m_possibleMotion & Left) {
        if((m_lastSamplePoint.x() - m_mouseMovePoint.x()) < m_config.minimumStrokeMotionPerPeriod)
            m_possibleMotion = (Motion)(m_possibleMotion & ~Left);
    }

    if(m_possibleMotion & Down) {
        if(m_mouseMovePoint.y() - m_lastSamplePoint.y() < m_config.minimumStrokeMotionPerPeriod)
            m_possibleMotion = (Motion)(m_possibleMotion & ~Down);
    }

    if(m_possibleMotion & Up) {
        if((m_lastSamplePoint.y() - m_mouseMovePoint.y()) < m_config.minimumStrokeMotionPerPeriod)
            m_possibleMotion = (Motion)(m_possibleMotion & ~Up);
    }

    m_lastSamplePoint = m_mouseMovePoint;

    if(m_possibleMotion == 0) {
        stopMouseTimer();
        pressAndHold();
    } else {
        speedMouseTimer();
    }

    //dumpPossibleMotion();
}

void EmbeddableKeyboardWidget::startMouseTimer()
{
    if(m_mouseTimer)
        return;

    if(m_notWord || m_boards.at(m_currentBoard)->isAlphabet())
        m_mouseTimer = startTimer(m_config.maximumClickTime);
    else
        m_mouseTimer = startTimer(m_config.maximumClickTime / 2);
    m_speedMouseTimer = false;
}

void EmbeddableKeyboardWidget::stopMouseTimer()
{
    if(m_mouseTimer) {
        killTimer(m_mouseTimer);
        m_mouseTimer = 0;
        m_speedMouseTimer = false;
    }
}

void EmbeddableKeyboardWidget::speedMouseTimer()
{
    if(m_mouseTimer && m_speedMouseTimer)
        return;

    if(m_mouseTimer)
        killTimer(m_mouseTimer);

    m_mouseTimer = startTimer(m_config.strokeMotionPeriod);
    m_speedMouseTimer = true;
}


QChar EmbeddableKeyboardWidget::closestCharacter(const QPoint &p, Board *board) const
{
    if(!board)
        board = m_boards.at(m_currentBoard);

    QString str = board->characters();
    int distance = -1;
    QChar c;

    QPoint bp = toBoardPoint(p);
    for(int ii = 0; ii < str.length(); ++ii) {
        QRect r = board->rect(str.at(ii));

        int x_delta = r.center().x() - bp.x();
        int y_delta = r.center().y() - bp.y();

        int t_distance = x_delta * x_delta + y_delta * y_delta;
        if(distance == -1 || t_distance < distance) {
            distance = t_distance;
            c = str.at(ii);
        }
    }

    return c;
}

/*
    Returns the rect in widget co-ordinates where \a c is visible, or QRect().
*/
QRect EmbeddableKeyboardWidget::rectForCharacter(const QChar &c) const
{
    QRect ret;

    Board *board = 0;
    if (-1 != m_currentBoard && (board = m_boards.at(m_currentBoard))) {
        ret = board->rect(c).translated(m_boardRect.x(), m_boardRect.y());
    }

    return ret;
}

/*
    Returns the rect in widget co-ordinates where \a word is visible, or QRect().
*/
QRect EmbeddableKeyboardWidget::rectForWord(const QString &word)
{
    QRect ret;

    for (int ii = 0; ii < m_words.count(); ++ii) {
        if (word == m_words.at(ii)) {
            QRect wr = m_options->wordRect(ii);
            ret = wr;
            ret.moveTopLeft(mapFromGlobal(m_options->mapToGlobal(wr.topLeft())));
            break;
        }
    }

    return ret;
}

/*
    Returns the words currently shown to the user.
*/
QStringList EmbeddableKeyboardWidget::words() const
{
    return m_words;
}

QPoint EmbeddableKeyboardWidget::toBoardPoint(const QPoint &p) const
{
    QPoint rv = p;
    rv.setX(rv.x() - m_boardRect.x());
    rv.setY(rv.y() - m_boardRect.y());
    return rv;
}

void EmbeddableKeyboardWidget::mouseClick(const QPoint &p)
{
    if(m_preeditSpace) {
        m_preeditSpace = false;

        if(m_boards.at(m_currentBoard)->isAlphabet() ||
           m_boards.at(m_currentBoard)->isNumeric()) {
            emit commit(QString(" "));
        } else {
            emit commit(QString());
        }
    }

    if(m_boards.at(m_currentBoard)->type() == Numeric || m_boards.at(m_currentBoard)->type() == NonAlphabet) {
        pressAndHoldChar(closestCharacter(p, m_boards.at(m_currentBoard)));
        return;
    }

    if(!m_boards.at(m_currentBoard)->isAlphabet())
        m_notWord = true;

    if(m_notWord)
        return;

    QPoint point = toBoardPoint(p);

    KeyOccurance occurance;
    occurance.type = KeyOccurance::MousePress;
    occurance.widPoint = p;
    occurance.board = m_currentBoard;
    m_occuranceHistory << occurance;

    m_predict->addTouch(point);

    updateWords();
}

void EmbeddableKeyboardWidget::pressAndHoldChar(const QChar &c)
{
    bool wasNotWord = m_notWord;

    if(!m_boards.at(m_currentBoard)->isAlphabet() && c != '\'')
        m_notWord = true;

    if(m_preeditSpace) {
        m_preeditSpace = false;

        if(m_boards.at(m_currentBoard)->isAlphabet() ||
           m_boards.at(m_currentBoard)->isNumeric()) {
            emit commit(QString(" "));
        } else {
            emit commit(QString());
        }
    }

    if(c == QChar(0x21b5)) {// Magic number - return key
        //m_preeditSpace = false;
        m_dontAddPreeditSpace = true;
        acceptWord();
        m_dontAddPreeditSpace = false;
        emit commit("\n");
    }
    else {
        if (!m_predictive) {
            emit commit(c);
            return;
        }
        KeyOccurance occurance;
        occurance.type = KeyOccurance::CharSelect;
        // Special-case carriage returns
        occurance.board = m_currentBoard;
        occurance.explicitChar = c;
        if(!wasNotWord && m_notWord)
            occurance.freezeWord = m_options->selectedWord();

        m_occuranceHistory << occurance;

        m_predict->addLetter(c.toLower().toLatin1());

        updateWords();
    }
}

/*!
    Returns true if the keyboard widget currently has input awaiting a commit, or if it has a preedit space pending, otherwise returns false.
    This is an effective check for a preedit string.
*/
bool EmbeddableKeyboardWidget::hasText()
{
    return !m_words.isEmpty() || m_preeditSpace;
}

void EmbeddableKeyboardWidget::updateWords()
{
    m_words = QStringList();

    if(!m_notWord)
        m_words = m_predict->words();

    if(m_words.isEmpty())
        m_words << closestWord();

    m_words = fixupCase(m_words);
    m_options->setWords(m_words);

    // Change to lowercase if appropriate for autocapitalisation -
    // i.e. if this is the first letter of the word, autocapitalisation is on,
    // and the current board is still uppercase.
    // Changing back to uppercase is handled in EmbeddableKeyboardWidget::acceptWord()
    if(m_autoCap && m_occuranceHistory.count() == 1 && m_boards.at(m_currentBoard)->type() == UpperCase) {
        setBoardByType(LowerCase);
    }

    update();

    if(m_words.isEmpty() && m_preeditSpace) {
        emit commit(" ");
    } else if(m_words.isEmpty()) {
        emit commit(QString());
    } else {
#ifdef PREFIX_PREEDIT
        if(!m_notWord && !m_predict->prefixedWord().isEmpty())
            emit preedit(m_predict->prefixedWord());
        else
#endif
        emit preedit(m_words.first());
    }
}

/*
  We change case if the user has any explicit characters or if the user does not
  stay on a lowercase board.
*/
QStringList EmbeddableKeyboardWidget::fixupCase(const QStringList &list) const
{
    bool needFixupCase = false;

    for(int ii = 0; !needFixupCase && ii < m_occuranceHistory.count(); ++ii) {
        const KeyOccurance &o = m_occuranceHistory.at(ii);
        if(o.type == KeyOccurance::CharSelect ||
           m_boards.at(o.board)->type() != LowerCase)
            needFixupCase = true;
    }

    if(!needFixupCase) {
        return list;
    } else {
        QStringList rv;

        // XXX - stupidly inefficient
        for(int ii = 0; ii < list.count(); ++ii) {
            QString str = list.at(ii);
            Q_ASSERT(str.length() == m_occuranceHistory.count());

            for(int jj = 0; jj < str.length(); ++jj) {
                if(m_boards.at(m_occuranceHistory.at(jj).board)->type()
                        == UpperCase)
                    str[jj] = str[jj].toUpper();
                else if(m_boards.at(m_occuranceHistory.at(jj).board)->type()
                        == LowerCase)
                    str[jj] = str[jj].toLower();
            }

            rv << str;
        }
        return rv;
    }
}

void EmbeddableKeyboardWidget::setBoardByType(BoardType newBoardType)
{
    int newBoard = -1;
    // find new board that matches type;

    for(int i = 0; i < m_boards.count(); i++)
    {
        if (m_boards.at(i)->type() == newBoardType)
        {
            newBoard = i;
            break;
        }
    }

    if(newBoard == -1) {
        qLog(Input) << "EmbeddableKeyboardWidget::setBoardByType - Failed to find board of type "<<newBoardType;
        return;
    };

    if( newBoard == m_currentBoard ) {
        qLog(Input) << "EmbeddableKeyboardWidget::setBoardByType - already at board of type "<<newBoardType;
        return;
    }

    if(isVisible()) {
        // animate
        m_oldBoard = m_currentBoard;
        m_currentBoard = newBoard;
        if(m_currentBoard < 0)
            m_currentBoard = m_boards.count() - 1;
        if(!m_boards.at(m_oldBoard)->isAlphabet() ||
                !m_boards.at(m_currentBoard)->isAlphabet()) {
            m_boardChangeTimeline.start();

            m_boardUp = ( m_currentBoard > m_oldBoard );
            if( abs(m_currentBoard - m_oldBoard) > m_boards.count() / 2)
                m_boardUp = !m_boardUp;
        } else {
            update();
        }
    } else {
        //not visible, so just change the board
        m_oldBoard = m_currentBoard;
        m_currentBoard = newBoard;
        update();
    }
}


void EmbeddableKeyboardWidget::stroke(Stroke s)
{
    switch(s) {
        case StrokeUp:
            m_oldBoard = m_currentBoard;
            m_currentBoard--;
            if(m_currentBoard < 0)
                m_currentBoard = m_boards.count() - 1;
            if(!m_boards.at(m_oldBoard)->isAlphabet() ||
               !m_boards.at(m_currentBoard)->isAlphabet()) {
                m_boardChangeTimeline.start();
                m_boardUp = true;
            } else {
               update();
            }
            break;

        case StrokeDown:
            m_oldBoard = m_currentBoard;
            m_currentBoard = (m_currentBoard + 1) % m_boards.count();
            if(!m_boards.at(m_oldBoard)->isAlphabet() ||
               !m_boards.at(m_currentBoard)->isAlphabet()) {
                m_boardChangeTimeline.start();
                m_boardChangeTimeline.start();
                m_boardUp = false;
            } else {
               update();
            }
            break;

        case StrokeLeft:
            doBackspace();
            break;

        case StrokeRight:
            acceptWord();
            break;

        case NoStroke:
            break;
    }
}

void EmbeddableKeyboardWidget::pressAndHold()
{
    m_pressAndHold = true;

    if(!m_charWindow) {
        m_charWindow = new KeyMagnifier(m_config.selectCircleOffset, 0);
        m_charWindow->setFixedSize(m_config.selectCircleDiameter,
                                m_config.selectCircleDiameter);
        m_charWindow->setImmediate(!m_predictive);
    }
    m_pressAndHoldChar = closestCharacter(m_mouseMovePoint);
    
    if (m_predictive)
        m_charWindow->setChar(m_pressAndHoldChar, windowPosForChar(), m_boards.at(m_currentBoard));

    //update();

    if (m_predictive)
        emit pressedAndHeld();
}

/*
   If the last character was an explicit character, we will remove just it.
   Otherwise we remove the entire word.
*/
void EmbeddableKeyboardWidget::doBackspace()
{
    bool wasEmpty = m_words.isEmpty();

    if(m_preeditSpace) {

        emit commit(QString());
        m_preeditSpace = false;
        return;

    } else if(m_occuranceHistory.isEmpty() || m_occuranceHistory.count() == 1 ||
       m_occuranceHistory.last().type == KeyOccurance::MousePress) {

        clear();

    } else {

        m_occuranceHistory.removeLast();
        resetToHistory();

    }

    if(m_words.isEmpty()) {
        if(wasEmpty) {
            m_options->clear(OptionsWidget::ClearImmediate);
            emit backspace();
        } else {
            m_options->clear(OptionsWidget::ClearSoon);
        }

        emit commit(QString());
    }
}

void EmbeddableKeyboardWidget::acceptWord()
{
    if(m_words.isEmpty()) {
        emit commit(" ");
        m_preeditSpace = false;
        return;
    }

    QString word = m_options->acceptWord(QPoint(0, 0), m_animate_accept /* XXX microfocushint */);
    m_options->clear(OptionsWidget::ClearEventually);
    clear();
    m_autoCap = m_autoCapitaliseEveryWord;

    if(m_autoCap && m_boards.at(m_currentBoard)->type() == LowerCase)
        setBoardByType(UpperCase);

    emit commit(word);
    if(!m_dontAddPreeditSpace) {
        m_preeditSpace = true;
        emit preedit(" ");
    }
}

void EmbeddableKeyboardWidget::clear()
{
    m_predict->reset();
    m_words.clear();
    m_notWord = false;
    m_occuranceHistory.clear();
}

void EmbeddableKeyboardWidget::resetToHistory()
{
    m_predict->reset();

    bool localNotWord = false;
    for(int ii = 0; ii < m_occuranceHistory.count(); ++ii ) {
        const KeyOccurance &o = m_occuranceHistory.at(ii);

        if(!m_boards.at(o.board)->isAlphabet())
            localNotWord = true;

        if(!localNotWord) {
            if(o.type == KeyOccurance::CharSelect) {
                m_predict->addLetter(o.explicitChar.toLower().toLatin1());
            } else if(o.type == KeyOccurance::MousePress) {
                if(!localNotWord) {
                    m_predict->addTouch(toBoardPoint(o.widPoint));
                }
            }
        }
    }
    m_notWord = localNotWord;

    updateWords();
}

void EmbeddableKeyboardWidget::setHint(const QString& hint)
{
    QStringList args = hint.split(" ");
    QString h=args.first();
    
    if (h == "words") {
        layoutBoard();
    }

    //bool wasVisible = isVisible();
    bool boardHasBeenSet = false;

    //qLog(Input) << "PredictiveKeyboard : setHint("""<< hint <<""", head is "<<h<<", full args are "<<args;

    // update microfocus
    //qwsServer->sendIMQuery ( Qt::ImMicroFocus );

    if (h.contains("propernouns")) // no tr
    {
        qLog(Input) << "PredictiveKeyboard::setHint(" << h << ") - setting autocapitalisation";
        // TODO: Set autocapitalisation
        setBoardByType(UpperCase);
        m_autoCap = true;
        m_autoCapitaliseEveryWord = true;
        boardHasBeenSet = true;
    } else if(args.contains("noautocapitalization")) {
        m_autoCap = false;
        m_autoCapitaliseEveryWord = false;
    } else {
        //m_autoCap = true;
        m_autoCap = false;  //###
        m_autoCapitaliseEveryWord = false;
    }

    if ((h == "phone" || h == "int")) { // no tr
        qLog(Input) << "PredictiveKeyboard::setHint(" << h << ") - changing to numbers mode";
        setBoardByType(Numeric);
        boardHasBeenSet = true;
        m_autoCap = false;
    };

    if(!boardHasBeenSet && h == "text" || h == "email" || h == "words")
    {
        if (!m_autoCap || h == "email")
            setBoardByType(LowerCase);
        else
            setBoardByType(UpperCase);
        boardHasBeenSet = true;
    }

    return;
};

/*!
    Filter key events.
    On Qt::Key_Select, the input method discards any preedit space, and commits any other preedit text.  The predictive keyboard never consumes Qt::Key_Select, it is always passed back to the system after processing.
    On Qt::Key_Back, the input method will consume the key press and release only if it has preedit text. If the key event is consumed, the KeyboardWidget will \l{doBackspace()}
    \sa acceptWord(), doBackspace()
*/
bool EmbeddableKeyboardWidget::filter ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
    Q_UNUSED(modifiers);
    Q_UNUSED(autoRepeat);

    if (!hasText())
        return false;

    if(keycode == Qt::Key_Select) {
        // on release, commit text
        if (isPress) return true;
        acceptWord();
        return true;
    }

    //Handle backspace
    if(keycode == Qt::Key_Back) {
        if(isPress) return true;
        doBackspace();
        return true;
    }

    return false;
}

#include "keyboard_p.moc"
