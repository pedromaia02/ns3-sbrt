#ifndef BSQLEARNING_H
#define BSQLEARNING_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <sstream>
#include <stdint.h>
#include <fstream>
#include <string.h>

class BsQLearning
{
    protected:

    private:

    public:
        BsQLearning();
        virtual ~BsQLearning();

    int node;
    int n_states; //Number of states
    int n_actions; //Number of actions
    double target_tput;
    double aggregated_target_tput;
    float alpha;
    float e_policy; // e-greedy policy
    float gama;

    double **m;
    int current_state;
    int current_action;
    int old_state;
    double cost;
    std::string NS3_PATH;

    ///////////////// LTE //////////////////////
    double LteTput;
    uint32_t LteTxData; // (BYTES)
    double LteBaseTput;
    //////////////// WI-FI /////////////////////
    double WifiTput;
    uint32_t WifiTxData; // (BYTES)
    double WifiBaseTput;

    int GetNode();
    void SetNode(int n);
    void SetLteBaseTput(double x);
    void SetWifiBaseTput(double x);
    void SetNs3Path(std::string s);
    void LogTputStats(double Tput,std::string op);
    void StartMatrix();
    void ShowMatrix();
    void SaveApTxData(uint32_t newval);
    void ResetApTxData();
    void SaveEnbTxData(uint32_t newval);
    void ResetEnbTxData();
    double GetAction (int state, char option1,int option2);
    double GetMinAction (int state, char option);
    float RandomIntUniform (int min, int max);
    float RandomRealUniform ();
    int VerifyState (double T_put);
    double VerifyAction (int a);
    double CalcCost (double current_Tput);
    int ChooseAction();
    void UpdateQMatrix();
    void Run();

};

#endif // BSQLEARNING_H
