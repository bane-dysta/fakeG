#include "structures.h"

namespace fakeg {
namespace data {

ElementMap::ElementMap() {
    initElementMap();
}

void ElementMap::initElementMap() {
    // Period 1
    elementMap["H"] = 1; elementMap["He"] = 2;
    
    // Period 2
    elementMap["Li"] = 3; elementMap["Be"] = 4; elementMap["B"] = 5; 
    elementMap["C"] = 6; elementMap["N"] = 7; elementMap["O"] = 8; 
    elementMap["F"] = 9; elementMap["Ne"] = 10;
    
    // Period 3
    elementMap["Na"] = 11; elementMap["Mg"] = 12; elementMap["Al"] = 13; 
    elementMap["Si"] = 14; elementMap["P"] = 15; elementMap["S"] = 16; 
    elementMap["Cl"] = 17; elementMap["Ar"] = 18;
    
    // Period 4
    elementMap["K"] = 19; elementMap["Ca"] = 20;
    elementMap["Sc"] = 21; elementMap["Ti"] = 22; elementMap["V"] = 23; 
    elementMap["Cr"] = 24; elementMap["Mn"] = 25; elementMap["Fe"] = 26; 
    elementMap["Co"] = 27; elementMap["Ni"] = 28; elementMap["Cu"] = 29; 
    elementMap["Zn"] = 30;
    elementMap["Ga"] = 31; elementMap["Ge"] = 32; elementMap["As"] = 33; 
    elementMap["Se"] = 34; elementMap["Br"] = 35; elementMap["Kr"] = 36;
    
    // Period 5
    elementMap["Rb"] = 37; elementMap["Sr"] = 38; elementMap["Y"] = 39; 
    elementMap["Zr"] = 40; elementMap["Nb"] = 41; elementMap["Mo"] = 42; 
    elementMap["Tc"] = 43; elementMap["Ru"] = 44; elementMap["Rh"] = 45; 
    elementMap["Pd"] = 46; elementMap["Ag"] = 47; elementMap["Cd"] = 48;
    elementMap["In"] = 49; elementMap["Sn"] = 50; elementMap["Sb"] = 51; 
    elementMap["Te"] = 52; elementMap["I"] = 53; elementMap["Xe"] = 54;
    
    // Period 6
    elementMap["Cs"] = 55; elementMap["Ba"] = 56; elementMap["La"] = 57;
    // Lanthanides
    elementMap["Ce"] = 58; elementMap["Pr"] = 59; elementMap["Nd"] = 60; 
    elementMap["Pm"] = 61; elementMap["Sm"] = 62; elementMap["Eu"] = 63; 
    elementMap["Gd"] = 64; elementMap["Tb"] = 65; elementMap["Dy"] = 66; 
    elementMap["Ho"] = 67; elementMap["Er"] = 68; elementMap["Tm"] = 69; 
    elementMap["Yb"] = 70; elementMap["Lu"] = 71;
    // Period 6 continuation
    elementMap["Hf"] = 72; elementMap["Ta"] = 73; elementMap["W"] = 74; 
    elementMap["Re"] = 75; elementMap["Os"] = 76; elementMap["Ir"] = 77; 
    elementMap["Pt"] = 78; elementMap["Au"] = 79; elementMap["Hg"] = 80;
    elementMap["Tl"] = 81; elementMap["Pb"] = 82; elementMap["Bi"] = 83; 
    elementMap["Po"] = 84; elementMap["At"] = 85; elementMap["Rn"] = 86;
    
    // Period 7
    elementMap["Fr"] = 87; elementMap["Ra"] = 88; elementMap["Ac"] = 89;
    // Actinides
    elementMap["Th"] = 90; elementMap["Pa"] = 91; elementMap["U"] = 92; 
    elementMap["Np"] = 93; elementMap["Pu"] = 94; elementMap["Am"] = 95; 
    elementMap["Cm"] = 96; elementMap["Bk"] = 97; elementMap["Cf"] = 98; 
    elementMap["Es"] = 99; elementMap["Fm"] = 100; elementMap["Md"] = 101; 
    elementMap["No"] = 102; elementMap["Lr"] = 103;
    // Period 7 continuation
    elementMap["Rf"] = 104; elementMap["Db"] = 105; elementMap["Sg"] = 106; 
    elementMap["Bh"] = 107; elementMap["Hs"] = 108; elementMap["Mt"] = 109; 
    elementMap["Ds"] = 110; elementMap["Rg"] = 111; elementMap["Cn"] = 112; 
    elementMap["Nh"] = 113; elementMap["Fl"] = 114; elementMap["Mc"] = 115; 
    elementMap["Lv"] = 116; elementMap["Ts"] = 117; elementMap["Og"] = 118;
}

int ElementMap::getAtomicNumber(const std::string& symbol) const {
    auto it = elementMap.find(symbol);
    return (it != elementMap.end()) ? it->second : 1; // 默认为氢原子
}

bool ElementMap::hasElement(const std::string& symbol) const {
    return elementMap.find(symbol) != elementMap.end();
}

} // namespace data
} // namespace fakeg 