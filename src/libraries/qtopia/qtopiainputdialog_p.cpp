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

#include "qtopiainputdialog_p.h"

#include <QtGui>
#include <private/qdialog_p.h>
#include <qwindowsystem_qws.h>
#include <QtopiaApplication>
#include "keyboard_p.h"

class QtopiaInputDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QtopiaInputDialog)
public:
    QtopiaInputDialogPrivate();
    void init(const QString &title, QWidget* label, QWidget *input);
    EmbeddableKeyboardWidget *m_keyboard;
    KeyboardInputWidget *m_input;
    QWidget *m_widget;

    void _q_preedit(QString text);
    void _q_submitWord(QString word);
    void _q_erase();
};

void QtopiaInputDialogPrivate::_q_preedit(QString text)
{
    QList<QInputMethodEvent::Attribute> attributes;
    QInputContext *qic = m_widget->inputContext();
    QVariant data = qic->standardFormat(QInputContext::PreeditFormat);
    attributes << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, text.length(), data);
    //QInputMethodEvent *ime = new QInputMethodEvent(text, attributes);
    //QApplication::postEvent(m_widget, ime);
    QInputMethodEvent ime(text, attributes);
    QApplication::sendEvent(m_widget, &ime);
}

void QtopiaInputDialogPrivate::_q_submitWord(QString word)
{
    if (qobject_cast<QSpinBox*>(m_widget) || qobject_cast<QTimeEdit*>(m_widget)) {
        //send key events
        QKeyEvent *kpe = new QKeyEvent(QEvent::KeyPress, 0, Qt::NoModifier, word);
        QKeyEvent *kre = new QKeyEvent(QEvent::KeyRelease, 0, Qt::NoModifier, word);
        QApplication::postEvent(m_widget, kpe);
        QApplication::postEvent(m_widget, kre);
        return;
    }
    //QInputMethodEvent *ime = new QInputMethodEvent();
    //ime->setCommitString(word);
    //QApplication::postEvent(m_widget, ime);
    QInputMethodEvent ime;
    ime.setCommitString(word);
    QApplication::sendEvent(m_widget, &ime);
}

void QtopiaInputDialogPrivate::_q_erase()
{
    QKeyEvent *kpe = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QKeyEvent *kre = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::postEvent(m_widget, kpe);
    QApplication::postEvent(m_widget, kre);
}

QtopiaInputDialogPrivate::QtopiaInputDialogPrivate()
    : QDialogPrivate(), m_keyboard(0), m_input(0), m_widget(0)
{
}

static QString stringForHint(QtopiaApplication::InputMethodHint hint, const QString &hintParam)
{
    Q_UNUSED(hintParam);
    switch(hint) {
    case QtopiaApplication::Number:
        return "int";
    case QtopiaApplication::PhoneNumber:
        return "phone";
    case QtopiaApplication::Words:
        return "words";
    case QtopiaApplication::ProperNouns:
        return "propernouns";
    case QtopiaApplication::Text:
        return "text";
    case QtopiaApplication::Named:
        return "named"; //###
    default:
        return "text";
    }
}

void QtopiaInputDialogPrivate::init(const QString &title, QWidget* label, QWidget *input)
{
    Q_Q(QtopiaInputDialog);
    q->setWindowTitle(title);
    QVBoxLayout *vbox = new QVBoxLayout(q);
    //vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)/2);   //### use widget-specific numbers instead
    vbox->setContentsMargins(0,0,0,0);

    if (QLabel *qlabel = qobject_cast<QLabel*>(label)) {
        if (qlabel->text().isEmpty())
            label->hide();
    }

    vbox->addStretch(1);
    vbox->addWidget(label);
    if (qobject_cast<QSpinBox*>(input) || qobject_cast<QTimeEdit*>(input)) {
        QHBoxLayout *hbox = new QHBoxLayout;
        vbox->addLayout(hbox);
        hbox->addStretch(1);
        hbox->addWidget(input);
        hbox->addStretch(1);
    } else {
        vbox->addWidget(input);
    }
    vbox->addStretch(1);

    if (qobject_cast<QLineEdit*>(input) || qobject_cast<QTextEdit*>(input)
        || qobject_cast<QSpinBox*>(input) || qobject_cast<QTimeEdit*>(input)) {
        if(!m_input) {
            m_input = new KeyboardInputWidget;
            QObject::connect(m_input->keyboard, SIGNAL(preedit(QString)), q, SLOT(_q_preedit(QString)));
            QObject::connect(m_input->keyboard, SIGNAL(commit(QString)), q, SLOT(_q_submitWord(QString)));
            QObject::connect(m_input->keyboard, SIGNAL(backspace()), q, SLOT(_q_erase()));
            m_input->keyboard->setAcceptDest(QPoint(0,0));  //###
        }
        QString hint;
        if (qobject_cast<QSpinBox*>(input) || qobject_cast<QTimeEdit*>(input))
            hint = "int";
        else
            hint = stringForHint(QtopiaApplication::inputMethodHint(input), QtopiaApplication::inputMethodHintParam(input));
        m_input->keyboard->setHint(hint);

        vbox->addWidget(m_input);
        m_widget = input;
    }
}

QtopiaInputDialog::QtopiaInputDialog(QWidget *parent, const QString &title, const QString &label, QWidget *input)
    : QDialog(*new QtopiaInputDialogPrivate, parent)
{
    Q_D(QtopiaInputDialog);
    QLabel *l = new QLabel(label);
    l->setBuddy(input);
    d->init(title, l, input);
}

QtopiaInputDialog::QtopiaInputDialog(QWidget *parent, const QString &title, QWidget* label, QWidget *input)
    : QDialog(*new QtopiaInputDialogPrivate, parent)
{
    Q_D(QtopiaInputDialog);
    d->init(title, label, input);
}

QtopiaInputDialog::~QtopiaInputDialog()
{
}

QString QtopiaInputDialog::getText(QWidget *parent, const QString &title, const QString &label, QLineEdit::EchoMode mode,
                                   QtopiaApplication::InputMethodHint hint, const QString &hintParam,
                                   const QString &text, bool *ok)
{
    QLineEdit *le = new QLineEdit;
    QFont font(le->font());
    font.setPointSize(font.pointSize()+3);
    le->setFont(font);
    QtopiaApplication::setInputMethodHint(le, hint, hintParam);
    le->setText(text);
    le->setEchoMode(mode);
    le->setFocus();
    le->selectAll();

    QtopiaInputDialog dlg(parent, title, label, le);

    QString result;
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (accepted)
        result = le->text();

    return result;
}

QString QtopiaInputDialog::getMultiLineText(QWidget *parent, const QString &title, const QString &label,
                                            QtopiaApplication::InputMethodHint hint, const QString &hintParam,
                                            const QString &text, bool *ok)
{
    QTextEdit *te = new QTextEdit;
    QtopiaApplication::setInputMethodHint(te, hint, hintParam);
    te->setPlainText(text);
    te->setFocus();
    te->selectAll();
    te->setMinimumHeight(100);

    QtopiaInputDialog dlg(parent, title, label, te);

    QString result;
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (accepted)
        result = te->toPlainText();

    return result;
}

QString QtopiaInputDialog::getItem(QWidget *parent, const QString &title, const QString &label, const QStringList &list,
                                   int current, bool *ok)
{
    QListWidget *listWidget = new QListWidget;
    listWidget->addItems(list);
    listWidget->setCurrentRow(current);
    listWidget->setFrameShadow(QFrame::Plain);

    QtopiaInputDialog dlg(parent, title, label, listWidget);
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (listWidget->currentItem())
        return listWidget->currentItem()->text();
    else
        return QString();
}

#define CALENDAR_FOR_DATE
QDate QtopiaInputDialog::getDate(QWidget *parent, const QString &title, const QString &label, const QDate &date,
                                 const QDate &minDate, const QDate &maxDate, bool *ok)
{
#ifdef CALENDAR_FOR_DATE
    QCalendarWidget *cal = new QCalendarWidget();
    cal->setSelectedDate(date);
    cal->setMinimumDate(minDate);
    cal->setMaximumDate(maxDate);
    cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    QTextCharFormat headerFormat = cal->headerTextFormat();
    headerFormat.setBackground(QApplication::palette().window());
    headerFormat.setForeground(QApplication::palette().windowText());
    cal->setHeaderTextFormat(headerFormat);
    QWidget *navBar = cal->findChild<QWidget*>("qt_calendar_navigationbar");
    if (navBar)
        navBar->setBackgroundRole(QPalette::Window);

    QtopiaInputDialog dlg(parent, title, label, cal);
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return cal->selectedDate();
#else
    QDateEdit *de = new QDateEdit(date);
    de->setMinimumDate(minDate);
    de->setMaximumDate(maxDate);

    QtopiaInputDialog dlg(parent, title, label, de);
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return de->date();
#endif
}

QTime QtopiaInputDialog::getTime(QWidget *parent, const QString &title, const QString &label, const QTime &time,
                                 const QTime &minTime, const QTime &maxTime, bool *ok)
{
    QTimeEdit *te = new QTimeEdit(time);
    QFont font(te->font());
    font.setPointSize(font.pointSize()+3);
    te->setFont(font);
    te->setMinimumTime(minTime);
    te->setMaximumTime(maxTime);

    QtopiaInputDialog dlg(parent, title, label, te);
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return te->time();
}

int QtopiaInputDialog::getInteger(QWidget *parent, const QString &title, const QString &label, int value,
                                  int minValue, int maxValue, int step, bool *ok)
{
    QSpinBox *sb = new QSpinBox;
    QFont font(sb->font());
    font.setPointSize(font.pointSize()+3);
    sb->setFont(font);
    sb->setValue(value);
    sb->setMinimum(minValue);
    sb->setMaximum(maxValue);
    sb->setSingleStep(step);

    QtopiaInputDialog dlg(parent, title, label, sb);
    bool accepted = (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return sb->value();
}

#include "moc_qtopiainputdialog_p.cpp"
