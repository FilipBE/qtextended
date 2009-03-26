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

#include "contextkeymanager_p.h"
#include <qvalidator.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qsettings.h>
#include <qcombobox.h>
#include <QKeySequence>
#include <qtextcursor.h>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextBrowser>
#include <QObject>
#include <QSoftKeyLabelHelper>
#include <QInputContext>
#ifndef QT_NO_WIZARD
#include <QWizard>
#endif

#include "qtopiaipcenvelope.h"
#include "qsoftmenubar.h"
#include "qtopialog.h"


class ContextKeyManagerQSpinBoxLineEditAccessor : public QSpinBox
{
    Q_OBJECT
public:
    QLineEdit *getLineEdit() { return lineEdit(); }
};

// should this be Q_GLOBAL_STATIC?
static ContextKeyManager *contextKeyMgr = 0;

ContextKeyManager::ContextKeyManager()
{
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    cfg.beginGroup("SoftKeys");
    int buttonCount = cfg.value("Count", 0).toInt();
    if (buttonCount) {
        for (int i=0; i<buttonCount; i++) {
            QString is = QString::number(i);
            int btn = QKeySequence(cfg.value("Key"+is).toString())[0];
            qLog(UI) << "Read key:" << cfg.value("Key"+is).toString() << "id:" << btn;
            buttons.append(btn);
        }

        // Setup non-standard class context labels.
        QSoftMenuBar::StandardLabel lbl = QSoftMenuBar::NoLabel;
        if( !Qtopia::mousePreferred() )
            lbl = QSoftMenuBar::EndEdit;

#ifndef  QTOPIA_TURN_OFF_OLD_SOFTKEY_HANDLING
        setClassStandardLabel("QLineEdit", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QDateTimeEdit", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QSpinBox", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
#endif
        setClassStandardLabel("QTextEdit", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QSlider", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QComboBox", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);

        setClassStandardLabel("QButton", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
        setClassStandardLabel("QMenu", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
        setClassStandardLabel("QTextBrowser", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QComboBoxPrivateContainer", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);

#ifndef  QTOPIA_TURN_OFF_OLD_SOFTKEY_HANDLING
        setClassStandardLabel("QLineEdit", Qt::Key_Select, QSoftMenuBar::Edit, QSoftMenuBar::NavigationFocus);
        setClassStandardLabel("QDateTimeEdit", Qt::Key_Select, QSoftMenuBar::Edit, QSoftMenuBar::NavigationFocus);
        setClassStandardLabel("QSpinBox", Qt::Key_Select, QSoftMenuBar::Edit, QSoftMenuBar::NavigationFocus);
#endif
        setClassStandardLabel("QTextEdit", Qt::Key_Select, QSoftMenuBar::Edit, QSoftMenuBar::NavigationFocus);
        setClassStandardLabel("QTabBar", Qt::Key_Select, QSoftMenuBar::NoLabel, QSoftMenuBar::NavigationFocus);
        setClassStandardLabel("QSlider", Qt::Key_Select, QSoftMenuBar::Edit, QSoftMenuBar::NavigationFocus);

        setClassStandardLabel("QMenu", Qt::Key_Back, QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
        if (QApplication::style()->inherits("Series60Style"))
            setClassStandardLabel("QMenu", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);

        setClassStandardLabel("QCalendarPopup", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);

        if( !Qtopia::mousePreferred() ) {
#ifndef  QTOPIA_TURN_OFF_OLD_SOFTKEY_HANDLING
            setClassStandardLabel("QLineEdit", Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QDateTimeEdit", Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
#endif
            setClassStandardLabel("QTextEdit", Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QSlider", Qt::Key_Back, QSoftMenuBar::RevertEdit, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QComboBoxPrivateContainer", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);
            setClassStandardLabel("QTextBrowser", Qt::Key_Back, QSoftMenuBar::Back, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QCalendarPopup", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);
        }

        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateLabelsForFocused()));
    }

    QSettings config("Trolltech","qpe");
    config.beginGroup( "ContextMenu" );
    lType = static_cast<QSoftMenuBar::LabelType>(config.value( "LabelType", QSoftMenuBar::TextLabel).toInt());
}

/*
    Setup the softkeyhelpers
*/
void ContextKeyManager::setupStandardSoftKeyHelpers()
{
    new QLineEditSoftKeyLabelHelper();
    new QDateTimeEditSoftKeyLabelHelper();

    updateContextLabels();
}

// Setup the appropriate context labels for widget w.
void ContextKeyManager::updateContextLabels()
{
    if (buttons.count())
        timer->start(0);
}

void ContextKeyManager::updateLabelsForFocused()
{
    if (!qApp->activeWindow())
        return;

    QWidget *w = QApplication::activePopupWidget();
    if (!w)
        w = qApp->focusWidget();
    if (!w)
        w = qApp->activeWindow();

    // first check for a softkey helper:
    if(findHelper(w)) {
        return;
    } else {
        QAbstractSoftKeyLabelHelper* classHelper = findClassHelper(w);
        if(classHelper)
        {
            classHelper->setCurrentWidget(w);
            classHelper->updateAllLabels();
            return;
        }
    }

    bool modal = true;
    if( !Qtopia::mousePreferred() )
        modal = w->hasEditFocus();

    int menuKey = QSoftMenuBar::menuKey();
    bool editMenu = false;
    QSoftMenuBar::StandardLabel backLabel = QSoftMenuBar::Back;
    QSoftMenuBar::StandardLabel selectLabel = QSoftMenuBar::Select;
    bool overrideBack = false;
    bool overrideSelect = false;

    if( !Qtopia::mousePreferred() ) {
        if (modal) {
            QLineEdit *l = qobject_cast<QLineEdit*>(w);
#ifndef  QTOPIA_TURN_OFF_OLD_SOFTKEY_HANDLING
            if (!l && w->inherits("QSpinBox"))
                l = (static_cast<ContextKeyManagerQSpinBoxLineEditAccessor*>(w))->getLineEdit();
#endif
            if (!l && w->inherits("QComboBox"))
                l = (static_cast<QComboBox*>(w))->lineEdit();
            if (l) {
                editMenu = !haveLabelForWidget(w, menuKey, modal);
                if ((l->text().length() == 0 && l->inputContext() && !l->inputContext()->isComposing())
                        || l->isReadOnly())
                    backLabel = QSoftMenuBar::RevertEdit;
                else if (l->cursorPosition() == 0 && l->inputContext() && !l->inputContext()->isComposing())
                    backLabel = QSoftMenuBar::NoLabel;
                else
                    backLabel = QSoftMenuBar::BackSpace;
                if (!haveLabelForWidget(w, Qt::Key_Back, modal))
                    overrideBack = true;
            } else if (w->inherits("QTextEdit")) {
                if (w->inherits("QTextBrowser")) {
                    QTextBrowser *tb = qobject_cast<QTextBrowser*>(w);
                    editMenu = !haveLabelForWidget(w, menuKey, modal);
                    if (!haveLabelForWidget(w, Qt::Key_Select, modal)) {
                        if (tb->textCursor().hasSelection() &&
                                !tb->textCursor().charFormat().anchorHref().isEmpty()) {
                            selectLabel = QSoftMenuBar::Select;
                            overrideSelect = true;
                        } else {
                            selectLabel = QSoftMenuBar::NoLabel;
                            overrideSelect = true;
                        }
                    }
                } else {
                    QTextEdit *e = qobject_cast<QTextEdit*>(w);
                    editMenu = !haveLabelForWidget(w, menuKey, modal);
                    if (e->document()->isEmpty() && e->textCursor().block().layout()->preeditAreaText().isEmpty()) {
                        backLabel = QSoftMenuBar::RevertEdit;
                        if (!haveLabelForWidget(w, Qt::Key_Back, modal))
                            overrideBack = true;
                    } else if (e->isReadOnly()) {
                        backLabel = QSoftMenuBar::Cancel;
                        if (!haveLabelForWidget(w, Qt::Key_Back, modal))
                            overrideBack = true;
                    }
                }
            } else if ( (w->windowFlags() & Qt::Popup) ) {
                if (!haveLabelForWidget(w, Qt::Key_Back, modal)) {
                    backLabel = QSoftMenuBar::Cancel;
                    if (!haveLabelForWidget(w, Qt::Key_Back, modal))
                        overrideBack = true;
                }
            }
        }
        QAbstractButton *b = qobject_cast<QAbstractButton*>(w);
        if (b && b->isCheckable()) {
            if (w->inherits("QCheckBox")) {
                if (b->isChecked())
                    selectLabel = QSoftMenuBar::Deselect;
                else
                    selectLabel = QSoftMenuBar::Select;
                overrideSelect = true;
            }
        }
#ifndef QT_NO_WIZARD
        QWizard *wizard = qobject_cast<QWizard *>(w->topLevelWidget());
        if (wizard) {
            if (wizard->currentPage()->isComplete()) {
                if (wizard->nextId() == -1)
                    backLabel = QSoftMenuBar::Finish;
                else
                    backLabel = QSoftMenuBar::Next;
                overrideBack = true;
            } else if (wizard->currentId() != wizard->startId()) {
                backLabel = QSoftMenuBar::Previous;
                overrideBack = true;
            }
        }
#endif
    }

    for (int i = 0; i < static_cast<int>(buttons.count()); i++) {
        if (buttons[i] == menuKey) {
            if (editMenu) {
                setStandard(w, menuKey, QSoftMenuBar::Options);
            } else if (!updateContextLabel(w, modal, menuKey)) {
                clearLabel(w, menuKey);
            }
        } else {
            switch (buttons[i]) {
                case Qt::Key_Select:
                    if (!Qtopia::mousePreferred() && overrideSelect) {
                        setStandard(w, Qt::Key_Select, selectLabel);
                    } else if (!updateContextLabel(w, modal, Qt::Key_Select)) {
                        if (w->focusPolicy() != Qt::NoFocus)
                            setStandard(w, Qt::Key_Select, QSoftMenuBar::Select);
                        else
                            clearLabel(w, Qt::Key_Select);
                    }
                    break;
                case Qt::Key_Back:
                    if (!Qtopia::mousePreferred() && overrideBack) {
                        setStandard(w, Qt::Key_Back, backLabel);
                    } else if (!updateContextLabel(w, modal, Qt::Key_Back)) {
                        setStandard(w, Qt::Key_Back, backLabel);
                    }
                    break;
                default:
                    if (!updateContextLabel(w, modal, buttons[i]))
                        clearLabel(w, buttons[i]);
            }
        }
    }
}

QAbstractSoftKeyLabelHelper *ContextKeyManager::findHelper(QString className)
{
    return helperClassMap.value(className);
}

QAbstractSoftKeyLabelHelper *ContextKeyManager::findClassHelper(QWidget *w)
{
    const QMetaObject* meta_w = w->metaObject();
    while(meta_w)
    {
        if(helperClassMap.contains(meta_w->className()))
        {
            // Relying on the helper to treat this as the appropriate class
            //                    helperClassMap.value(meta_w->className())->setCurrentWidget(w);
            //                    helperClassMap.value(meta_w->className())->focusIn(w);
            return helperClassMap.value(meta_w->className());
        };
        meta_w = meta_w->superClass();
    };
    return false;
}

QAbstractSoftKeyLabelHelper *ContextKeyManager::findHelper(QWidget *w)
{
    return helperMap.value(w);
}

bool ContextKeyManager::updateContextLabel(QWidget *w, bool modal, int key)
{
    // first check for softkey helper, and do nothing if one is present
    if(findHelper(w)) {
        return true;
    } else {
        QAbstractSoftKeyLabelHelper* helper = findClassHelper(w);
        if(helper) {
            helper->setCurrentWidget(w);
            return true;
        };
    };

    // Next see if label is set for this widget explicitly
    QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
    if (wit != contextWidget.end()) {
        KeyMap::Iterator it = (*wit).find(key);
        if (it != (*wit).end()) {
            switch ((*it).type(modal)) {
                case ModalState::Custom: {
                        QString text((*it).text(modal));
                        QString pixmap((*it).pixmap(modal));
                        if (pixmap.isEmpty() || lType == QSoftMenuBar::TextLabel)
                            setText(w, key, text);
                        else
                            setPixmap(w, key, pixmap);
                    }
                    return true;
                case ModalState::Standard:
                    setStandard(w, key, (*it).label(modal));
                    return true;
                default:
                    break;
            }
        }
    }

    // Next see if label is set for the class (must accept focus)
    if (w->focusPolicy() != Qt::NoFocus
        || (w->windowFlags() & Qt::Popup)) {
        QList<ClassModalState>::Iterator cit;
        for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
            if (w->inherits((*cit).className)) {
                KeyMap::Iterator it = (*cit).keyMap.find(key);
                if (it != (*cit).keyMap.end()) {
                    switch ((*it).type(modal)) {
                        case ModalState::Custom: {
                                QString text((*it).text(modal));
                                QString pixmap((*it).pixmap(modal));
                                if (pixmap.isEmpty() || lType == QSoftMenuBar::TextLabel)
                                    setText(w, key, text);
                                else
                                    setPixmap(w, key, pixmap);
                            }
                            return true;
                        case ModalState::Standard:
                            setStandard(w, key, (*it).label(modal));
                            return true;
                        default:
                            break;
                    }
                }
            }
        }
    }

    // Otherwise try our parent
    if (w->parentWidget() && !w->isTopLevel()) {
        return updateContextLabel(w->parentWidget(), false, key);
    }

    return false;
}

QWidget *ContextKeyManager::findTargetWidget(QWidget *w, int key, bool modal)
{
    // First see if key is set for this widget explicitly
    QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
    if (wit != contextWidget.end()) {
        KeyMap::Iterator it = (*wit).find(key);
        if (it != (*wit).end()) {
            if ((*it).type(modal) != ModalState::NoLabel)
                return w;
        }
    }

    // Next see if key is set for the class (must accept focus)
    if (w->focusPolicy() != Qt::NoFocus) {
        QList<ClassModalState>::Iterator cit;
        for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
            if (w->inherits((*cit).className)) {
                KeyMap::Iterator it = (*cit).keyMap.find(key);
                if (it != (*cit).keyMap.end()) {
                    if ((*it).type(modal) != ModalState::NoLabel)
                        return w;
                }
            }
        }
    }

    // otherwise try our parent
    if (w->parentWidget() && !w->isTopLevel())
        return findTargetWidget(w->parentWidget(), key, false);

    // No one has claimed this key.
    return 0;
}

bool ContextKeyManager::haveLabelForWidget(QWidget *w, int key, bool modal)
{
    if (buttons.count()) {
        QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
        if (wit != contextWidget.end()) {
            KeyMap::Iterator it = (*wit).find(key);
            if (it != (*wit).end()) {
                if ((*it).type(modal) != ModalState::NoLabel)
                    return true;
            }
        }
    }

    return false;
}

bool ContextKeyManager::haveCustomLabelForWidget(QWidget *w, int key, bool modal)
{
    if (buttons.count()) {
        QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
        if (wit != contextWidget.end()) {
            KeyMap::Iterator it = (*wit).find(key);
            if (it != (*wit).end()) {
                if ((*it).type(modal) == ModalState::Custom)
                    return true;
            }
        }
    }

    return false;
}

void ContextKeyManager::setContextText(QWidget *w, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mText = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmText = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextPixmap(QWidget *w, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mPixmap = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmPixmap = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextStandardLabel(QWidget *w, int key, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Standard;
        ms.mStandard = label;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Standard;
        ms.nmStandard = label;
    }
    updateContextLabels();
}


void ContextKeyManager::clearContextLabel(QWidget *w, int key, QSoftMenuBar::FocusState state)
{
    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::NoLabel;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::NoLabel;
    }
    updateContextLabels();
}

void ContextKeyManager::setClassText(const QByteArray &className, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mText = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmText = t;
    }
}

void ContextKeyManager::setClassPixmap(const QByteArray &className, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mPixmap = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmPixmap = t;
    }
}

void ContextKeyManager::setClassStandardLabel(const QByteArray &className, int key, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Standard;
        ms.mStandard = label;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Standard;
        ms.nmStandard = label;
    }
}

void ContextKeyManager::removeSenderFromWidgetContext()
{
    if ( contextWidget.contains( qobject_cast<QWidget*>( sender()))) {
        QWidget *w = qobject_cast<QWidget*>(sender());
        contextWidget.remove(w);
        updateContextLabels();
    }
}

ContextKeyManager *ContextKeyManager::instance()
{
    if (!contextKeyMgr) {
        contextKeyMgr = new ContextKeyManager();
#ifdef QTOPIA_TURN_OFF_OLD_SOFTKEY_HANDLING
        contextKeyMgr->setupStandardSoftKeyHelpers();
#endif
    }

    return contextKeyMgr;
}

void ContextKeyManager::setStandard(QWidget *w, int key, QSoftMenuBar::StandardLabel label)
{
    QString l = standardPixmap(label);
    if (lType == QSoftMenuBar::IconLabel && !l.isEmpty())
        setPixmap(w, key, l);
    else
        setText(w, key, standardText(label));
}

void ContextKeyManager::setText(QWidget *w, int key, const QString &text)
{
    if(!w || !w->topLevelWidget())
        return;

    w = w->topLevelWidget();
    if (w->windowFlags() & Qt::Popup && QApplication::activeWindow())
        w = QApplication::activeWindow();
    int win = w->winId();

    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "setLabelText(int,int,QString)");
    e << win;
    e << key;
    e << text;
}

void ContextKeyManager::setPixmap(QWidget *w, int key, const QString &pm)
{
    if(!w || !w->topLevelWidget())
        return;

    w = w->topLevelWidget();

    if (w->windowFlags() & Qt::Popup && QApplication::activeWindow())
        w = QApplication::activeWindow();
    int win = w->winId();

    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "setLabelPixmap(int,int,QString)");
    e << win;
    e << key;
    e << pm;
}

void ContextKeyManager::clearLabel(QWidget *w, int key)
{
    if(!w || !w->topLevelWidget())
        return;

    w = w->topLevelWidget();
    if (w->windowFlags() & Qt::Popup && QApplication::activeWindow())
        w = QApplication::activeWindow();
    int win = w->winId();
    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "clearLabel(int,int)");
    e << win;
    e << key;
}

void ContextKeyManager::setLabelType(QSoftMenuBar::LabelType type)
{
    lType = type;
}

QString ContextKeyManager::standardText(QSoftMenuBar::StandardLabel l)
{
    switch (l) {
        case QSoftMenuBar::NoLabel:
            return "";
        case QSoftMenuBar::Options:
            return tr("Options");
        case QSoftMenuBar::Ok:
            return tr("OK");
        case QSoftMenuBar::Edit:
            return tr("Edit");
        case QSoftMenuBar::Select:
            return tr("Select");
        case QSoftMenuBar::View:
            return tr("View");
        case QSoftMenuBar::Cancel:
            return tr("Cancel");
        case QSoftMenuBar::Back:
            return tr("Back");
        case QSoftMenuBar::BackSpace:
            return tr("Delete");
        case QSoftMenuBar::Next:
            return tr("Next");
        case QSoftMenuBar::Previous:
            return tr("Prev");
        case QSoftMenuBar::EndEdit:
            return tr("Accept");
        case QSoftMenuBar::RevertEdit:
            return tr("Cancel");
        case QSoftMenuBar::Deselect:
            return tr("Deselect");
        case QSoftMenuBar::Finish:
            return tr("Finish");
        default:
            return QString();
    }
}

QString ContextKeyManager::standardPixmap(QSoftMenuBar::StandardLabel l)
{
    switch (l) {
        case QSoftMenuBar::NoLabel:
            return "";
        case QSoftMenuBar::Options:
            return "options";
        case QSoftMenuBar::Back:
            return "i18n/back";
        case QSoftMenuBar::Cancel:
            return "cancel";
        case QSoftMenuBar::Ok:
            return "ok";
        case QSoftMenuBar::Edit:
            return "edit";
        case QSoftMenuBar::Select:
            return "select";
        case QSoftMenuBar::View:
            return "view";
        case QSoftMenuBar::BackSpace:
            return "i18n/backspace";
        case QSoftMenuBar::Next:
            return "i18n/next";
        case QSoftMenuBar::Previous:
            return "i18n/previous";
        case QSoftMenuBar::EndEdit:
            return "edit-end";
        case QSoftMenuBar::RevertEdit:
            return "edit-revert";
        case QSoftMenuBar::Deselect:
            return "select";
        case QSoftMenuBar::Finish:
            return "done";
        default:
            return QString();
    }
}

void ContextKeyManager::setContextKeyHelper(QWidget* w, QAbstractSoftKeyLabelHelper* helper)
{
    helperMap.insert ( w, helper );
};


void ContextKeyManager::clearContextKeyHelper(QWidget* w)
{
    helperMap.remove ( w );
};


void ContextKeyManager::setContextKeyHelper(const QString& className, QAbstractSoftKeyLabelHelper* helper)
{
    helperClassMap.insert ( className , helper);
};

void ContextKeyManager::clearContextKeyHelper(QString& className)
{
    helperClassMap.remove ( className );
};
