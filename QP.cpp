#include "QP.h"

using namespace std;

Eigen::SparseMatrix<double> Q;
Eigen::VectorXd b_x;
Eigen::VectorXd b_y;

void create_sparse_matrix_star() {

    unordered_map<NET*, unsigned int> nets_idx_to_star_node_idx;
    unsigned int Q_matrix_size = modules.size();
    for(unordered_map<string, NET*>::iterator net_it = nets.begin(); net_it != nets.end(); net_it++) {
        if((net_it->second->module_num >= 2) && (net_it->second->pins.size() > 2)) { //add star node whenever net is connected to more than 1 module and more than 2 pin
            nets_idx_to_star_node_idx[net_it->second] = Q_matrix_size;
            Q_matrix_size++;
        }
    }

    vector<Eigen::Triplet<double>> triplets;
    b_x.resize(Q_matrix_size);
    b_x.setZero();
    b_y.resize(Q_matrix_size);
    b_y.setZero();
    Q.resize(Q_matrix_size, Q_matrix_size);
    Q.setZero();

    for(unordered_map<string, NET*>::iterator net_it = nets.begin(); net_it != nets.end(); net_it++) {

        NET   *net_ptr      = net_it->second;
        double net_weight   = 1.0;

        if((net_ptr->module_num >= 2) && (net_ptr->pins.size() > 2)) {

            unsigned int star_node_idx = nets_idx_to_star_node_idx[net_ptr];
            net_weight                 = 1.0 / net_ptr->pins.size();

            for(int pin_cnt = 0; pin_cnt < net_ptr->pins.size(); pin_cnt++) {

                MODULE   *module_ptr   = net_ptr->pins[pin_cnt]->module;
                TERMINAL *terminal_ptr = net_ptr->pins[pin_cnt]->terminal;

                triplets.emplace_back(star_node_idx, star_node_idx, (net_weight)*2.0);

                if(module_ptr != nullptr) {

                    unsigned int  module_id = module_ptr->id;
                    triplets.emplace_back(module_id, module_id, (net_weight) * 2.0);
                    triplets.emplace_back(module_id, star_node_idx, -1 * (net_weight) * 2.0);
                    triplets.emplace_back(star_node_idx, module_id, -1 * (net_weight) * 2.0);

                } else {

                    b_x[star_node_idx] -= 2 * net_weight * (terminal_ptr->coordinate.x);
                    b_y[star_node_idx] -= 2 * net_weight * (terminal_ptr->coordinate.y);
                } 
            }
        } else if((net_ptr->module_num == 2) && (net_ptr->pins.size() == 2)) { 
            net_weight               = 1.0;

            unsigned int module_id_1 = net_ptr->pins[0]->module->id;
            unsigned int module_id_2 = net_ptr->pins[1]->module->id;

            triplets.emplace_back(module_id_1, module_id_1, (net_weight) * 2.0);
            triplets.emplace_back(module_id_2, module_id_2, (net_weight) * 2.0);
            triplets.emplace_back(module_id_1, module_id_2, -1 * (net_weight) * 2.0);
            triplets.emplace_back(module_id_2, module_id_1, -1 * (net_weight) * 2.0);

        } else if((net_ptr->module_num == 1) && (net_ptr->pins.size() >= 2)) {
            net_weight                       = 1.0/(net_ptr->pins.size()-1);

            unsigned int terminal_num        = net_ptr->terminal_num;
            FPOS         terminal_coordinate = FPOS();
            unsigned int module_id           = 0;

            for(int pin_cnt = 0; pin_cnt < net_ptr->pins.size(); pin_cnt++) {

                MODULE   *module_ptr   = net_ptr->pins[pin_cnt]->module;
                TERMINAL *terminal_ptr = net_ptr->pins[pin_cnt]->terminal;

                if(module_ptr != nullptr) {
                    module_id = module_ptr->id;
                } else {
                    terminal_coordinate = terminal_coordinate + terminal_ptr->coordinate;
                }
            }
            triplets.emplace_back(module_id, module_id, terminal_num * (net_weight)  *2.0);
            b_x[module_id] -= 2 * net_weight * (terminal_coordinate.x);
            b_y[module_id] -= 2 * net_weight * (terminal_coordinate.y);

        } else {
            continue;
        }
    }
    Q.setFromTriplets(triplets.begin(), triplets.end(), std::plus<>());
    return;
}

void create_sparse_matrix_clique() {

    unsigned int Q_matrix_size = modules.size();

    vector<Eigen::Triplet<double>> triplets;
    b_x.resize(Q_matrix_size);
    b_x.setZero();
    b_y.resize(Q_matrix_size);
    b_y.setZero();
    Q.resize(Q_matrix_size, Q_matrix_size);
    Q.setZero();

    for(unsigned int net_cnt1 = 0; net_cnt1 < nets_vec.size(); net_cnt1++) {
        
        NET* net_ptr      = nets_vec[net_cnt1];
        double net_weight = 1.0 / (double)net_ptr->pins.size();

        for(unsigned int pin_cnt1 = 0; pin_cnt1 < net_ptr->pins.size(); pin_cnt1++) {

            PIN* pin_ptr1 = net_ptr->pins[pin_cnt1];

            for(unsigned int pin_cnt2 = pin_cnt1 + 1; pin_cnt2 < net_ptr->pins.size(); pin_cnt2++) {

                PIN* pin_ptr2 = net_ptr->pins[pin_cnt2];

                if(pin_ptr1->terminal != nullptr && pin_ptr2->terminal != nullptr) {
                    continue;
                } else if(pin_ptr1->terminal != nullptr && pin_ptr2->terminal == nullptr) {

                    TERMINAL* terminal_ptr = pin_ptr1->terminal;
                    unsigned int module_id = pin_ptr2->module->id;

                    triplets.emplace_back(module_id, module_id, (net_weight) * 2.0);

                    b_x[module_id] -= 2.0 * net_weight * (terminal_ptr->coordinate.x);
                    b_y[module_id] -= 2.0 * net_weight * (terminal_ptr->coordinate.y);

                } else if(pin_ptr1->terminal == nullptr && pin_ptr2->terminal != nullptr) {
                    
                    TERMINAL* terminal_ptr = pin_ptr2->terminal;
                    unsigned int module_id = pin_ptr1->module->id;

                    triplets.emplace_back(module_id, module_id, (net_weight) * 2.0);

                    b_x[module_id] -= 2.0 * net_weight * (terminal_ptr->coordinate.x);
                    b_y[module_id] -= 2.0 * net_weight * (terminal_ptr->coordinate.y);

                } else {
                    
                    unsigned int module_id_1 = pin_ptr1->module->id;
                    unsigned int module_id_2 = pin_ptr2->module->id;

                    triplets.emplace_back(module_id_1, module_id_1, (net_weight) * 2.0);
                    triplets.emplace_back(module_id_2, module_id_2, (net_weight) * 2.0);
                    triplets.emplace_back(module_id_1, module_id_2, -1 * (net_weight) * 2.0);
                    triplets.emplace_back(module_id_2, module_id_1, -1 * (net_weight) * 2.0);

                }
            }
        }
    }
    Q.setFromTriplets(triplets.begin(), triplets.end(), std::plus<>());
    return;
}

void solve_directly() {
    Eigen::VectorXd x_result, y_result;

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
    solver.compute(Q);

    if (solver.info() != Eigen::Success) {
        cerr << "Decomposition failed!" << endl;
        return;
    }

    x_result = solver.solve(-b_x);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Solving for x failed!" << endl;
    }

    y_result = solver.solve(-b_y);
    if (solver.info() != Eigen::Success) {
        cerr << "Solving for y failed!" << endl;
    }

    for(unordered_map<string, MODULE*>::iterator module_it = modules.begin(); module_it != modules.end(); module_it++) {
        MODULE *module_ptr = module_it->second;
        module_ptr->coordinate.ct = FPOS(x_result[module_ptr->id], y_result[module_ptr->id]);
        module_ptr->coordinate.update_lb_ur(module_ptr->dimension);
    }
    return;
}

void solve_cg() {
    Eigen::VectorXd x_result, y_result;

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> solver;
    solver.compute(Q); 

    if (solver.info() != Eigen::Success) {
        cerr << "CG decomposition failed!" << endl;
        return;
    }

    x_result = solver.solve(-b_x);
    if (solver.info() != Eigen::Success) {
        cerr << "CG solving for x failed!" << endl;
    }

    y_result = solver.solve(-b_y);
    if (solver.info() != Eigen::Success) {
        cerr << "CG solving for y failed!" << endl;
    }

    for (auto &module_pair : modules) {
        MODULE *module_ptr = module_pair.second;
        module_ptr->coordinate.ct = FPOS(x_result[module_ptr->id], y_result[module_ptr->id]);
        module_ptr->coordinate.update_lb_ur(module_ptr->dimension);
    }
}

void print_dense_Q() {
    Eigen::MatrixXd Q_dense = Eigen::MatrixXd(Q); 
    std::cout << "Dense Q matrix:" << std::endl;
    std::cout << Q_dense << std::endl;
}
void print_b_vectors() {
    std::cout << "b_x:\n";
    for (int i = 0; i < b_x.size(); ++i) {
        std::cout << "b_x[" << i << "] = " << b_x[i] << "\n";
    }

    std::cout << "\nb_y:\n";
    for (int i = 0; i < b_y.size(); ++i) {
        std::cout << "b_y[" << i << "] = " << b_y[i] << "\n";
    }
}