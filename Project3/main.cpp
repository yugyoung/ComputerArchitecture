#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <algorithm> //toupper
#include <bitset>
#include <iomanip>
#include <cstdlib> // abs for long int
#include <cmath>
#include <cstring>

using namespace std;


map<long long,string> memory_data; //16 진수로 저장
map<long long,string> memory_text; //instruction memory //2진수로 저장
map<int,long long> register_file; // register file //16진수로 저장
int PC=0x400000;
int PC_data=0x10000000;
int n=-1; //default
int cycle=0;
string stage[5]; //""으로 초기화됨
map<string,string> option;
void Init(){
    for(int i=0;i<32;i++) register_file[i]=0x0; //나중에 출력할때 0포함해서 출력해야해
    option["-atp"]="";
    option["-antp"]="";
    option["-m"]="";
    option["-d"]="";
    option["-p"]="";
}
//변환 함수
string dec2hex(long long dec){
    stringstream hex; //양, 음 구분해서 받아
    hex<<"0x"<<std::hex<<dec;
    return hex.str();
}
void PC_increase(){
    PC+=4;
}
long long hex2dec(string hex){ //양수로 취급
    long long hex_dec=strtoll(hex.c_str(),NULL,0);
    return hex_dec;
}
long long bin2dec(string bin){ //양수로
    long long dec=strtoll(bin.c_str(),NULL,2);
    return dec;
}
string bin2hex(string binary){
    long long dec_=bin2dec(binary);
    stringstream hex;
    hex<<"0x"<<std::hex<<dec_;
    return hex.str();
}
string sign_ext(string bin){
    string bin_;
    for(int i=0; i!=32-bin.size();i++){
        bin_+=bin[0];
    }
    bin_+=bin;
    return bin_;
}
string unsign_ext(string bin){
    string bin_;
    for(int i=0;i!=32-bin.size();i++){
        bin_+="0";
    }
    bin_+=bin;
    return bin_;
}
long long bin2dec_sign(string bin){
    if(bin[0]=='1'){
        long long bin_minus;
        bin_minus=(-1)*pow(2,bin.size()-1);
        for(int i=bin.size()-2;i!=-1;i--){
            if(bin[bin.size()-1-i]=='1'){
                bin_minus+=pow(2,i);
            }
        }
        return bin_minus;
    }//[0]=='0'이어서 양수일 때
    return bin2dec(bin);
}
string hex2bin_char(char hex){
    switch(toupper(hex)){
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'a':
        case 'A': return "1010";
        case 'b':
        case 'B': return "1011";
        case 'c':
        case 'C': return "1100";
        case 'd':
        case 'D': return "1101";
        case 'e':
        case 'E': return "1110";
        case 'f':
        case 'F': return "1111";
    }
}
string hex2bin(string hex){
    string bin;
    hex=hex.substr(2);//0x제거
    for(int i=0; i!=hex.length(); ++i) bin+=hex2bin_char(hex[i]);
    return bin;
}
void read_file(string fname){
    ifstream fileopen;
    fileopen.open(fname,ios::in); //sample.o열어
    char low[256];
    if (fileopen.is_open()) {
        while (!fileopen.eof()) //파일 끝까지 읽었는지 확인
        {
            //memory_text setting
            fileopen.getline(low, 256);
            long long num_text=strtoll(low,NULL,0);
            long long i=num_text/4 ; //생성 개수
            //memory_data setting
            fileopen.getline(low,256);
            long long num_data=strtoll(low,NULL,0);
            long long j=num_data/4; //data 생성개수
            // while() n개수만큼만 실행해야해
            while(i!=0){
                fileopen.getline(low, 256); //한줄씩 읽어오기 , 이전에 읽었던 줄은 날아가버려
                long long decimal=strtoll(low,NULL,0);
                string bincode_text=std::bitset<32>(decimal).to_string();
                memory_text[PC] = bincode_text;
                PC_increase();
                i--;
            }
            while(j!=0){
                fileopen.getline(low, 256); //한줄씩 읽어오기 , 이전에 읽었던 줄은 날아가버려
                memory_data[PC_data]=low;
                PC_data+=4;
                j--;
            }// while(j!=0) end
        } //while (!fileopen.eof())  end
    }//if (fileopen.is_open()) end
}
void p_print(){
    cout<<"===== Cycle "<<cycle<<" ====="<<endl;
    cout<<"Current pipeline PC state:"<<endl;
    cout<<"{"<<stage[0]<<"|"<<stage[1]<<"|"<<stage[2]<<"|"<<stage[3]<<"|"<<stage[4]<<"}\n"<<endl;
}
void m_print(){
    cout<<"Memory content ["<<option["-m"].substr(0,option["-m"].find(":"))<<".."<<option["-m"].substr(option["-m"].find(":")+1)<<"]:"<<endl;
    cout<<"--------------------------------------"<<endl;
    string m1=option["-m"].substr(0,option["-m"].find(':'));
    string m2=option["-m"].substr(option["-m"].find(':')+1);
    long long m1_dec=hex2dec(m1);
    long long m2_dec=hex2dec(m2)+4;
    //첫 시작주소부터 범위벗어났을 경우
    auto itr_d=memory_data.find(m1_dec);
    auto itr_t=memory_text.find(m1_dec);
    if(m1_dec<0x400000){
        cout<<"out of start address"<<endl;
        return;
    }
    if(hex2dec(m2)<m1_dec){
        cout<<"range error"<<endl;
        return;
    }
    for(m1_dec; m1_dec!=m2_dec; m1_dec+=4){
        auto itr_d=memory_data.find(m1_dec);
        auto itr_t=memory_text.find(m1_dec);
        if(itr_d!=memory_data.end()){ //m1 key값을 가진 value 발견되면,
            cout<<dec2hex(itr_d->first)<<": "<<itr_d->second<<endl;
        }
        else if(itr_t!=memory_text.end()){
            cout<<dec2hex(itr_t->first)<<": "<<bin2hex(itr_t->second)<<endl;
        }
        else{
            cout<<dec2hex(m1_dec)<<": 0x0"<<endl;
        }
    }cout<<endl;
}
void regi_print(){
    cout<<"Current register values: "<<endl;
    cout<<"PC: "<<dec2hex(PC)<<endl; //0x포함됨
    cout<<"Registers:"<<endl;
    for(int i=0; i<32; i++){
        cout<<"R"<<i<<": "<<dec2hex(register_file[i])<<endl;
    }
    cout<<endl;
}
int last_signal=0;
int check=0;

void output() {
    if(last_signal<5){
        if(option["-p"]!="") p_print();
        if(option["-d"]!="") {
            regi_print();
            if(option["-m"]!="") m_print();
        }
    }
    if(last_signal==0 and n==0){ //n이 부분만가지다 0으로 끝났을 때
        if(option["-d"]=="" and option["-m"]!="") m_print();

        return;
    }
    //n==0일때
    if(last_signal==4){
        //끝났는데, 원래 옵션들은 있었어
        if(option["-d"]=="" and option["-m"]!="") m_print();
        cout<<"===== Completion cycle: "<<cycle<<" ====="<<endl<<endl;
        cout<<"Current pipeline PC state:"<<endl;
        cout<<"{||||}"<<endl<<endl;
        regi_print();
        return;
    }
    if(last_signal==4 and check==1){ //atp,antp 옵션만 있는 경우
        cout<<"===== Completion cycle: "<<cycle<<" ====="<<endl<<endl;
        cout<<"Current pipeline PC state:"<<endl;
        cout<<"{||||}"<<endl<<endl;
        regi_print();
    }
}

class IF{ //IF
public:
    int flush=0;
    int lu_hazard=0;
};
class ID{ //IF_ID를 읽어와
public:
    string Instr;
    long long PC;
    long long NPC=0;
    int flush;
};
class EX{ //ID_EX
public:
    long long PC;
    int MemRead=0;
    long long NPC=0;
    string Instr_type;
    int Instr_funct;
    int Instr_opcode;
    int rs; //레지스터 번호
    int rt;
    int rd;
    int shamt;
    long long rs_regi;
    long long rt_regi;
    long long rd_regi;
    string IMM;
    int flush;
};
class MEM{
public:
    long long PC;
    int RegWrite=0;
    int MemWrite=0;
    int MemRead=0;
    int Zero_signal=0; //bne, beq signal
    int RegisterRd;
    unsigned int RegisterRd_regi;
    int store_source;
    int get_word=0;
    unsigned int ALU_OUT=0;
    long long BR_TARGET=0;
};
class WB{
public:
    long long PC;
    int RegisterRd;
    int MemRead=0;
    unsigned int RegisterRd_regi;
    int RegWrite=0;
    unsigned int ALU_OUT=0;
    string MEM_OUT;
};
IF IF;
ID IF_ID; //ID에서 IF_ID 이용
EX ID_EX;
MEM EX_MEM;
WB MEM_WB;
void ALU(){ //EX stage에서 실행
    EX_MEM.Zero_signal=-1;//signal 초기화 //branch문이 아니라는 신호
    EX_MEM.MemRead=0; //signal 초기화
    EX_MEM.MemWrite=0;
    EX_MEM.store_source=0;
    //R type
    if(ID_EX.Instr_type=="R"){
        EX_MEM.RegWrite=1;
        EX_MEM.RegisterRd=ID_EX.rd;
        switch(ID_EX.Instr_funct){
            case 0x21 : { //ADDU
                EX_MEM.ALU_OUT=(ID_EX.rs_regi+ID_EX.rt_regi)%0x100000000;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x24 :  {//AND
                EX_MEM.ALU_OUT=ID_EX.rs_regi&ID_EX.rt_regi;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x27 : { //NOR
                EX_MEM.ALU_OUT=~(ID_EX.rs_regi|ID_EX.rt_regi);
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x25 : { //OR
                EX_MEM.ALU_OUT=(ID_EX.rs_regi|ID_EX.rt_regi);
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x2b : {//SLTU
                if(ID_EX.rs_regi<ID_EX.rt_regi) EX_MEM.ALU_OUT=1;
                else EX_MEM.ALU_OUT=0;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0 : { //SLL -Noop
                EX_MEM.ALU_OUT=ID_EX.rt_regi<<ID_EX.shamt; //범위 벗어나는 경우 생각하기
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 2 : {//SRL
                EX_MEM.ALU_OUT=ID_EX.rt_regi>>ID_EX.shamt; //범위 벗어나는 경우 생각하기
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x23 : {//SUBU
                ID_EX.rt_regi*=-1;
                string com_bin=std::bitset<32>(ID_EX.rt_regi).to_string();
                EX_MEM.ALU_OUT=(ID_EX.rs_regi+bin2dec(com_bin))%0x100000000;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x8 : {//JR
                EX_MEM.RegWrite=0;
            }
        }
    }
        //I type
    else{
        EX_MEM.RegWrite=1;
        EX_MEM.RegisterRd=ID_EX.rt;
        switch(ID_EX.Instr_opcode){ //각 변수에 noop가 들어오는 경우도 생각해야해
            case 9: { //ADDIU
                EX_MEM.ALU_OUT=(ID_EX.rs_regi+bin2dec_sign(sign_ext(ID_EX.IMM)))%0x100000000;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT; //destination register에 저장할 값
                break;
            }
            case 0xc: { //ANDI
                EX_MEM.ALU_OUT=ID_EX.rs_regi&bin2dec(unsign_ext(ID_EX.IMM));
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 4 : { //BEQ
                EX_MEM.RegWrite=0;
                EX_MEM.RegisterRd=0; //아래와 같이 한번 더 확인하기
                EX_MEM.RegisterRd_regi=0;
                EX_MEM.ALU_OUT=ID_EX.rs_regi-ID_EX.rt_regi;
                if(EX_MEM.ALU_OUT==0) EX_MEM.Zero_signal=1; //Zero_signal =1인지 확인하기
                else EX_MEM.Zero_signal=0; //branch안해
                EX_MEM.BR_TARGET=ID_EX.NPC+bin2dec(ID_EX.IMM)*4;
                break;
            }
            case 5 : { //BNE
                EX_MEM.RegWrite=0;
                EX_MEM.RegisterRd=0; //아래와 같이 한번 더 확인하기
                EX_MEM.RegisterRd_regi=0;
                EX_MEM.ALU_OUT=ID_EX.rs_regi-ID_EX.rt_regi;
                if(EX_MEM.ALU_OUT==0) EX_MEM.Zero_signal=0;
                else EX_MEM.Zero_signal=1; //다를경우 branch수행
                EX_MEM.BR_TARGET=ID_EX.NPC+bin2dec(ID_EX.IMM)*4;
                break;
            }
                //J type
            case 3 : {
                EX_MEM.RegisterRd=31;
                EX_MEM.ALU_OUT=ID_EX.NPC;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 2 : {
                EX_MEM.RegWrite=0; //아무것도 못하도록 //다른 signal은 위에서 제어
                break;
            }
            case 0xf : { //LUI
                EX_MEM.RegWrite=-1;
                EX_MEM.ALU_OUT=bin2dec(ID_EX.IMM); //return type맞추기 위해 어쩔수 없이 한번 전환해야해
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT*=65536; //bin2dec임
                break;
            }
            case 0x23 : { //LW //MEM stage에서 memory에서 word 데이터 들고와야 해.
                EX_MEM.RegWrite=1; //삭제가능
                EX_MEM.MemRead=1;
                EX_MEM.ALU_OUT=ID_EX.rs_regi+bin2dec(ID_EX.IMM);//양수만 취급하여!!!!/계산한 주소
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT; //_regi에 저장할 필요없음 -->안할거면 초기화 필요
                EX_MEM.get_word=1;
                break;
            }
            case 0x20 : { //LB
                EX_MEM.RegWrite=1;
                EX_MEM.MemRead=1;
                EX_MEM.ALU_OUT=ID_EX.rs_regi+bin2dec(ID_EX.IMM);//양수만 취급하여!!!!/계산한 주소
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                EX_MEM.get_word=0;
                break;
            }
            case 0xd : {//ORI
                EX_MEM.ALU_OUT=(ID_EX.rs_regi|bin2dec(ID_EX.IMM));//양수만 취급하여!!!!/계산한 주소
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0xb : {//SLTIU
                if(ID_EX.rs_regi<bin2dec_sign(sign_ext(ID_EX.IMM))) EX_MEM.ALU_OUT=1; //임시로 ALU_OUT으로 지정
                else EX_MEM.ALU_OUT=0;
                EX_MEM.RegisterRd_regi=EX_MEM.ALU_OUT;
                break;
            }
            case 0x2b : {//SW //다시보기!!
                EX_MEM.RegWrite=0;
                EX_MEM.MemWrite=1;
                EX_MEM.get_word=1;
                EX_MEM.RegisterRd_regi=ID_EX.rt_regi;//rt의 word를 메모리에 저장하니까//다시확인하기!!
                EX_MEM.store_source=ID_EX.rt;
                EX_MEM.ALU_OUT=ID_EX.rs_regi+bin2dec(ID_EX.IMM);
                EX_MEM.RegisterRd=EX_MEM.ALU_OUT; //임시로 주소를 저장해놔!!
                break;
            }
            case 0x28 : {//SB //다시보기!!
                EX_MEM.RegWrite=0;
                EX_MEM.MemWrite=1;
                EX_MEM.get_word=0;
                EX_MEM.store_source=ID_EX.rt;
                EX_MEM.RegisterRd_regi=ID_EX.rt_regi;//rt의 word를 메모리에 저장하니까//다시확인하기!!
                EX_MEM.ALU_OUT=ID_EX.rs_regi+bin2dec(ID_EX.IMM);
                EX_MEM.RegisterRd=EX_MEM.ALU_OUT; //임시로 주소를 저장해놔!!
                break;
            }
        }
    }
}
void IF_stage(){ //끝남을 어떻게 알려주지??
    if(auto itr=memory_text.find(PC)!=memory_text.end()){
        stage[0]=dec2hex(PC);
        if(IF.lu_hazard==1){
            //IF_ID.Instr가만히 둬야해
            //PC도 증가하지 않고 가만히 두기
            //다음 cycle에서도 같은 작업수행하기 위해
            IF.lu_hazard=0;
            return;
        }
        if(IF.flush==0) {
            IF_ID.Instr=memory_text[PC];
            IF_ID.PC=PC;
            PC+=4; //PC증가시켜
            IF_ID.NPC=PC;
        }
        else if(IF.flush==1){ //PC=branch target instrs
            stage[0]="";
            IF.flush=0; //다음부터는 위에를 수행
            //PC를 증가시키지 않아 //flush이후에 한번더 그 PC값을 돌리기 위해
        }
        //if(IF_ID.Flush==0) IF_ID.Instr="0"; //다음 ID에 noop을 넘겨.
    }
    else{
        last_signal+=1;
        stage[0]="";
    }
}
void ID_stage(){
    if(cycle<1) return;
    stage[1]=dec2hex(IF_ID.PC); //현재 수행되고있는 instr
    //Instr 패치가 안되었을 때 ==noop 혹은 수행 끝
    if(stage[0]=="") {
        stage[1]="";
        return;
    }
    //ID stage stall
    if(IF_ID.flush==1) {
        IF_ID.flush=0;
        stage[1]="";
        ID_EX.PC=0;
        ID_EX.MemRead=0;
        ID_EX.NPC=0;
        ID_EX.Instr_type="";
        ID_EX.Instr_funct=0;
        ID_EX.Instr_opcode=0;
        ID_EX.rs=0; //레지스터 번호
        ID_EX.rt=0;
        ID_EX.rd=0;
        ID_EX.shamt=0;
        ID_EX.rs_regi=0;
        ID_EX.rt_regi=0;
        ID_EX.rd_regi=0;
        ID_EX.IMM="";
        ID_EX.flush=0;
        //아무것도 수행하지마.
        return;//noop 수행 x.
    }
    //공통수행
    //PC전달
    ID_EX.NPC=IF_ID.NPC;
    ID_EX.PC=IF_ID.PC;
    ID_EX.Instr_opcode=stoi(IF_ID.Instr.substr(0,6),NULL,2);//opcode만 보고 exe수행하도록
    //type_check
    if(IF_ID.Instr.substr(0,6)=="000000"){
        ID_EX.Instr_type="R";
        ID_EX.Instr_funct=stoi(IF_ID.Instr.substr(IF_ID.Instr.size()-6,6),NULL,2);
    }
    else{ //아니라면 이전 constrol signal 초기화
        ID_EX.Instr_type="";
        ID_EX.Instr_funct=0; //0으로 해도되는지 확인하기
    }
    //lw,lb load-use data hazard
    if(ID_EX.MemRead==1){ //미리 보냈던 signal이 MemRead인 경우 data hazrd를 체크해야해!
        if(ID_EX.rt==stoi(IF_ID.Instr.substr(6,5),NULL,2) or ID_EX.rt==stoi(IF_ID.Instr.substr(11,5),NULL,2)){
            //rs와 rt를 갖지않는 J format은 실행하면 안됨.
            if(ID_EX.Instr_type=="R"){
                if(ID_EX.Instr_opcode!=2 and ID_EX.Instr_opcode!=3 and ID_EX.Instr_funct!=8){
                    ID_EX.flush=1; //다음 사이클에서 EX stage를 noop
                    ID_EX.MemRead=0;
                    IF.lu_hazard=1;
                    return; //IF_ID pipeline그대로 두고, ID_EX noop으로 날려버려
                }
            }
        }
        ID_EX.MemRead=0; //datahazard가 아닐때도 신호를 해지시켜줘야해
    }
    //cycle 끝날 때 저장할 값 //모든 instr 공통
    ID_EX.rs=stoi(IF_ID.Instr.substr(6,5),NULL,2); //2진수->10진수
    ID_EX.rt=stoi(IF_ID.Instr.substr(11,5),NULL,2);
    ID_EX.rd=stoi(IF_ID.Instr.substr(16,5),NULL,2);
    ID_EX.IMM=IF_ID.Instr.substr(16); //일단 양수로 가져왔다!!
    ID_EX.shamt=bin2dec(IF_ID.Instr.substr(21,5)); //shamt는 양수만 오는 경우만 생각
    ID_EX.rs_regi=register_file[ID_EX.rs];
    ID_EX.rt_regi=register_file[ID_EX.rt];
    ID_EX.rd_regi=register_file[ID_EX.rd];
    //option & branch
    if(option["-atp"]!=""){//atp옵션일 때
        // branch instr(bne,beq)
        if(ID_EX.Instr_opcode==4 or ID_EX.Instr_opcode==5){ //BEQ, BNE
            IF.flush=1;
            PC=IF_ID.NPC+bin2dec(ID_EX.IMM)*4; //PC를 target addr로 변경
        }
    }
    //jump instr은 무조건 한사이클 지연 필요
    //J / JAL
    if(ID_EX.Instr_opcode==2 or ID_EX.Instr_opcode==3){
        IF.flush=1;
        PC=(IF_ID.PC)&0xF0000000|(bin2dec(IF_ID.Instr.substr(6))*4);
    }
    //JR //Instr_type과 Instr_funct항상 동시에 체크하기!
    if(ID_EX.Instr_type=="R" and ID_EX.Instr_funct==8){
        if(ID_EX.rs_regi!=0) {
            IF.flush = 1;
            PC = ID_EX.rs_regi;
        }
    }
    //lw,lb // load-use data hazard
    if(ID_EX.Instr_opcode==0x23 or ID_EX.Instr_opcode==0x20){
        ID_EX.MemRead=1; //signal 미리 보내줘야해
    }
}
void EX_stage(){
    if(cycle<2) return;
    stage[2]=dec2hex(ID_EX.PC);
    if(stage[1]=="") {
        stage[2]="";
        return;
    }
    //load-use data hazard //noop
    if(ID_EX.flush==1) {
        stage[2]="";
        ID_EX.flush=0;
        EX_MEM.PC=0;
        EX_MEM.RegWrite=0;
        EX_MEM.MemWrite=0;
        EX_MEM.MemRead=0;
        EX_MEM.Zero_signal=0; //bne, beq signal
        EX_MEM.RegisterRd=0;
        EX_MEM.RegisterRd_regi=0;
        EX_MEM.store_source=0;
        EX_MEM.get_word=0;
        EX_MEM.ALU_OUT=0;
        EX_MEM.BR_TARGET=0;//초기값 //0으로 해도 되나??
        return;
    }
    EX_MEM.PC=ID_EX.PC;
    ALU();
}
void MEM_stage(){
    if(cycle<3) return;
    stage[3]=dec2hex(EX_MEM.PC);
    if(stage[2]=="") {
        stage[3]="";
        return;
    }
    //다음 pipeline stage로 저장
    MEM_WB.PC=EX_MEM.PC;
    MEM_WB.RegWrite=EX_MEM.RegWrite;
    MEM_WB.ALU_OUT=EX_MEM.ALU_OUT;
    MEM_WB.RegisterRd_regi=EX_MEM.RegisterRd_regi;
    MEM_WB.RegisterRd=EX_MEM.RegisterRd;
    //EX Forward Unit
    if(EX_MEM.RegWrite==1 or EX_MEM.RegWrite==-1){
        if(EX_MEM.RegisterRd!=0) {
            //registerRd가 rs와 rt둘다와 동일하면 if문 두개다 실행될 수 있음.
            if (EX_MEM.RegisterRd == ID_EX.rs) ID_EX.rs_regi = EX_MEM.RegisterRd_regi;
            if (EX_MEM.RegisterRd == ID_EX.rt) ID_EX.rt_regi = EX_MEM.RegisterRd_regi;
        }
    }
    //mem stage수행
    if(EX_MEM.MemRead==1){ //lw,lb
        if(EX_MEM.get_word==0) { //8bit만 가져올 때
            string se_imm;
            string bin;
            string word_value;
            unsigned int addr=EX_MEM.RegisterRd_regi; //RegisterRd_regi==ALU_OUT
            int index=addr%4; //주소 나머지//index=1,2,3
            if(auto itr=memory_data.find(addr)==memory_data.end()){
                //못 찾았을 경우.
                string value=hex2bin(memory_data[addr-index]); //4Byte들어고와
                value=unsign_ext(value);
                word_value=value.substr(index*8,8);
            }
            else{
                bin=memory_data[addr];
                word_value=hex2bin("0x"+bin.substr(2,2));
            }
            MEM_WB.MEM_OUT=sign_ext(word_value);//binary
        }
        else{
            long long addr=EX_MEM.RegisterRd_regi;
            int index=addr%4;
            string fourwordvalue;
            if(auto ltr=memory_data.find(addr)==memory_data.end()){
                //주소에 없는 경우에서 들고오는건 생각안해.
                string value1=hex2bin(memory_data[addr-index]);
                value1=unsign_ext(value1);
                string value2=hex2bin(memory_data[addr+(4-index)]);//다음값
                value1=unsign_ext(value2);
                fourwordvalue=value1.substr(index*8)+value2.substr(0,(4-index)*8); //binary
            }
            else{
                fourwordvalue=
                        hex2bin(memory_data[addr]);//binary
            }
            MEM_WB.MEM_OUT=fourwordvalue;
        }
        MEM_WB.MemRead=1;
        MEM_WB.RegWrite=1; //삭제해도 됨
        MEM_WB.RegisterRd=EX_MEM.RegisterRd; //destination 번호
        MEM_WB.RegisterRd_regi=bin2dec(MEM_WB.MEM_OUT); //RegisterRd_regi는 longlong type//destination 주소 //양수로 취급
    }
    if(EX_MEM.MemWrite==1){//SW, SB
        if(EX_MEM.get_word==1){//sW
            long long addr=EX_MEM.RegisterRd;
            int index=addr%4;
            if(addr%4!=0){ //주소가 4의 배수를 가리키고 있지 않을 때.
                if(auto itr=memory_data.find(addr-index)==memory_data.end()){
                    memory_data[addr-index]="0x0";
                    memory_data[addr+(4-index)]="0x0";
                }
                string value1=unsign_ext(hex2bin(memory_data[addr-index]));//32bit
                string value2=unsign_ext(hex2bin(memory_data[addr+(4-index)]));//32bit
                string rt_value=unsign_ext(hex2bin(dec2hex(EX_MEM.RegisterRd_regi)));//32bit
                value1=value1.replace(index*8,(value1.size()-(index*8)),rt_value.substr(0,(value1.size()-(index*8))));
                value2=value2.replace(0,(4-index)*8,rt_value.substr((value1.size()-(index*8))));
                memory_data[addr-index]=bin2hex(value1);
                memory_data[addr+(4-index)]=bin2hex(value2);
            }
            else{
                memory_data[addr]=
                        dec2hex(EX_MEM.RegisterRd_regi); //확인하기
            }
        }
        else if(EX_MEM.get_word==0){//SB
            string bin=unsign_ext(hex2bin(dec2hex(EX_MEM.RegisterRd_regi))); //rt에 있는 값.
            long long addr=EX_MEM.RegisterRd;
            int index = addr%4;
            if(addr%4!=0){ //4단위가 아닐때
                if(auto itr=memory_data.find(addr-index)==memory_data.end()){
                    memory_data[addr-index]="0x0";
                }
                string value_bin = unsign_ext(hex2bin(memory_data[addr-index]));
                value_bin=value_bin.replace(index*8,8,bin.substr(bin.size()-8));
                memory_data[addr-index]=bin2hex(value_bin);
            }
            else{
                if(auto itr=memory_data.find(addr)==memory_data.end()){ //저장되어있지 않은 메모리라면, 0x0으로 초기화 후 저장
                    memory_data[addr]="0x0";
                }
                string bin_or=unsign_ext(hex2bin(memory_data[addr]));
                memory_data[addr]=
                        bin2hex(bin.substr(bin.size()-8)+bin_or.substr(8));
            }
        }
        MEM_WB.RegWrite=0;
    }
    if(EX_MEM.Zero_signal==1){
        if(option["-antp"]!=""){ //-atp일 때는 분기하는게 맞았으므로 예측성공
            ID_EX.flush=1;
            IF_ID.flush=1;
            IF.flush=1;
            //branch target addr
            PC=EX_MEM.BR_TARGET; //zero_signal을 초기화하기 때문에 BR_TARGET까지 제어할 필요 x.
        }
    }
    if(EX_MEM.Zero_signal==0 and option["-atp"]!=""){ //branch안하는데 한다고 예측했을 때
        ID_EX.flush=1;
        IF_ID.flush=1;
        IF.flush=1;
        PC=EX_MEM.PC+4; //NPC??
    }
}
void WB_stage(){
    if(cycle<4) return; //cycle 4이상부터 실행
    stage[4]=dec2hex(MEM_WB.PC);
    if(stage[3]=="") {
        stage[4]="";
        return;
    }
    //WB to MEM data hazard-forwarding
    //lw, lb->sb,sw
    if(MEM_WB.MemRead==1 and MEM_WB.RegWrite==1){ //register에 쓰기 전에 sw, sb에 보내
        if(EX_MEM.MemWrite==1 and EX_MEM.store_source==MEM_WB.RegisterRd){//sb,sw밖에없음
            EX_MEM.RegisterRd_regi=MEM_WB.RegisterRd_regi;
        }
        MEM_WB.MemRead=0;
    }
    //EX stage에 data forwarding을 하기 위해서
    if(MEM_WB.RegWrite==1 or MEM_WB.RegWrite==-1){
        if( MEM_WB.RegisterRd!=0) {//MEM Forward Unit
            if (EX_MEM.RegisterRd != ID_EX.rs and MEM_WB.RegisterRd == ID_EX.rs) ID_EX.rs_regi = MEM_WB.RegisterRd_regi;
            if (EX_MEM.RegisterRd != ID_EX.rt and MEM_WB.RegisterRd == ID_EX.rt) ID_EX.rt_regi = MEM_WB.RegisterRd_regi;
        }
    }
    // 굳이 luui도 regWrite -1할 필요없음
    if(MEM_WB.RegWrite==1 or MEM_WB.RegWrite==-1){ //저장 //JAl도 적용됨.
        register_file[MEM_WB.RegisterRd]=MEM_WB.RegisterRd_regi;
    }
    if(n!=-1) n--;//wb 수행 후에 instr 수행 끝
}

void pipeline_exe(){
    PC=0x400000;
    if(n==0) { //option -n 0일때
        if(option["-m"]!="") m_print();
        cout<<"===== Completion cycle: "<<cycle<<" ====="<<endl<<endl;
        cout<<"Current pipeline PC state:"<<endl;
        cout<<"{||||}"<<endl<<endl;
        regi_print();
        return;
    }
    else if(n>0){ //option -n 이 있을 때
        while(n!=0){
            WB_stage(); //data forwarding을 고려하기 위해
            MEM_stage();
            EX_stage();
            ID_stage();
            IF_stage(); //IF~WB 까지 가야 instr 하나 끝나
            cycle++; //1사이클 증가
            output();
        }
    }
    else if(n==-1){
        while (last_signal<4){
            WB_stage(); //data forwarding을 고려하기 위해
            MEM_stage();
            EX_stage();
            ID_stage();
            IF_stage(); //IF~WB 까지 가야 instr 하나 끝나
            cycle++; //1사이클 증가
            output();
        }
    }
}

int main(int argc, char* argv[]) {
    Init();
    int i=1;
    map<string,string>::iterator itr;
    int argc_=argc;
    if (argv[argc-1][strlen(argv[argc-1])-1]!='o'){
        cout<<"Exception : Please, type the binary file name."<<endl;
        return 0;
    }
    while(--argc_!=0){
        itr=option.find(argv[i]); //해당 key값을 찾아
        // argv[i]는 포인터임 *char 타입
        if (strcmp(argv[i],"-n")==0){
            n=stoi(argv[++i]);
            check++;
            continue;
        }
        if (itr==option.end()) break; //해당 key가 없음//sample.o만나면 그만
        if (itr->first=="-m"){
            itr->second=argv[++i];//해당 key의 value에 그다음 값을 넣어
            i++;
            check++;
            continue;
        }
        //존재한다는 얘기
        option[argv[i]]="1"; //1 flag : option 존재한다.
        check++;
        i++;
    }
    if (option["-atp"]=="" and option["-antp"]==""){
        cout<<"Exception : Please, type the branch prediction action."<<endl;
        return 0;
    }
    read_file(argv[argc-1]);
    pipeline_exe();
    return 0;
}
