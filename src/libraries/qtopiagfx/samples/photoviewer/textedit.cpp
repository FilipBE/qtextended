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

#include "textedit.h"

TextEdit::TextEdit(GfxCanvasItem *parent)
: GfxCanvasItem(parent), _text(0) , _timer(0), lastKey(0), lastKeyOp(0), line(Qt::white, QSize(10, 1), this)
{
    line.visible().setValue(0.);
    widths << 0;
    newText();
    line.y().setValue(10);
}

void TextEdit::newText()
{
    _text = new GfxCanvasText(QSize(0, 30), this);
    _text->setColor(Qt::white);
    QFont f;
    f.setPointSize(24);
    _text->setFont(f);
    _text->setAlignmentFlags((Qt::AlignmentFlag)(int)(Qt::AlignLeft|Qt::AlignVCenter));
}

static const struct {
    char key;
    const char *options;
} keys[] =
{
  { '2', "abcABC2" },
  { '3', "defDEF3" },
  { '4', "ghiGHI4" },
  { '5', "jklJKL5" },
  { '6', "mnoMNO6" },
  { '7', "pqrsPQRS7" },
  { '8', "tuvTUV8" },
  { '9', "wxyzWXYZ9" }
};

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_0:
        case Qt::Key_Right:
        case Qt::Key_Back:
            break;
        default:
            return;
    }
    e->accept();

    if(_timer) { killTimer(_timer); _timer = 0; }

    if(e->key() == Qt::Key_Right) {
        if(lastKey)
            timerEvent(0);
    } else if(e->key() == Qt::Key_Back) {
        if(!str.isEmpty()) {
            str = str.left(str.count() - 1);
            lastKey = 0;
            lastKeyOp = 0;
            widths.removeLast();
            _text->setText(str);
            emit textChanged(str);
        }

    } else {
        char key = e->text().at(0).toLatin1();

        if(e->key() == Qt::Key_0) {
            str.append(" ");
            lastKey = 0;
            lastKeyOp = 0;
            _text->setText(str);
            widths << _text->image().width();
            emit textChanged(str);

        } else if(key != lastKey) {
            for(int ii = 0; ii < sizeof(keys) / sizeof(keys[0]); ++ii) {
                if(keys[ii].key == key) {
                    emit textChanged(str);
                    str.append(keys[ii].options[0]);
                    lastKey = key;
                    lastKeyOp = 0;
                    _text->setText(str);
                    widths << _text->image().width();
                    emit tentativeTextChanged(str);
                    break;
                }
            }
        } else {
            for(int ii = 0; ii < sizeof(keys) / sizeof(keys[0]); ++ii) {
                if(keys[ii].key == key) {

                    lastKeyOp = (lastKeyOp + 1) % ::strlen(keys[ii].options);
                    str = str.left(str.count() - 1);
                    str.append(keys[ii].options[lastKeyOp]);
                    widths.removeLast();
                    _text->setText(str);
                    widths << _text->image().width();
                    emit tentativeTextChanged(str);
                    break;
                }
            }
        }

        _timer = startTimer(2000);
    }

    if(str.isEmpty() || lastKey == 0) {
        tl.clear();
        tl.move(line.visible(), 0., 150);
    } else {
        tl.clear();
        tl.move(line.visible(), 1., 150);
        line.x().setValue((widths.last() - widths.at(widths.count() - 2)) / 2 + widths.at(widths.count() - 2) + 1);
    }
}

void TextEdit::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_0:
        case Qt::Key_Right:
        case Qt::Key_Back:
            break;
        default:
            return;
    }
    e->accept();
}

void TextEdit::timerEvent(QTimerEvent *)
{
    if(_timer) killTimer(_timer);
    _timer = 0;
    lastKey = 0;
    lastKeyOp = 0;
    tl.clear();
    tl.move(line.visible(), 0., 150);
    emit textChanged(str);
}

QString TextEdit::text() const
{
    return str;
}

void TextEdit::confirmText()
{
    timerEvent(0);
}

