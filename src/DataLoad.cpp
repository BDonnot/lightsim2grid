// Copyright (c) 2020, RTE (https://www.rte-france.com)
// See AUTHORS.txt
// This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
// If a copy of the Mozilla Public License, version 2.0 was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// SPDX-License-Identifier: MPL-2.0
// This file is part of LightSim2grid, LightSim2grid implements a c++ backend targeting the Grid2Op platform.

#include "DataLoad.h"
void DataLoad::init(const Eigen::VectorXd & loads_p,
                    const Eigen::VectorXd & loads_q,
                    const Eigen::VectorXi & loads_bus_id)
{
    p_mw_ = loads_p;
    q_mvar_ = loads_q;
    bus_id_ = loads_bus_id;
    status_ = std::vector<bool>(loads_p.size(), true);
}


DataLoad::StateRes DataLoad::get_state() const
{
     std::vector<double> p_mw(p_mw_.begin(), p_mw_.end());
     std::vector<double> q_mvar(q_mvar_.begin(), q_mvar_.end());
     std::vector<int> bus_id(bus_id_.begin(), bus_id_.end());
     std::vector<bool> status = status_;
     DataLoad::StateRes res(p_mw, q_mvar, bus_id, status);
     return res;
}
void DataLoad::set_state(DataLoad::StateRes & my_state )
{
    reset_results();

    std::vector<double> & p_mw = std::get<0>(my_state);
    std::vector<double> & q_mvar = std::get<1>(my_state);
    std::vector<int> & bus_id = std::get<2>(my_state);
    std::vector<bool> & status = std::get<3>(my_state);
    // TODO check sizes

    // input data
    p_mw_ = Eigen::VectorXd::Map(&p_mw[0], p_mw.size());
    q_mvar_ = Eigen::VectorXd::Map(&q_mvar[0], q_mvar.size());
    bus_id_ = Eigen::VectorXi::Map(&bus_id[0], bus_id.size());
    status_ = status;
}


void DataLoad::fillSbus(Eigen::VectorXcd & Sbus, bool ac, const std::vector<int> & id_grid_to_solver){
    int nb_load = nb();
    int bus_id_me, bus_id_solver;
    cdouble tmp;
    for(int load_id = 0; load_id < nb_load; ++load_id){
        //  i don't do anything if the load is disconnected
        if(!status_[load_id]) continue;

        bus_id_me = bus_id_(load_id);
        bus_id_solver = id_grid_to_solver[bus_id_me];
        if(bus_id_solver == _deactivated_bus_id){
            //TODO improve error message with the gen_id
            throw std::runtime_error("One load is connected to a disconnected bus.");
        }
        tmp = static_cast<cdouble>(p_mw_(load_id));
        if(ac) tmp += my_i * q_mvar_(load_id);
        Sbus.coeffRef(bus_id_solver) -= tmp;
    }
}

void DataLoad::compute_results(const Eigen::Ref<Eigen::VectorXd> & Va,
                               const Eigen::Ref<Eigen::VectorXd> & Vm,
                               const Eigen::Ref<Eigen::VectorXcd> & V,
                               const std::vector<int> & id_grid_to_solver,
                               const Eigen::VectorXd & bus_vn_kv)
{
    int nb_load = nb();
    v_kv_from_vpu(Va, Vm, status_, nb_load, bus_id_, id_grid_to_solver, bus_vn_kv, res_v_);
    res_p_ = p_mw_;
    res_q_ = q_mvar_;
}

void DataLoad::reset_results(){
    res_p_ = Eigen::VectorXd();  // in MW
    res_q_ =  Eigen::VectorXd();  // in MVar
    res_v_ = Eigen::VectorXd();  // in kV
}

void DataLoad::change_p(int load_id, double new_p, bool & need_reset)
{
    bool my_status = status_.at(load_id); // and this check that load_id is not out of bound
    if(!my_status) throw std::runtime_error("Impossible to change the active value of a disconnected load");
    p_mw_(load_id) = new_p;
}

void DataLoad::change_q(int load_id, double new_q, bool & need_reset)
{
    bool my_status = status_.at(load_id); // and this check that load_id is not out of bound
    if(!my_status) throw std::runtime_error("Impossible to change the reactive value of a disconnected load");
    q_mvar_(load_id) = new_q;
}

double DataLoad::get_p_slack(int slack_bus_id)
{
    int nb_element = nb();
    double res = 0.;
    for(int load_id = 0; load_id < nb_element; ++load_id)
    {
        if(!status_[load_id]) continue;
        if(bus_id_(load_id) == slack_bus_id) res += res_p_(load_id);
    }
    return res;
}

void DataLoad::get_q(std::vector<double>& q_by_bus)
{
    int nb_element = nb();
    for(int load_id = 0; load_id < nb_element; ++load_id)
    {
        if(!status_[load_id]) continue;
        int bus_id = bus_id_[load_id];
        q_by_bus[bus_id] += res_q_(load_id); //TODO weird that i need to put a + here and a - for the active!
    }
}
