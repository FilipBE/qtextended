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
#ifndef QTOPIASERVICESELECTOR_H
#define QTOPIASERVICESELECTOR_H

#include <qdialog.h>
#include <qtopiaservices.h>

class QtopiaServiceDescriptionPrivate;
class QTOPIA_EXPORT QtopiaServiceDescription
{
public:
    QtopiaServiceDescription();
    QtopiaServiceDescription(const QtopiaServiceRequest& r, const QString& l, const QString& ic, const QVariantMap& p = QVariantMap());
    ~QtopiaServiceDescription();

    QtopiaServiceDescription(const QtopiaServiceDescription& other);
    QtopiaServiceDescription& operator=(const QtopiaServiceDescription& other);

    bool operator==(const QtopiaServiceDescription& other) const;
    bool isNull() const;

    QtopiaServiceRequest request() const;
    QString label() const;
    QString iconName() const;

    void setRequest(const QtopiaServiceRequest& r);
    void setLabel(const QString& l);
    void setIconName(const QString& i);

    QVariant optionalProperty(const QString& name) const;
    void setOptionalProperty(const QString& name, const QVariant &value);
    void removeOptionalProperty(const QString& name);

    QVariantMap optionalProperties() const;
    void setOptionalProperties(QVariantMap properties);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QtopiaServiceDescriptionPrivate* d;
};

Q_DECLARE_USER_METATYPE(QtopiaServiceDescription)

class QSmoothListWidgetItem;
class QTranslatableSettings;
class QLabel;
class QSmoothListWidget;

class QTOPIA_EXPORT QtopiaServiceSelector : public QDialog
{
    Q_OBJECT
public:
    explicit QtopiaServiceSelector(QWidget* parent);

    void addApplications();

    QtopiaServiceDescription descriptionFor(const QtopiaServiceRequest& req) const;

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent* e);

public slots:
    bool edit(const QString& targetlabel, QtopiaServiceDescription& item);

private slots:
    void selectAction(int a);
    void selectAction(QSmoothListWidgetItem *i);

private:
    QtopiaServiceDescription descFor(QSmoothListWidgetItem* item) const;
    void populateActionsList();
    void populateActionsList(const QString& srv, QTranslatableSettings &cfg);

    QLabel *label;
    QSmoothListWidget *actionlist;
    int selection;
};

#endif
