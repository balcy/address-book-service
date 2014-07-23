# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2014 Canonical
# Author: Omer Akram <omer.akram@canonical.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import subprocess
import sysconfig
import time

from fixtures import EnvironmentVariable, Fixture


def get_service_library_path():
    """Return path of address-book-service binary directory."""
    architecture = sysconfig.get_config_var('MULTIARCH')

    return os.path.join(
        '/usr/lib/',
        architecture,
        'address-book-service/')


class AddressBookServiceDummyBackend(Fixture):
    """Fixture to load test vcard for client applications

    Call the fixture without any paramter to load a default vcard

    :parameter vcard: call the fixture with a vcard to be used by
                      test application.

    """
    def __init__(self, vcard=None):
        self.contact_data = vcard

    def setUp(self):
        super(AddressBookServiceDummyBackend, self).setUp()
        self.useFixture(SetupEnvironmentVariables(self.contact_data))
        self.useFixture(RestartService())


class SetupEnvironmentVariables(Fixture):

    def __init__(self, vcard):
        self.vcard = vcard

    def setUp(self):
        super(SetupEnvironmentVariables, self).setUp()
        self._setup_environment()

    def _setup_environment(self):
        self.useFixture(EnvironmentVariable(
            'ALTERNATIVE_CPIM_SERVICE_NAME', 'com.canonical.test.pim'))
        self.useFixture(EnvironmentVariable(
            'FOLKS_BACKEND_PATH',
            os.path.join(get_service_library_path(), 'dummy.so')))
        self.useFixture(EnvironmentVariable('FOLKS_BACKENDS_ALLOWED', 'dummy'))
        self.useFixture(EnvironmentVariable('FOLKS_PRIMARY_STORE', 'dummy'))
        self.useFixture(EnvironmentVariable(
            'ADDRESS_BOOK_SERVICE_DEMO_DATA',
            self._get_vcard_location()))

    def _get_vcard_location(self):
        if self.vcard:
            return self.vcard

        local_location = os.path.abspath('vcard.vcf')
        bin_location = '/usr/share/address-book-service/data/vcard.vcf'
        if os.path.exists(local_location):
            return local_location
        elif os.path.exists(bin_location):
            return bin_location
        else:
            raise RuntimeError('No VCARD found.')


class RestartService(Fixture):

    def setUp(self):
        super(RestartService, self).setUp()
        self.addCleanup(self._kill_address_book_service)
        self._restart_address_book_service()

    def _kill_address_book_service(self):
        try:
            pid = subprocess.check_output(
                ['pidof', 'address-book-service']).strip()
            subprocess.call(['kill', '-3', pid])
        except subprocess.CalledProcessError:
            # Service not running, so do nothing.
            pass

    def _restart_address_book_service(self):
        path = os.path.join(
            get_service_library_path(), 'address-book-service')

        self._kill_address_book_service()
        subprocess.Popen([path])
        self._ensure_service_running()

    def _ensure_service_running(self):
        import dbus
        bus = dbus.SessionBus()
        proxy = bus.get_object(
            'com.canonical.pim', '/com/canonical/pim/AddressBook')

        for i in range(10):
            try:
                proxy.Introspect()
                break
            except dbus.exceptions.DBusException:
                time.sleep(1)
            else:
                raise RuntimeError('address-book-service never started.')
