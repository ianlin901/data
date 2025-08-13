#include <bits/stdc++.h>
using namespace std;

#include "parser.h"
#include "QP.h"

int main(int argc, char *argv[]) {
    
    parser_in(argv[1], argv[2], argv[3]);

    create_sparse_matrix_star();
    solve_directly();

    parser_matlab("result.m");
    

    return 0;
}