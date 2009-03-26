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

#include "gui.h"
#include "qoutput.h"
#include <QCoreApplication>
#include <QHBoxLayout>
#include "qbuild.h"
#include <QProgressBar>
#include <QLabel>
#include <QVariant>
#include <QTextEdit>
#include <QTimer>
#include "ruleengine.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static GuiBase *gui = 0;
static ConsoleGui *cgui = 0;
static FILE *tty = 0;

/*!
  \class GuiBase
  \brief The GuiBase class manages the progress bar.

  There is a GUI-based progress bar (-gui) and a console-based progress bar (-progress).
  They are both implemented as sub-classes of GuiBase.
*/

/*!
  \internal
*/
GuiBase::GuiBase()
: m_postInProgress(false), m_certain(false), m_completed(0), m_total(0),
  m_engine(0)
{
    Q_ASSERT(!gui);
    gui = this;
}

/*!
  \internal
*/
GuiBase::~GuiBase()
{
}

/*!
  The GuiBase class has a longer life span than the RuleEngine. Set \a e to the
  rule engine when it is created and set it to 0 when it is deleted.
*/
void GuiBase::setRuleEngine(RuleEngine *e)
{
    m_engine = e;
}

/*!
  Set the \a total and \a completed values. Set \a certain to true if it is
  certain how many more rules there are to run.
*/
void GuiBase::setRuleProgress(int completed, int total, bool certain)
{
    LOCK(Gui);
    m_lock.lock();
    m_completed = completed;
    m_total = total;
    m_certain = certain;
    if (!m_postInProgress && object()) {
        QCoreApplication::postEvent(object(), new QEvent(QEvent::User));
        m_postInProgress = true;
    }
    m_lock.unlock();
}

/*!
  Returns the singleton instance.
*/
GuiBase *GuiBase::instance()
{
    return gui;
}

/*!
  This should be called when writing to stdout or stderr.
  It minimizes conflicts between the console-based progress bar and the regular output.
  Write to \a fd, from \a buffer, \a length characters.

  ie.
  \code
  ::write(1, buf, len);
  \endcode
  becomes
  \code
  GuiBase::instance()->write(1, buf, len);
  \endcode
*/
void GuiBase::write( int fd, const char *buffer, int length )
{
    if ( cgui )
        cgui->write( fd, buffer, length );
    else
        ::write( fd, buffer, length );
}

/*!
  \fn GuiBase::object()
  \internal
*/
// =====================================================================

Gui::Gui()
    : GuiBase()
{
    QVBoxLayout *vlayout = new QVBoxLayout;
    setLayout(vlayout);
    QHBoxLayout *layout = new QHBoxLayout;
    vlayout->addLayout(layout);

    m_bar = new QProgressBar;
    m_label = new QLabel;
    layout->addWidget(m_bar);
    layout->addWidget(m_label);

    m_label->setText("0");
    m_bar->setMinimum(0);
    m_bar->setMaximum(0);

    m_text = new QTextEdit;
    vlayout->addWidget(m_text);

    QTimer *t = new QTimer(this);
    QObject::connect(t, SIGNAL(timeout()), this, SLOT(updateData()));
    t->start(500);
}

Gui::~Gui()
{
}

void Gui::updateData()
{
    QHash<QString, int> results;
    if (m_engine)
        results = m_engine->categories();

    m_text->clear();
    QString value;
    for (QHash<QString, int>::Iterator iter = results.begin(); iter != results.end(); ++iter) {
        QString line = iter.key() + ": " + QString::number(iter.value()) + "\n";
        value.append(line);
    }
    m_text->setText(value);
}

bool Gui::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        LOCK(Gui);
        m_lock.lock();
        QString text;
        text += QString::number(m_completed);
        text += "/";
        if (m_certain)
            text += "<font color=\"green\">";
        else
            text += "<font color=\"red\">~";
        text += QString::number(m_total);
        text += "</font>";
        m_label->setText(text);
        m_bar->setMinimum(0);
        m_bar->setMaximum(m_total);
        m_bar->setValue(m_completed);
        m_postInProgress = false;
        m_lock.unlock();
        return true;
    }
    return QWidget::event(e);
}

// ====================================================================

static bool terminal_resized = true;
static void catch_sigwinch(int)
{
    terminal_resized = true;
}

ConsoleGui::ConsoleGui()
    : GuiBase()
{
    Q_ASSERT(!cgui);
    cgui = this;
    tty = 0;
    int ttyfd = open("/dev/tty", O_WRONLY);
    columns = 0;
    buffer = 0;
    if ( ttyfd != -1 ) {
        terminal_resized = true;
        signal(SIGWINCH, catch_sigwinch);
        tty = fdopen(ttyfd, "w");
    }
    enabled = true;
}

ConsoleGui::~ConsoleGui()
{
    if (columns)
        delete buffer;
}

ConsoleGui *ConsoleGui::instance()
{
    return cgui;
}

bool ConsoleGui::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        //lock(); // no need to flush
        LOCK(Gui);
        m_lock.lock();
        if (tty) {
            fprintf(tty, "\033[K"); // clear out the progress bar
            draw();
        }
        m_postInProgress = false;
        m_lock.unlock();
        return true;
    }
    return QObject::event(e);
}

void ConsoleGui::lock()
{
    LOCK(Gui);
    m_lock.lock();
    if (tty) {
        fprintf(tty, "\033[K"); // clear out the progress bar
        fflush(tty);
    }
}

void ConsoleGui::release( bool _draw )
{
    if ( _draw ) draw();
    m_lock.unlock();
}

void ConsoleGui::draw()
{
    if ( !enabled || !tty ) return;

    // If the terminal has been resized, re-check the width
    if ( terminal_resized ) {
        terminal_resized = false;
        delete buffer; buffer = 0;
#if defined(TIOCGSIZE)
        struct ttysize win;
        if (ioctl(STDIN_FILENO, TIOCGSIZE, &win) == 0) {
            columns = win.ts_cols;
            /*qWarning() << "TIOCGSIZE" << columns;
              } else {
              qWarning() << "TIOCGSIZE failed";*/
        }
#elif defined(TIOCGWINSZ)
        struct winsize win;
        if ( ioctl(STDIN_FILENO, TIOCGWINSZ, &win) == 0) {
            columns = win.ws_col;
            /*qWarning() << "TIOCGWINSZ" << columns;
              } else {
              qWarning() << "TIOCGWINSZ failed";*/
        }
#endif
        if ( !columns ) {
            columns = QVariant(qgetenv("COLUMNS")).toInt();
            //qWarning() << "qgetenv" << columns;
        }
        if (columns)
            buffer = new char[qMax(columns, 255)];
    }

    if ( columns ) {
        memset(buffer, 0, columns); // clear the buffer (so we can avoid writing a trailing '\0' later)
        // 1 / 100 [========================                       ]
        sprintf(buffer, " %d / %d [", m_completed, m_total);
        int buflen = strlen(buffer);
        int cols = columns - (buflen + 2);
        float percent = m_total?((float)m_completed / (float)m_total):0.0;
        int progress = (int)(percent * cols);

        char *ptr = buffer + buflen;
        for ( int i = 0; i < progress; i++ )
            *ptr++ = '=';
        for ( int i = progress; i < cols; i++ )
            *ptr++ = ' ';

        fprintf(tty, "%s]\r", buffer);
    } else {
        // 1 / 100
        fprintf(tty, " %d / %d [Terminal width unknown (export COLUMNS)]\r", m_completed, m_total);
    }
    fflush(tty);
}

void ConsoleGui::write( int fd, const char *buffer, int length )
{
    QByteArray in(buffer, length);
    int newline = in.lastIndexOf('\n');
    QByteArray *leftover = (fd==fileno(stdout)?&leftover_out:&leftover_err);
    if ( newline == -1 ) {
        LOCK(Gui);
        m_lock.lock();
        leftover->append(in);
        m_lock.unlock();
        return;
    }
    LOCK(Gui);
    lock();
    QByteArray line = (*leftover);
    if ( newline == in.count()-1 ) {
        line.append(in);
        leftover->clear();
    } else {
        line.append(in.left(newline+1));
        (*leftover) = in.mid(newline+1);
    }
    ::write(fd, line.constData(), line.count());
    release();
}

void ConsoleGui::dumpLeftovers()
{
    if ( leftover_out.count() ) {
        ::write(fileno(stdout), leftover_out.constData(), leftover_out.count());
    }
    if ( leftover_err.count() ) {
        ::write(fileno(stderr), leftover_err.constData(), leftover_err.count());
    }
}

void ConsoleGui::disable()
{
    enabled = false;
    release(false);
}

