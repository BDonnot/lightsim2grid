#ifndef GRIDMODEL_H
#define GRIDMODEL_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <cstdint> // for int32
#include <chrono>
#include <complex>      // std::complex, std::conj
#include <cmath>  // for PI

// eigen is necessary to easily pass data from numpy to c++ without any copy.
// and to optimize the matrix operations
#include "Eigen/Core"
#include "Eigen/Dense"
#include "Eigen/SparseCore"
#include "Eigen/SparseLU"

// import data classes
#include "Utils.h"
#include "DataGeneric.h"
#include "DataLine.h"
#include "DataShunt.h"
#include "DataTrafo.h"
#include "DataLoad.h"
#include "DataGen.h"


// import klu solver
#include "KLUSolver.h"

//TODO implement a BFS check to make sure the Ymatrix is "connected" [one single component]
class GridModel : public DataGeneric
{
    public:
        GridModel():need_reset_(true){};

        // All methods to init this data model, all need to be pair unit when applicable
        void init_bus(const Eigen::VectorXd & bus_vn_kv, int nb_line, int nb_trafo);

        void init_powerlines(const Eigen::VectorXd & branch_r,
                             const Eigen::VectorXd & branch_x,
                             const Eigen::VectorXcd & branch_h,
                             const Eigen::VectorXi & branch_from_id,
                             const Eigen::VectorXi & branch_to_id
                             ){
            powerlines_.init(branch_r, branch_x, branch_h, branch_from_id, branch_to_id);
        }
        void init_shunt(const Eigen::VectorXd & shunt_p_mw,
                        const Eigen::VectorXd & shunt_q_mvar,
                        const Eigen::VectorXi & shunt_bus_id){
            shunts_.init(shunt_p_mw, shunt_q_mvar, shunt_bus_id);
        }
        void init_trafo(const Eigen::VectorXd & trafo_r,
                        const Eigen::VectorXd & trafo_x,
                        const Eigen::VectorXcd & trafo_b,
                        const Eigen::VectorXd & trafo_tap_step_pct,
//                        const Eigen::VectorXd & trafo_tap_step_degree,  //TODO handle that too!
                        const Eigen::VectorXd & trafo_tap_pos,
                        const Eigen::Vector<bool, Eigen::Dynamic> & trafo_tap_hv,  // is tap on high voltage (true) or low voltate
                        const Eigen::VectorXi & trafo_hv_id,
                        const Eigen::VectorXi & trafo_lv_id
                        ){
            trafos_.init(trafo_r, trafo_x, trafo_b, trafo_tap_step_pct, trafo_tap_pos, trafo_tap_hv, trafo_hv_id, trafo_lv_id);
        }
        void init_generators(const Eigen::VectorXd & generators_p,
                             const Eigen::VectorXd & generators_v,
                             const Eigen::VectorXi & generators_bus_id){
            generators_.init(generators_p, generators_v, generators_bus_id);
        }
        void init_loads(const Eigen::VectorXd & loads_p,
                        const Eigen::VectorXd & loads_q,
                        const Eigen::VectorXi & loads_bus_id){
            loads_.init(loads_p, loads_q, loads_bus_id);
        }

        void add_slackbus(int slack_bus_id){
            slack_bus_id_ = slack_bus_id;
        }

        //powerflows
        // dc powerflow
        Eigen::VectorXcd dc_pf(const Eigen::VectorXcd & Vinit,
                               int max_iter,  // not used for DC
                               double tol  // not used for DC
                               );

        // ac powerflow
        Eigen::VectorXcd ac_pf(const Eigen::VectorXcd & Vinit,
                               int max_iter,
                               double tol);


        // deactivate a bus. Be careful, if a bus is deactivated, but an element is
        //still connected to it, it will throw an exception
        void deactivate_bus(int bus_id) {_deactivate(bus_id, bus_status_, need_reset_); }
        // if a bus is connected, but isolated, it will make the powerflow diverge
        void reactivate_bus(int bus_id) {_reactivate(bus_id, bus_status_, need_reset_); }

        //deactivate a powerline (disconnect it)
        void deactivate_powerline(int powerline_id) {powerlines_.deactivate(powerline_id, need_reset_); }
        void reactivate_powerline(int powerline_id) {powerlines_.reactivate(powerline_id, need_reset_); }
        void change_bus_powerline_or(int powerline_id, int new_bus_id) {powerlines_.change_bus_or(powerline_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }
        void change_bus_powerline_ex(int powerline_id, int new_bus_id) {powerlines_.change_bus_ex(powerline_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }

        //deactivate trafo
        void deactivate_trafo(int trafo_id) {trafos_.deactivate(trafo_id, need_reset_); }
        void reactivate_trafo(int trafo_id) {trafos_.reactivate(trafo_id, need_reset_); }
        void change_bus_trafo_hv(int trafo_id, int new_bus_id) {trafos_.change_bus_hv(trafo_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }
        void change_bus_trafo_lv(int trafo_id, int new_bus_id) {trafos_.change_bus_lv(trafo_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }

        //load
        void deactivate_load(int load_id) {loads_.deactivate(load_id, need_reset_); }
        void reactivate_load(int load_id) {loads_.reactivate(load_id, need_reset_); }
        void change_bus_load(int load_id, int new_bus_id) {loads_.change_bus(load_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }
        void change_p_load(int load_id, double new_p) {loads_.change_p(load_id, new_p, need_reset_); }
        void change_q_load(int load_id, double new_q) {loads_.change_q(load_id, new_q, need_reset_); }

        //generator
        void deactivate_gen(int gen_id) {generators_.deactivate(gen_id, need_reset_); }
        void reactivate_gen(int gen_id) {generators_.reactivate(gen_id, need_reset_); }
        void change_bus_gen(int gen_id, int new_bus_id) {generators_.change_bus(gen_id, new_bus_id, need_reset_, bus_vn_kv_.size()); }
        void change_p_gen(int gen_id, double new_p) {generators_.change_p(gen_id, new_p, need_reset_); }
        void change_v_gen(int gen_id, double new_v_pu) {generators_.change_v(gen_id, new_v_pu, need_reset_); }

        //shunt
        void deactivate_shunt(int shunt_id) {shunts_.deactivate(shunt_id, need_reset_); }
        void reactivate_shunt(int shunt_id) {shunts_.reactivate(shunt_id, need_reset_); }
        void change_bus_shunt(int shunt_id, int new_bus_id) {shunts_.change_bus(shunt_id, new_bus_id, need_reset_, bus_vn_kv_.size());  }
        void change_p_shunt(int shunt_id, double new_p) {shunts_.change_p(shunt_id, new_p, need_reset_); }
        void change_q_shunt(int shunt_id, double new_q) {shunts_.change_q(shunt_id, new_q, need_reset_); }

        // All results access
        tuple3d get_loads_res() const {return loads_.get_res();}
        tuple3d get_shunts_res() const {return shunts_.get_res();}
        tuple3d get_gen_res() const {return generators_.get_res();}
        tuple4d get_lineor_res() const {return powerlines_.get_lineor_res();}
        tuple4d get_lineex_res() const {return powerlines_.get_lineex_res();}
        tuple4d get_trafohv_res() const {return trafos_.get_res_hv();}
        tuple4d get_trafolv_res() const {return trafos_.get_res_lv();}

        // get some internal information, be cerafull the ID of the buses might not be the same
        // TODO convert it back to this ID, that will make copies, but who really cares ?
        Eigen::SparseMatrix<cdouble> get_Ybus(){
            return Ybus_;
        }
        Eigen::VectorXcd get_Sbus(){
            return Sbus_;
        }
        Eigen::VectorXi get_pv(){
            return bus_pv_;
        }
        Eigen::VectorXi get_pq(){
            return bus_pq_;
        }
        Eigen::Ref<Eigen::VectorXd> get_Va(){
            return _solver.get_Va();
        }
        Eigen::Ref<Eigen::VectorXd> get_Vm(){
            return _solver.get_Vm();
        }
        Eigen::SparseMatrix<double> get_J(){
            return _solver.get_J();
        }

    protected:
    // add method to change topology, change ratio of transformers, change

        // compute admittance matrix
        // dc powerflow
        // void init_dcY(Eigen::SparseMatrix<double> & dcYbus);

        // ac powerflows
        void init_Ybus(Eigen::SparseMatrix<cdouble> & Ybus, Eigen::VectorXcd & Sbus,
                       std::vector<int>& id_me_to_solver, std::vector<int>& id_solver_to_me,
                       int & slack_bus_id_solver);
        void fillYbus(Eigen::SparseMatrix<cdouble> & res, bool ac, const std::vector<int>& id_me_to_solver);
        void fillSbus(Eigen::VectorXcd & res, bool ac, const std::vector<int>& id_me_to_solver, int slack_bus_id_solver);
        void fillpv_pq(const std::vector<int>& id_me_to_solver);

        // results
        /**
        Compute the results vector from the Va, Vm post powerflow
        **/
        void compute_results();
        /**
        reset the results in case of divergence of the powerflow.
        **/
        void reset_results();

    protected:
        // member of the grid
        // static const int _deactivated_bus_id;
        bool need_reset_;

        // powersystem representation
        // 1. bus
        Eigen::VectorXd bus_vn_kv_;
        std::vector<bool> bus_status_;  //TODO that is not handled at the moment

        // always have the length of the number of buses,
        // id_me_to_model_[id_me] gives -1 if the bus "id_me" is deactivated, or "id_model" if it is activated.
        std::vector<int> id_me_to_solver_;
        // convert the bus id from the model to the bus id of me.
        // it has a variable size, that depends on the number of connected bus. if "id_model" is an id of a bus
        // sent to the solver, then id_model_to_me_[id_model] is the bus id of this model of the grid.
        std::vector<int> id_solver_to_me_;

        // 2. powerline
        DataLine powerlines_;

        // 3. shunt
        DataShunt shunts_;

        // 4. transformers
        // have the r, x, h and ratio
        // ratio is computed from the tap, so maybe store tap num and tap_step_pct
        DataTrafo trafos_;

        // 5. generators
        DataGen generators_;

        // 6. loads
        DataLoad loads_;

        // 7. slack bus
        int slack_bus_id_;
        int slack_bus_id_solver_;

        // as matrix, for the solver
        Eigen::SparseMatrix<cdouble> Ybus_;
        Eigen::VectorXcd Sbus_;
        Eigen::VectorXi bus_pv_;  // id are the solver internal id and NOT the initial id
        Eigen::VectorXi bus_pq_;  // id are the solver internal id and NOT the initial id

        // TODO have version of the stuff above for the public api, indexed with "me" and not "solver"

        // to solve the newton raphson
        KLUSolver _solver;

};

#endif  //GRIDMODEL_H