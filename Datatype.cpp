////////////////////////////////////////////////////
// * brief
// * LAB       NCKU_SEDA 112
// * date      2024/7/11 update
// * version   1.0.0

// * author    PY , EasyMoney
////////////////////////////////////////////////////

#include "Datatype.h"
using namespace std;

//..................component information....................//

unordered_map<string, TERMINAL*> terminals;
unordered_map<string, MODULE*>   modules;
unordered_map<string, NET*>      nets;

vector<MODULE*>                  modules_vec;
vector<NET*>                     nets_vec;

vector<vector<BIN*>>             bins_vec;

double total_module_area = 0.0;

vector<RECT>                     best_result;

//..................component information....................//

//....................chip information.......................//

FPOS   outline_lb = FPOS(), outline_ur = FPOS(); 

double outline_H  = 0.0,    outline_W  = 0.0;

double white_space_ratio     = 0.15;

double outline_aspect_ratio  = 1.0;

FPOS   chip_ur               = FPOS();

double average_realistic_density = 0.0;

//....................chip information.......................//

//................parallization information..................//

int    thread_num = 40;

//................parallization information..................//


// ............... Data Structure Function ................. //
void FPOS::SetZero() {
    x = y = 0;
}

void FPOS::Min(FPOS a) {
    x = min(x, a.x);
    y = min(y, a.y);
}
void FPOS::SetMin(FPOS a, FPOS b) {
    x = min(a.x, b.x);
    y = min(a.y, b.y);
}

void FPOS::Max(FPOS a) {
    x = max(x, a.x);
    y = max(y, a.y);
}
void FPOS::SetMax(FPOS a, FPOS b) {
    x = max(a.x, b.x);
    y = max(a.y, b.y);
}

void FPOS::Center(FPOS lb, FPOS W_H) {
    x = lb.x + (W_H.x / 2.);
    y = lb.y + (W_H.y / 2.);
}

double FPOS::Cal_area(FPOS ur) {
    return (ur.x - x) * (ur.y - y);
}
double FPOS::Cal_area() {
    return x * y;
}

double FPOS::Cal_dis(FPOS b) {
    return fabs(x - b.x) + fabs(y - b.y);
}

double FPOS::GetProduct() {
    return x * y;
}

void FPOS::Swap() {
    double temp = x;
    x = y;
    y = temp;
}

FPOS::~FPOS() {}

double RECT::Cal_area() {
    return (ur.x - lb.x) * (ur.y - lb.y);
}

void RECT::update_lb_ur(FPOS dimension) {
    lb = ct - dimension/2.0;
    ur = ct + dimension/2.0;
}

RECT::~RECT() {}


// ..... MODULE ..... //

MODULE::MODULE() { 
    name = "";
    id   = 0;

    hard_soft  = 0;

    fixed      = 0;

    coordinate = RECT();

    dimension  = FPOS(); 

    area = 0,
    AR_L = 1,
    AR_U = 1;

    nets.clear();
    pins.clear();

    static_power = 0;
}

MODULE::MODULE(string Name, unsigned int Id, bool Hard_soft, FPOS Dimension, double Area, double aR_L, double aR_U) { 
    name = Name;
    id   = Id;

    hard_soft  = Hard_soft;

    fixed      = false;

    coordinate = RECT();

    dimension  = Dimension; 

    area = Area,
    AR_L = aR_L,
    AR_U = aR_U;

    nets.clear();
    pins.clear();

    static_power = 0;
}

MODULE::~MODULE() {}

// .......TERMINAL.....//

TERMINAL::TERMINAL() {
    name = "";
    id   = 0;
    
    coordinate = FPOS();

    nets.clear();
    pins.clear();
}

TERMINAL::TERMINAL(string Name, unsigned int Id) {
    name = Name;
    id   = Id;

    coordinate = FPOS();

    nets.clear();
    pins.clear();
}

TERMINAL::~TERMINAL() {}

// ..... NET ..... //

NET::NET() {
    name = "";
    id   = 0;

    net_weight = 1;

    pins.clear();

    WL = 0;
    ExpSum_P = FPOS();
    ExpSum_N = FPOS();
    ExpMultSum_P = FPOS();
    ExpMultSum_N = FPOS();

    module_num   = 0;
    terminal_num = 0;
}

NET::NET(string Name, unsigned int Id) {
    name = Name;
    id   = Id;

    net_weight = 1;

    pins.clear();

    WL = 0;
    ExpSum_P = FPOS();
    ExpSum_N = FPOS();
    ExpMultSum_P = FPOS();
    ExpMultSum_N = FPOS();

    module_num   = 0;
    terminal_num = 0;
}

NET::~NET() {}

// ..... PIN ..... //

PIN::PIN() {
    id = 0;

    terminal = nullptr; 
    module   = nullptr;
    net      = nullptr;

    offset = FPOS();
}

PIN::PIN(unsigned int Id, NET *net_ptr) {
    id = Id;

    terminal = nullptr; 
    module   = nullptr;
    net      = net_ptr;

    offset = FPOS();
}

PIN::~PIN() {}

BIN::BIN() {}

BIN::~BIN() {}
