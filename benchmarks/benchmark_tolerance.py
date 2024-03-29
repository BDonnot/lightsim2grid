# Copyright (c) 2020, RTE (https://www.rte-france.com)
# See AUTHORS.txt
# This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
# If a copy of the Mozilla Public License, version 2.0 was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.
# SPDX-License-Identifier: MPL-2.0
# This file is part of LightSim2grid, LightSim2grid a implements a c++ backend targeting the Grid2Op platform.

import numpy as np
import os

from grid2op import make
from grid2op.Agent import DoNothingAgent
try:
    from grid2op.Chronics import GridStateFromFileWithForecastsWithoutMaintenance as GridStateFromFile
except ImportError:
    print("Be carefull: there might be maintenance")
    from grid2op.Chronics import GridStateFromFile

from grid2op.Parameters import Parameters
import lightsim2grid
from lightsim2grid.lightSimBackend import LightSimBackend
from utils_benchmark import print_res, run_env, str2bool, get_env_name_displayed
TABULATE_AVAIL = False
try:
    from tabulate import tabulate
    TABULATE_AVAIL = True
except ImportError:
    print("The tabluate package is not installed. Some output might not work properly")

import pdb

MAX_TS = 1000
ENV_NAME = "rte_case14_realistic"


def main(max_ts, ENV_NAME, test=True):
    param = Parameters()
    param.init_from_dict({"NO_OVERFLOW_DISCONNECTION": True})

    env_pp = make(ENV_NAME, param=param, test=test,
                   data_feeding_kwargs={"gridvalueClass": GridStateFromFile})
    agent = DoNothingAgent(action_space=env_pp.action_space)
    nb_ts_pp, time_pp, aor_pp, gen_p_pp, gen_q_pp = run_env(env_pp, max_ts, agent, chron_id=0, env_seed=0)
    pp_time_pf = env_pp._time_powerflow
    wst = False  # print extra info in the run_env function

    env_lightsim = make(ENV_NAME, backend=LightSimBackend(), param=param, test=test,
                        data_feeding_kwargs={"gridvalueClass": GridStateFromFile})
    li_tols = [10., 1., 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9]

    nb_ts = []
    time = []
    aor = []
    gen_p = []
    gen_q = []
    comp_time = []
    time_pf = []
    for tol in li_tols:
        env_lightsim.backend.set_tol(tol)
        nb_ts_, time_, aor_, gen_p_, gen_q_ = run_env(env_lightsim, max_ts, agent, chron_id=0,
                                                      with_type_solver=wst, env_seed=0)
        comp_time_ = env_lightsim.backend.comp_time
        time_pf_ = env_lightsim._time_powerflow
        nb_ts.append(nb_ts_)
        time.append(time_)
        aor.append(aor_)
        gen_p.append(gen_p_)
        gen_q.append(gen_q_)
        comp_time.append(comp_time_)
        time_pf.append(time_pf_)

    # NOW PRINT THE RESULTS
    env_name = get_env_name_displayed(ENV_NAME)
    hds = [f"{env_name} ({nb_ts_pp} iter)", f"speed (it/s)", f"Δ aor (amps)", f"Δ gen_p (MW)", f"Δ gen_q (MVAr)"]
    tab = [["PP", int(nb_ts_pp/time_pp), "0.00", "0.00", "0.00"]]
    for i, tol in enumerate(li_tols):
        if lightsim2grid.SolverType.GaussSeidel:
            tab.append([f"{tol:.2e}",
                        f"{int(nb_ts[i] / time[i])}",
                        f"{np.max(np.abs(aor[i] - aor_pp)):.2e}",
                        f"{np.max(np.abs(gen_p[i] - gen_p_pp)):.2e}",
                        f"{np.max(np.abs(gen_q[i] - gen_q_pp)):.2e}"])

    res_tol = tabulate(tab, headers=hds,  tablefmt="rst")
    print(res_tol)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Benchmark of lightsim with a "do nothing" agent '
                                                 '(compare multiple lightsim solvers with default grid2op backend '
                                                 'PandaPower)')
    parser.add_argument('--name', default=ENV_NAME, type=str,
                        help='Environment name to be used for the benchmark.')
    parser.add_argument('--number', type=int, default=MAX_TS,
                        help='Maximum number of time steps for which the benchmark will be run.')
    parser.add_argument('--no_test', type=str2bool, nargs='?',
                        const=True, default=False,
                        help='Do not use test environment for the benchmark (default False: use test environment)')

    args = parser.parse_args()

    max_ts = int(args.number)
    name = str(args.name)
    test_env = not args.no_test
    main(max_ts, name, test_env)
