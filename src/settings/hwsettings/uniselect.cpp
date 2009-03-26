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

#include "uniselect.h"

#include <qtopianamespace.h>

#include <QComboBox>
#include <QFont>
#include <QFontMetrics>
#include <QLayout>
#include <QPainter>
#include <QScrollArea>
#include <QPaintEvent>

typedef struct BlockMap {
    ushort start;
    ushort stop;
    const char *name;
};

//# Start Code; Block Name

static const BlockMap blockMap[] =
{
{0x0000, 0x007F, QT_TRANSLATE_NOOP("UniKeyboard", "Basic Latin")},
{0x0080, 0x00FF, QT_TRANSLATE_NOOP("UniKeyboard", "Latin-1 Supplement")},
{0x0100, 0x017F, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended-A")},
{0x0180, 0x024F, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended-B")},
{0x0250, 0x02AF, QT_TRANSLATE_NOOP("UniKeyboard", "IPA Extensions")},
{0x02B0, 0x02FF, QT_TRANSLATE_NOOP("UniKeyboard", "Spacing Modifier Letters")},
{0x0300, 0x036F, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Diacritical Marks")},
{0x0370, 0x03FF, QT_TRANSLATE_NOOP("UniKeyboard", "Greek")},
{0x0400, 0x04FF, QT_TRANSLATE_NOOP("UniKeyboard", "Cyrillic")},
{0x0530, 0x058F, QT_TRANSLATE_NOOP("UniKeyboard", "Armenian")},
{0x0590, 0x05FF, QT_TRANSLATE_NOOP("UniKeyboard", "Hebrew")},
{0x0600, 0x06FF, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic")},
{0x0700, 0x074F, QT_TRANSLATE_NOOP("UniKeyboard", "Syriac")},
{0x0780, 0x07BF, QT_TRANSLATE_NOOP("UniKeyboard", "Thaana")},
{0x0900, 0x097F, QT_TRANSLATE_NOOP("UniKeyboard", "Devanagari")},
{0x0980, 0x09FF, QT_TRANSLATE_NOOP("UniKeyboard", "Bengali")},
{0x0A00, 0x0A7F, QT_TRANSLATE_NOOP("UniKeyboard", "Gurmukhi")},
{0x0A80, 0x0AFF, QT_TRANSLATE_NOOP("UniKeyboard", "Gujarati")},
{0x0B00, 0x0B7F, QT_TRANSLATE_NOOP("UniKeyboard", "Oriya")},
{0x0B80, 0x0BFF, QT_TRANSLATE_NOOP("UniKeyboard", "Tamil")},
{0x0C00, 0x0C7F, QT_TRANSLATE_NOOP("UniKeyboard", "Telugu")},
{0x0C80, 0x0CFF, QT_TRANSLATE_NOOP("UniKeyboard", "Kannada")},
{0x0D00, 0x0D7F, QT_TRANSLATE_NOOP("UniKeyboard", "Malayalam")},
{0x0D80, 0x0DFF, QT_TRANSLATE_NOOP("UniKeyboard", "Sinhala")},
{0x0E00, 0x0E7F, QT_TRANSLATE_NOOP("UniKeyboard", "Thai")},
{0x0E80, 0x0EFF, QT_TRANSLATE_NOOP("UniKeyboard", "Lao")},
{0x0F00, 0x0FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Tibetan")},
{0x1000, 0x109F, QT_TRANSLATE_NOOP("UniKeyboard", "Myanmar")},
{0x10A0, 0x10FF, QT_TRANSLATE_NOOP("UniKeyboard", "Georgian")},
{0x1100, 0x11FF, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Jamo")},
{0x1200, 0x137F, QT_TRANSLATE_NOOP("UniKeyboard", "Ethiopic")},
{0x13A0, 0x13FF, QT_TRANSLATE_NOOP("UniKeyboard", "Cherokee")},
{0x1400, 0x167F, QT_TRANSLATE_NOOP("UniKeyboard", "Unified Canadian Aboriginal Syllabics")},
{0x1680, 0x169F, QT_TRANSLATE_NOOP("UniKeyboard", "Ogham")},
{0x16A0, 0x16FF, QT_TRANSLATE_NOOP("UniKeyboard", "Runic")},
{0x1780, 0x17FF, QT_TRANSLATE_NOOP("UniKeyboard", "Khmer")},
{0x1800, 0x18AF, QT_TRANSLATE_NOOP("UniKeyboard", "Mongolian")},
{0x1E00, 0x1EFF, QT_TRANSLATE_NOOP("UniKeyboard", "Latin Extended Additional")},
{0x1F00, 0x1FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Greek Extended")},
{0x2000, 0x206F, QT_TRANSLATE_NOOP("UniKeyboard", "General Punctuation")},
{0x2070, 0x209F, QT_TRANSLATE_NOOP("UniKeyboard", "Superscripts and Subscripts")},
{0x20A0, 0x20CF, QT_TRANSLATE_NOOP("UniKeyboard", "Currency Symbols")},
{0x20D0, 0x20FF, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Marks for Symbols")},
{0x2100, 0x214F, QT_TRANSLATE_NOOP("UniKeyboard", "Letterlike Symbols")},
{0x2150, 0x218F, QT_TRANSLATE_NOOP("UniKeyboard", "Number Forms")},
{0x2190, 0x21FF, QT_TRANSLATE_NOOP("UniKeyboard", "Arrows")},
{0x2200, 0x22FF, QT_TRANSLATE_NOOP("UniKeyboard", "Mathematical Operators")},
{0x2300, 0x23FF, QT_TRANSLATE_NOOP("UniKeyboard", "Miscellaneous Technical")},
{0x2400, 0x243F, QT_TRANSLATE_NOOP("UniKeyboard", "Control Pictures")},
{0x2440, 0x245F, QT_TRANSLATE_NOOP("UniKeyboard", "Optical Character Recognition")},
{0x2460, 0x24FF, QT_TRANSLATE_NOOP("UniKeyboard", "Enclosed Alphanumerics")},
{0x2500, 0x257F, QT_TRANSLATE_NOOP("UniKeyboard", "Box Drawing")},
{0x2580, 0x259F, QT_TRANSLATE_NOOP("UniKeyboard", "Block Elements")},
{0x25A0, 0x25FF, QT_TRANSLATE_NOOP("UniKeyboard", "Geometric Shapes")},
{0x2600, 0x26FF, QT_TRANSLATE_NOOP("UniKeyboard", "Miscellaneous Symbols")},
{0x2700, 0x27BF, QT_TRANSLATE_NOOP("UniKeyboard", "Dingbats")},
{0x2800, 0x28FF, QT_TRANSLATE_NOOP("UniKeyboard", "Braille Patterns")},
{0x2E80, 0x2EFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Radicals Supplement")},
{0x2F00, 0x2FDF, QT_TRANSLATE_NOOP("UniKeyboard", "Kangxi Radicals")},
{0x2FF0, 0x2FFF, QT_TRANSLATE_NOOP("UniKeyboard", "Ideographic Description Characters")},
{0x3000, 0x303F, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Symbols and Punctuation")},
{0x3040, 0x309F, QT_TRANSLATE_NOOP("UniKeyboard", "Hiragana")},
{0x30A0, 0x30FF, QT_TRANSLATE_NOOP("UniKeyboard", "Katakana")},
{0x3100, 0x312F, QT_TRANSLATE_NOOP("UniKeyboard", "Bopomofo")},
{0x3130, 0x318F, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Compatibility Jamo")},
{0x3190, 0x319F, QT_TRANSLATE_NOOP("UniKeyboard", "Kanbun")},
{0x31A0, 0x31BF, QT_TRANSLATE_NOOP("UniKeyboard", "Bopomofo Extended")},
{0x3200, 0x32FF, QT_TRANSLATE_NOOP("UniKeyboard", "Enclosed CJK Letters and Months")},
{0x3300, 0x33FF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility")},
{0x3400, 0x4DB5, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Unified Ideographs Extension A")},
{0x4E00, 0x9FFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Unified Ideographs")},
{0xA000, 0xA48F, QT_TRANSLATE_NOOP("UniKeyboard", "Yi Syllables")},
{0xA490, 0xA4CF, QT_TRANSLATE_NOOP("UniKeyboard", "Yi Radicals")},
{0xAC00, 0xD7A3, QT_TRANSLATE_NOOP("UniKeyboard", "Hangul Syllables")},
{0xD800, 0xDB7F, QT_TRANSLATE_NOOP("UniKeyboard", "High Surrogates")},
{0xDB80, 0xDBFF, QT_TRANSLATE_NOOP("UniKeyboard", "High Private Use Surrogates")},
{0xDC00, 0xDFFF, QT_TRANSLATE_NOOP("UniKeyboard", "Low Surrogates")},
{0xE000, 0xF8FF, QT_TRANSLATE_NOOP("UniKeyboard", "Private Use")},
{0xF900, 0xFAFF, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility Ideographs")},
{0xFB00, 0xFB4F, QT_TRANSLATE_NOOP("UniKeyboard", "Alphabetic Presentation Forms")},
{0xFB50, 0xFDFF, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic Presentation Forms-A")},
{0xFE20, 0xFE2F, QT_TRANSLATE_NOOP("UniKeyboard", "Combining Half Marks")},
{0xFE30, 0xFE4F, QT_TRANSLATE_NOOP("UniKeyboard", "CJK Compatibility Forms")},
{0xFE50, 0xFE6F, QT_TRANSLATE_NOOP("UniKeyboard", "Small Form Variants")},
{0xFE70, 0xFEFE, QT_TRANSLATE_NOOP("UniKeyboard", "Arabic Presentation Forms-B")},
{0xFF00, 0xFFEF, QT_TRANSLATE_NOOP("UniKeyboard", "Halfwidth and Fullwidth Forms")},
{0xFFF0, 0xFFFD, QT_TRANSLATE_NOOP("UniKeyboard", "Specials")},
{0xFFFF, 0xFFFF,        0} };

class CharacterView : public QWidget
{
    Q_OBJECT

    friend class UniSelect;
    friend class ScrolledWidget;
public:
    CharacterView(QWidget *parent, UniSelect *parentUniSelect);

    uint character() const;
    QString text() const;

    void addSpecial(uint, const QString &);
    void clearSpecials();
public slots:
    void selectSet(int);

signals:
    void selected(uint);
    void selected(const QString &);

protected slots:
    void updateSelectedSet(int x, int y);

protected:
    void keyPressEvent(QKeyEvent *e);
    void setSetForIndex(int ind);

    void paintEvent(QPaintEvent *event);
    void drawCell(QPainter *p, const QRect &, const QString &, int);

    void setHighlighted(int);
    void setIndex(int);
    virtual bool ensureVisible(int index);

    void updateCellForIndex(int ind);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *);

    void fontChange(const QFont &);
    void layoutGlyphs();

    int numGlyphs() const;

    virtual void focusInEvent ( QFocusEvent * event );
private:
    void addNonPrinting();

    QRect bounds( int ) const;
    int index( const QPoint & ) const;

    void coordToIndex(int row, int col, int &ind) const;
    void indexToCoord(int ind, int &srow, int &scol) const;

    void fillMap();

    UniSelect *uniSelect;

    QVector<ushort> mGlyphMap;
    QMap<int, int> mSetMap;

    QPoint glyphSize;
    uchar glyphsPerRow;
    uchar glyphsPerCol;
    int mHighlighted;
    int mCurrent;
    struct SpecialChar {
        uint code;
        QString name;
    };
    QList<SpecialChar> mSpecials;
    bool updatingCurrent;
};

CharacterView::CharacterView(QWidget *parent, UniSelect *parentUniSelect)
    : QWidget(parent), uniSelect(parentUniSelect), mHighlighted(-1), mCurrent(-1),
    updatingCurrent(false)
{
//    QWidget *vw = new QWidget();
//    vw->setObjectName( "viewground" );
//    setWidget( vw );
    fillMap();
    layoutGlyphs();

    setFocusPolicy( Qt::StrongFocus );

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CharacterView::layoutGlyphs()
{
    // wait till proper resize.

    QFontMetrics fm = font();
    glyphSize = QPoint((fm.maxWidth()*3)/2, (fm.height()*3)/2);

    if (parentWidget()->width() < 1)
        glyphsPerRow = 8;
    else
        glyphsPerRow = qMax(1, ((parentWidget()->width() - 1) / glyphSize.x())-1); // width()-1 to allow one pixel for the right edge line.

    // Pad out the cell if necessary to make it look better
    int horizontalRemainder = parentWidget()->width() - 1 - (glyphSize.x() * glyphsPerRow); // -1 to allow one pixel for the right edge line.
    int horizontalPadding = horizontalRemainder/(glyphsPerRow);
    if(horizontalPadding > 0)
    {
        glyphSize.setX(glyphSize.x()+horizontalPadding);
    };

    int width = glyphSize.x() * glyphsPerRow + 1;

    glyphsPerCol = 1 + numGlyphs()/glyphsPerRow;

    int height = glyphSize.y() * glyphsPerCol + 1;

    setMaximumSize(width, height + (mSpecials.count() * glyphSize.y()));
    resize(width, height + (mSpecials.count() * glyphSize.y()));
    mHighlighted = -1;
    update();
}

void CharacterView::setIndex(int ind)
{
    if (ind != mCurrent) {
        if (ind < 0 || ind >= (int)mSpecials.count() + numGlyphs()) {
            ind = -1;
        }
        int oldindex = mCurrent;
        mCurrent = ind;
        if (mCurrent != -1) {
            /* need to check map in case need to update combo box */

            setSetForIndex(mCurrent);
            updateCellForIndex(mCurrent);
            ensureVisible(mCurrent);

            emit selected(character());
            emit selected(text());
        }
        if (oldindex != -1)
            updateCellForIndex(oldindex);
    }
}

bool CharacterView::ensureVisible(int index)
{
    QRect currentCellBounds = bounds(index);

    if(!parent() || !parent()->parent()) return false;
    QScrollArea* scroller = qobject_cast<QScrollArea*>(parent()->parent());

    if(scroller){
        scroller->ensureVisible(currentCellBounds.center().x(), currentCellBounds.center().y(), currentCellBounds.width(), currentCellBounds.height()*2);
        return true;
    } else {
        return false;
    };
}

void CharacterView::keyPressEvent(QKeyEvent *e)
{
    bool ignore=true;
/*    if( !Qtopia::mousePreferred() ) {
        if (!hasEditFocus()) {
            if (e->key() == Qt::Key_Select) {
                setEditFocus(true);
                ensureVisible(mCurrent); // TEMP: hack around scrollarea moving on defocus
                updateCellForIndex(mCurrent);
            } else {
                e->ignore();
            }
            return;
        }
    }*/

    int srow,scol;
    if(mHighlighted == -1)
        indexToCoord(mCurrent, srow, scol);
    else
        indexToCoord(mHighlighted, srow, scol);
    int row = srow;
    int col = scol;
    switch(e->key()) {
        case Qt::Key_Up:
            if (row > 0) {
                row--;
            }
            else e->ignore();
            break;
        case Qt::Key_Down:
            if (row < glyphsPerCol + (int)mSpecials.count()){
                row++;
            }
            else e->ignore();
            break;
        case Qt::Key_Left:
            if (col > 0) {
                col--;
            }
            break;
        case Qt::Key_Right:
            if (col < glyphsPerRow-1){
                col++;
            }
            break;
        case Qt::Key_Space:
        case Qt::Key_Select:
            {
                int old = mCurrent;
                setIndex(mHighlighted);
                setHighlighted(-1);
                updateCellForIndex(mHighlighted);
                updateCellForIndex(old);
            }
            break;
        default:
            ;
    }
    if (srow != row || scol != col) {
        int ind;
        coordToIndex(row, col, ind);
        if (ind != -1){
            setHighlighted(ind);
            uniSelect->ensureVisible(ind);
        }
    }
    if(ignore)
        e->ignore();
}

void CharacterView::setHighlighted(int glyphindex)
{
    if (mHighlighted != glyphindex) {
        if (glyphindex < 0 || glyphindex >= numGlyphs()+(int)mSpecials.count())
            glyphindex = -1;
        if (mHighlighted != -1)
            updateCellForIndex(mHighlighted);
        mHighlighted = glyphindex;
        if (mHighlighted != -1)
            updateCellForIndex(mHighlighted);
    }
}

uint CharacterView::character() const
{
    if (mCurrent == -1) {
        return 0;
    } else if (mCurrent < (int)mSpecials.count()) {
        return mSpecials[mCurrent].code;
    } else {
        return mGlyphMap[mCurrent-mSpecials.count()];
    }
}

QString CharacterView::text() const
{
    if (mCurrent == -1) {
        return QString();
    } else if (mCurrent < (int)mSpecials.count()) {
        return mSpecials[mCurrent].name;
    } else {
        ushort code = mGlyphMap[mCurrent-mSpecials.count()];
        return QString(QChar(code));
    }
}

void CharacterView::paintEvent(QPaintEvent *event)
{
    const QRect eventRect = event->rect();
    const int x = event->rect().x();
    const int y = event->rect().y();
    const int h = event->rect().height();
    const int w = event->rect().width();

    QPainter p(this);

    int startrow = y ? y / glyphSize.y() : 0;
    int endrow = (y+h) ? (y+h) / glyphSize.y() : 0;

    int startcol = x ? x / glyphSize.x() : 0;
    startcol = qMin(glyphsPerRow-1, startcol);
    int endcol = (x+w) ? (x+w) / glyphSize.x() : 0;
    endcol = qMin(glyphsPerRow-1, endcol);

    for (int row = startrow; row <= endrow; ++row) {
        if (row < (int)mSpecials.count()) {
            drawCell(&p, bounds(row), mSpecials[row].name, row);
        } else {
            int glyphrow = row - mSpecials.count();
            for (int col = startcol; col <= endcol; ++col) {
                int glyphindex = glyphrow*glyphsPerRow+col;
                if (glyphindex < numGlyphs()) {
                    QChar glyph = mGlyphMap[glyphindex];
                    int index = glyphindex + mSpecials.count();
                    drawCell(&p, bounds(index), QString( glyph ), index);
                }
            }
        }
    }
}

void CharacterView::drawCell(QPainter *p, const QRect &b,
        const QString &text, int glyphindex)
{
    QRect bounds = b;
    if (glyphindex == mCurrent) {
        p->setBrush(palette().color(QPalette::Shadow));
        if (!Qtopia::mousePreferred() && hasEditFocus()) {
            p->setPen(palette().color(QPalette::Highlight));
            p->drawRect(bounds);
            bounds.setRect(bounds.x()+1, bounds.y()+1,
                    bounds.width()-2,bounds.height()-2);
            p->drawRect(bounds);
            p->setPen(palette().color(QPalette::BrightText));
        } else
        {
            p->setPen(palette().color(QPalette::Dark));
            p->drawRect(bounds);
            p->setPen(palette().color(QPalette::BrightText));
        }
    } else if (glyphindex == mHighlighted) {
        p->setBrush(palette().color(QPalette::Highlight));
        p->setPen(palette().color(QPalette::HighlightedText));
        p->drawRect(bounds);
    } else {
        p->setBrush(palette().color(QPalette::Base));
        p->setPen(palette().color(QPalette::Text));
        p->drawRect(bounds);
    }

    p->drawText(bounds, Qt::AlignCenter, text);
}

void CharacterView::updateCellForIndex(int ind)
{
     QRect temp=bounds(ind);
     temp.adjust(0,0,0,2);
     update(temp);
//    update(bounds(ind));
}

QRect CharacterView::bounds(int ind) const
{
    if (ind < (int)mSpecials.count())
        return QRect(0, ind*glyphSize.y(), glyphSize.x()*glyphsPerRow, glyphSize.y());
    else {
        ind -= mSpecials.count();
        int srow = ind ? ind / glyphsPerRow: 0;
        int scol = ind ? ind % glyphsPerRow: 0;
        srow += mSpecials.count();
        return QRect(scol*glyphSize.x(), srow*glyphSize.y(), glyphSize.x(), glyphSize.y());
    }
}

int CharacterView::index(const QPoint &pos) const
{
    int row = pos.y() ? pos.y() / glyphSize.y() : 0;
    int col = pos.x() ? pos.x() / glyphSize.x() : 0;
    if (col >= glyphsPerRow) {
        return -1;
    } else if (row < (int)mSpecials.count()) {
        return row;
    } else {
        row -= mSpecials.count();
        return (row*glyphsPerRow)+col+mSpecials.count();
    }
}

void CharacterView::coordToIndex(int row, int col, int &ind) const
{
    if (col >= glyphsPerRow) {
        ind = -1;
    } else if (row < (int)mSpecials.count()) {
        ind = row;
    } else {
        row -= mSpecials.count();
        ind = (row*glyphsPerRow)+col+mSpecials.count();
    }
    if (ind < 0 || ind >= (int)(mGlyphMap.count() + mSpecials.count()))
        ind = -1;
}

void CharacterView::indexToCoord(int ind, int &srow, int &scol) const
{
    if (ind < (int)mSpecials.count()) {
        srow = ind;
        scol = 0;
    } else {
        ind -= mSpecials.count();
        srow = ind ? ind / glyphsPerRow : 0;
        scol = ind ? ind % glyphsPerRow : 0;
        srow += mSpecials.count();
    }
}

void CharacterView::resizeEvent(QResizeEvent *)
{
    layoutGlyphs();
}

void CharacterView::fontChange(const QFont &)
{
    fillMap();
    layoutGlyphs();
}

int CharacterView::numGlyphs() const
{
    return mGlyphMap.size();
}

// Assume same name isn't already present.
void CharacterView::addSpecial(uint code, const QString &name)
{
    if ( code == 0 ) {
        if (mSpecials.count() == 0) {
            addNonPrinting();
        }
        // ok to add
        SpecialChar sp;
        sp.code = code;
        sp.name = name;
        mSpecials.append(sp);
        // resizeContents for extra row.
        resize(width(), height() + glyphSize.y());
        // update entire contents since at beginning.
        update( rect() );
        if (mCurrent > (int)mSpecials.count())
            setIndex(mCurrent+1);
    }
}

void CharacterView::clearSpecials()
{
    if (mSpecials.count() > 0) {
        for (int i = mSpecials.count(); i > 0; i--) {
            uniSelect->mSetSelect->removeItem(0);
        }
        int countwas = mSpecials.count();
        resize( width(), height() - (mSpecials.count() * glyphSize.y()));
        mSpecials.clear();
        update( rect() );
        if (mCurrent < countwas && mCurrent != -1)
            setIndex(0);
    }
}

void CharacterView::updateSelectedSet(int x, int y)
{
    setSetForIndex(index(QPoint(x,y)));
}

void CharacterView::setSetForIndex(int ind)
{
    if (updatingCurrent)
        return;
    QMap<int,int>::iterator it;
    for( it = mSetMap.begin(); it != mSetMap.end(); ++it ) {
        int index = it.value() + mSpecials.count();
        int cindex = it.key();
        if (mSpecials.count() > 0)
            cindex++;
        if (ind < index) {
            // set to previous...
            if (cindex > 0) {
                uniSelect->mSetSelect->blockSignals(true);
                uniSelect->mSetSelect->setCurrentIndex(cindex-1);
                uniSelect->mSetSelect->blockSignals(false);
            }
            break;
        }
    }
}

void CharacterView::selectSet(int item)
{
    if (mSpecials.count()) {
        if (item == 0) {
            setIndex(0);
            return;
        }
        item--;
    }
    if (mSetMap.contains(item)) {
        setIndex(mSetMap[item]+mSpecials.count());
    }
}

void CharacterView::addNonPrinting()
{
    uniSelect->mSetSelect->insertItem(0, tr("Non printing", "refers to [ENTER], [TAB] etc."));
}

void CharacterView::fillMap()
{
    uniSelect->mSetSelect->clear();
    // fill with the specials
    if ( mSpecials.count() > 0 ) {
        addNonPrinting();
    }
    mGlyphMap.resize(0xffff);
    QFontMetrics fm(font());
    int index = 0; // index into array
    int cindex = 0; // index into combo box (ignores nonprinting)
    int bindex = 0; // index into named sets (blockMap)
    for (ushort candidate = 0x0000; candidate < 0xffff; ++candidate) {
        QChar glyph = candidate;
        if (glyph.isPrint() && fm.inFont(glyph)) {
            while (blockMap[bindex].stop < candidate) {
                bindex++;
            }
            if (blockMap[bindex].start <= candidate) {
                uniSelect->mSetSelect->addItem(tr(blockMap[bindex].name));
                mSetMap[cindex] = index;
                cindex++;
                bindex++;
            }
            mGlyphMap[index] = candidate;
            index++;
        }
    }
    mGlyphMap.resize(index);
    mCurrent = (index > 0 || mSpecials.count() > 0) ? 0 : -1;
}

void CharacterView::mousePressEvent(QMouseEvent *e)
{
    setHighlighted(index(e->pos()));
    QWidget::mousePressEvent(e);
}

void CharacterView::mouseMoveEvent(QMouseEvent *e)
{
    setHighlighted(index(e->pos()));
    QWidget::mouseMoveEvent(e);
}

void CharacterView::mouseReleaseEvent(QMouseEvent *e)
{
    setHighlighted(-1);
    setIndex(index(e->pos()));
    QWidget::mouseReleaseEvent(e);
}

void CharacterView::focusInEvent ( QFocusEvent * event )
{
    Q_UNUSED(event);
    uniSelect-> ensureVisible(mCurrent);
};
/*
   XXX still need to deal with font changes properly.
 */
UniSelect::UniSelect( QWidget *parent, const char *name, Qt::WFlags f )
    : QWidget( parent, f )
{
    setObjectName( name );
    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setSpacing(4);
    mSetSelect = new QComboBox(this);
    vl->addWidget(mSetSelect);

    mScroller = new QScrollArea(this);
    mScroller->setFocusPolicy(Qt::NoFocus);
    mGlyphSelect = new CharacterView(mScroller, this);
    mScroller->setWidget(mGlyphSelect);
    vl->addWidget(mScroller);

    connect(mSetSelect, SIGNAL(activated(int)), mGlyphSelect, SLOT(selectSet(int)));
    connect(mGlyphSelect, SIGNAL(selected(uint)),
            this, SIGNAL(selected(uint)));
    connect(mGlyphSelect, SIGNAL(selected(QString)),
            this, SIGNAL(selected(QString)));
}

UniSelect::~UniSelect()
{
}

void UniSelect::addSpecial(uint val, const QString &name)
{
    mGlyphSelect->addSpecial(val, name);
}

void UniSelect::clearSpecials()
{
    mGlyphSelect->clearSpecials();
}

uint UniSelect::character() const
{
    return mGlyphSelect->character();
}

QString UniSelect::text() const
{
    return mGlyphSelect->text();
}

void UniSelect::ensureVisible(int index)
{
    QRect bounds = mGlyphSelect->bounds(index);
    // find the midpoint, and set the margin to half the width/height +50
    int xMid = bounds.center().x();
    int yMid = bounds.center().y();
    int xMargin = xMid-bounds.left()+50;
    int yMargin = yMid-bounds.top()+50;

    mScroller->ensureVisible(xMid, yMid, xMargin, yMargin);
};
#include "uniselect.moc"
