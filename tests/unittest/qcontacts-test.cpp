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

#include "base-client-test.h"
#include "common/source.h"
#include "common/dbus-service-defs.h"
#include "common/vcard-parser.h"

#include <QObject>
#include <QtDBus>
#include <QtTest>
#include <QDebug>
#include <QtContacts>

#include "config.h"

using namespace QtContacts;

class QContactsTest : public BaseClientTest
{
    Q_OBJECT
private:
    QContactManager *m_manager;

    QContact testContact()
    {
        // create a contact
        QContact contact;

        QContactName name;
        name.setFirstName("Fulano");
        name.setMiddleName("de");
        name.setLastName("Tal");
        contact.saveDetail(&name);

        QContactEmailAddress email;
        email.setEmailAddress("fulano@email.com");
        contact.saveDetail(&email);

        return contact;
    }

private Q_SLOTS:
    void initTestCase()
    {
        BaseClientTest::initTestCase();
        QCoreApplication::setLibraryPaths(QStringList() << QT_PLUGINS_BINARY_DIR);
    }

    void init()
    {
        BaseClientTest::init();
        m_manager = new QContactManager("galera");
    }

    void cleanup()
    {
        delete m_manager;
        BaseClientTest::cleanup();
    }

    void testAvailableManager()
    {
        QVERIFY(QContactManager::availableManagers().contains("galera"));
    }

    /*
     * Test create a new contact
     */
    void testCreateContact()
    {
        // filter all contacts
        QContactFilter filter;

        // check result, must be empty
        QList<QContact> contacts = m_manager->contacts(filter);
        QCOMPARE(contacts.size(), 0);

        // create a contact
        QContact contact = testContact();
        QSignalSpy spyContactAdded(m_manager, SIGNAL(contactsAdded(QList<QContactId>)));
        bool result = m_manager->saveContact(&contact);
        QCOMPARE(result, true);
        QTRY_COMPARE(spyContactAdded.count(), 1);

        // query for new contacts
        contacts = m_manager->contacts(filter);
        QCOMPARE(contacts.size(), 1);

        QContact createdContact = contacts[0];
        // id
        QVERIFY(!createdContact.id().isNull());

        // email
        QContactEmailAddress email = contact.detail<QContactEmailAddress>();
        QContactEmailAddress createdEmail = createdContact.detail<QContactEmailAddress>();
        QCOMPARE(createdEmail.emailAddress(), email.emailAddress());

        // name
        QContactName name = contact.detail<QContactName>();
        QContactName createdName = createdContact.detail<QContactName>();
        QCOMPARE(createdName.firstName(), name.firstName());
        QCOMPARE(createdName.middleName(), name.middleName());
        QCOMPARE(createdName.lastName(), name.lastName());

        QContactSyncTarget target = contact.detail<QContactSyncTarget>();
        QCOMPARE(target.syncTarget(), QString("Dummy personas"));
    }
#if 0
    /*
     * Test create a new contact
     */
    void testUpdateContact()
    {
        // filter all contacts
        QContactFilter filter;

        // create a contact
        QContact contact = testContact();
        QSignalSpy spyContactAdded(m_manager, SIGNAL(contactsAdded(QList<QContactId>)));
        bool result = m_manager->saveContact(&contact);
        QCOMPARE(result, true);
        QTRY_COMPARE(spyContactAdded.count(), 1);

        QContactName name = contact.detail<QContactName>();
        name.setMiddleName("da");
        name.setLastName("Silva");
        contact.saveDetail(&name);

        QSignalSpy spyContactChanged(m_manager, SIGNAL(contactsChanged(QList<QContactId>)));
        result = m_manager->saveContact(&contact);
        QCOMPARE(result, true);
        QTRY_COMPARE(spyContactChanged.count(), 1);

        // query for the contacts
        QList<QContact> contacts = m_manager->contacts(filter);
        QCOMPARE(contacts.size(), 1);
        QContact updatedContact = contacts[0];

        // name
        QContactName updatedName = updatedContact.detail<QContactName>();
        QCOMPARE(updatedName.firstName(), name.firstName());
        QCOMPARE(updatedName.middleName(), name.middleName());
        QCOMPARE(updatedName.lastName(), name.lastName());
    }

    /*
     * Test create a contact source using the contact group
     */
    void testCreateGroup()
    {
        QContactManager manager("galera");

        // create a contact
        QContact contact;
        contact.setType(QContactType::TypeGroup);

        QContactDisplayLabel label;
        label.setLabel("test group");
        contact.saveDetail(&label);

        bool result = manager.saveContact(&contact);
        QCOMPARE(result, true);

        // query for new contacts
        QContactDetailFilter filter;
        filter.setDetailType(QContactDetail::TypeType, QContactType::FieldType);
        filter.setValue(QContactType::TypeGroup);
        QList<QContact> contacts = manager.contacts(filter);

        // will be two sources since we have the main source already created
        QCOMPARE(contacts.size(), 2);

        QContact createdContact = contacts[0];
        // id
        QVERIFY(!createdContact.id().isNull());

        // label
        QContactDisplayLabel createdlabel = createdContact.detail<QContactDisplayLabel>();
        QCOMPARE(createdlabel.label(), label.label());
    }

    /*
     * Test query a contact source using the contact group
     */
    void testQueryGroups()
    {
        QContactManager manager("galera");

        // filter all contact groups/addressbook
        QContactDetailFilter filter;
        filter.setDetailType(QContactDetail::TypeType, QContactType::FieldType);
        filter.setValue(QContactType::TypeGroup);

        // check result
        QList<QContact> contacts = manager.contacts(filter);
        QCOMPARE(contacts.size(), 1);
        QCOMPARE(contacts[0].id().toString(), QStringLiteral("qtcontacts:galera::dummy-store"));
        QCOMPARE(contacts[0].type(), QContactType::TypeGroup);

        QContactDisplayLabel label = contacts[0].detail(QContactDisplayLabel::Type);
        QCOMPARE(label.label(), QStringLiteral("Dummy personas"));
    }
#endif
};

QTEST_MAIN(QContactsTest)

#include "qcontacts-test.moc"
