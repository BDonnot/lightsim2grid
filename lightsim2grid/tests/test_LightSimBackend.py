# Copyright (c) 2019-2020, RTE (https://www.rte-france.com)
# See AUTHORS.txt
# This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
# If a copy of the Mozilla Public License, version 2.0 was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.
# SPDX-License-Identifier: MPL-2.0
# This file is part of Grid2Op, Grid2Op a testbed platform to model sequential decision making in power systems.
import unittest

from grid2op.tests.helper_path_test import PATH_DATA_TEST_PP, PATH_DATA_TEST

from grid2op.tests.helper_path_test import HelperTests
from grid2op.tests.BaseBackendTest import BaseTestNames, BaseTestLoadingCase, BaseTestLoadingBackendFunc
from grid2op.tests.BaseBackendTest import BaseTestTopoAction, BaseTestEnvPerformsCorrectCascadingFailures
from grid2op.tests.BaseBackendTest import BaseTestChangeBusAffectRightBus, BaseTestShuntAction
from grid2op.tests.BaseBackendTest import BaseTestResetEqualsLoadGrid, BaseTestVoltageOWhenDisco, BaseTestChangeBusSlack
from grid2op.tests.BaseBackendTest import BaseIssuesTest, BaseStatusActions

PATH_DATA_TEST_INIT = PATH_DATA_TEST
PATH_DATA_TEST = PATH_DATA_TEST_PP
from lightsim2grid.LightSimBackend import LightSimBackend


class TestNames(HelperTests, BaseTestNames):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)

    def get_path(self):
        return PATH_DATA_TEST_INIT


class TestLoadingCase(HelperTests, BaseTestLoadingCase):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)

    def get_path(self):
        return PATH_DATA_TEST

    def get_casefile(self):
        return "test_case14.json"


class TestLoadingBackendFunc(HelperTests, BaseTestLoadingBackendFunc):
    def setUp(self):
        # TODO find something more elegant
        BaseTestLoadingBackendFunc.setUp(self)
        self.tests_skipped = set()

        # lightsim does not support DC powerflow at the moment
        self.tests_skipped.add("test_pf_ac_dc")
        self.tests_skipped.add("test_apply_action_active_value")
        self.tests_skipped.add("test_runpf_dc")

    def tearDown(self):
        # TODO find something more elegant
        BaseTestLoadingBackendFunc.tearDown(self)

    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)

    def get_path(self):
        return PATH_DATA_TEST

    def get_casefile(self):
        return "test_case14.json"


class TestTopoAction(HelperTests, BaseTestTopoAction):
    def setUp(self):
        BaseTestTopoAction.setUp(self)

    def tearDown(self):
        # TODO find something more elegant
        BaseTestTopoAction.tearDown(self)

    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)

    def get_path(self):
        return PATH_DATA_TEST

    def get_casefile(self):
        return "test_case14.json"


class TestEnvPerformsCorrectCascadingFailures(HelperTests, BaseTestEnvPerformsCorrectCascadingFailures):
    def setUp(self):
        BaseTestEnvPerformsCorrectCascadingFailures.setUp(self)

    def tearDown(self):
        # TODO find something more elegant
        BaseTestEnvPerformsCorrectCascadingFailures.tearDown(self)

    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)

    def get_casefile(self):
        return "test_case14.json"

    def get_path(self):
        return PATH_DATA_TEST


class TestChangeBusAffectRightBus(HelperTests, BaseTestChangeBusAffectRightBus):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestShuntAction(HelperTests, BaseTestShuntAction):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestResetEqualsLoadGrid(HelperTests, BaseTestResetEqualsLoadGrid):
    def setUp(self):
        BaseTestResetEqualsLoadGrid.setUp(self)

    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestVoltageOWhenDisco(HelperTests, BaseTestVoltageOWhenDisco):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestChangeBusSlack(HelperTests, BaseTestChangeBusSlack):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestIssuesTest(HelperTests, BaseIssuesTest):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


class TestStatusAction(HelperTests, BaseStatusActions):
    def make_backend(self, detailed_infos_for_cascading_failures=False):
        return LightSimBackend(detailed_infos_for_cascading_failures=detailed_infos_for_cascading_failures)


if __name__ == "__main__":
    unittest.main()