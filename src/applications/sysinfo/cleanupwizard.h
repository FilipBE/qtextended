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

#ifndef CLEANUPWIZARD_H
#define CLEANUPWIZARD_H

#include <QWizard>
#include <QDialog>

#include "ui_documenttypeselector.h"

class QCheckBox;
class QSpinBox;
class QLabel;
class DocumentsSelectionDialog;
class DocumentsCleanupDialog;
class QProgressBar;
class QDateEdit;
class DocumentListModel;
class QWaitWidget;
class QKeyEvent;
class QTableView;
class QContentSet;
class QContentSetModel;

class CleanupWizard: public QWizard
{
    Q_OBJECT
public:
    CleanupWizard(QWidget* parent = 0);
    ~CleanupWizard(){};
    enum pageID {
        preselectionID,
        documentTypeSelectionID,
        documentListSelectionID,
        mailSelectionID,
        eventSelectionID,
        finalSummaryID
    };

private slots:
    void updateWizard(int id);
};


class PreselectionPage : public QWizardPage
{
    Q_OBJECT
public:
    PreselectionPage(QWidget* parent = 0);
    int nextId() const;
public slots:
    bool isComplete() const;
private:
    QCheckBox* documentCheckBox;
    QCheckBox* mailCheckBox;
    QCheckBox* eventCheckBox;
};


class MailSelectionPage : public QWizardPage
{
    Q_OBJECT
public:
    MailSelectionPage(QWidget* parent = 0);
    int nextId() const;
    bool validatePage();
private:
    QSpinBox* sizeBox;
    QDateEdit* dp;
};


class EventSelectionPage : public QWizardPage
{
    Q_OBJECT
public:
    EventSelectionPage(QWidget* parent = 0);
    int nextId() const;
    bool validatePage();

private:
    QDateEdit* dp;
};

class DocumentTypeSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    DocumentTypeSelectionPage(QWidget* parent = 0);

    int nextId() const;
    bool isComplete() const;

public slots:
    void alltypes(bool allSChecked);

private:
    Ui::DocumentTypeSelector *m_ui;

friend class DocumentListSelectionPage;
};

class DocumentListSelectionPage : public QWizardPage
{
    Q_OBJECT
public:
    DocumentListSelectionPage(QWidget* parent = 0);
    int nextId() const;
    void initializePage();
    bool validatePage();
public slots:
    void updateCleanupStopped(bool c){ cleanupStopped = c;  };
    void stopCleanup(){ cleanupStopped = true; };
    void cleanup();

    void updateFinished();
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

signals:
    void documentsDeleted(int d,bool a);
protected:
    void updateHeader();
    void changeSelectedSize(int size){ selectedSize += size;
                                       updateHeader(); };
private:
    int docDeleted;
    int selectedSize;
    int selectedDocuments;

    QLabel *header;
    QTableView *list;

    DocumentsCleanupDialog *documentsCleanupDialog;

    bool cleanupStopped;

    QContentSet *documents;
    QContentSetModel *model;

    QContentSet *largeDocuments;
    DocumentListModel *largeModel;

    QWaitWidget *waitWidget;
};

class DocumentsCleanupDialog : public QDialog
{
    Q_OBJECT
public:
    DocumentsCleanupDialog(QWidget* parent = 0);
private:
    QProgressBar* progressBar;
    QLabel*       label;
    QPushButton*  pushButton;

friend class DocumentListSelectionPage;
};


class FinalSummaryPage : public QWizardPage
{
    Q_OBJECT
public:
    FinalSummaryPage(QWidget* parent = 0);
    int nextId() const;
    void initializePage();
public slots:
    void documentsDeleted(int d,bool a){ docDeleted = d;
                                         cleanupAborted = a; };
private:
    int docDeleted;
    bool cleanupAborted;
    QLabel* header;
    QLabel* summary;
};

#endif
