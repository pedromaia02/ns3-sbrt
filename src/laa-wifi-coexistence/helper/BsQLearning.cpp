#include "BsQLearning.h"
#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

BsQLearning::BsQLearning()
{
    n_states = 6;
    n_actions = 4;
    m = new double*[n_states];
    for ( int i = 0; i < n_states; ++i ){
        m[i] = new double[n_actions];
    }
    this->StartMatrix();

    target_tput = 100;
    aggregated_target_tput = 200;
    alpha = 0.5;
    e_policy = 0.03;
    gama = 0.9;
    current_state = 0;
    current_action = 0;
    old_state = 0;
    cost = 0;
    LteTput = 0;
    LteTxData = 0;
    WifiTput = 0;
    WifiTxData = 0;
    NS3_PATH = "/home/labpropsim-02/ns-3/ns3_sbrt";
}

BsQLearning::~BsQLearning()
{
    for ( int i = 0; i < n_states; ++i ){
        delete [] m[i];
        delete [] m;
    }
}

int BsQLearning::GetNode(){
    return node;
}

void BsQLearning::SetNode(int n){
    node = n;
}

void BsQLearning::SetNs3Path(std::string s){
    NS3_PATH = s;
}

void BsQLearning::LogTputStats(double Tput){
    std::string fileName = NS3_PATH + "Tput_Lte_";
    fileName += SSTR( node );
    char outFileName[1024];
    strcpy(outFileName, fileName.c_str());
    std::ofstream outFile;

    outFile.open (outFileName, std::ios_base::app);
    outFile << Tput << std::endl;
    outFile.close();
}

void BsQLearning::StartMatrix(){
    for (int i=0;i<n_states;i++){
        for (int j=0;j<n_actions;j++){
            m[i][j] = 0;
        }
    }
}

void BsQLearning::ShowMatrix(){
    for (int i=0;i<n_states;i++){
        std::cout << std::endl;
        for (int j=0;j<n_actions;j++){
            std::cout<< m[i][j] << " ";
        }
    }
}

void BsQLearning::SaveApTxData(uint32_t newval){
    WifiTxData += newval;
}

void BsQLearning::ResetApTxData(){
    WifiTxData = 0;
}

void BsQLearning::SaveEnbTxData(uint32_t newval){
    LteTxData += newval;
}

void BsQLearning::ResetEnbTxData(){
    LteTxData = 0;
}

double BsQLearning::GetMinAction (int state, char option){
	int i_min = 0;
	for (int i=0;i<n_actions;i++){
		if (m[state][i] < m[state][i_min]){
			i_min = i;
		}
	}
	if (option == 'a') { return i_min; }
	else if (option == 'v') { return m[state][i_min]; }
	else { std::cout << "Erro: No action found from GetMinAction!!" << std::endl; exit(1); }
}

float BsQLearning::RandomIntUniform (int v_min, int v_max){
	srand(time(0));
	return v_min + (rand() % static_cast<int>(v_max - v_min + 1));
}

float BsQLearning::RandomRealUniform (){
	srand(time(0));
	return rand() / (RAND_MAX + 1.);
}

int BsQLearning::VerifyState (double T_put){
	int state;
	if (T_put <= 10){ state = 0; }
	else if ( T_put > 10 && T_put <= 20) { state = 1; }
	else if ( T_put > 20 && T_put <= 30) { state = 2; }
	else if ( T_put > 30 && T_put <= 40) { state = 3; }
	else if ( T_put > 40 && T_put <= 50) { state = 4; }
	else { state = 5; }
	return state;
}

double BsQLearning::VerifyAction (int a){
	double action = 9999;
	switch (a) {
		case 0: action = 0.2;
			break;
		case 1: action = 0.4;
			break;
		case 2: action = 0.6;
			break;
		case 3: action = 0.8;
			break;
	}
	if (action == 9999){ std::cout << "Erro: No action selected from VerifyAction!!" << std::endl; exit(1); }
	return action;
}

double BsQLearning::CalcCost (double current_Tput){
	return abs(target_tput - current_Tput);
	//return abs(aggregated_target_tput - current_Tput - WifiTput);
	//return abs(current_Tput - WifiTput);
}

int BsQLearning::ChooseAction(){

    float action_decision = RandomRealUniform();

    if (action_decision < e_policy){
    	return RandomIntUniform(0,n_actions-1);
    }
    else{
    	return GetMinAction(current_state, 'a');
    }
}

void BsQLearning::UpdateQMatrix(){

	int s = current_state;
	int last_s = old_state;
	int a = current_action;
	double min_Q = GetMinAction (s, 'v');
	double r = cost;

	m[last_s][a] = (1-alpha)*m[last_s][a] + alpha * ( r + (gama * min_Q) - m[last_s][a]);
}

void BsQLearning::Run(){

	if (LteTxData == 0) { return; }

	// Calculate APs current Tput
	WifiTput = WifiTxData * 8 / 0.04 / 1000 / 1000;

	//Calculate eNBs current Tput
	LteTput = LteTxData * 8 / 0.04 / 1000 / 1000;

	//Save Tput to log file.
	LogTputStats(LteTput);
	std::cout << LteTput << std::endl;

    //Reset TxData buffers
	ResetApTxData();
	ResetEnbTxData();

	//Check the corresponding state
	old_state = current_state; // Save the old state.
	current_state = VerifyState (LteTput); // New state.

	//Calculate the corresponding cost (reward) of the last (state,action) pair.
	cost = CalcCost(LteTput);

	//Update Q matrix
	UpdateQMatrix();

    /*          NEW PROCESS          */

	//Choose action from s using e-greedy policy.
	current_action = ChooseAction();

    //Check the corresponding action
	double action_to_be_taken = VerifyAction(current_action);

	//Execute the action choosen:
	std::string path_phy = "/NodeList/" + SSTR( node ) + "/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbPhy/SetAbs";
	std::string path_mac = "/NodeList/" + SSTR( node ) + "/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbMac/SetAbs";
	Config::Set (path_mac, DoubleValue (action_to_be_taken));
	Config::Set (path_phy, DoubleValue (action_to_be_taken));
}
