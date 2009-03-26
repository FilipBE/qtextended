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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "testdatetimeedit.h"
#include "testwidgetslog.h"

#include <QLayout>
#include <QLineEdit>
#include <QStyleOptionSpinBox>

TestDateTimeEdit::TestDateTimeEdit(QObject* _q)
    : TestGenericTextWidget(_q), q(qobject_cast<QDateTimeEdit*>(_q))
{
    TestWidgetsLog();
    connect(q,    SIGNAL(dateTimeChanged(QDateTime)),
            this, SLOT(onDateTimeChanged(QDateTime)));
}

void TestDateTimeEdit::onDateTimeChanged(QDateTime const& dt)
{ emit entered(dt); }

QString TestDateTimeEdit::text() const
{
    TestWidgetsLog();
    return q->dateTime().toString();
}

QRegExp dateFormatToRegExp(QString const& format)
{
    QString re;
    QString fmt = format;

#define REPLACE_TOK(A,B) \
    if (fmt.startsWith(A)) { \
        re += B; \
        fmt.remove(0, qstrlen(A)); \
        continue; \
    }
    while (!fmt.isEmpty()) {
        REPLACE_TOK("yyyy", "(\\d{4})");
        REPLACE_TOK("yy", "(\\d{2})");

        REPLACE_TOK("MMMM", "(\\w+)");
        REPLACE_TOK("MMM", "(\\w+)");
        REPLACE_TOK("MM", "([01]\\d)");
        REPLACE_TOK("M", "(\\d\\d?)");

        REPLACE_TOK("dddd", "(\\w+)");
        REPLACE_TOK("ddd", "(\\w+)");
        REPLACE_TOK("dd", "([0123]\\d)");
        REPLACE_TOK("d", "(\\d{1,2})");

        REPLACE_TOK("hh", "([012]\\d)");
        REPLACE_TOK("h", "(\\d{1,2})");

        REPLACE_TOK("HH", "([012]\\d)");
        REPLACE_TOK("H", "(\\d{1,2})");

        REPLACE_TOK("mm", "([0-5]\\d)");
        REPLACE_TOK("m", "(\\d{1,2})");

        REPLACE_TOK("ss", "([0-5]\\d)");
        REPLACE_TOK("s", "(\\d{1,2})");

        REPLACE_TOK("zzz", "(\\d{3})");
        REPLACE_TOK("z", "(\\d{0,3})");

        REPLACE_TOK("AP", "(AM|PM)");
        REPLACE_TOK("ap", "(am|pm)");
        REPLACE_TOK("A", "(AM|PM)");
        REPLACE_TOK("a", "(am|pm)");

        re += QString("\\x%1").arg(fmt.at(0).unicode(), 4, 16, QChar('0'));
        fmt.remove(0,1);
    }
#undef REPLACE_TOK

    TestWidgetsLog() << format << "resulted in" << re;

    return QRegExp(re);
}

bool TestDateTimeEdit::canEnter(QVariant const& item) const
{
    QString text;
    if (item.canConvert<QDateTime>()) {
        text = item.value<QDateTime>().toString( q->displayFormat() );
    } else if (item.canConvert<QDate>()) {
        text = item.value<QDate>().toString( q->displayFormat() );
    } else if (item.canConvert<QTime>()) {
        text = item.value<QTime>().toString( q->displayFormat() );
    } else {
        text = item.toString();
    }
    int dont_care = 0;
    TestWidgetsLog() << text << "format:" << q->displayFormat();
    bool ret = (QValidator::Acceptable ==
            static_cast<QAbstractSpinBox*>(q)->validate(text, dont_care));
    if (ret) {
        TestWidgetsLog() << "Can enter" << item;
    } else {
        TestWidgetsLog() << "Can't enter" << item;
    }
    return ret;
}

QPoint TestDateTimeEdit::nextClick( QStringList const& cap,
        QMap<QDateTimeEdit::Section,int> const& capMap,
        bool *final, bool *ok)
{
    if (ok) *ok = false;
    if (final) *final = false;

    QPoint ret;

    int activeSection = -1;
    int moveUpDown = 0;
    int sectionsNeedingChanges = 0;

    /* Sections, from most to least significant.
     * It is necessary to edit most significant first because that can affect
     * the allowed values from less significant fields (e.g., can't select
     * '30' in days if '02' is selected for month).
     */
    static const QDateTimeEdit::Section sections[] = {
            QDateTimeEdit::YearSection,
            QDateTimeEdit::MonthSection,
            QDateTimeEdit::DaySection,
            QDateTimeEdit::HourSection,
            QDateTimeEdit::MinuteSection,
            QDateTimeEdit::SecondSection,
            QDateTimeEdit::MSecSection,
            QDateTimeEdit::AmPmSection,
            (QDateTimeEdit::Section)0
    };

    {
    int i = 0;
    for (QDateTimeEdit::Section section = sections[i];
            section;
            section = sections[++i]) {
        if (!(q->displayedSections() & section)) continue;
        QString text = q->sectionText(section);
        int cap_i = capMap.value(section);
        TestWidgetsLog() << "i" << i << "text" << text << "cap_i" << cap_i
                << "cap" << cap.at(cap_i);
        if (text != cap.at(cap_i)) {
            ++sectionsNeedingChanges;
            if (-1 == activeSection) {
                bool is_int = false;
                int src  = text.toInt(&is_int);
                if (!is_int) return ret;
                int dest = cap.at(cap_i).toInt(&is_int);
                if (!is_int) return ret;
                moveUpDown = (dest - src);
                activeSection = cap_i;
            }
        }
    }
    }

    /* If there is only one section that requires a change,
     * AND that section is off the desired value by one,
     * AND that section already has focus,
     * then this should be the last necessary click. */
    if (final && (1 == sectionsNeedingChanges) && (1 == qAbs(moveUpDown)) && q->currentSectionIndex() == activeSection)
       *final = true;

    TestWidgetsLog() << "activeSection" << activeSection << "currentSectionIndex" << q->currentSectionIndex();

    if (-1 == activeSection) {
        /* No clicking required; already at target value */
        if (final) *final = true;
    } else if (q->currentSectionIndex() != activeSection) {
        /* Clicking required; need to move cursor to correct section */
        int begin = -1, end = -1;
        {
            QString text = q->text();
            for (int i = 0; i < activeSection; ++i) {
                QString sec = q->sectionText(q->sectionAt(i));
                text.replace(text.indexOf(sec), sec.length(), QString(sec.length(), '_'));
            }
            QString sec = q->sectionText(q->sectionAt(activeSection));
            begin = text.indexOf(sec);
            end   = begin + sec.length();
            TestWidgetsLog() << "sec" << sec << "lies from" << begin << "to" << end << "in" << text;
        }
        QPoint pos(0, q->height()/2);
        QLineEdit* le = q->findChild<QLineEdit*>();
        QPoint clickPos;
        while (le->rect().contains(pos) && clickPos.isNull()) {
            int cursor = le->cursorPositionAt(pos);
            if (cursor >= begin && cursor < end) {
                clickPos = q->mapFromGlobal(le->mapToGlobal(pos));
            }
            pos.setX(pos.x() + 2);
        }
        ret = clickPos;
        if (ret.isNull()) {
            if (ok) *ok = false;
            QtUiTest::setErrorString(QString("Could not determine where to click in "
                "date edit to move cursor between position %1 and %2").arg(begin).arg(end));
            return ret;
        }
    } else if (moveUpDown != 0) {
        /* Clicking required; need to move up */
        QStyle const* style = q->style();
        QStyleOptionSpinBox opt;
        opt.initFrom(q);
        QRect rect = style->subControlRect(QStyle::CC_SpinBox, &opt,
                (moveUpDown > 0) ? QStyle::SC_SpinBoxUp : QStyle::SC_SpinBoxDown,
                q);
        TestWidgetsLog() << "move " << ((moveUpDown > 0) ? "up" : "down") << rect;
        ret = rect.center();
    }

    if (ok) *ok = true;
    return ret;
}

bool TestDateTimeEdit::enterByMouse(QString const& format, QDateTime const& dt)
{
    bool final = false;
    bool ok = true;

    QRegExp re( dateFormatToRegExp(format) );
    if (-1 == re.indexIn( dt.toString(format) )) {
        TestWidgetsLog() << "regex didn't match, re:" << re << "date:" << dt.toString(format);
        return false;
    }

    QStringList cap = re.capturedTexts();
    cap.removeFirst();

    TestWidgetsLog() << "cap" << cap << "text" << q->text();

    /* Mapping from section to index in cap.
     * Wouldn't be necessary if QDateTimeEdit had the inverse of sectionAt().
     */
    QMap<QDateTimeEdit::Section,int> capMap;
    for (int i = 0; i < q->sectionCount(); ++i) {
        capMap.insert( q->sectionAt(i), i );
    }

    do {
        QPoint nc = nextClick(cap, capMap, &final, &ok);
        if (ok) {
            if (!ensureVisiblePoint(nc)) return false;
            QtUiTest::mouseClick(q->mapToGlobal(nc));
        }
    } while (!final && ok);

    return ok;
}

bool TestDateTimeEdit::enterByKeys(QString const& format, QDateTime const& dt, bool noCommit)
{
    if (!hasEditFocus() && !setEditFocus(true)) return false;
    TestWidgetsLog() << "got focus";

    for (int i = 0, m = q->sectionCount(); i < m; ++i) {
        if (!enterSectionByKeys(q->sectionAt(i), format, dt))
            return false;
    }

    if (noCommit) return true;

    return setEditFocus(false);
}

bool TestDateTimeEdit::enter(QVariant const& item, bool noCommit)
{
    QString text;
    QDateTime dt;
    QString format = q->displayFormat();
    TestWidgetsLog() << "Format: " << format;
    if (item.canConvert<QDateTime>()) {
        dt = item.value<QDateTime>();
        text = dt.toString( format );
        TestWidgetsLog() << "QDateTime";
    } else if (item.canConvert<QDate>()) {
        dt = QDateTime(item.value<QDate>(), QTime());
        text = dt.toString( format );
        TestWidgetsLog() << "QDate";
    } else if (item.canConvert<QTime>()) {
        dt = QDateTime(QDate(1970,1,1), item.value<QTime>());
        text = dt.toString( format );
        TestWidgetsLog() << "QTime";
    } else {
        text = item.toString();
        dt = QDateTime::fromString(text, format);
        TestWidgetsLog() << "String";
    }
    int dont_care = 0;
    if (QValidator::Acceptable !=
            static_cast<QAbstractSpinBox*>(q)->validate(text, dont_care)) {
        TestWidgetsLog() << "Can't enter" << item << "(text:" << text << ")";
        return false;
    }

    if (text == q->text()) return true;

    if (QtUiTest::mousePreferred()) {
        return enterByMouse(format, dt);
    } else {
        return enterByKeys(format, dt, noCommit);
    }

    return false;
}

bool TestDateTimeEdit::enterSectionByKeys(QDateTimeEdit::Section section,
        QString const& fmt, QDateTime const& dt)
{
    using namespace QtUiTest;

    if (fmt.isEmpty() || !dt.isValid()) return false;

#define TRY(str) if (fmt.contains(str)) { \
    text = dt.toString(str); \
}

    QString text;
    switch (section) {
        case QDateTimeEdit::YearSection:
            TRY("yyyy")
            else TRY("yy");
            break;
        case QDateTimeEdit::MonthSection:
            TRY("MMMM")
            else TRY("MMM")
            else TRY("MM")
            else TRY("M");
            break;
        case QDateTimeEdit::DaySection:
            TRY("dddd")
            else TRY("ddd")
            else TRY("dd")
            else TRY("d");
            break;
        case QDateTimeEdit::HourSection:
            TRY("hh")
            else TRY("h")
            else TRY("HH")
            else TRY("H");
            if (fmt.contains("AP") || fmt.contains("ap")) {
                // Need to use 12 hour format
                if (fmt.contains("hh")) {
                    text = dt.toString("hh ap");
                    text = text.left(text.indexOf(' '));
                }
                else if (fmt.contains("h")) {
                    text = dt.toString("h ap");
                    text = text.left(text.indexOf(' '));
                }
            }
            break;
        case QDateTimeEdit::MinuteSection:
            TRY("mm")
            else TRY("m");
            break;
        case QDateTimeEdit::SecondSection:
            TRY("ss")
            else TRY("s");
            break;
        case QDateTimeEdit::MSecSection:
            TRY("zzz")
            else TRY("z");
            break;
        case QDateTimeEdit::AmPmSection:
            TRY("AP")
            else TRY("ap");
            break;
        default:
            return false;
    }
#undef TRY

    if (text.isEmpty()) return false;

    QMap<int,int> sectionMap;
    for (int i = 0; i < q->sectionCount(); ++i) {
        sectionMap.insert(q->sectionAt(i), i);
    }

    int i = 0;
    while (q->currentSection() != section && ++i < 100) {
        Qt::KeyboardModifiers mod = (q->currentSectionIndex() < sectionMap[section]) ? Qt::KeyboardModifiers(0) : Qt::ShiftModifier;
	TestWidgetsLog() << "Entering extra key click (before " + text + ") to move to prev/next section";
        if (!keyClick(q, Qt::Key_Tab)) {
            setErrorString("Key click did not go to the expected widget.");
            return false;
        }
    }
    if (q->currentSection() != section) {
        setErrorString("Could not move focus to desired section.");
        return false;
    }

    TestWidgetsLog() << "Going to enter" << text << "in section" << section;

    foreach (QChar const& c, text) {
        if (!keyClick(q, asciiToKey(c.toLatin1()))) {
            return false;
        }
    }

    return true;
}

bool TestDateTimeEdit::canWrap(QObject *o)
{ return qobject_cast<QDateTimeEdit*>(o); }

