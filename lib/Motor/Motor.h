#pragma once
#include <string>
using namespace std;

class Motor {
    public:
        void motorOff();
        void step(int nDirection);
        void setup();
        void oneTurnCW();
        void oneTurnCCW();
        void nTurnsCW(float turns); 
        void nTurnsCCW(float turns); 

    private:
        void write(int a,int b,int c,int d);
};