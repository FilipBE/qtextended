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

#include "dayview.h"

#include <qtimestring.h>
#include <qappointment.h>
#include <qappointmentmodel.h>
#include <qappointmentview.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QScrollArea>
#include <QItemSelectionModel>
#include <QTimer>
#include <QPushButton>

namespace DuplicatedFromCalendarWidget
{
    class QCalendarDateSectionValidator
    {
        public:

            enum Section {
                NextSection,
                ThisSection,
                PrevSection
            };

            QCalendarDateSectionValidator() {}
            virtual ~QCalendarDateSectionValidator() {}
            virtual Section handleKey(int key) = 0;
            virtual QDate applyToDate(const QDate &date) const = 0;
            virtual void setDate(const QDate &date) = 0;
            virtual QString text() const = 0;
            virtual QString text(const QDate &date, int repeat) const = 0;

            QLocale m_locale;

        protected:
            QString highlightString(const QString &str, int pos) const;
        private:
    };

    QString QCalendarDateSectionValidator::highlightString(const QString &str, int pos) const
    {
        if (pos == 0)
            return QLatin1String("<b>") + str + QLatin1String("</b>");
        int startPos = str.length() - pos;
        return str.mid(0, startPos) + QLatin1String("<b>") + str.mid(startPos, pos) + QLatin1String("</b>");

    }

    class QCalendarDayValidator : public QCalendarDateSectionValidator
    {

        public:
            QCalendarDayValidator();
            virtual Section handleKey(int key);
            virtual QDate applyToDate(const QDate &date) const;
            virtual void setDate(const QDate &date);
            virtual QString text() const;
            virtual QString text(const QDate &date, int repeat) const;
        private:
            int m_pos;
            int m_day;
            int m_oldDay;
    };

    QCalendarDayValidator::QCalendarDayValidator()
        : QCalendarDateSectionValidator(), m_pos(0), m_day(1), m_oldDay(1)
    {
    }

    QCalendarDateSectionValidator::Section QCalendarDayValidator::handleKey(int key)
    {
        if (key == Qt::Key_Right || key == Qt::Key_Left) {
            m_pos = 0;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Up) {
            m_pos = 0;
            ++m_day;
            if (m_day > 31)
                m_day = 1;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Down) {
            m_pos = 0;
            --m_day;
            if (m_day < 1)
                m_day = 31;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Back || key == Qt::Key_Backspace) {
            --m_pos;
            if (m_pos < 0)
                m_pos = 1;

            if (m_pos == 0)
                m_day = m_oldDay;
            else
                m_day = m_day / 10;
            //m_day = m_oldDay / 10 * 10 + m_day / 10;

            if (m_pos == 0)
                return QCalendarDateSectionValidator::PrevSection;
            return QCalendarDateSectionValidator::ThisSection;
        }
        if (key < Qt::Key_0 || key > Qt::Key_9)
            return QCalendarDateSectionValidator::ThisSection;
        int pressedKey = key - Qt::Key_0;
        if (m_pos == 0)
            m_day = pressedKey;
        else
            m_day = m_day % 10 * 10 + pressedKey;
        if (m_day > 31)
            m_day = 31;
        ++m_pos;
        if (m_pos > 1) {
            m_pos = 0;
            return QCalendarDateSectionValidator::NextSection;
        }
        return QCalendarDateSectionValidator::ThisSection;
    }

    QDate QCalendarDayValidator::applyToDate(const QDate &date) const
    {
        int day = m_day;
        if (day < 1)
            day = 1;
        else if (day > 31)
            day = 31;
        if (day > date.daysInMonth())
            day = date.daysInMonth();
        return QDate(date.year(), date.month(), day);
    }

    void QCalendarDayValidator::setDate(const QDate &date)
    {
        m_day = m_oldDay = date.day();
        m_pos = 0;
    }

    QString QCalendarDayValidator::text() const
    {
        QString str;
        if (m_day / 10 == 0)
            str += QLatin1String("0");
        str += QString::number(m_day);
        return highlightString(str, m_pos);
    }

    QString QCalendarDayValidator::text(const QDate &date, int repeat) const
    {
        if (repeat <= 1) {
            return QString::number(date.day());
        } else if (repeat == 2) {
            QString str;
            if (date.day() / 10 == 0)
                str += QLatin1String("0");
            return str + QString::number(date.day());
        } else if (repeat == 3) {
            return m_locale.dayName(date.dayOfWeek(), QLocale::ShortFormat);
        } else if (repeat >= 4) {
            return m_locale.dayName(date.dayOfWeek(), QLocale::LongFormat);
        }
        return QString();
    }

    //////////////////////////////////

    class QCalendarMonthValidator : public QCalendarDateSectionValidator
    {

        public:
            QCalendarMonthValidator();
            virtual Section handleKey(int key);
            virtual QDate applyToDate(const QDate &date) const;
            virtual void setDate(const QDate &date);
            virtual QString text() const;
            virtual QString text(const QDate &date, int repeat) const;
        private:
            int m_pos;
            int m_month;
            int m_oldMonth;
    };

    QCalendarMonthValidator::QCalendarMonthValidator()
        : QCalendarDateSectionValidator(), m_pos(0), m_month(1), m_oldMonth(1)
    {
    }

    QCalendarDateSectionValidator::Section QCalendarMonthValidator::handleKey(int key)
    {
        if (key == Qt::Key_Right || key == Qt::Key_Left) {
            m_pos = 0;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Up) {
            m_pos = 0;
            ++m_month;
            if (m_month > 12)
                m_month = 1;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Down) {
            m_pos = 0;
            --m_month;
            if (m_month < 1)
                m_month = 12;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Back || key == Qt::Key_Backspace) {
            --m_pos;
            if (m_pos < 0)
                m_pos = 1;

            if (m_pos == 0)
                m_month = m_oldMonth;
            else
                m_month = m_month / 10;
            //m_month = m_oldMonth / 10 * 10 + m_month / 10;

            if (m_pos == 0)
                return QCalendarDateSectionValidator::PrevSection;
            return QCalendarDateSectionValidator::ThisSection;
        }
        if (key < Qt::Key_0 || key > Qt::Key_9)
            return QCalendarDateSectionValidator::ThisSection;
        int pressedKey = key - Qt::Key_0;
        if (m_pos == 0)
            m_month = pressedKey;
        else
            m_month = m_month % 10 * 10 + pressedKey;
        if (m_month > 12)
            m_month = 12;
        ++m_pos;
        if (m_pos > 1) {
            m_pos = 0;
            return QCalendarDateSectionValidator::NextSection;
        }
        return QCalendarDateSectionValidator::ThisSection;
    }

    QDate QCalendarMonthValidator::applyToDate(const QDate &date) const
    {
        int month = m_month;
        if (month < 1)
            month = 1;
        else if (month > 12)
            month = 12;
        QDate newDate(date.year(), m_month, 1);
        int day = date.day();
        if (day > newDate.daysInMonth())
            day = newDate.daysInMonth();
        return QDate(date.year(), month, day);
    }

    void QCalendarMonthValidator::setDate(const QDate &date)
    {
        m_month = m_oldMonth = date.month();
        m_pos = 0;
    }

    QString QCalendarMonthValidator::text() const
    {
        QString str;
        if (m_month / 10 == 0)
            str += QLatin1String("0");
        str += QString::number(m_month);
        return highlightString(str, m_pos);
    }

    QString QCalendarMonthValidator::text(const QDate &date, int repeat) const
    {
        if (repeat <= 1) {
            return QString::number(date.month());
        } else if (repeat == 2) {
            QString str;
            if (date.month() / 10 == 0)
                str += QLatin1String("0");
            return str + QString::number(date.month());
        } else if (repeat == 3) {
            return m_locale.monthName(date.month(), QLocale::ShortFormat);
        } else if (repeat >= 4) {
            return m_locale.monthName(date.month(), QLocale::LongFormat);
        }
        return QString();
    }

    //////////////////////////////////

    class QCalendarYearValidator : public QCalendarDateSectionValidator
    {

        public:
            QCalendarYearValidator();
            virtual Section handleKey(int key);
            virtual QDate applyToDate(const QDate &date) const;
            virtual void setDate(const QDate &date);
            virtual QString text() const;
            virtual QString text(const QDate &date, int repeat) const;
        private:
            int pow10(int n);
            int m_pos;
            int m_year;
            int m_oldYear;
    };

    QCalendarYearValidator::QCalendarYearValidator()
        : QCalendarDateSectionValidator(), m_pos(0), m_year(2000), m_oldYear(2000)
    {
    }

    int QCalendarYearValidator::pow10(int n)
    {
        int power = 1;
        for (int i = 0; i < n; i++)
            power *= 10;
        return power;
    }

    QCalendarDateSectionValidator::Section QCalendarYearValidator::handleKey(int key)
    {
        if (key == Qt::Key_Right || key == Qt::Key_Left) {
            m_pos = 0;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Up) {
            m_pos = 0;
            ++m_year;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Down) {
            m_pos = 0;
            --m_year;
            return QCalendarDateSectionValidator::ThisSection;
        } else if (key == Qt::Key_Back || key == Qt::Key_Backspace) {
            --m_pos;
            if (m_pos < 0)
                m_pos = 3;

            int pow = pow10(m_pos);
            m_year = m_oldYear / pow * pow + m_year % (pow * 10) / 10;

            if (m_pos == 0)
                return QCalendarDateSectionValidator::PrevSection;
            return QCalendarDateSectionValidator::ThisSection;
        }
        if (key < Qt::Key_0 || key > Qt::Key_9)
            return QCalendarDateSectionValidator::ThisSection;
        int pressedKey = key - Qt::Key_0;
        int pow = pow10(m_pos);
        m_year = m_year / (pow * 10) * (pow * 10) + m_year % pow * 10 + pressedKey;
        ++m_pos;
        if (m_pos > 3) {
            m_pos = 0;
            return QCalendarDateSectionValidator::NextSection;
        }
        return QCalendarDateSectionValidator::ThisSection;
    }

    QDate QCalendarYearValidator::applyToDate(const QDate &date) const
    {
        int year = m_year;
        if (year < 1)
            year = 1;
        QDate newDate(year, date.month(), 1);
        int day = date.day();
        if (day > newDate.daysInMonth())
            day = newDate.daysInMonth();
        return QDate(year, date.month(), day);
    }

    void QCalendarYearValidator::setDate(const QDate &date)
    {
        m_year = m_oldYear = date.year();
        m_pos = 0;
    }

    QString QCalendarYearValidator::text() const
    {
        QString str;
        int pow = 10;
        for (int i = 0; i < 3; i++) {
            if (m_year / pow == 0)
                str += QLatin1String("0");
            pow *= 10;
        }
        str += QString::number(m_year);
        return highlightString(str, m_pos);
    }

    QString QCalendarYearValidator::text(const QDate &date, int repeat) const
    {
        if (repeat < 4) {
            QString str;
            int year = date.year() % 100;
            if (year / 10 == 0)
                str = QLatin1String("0");
            return str + QString::number(year);
        }
        return QString::number(date.year());
    }

    ///////////////////////////////////

    class QCalendarDateValidator
    {
        public:
            QCalendarDateValidator();
            ~QCalendarDateValidator();

            void handleKeyEvent(QKeyEvent *keyEvent);
            QString currentText() const;
            QDate currentDate() const { return m_currentDate; }
            void setFormat(const QString &format);
            void setInitialDate(const QDate &date);

            void setLocale(const QLocale &locale);

        private:

            struct SectionToken {
                SectionToken(QCalendarDateSectionValidator *val, int rep) : validator(val), repeat(rep) {}
                QCalendarDateSectionValidator *validator;
                int repeat;
            };

            void toNextToken();
            void toPreviousToken();
            void applyToDate();

            int countRepeat(const QString &str, int index) const;
            void clear();

            QStringList m_separators;
            QList<SectionToken *> m_tokens;
            QCalendarDateSectionValidator *m_yearValidator;
            QCalendarDateSectionValidator *m_monthValidator;
            QCalendarDateSectionValidator *m_dayValidator;

            SectionToken *m_currentToken;

            QDate m_initialDate;
            QDate m_currentDate;

            QCalendarDateSectionValidator::Section m_lastSectionMove;
    };

    QCalendarDateValidator::QCalendarDateValidator()
        : m_currentToken(0), m_lastSectionMove(QCalendarDateSectionValidator::ThisSection)
    {
        m_initialDate = m_currentDate = QDate::currentDate();
        m_yearValidator = new QCalendarYearValidator();
        m_monthValidator = new QCalendarMonthValidator();
        m_dayValidator = new QCalendarDayValidator();
    }

    void QCalendarDateValidator::setLocale(const QLocale &locale)
    {
        m_yearValidator->m_locale = locale;
        m_monthValidator->m_locale = locale;
        m_dayValidator->m_locale = locale;
    }

    QCalendarDateValidator::~QCalendarDateValidator()
    {
        delete m_yearValidator;
        delete m_monthValidator;
        delete m_dayValidator;
        clear();
    }

    // from qdatetime.cpp
    int QCalendarDateValidator::countRepeat(const QString &str, int index) const
    {
        Q_ASSERT(index >= 0 && index < str.size());
        int count = 1;
        const QChar ch = str.at(index);
        while (index + count < str.size() && str.at(index + count) == ch)
            ++count;
        return count;
    }

    void QCalendarDateValidator::setInitialDate(const QDate &date)
    {
        m_yearValidator->setDate(date);
        m_monthValidator->setDate(date);
        m_dayValidator->setDate(date);
        m_initialDate = date;
        m_currentDate = date;
        m_lastSectionMove = QCalendarDateSectionValidator::ThisSection;
    }

    QString QCalendarDateValidator::currentText() const
    {
        QString str;
        QStringListIterator itSep(m_separators);
        QListIterator<SectionToken *> itTok(m_tokens);
        while (itSep.hasNext()) {
            str += itSep.next();
            if (itTok.hasNext()) {
                SectionToken *token = itTok.next();
                QCalendarDateSectionValidator *validator = token->validator;
                if (m_currentToken == token)
                    str += validator->text();
                else
                    str += validator->text(m_currentDate, token->repeat);
            }
        }
        return str;
    }

    void QCalendarDateValidator::clear()
    {
        QListIterator<SectionToken *> it(m_tokens);
        while (it.hasNext())
            delete it.next();

        m_tokens.clear();
        m_separators.clear();

        m_currentToken = 0;
    }

    void QCalendarDateValidator::setFormat(const QString &format)
    {
        clear();

        int pos = 0;
        const QLatin1String quote("'");
        bool quoting = false;
        QString separator;
        while (pos < format.size()) {
            QString mid = format.mid(pos);
            int offset = 1;

            if (mid.startsWith(quote)) {
                quoting = !quoting;
            } else {
                const QChar nextChar = format.at(pos);
                if (quoting) {
                    separator += nextChar;
                } else {
                    SectionToken *token = 0;
                    if (nextChar == QLatin1Char('d')) {
                        offset = qMin(4, countRepeat(format, pos));
                        token = new SectionToken(m_dayValidator, offset);
                    } else if (nextChar == QLatin1Char('M')) {
                        offset = qMin(4, countRepeat(format, pos));
                        token = new SectionToken(m_monthValidator, offset);
                    } else if (nextChar == QLatin1Char('y')) {
                        offset = qMin(4, countRepeat(format, pos));
                        token = new SectionToken(m_yearValidator, offset);
                    } else {
                        separator += nextChar;
                    }
                    if (token) {
                        m_tokens.append(token);
                        m_separators.append(separator);
                        separator = QString();
                        if (!m_currentToken)
                            m_currentToken = token;

                    }
                }
            }
            pos += offset;
        }
        m_separators += separator;
    }

    void QCalendarDateValidator::applyToDate()
    {
        m_currentDate = m_yearValidator->applyToDate(m_currentDate);
        m_currentDate = m_monthValidator->applyToDate(m_currentDate);
        m_currentDate = m_dayValidator->applyToDate(m_currentDate);
    }

    void QCalendarDateValidator::toNextToken()
    {
        const int idx = m_tokens.indexOf(m_currentToken);
        if (idx == -1)
            return;
        if (idx + 1 >= m_tokens.count())
            m_currentToken = m_tokens.first();
        else
            m_currentToken = m_tokens.at(idx + 1);
    }

    void QCalendarDateValidator::toPreviousToken()
    {
        const int idx = m_tokens.indexOf(m_currentToken);
        if (idx == -1)
            return;
        if (idx - 1 < 0)
            m_currentToken = m_tokens.last();
        else
            m_currentToken = m_tokens.at(idx - 1);
    }

    void QCalendarDateValidator::handleKeyEvent(QKeyEvent *keyEvent)
    {
        if (!m_currentToken)
            return;

        int key = keyEvent->key();
        if (m_lastSectionMove == QCalendarDateSectionValidator::NextSection) {
            if (key == Qt::Key_Back || key == Qt::Key_Backspace)
                toPreviousToken();
        }
        if (key == Qt::Key_Right)
            toNextToken();
        else if (key == Qt::Key_Left)
            toPreviousToken();

        m_lastSectionMove = m_currentToken->validator->handleKey(key);

        applyToDate();
        if (m_lastSectionMove == QCalendarDateSectionValidator::NextSection)
            toNextToken();
        else if (m_lastSectionMove == QCalendarDateSectionValidator::PrevSection)
            toPreviousToken();
    }

    class QCalendarTextNavigator: public QObject
    {
        Q_OBJECT
        public:
            QCalendarTextNavigator(QObject *parent = 0)
                : QObject(parent), m_dateText(0), m_dateFrame(0), m_dateValidator(0), m_calendar(0), m_editDelay(1500) { }

            DayView *calendar() const;
            void setCalendar(DayView *calendar);
            void applyDate();
            void updateDateLabel();
            void createDateLabel();
            void removeDateLabel();
            int dateEditAcceptDelay() const;
            void setDateEditAcceptDelay(int delay);

            bool eventFilter(QObject *o, QEvent *e);
            void timerEvent(QTimerEvent *e);

            void setLocale(const QLocale &locale);

        signals:
            void changeDate(const QDate &date, bool changeMonth);
            void editingFinished();

        private:
            QLabel *m_dateText;
            QFrame *m_dateFrame;
            QBasicTimer m_acceptTimer;
            QCalendarDateValidator *m_dateValidator;
            DayView *m_calendar;
            int m_editDelay;

            QLocale m_locale;
    };

    DayView *QCalendarTextNavigator::calendar() const
    {
        return m_calendar;
    }

    void QCalendarTextNavigator::setCalendar(DayView *calendar)
    {
        m_calendar = calendar;
    }

    void QCalendarTextNavigator::updateDateLabel()
    {
        if (!m_calendar)
            return;

        m_acceptTimer.start(m_editDelay, this);

        m_dateText->setText(m_dateValidator->currentText());

        QSize s = m_dateFrame->sizeHint();
        QRect r = m_calendar->geometry(); // later, just the table section
        QRect newRect((r.width() - s.width()) / 2, (r.height() - s.height()) / 2, s.width(), s.height());
        m_dateFrame->setGeometry(newRect);
        // need to set palette after geometry update as phonestyle sets transparency
        // effect in move event.
        QPalette p = m_dateFrame->palette();
        p.setBrush(QPalette::Window, m_dateFrame->window()->palette().brush(QPalette::Window));
        m_dateFrame->setPalette(p);

        m_dateFrame->raise();
        m_dateFrame->show();
    }

    void QCalendarTextNavigator::applyDate()
    {
        QDate date = m_dateValidator->currentDate();
        emit changeDate(date, true);
    }

    void QCalendarTextNavigator::setLocale(const QLocale &locale)
    {
        m_locale = locale;
        if (m_dateValidator != 0) {
            m_dateValidator->setLocale(locale);
            updateDateLabel();
        }
    }

    void QCalendarTextNavigator::createDateLabel()
    {
        if (m_dateFrame)
            return;
        m_dateFrame = new QFrame(m_calendar);
        QVBoxLayout *vl = new QVBoxLayout;
        m_dateText = new QLabel;
        vl->addWidget(m_dateText);
        m_dateFrame->setLayout(vl);
        m_dateFrame->setFrameShadow(QFrame::Plain);
        m_dateFrame->setFrameShape(QFrame::Box);
        m_dateValidator = new QCalendarDateValidator();
        m_dateValidator->setLocale(m_locale);
        m_dateValidator->setFormat(m_locale.dateFormat(QLocale::ShortFormat));
        m_dateValidator->setInitialDate(m_calendar->currentDate());

        m_dateFrame->setAutoFillBackground(true);
        m_dateFrame->setBackgroundRole(QPalette::Window);
    }

    void QCalendarTextNavigator::removeDateLabel()
    {
        if (!m_dateFrame)
            return;
        m_acceptTimer.stop();
        m_dateFrame->hide();
        m_dateFrame->deleteLater();
        delete m_dateValidator;
        m_dateFrame = 0;
        m_dateText = 0;
        m_dateValidator = 0;
    }

    bool QCalendarTextNavigator::eventFilter(QObject *o, QEvent *e)
    {
        if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if (ke->text().length() > 0 && ke->text()[0].isPrint() || m_dateFrame) {
                if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Select) {
                    applyDate();
                    emit editingFinished();
                    removeDateLabel();
                } else if (ke->key() == Qt::Key_Escape) {
                    removeDateLabel();
                } else if (e->type() == QEvent::KeyPress) {
                    createDateLabel();
                    m_dateValidator->handleKeyEvent(ke);
                    updateDateLabel();
                }
                ke->accept();
                return true;
            }
        }
        return QObject::eventFilter(o,e);
    }

    void QCalendarTextNavigator::timerEvent(QTimerEvent *e)
    {
        if (e->timerId() == m_acceptTimer.timerId()) {
            applyDate();
            removeDateLabel();
        }
    }

    int QCalendarTextNavigator::dateEditAcceptDelay() const
    {
        return m_editDelay;
    }

    void QCalendarTextNavigator::setDateEditAcceptDelay(int delay)
    {
        m_editDelay = delay;
    }
};


DayView::DayView(QWidget *parent, const QCategoryFilter& c, QSet<QPimSource> set)
    : QWidget(parent), timedModel(0), allDayModel(0), targetDate(QDate::currentDate()), cacheDelayTimer(0), nav(0)
{
    mWeekdayLabel = new QLabel();
    mDateLabel = new QLabel();
    mHiddenIndicator = new QLabel();
    mWeekdayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mDateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    bool rtl = qApp->layoutDirection() == Qt::RightToLeft;

    // Add some buttons for next/previous day
    if (Qtopia::mousePreferred()) {
        mNextDay = new QPushButton(QIcon(rtl ? ":icon/left" : ":icon/right"), QString());
        mPrevDay = new QPushButton(QIcon(rtl ? ":icon/right" : ":icon/left"), QString());

        QSize btnSize = mNextDay->iconSize();
        int margin =  2 * (2 + style()->pixelMetric(QStyle::PM_ButtonMargin));
        btnSize += QSize(margin, margin);

        mNextDay->setFixedSize(btnSize);
        mPrevDay->setFixedSize(btnSize);
        mNextDay->setFocusPolicy(Qt::NoFocus);
        mPrevDay->setFocusPolicy(Qt::NoFocus);

        connect(mNextDay, SIGNAL(clicked()), this, SLOT(nextDay()));
        connect(mPrevDay, SIGNAL(clicked()), this, SLOT(prevDay()));
    } else {
        mNextDay = 0;
        mPrevDay = 0;
    }

    QHBoxLayout *ll = new QHBoxLayout();
    ll->setMargin(4);
    ll->setSpacing(4);
    if (mNextDay && mPrevDay) {
        ll->addWidget(mPrevDay);
        ll->addWidget(mWeekdayLabel);
        ll->addWidget(mDateLabel);
        ll->addWidget(mNextDay);
    } else {
        ll->addWidget(mWeekdayLabel);
        ll->addWidget(mDateLabel);
    }

    mTimeManager = new CompressedTimeManager();
    mTimedView = new TimedView();
    QAppointmentDelegate *id = new QAppointmentDelegate(this);

    QDateTime start(mTimedView->date(), QTime(0, 0));
    QDateTime end(mTimedView->date().addDays(1), QTime(0, 0));

    timedModel = new QOccurrenceModel(start, end, this);
    timedModel->setDurationType(QAppointmentModel::TimedDuration);
    if (set.count() > 0)
        timedModel->setVisibleSources(set);
    timedModel->setCategoryFilter(c);
    mTimedView->setModel(timedModel);
    mTimedView->setItemDelegate(id);

    allDayModel = new QOccurrenceModel(start, end, this);
    allDayModel->setDurationType(QAppointmentModel::AllDayDuration);
    if (set.count() > 0)
        allDayModel->setVisibleSources(set);
    allDayModel->setCategoryFilter(c);
    mAllDayList = new AppointmentList();
    mAllDayList->setModel(allDayModel);
    mAllDayList->setItemDelegate(id);
    mAllDayList->setFolded(true);

    connect(mTimedView, SIGNAL(selectionChanged(QModelIndex)), this, SLOT(timedSelectionChanged(QModelIndex)));
    connect(timedModel, SIGNAL(modelReset()), this, SLOT(modelsReset()));
    connect(mAllDayList, SIGNAL(changeHiddenCount(int)), this, SLOT(updateHiddenIndicator(int)));
    connect(mAllDayList, SIGNAL(activated(QModelIndex)), this, SLOT(allDayOccurrenceActivated(QModelIndex)));
    connect(mAllDayList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(allDayOccurrenceChanged(QModelIndex)));
    connect(allDayModel, SIGNAL(modelReset()), this, SLOT(modelsReset()));

    mScrollArea = new QScrollArea();
    mScrollArea->setFocusPolicy(Qt::NoFocus);
    mScrollArea->setWidgetResizable(true);
    mScrollArea->viewport()->installEventFilter(this);
    mScrollArea->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *ml = new QVBoxLayout();
    ml->addLayout(ll);
    ml->addWidget(mAllDayList);
    ml->addWidget(mHiddenIndicator);
    ml->addWidget(mScrollArea);
    ml->setMargin(0);
    ml->setSpacing(0);
    setLayout(ml);

    QWidget *contents = new QWidget();
    QHBoxLayout *hl = new QHBoxLayout(contents);


    hl->addWidget(mTimeManager);
    hl->addWidget(mTimedView);
    hl->setMargin(0);
    hl->setSpacing(0);

    contents->setLayout(hl);

    mScrollArea->setWidget(contents);
    updateHeaderText();

    mTimedView->setTimeManager(mTimeManager);

    setFocusPolicy(Qt::StrongFocus);

    firstTimed();

    cacheDelayTimer = new QTimer(this);
    cacheDelayTimer->setSingleShot(true);
    cacheDelayTimer->setInterval(1);
    connect(cacheDelayTimer, SIGNAL(timeout()), this, SLOT(updateView()));

    DuplicatedFromCalendarWidget::QCalendarTextNavigator
        *nav = new DuplicatedFromCalendarWidget::QCalendarTextNavigator(this);
    nav->setLocale(locale());
    nav->setCalendar(this);
    connect(nav, SIGNAL(changeDate(QDate,bool)),
            this, SLOT(selectDate(QDate)));
    //connect(nav, SIGNAL(editingFinished()),
            //this, SLOT(...()));
    installEventFilter(nav);
}

bool DayView::event(QEvent *event)
{
    if(event->type() == QEvent::LocaleChange)
        nav->setLocale(locale());
    return QWidget::event(event);
}

void DayView::prevDay()
{
    selectDate(currentDate().addDays(-1));
}

void DayView::nextDay()
{
    selectDate(currentDate().addDays(1));
}

void DayView::keyPressEvent(QKeyEvent *e)
{

    bool rtl = qApp->layoutDirection() == Qt::RightToLeft;
    QDate cDate = currentDate();
    switch(e->key()) {
        case Qt::Key_Up:
            previousOccurrence();
            break;
        case Qt::Key_Down:
            nextOccurrence();
            break;
        case Qt::Key_Left:
            if (rtl)
                nextDay();
            else
                prevDay();
            break;
        case Qt::Key_Right:
            if (rtl)
                prevDay();
            else
                nextDay();
            break;
        case Qt::Key_1:
            selectDate(cDate.addDays(rtl ? 7 : -7));
            break;
        case Qt::Key_3:
            selectDate(cDate.addDays(rtl ? -7 : 7));
            break;
        case Qt::Key_4:
            selectDate(cDate.addMonths(rtl ? 1 : -1));
            break;
        case Qt::Key_6:
            selectDate(cDate.addMonths(rtl ? -1 : 1));
            break;
        case Qt::Key_7:
            selectDate(cDate.addYears(rtl ? 1 : -1));
            break;
        case Qt::Key_9:
            selectDate(cDate.addYears(rtl ? -1 : 1));
            break;
        case Qt::Key_5:
            selectDate(QDate::currentDate());
            break;
        case Qt::Key_Select:
            if ( currentIndex().isValid() )
                emit showDetails();
            break;
        case Qt::Key_Back:
            emit closeView();
            e->accept();
            break;
        default:
            e->ignore();
    }
}

void DayView::mouseReleaseEvent( QMouseEvent * event )
{
    QModelIndex mi = mTimedView->index ( event->globalPos() );
    if ( mi.isValid() ) {
        mTimedView->setCurrentIndex( mi );
        mAllDayList->selectionModel()->clear();
        emit showDetails();
    } else {
        // create an event at the hour represented by the click
        // if it is inside the timedview
        QDateTime startTime = mTimedView->timeAtPoint( event->globalPos(), -1 );
        if ( ! startTime.isNull() ) {
            QDateTime endTime = mTimedView->timeAtPoint( event->globalPos(), 1 );
            if ( ! endTime.isNull() )
                emit newAppointment(startTime, endTime);
        }
        // XXX or expand the appointments if
        // the "%d more all day appointments" label is clicked?
    }
}

void DayView::setVisibleSources(QSet<QPimSource> set)
{
    allDayModel->setVisibleSources(set);
    timedModel->setVisibleSources(set);
}

QModelIndex DayView::currentIndex() const
{
    QModelIndex index = mTimedView->currentIndex();
    if (index.isValid())
        return index;

    return mAllDayList->currentIndex();
}

void DayView::categorySelected( const QCategoryFilter &c )
{
    timedModel->setCategoryFilter( c );
    allDayModel->setCategoryFilter( c );
}

QOccurrenceModel *DayView::currentModel() const
{
    QModelIndex index = mTimedView->currentIndex();
    if (index.isValid())
        return timedModel;
    else
        return allDayModel;
}

QAppointment DayView::currentAppointment() const
{
    return currentModel()->appointment(currentIndex());
}

QOccurrence DayView::currentOccurrence() const
{
    QModelIndex index = currentIndex();
    if (index.isValid())
        return currentModel()->occurrence(currentIndex());
    return QOccurrence();
}

void DayView::selectDate( const QDate &date )
{
    if (!date.isValid() || date == currentDate())
        return;

    targetDate = date;
    cacheDelayTimer->start();
}

void DayView::updateView()
{

    QDateTime start(targetDate, QTime(0,0));
    QDateTime end(targetDate.addDays(1), QTime(0,0));

    mTimedView->setDate(targetDate);

    allDayModel->setRange(start, end);

    firstTimed();

    updateHeaderText();

    emit dateChanged();
}

void DayView::selectDate( int y, int m)
{
    QDate cDate = currentDate();
    if ( y != cDate.year() || m != cDate.month() ) {
        QDate nd( y, m, 1 );
        if ( nd.daysInMonth() < cDate.day() )
            selectDate(QDate(y, m, nd.daysInMonth()));
        else
            selectDate(QDate(y, m, cDate.day()));
    }
}

QDate DayView::currentDate() const
{
    return targetDate;
}

bool DayView::allDayFolded() const
{
    return mAllDayList->isFolded();
}

bool DayView::allDayFoldingAvailable() const
{
    return mAllDayList->provideFoldingOption();
}

void DayView::setAllDayFolded(bool f)
{
    mAllDayList->setFolded(f);
}

void DayView::updateHeaderText()
{
    QDate cDate = currentDate();

    if (cDate == QDate::currentDate())
        mWeekdayLabel->setText(tr("Today (%1)", "Today (Fri)").arg(QTimeString::localDayOfWeek(cDate, QTimeString::Medium)));
    else
        mWeekdayLabel->setText(QTimeString::localDayOfWeek(cDate, QTimeString::Long));

    mDateLabel->setText(QTimeString::localYMD(cDate));
}

void DayView::setDaySpan( int starthour, int endhour )
{
    mTimedView->setDaySpan(starthour*60, endhour*60);
}

void DayView::firstTimed()
{
    if (timedModel->rowCount()) {
        mAllDayList->selectionModel()->clear();
        mTimedView->setCurrentIndex(timedModel->index(0, 0));
    } else if (allDayModel->rowCount())
        mAllDayList->setCurrentIndex(allDayModel->index(0, 0));
}

void DayView::lastTimed()
{
    if (timedModel->rowCount()) {
        mAllDayList->selectionModel()->clear();
        mTimedView->setCurrentIndex(timedModel->index(timedModel->rowCount() - 1, 0));
    } else if (allDayModel->rowCount()){
        mAllDayList->setCurrentIndex(allDayModel->index(allDayModel->rowCount() - 1, 0));
    }
}

void DayView::firstAllDay()
{
    if (allDayModel->rowCount()) {
        mAllDayList->setCurrentIndex(allDayModel->index(0, 0));
        mTimedView->setCurrentIndex(QModelIndex());
    } else if ( timedModel->rowCount() > 0 ) {
        mTimedView->setCurrentIndex(timedModel->index(0,0));
    }
}

void DayView::lastAllDay()
{
    if (allDayModel->rowCount()) {
        mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->visibleRowCount() - 1, 0));
        mTimedView->setCurrentIndex(QModelIndex());
    } else if ( timedModel->rowCount() > 0 ) {
        // Wrap around
        mTimedView->setCurrentIndex(timedModel->index(timedModel->rowCount()-1,0));
    }
}

void DayView::nextOccurrence()
{
    if (currentModel() == allDayModel) {
        if (mAllDayList->currentIndex().row() == mAllDayList->visibleRowCount() - 1) {
            firstTimed();
        } else {
            mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->currentIndex().row() + 1,
                                         0));
        }
    } else {
        // Down key activated for multiple meetings/appointments.
        if (mTimedView->currentIndex().row() < timedModel->rowCount() - 1) {
            mTimedView->setCurrentIndex(timedModel->index(mTimedView->currentIndex().row() + 1, 0));
        } else {
            // Wrap around.
            firstAllDay();
        }
    }
}

void DayView::previousOccurrence()
{
    if (currentModel() == timedModel) {
        if (mTimedView->currentIndex().row() == 0) {
            lastAllDay();
        } else {
            mTimedView->setCurrentIndex(timedModel->index(mTimedView->currentIndex().row() - 1,
                                        0));
        }
    } else {
        if (mAllDayList->currentIndex().row() > 0) {
            mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->currentIndex().row() - 1,
                                        0));
        } else {
            lastTimed();
        }
    }
}

void DayView::setCurrentOccurrence(const QOccurrence &o)
{
    if (o.startInCurrentTZ().date() != mTimedView->date())
        selectDate(o.startInCurrentTZ().date());

    if (o.appointment().isAllDay()) {
        mAllDayList->setCurrentIndex(allDayModel->index(o));
        mTimedView->setCurrentIndex(QModelIndex());
    } else {
        mTimedView->setCurrentIndex(timedModel->index(o));
        mAllDayList->selectionModel()->clear();
    }
}

void DayView::setCurrentAppointment(const QAppointment &a)
{
    setCurrentOccurrence(a.firstOccurrence());
}

bool DayView::eventFilter(QObject *o, QEvent *e)
{
    if (o == mScrollArea->viewport() && e->type() == QEvent::Resize) {
        mTimeManager->setIdealHeight(mScrollArea->viewport()->height());
    }

    return false;
}

void DayView::resizeEvent(QResizeEvent *)
{
    //  Allow all day events to take no more than 1/3 of the screen, and 2/3 when expanded
    mAllDayList->setMaximumFoldedHeight(height() / 3);
    mAllDayList->setMaximumUnfoldedHeight(2 * height() / 3);
    layout()->activate();
}

void DayView::timedSelectionChanged(const QModelIndex &index)
{
    // Save the last selected occurrence, too
    if (index.isValid()) {
        lastSelectedTimedId = timedModel->id(index);
        lastSelectedAllDayId = QUniqueId();

        QRect r = mTimedView->occurrenceRect(index);

        mScrollArea->ensureVisible( 0, r.bottom(), 10, 10 );
        mScrollArea->ensureVisible( 0, r.top(), 10, 10 );

        // And clear this
        mAllDayList->selectionModel()->clear();
    } else {
        lastSelectedTimedId = QUniqueId();
    }

    emit selectionChanged();
}

void DayView::allDayOccurrenceActivated(const QModelIndex &index)
{
    if ( index.isValid() )
        emit showDetails();
}

void DayView::allDayOccurrenceChanged(const QModelIndex &index)
{
    if (index.isValid()) {
        // and make sure this is selected
        mAllDayList->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);

        // And save the id
        lastSelectedAllDayId = allDayModel->id(index);
        lastSelectedTimedId = QUniqueId();

        // Make sure this is cleared (after the above, as index gets invalidated because of this)
        mTimedView->setCurrentIndex(QModelIndex());
    } else {
        lastSelectedAllDayId = QUniqueId();
    }

    emit selectionChanged();
}

void DayView::modelsReset()
{
    bool foundSomething = false;
    // We lose our selections when the models get reset, so try and restore them
    if ( ! lastSelectedAllDayId.isNull()) {
        QModelIndex newsel = allDayModel->index(lastSelectedAllDayId);
        if ( newsel.isValid() ) {
            foundSomething = true;
            mAllDayList->selectionModel()->setCurrentIndex(newsel, QItemSelectionModel::SelectCurrent);
        }
    } else if ( !lastSelectedTimedId.isNull()) {
        QModelIndex newsel = timedModel->index(lastSelectedTimedId);
        if ( newsel.isValid() ) {
            foundSomething = true;
            mTimedView->setCurrentIndex(newsel);
        }
    }

    /* Well, didn't select anything, so select the first event */
    if (!foundSomething) {
        firstTimed(); // same behaviour as setDate
    }
}

void DayView::updateHiddenIndicator(int hidden)
{
    if (hidden) {
        mHiddenIndicator->setText(tr("(%1 more all day appointments)", "%1= number and always > 1").arg(hidden));
        mHiddenIndicator->show();

        // Also have to make sure if the previous selection is now hidden, that we select a different occurrence
        if (mAllDayList->currentIndex().isValid()
                && mAllDayList->currentIndex().row() >= mAllDayList->visibleRowCount()) {
            lastAllDay();
        }
    } else {
        mHiddenIndicator->hide();
    }
}


#include "dayview.moc"
