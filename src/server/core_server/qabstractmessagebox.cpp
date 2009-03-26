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

#include "qabstractmessagebox.h"

#include <QBasicTimer>

#ifdef QTOPIA_USE_TEST_SLAVE
#  include <private/testslaveinterface_p.h>
#endif

#define QABSTRACTMESSAGEBOX_MISSING_COMPONENT \
    "Can't create messagebox, no server component provides QAbstractMessageBox!"

// declare QAbstractMessageBoxPrivate
class QAbstractMessageBoxPrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractMessageBoxPrivate(int time, QAbstractMessageBox::Button but,
                               QObject *parent)
    : QObject(parent), button(but), timeout(time)
    {
    }

    void changeTimeout(int time, QAbstractMessageBox::Button but)
    {
        endTimeout();
        timeout = time;
        button = but;
    }

    void startTimeout()
    {
        if (timeout > 0)
            timer.start(timeout, this);
    }

    void endTimeout()
    {
        timer.stop();
    }

signals:
    void done(int);

protected:
    virtual void timerEvent(QTimerEvent *e)
    {
        if (timer.timerId() == e->timerId()) {
            endTimeout();
            emit done(button);
        }
    }

private:
    QAbstractMessageBox::Button button;
    int timeout;
    QBasicTimer timer;
};

/*!
  \class QAbstractMessageBox
    \inpublicgroup QtBaseModule
  \brief The QAbstractMessageBox class allows developers to replace the message box portion of the Qt Extended server UI.
    \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  The abstract message box is part of the 
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework}
  and represents the portion of the Qt Extended server UI that is shown to users 
  when a message box is needed.
  
  A small tutorial on how to develop new server widgets using one of the 
  abstract widgets as base can be found in QAbstractServerInterface class 
  documentation.

  The QAbstractMessageBox API is intentionally designed to be similar to the 
  QMessageBox API to facilitate easily replacing one with the other.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */

/*!
  \enum QAbstractMessageBox::Icon

  Represents the icon to show in the message box.

  \value NoIcon No icon will be shown.
  \value Question A question icon will be shown.
  \value Information An information icon will be shown.
  \value Warning A warning icon will be shown.
  \value Critical A critical condition icon will be shown.
 */

/*!
  \enum QAbstractMessageBox::Button

  Represents a standard button that may be displayed on the message box.

  \value NoButton An empty button.  This indicates that no button is required.
  \value Ok An Ok button.
  \value Cancel A cancel button.
  \value Yes A yes button.
  \value No A no button.
 */

/*!
  \fn void QAbstractMessageBox::setButtons(Button button1, Button button2)

  Sets the buttons on the message box to the standard buttons \a button1 and
  \a button2.
  */

/*!
  \fn void QAbstractMessageBox::setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)

  Sets the buttons on the message box to custom buttons.

  Up to three custom buttons may be specified with \a button0Text, 
  \a button1Text and \a button2Text which are mapped to keys in a system 
  specific way.  The exec() return value for each of these buttons will be
  0, 1 or 2 repspectively.

  The \a defaultButtonNumber id is returned from exec if the dialog is simply
  "accepted" - usually by pressing the select key.  Likewise the 
  \a escapeButtonNumber is returned if the dialog is dismissed.
  */

/*!
  \fn QString QAbstractMessageBox::title() const

  Returns the title text of the message box.
  */

/*!
  \fn void QAbstractMessageBox::setTitle(const QString &title)

  Sets the \a title text of the message box.
  */

/*!
  \fn QAbstractMessageBox::Icon QAbstractMessageBox::icon() const

  Returns the message box icon. If the icon was set via setIconPixmap()
  this function returns QAbstractMessageBox::NoIcon.
  */

/*!
  \fn void QAbstractMessageBox::setIcon(Icon icon)

  Sets the message box \a icon.
  */

/*!
  \fn void QAbstractMessageBox::setIconPixmap(const QPixmap& pixmap)

  Sets the icon to be used by the message box to \a pixmap.
  \sa icon()
 */

/*!
  \fn QString QAbstractMessageBox::text() const

  Returns the message box text.
  */

/*!
  \fn void QAbstractMessageBox::setText(const QString &text)

  Sets the message box \a text.
  */

/*!
  Constructs a new QAbstractMessageBox instance, with the specified
  \a parent and widget \a flags.
  */
QAbstractMessageBox::QAbstractMessageBox(QWidget *parent,
                                         Qt::WFlags flags)
: QDialog(parent, flags), d(0)
{
}

static int QAbstractMessageBox_exec(QWidget *parent, QAbstractMessageBox::Icon icon, const QString &title, const QString &text, QAbstractMessageBox::Button button1, QAbstractMessageBox::Button button2)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    if (box) {
        box->setIcon(icon);
        box->setTitle(title);
        box->setText(text);
        box->setButtons(button1, button2);
        int rv = QtopiaApplication::execDialog(box);
        delete box;
        return rv;
    }
    qCritical() << QABSTRACTMESSAGEBOX_MISSING_COMPONENT
        << "\n" << title << ":" << text;
    return 0;
}

/*!
  Opens a critical message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::critical(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Critical, title, text, button1, button2);
}

/*!
  Opens a warning message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::warning(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Warning, title, text, button1, button2);
}

/*!
  Opens an informational message box with the \a title and \a text.  The 
  standard buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::information(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Information, title, text, button1, button2);
}

/*!
  Opens a question message box with the \a title and \a text.  The standard
  buttons \a button1 and \a button2 are added to the message box.

  Returns the identity of the standard button that was activated.

  If \a parent is 0, the message box becomes an application-global message box.
  If \a parent is a widget, the message box becomes modal relative to
  \a parent.
 */
int QAbstractMessageBox::question(QWidget *parent, const QString &title, const QString &text, Button button1, Button button2)
{
    return QAbstractMessageBox_exec(parent, Question, title, text, button1, button2);
}

/*!
  Returns a new message box instance with the specified \a parent, \a title,
  \a text, \a icon and standard buttons \a button0 and \a button1.
 */
QAbstractMessageBox *QAbstractMessageBox::messageBox(QWidget *parent, const QString &title, const QString &text, Icon icon, Button button0, Button button1)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    if (box) {
        box->setIcon(icon);
        box->setTitle(title);
        box->setText(text);
        box->setButtons(button0, button1);
    } else {
        qCritical() << QABSTRACTMESSAGEBOX_MISSING_COMPONENT
            << "\n" << title << ":" << text;
    }
    return box;
}

/*!
  Returns a new custom message box instance with the specified \a parent, 
  \a title, \a text and \a icon. 
  
  Up to three custom buttons may be specified with \a button0Text, 
  \a button1Text and \a button2Text which are mapped to keys in a system 
  specific way.  The exec() return value for each of these buttons will be
  0, 1 or 2 repspectively.

  The \a defaultButtonNumber id is returned from exec if the dialog is simply
  "accepted" - usually by pressing the select key.  Likewise the 
  \a escapeButtonNumber is returned if the dialog is dismissed.
 */
QAbstractMessageBox *QAbstractMessageBox::messageBoxCustomButton(QWidget *parent, const QString &title, const QString &text, Icon icon,
            const QString & button0Text, const QString &button1Text,
            const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
    QAbstractMessageBox *box = qtopiaWidget<QAbstractMessageBox>(parent);
    if (box) {
        box->setIcon(icon);
        box->setTitle(title);
        box->setText(text);
        box->setButtons(button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber);
    } else {
        qCritical() << QABSTRACTMESSAGEBOX_MISSING_COMPONENT
            << "\n" << title << ":" << text;
    }
    return box;
}

/*!
  Set an auto timeout value of \a timeoutMs milliseconds.  The dialog will be
  automatically accepted as though the user pressed the \a button key after
  this time.
 */
void QAbstractMessageBox::setTimeout(int timeoutMs, Button button)
{
    if(timeoutMs) {
        if(d) {
            d->changeTimeout(timeoutMs, button);
        } else {
            d = new QAbstractMessageBoxPrivate(timeoutMs, button, this);
            connect( d, SIGNAL(done(int)), this, SLOT(done(int)) );
        }

        if(!isHidden())
            d->startTimeout();
    } else {
        if(d) {
            delete d;
            d = 0;
        }
    }
}


/*! \internal */
void QAbstractMessageBox::hideEvent(QHideEvent *e)
{
    if(d)
        d->endTimeout();

    QDialog::hideEvent(e);
}

/*! \internal */
void QAbstractMessageBox::showEvent(QShowEvent *e)
{
    if(d)
        d->startTimeout();

#ifdef QTOPIA_USE_TEST_SLAVE
    if (QtopiaApplication::instance()->testSlave()) {
        QtopiaApplication::instance()->testSlave()->showMessageBox(this, windowTitle(), text());
    }
#endif

    QDialog::showEvent(e);
}

#include "qabstractmessagebox.moc"
