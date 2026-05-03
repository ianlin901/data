#ifndef _QP_h_
#define _QP_h_
// qp
#include <vector>
#include <iostream>
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "Eigen/SparseCholesky"

#include "Datatype.h"

using namespace std;

void create_sparse_matrix_clique();
void create_sparse_matrix_star();
void solve_directly();
void solve_cg();

void print_dense_Q();
void print_b_vectors();

extern Eigen::SparseMatrix<double> Q;
extern Eigen::VectorXd b_x;
extern Eigen::VectorXd b_y;

#endif
