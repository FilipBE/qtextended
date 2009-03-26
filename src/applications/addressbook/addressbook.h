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
#ifndef Addressbook_H
#define Addressbook_H

#include <QMainWindow>
#include <QDialog>
#include <QSet>
#include <QItemDelegate>

#include <qcategorymanager.h>
#include <qcontact.h>
#include <qpimsource.h>
#include <qtopiaabstractservice.h>
#include "qtopiaserviceselector.h"

#ifdef QTOPIA_CELL
#   include <qtopiaphone/qphonebook.h>
class QSimInfo;
#endif

class QAction;
class QVBoxLayout;
class QRadioButton;
class QButtonGroup;
class QStackedWidget;
class QContact;
class QContactModel;
class QUniqueId;
class QDSData;
class QDSActionRequest;
class AbEditor;
class AbFullEditor;
class AbSimEditor;
class GroupView;
class ContactListPane;
class ContactDetails;
class DeskphoneContactDetails;
class LoadIndicator;
class QContactSimContext;
#ifdef QTOPIA_HOMEUI
class DeskphoneEditor;
#endif
class QContactFieldDefinition;

class AddressbookWindow : public QMainWindow
{
    Q_OBJECT
    friend class ContactsService;
    friend class ContactsPhoneService;
public:
    AddressbookWindow(QWidget *parent = 0, Qt::WFlags f = 0);
    virtual ~AddressbookWindow();

    QUniqueId addContact(const QContact &c);
    void updateContact(const QContact &c);

protected:
    void resizeEvent(QResizeEvent * e);
    void newEntry();
    void newEntry(const QContact &cnt);
    void editEntry(const QContact &cnt);
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *receiver, QEvent *e);
    QString pickEmailAddress(QStringList emails);
    int pickFavoritesType(QStringList emails, QMap<QContact::PhoneType,QString> numbers);
    bool updateFavoritesPhoneServiceDescription(QtopiaServiceDescription* desc, const QContact& ent, QContact::PhoneType phoneType, bool isSms);
    bool updateFavoritesEmailServiceDescription(QtopiaServiceDescription* desc, const QContact& ent, const QString& emailAddress);
    bool updateFavoritesViewServiceDescription(QtopiaServiceDescription* desc, const QContact& ent);
public slots:
    void appMessage(const QString &, const QByteArray &);
    void setDocument(const QString &);
    void reload();
    void flush();
#ifdef QTOPIA_CELL
    void vcardDatagram( const QByteArray& data );
#endif

private slots:
    // QAction slots
    void sendContact();
    void sendContactCat();
    void deleteContact();
    void markCurrentAsPersonal();
    void addToFavorites();
    void configure();
    void showPersonalView();
    void createNewContact();
    void previousView();
    void groupList();

#ifdef QTOPIA_HOMEUI
    // Sorting slots
    void viewFirstLast();
    void viewLastFirst();
    void filterPresence();
    void sortLabel();
    void sortCompany();
#endif

    // Details pane slots
    void callCurrentContact();
    void textCurrentContact();
    void emailCurrentContact();
    void editCurrentContact();

    void contactActivated(QContact);

    // Misc slots
    void delayedInit();
    void editPersonal();
    void addPhoneNumberToContact(const QString& phoneNumber);
    void createNewContact(const QString& phoneNumber);
    void setContactImage(const QString& filename);
    void setPersonalImage(const QString& filename);
    void showCategory( const QString &);
    void updateContextMenu();
    void updateContextMenuIfDirty();
    void setContextMenuDirty();
    void setCurrentContact( const QContact &entry );
    void currentContactSelectionChanged();
    void loadMoreVcards();
    void cancelLoad();

    void removeContactFromCurrentGroup();
#ifdef QTOPIA_CELL
    void updateLoadLabel();
#endif

    void setContactImage( const QDSActionRequest& request );
    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );

    void updateEditDetails(const QContact &);

    void contactsChanged();

private:
#ifdef QTOPIA_CELL
    void smsBusinessCard();
#endif

private:
    void createViewMenu();
    void createDetailedView();
    void createGroupListView();
    void createGroupMemberView();
    void showCategory( const QCategoryFilter &, bool saveState);

    void receiveFile(const QString &filename, const QString &mimetype = QString());

    void readConfig();
    void writeConfig();

    void showDetailsView(bool saveState);
    void showGroupListView(bool saveState);
    void showGroupMemberView(bool saveState);
    void showListView(bool saveState);

    void saveViewState();
    void restoreViewState();

    void clearSearchBars();

    void showJustItem(const QUniqueId &uid);

    void updateGroupNavigation();

    void updateDependentAppointments(const QContact& src, AbEditor* editor);
    void updateFavoritesEntries(const QContact& c);
    void removeFavoritesEntries(const QContact& c);

    void removeChatSubscriptions(const QContact& cnt);
    void removeChatAddress(const QContactFieldDefinition& def, const QContact& cnt);
    void addChatAddress(const QContactFieldDefinition& def, const QContact& cnt);

    bool checkSyncing();

    QDSData contactQDLLink( const QContact& contact );
    void removeContactQDLLink( const QContact& contact );

    QContactModel *mModel;
    QContactModel *mFilterModel;

    bool mCloseAfterView;
    bool mHasSim;

    mutable bool mContextMenuDirty;

    QStackedWidget *centralView;

    ContactListPane *mListView;
    GroupView *mGroupsListView;
    ContactListPane *mGroupMemberView;
#ifndef QTOPIA_HOMEUI
    ContactDetails *mDetailsView;
#else
    DeskphoneContactDetails *mDetailsView;
#endif

    AbEditor *editor(const QUniqueId &);
#ifdef QTOPIA_HOMEUI
    DeskphoneEditor *dpEditor;
#else
    AbFullEditor *abFullEditor;
#ifdef QTOPIA_CELL
    AbSimEditor *abSimEditor;
#endif
#endif

    QAction *actionNew,
            *actionEdit,
            *actionTrash,
            *actionPersonal;
// group actions
    QAction *actionShowGroups,
            *actionAddGroup,
            *actionRemoveGroup,
            *actionRenameGroup,
            *actionManageMembers;
#if defined(QTOPIA_TELEPHONY)
    QAction *actionSetRingTone;
#endif

    QAction *actionSetPersonal,
            *actionResetPersonal,
            *actionSend,
            *actionSendCat,
            *actionFavorites;

#ifdef QTOPIA_HOMEUI
    QAction *actionViewFirstLast,
            *actionViewLastFirst,
            *actionFilterPresence,
            *actionSortLabel,
            *actionSortCompany;
    int mViewIndex;
    int mSortIndex;
    bool mFilterPresence;
#else
    QAction *actionSettings;
#endif

    bool syncing;

    QCategoryFilter mCurrentFilter;

    QContact currentContact() const;
    mutable QContact mCurrentContact;
    mutable bool mCurrentContactDirty;

    typedef struct {
        QContact contact;
        enum {List, Groups, GroupMembers, Details} pane;
    } AB_State;
    QList<AB_State> mContactViewStack;

    // Incremental vCard loading
    LoadIndicator *loadinfo;
    enum { Start, Read, DuplicateCheck, Process, ConfirmAdd, Add, Done } loadState;
    QList<QContact> loadedcl;
    QList<QContact> loadednewContacts, loadedoldContacts;
    int loadednewContactsCursor;
    bool loadedWhenHidden;
    QString loadingFile;
    bool deleteLoadingFile;
};

class AbDisplaySettings : public QDialog
{
    Q_OBJECT

public:
    AbDisplaySettings(QWidget *parent);

    void saveFormat();
    QString format();

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    QVBoxLayout* layout;
    QButtonGroup* bg;
};

class AbSourcesDialog : public QDialog
{
    Q_OBJECT

public:
    AbSourcesDialog(QWidget *parent, const QSet<QPimSource> &availSources);
    void setSources(const QSet<QPimSource> &sources);
    QSet<QPimSource> sources() const;

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    QVBoxLayout* layout;
    QButtonGroup* bg;
    QRadioButton* phoneButton;
    QRadioButton* simButton;
    QRadioButton* bothButton;
    QSet<QPimSource> availableSources;
};

class ContactsService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class AddressbookWindow;
private:
    ContactsService( AddressbookWindow *parent )
        : QtopiaAbstractService( "Contacts", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ContactsService();

public slots:
    void editPersonal();
    void editPersonalAndClose();
    void addContact(const QContact& contact);
    void removeContact(const QContact& contact);
    void updateContact(const QContact& contact);
    void addAndEditContact(const QContact& contact);
    void addPhoneNumberToContact(const QString& phoneNumber);
    void createNewContact(const QString& phoneNumber);
    void showContact(const QUniqueId& uid);
    void showContacts();
    void setContactImage(const QString& filename);
    void setContactImage( const QDSActionRequest& request );
    void setPersonalImage(const QString& filename);
    void activateLink( const QDSActionRequest& request );
    void requestLinks( const QDSActionRequest& request );
    void peerPublishRequest(const QString& provider);

    // for returns from 'external lookup' requests.
    void updateEditDetails(const QContact &);

private:
    AddressbookWindow *parent;
};

class ContactsPhoneService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class AddressbookWindow;
private:
    ContactsPhoneService( AddressbookWindow *parent )
        : QtopiaAbstractService( "ContactsPhone", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ContactsPhoneService();

public slots:
#ifdef QTOPIA_CELL
    void smsBusinessCard();
    void pushVCard( const QDSActionRequest& request );
#endif

private:
    AddressbookWindow *parent;
};
#endif
