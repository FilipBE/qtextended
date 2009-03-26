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
#include "qribbonselector.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollArea>

class QRibbonSelectorInnerWidget : public QWidget
{
    Q_OBJECT
public:
    QRibbonSelectorInnerWidget(QWidget *parent = 0, Qt::Orientation orientation = Qt::Vertical, bool expando = false);

    void setLabels(const QStringList labels);
    QStringList labels() const;

    void setCurrentIndices(QList<int> indices);
    QList<int> currentIndices() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setExpanding(bool expanding);
    bool expanding() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void indexSelected(int);

private:
    void ensureLayout();

    QStringList mText;
    Qt::Orientation mOrientation;
    int mMaxWidth;
    int mMaxHeight;
    int mMaxLeftWidth;
    int mMaxRightWidth;
    bool mTwoGroups;
    bool mDirty;
    bool mExpanding;
    QSize mSizeHint;
    QList<int> mItemWidths; // for Horizontal mode
};

QRibbonSelectorInnerWidget::QRibbonSelectorInnerWidget(QWidget *parent, Qt::Orientation orientation, bool expando)
    : QWidget(parent), mOrientation(orientation), mDirty(true), mExpanding(expando)
{
}

void QRibbonSelectorInnerWidget::setLabels(QStringList labels)
{
    mText = labels;
    mDirty = true;
    updateGeometry();
}

QStringList QRibbonSelectorInnerWidget::labels() const
{
    return mText;
}

Qt::Orientation QRibbonSelectorInnerWidget::orientation() const
{
    return mOrientation;
}

void QRibbonSelectorInnerWidget::setOrientation(Qt::Orientation orientation)
{
    if (mOrientation != orientation) {
        mOrientation = orientation;
        mDirty = true;
        updateGeometry();
    }
}

bool QRibbonSelectorInnerWidget::expanding() const
{
    return mExpanding;
}

void QRibbonSelectorInnerWidget::setExpanding(bool expando)
{
    if (mExpanding != expando) {
        mExpanding = expando;
        mDirty = true;
        updateGeometry();
    }
}

QSize QRibbonSelectorInnerWidget::sizeHint() const
{
    const_cast<QRibbonSelectorInnerWidget*>(this)->ensureLayout();
    return mSizeHint;
}

QSize QRibbonSelectorInnerWidget::minimumSizeHint() const
{
    return sizeHint();
}

void QRibbonSelectorInnerWidget::ensureLayout()
{
    if (mDirty) {
        // We'll either have one long line, or two lines
        mMaxWidth = 0;
        mMaxLeftWidth = 0;
        mMaxRightWidth = 0;
        mItemWidths.clear();

        QFontMetricsF fm(fontMetrics());

        // XXX This is a bad assumption
        QSize available = QApplication::desktop()->availableGeometry().size();

        /* See if we have to change the font size to fit */
        if (mOrientation == Qt::Horizontal) {
            // XXX this has to happen after we measure, not before

        } else {
            // Vertical
            if (fm.lineSpacing() * ((mText.count() + 1)/2) > available.height() - 4 && mText.count() > 0) {
                qreal requiredSpacing = (available.height() - 5 )/ ((mText.count() + 1) / 2);
                QFont f = font();
                qreal pointSize = f.pointSizeF();

                while (pointSize > 0) {
                    QFontMetricsF fm2(f);
                    if (fm2.lineSpacing() <= requiredSpacing) {
                        break;
                    }
                    pointSize -= 0.5f;
                    f.setPointSizeF(pointSize);
                }

                setFont(f);
                fm = QFontMetricsF(f);
            }
        }

        if (mOrientation == Qt::Horizontal) {
            foreach(QString text, mText) {
                int width = (int) fm.width(text) + 2;
                mMaxWidth += width;
                mItemWidths.append(width);
            }
        } else {
            int i = 0;
            foreach(QString text, mText) {
                int width = (int) fm.width(text);
                mMaxWidth = qMax(mMaxWidth, width);
                if (i++ & 1)
                    mMaxRightWidth = qMax(mMaxRightWidth, width);
                else
                    mMaxLeftWidth = qMax(mMaxLeftWidth, width);
            }
        }

        if (mExpanding || mOrientation == Qt::Horizontal) {
            mTwoGroups = false;
        } else {
            // See if we need to split or not
            if (available.height() >= 4 + (fm.lineSpacing() * mText.count()))
                mTwoGroups = false; // One group
            else
                mTwoGroups = true;  // Two groups
        }

        if (mOrientation == Qt::Horizontal) {
            if (mExpanding)
                mSizeHint =  QSize(mMaxWidth, (int) fm.lineSpacing() + 10);
            else {
                if (mTwoGroups)
                    mSizeHint = QSize(available.width(), (2 * (int)fm.lineSpacing()) + 14);
                else
                    mSizeHint = QSize(available.width(), (int)fm.lineSpacing() + 10);
            }
        } else {
            if (mExpanding)
                mSizeHint = QSize(mMaxWidth + 10, 8 + mText.count() * (int)fm.lineSpacing()); // 4px top and bottom
            else {
                if (mTwoGroups)
                    mSizeHint = QSize(mMaxLeftWidth + mMaxRightWidth + 14, available.height());
                else
                    mSizeHint = QSize(mMaxWidth + 10, available.height());
            }
        }

        mDirty = false;

        // Orientation affects the size policy, but expansion affects the sizehint
        if (mOrientation == Qt::Horizontal) {
            QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
            setSizePolicy(policy);
        } else {
            QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
            setSizePolicy(policy);
        }

        updateGeometry();
    }
}

void QRibbonSelectorInnerWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    bool rtl = layoutDirection() == Qt::RightToLeft;

    QFontMetrics fm = fontMetrics();
    QTextOption to;
    to.setAlignment(Qt::AlignCenter);

    if (mOrientation == Qt::Horizontal) {
        // Horizontal ribbon
        QLinearGradient lg(0,0,0, width());
        lg.setColorAt(qreal(0.0f), QColor(108,108,108));
        lg.setColorAt(qreal(0.5f), QColor(148,148,148));
        lg.setColorAt(qreal(1.0f), QColor(108,108,108));

        // Gradient with a thin line on the bottom
        p.fillRect(rect().adjusted(0, 0, 0, -2), lg);
        p.fillRect(rect().adjusted(0, height() - 2, 0, 0), QColor(80,80,80));

        // Draw each in turn.
        int extraWidth = rect().width() - 4 - mMaxWidth;
        qreal extraPadding = mText.count() <= 1 ? 0 : extraWidth / mText.count();

        // Try to space things out
        QRectF r(4, 2, mMaxWidth, fm.lineSpacing());
        for (int i = 0; i < mText.count(); i++) {
            r.setRight(r.left() + mItemWidths.value(i) + extraPadding);
            p.drawText(r, mText.value(i), to);
            r.translate(r.width(), 0);
        }
    } else {
        // Vertical ribbon
        QLinearGradient lg(0,0,0, height());
        lg.setColorAt(qreal(0.0f), QColor(108,108,108));
        lg.setColorAt(qreal(0.5f), QColor(148,148,148));
        lg.setColorAt(qreal(1.0f), QColor(108,108,108));

        // Gradient with a thin line on the side
        p.fillRect(rect().adjusted(rtl ? 0 : 2, 0, rtl ? -2 : 0,0), lg);
        p.fillRect(rect().adjusted(rtl ? width() - 2 : 0,0,rtl ? 0 : 2 - rect().width(),0), QColor(80,80,80));

        int height = rect().height() - 4;

        // Now our alphabet...
        if (mTwoGroups) {
            int totalRows = (mText.count() + 1) / 2;
            qreal extraPadding = totalRows == 0 ? 0 : (height - totalRows * fm.lineSpacing()) / totalRows;
            qreal extraWidth = (width() - 4 - mMaxLeftWidth - mMaxRightWidth) / 2;

            // Two columns
            QRectF left(rtl ? width() - 2 - extraWidth - mMaxLeftWidth : 4, 4, mMaxLeftWidth + extraWidth, fm.lineSpacing() + extraPadding);
            QRectF right(rtl ? 4 : width() - 2 - extraWidth - mMaxRightWidth, 4, mMaxRightWidth + extraWidth, fm.lineSpacing() + extraPadding);

            for (int i = 0; i < mText.count(); i++) {
                if (i & 1) {
                    p.drawText(right, mText.value(i), to);
                    right.translate(0, right.height());
                } else {
                    p.drawText(left, mText.value(i), to);
                    left.translate(0, left.height());
                }
            }
        } else {
            // Just one column
            int extraHeight = height - (mText.count() * fm.lineSpacing());
            qreal extraPadding = mText.count() <= 1 ? 0 : extraHeight / mText.count();
            int hPos = width() - mMaxWidth + rtl ? 0 : 2;

            // Try to space things out
            QRectF r(hPos, 2, mMaxWidth, fm.lineSpacing() + extraPadding);
            for (int i = 0; i < mText.count(); i++) {
                p.drawText(r, mText.value(i), to);
                r.translate(0, r.height());
            }
        }
    }
}

void QRibbonSelectorInnerWidget::mousePressEvent(QMouseEvent *me)
{
    int band = -1;
    bool rtl = layoutDirection() == Qt::RightToLeft;
    if (mOrientation == Qt::Horizontal) {
        band = (me->pos().x() * mText.count()) / width();
    } else {
        if (mTwoGroups) {
            int numRows = (mText.count() + 1) / 2;
            band = 2 * ((me->pos().y() * numRows) / height());

            // Adjust horizontally
            if (me->pos().x() > (width() / 2)) {
                if (!rtl)
                    band = qMin(mText.count() - 1, band + 1);
            } else {
                if (rtl)
                    band = qMin(mText.count() - 1, band + 1);
            }
        } else
            band = (me->pos().y() * mText.count()) / height();
    }

    if (band >= 0 && band < mText.count())
        emit indexSelected(band);
}

void QRibbonSelectorInnerWidget::mouseMoveEvent(QMouseEvent *me)
{
    if (me->buttons() & Qt::LeftButton) {
        mousePressEvent(me);
    }
}

class SmallerScrollArea : public QScrollArea
{
    public:
        QSize minimumSizeHint() const {return QSize(1,1);}
};

QRibbonSelector::QRibbonSelector(QWidget *parent, Qt::Orientation orientation, bool expando)
    : QWidget(parent)
{
    // We create a scrollarea for the actual contents
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);


    mInner = new QRibbonSelectorInnerWidget(0, orientation, expando);
    connect(mInner, SIGNAL(indexSelected(int)), this, SIGNAL(indexSelected(int)));

    vl->addWidget(mInner);
    setLayout(vl);
}

void QRibbonSelector::setLabels(QStringList labels)
{
    mInner->setLabels(labels);
}

QStringList QRibbonSelector::labels() const
{
    return mInner->labels();
}

void QRibbonSelector::setOrientation(Qt::Orientation orientation)
{
    mInner->setOrientation(orientation);
}

Qt::Orientation QRibbonSelector::orientation() const
{
    return mInner->orientation();
}

void QRibbonSelector::setExpanding(bool expanding)
{
    mInner->setExpanding(expanding);
}

bool QRibbonSelector::expanding() const
{
    return mInner->expanding();
}

#include "qribbonselector.moc"
