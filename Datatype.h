////////////////////////////////////////////////////
// * brief
// * LAB       NCKU_SEDA 112
// * date      2024/7/11 update
// * version   1.0.0

// * author    PY , EasyMoney
////////////////////////////////////////////////////

#ifndef _DATATYPE_h_
#define _DATATYPE_h_

#include <algorithm>  //sort 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <omp.h> 
#include <unordered_map>


// - constant
#define PI    3.141592653589793238462L  //加L表示long double
#define SQRT2 1.414213562373095048801L
//K_sample：你要把原始波形（時域）切成幾個點來表示。
//K_freq：你要展開幾個頻率成分（傅立葉項數）來還原波形。

#define K_freq   256    // K_freq   is the number of transaction point of the Fourier series - namely the number of terms in the series
#define K_sample 256    // K_sample is the number of sample point of the outline range

using namespace std;

// ............... Data Structure Overveiw ................. //
class RECT;

class MODULE;

class NET;

class PIN;

class BIN;

struct FPOS {
public:
    double x = 0;
    double y = 0;

    FPOS() : x(0), y(0) {}
    FPOS(double xloc, double yloc) : x(xloc), y(yloc) {}

    FPOS &operator=(const FPOS &f){  //加上const，表示這個函式不會改變傳入的參數
        //&表示這個函式會返回一個對FPOS物件的引用，這樣可以直接修改原來的物件，而不是返回一個新的物件
        //這樣可以避免不必要的複製，提升效率

        //如果傳入的參數不是這個物件本身，就把傳入的參數的值賦給這個物件
        //這樣可以避免不必要的複製，提升效率
        if (this != &f)             //[this] 是指到目前再操作的FPOS instance的指標 
        {
            this->x = f.x;
            this->y = f.y;
        }
        return *this;
    }

    FPOS operator+(const FPOS &f) const{    //?????
        return FPOS(this->x + f.x, this->y + f.y);
    }

    FPOS operator-(const FPOS &f) const{
        return FPOS(this->x - f.x, this->y - f.y);
    }

    FPOS operator*(const FPOS &f) const{
        return FPOS(this->x * f.x, this->y * f.y);
    }

    FPOS operator*(const double d) const{
        return FPOS(this->x / d, this->y / d);
    }

    FPOS operator/(const double d) const{
        return FPOS(this->x / d, this->y / d);
    }

    bool operator==(const FPOS &f) const{
        return fabs(this->x - f.x) < 0.0001 && fabs(this->y - f.y) < 0.0001; 
    }

    friend std::ostream &operator<<(std::ostream &os, const FPOS &p)
    {
        os << fixed << setprecision(3) << " (" << p.x << ", " << p.y << ") ";
        return os;
    }

    ~FPOS();

    void SetZero();

    void Min(FPOS a);
    void SetMin(FPOS a, FPOS b);
    void Max(FPOS a);
    void SetMax(FPOS a, FPOS b);
    double GetProduct();

    void Center(FPOS lb, FPOS W_H);

    double Cal_area(FPOS ur); 
    double Cal_area(); 

    double Cal_dis(FPOS b); 

    void Swap();
};

namespace std //表示這段程式碼是在標準命名空間 std 中進行的特化。
{
    template <> // 表示這是一個模板的完全特化版本，專門針對 FPOS 類型
    struct hash<FPOS>
    {
        size_t operator()(const FPOS &fpos) const
        {
            return hash<double>()(fpos.x) ^ (hash<double>()(fpos.y) << 1);
        }
    };
}


class RECT {
public:
    RECT() : ct(FPOS()), lb(FPOS()), ur(FPOS()) {}
    RECT(FPOS Ct) : ct(Ct) {}

    RECT &operator=(const RECT &r)
    {
        if (this != &r)
        {
            this->ct = r.ct;
            this->lb = r.lb;
            this->ur = r.ur;
        }
        return *this;
    }

    bool operator==(const RECT &r)
    {
        return (this->ct == r.ct && this->lb == r.lb && this->ur == r.ur);
    }
    
    friend std::ostream &operator<<(std::ostream &os, const RECT &p)
    {
        os << " <" << p.lb << ", " << p.ur << "> ";
        return os;
    }

    ~RECT();

    FPOS ct, lb, ur;

    double Cal_area();
    void   update_lb_ur(FPOS dimension);

};

// ..... Data Structure ..... //

class MODULE {
public:
    MODULE();
    MODULE(string Name, unsigned int Id, bool Hard_soft, FPOS Dimension, double Area, double aR_L, double aR_U);
    MODULE &operator=(const MODULE &);
    ~MODULE();

    string name;
    unsigned int id;

    bool   hard_soft;  

    bool   fixed;

    RECT   coordinate;
    FPOS   dimension; 
    double area;
    double AR_L,
           AR_U;

    // - connection
    vector<NET *> nets;
    vector<PIN *> pins;

    // - Power
    double static_power;

    FPOS total_grad, wl_grad, glo_d_grad, lg_d_grad, thermal_grad;
};

class TERMINAL {
public:
    TERMINAL();
    TERMINAL(string Name, unsigned int Id);
    ~TERMINAL();

    string name;
    unsigned int id;

    FPOS   coordinate;

    vector<NET *> nets;
    vector<PIN *> pins;
};

class NET {
public:
    NET();
    NET(string Name, unsigned int Id);
    NET &operator=(const NET &);
    ~NET();

    string name;
    unsigned int id;

    double net_weight;

    vector<PIN *> pins;

    // - wirelength
    double WL;
    FPOS ExpSum_P, ExpSum_N;         
    FPOS ExpMultSum_P, ExpMultSum_N; 

    unsigned int module_num;
    unsigned int terminal_num;

};

class PIN {
public:
    PIN();
    PIN(unsigned int Id, NET *net_ptr);
    PIN &operator=(const PIN &);
    ~PIN();

    unsigned int id;

    TERMINAL *terminal;
    MODULE   *module;
    NET      *net;

    FPOS offset;
};

class BIN {
public:
    BIN();
    BIN &operator=(const BIN &);
    ~BIN();
    
    pair<unsigned int, unsigned int> index; 

    FPOS center, lb, ur;
    FPOS W_H;

    double area         = 0.0;

    double static_power = 0.0;

    double temperature  = 0.0;
};

//..................component information....................//

extern unordered_map<string, TERMINAL*> terminals;
extern unordered_map<string, MODULE*>   modules;
extern unordered_map<string, NET*>      nets;

extern vector<MODULE*>                  modules_vec;
extern vector<NET*>                     nets_vec;

extern vector<vector<BIN*>>             bins_vec;

extern double total_module_area;

extern vector<RECT>                     best_result;

//..................component information....................//

//....................chip information.......................//

extern FPOS   outline_lb, outline_ur; 

extern double outline_H,  outline_W;

extern double white_space_ratio;

extern double outline_aspect_ratio;

extern FPOS   chip_ur; //? the range of the chip is usually bigger than floorplanning range(fixed-outline)

extern double average_realistic_density;

//....................chip information.......................//

//................parallization information..................//

extern int    thread_num;

//................parallization information..................//

#endif