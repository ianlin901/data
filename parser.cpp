#include "parser.h"
using namespace std;

void parser_in(string block_file, string net_file, string pl_file) {
    parser_block(block_file);
    cout << "block finish ........\n";
    parser_net(net_file);
    cout << "net finish ........\n";
    parser_pl(pl_file);
    cout << "pl finish ........\n";

    return;
}

void parser_block(string block_file) {
    ifstream infile(block_file);
    if (!infile.is_open()) {
        cerr << "Error opening file.\n";
    }

    string line;
    unsigned int numSoft = 0, numHard = 0, numTerm = 0;

    while (getline(infile, line)) {
        if (line.find("NumSoftRectangularBlocks") != string::npos) {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numSoft;
        } else if (line.find("NumHardRectilinearBlocks") != string::npos) {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numHard;
        } else if (line.find("NumTerminals") != string::npos) {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numTerm;
            break;  // done with header
        }
    }
    getline(infile, line);

    modules_vec.resize(numSoft + numHard);

    unsigned int softRead = 0;
    while (softRead < numSoft && getline(infile, line)) {
        istringstream iss(line);
        string name, type;
        double area, rMin, rMax;
        iss >> name >> type >> area >> rMin >> rMax;

        if (type == "softrectangular") {
            MODULE* module_ptr    = new MODULE(name, softRead, 1, FPOS(ceil(sqrt(area)), ceil(sqrt(area))), area, rMin, rMax);
            modules[name]         = module_ptr;
            modules_vec[softRead] = module_ptr; 
            ++softRead;
        }
    }
    if(softRead != 0) getline(infile, line);

    unsigned int hardRead = 0;
    while (hardRead < numHard && getline(infile, line)) {
        istringstream iss(line);
        string name, type, trash;
        char   trash_2;
        FPOS   lb,   ur;
        double power;
        iss >> name  >> type;
        iss >> trash >> trash >> trash >> trash >> trash;
        iss >> trash_2 >> ur.x >> trash_2 >> ur.y >> trash_2;

        if (type == "hardrectilinear") {
            MODULE* module_ptr               = new MODULE(name, softRead + hardRead, 1, FPOS(ur.x, ur.y), ur.x * ur.y, 1.0, 1.0);
            modules[name]                    = module_ptr;
            modules_vec[softRead + hardRead] = module_ptr; 
            ++hardRead;
        }
    }
    if(hardRead != 0) getline(infile, line);

    unsigned int termRead = 0;
    while (termRead < numTerm && getline(infile, line)) {
        istringstream iss(line);
        string name, type;
        iss >> name >> type;

        if (type == "terminal") {
            TERMINAL *terminal_ptr = new TERMINAL(name, termRead);
            terminals[name]        = terminal_ptr;
            ++termRead;
        }
    }
    return;
}

void parser_net(string net_file) {
    ifstream infile(net_file);
    if (!infile.is_open()) {
        cerr << "Error opening file.\n";
    }

    string line;
    unsigned int numNet, numPin; 

    while (getline(infile, line)) {
        if (line.find("NumNets") != string::npos) {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numNet;
        } else if (line.find("NumPins") != string::npos) {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numPin;
            break;
        } 
    }

    nets_vec.resize(numNet);

    unsigned int netread = 0;
    while (netread < numNet && getline(infile, line)) {
        istringstream iss(line);
        string trash;
        int    degree;
        iss >> trash >> trash >> degree;

        unsigned int pinread = 0;
        NET *net_ptr = new NET(to_string(netread), netread);
        while (pinread < degree && getline(infile, line)){
            istringstream iss(line);
            string module_terminal;
            iss >> module_terminal >> trash;

            PIN *pin_ptr = new PIN(pinread, net_ptr);

            if(modules.find(module_terminal) != modules.end()){
                MODULE *module_ptr = modules[module_terminal];
                pin_ptr->module = module_ptr;
                module_ptr->pins.push_back(pin_ptr);
                module_ptr->nets.push_back(net_ptr);

                net_ptr->module_num++;
            } else {
                TERMINAL *terminal_ptr = terminals[module_terminal];
                pin_ptr->terminal = terminal_ptr;
                terminal_ptr->pins.push_back(pin_ptr);
                terminal_ptr->nets.push_back(net_ptr);

                net_ptr->terminal_num++;
            } 
            net_ptr->pins.push_back(pin_ptr);
            pinread++;
        }
        nets[net_ptr->name] = net_ptr;
        nets_vec[netread]   = net_ptr;
        netread++;
    }
    return;
}

void parser_pl(string pl_file) {
    ifstream infile(pl_file);
    if (!infile.is_open()) {
        cerr << "Error opening file.\n";
    }

    string line;

    for(int j = 0; j < 5; j++)
        getline(infile, line);
    for(int i = 0; i < modules.size(); i++)
        getline(infile, line);
    getline(infile, line); 

    for(int i = 0; i < terminals.size(); i++) {
        getline(infile, line);
        istringstream iss(line);
        string name;
        double x, y;
        iss >> name >> x >> y;

        terminals[name]->coordinate = FPOS(x,y);

        chip_ur.x = (chip_ur.x < x) ? x : chip_ur.x;
        chip_ur.y = (chip_ur.y < y) ? y : chip_ur.y;
    }
    return;
}

void parser_matlab(string matlab_file) {
    ofstream outfile(matlab_file);

    outfile<<"axis equal;\n";
    outfile<<"hold on;\n";
    outfile<<"grid on;\n";

    for(unordered_map<string, MODULE*>::iterator module_it = modules.begin(); module_it != modules.end(); module_it++) {
        MODULE* module_ptr = module_it->second;
        double lb_x, lb_y, ur_x, ur_y;
        lb_x = module_ptr->coordinate.lb.x;
        lb_y = module_ptr->coordinate.lb.y;
        ur_x = module_ptr->coordinate.ur.x;
        ur_y = module_ptr->coordinate.ur.y;
        outfile<<"block_x=[ "
        <<lb_x<<" "
        <<ur_x<<" "
        <<ur_x<<" "
        <<lb_x<<" "
        <<lb_x<<" ];\n";
        outfile<<"block_y=[ "
        <<lb_y<<" "
        <<lb_y<<" "
        <<ur_y<<" "
        <<ur_y<<" "
        <<lb_y<<" ];\n";

        outfile<<"h = fill(block_x, block_y, 'b');"<<endl;
        outfile<<"set(h, 'FaceAlpha', 0.3);"<<endl;
        outfile<<"set(h, 'EdgeColor', 'k');"<<endl;
    }

    double min_x = 1e9, min_y = 1e9, max_x = 0, max_y = 0;
    for(unordered_map<string, TERMINAL*>::iterator terminal_it = terminals.begin(); terminal_it != terminals.end(); terminal_it++) {
        TERMINAL* terminal_ptr = terminal_it->second;
        double lb_x, lb_y, ur_x, ur_y;
        lb_x = terminal_ptr->coordinate.x-5;
        lb_y = terminal_ptr->coordinate.y-5;
        ur_x = terminal_ptr->coordinate.x+5;
        ur_y = terminal_ptr->coordinate.y+5;
        min_x = (min_x > lb_x) ? lb_x : min_x;
        min_y = (min_y > lb_y) ? lb_y : min_y;
        max_x = (max_x < ur_x) ? ur_x : max_x;
        max_y = (max_y < ur_y) ? ur_y : max_y;
        outfile<<"block_x=[ "
        <<lb_x<<" "
        <<ur_x<<" "
        <<ur_x<<" "
        <<lb_x<<" "
        <<lb_x<<" ];\n";
        outfile<<"block_y=[ "
        <<lb_y<<" "
        <<lb_y<<" "
        <<ur_y<<" "
        <<ur_y<<" "
        <<lb_y<<" ];\n";

        outfile<<"h = fill(block_x, block_y, 'r');"<<endl;
        outfile<<"set(h, 'FaceAlpha', 0.7);"<<endl;
        outfile<<"set(h, 'EdgeColor', 'k');"<<endl;
    }

    return;
}