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

#include "navigationbar.h"

#include <qtopianamespace.h>
#include <QGridLayout>
#include <QDebug>
#include <QToolButton>
#include <QApplication>


// Provide a pair of arrow buttons which can be enabled/disabled to additionally display
// labels, when the labelsChanged(const QString &,const QString &) is
// activated.
// This class is not part of the public API. The NavigationBar is intended to display
// at the base of the HelpBrowser. The arrow buttons indicate to the user that they
// can navigate forwards and/or backwards within the help documentation hierarchy.
// The buttons also display the titles of the 'next' or 'previous' documents,
// or nothing, as appropriate.
NavigationBar::NavigationBar( QWidget* parent) : QWidget(parent) {
    init();
}

void NavigationBar::init()
{
    leftBn = 0;

    QGridLayout *layout = new QGridLayout;
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    // Set up the left button.
    leftBn = createButton(Qt::LeftArrow);
    // Set up the right button.
    rightBn = createButton(Qt::RightArrow);

    // Normal font size. Actual buttons can be shrunk to fit.
    updateNormalSize();

    //Note: We have double negation. Qt swaps the button layout around in RTL.
    //However this is a case where we don't want this behavior. Hence we have to
    //swap the layout again
    if ( QApplication::layoutDirection() == Qt::RightToLeft ) {
        layout->addWidget(rightBn,0,0);
        layout->addWidget(leftBn,0,1);
    } else {
        layout->addWidget(leftBn,0,0);
        layout->addWidget(rightBn,0,1);
    }
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,1);
}

void NavigationBar::setBack(QAction* a)
{
    leftBn->setDefaultAction(a);
}

void NavigationBar::setForward(QAction* a)
{
    rightBn->setDefaultAction(a);
}


QToolButton *NavigationBar::createButton(Qt::ArrowType arrowType)
{
    // Create an arrow button, which will also contain text.
    QToolButton *bn = new QToolButton;
    bn->setArrowType(arrowType);
    bn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Ensure that the widget does not get automatic focus.
    bn->setFocusPolicy(Qt::NoFocus);

    // Ensure that the widget expands horizontally to fill the area allocated to it.
    QSizePolicy policy = bn->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    bn->setSizePolicy(policy);

    if ( !Qtopia::mousePreferred() ) {
        // Just for context not pressing,
        // so reduce the default height of the widget.
        int height = bn->fontMetrics().size(0,"Qg").height();
        bn->setMaximumHeight(height);
        bn->setMinimumHeight(height);
    }

    //bn->setEnabled(false); // initial state

    return bn;
}

void NavigationBar::updateNormalSize()
{
    if (leftBn)
        normalSize = leftBn->font().pointSize();
    // XXX don't have strings, so can't update labels properly
}

// Handler for the event that the forwards and backwards labels need updating.
void NavigationBar::labelsChanged(const QString &previous,const QString &next)
{
    // We need to draw the strings 'previous' and 'next' in leftLabel and rightLabel,
    // respectively. We've only got a certain amount of room, and if we have to shrink
    // one of the pieces of text, we need to make the best use of the screen real estate.
    // That is, if one string is very short, it should not take up a whole half of the
    // available area at the expense of the other string being elided.
    // We can also reduce the font size a little if necessary to avoid elision.

    QString nextText;
    QString previousText;
    int leftWidth;
    int rightWidth;

    const double scale[] = {1,0.75,0.5,-1};
    for (int resize=0; scale[resize]>=0; ++resize) {
        QFont f = leftBn->font();
        // Okay to be smaller than user's standard, since not essential text to be readable.
        f.setPointSizeF(normalSize*scale[resize]);
        leftBn->setFont(f);
        rightBn->setFont(f);

        // Find out how much space we've got to draw both sets of text. (!!!!Unfortunately,
        // have had to put a magic number in here, since wasn't able to find out how much
        // space you get to draw in on a button, despite heroic efforts...)
        int labelSpace = leftBn->contentsRect().width() + rightBn->contentsRect().width()
                         - leftBn->iconSize().width() - rightBn->iconSize().width() - 25;

        //swap text when RTL
        if ( QApplication::layoutDirection() == Qt::LeftToRight ) {
            nextText = next;
            previousText = previous;
        } else {
            nextText = previous;
            previousText = next;
        }

        // Find out how much room the given strings will take to draw.
        leftWidth = leftBn->fontMetrics().boundingRect(previousText).width()+2;
        rightWidth = rightBn->fontMetrics().boundingRect(nextText).width()+2;

        // Find out if we've got too much text and not enough room to draw it in.
        int excess = leftWidth + rightWidth - labelSpace;

        if ( excess > 0 ) {
            if ( scale[resize]==0 ) {
                // No further font shrinking - must reduce width on one or both labels.
                int diff = qAbs(leftWidth-rightWidth); // difference between width required for each string
                if ( diff != 0 ) {
                    // Labels are not equal size.
                    if ( diff > excess ) {
                        // Only need to reduce the largest label.
                        if ( leftWidth > rightWidth ) {
                            leftWidth -= excess;
                        } else {
                            rightWidth -= excess;
                        }
                    } else {
                        // Make the two labels the same width by taking diff from the widest.
                        leftWidth = qMin(leftWidth,rightWidth);
                        rightWidth = leftWidth;
                        // Figure out how much we've got left to get rid of.
                        excess -= diff;
                        // Treat them evenly, now.
                        leftWidth -= (excess/2);
                        rightWidth -= (excess/2);
                    }
                } else {
                    // No difference - labels are equal size.
                    leftWidth -= (excess/2);
                    rightWidth -= (excess/2);
                }
            }
        } else {
            // Share it anyway
            leftWidth -= (excess/2);
            rightWidth -= (excess/2);
            break;
        }
    }

    // Shrink and elide the strings, if necessary.
    QString newPrevious = leftBn->fontMetrics().elidedText(previousText,Qt::ElideRight,leftWidth);
    QString newNext = rightBn->fontMetrics().elidedText(nextText,Qt::ElideRight,rightWidth);

    // Set the labels.
    leftBn->setText(newPrevious);
    rightBn->setText(newNext);

    // Force
    //leftBn->setEnabled(true);
    //rightBn->setEnabled(true);
}
