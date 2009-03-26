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

#ifndef DESKCALLSCREENDIALOGS_H
#define DESKCALLSCREENDIALOGS_H

#include <QDialog>

class QPhoneCall;
class QContact;
class QAbstractButton;
class QLayout;
class QLabel;
class IncomingCallWidget;


class CallDialog : public QDialog
{
    Q_OBJECT
public:
    CallDialog(QWidget *parent = 0);
    virtual ~CallDialog();

    virtual void setPhoneCall(const QPhoneCall &call) = 0;

protected:
    void setupDisplay();
    QWidget *createTitleWidget(const QString &title) const;
    QLayout *createCallerIdLayout(QLabel *nameLabel, QLabel *numberLabel) const;
    virtual QLayout *createMainLayout() = 0;

    QString callerNameFromContact(const QContact &contact) const;

    void setupAffirmativeButton(QAbstractButton *button, bool connectSlot = true);
    void setupNegativeButton(QAbstractButton *button, bool connectSlot = true);

private:
    void setupButton(QWidget *w, const QColor &color) const;
};


class IncomingCallDialogPrivate;
class IncomingCallDialog : public CallDialog
{
    Q_OBJECT
public:
    enum AnswerMode {
        NoAnswerMode,
        HoldAndAnswer,
        EndAndAnswer,
        MergeCalls
    };

    IncomingCallDialog(QWidget *parent = 0);
    ~IncomingCallDialog();
    void setPhoneCall(const QPhoneCall &call);
    AnswerMode answerMode() const;

protected:
    virtual QLayout *createMainLayout();

private slots:
    void callConnected(const QPhoneCall &call);
    void callDropped(const QPhoneCall &call);
    void affirmativeButtonPressed(int id);

private:
    enum PhoneState {
        UnknownState,
        NoOtherCallsActive,
        OtherCallsActive
    };

    void updateState();
    void phoneStateChanged(PhoneState state);
    void setupCallWidget(IncomingCallWidget *callWidget, PhoneState state);

    friend class IncomingCallDialogPrivate;
    IncomingCallDialogPrivate *d;
};


class CallReviewDialogPrivate;
class CallReviewDialog : public CallDialog
{
    Q_OBJECT
public:
    CallReviewDialog(QWidget *parent = 0);
    ~CallReviewDialog();
    void setPhoneCall(const QPhoneCall &call);
    void setPhoneCall(const QPhoneCall &call, bool isMultiPartyCall);

protected:
    QLayout *createMainLayout();

private slots:
    void contactCreationRequested(const QString &fullNumber);

private:
    QLayout *createDetailLayout(const QString &label, QWidget *fieldWdgt) const;

    CallReviewDialogPrivate *d;
};


#endif
