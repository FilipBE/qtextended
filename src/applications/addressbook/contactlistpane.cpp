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

#include "contactlistpane.h"
#include "addressbook.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>

#include "qcontactmodel.h"
#include "qcontactview.h"
#include "qtopiaapplication.h"
#include "qcategoryselector.h"
#include "qsoftmenubar.h"
#include <QTextEntryProxy>
#include <QtopiaItemDelegate>
#include <QPainter>
#include <QTimer>
#include <QScrollArea>

#ifdef QTOPIA_HOMEUI
#include "deskphonewidgets.h"
#include "contactdetails.h"
#endif

class ContactsAlphabetInnerWidget;
class ContactsAlphabetRibbon : public QWidget
{
    Q_OBJECT;
public:
    ContactsAlphabetRibbon(QWidget *parent, Qt::Orientation orientation = Qt::Vertical);

    void setLabels(const QStringList labels);
    QStringList labels() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

signals:
    void indexSelected(int);

private:
    ContactsAlphabetInnerWidget* mInner;
};

ContactListPane::ContactListPane(QWidget *w, QContactModel* model)
    : QWidget(w), mTextProxy(0), mCategoryLabel(0)
{
    mModel = model;
    connect(mModel, SIGNAL(modelReset()), this, SLOT(contactsChanged()));

    mLayout = new QVBoxLayout();

    mLayout->setMargin(0);
    mLayout->setSpacing(2);

    //
    //  Create the contact list itself.
    //

    mListView = new QSmoothContactListView(0);
    mListView->setEmptyText(tr("No Contacts"));
#ifdef QTOPIA_HOMEUI
    mDelegate = new DeskphoneContactDelegate(mListView);
#else
    if (style()->inherits("Series60Style"))
        mDelegate = new QtopiaItemDelegate(mListView);
    else
        mDelegate = new QContactDelegate(mListView);
#endif

    mListView->setItemDelegate(mDelegate);
    mListView->setModel(mModel);

#ifdef QTOPIA_HOMEUI_WIDE
    Qt::Orientation dir = Qt::Horizontal;
#else
    Qt::Orientation dir = Qt::Vertical;
#endif

    /* If we prefer touch/mouse input, put the ribbon there and hide the filter box (?) */
    if (Qtopia::mousePreferred()) {
        mJumpField = QContactModel::Label;
        mRibbon = new ContactsAlphabetRibbon(this, dir);
        QStringList labels;

        // Similar code to predictive keyboard, but that might not be enabled
        QSettings lcfg("Trolltech", "/AlphabetRibbonLayout");
        QString ilang = QSettings("Trolltech", "locale").value("Language/InputLanguages").toStringList().at(0);
        QSettings icfg("Trolltech", ilang+"/AlphabetRibbonLayout");

        QSettings *cfg = icfg.contains("Contacts/Labels") ? &icfg : &lcfg;

        cfg->beginGroup("Contacts");
        mRibbon->setLabels(cfg->value("Labels").toStringList());
        mJumpTexts = cfg->value("MatchStrings").toStringList();
        cfg->endGroup();

        QFont f = mRibbon->font();
        f.setBold(true);
        mRibbon->setFont(f);

        connect(mRibbon, SIGNAL(indexSelected(int)), this, SLOT(jump(int)));
        QBoxLayout *hl = dir==Qt::Horizontal?(QBoxLayout*)new QVBoxLayout():(QBoxLayout*)new QHBoxLayout();

        hl->setMargin(0);
        hl->setSpacing(0);

        hl->addWidget(mListView, 10);
        hl->addWidget(mRibbon);
        mLayout->addLayout(hl);
    }  else
        mLayout->addWidget(mListView);

    // Don't show find bar for QThumbStyle
    if (!style()->inherits("QThumbStyle")) {
        mTextProxy = new QTextEntryProxy(this, mListView);
        int mFindHeight = mTextProxy->sizeHint().height();
        mFindIcon = new QLabel;
        mFindIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
        mFindIcon->setMargin(2);

        QHBoxLayout *findLayout = new QHBoxLayout;
        findLayout->addWidget(mFindIcon);
        findLayout->addWidget(mTextProxy);
        mLayout->addLayout(findLayout);

        //
        //  If this is a touch screen phone, tie the search box into the contact list.
        //
        connect( mTextProxy, SIGNAL(textChanged(QString)),
                this, SLOT(search(QString)) );

        QtopiaApplication::setInputMethodHint( mListView, QtopiaApplication::Text );
    }


    mListView->installEventFilter(this);

    connect(mListView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(contactActivated(QModelIndex)));
    connect(mListView, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateIcons()));
    mListView->setWhatsThis( tr("List of contacts in the selected category.  Click to view"
            " detailed information.") );


#ifdef QTOPIA_CELL
    mLoadingLabel = 0;
#endif

    setLayout(mLayout);

    QTimer::singleShot(0, this, SLOT(updateIcons()));

#ifdef QTOPIA_HOMEUI
    // Show the launcher when the list is visible
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), "Launcher");
#endif
}

void ContactListPane::resetSearchText()
{
    if (mTextProxy)
        mTextProxy->clear();
}

bool ContactListPane::eventFilter( QObject *o, QEvent *e )
{
    if(o == mListView && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent *)e;
        if ( ke->key() == Qt::Key_Select ) {
            emit contactActivated(mModel->contact(mListView->currentIndex()));
            return true;
        } else if (ke->key() == Qt::Key_Back && (mTextProxy == NULL || mTextProxy->text().length() == 0)) {
            emit closeView();
            return true;
        }
    }
    return false;
}

#ifdef QTOPIA_CELL
void ContactListPane::showLoadLabel(bool show)
{
    if (show) {
        if( !mLoadingLabel ) {
            mLoadingLabel = new QLabel(tr("Loading SIM..."), 0);
            mLoadingLabel->setAlignment(Qt::AlignCenter);
            // Put this above the filter bar
            mLayout->insertWidget(mLayout->count() - 1, mLoadingLabel);
        }
        mLoadingLabel->show();
    } else {
        if (mLoadingLabel)
            mLoadingLabel->hide();
    }
}
#endif

void ContactListPane::jump(int idx)
{
    // See if we can match this
    QString text = mJumpTexts.value(idx);

    // See if we need to rebuild the jump table
    if (mJumpIndices.count() == 0) {
        QModelIndex idx = mModel->index(mModel->rowCount() - 1,0);

        // We reverse the list, because we want missing jumps to go to
        // the next match
        QStringList reversed;

        foreach(QString s, mJumpTexts)
            reversed.prepend(s);

        foreach(QString s, reversed) {
            // This does assume that matched indices are contiguous
            QModelIndex matched = mModel->match(mJumpField, QVariant(s), Qt::MatchWildcard, 0, 1).value(0);
            if (!matched.isValid()) {
                // Use the previous valid index (or the default from above)
                matched = idx;
            }
            idx = matched;
            mJumpIndices.insert(s, matched);
        }
    }

    mListView->scrollTo(mJumpIndices.value(text), QSmoothList::PositionAtTop);
}

void ContactListPane::search( const QString &text )
{
    if (text.isEmpty()) {
        mModel->clearFilter();
    } else {
        mModel->setFilter( text );
    }
}

void ContactListPane::contactActivated(const QModelIndex &idx)
{
    /* We should get a selection changed event before this, so don't do the rest of the processing here */
    emit contactActivated(mModel->contact(idx));
}

/*
   Called when the ContactModel is reset (e.g. after refreshing
   cache from SIM or changing filter).  If we have any contacts, select the first,
   otherwise we show the "new" option.
*/
void ContactListPane::contactsChanged()
{
    if (!mListView->currentIndex().isValid()) {
        QModelIndex newSel = mModel->index(0,0);

        if(newSel.isValid()) {
            mListView->setCurrentIndex(newSel);
//            mListView->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::ClearAndSelect);
//            mListView->scrollTo(newSel, QAbstractItemView::PositionAtCenter);
        } else {
            // view doesn't emit selection changed over a model reset
            emit currentChanged(QModelIndex(), QModelIndex());
        }
    }

    mJumpIndices.clear();
    updateIcons();
}

void ContactListPane::updateIcons()
{
    if(mModel->rowCount() > 0)
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::View);
    else
        QSoftMenuBar::setLabel( this, Qt::Key_Select, "new" /* <- icon filename */, tr("New") );
}

QContact ContactListPane::currentContact() const
{
    return mListView->currentContact();
}

void ContactListPane::closeEvent( QCloseEvent *e)
{
#ifdef QTOPIA_CELL
    if( mLoadingLabel )
        mLoadingLabel->hide();
#endif
    QWidget::closeEvent(e);
}

void ContactListPane::showCategory( const QCategoryFilter &c )
{
    setWindowTitle( tr("Contacts") + " - " + c.label("Address Book") );

    // The model should already be filtered.

    /* XXX for non S60 UI we may want this back */
    /*
    if(c == QCategoryFilter(QCategoryFilter::All)) {
        if (mCategoryLabel)
            mCategoryLabel->hide();
    } else {
        if (!mCategoryLabel) {
            mCategoryLabel = new QLabel();
            mLayout->addWidget(mCategoryLabel);
        }
        mCategoryLabel->setText(tr("Category: %1").arg(c.label("Address Book")));
        mCategoryLabel->show();
    }
    */
}

void ContactListPane::setCurrentContact( const QContact& contact)
{
    QModelIndex idx = mModel->index(contact.uid());
    if ( !idx.isValid())
        idx = mModel->index(0,0);
    if(idx.isValid() && idx != mListView->currentIndex()) {
        mListView->setCurrentIndex(idx);
//        mListView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
//        mListView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    }
}

/* ============================================================ */

class ContactsAlphabetInnerWidget : public QWidget
{
    Q_OBJECT
public:
    ContactsAlphabetInnerWidget(QWidget *parent = 0, Qt::Orientation orientation = Qt::Vertical);

    void setLabels(const QStringList labels);
    QStringList labels() const;

    void setCurrentIndices(QList<int> indices);
    QList<int> currentIndices() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

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
    QSize mSizeHint;
    QList<int> mItemWidths; // for Horizontal mode
};

ContactsAlphabetInnerWidget::ContactsAlphabetInnerWidget(QWidget *parent, Qt::Orientation orientation)
    : QWidget(parent), mOrientation(orientation), mDirty(true)
{
}

void ContactsAlphabetInnerWidget::setLabels(QStringList labels)
{
    mText = labels;
    mDirty = true;
    updateGeometry();
}

QStringList ContactsAlphabetInnerWidget::labels() const
{
    return mText;
}

Qt::Orientation ContactsAlphabetInnerWidget::orientation() const
{
    return mOrientation;
}

void ContactsAlphabetInnerWidget::setOrientation(Qt::Orientation orientation)
{
    if (mOrientation != orientation) {
        mOrientation = orientation;
        mDirty = true;
        updateGeometry();
    }
}

QSize ContactsAlphabetInnerWidget::sizeHint() const
{
    const_cast<ContactsAlphabetInnerWidget*>(this)->ensureLayout();
    return mSizeHint;
}

QSize ContactsAlphabetInnerWidget::minimumSizeHint() const
{
    return sizeHint();
}

void ContactsAlphabetInnerWidget::ensureLayout()
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

        if (mOrientation == Qt::Horizontal) {
            mTwoGroups = false;
        } else {
            // See if we need to split or not
            if (available.height() >= 4 + (fm.lineSpacing() * mText.count()))
                mTwoGroups = false; // One group
            else
                mTwoGroups = true;  // Two groups
        }

        if (mOrientation == Qt::Horizontal) {
            if (mTwoGroups)
                mSizeHint = QSize(available.width(), (2 * (int)fm.lineSpacing()) + 14);
            else
                mSizeHint = QSize(available.width(), (int)fm.lineSpacing() + 10);
        } else {
            if (mTwoGroups)
                mSizeHint = QSize(mMaxLeftWidth + mMaxRightWidth + 14, available.height());
            else
                mSizeHint = QSize(mMaxWidth + 10, available.height());
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

void ContactsAlphabetInnerWidget::paintEvent(QPaintEvent *)
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


    QPainterPath pp;
    pp.addRoundedRect(QRectF(rect()),8,16);
    p.fillPath(pp,lg);

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

void ContactsAlphabetInnerWidget::mousePressEvent(QMouseEvent *me)
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

void ContactsAlphabetInnerWidget::mouseMoveEvent(QMouseEvent *me)
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

ContactsAlphabetRibbon::ContactsAlphabetRibbon(QWidget *parent, Qt::Orientation orientation)
    : QWidget(parent)
{
    // We create a scrollarea for the actual contents
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);

    mInner = new ContactsAlphabetInnerWidget(0, orientation);
    connect(mInner, SIGNAL(indexSelected(int)), this, SIGNAL(indexSelected(int)));

    if (orientation == Qt::Horizontal) {
       QHBoxLayout* hl = new QHBoxLayout;
       hl->addSpacing (25);
       hl->addWidget(mInner);
       hl->addSpacing (25);
       setLayout (hl);
    } else {
        QVBoxLayout *vl = new QVBoxLayout;
        vl->setMargin(0);
        vl->addWidget(mInner);
        setLayout(vl);
    }
}

void ContactsAlphabetRibbon::setLabels(QStringList labels)
{
    mInner->setLabels(labels);
}

QStringList ContactsAlphabetRibbon::labels() const
{
    return mInner->labels();
}

void ContactsAlphabetRibbon::setOrientation(Qt::Orientation orientation)
{
    mInner->setOrientation(orientation);
}

Qt::Orientation ContactsAlphabetRibbon::orientation() const
{
    return mInner->orientation();
}

#include "contactlistpane.moc"

