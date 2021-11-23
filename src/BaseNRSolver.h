// Copyright (c) 2020, RTE (https://www.rte-france.com)
// See AUTHORS.txt
// This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
// If a copy of the Mozilla Public License, version 2.0 was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// SPDX-License-Identifier: MPL-2.0
// This file is part of LightSim2grid, LightSim2grid implements a c++ backend targeting the Grid2Op platform.

#ifndef BASENRSOLVER_H
#define BASENRSOLVER_H

#include "BaseSolver.h"

/**
Base class for Newton Raphson based solver
**/
class BaseNRSolver : public BaseSolver
{
    public:
        BaseNRSolver():need_factorize_(true), timer_initialize_(0.), timer_dSbus_(0.), timer_fillJ_(0.) {}

        ~BaseNRSolver(){}

        virtual
        Eigen::SparseMatrix<real_type> get_J(){
            return J_;
        }

        virtual
        std::tuple<double, double, double, double, double, double, double> get_timers_jacobian()
        {
            // TODO refacto that, and change the order
            auto res = std::tuple<double, double, double, double, double, double, double>(
              timer_Fx_, timer_solve_, timer_initialize_, timer_check_, timer_dSbus_, timer_fillJ_, timer_total_nr_);
            return res;
        }

        virtual
        bool compute_pf(const Eigen::SparseMatrix<cplx_type> & Ybus,
                        CplxVect & V,
                        const CplxVect & Sbus,
                        const Eigen::VectorXi & slack_ids,
                        const RealVect & slack_weights,
                        const Eigen::VectorXi & pv,
                        const Eigen::VectorXi & pq,
                        int max_iter,
                        real_type tol
                        ) ;

        virtual
        void reset();

    protected:
        virtual void reset_timer(){
            BaseSolver::reset_timer();
            timer_dSbus_ = 0.;
            timer_fillJ_ = 0.;
            timer_initialize_ = 0.;
        }
        virtual
        void initialize()=0;

        virtual
        void solve(RealVect & b, bool has_just_been_inialized)=0;

        void _dSbus_dV(const Eigen::Ref<const Eigen::SparseMatrix<cplx_type> > & Ybus,
                       const Eigen::Ref<const CplxVect > & V);

        void _get_values_J(int & nb_obj_this_col,
                           std::vector<int> & inner_index,
                           std::vector<real_type> & values,
                           const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & mat,  // ex. dS_dVa_r
                           const std::vector<int> & index_row_inv, // ex. pvpq_inv
                           const Eigen::VectorXi & index_col, // ex. pvpq
                           int col_id,
                           int row_lag,  // 0 for J11 for example, n_pvpq for J12
                           int col_lag
                           );
        void _get_values_J(int & nb_obj_this_col,
                           std::vector<int> & inner_index,
                           std::vector<real_type> & values,
                           const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & mat,  // ex. dS_dVa_r
                           const std::vector<int> & index_row_inv, // ex. pvpq_inv
                           int col_id_mat, // ex. pvpq(col_id)
                           int row_lag,  // 0 for J11 for example, n_pvpq for J12
                           int col_lag  // to remove the ref slack bus from this
                           );

        void fill_jacobian_matrix(const Eigen::SparseMatrix<cplx_type> & Ybus,
                                  const CplxVect & V,
                                  Eigen::Index slack_bus_id,
                                  const RealVect & slack_weights,
                                  const Eigen::VectorXi & pq,
                                  const Eigen::VectorXi & pvpq,
                                  const std::vector<int> & pq_inv,
                                  const std::vector<int> & pvpq_inv
                                  );
        void fill_jacobian_matrix_kown_sparsity_pattern(
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVa_r,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVa_i,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVm_r,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVm_i,
                 // const Eigen::SparseMatrix<cplx_type> & Ybus,
                 // const CplxVect & V,
                 Eigen::Index slack_bus_id,
                 const Eigen::VectorXi & pq,
                 const Eigen::VectorXi & pvpq //,
                //  const std::vector<int> & pq_inv,
                //  const std::vector<int> & pvpq_inv
                 );
        void fill_jacobian_matrix_unkown_sparsity_pattern(
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVa_r,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVa_i,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVm_r,
                 // const Eigen::Ref<const Eigen::SparseMatrix<real_type> > & dS_dVm_i,
                 const Eigen::SparseMatrix<cplx_type> & Ybus,
                 const CplxVect & V,
                 Eigen::Index slack_bus_id,
                 const RealVect & slack_weights,
                 const Eigen::VectorXi & pq,
                 const Eigen::VectorXi & pvpq,
                 const std::vector<int> & pq_inv,
                 const std::vector<int> & pvpq_inv
                 );

        void fill_value_map(Eigen::Index slack_bus_id,
                            const Eigen::VectorXi & pq,
                            const Eigen::VectorXi & pvpq);

    protected:

        // solution of the problem
        Eigen::SparseMatrix<real_type> J_;  // the jacobian matrix
        Eigen::SparseMatrix<cplx_type> dS_dVm_;
        Eigen::SparseMatrix<cplx_type> dS_dVa_;
        bool need_factorize_;

        // to store the mapping from the element of J_ in dS_dVm_ and dS_dVa_
        // it does not own any memory at all !
        std::vector<cplx_type*> value_map_;

        // timers
        double timer_initialize_;
        double timer_dSbus_;
        double timer_fillJ_;

    private:
        // no copy allowed
        BaseNRSolver( const BaseNRSolver & ) ;
        BaseNRSolver & operator=( const BaseNRSolver & ) ;

    private:
        Eigen::SparseMatrix<cplx_type>
            _make_diagonal_matrix(const Eigen::Ref<const CplxVect > & diag_val){
            // TODO their might be a more efficient way to do that
            auto n = diag_val.size();
            Eigen::SparseMatrix<cplx_type> res(n,n);
            // first method, without a loop of mine
            res.setIdentity();  // segfault if attempt to use this function without this
            res.diagonal() = diag_val;

            // second method, with an "optimized" loop
            // res.reserve(Eigen::VectorXi::Constant(n,1)); // i reserve one number per columns (speed optim)
            // for(unsigned int i = 0; i < n; ++i){
            //    res.insert(i,i) = diag_val(i);
            //}
            return res;
        }
};

#endif // BASENRSOLVER_H
