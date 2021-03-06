/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of contact-service-app.
 *
 * contact-service-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * contact-service-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GALERA_ADDRESSBOOK_H__
#define __GALERA_ADDRESSBOOK_H__

#include "common/source.h"

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSettings>

#include <QtDBus/QtDBus>

#include <QtContacts/QContact>

#include <folks/folks.h>
#include <glib.h>
#include <glib-object.h>

typedef struct _MessagingMenuMessage MessagingMenuMessage;
typedef struct _MessagingMenuApp MessagingMenuApp;
typedef struct _ESource ESource;
typedef struct _ESourceRegistry ESourceRegistry;

namespace galera
{
class View;
class ContactsMap;
class AddressBookAdaptor;
class QIndividual;
class DirtyContactsNotify;

class AddressBook: public QObject
{
    Q_OBJECT
public:
    AddressBook(QObject *parent=0);
    virtual ~AddressBook();

    static QString objectPath();
    bool start(QDBusConnection connection);

    // Adaptor
    QString linkContacts(const QStringList &contacts);
    View *query(const QString &clause, const QString &sort, int maxCount, bool showInvisible, const QStringList &sources);
    QStringList sortFields();
    bool unlinkContacts(const QString &parent, const QStringList &contacts);
    bool isReady() const;
    void setSafeMode(bool flag);

    static bool isSafeMode();
    static int init();

Q_SIGNALS:
    void stopped();
    void readyChanged();
    void safeModeChanged();
    void sourcesChanged();

public Q_SLOTS:
    bool start();
    void shutdown();
    SourceList availableSources(const QDBusMessage &message);
    Source source(const QDBusMessage &message);
    Source createSource(const QString &sourceName, uint accountId, bool setAsPrimary, const QDBusMessage &message);
    SourceList updateSources(const SourceList &sources, const QDBusMessage &message);
    void removeSource(const QString &sourceId, const QDBusMessage &message);
    QString createContact(const QString &contact, const QString &source, const QDBusMessage &message = QDBusMessage());
    int removeContacts(const QStringList &contactIds, const QDBusMessage &message);
    QStringList updateContacts(const QStringList &contacts, const QDBusMessage &message);
    void purgeContacts(const QDateTime &since, const QString &sourceId, const QDBusMessage &message);
    void updateContactsDone(const QString &contactId, const QString &error);

private Q_SLOTS:
    void viewClosed();
    void individualChanged(QIndividual *individual);
    void onEdsServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void onSafeModeChanged();

    // Unix signal handlers.
    void handleSigQuit();

    // WORKAROUND: Check if EDS was running when the service started
    void checkForEds();
    void unprepareFolks();

    // check compatibility and if the safe mode should be enabled
    void checkCompatibility();

private:
    FolksIndividualAggregator *m_individualAggregator;
    ContactsMap *m_contacts;
    QSet<View*> m_views;
    AddressBookAdaptor *m_adaptor;
    // timer to avoid send several updates at the same time
    DirtyContactsNotify *m_notifyContactUpdate;
    QDBusServiceWatcher *m_edsWatcher;
    MessagingMenuApp *m_messagingMenu;
    MessagingMenuMessage *m_messagingMenuMessage;
    ESourceRegistry *m_sourceRegistryListener;
    static QSettings m_settings;

    bool m_edsIsLive;
    bool m_ready;
    bool m_isAboutToQuit;
    bool m_isAboutToReload;
    gulong m_individualsChangedDetailedId;
    gulong m_notifyIsQuiescentHandlerId;
    QDBusConnection m_connection;

    // Update command
    QMutex m_updateLock;
    QDBusMessage m_updateCommandReplyMessage;
    QStringList m_updateCommandResult;
    QStringList m_updatedIds;
    QStringList m_updateCommandPendingContacts;

    // Unix signals
    static int m_sigQuitFd[2];
    QSocketNotifier *m_snQuit;

    // dbus service name
    QString m_serviceName;

    // Disable copy contructor
    AddressBook(const AddressBook&);

    void getSource(const QDBusMessage &message, bool onlyTheDefault);

    void setupUnixSignals();

    // Unix signal handlers.
    void prepareUnixSignals();
    static void quitSignalHandler(int unused);

    bool processUpdates();
    void prepareFolks();
    void unprepareEds();
    void connectWithEDS();
    void continueShutdown();
    void setIsReady(bool isReady);
    bool registerObject(QDBusConnection &connection);
    QString removeContact(FolksIndividual *individual, bool *visible);
    QString addContact(FolksIndividual *individual, bool visible);
    FolksPersonaStore *getFolksStore(const QString &source);

    static void availableSourcesDoneListAllSources(FolksBackendStore *backendStore,
                                                   GAsyncResult *res,
                                                   QDBusMessage *msg);
    static void availableSourcesDoneListDefaultSource(FolksBackendStore *backendStore,
                                                      GAsyncResult *res,
                                                      QDBusMessage *msg);
    static SourceList availableSourcesDoneImpl(FolksBackendStore *backendStore,
                                               GAsyncResult *res);
    static void individualsChangedCb(FolksIndividualAggregator *individualAggregator,
                                     GeeMultiMap *changes,
                                     AddressBook *self);
    static void isQuiescentChanged(GObject *source,
                                   GParamSpec *param,
                                   AddressBook *self);
    static void prepareFolksDone(GObject *source,
                                 GAsyncResult *res,
                                 AddressBook *self);
    static void createContactDone(FolksIndividualAggregator *individualAggregator,
                                  GAsyncResult *res,
                                  void *data);
    static void removeContactDone(FolksIndividualAggregator *individualAggregator,
                                  GAsyncResult *result,
                                  void *data);
    static void createSourceDone(GObject *source,
                                 GAsyncResult *res,
                                 void *data);
    static void removeSourceDone(GObject *source,
                                 GAsyncResult *res,
                                 void *data);
    static void folksUnprepared(GObject *source,
                               GAsyncResult *res,
                               void *data);
    static void edsUnprepared(GObject *source,
                              GAsyncResult *res,
                              void *data);
    static void edsPrepared(GObject *source,
                            GAsyncResult *res,
                            void *data);
    static void onSafeModeMessageActivated(MessagingMenuMessage *message,
                                           const char *actionId,
                                           GVariant *param,
                                           AddressBook *self);

    //EDS helper
    void updateSourcesEDS(void *data);
    static Source parseEDSSource(ESourceRegistry *registry, ESource *eSource);
    static void edsRemoveContact(FolksIndividual *individual);
    static void updateSourceEDSDone(GObject *source,
                                    GAsyncResult *res,
                                    void *data);
    static void sourceEDSChanged(ESourceRegistry *registry,
                                 ESource *source,
                                 AddressBook *self);

    friend class DirtyContactsNotify;
};

} //namespace

#endif

