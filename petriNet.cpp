#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
using namespace std;

class Token{
    private:
        string name;
    public:
        Token(string name){this->name = name;}
        void setName(string name){ this->name = name; }
        string getName(){ return name; }

};

// Place class
class Place {
private:
    int tokenCount;
    queue<Token> tokens;
    vector<Token> tokens2;
public:
    Place() : tokenCount(0) {}
    void addToken(string name) { 
        Token token(name);
        tokenCount++; 
        tokens.push(token);
        tokens2.push_back(token);
    }
    void addTokens(vector<Token> token) {
        tokenCount += token.size();
        for(Token tok : token){
            //cout << tok.getName();
            tokens.push(tok);
        }
    }
    Token removeToken() { 
        tokenCount--;
        auto temp = tokens.front(); 
        tokens.pop();
        tokens2.erase(tokens2.begin());
        return temp;
    }
    void removeTokens(int num) {tokenCount -= num;}
    int getTokenCount() { return tokenCount; }
    int getQueueSize() {return tokens.size();}
    queue<Token> getTokens() { return tokens; }
    vector<Token> getTokens2() { return tokens2; }
};

// Transition class
class Transition {
private:
    string name;
    int min = 0;
    vector<Place*> inputPlaces;
    vector<Place*> outputPlaces;
    vector<string> registers;
    vector<string> dataMem;
    vector<string> instructions;
public:
    Transition(int input, string name) {min = input; this->name = name; if(name == "alu"){}}
    Transition(int input, string name, vector<string> registers) {min = input; this->name = name; this->registers = registers;}
    string getName(){return name;}
    void setRegisters(vector<string> data){registers = data;}
    vector<string> getRegisters(){return registers;}
    void setData(vector<string> data){dataMem = data;}
    void setInstructions(vector<string> data){instructions = data;}
    vector<string> getInstructions(){return instructions;}
    void addInputPlace(Place* place) { inputPlaces.push_back(place); }
    void addOutputPlace(Place* place) { outputPlaces.push_back(place); }
    bool isFireable() {
        int count = 0;
        for (Place* place : inputPlaces) {
            /*if (place->getTokenCount() == 0) { // returns false if not enough tokens
                return false;
            }*/
            if(place->getTokenCount() > 1){
                count++;
            }
            else {
                count += place->getTokenCount();
            }
        }
        if(count < min){
            //cout << "min: " << min << "\t";
            //cout << "count: " << count << "\t";
            return false;
        }
        return true;
    }
    void read(Transition* decode){
        vector<string> regs;
        string result = this->getInstructions()[0].substr(1, this->getInstructions()[0].size() - 2); // removes <>
        stringstream ss(result);
        string opcode, destination, first, second;
        bool change1 = false, change2 = false;
        char delimiter = ',';
        getline(ss, opcode, delimiter);
        getline(ss, destination, delimiter);
        getline(ss, first, delimiter);
        getline(ss, second, delimiter);
        for(string reg : registers) {
            if(reg.find(first) != string::npos){
                regs.push_back(reg);
                change1=true;
            }
            if(reg.find(second) != string::npos){
                regs.push_back(reg);
                change2=true;
            }
            if(change1 && change2){
                break;
            }
        } 
        //cout << "Read: Instruction being read " <<  this->getInstructions()[0] << endl;
        decode->setRegisters(regs);
        instructions.erase(instructions.begin());
        
    }
    void fire() {
        if (isFireable()) {
            vector<Token> removedTokens;
            for (Place* place : inputPlaces) {
                if(name == "issue1" && place->getTokens().front().getName().find("LD") != string::npos) { // found a load instruction
                    return;
                }
                else if(name == "issue2" && place->getTokens().front().getName().find("ADD") != string::npos) { // found add instruction
                    return;
                }
                removedTokens.push_back(place->removeToken());
            }
            //cout << name << " Token being removed: " << removedTokens[0].getName() << endl;
            string opcode, destination, first, second;
            string result = removedTokens[0].getName().substr(1, removedTokens[0].getName().size() - 2); // removes <>
            stringstream ss(result);
            if(this->name == "decode"){
                char delimiter = ',';
                getline(ss, opcode, delimiter);
                getline(ss, destination, delimiter);
                getline(ss, first, delimiter);
                getline(ss, second, delimiter);
                bool change1 = false, change2 = false;
                int source1 =0, source2 = 0;
                string newToken = "";
                for(string reg : registers) {
                    if(reg.find(first) != string::npos){
                        //cout << first;
                        //cout << reg;
                        string temp = reg.substr(1, reg.size() - 2);
                        stringstream s(temp);
                        temp = "";
                        getline(s, temp, delimiter); // removes register
                        temp = "";
                        getline(s, temp, delimiter); // gets value
                        source1 = stoi(temp);
                        change1=true;
                    }
                    if(reg.find(second) != string::npos){
                        string temp = reg.substr(1, reg.size() - 2);
                        stringstream s(temp);
                        temp = "";
                        getline(s, temp, delimiter); // removes register
                        temp = "";
                        getline(s, temp, delimiter); // gets value
                        source2 = stoi(temp);
                        change2=true;
                    }
                        /*if(reg.find(destination) != string::npos){
                            //dest = count;
                            newToken.append("<R").append(to_string(count)).append(",");
                        }*/
                    if(change1 && change2){
                        break;
                    }
                        //count++;
                }
                newToken.append("<" + opcode + "," + destination + ",").append(to_string(source1)).append(",").append(to_string(source2)).append(">");
                removedTokens[0].setName(newToken);            
            }
            else if(this->name == "alu"){
                if(removedTokens[0].getName().find("ADD") != string::npos){
                    char delimiter = ',';
                    getline(ss, opcode, delimiter);
                    getline(ss, destination, delimiter);
                    getline(ss, first, delimiter);
                    getline(ss, second, delimiter);
                    int source1 = stoi(first);
                    int source2 = stoi(second);
                    string newToken = "";
                    int sum = source1 + source2;
                    newToken.append("<" + destination + ",").append(to_string(sum)).append(">");
                    removedTokens[0].setName(newToken);
                }
                else if (removedTokens[0].getName().find("SUB") != string::npos){
                    char delimiter = ',';
                    getline(ss, opcode, delimiter);
                    getline(ss, destination, delimiter);
                    getline(ss, first, delimiter);
                    getline(ss, second, delimiter);
                    int source1 = stoi(first);
                    int source2 = stoi(second);
                    string newToken = "";
                    int sum = source1 - source2;
                    newToken.append("<" + destination + ",").append(to_string(sum)).append(">");
                    removedTokens[0].setName(newToken);   
                }
                else if (removedTokens[0].getName().find("AND") != string::npos){
                    char delimiter = ',';
                    getline(ss, opcode, delimiter);
                    getline(ss, destination, delimiter);
                    getline(ss, first, delimiter);
                    getline(ss, second, delimiter);
                    int source1 = stoi(first);
                    int source2 = stoi(second);
                    string newToken = "";
                    int sum = source1 & source2;
                    newToken.append("<" + destination + ",").append(to_string(sum)).append(">");
                    removedTokens[0].setName(newToken);
                }
                else if (removedTokens[0].getName().find("OR") != string::npos){
                    char delimiter = ',';
                    getline(ss, opcode, delimiter);
                    getline(ss, destination, delimiter);
                    getline(ss, first, delimiter);
                    getline(ss, second, delimiter);
                    int source1 = stoi(first);
                    int source2 = stoi(second);
                    string newToken = "";
                    int sum = source1 | source2;
                    newToken.append("<" + destination + ",").append(to_string(sum)).append(">");
                    removedTokens[0].setName(newToken);
                }
            }
            else if (this->name == "addr"){
                char delimiter = ',';
                getline(ss, opcode, delimiter);
                getline(ss, destination, delimiter);
                getline(ss, first, delimiter);
                getline(ss, second, delimiter);
                int source1 = stoi(first);
                int source2 = stoi(second);
                string newToken = "";
                int sum = source1 + source2;
                newToken.append("<" + destination + ",").append(to_string(sum)).append(">");
                removedTokens[0].setName(newToken);
            }
            else if (this->name == "load"){
                char delimiter = ',';
                getline(ss, destination, delimiter);
                getline(ss, first, delimiter);
                string temp = dataMem[stoi(first)].substr(1,dataMem[stoi(first)].size()-2);
                stringstream s(temp);
                temp = "";
                getline(s, first, delimiter); // index
                getline(s, second, delimiter); // value
                string newToken = "";
                newToken.append("<" + destination + "," + second + ">");
                removedTokens[0].setName(newToken);
            }
            else if (this->name == "write"){
                char delimiter = ',';
                getline(ss, destination, delimiter);
                for(int i =0; i< registers.size(); i++){
                    if(registers[i].find(destination) != string::npos){
                        registers[i] = removedTokens[0].getName();
                    }
                }
            }
            for (Place* place : outputPlaces) {
                place->addToken(removedTokens[0].getName());
            }
        }
    }
};

// PetriNet class
class PetriNet {
private:
    vector<Place*> places;
    vector<Transition*> transitions;
    vector<string> instructions;
    vector<string> dataMem;
    vector<string> registers;
public:
    vector<string> getInstructions(){return instructions;}
    vector<string> getDataMem(){return dataMem;}
    vector<string> getRegisters(){return registers;}
    vector<Transition*> getTransitions(){return transitions;}
    vector<Place*> getPlaces(){return places;}
    void addPlace(Place* place) { places.push_back(place); }
    void addTransition(Transition* transition) { transitions.push_back(transition); }
    void prepFiles(){
        ifstream file("inst.txt");
        if(file.is_open()){
            string line;
            while(getline(file, line)){
                instructions.push_back(line);
            }
        }

        file.close();
        file.open("datamem.txt");

        if(file.is_open()){
            string line;
            while(getline(file, line)){
                dataMem.push_back(line);
            }
        }

        file.close();
        file.open("reg.txt");

        if(file.is_open()){
            string line;
            while(getline(file, line)){
                registers.push_back(line);
            }
        }
    }
    
};

void fireTransition(Transition* transition){
    transition->fire();
}

void simulate(vector<string> instructions, vector<string> registers, vector<string> dataMem, vector<Transition*> transitions, vector<Place*> places, Transition read, string fileName) {
        queue<Transition*> willFire;
        ofstream outputFile(fileName);
        bool firingOccurred = true;
        int count = 0;
        while (firingOccurred) {
            firingOccurred = false;
            outputFile << "STEP " << count << ":\n";
            outputFile << "INM:";
            for(int i = count; i < instructions.size(); i++){
                if(i == instructions.size() - 1){
                    outputFile << instructions[i];
                }
                else{
                    outputFile << instructions[i] << ", ";
                }
            }
            outputFile << endl;
            outputFile << "INB:";
            for(int i = 0; i<places[1]->getTokens().size(); i++){
                if(i == places[1]->getTokens().size() - 1){
                    outputFile << places[1]->getTokens2()[i].getName();
                }
                else{
                    outputFile << places[1]->getTokens2()[i].getName() << ", ";
                }
            }
            outputFile << endl;
            outputFile << "AIB:";
            for(int i = 0; i<places[2]->getTokens().size(); i++){
                if(i == places[2]->getTokens().size() - 1){
                    outputFile << places[2]->getTokens2()[i].getName();
                }
                else{
                    outputFile << places[2]->getTokens2()[i].getName() << ", ";
                }
            }
            outputFile << endl;
            outputFile << "LIB:";
            for(int i = 0; i<places[3]->getTokens().size(); i++){
                if(i == places[3]->getTokens().size() - 1){
                    outputFile << places[3]->getTokens2()[i].getName();
                }
                else{
                    outputFile << places[3]->getTokens2()[i].getName() << ", ";
                }
            }
            outputFile << endl;
            outputFile << "ADB:";
            for(int i = 0; i<places[4]->getTokens().size(); i++){
                if(i == places[4]->getTokens().size() - 1){
                    outputFile << places[4]->getTokens2()[i].getName();
                }
                else{
                    outputFile << places[4]->getTokens2()[i].getName() << ", ";
                }
            }
            outputFile << endl;
            outputFile << "REB:";
            for(int i = 0; i<places[5]->getTokens().size(); i++){
                if(i == places[5]->getTokens().size() - 1){
                    outputFile << places[5]->getTokens2()[i].getName();
                }
                else{
                    outputFile << places[5]->getTokens2()[i].getName() << ", ";
                }
            }
            outputFile << endl;
            outputFile << "RGF:";
            int index = 0;
            for(string reg : registers){
                outputFile << reg;
                if(index < registers.size() - 1){
                    outputFile << ",";
                }
                index++;
            }
            outputFile << endl;
            outputFile << "DAM:";
            index = 0;
            for(string data : dataMem){
                outputFile << data;
                if(index < registers.size() - 1){
                    outputFile << ",";
                }
                index++;
            }
            for (Transition* transition : transitions) {
                if (transition->isFireable()) {
                    willFire.push(transition);
                }
                //threads.emplace_back([&]() {fireTransition(transition);});
            }
            while(!willFire.empty()){
                if (willFire.front()->getName() == "decode"){
                    read.read(willFire.front());
                }
                willFire.front()->fire();
                firingOccurred = true;
                if (willFire.front()->getName() == "write"){
                    /*for(string reg : registers){
                        cout << reg;
                    }*/
                    registers = willFire.front()->getRegisters();
                    read.setRegisters(registers); // changed line
                    /*for(string reg : registers){
                        cout << reg;
                    }*/
                }
                willFire.pop();
            }
            /*for (std::thread& thread : threads) {
                thread.join();
            }*/
            /*if (count < 12){
                firingOccurred = true;
                count++;
                cout << endl;
            }*/
            if(firingOccurred != false){
                outputFile << endl << endl;
            }
            
            count++;
        }
        outputFile.close();
    }

int main(){
    Place INM, AIB, INB, LIB, ADB, REB, RGF, DAM;
    Transition decode(1, "decode"), read(2, "read"), write(1, "write"), load(2, "load"), issue1(1, "issue1"), issue2(1, "issue2"), addr(1, "addr"), alu(1, "alu");
    PetriNet petriNet;
    //ofstream outputFile;

    decode.addInputPlace(&INM);
    issue1.addInputPlace(&INB);
    issue2.addInputPlace(&INB);
    addr.addInputPlace(&LIB);
    alu.addInputPlace(&AIB);
    //addr.addInputPlace(&LIB);
    load.addInputPlace(&ADB);
    load.addInputPlace(&DAM);
    write.addInputPlace(&REB);
    read.addInputPlace(&RGF);

    decode.addOutputPlace(&INB);
    issue1.addOutputPlace(&AIB);
    issue2.addOutputPlace(&LIB);
    alu.addOutputPlace(&REB);
    addr.addOutputPlace(&ADB);
    load.addOutputPlace(&REB);
    write.addOutputPlace(&RGF);
    read.addOutputPlace(&INB);

    petriNet.addPlace(&INM); // 0
    petriNet.addPlace(&INB); // 1
    petriNet.addPlace(&AIB); // 2
    petriNet.addPlace(&LIB); // 3
    petriNet.addPlace(&ADB); // 4
    petriNet.addPlace(&REB); // 5
    petriNet.addPlace(&RGF);
    petriNet.addPlace(&DAM);

    //petriNet.addTransition(&decode); // INM to INB
    petriNet.addTransition(&issue1); // INB to AIB
    petriNet.addTransition(&issue2); // INB to LIB
    petriNet.addTransition(&write);
    petriNet.addTransition(&read);
    petriNet.addTransition(&load);
    petriNet.addTransition(&alu);
    petriNet.addTransition(&addr);
    petriNet.addTransition(&decode); // INM to INB

    petriNet.prepFiles();
    // Initial tokens
    for(string instruction : petriNet.getInstructions()){
        INM.addToken(instruction);
    }
    for(string data : petriNet.getDataMem()){
        DAM.addToken(data);
    }
    for(string reg : petriNet.getRegisters()){
        RGF.addToken(reg);
    }
    read.setInstructions(petriNet.getInstructions());
    read.setRegisters(petriNet.getRegisters());
    load.setData(petriNet.getDataMem());
    write.setRegisters(petriNet.getRegisters());
    simulate(petriNet.getInstructions(), petriNet.getRegisters(), petriNet.getDataMem(), petriNet.getTransitions(), petriNet.getPlaces(), read, "simulation.txt");
    return 0;
}
