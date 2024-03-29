# Copyright (c) 2019-2023, RTE (https://www.rte-france.com)
# See AUTHORS.txt
# This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
# If a copy of the Mozilla Public License, version 2.0 was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.
# SPDX-License-Identifier: MPL-2.0
# This file is part of Grid2Op, Grid2Op a testbed platform to model sequential decision making in power systems.

import warnings
import unittest
import numpy as np

try:
    import grid2op
    from grid2op.Action import PlayableAction
    from grid2op.Environment import Environment
    from grid2op.Runner import Runner
    GLOP_AVAIL = True
except ImportError:
    GLOP_AVAIL = False

try:
    from grid2op.gym_compat import GymEnv, BoxGymActSpace, BoxGymObsSpace, DiscreteActSpace, MultiDiscreteActSpace
    GYM_AVAIL = True
except ImportError as exc_:
    print(exc_)
    GYM_AVAIL = False
    
from lightsim2grid import LightSimBackend
            
            
class TestEnvironmentBasic(unittest.TestCase):          
    def setUp(self) -> None:
        if not GLOP_AVAIL:
            self.skipTest("grid2op not installed")
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
            self.env = grid2op.make("educ_case14_storage",
                                    test=True,
                                    action_class=PlayableAction,
                                    _add_to_name=type(self).__name__,
                                    backend=LightSimBackend())
        self.line_id = 3
        th_lim = self.env.get_thermal_limit() * 2.  # avoid all problem in general
        th_lim[self.line_id] /= 10.  # make sure to get trouble in line 3
        self.env.set_thermal_limit(th_lim)
        
        TestEnvironmentBasic._init_env(self.env)
        
    @staticmethod  
    def _init_env(env):
        env.set_id(0)
        env.seed(0)
        env.reset()
        
    def tearDown(self) -> None:
        self.env.close()
        return super().tearDown()
    
    def test_right_type(self):
        assert isinstance(self.env, Environment)
        
    def test_ok(self):
        act = self.env.action_space()
        for i in range(10):
            obs_in, reward, done, info = self.env.step(act)
            if i < 3:
                assert obs_in.timestep_overflow[self.line_id] == i + 1, f"error for step {i}: {obs_in.timestep_overflow[self.line_id]}"
            else:
                assert not obs_in.line_status[self.line_id]
    
    def test_reset(self):
        # timestep_overflow should be 0 initially even if the flow is too high
        obs = self.env.reset()
        assert obs.timestep_overflow[self.line_id] == 0
        assert obs.rho[self.line_id] > 1.
        

class TestEnvironmentBasicCpy(TestEnvironmentBasic):
    def setUp(self) -> None:
        super().setUp()
        init_int = self.env
        self.env = self.env.copy()
        init_int.close()
        

class TestBasicEnvironmentRunner(unittest.TestCase):    
    def setUp(self) -> None:
        TestEnvironmentBasic.setUp(self)
        self.max_iter = 10

    def tearDown(self) -> None:
        self.env.close()
        return super().tearDown()
        
    def test_runner_can_make(self):
        runner = Runner(**self.env.get_params_for_runner())
        env2 = runner.init_env()
        assert isinstance(env2, Environment)
    
    def test_runner(self):
        # create the runner
        runner_in = Runner(**self.env.get_params_for_runner())
        res_in, *_ = runner_in.run(nb_episode=1, max_iter=self.max_iter, env_seeds=[0], episode_id=[0], add_detailed_output=True)
        res_in2, *_ = runner_in.run(nb_episode=1, max_iter=self.max_iter, env_seeds=[0], episode_id=[0])
        # check correct results are obtained when agregated
        assert res_in[3] == 10
        assert res_in2[3] == 10
        ref_val = 164.1740722
        assert np.allclose(res_in[2], ref_val), f"{res_in[2]} vs {ref_val}"
        assert np.allclose(res_in2[2], ref_val), f"{res_in2[2]} vs {ref_val}"
        
        # check detailed results
        ep_data_in = res_in[-1]
        for i in range(1, self.max_iter + 1):
            # there is a bug in grid2op 1.6.4 for i = 0...
            obs_in = ep_data_in.observations[i]
            assert obs_in is not None, f"error for step {i}"
            if i < 4:
                assert obs_in.timestep_overflow[self.line_id] == i, f"error for step {i}: {obs_in.timestep_overflow[self.line_id]}"
            else:
                assert not obs_in.line_status[self.line_id], f"error for step {i}: line is not disconnected"
    
        
class TestBasicEnvironmentGym(unittest.TestCase):
    def setUp(self) -> None:
        if not GYM_AVAIL:
            self.skipTest("gym is not installed (gymnasium did not exist in 1.6.4)")
        TestEnvironmentBasic.setUp(self)

    def tearDown(self) -> None:
        self.env.close()
        return super().tearDown()
    
    def _aux_run_envs(self, act, env_gym):
        for i in range(10):
            obs_in, reward, done, info = env_gym.step(act)
            if i < 3:
                assert obs_in["timestep_overflow"][self.line_id] == i + 1, f"error for step {i}: {obs_in['timestep_overflow'][self.line_id]}"
            else:
                assert not obs_in["line_status"][self.line_id]
    
    def test_gym_with_step(self):
        """test the step function also disconnects (or not) the lines"""
        env_gym = GymEnv(self.env)
        act = {}
        self._aux_run_envs(act, env_gym)
        env_gym.reset()
        self._aux_run_envs(act, env_gym)
            
    def test_gym_normal(self):
        """test I can create the gym env"""
        env_gym = GymEnv(self.env)
        env_gym.reset()
    
    def test_gym_box(self):
        """test I can create the gym env with box ob space and act space"""
        env_gym = GymEnv(self.env)
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
            env_gym.action_space = BoxGymActSpace(self.env.action_space)
            env_gym.observation_space = BoxGymObsSpace(self.env.observation_space)
        env_gym.reset()
    
    def test_gym_discrete(self):
        """test I can create the gym env with discrete act space"""
        env_gym = GymEnv(self.env)
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
            env_gym.action_space = DiscreteActSpace(self.env.action_space)
        env_gym.reset()
        act = 0
        self._aux_run_envs(act, env_gym)
        
    def test_gym_multidiscrete(self):
        """test I can create the gym env with multi discrete act space"""
        env_gym = GymEnv(self.env)
        with warnings.catch_warnings():
            warnings.filterwarnings("ignore")
            env_gym.action_space = MultiDiscreteActSpace(self.env.action_space)
        env_gym.reset()
        act = env_gym.action_space.sample()
        act[:] = 0
        self._aux_run_envs(act, env_gym)


if __name__ == "__main__":
    unittest.main()
