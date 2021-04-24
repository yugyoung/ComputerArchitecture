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

using namespace std;

static map<string,int> R;
static map<string,int> I;
void Init(){
    //R - function field
    R["100001"]=0; //ADDU
    R["100100"]=1; //AND
    R["001000"]=2; //JR
    R["100111"]=3; //NOR
    R["100101"]=4; //OR
    R["101011"]=5; //SLTU
    R["000000"]=6; //SLL
    R["000010"]=7; //SRL
    R["100011"]=8; //SUBU
    //I - opcode
    I["001001"]=0;//ADDIU
    I["001100"]=1;//ANDI
    I["000100"]=2;//BEQ
    I["000101"]=3;//BNE
    I["001111"]=4;//LUI
    I["100011"]=5;//LW
    I["100000"]=6;//LB
    I["001101"]=7;//ORI
    I["001011"]=8;//SLTIU
    I["101011"]=9;//SW
    I["101000"]=10;//SB
}

class MIPS{
    map<string,string> memory_data;
    map<string,string> memory_text;
    map<int,string> register_file;
    string PC="0x400000";
    string PC_fake="0x10000000";
public:
    map<string,string> option;
    MIPS(){
        //register_file setting
        for(int i=0;i<32;i++) register_file[i]="0x0";
        option["m"]="";
        option["d"]="";
        option["n"]="";
    };
    ~MIPS(){};

    void read_file(string fname){
        ifstream fileopen;
        fileopen.open(fname,ios::in); //sample.o열어
        char low[256];
        if (fileopen.is_open()) {
            while (!fileopen.eof()) //파일 끝까지 읽었는지 확인
            {
                //memory_text setting
                fileopen.getline(low, 256);
                string word_text;
                stringstream sample_stream; //stringstream도 초기화.
                sample_stream.str(low);
                sample_stream >> word_text;
                long long num_text=strtoll(word_text.c_str(),NULL,0);
                long long i=num_text/4 ; //생성 개수
                long long same_i=i;
                //memory_data setting
                fileopen.getline(low,256);
                string word_data;
                stringstream sample_stream_data;
                sample_stream_data.str(low);
                sample_stream_data>>word_data;
                long long num_data=strtoll(word_data.c_str(),NULL,0);
                long long j=num_data/4; //data 생성개수
                long long same_j=j;
                // while() n개수만큼만 실행해야해
                while(i!=0){
                    fileopen.getline(low, 256); //한줄씩 읽어오기 , 이전에 읽었던 줄은 날아가버려
                    string word; //while안에서만 생성되는 word
                    stringstream sample_stream; //stringstream도 초기화.
                    sample_stream.str(low);
                    sample_stream >> word;
                    long long decimal=strtoll(word.c_str(),NULL,0);
                    string bincode_text=std::bitset<32>(decimal).to_string();
                    if(same_i!=i) {
                        long long PC_dec =strtoll(PC.c_str(), NULL, 0) + 4;
                        stringstream PC_hex;
                        PC_hex <<"0x"<< std::hex << PC_dec;
                        memory_text[PC_hex.str()] = bincode_text;
                        PC = PC_hex.str();
                        i--;
                    }
                    else{
                        memory_text[PC]=bincode_text;
                        i--;
                    }
                }
                while(j!=0){
                    fileopen.getline(low, 256); //한줄씩 읽어오기 , 이전에 읽었던 줄은 날아가버려
                    string word; //while안에서만 생성되는 word
                    stringstream sample_stream; //stringstream도 초기화.
                    sample_stream.str(low);
                    sample_stream >> word;
                    if(same_j!=j){
                        long long PC_dec=strtoll(PC_fake.c_str(),NULL,0)+4;
                        stringstream PC_hex;
                        PC_hex<<"0x"<<std::hex<<PC_dec;
                        memory_data[PC_hex.str()]=word; // 16진수로 그대로 저장.
                        PC_fake=PC_hex.str();
                        j--;
                    }
                    else{
                        memory_data[PC_fake]=word;
                        j--;
                    }
                }// while(j!=0) end
            } //while (!fileopen.eof())  end
        }//if (fileopen.is_open()) end
    }
    void PC_increase(){
        long long PC_incr=strtoll(PC.c_str(),NULL,0)+4;
        stringstream PC_hex;
        PC_hex<<"0x"<<std::hex<<PC_incr;
        PC=PC_hex.str();
    }
    string hex_increase(string hex){
        long long hex_dec=strtoll(hex.c_str(),NULL,0)+4;
        stringstream hex_;
        hex_<<"0x"<<std::hex<<hex_dec;
        return hex_.str();
    }
    long long bin2dec(string bin){
        long long dec=strtoll(bin.c_str(),NULL,2);
        return dec;
    }
    string bin2hex(string binary){
        long long dec_=bin2dec(binary);
        stringstream hex;
        hex<<"0x"<<std::hex<<dec_;
        return hex.str();
    }
    long long hex2dec(string hex){
        long long hex_dec=strtoll(hex.c_str(),NULL,0);
        return hex_dec;
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

    string dec2hex(long long dec){
        stringstream hex; //양, 음 구분해서 받아
        hex<<"0x"<<std::hex<<dec;
        return hex.str();
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
    string hex2combin(string hex){
        long long dec=hex2dec(hex);
        dec*=-1;
        string com_bin=std::bitset<32>(dec).to_string();
        return com_bin;
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

    void R_emulate(string rs, string rt, string rd,string shamt, string func){
        long long rs_dec=strtoll(register_file[stoi(rs,NULL,2)].c_str(),NULL,0);
        long long rt_dec=strtoll(register_file[stoi(rt,NULL,2)].c_str(),NULL,0);
        switch (R[func]){
            case 0 : {//ADDU(withoug overflow)
                long long addu=(rs_dec+rt_dec)%0x100000000;
                register_file[stoi(rd,NULL,2)]=dec2hex(addu);
                PC_increase();
                break;
            }
            case 1:{//AND
                long long AND=rs_dec&rt_dec; //bitwise AND
                register_file[stoi(rd,NULL,2)]=dec2hex(AND);
                PC_increase();
                break;
            }
            case 2:{//JR // rs에 들어있는 명령어의 주소로 무조건 분기(jump)
                PC=register_file[stoi(rs,NULL,2)];
                break;
            }
            case 3:{//NOR
                int nor=~(rs_dec|rt_dec);
                stringstream NOR;
                NOR<<"0x"<<std::hex<<nor;
                register_file[stoi(rd,NULL,2)]=NOR.str();
                PC_increase();
                break;
            }
            case 4:{//OR
                //string binrs=hex2bin(register_file[stoi(rs,NULL,2)]);
                //string binrt=hex2bin(register_file[stoi(rt,NULL,2)]);
                long long OR=rs_dec|rt_dec;
                register_file[stoi(rd,NULL,2)]=dec2hex(OR);
                PC_increase();
                break;
            }
            case 5:{//SLTU//unsigned
                if(rs_dec<rt_dec){
                    register_file[stoi(rd,NULL,2)]="0x1";
                }
                else{
                    register_file[stoi(rd,NULL,2)]="0x0";
                }
                PC_increase();
                break;
            }
            case 6:{//SLL
                unsigned int i = stoi(shamt,NULL,2);
                string bin_rt=hex2bin(register_file[stoi(rt,NULL,2)]);
                if(bin_rt.size()+i>32){//32bit 넘어가면
                    bin_rt=bin_rt.substr(i);
                    for(i; i!=0;i--){
                        bin_rt+="0";
                    }
                    register_file[stoi(rd,NULL,2)]=bin2hex(bin_rt);
                }
                else{
                    rt_dec=rt_dec<<i;
                    register_file[stoi(rd,NULL,2)]=dec2hex(rt_dec);
                }
                PC_increase();
                break;
            }
            case 7:{//SRL
                unsigned int i = stoi(shamt,NULL,2);
                string bin_rt=hex2bin(register_file[stoi(rt,NULL,2)]);
                if(bin_rt.size()+i>32){
                    string front;
                    bin_rt=bin_rt.substr(0,bin_rt.size()-i);
                    for(i; i!=0; i--){
                        front+="0";
                    }
                    front+=bin_rt;
                    register_file[stoi(rd,NULL,2)]=bin2hex(front);
                }
                else{
                    rt_dec=rt_dec>>i;
                    register_file[stoi(rd,NULL,2)]=dec2hex(rt_dec);
                }
                PC_increase();
                break;
            }
            case 8:{//SUBU //$rs+(-$rt)
                string com_bin=hex2combin(register_file[stoi(rt,NULL,2)]);
                string hex_diff=(dec2hex((rs_dec+bin2dec(com_bin))%0x100000000));
                register_file[stoi(rd,NULL,2)]=hex_diff;
                PC_increase();
                break;
            }
        }
    }
    void I_emulate(string opcode, string rs, string rt, string off){
        //rs, rt 양수로 취급.
        long long rs_dec=strtoll(register_file[stoi(rs,NULL,2)].c_str(),NULL,0);
        long long rt_dec=strtoll(register_file[stoi(rt,NULL,2)].c_str(),NULL,0);
        switch(I[opcode]){
            case 0:{//ADDIU // without overfow
                string se_imm;
                for(int i=0; i!=16; i++){//sign-extended
                    se_imm+=off[0];
                }
                register_file[stoi(rt,NULL,2)]= //rs에 있는 값을 양수로봐.
                        dec2hex(((rs_dec+bin2dec_sign(se_imm+off))%0x100000000));
                PC_increase();
                break;
            }
            case 1:{//ANDI
                string ze_imm;
                for(int i=0;i!=16;i++) ze_imm+="0";
                register_file[stoi(rt,NULL,2)]=
                        dec2hex(((rs_dec)&bin2dec(ze_imm+off)));//imm을 unsigned로 취급
                PC_increase();
                break;
            }
            case 2:{//BEQ
                if(register_file[stoi(rs,NULL,2)]==register_file[stoi(rt,NULL,2)]){
                    PC=dec2hex(hex2dec(PC)+(bin2dec(off)*4)+4);//PC+offset*4+4//offset 16bit
                }
                else{
                    PC_increase();
                }
                break;
            }
            case 3:{//BNE
                if(register_file[stoi(rs,NULL,2)]!=register_file[stoi(rt,NULL,2)]){
                    PC=dec2hex(hex2dec(PC)+(bin2dec(off)*4)+4); //off는 양수만
                }
                else {
                    PC_increase();
                }
                break;
            }
            case 4:{//LUI
                register_file[stoi(rt,NULL,2)]=bin2hex(off+"0000000000000000");
                PC_increase();
                break;
            }
            case 5:{//LW //현재 memory_data에 없는 주소일때는 고려 x
                long long addr=rs_dec+bin2dec(off);
                int index=addr%4;
                if(auto ltr=memory_data.find(dec2hex(addr))==memory_data.end()){
                    //주소에 없는 경우에서 들고오는건 생각안해.
                    string value1=hex2bin(memory_data[dec2hex(addr-index)]);
                    value1=unsign_ext(value1);
                    string value2=hex2bin(memory_data[dec2hex(addr+(4-index))]);//다음값
                    value1=unsign_ext(value2);
                    string fourwordvalue=value1.substr(index*8)+value2.substr(0,(4-index)*8);
                    register_file[stoi(rt,NULL,2)]=bin2hex(fourwordvalue);
                }
                else{
                    register_file[stoi(rt,NULL,2)]=
                            memory_data[dec2hex(rs_dec+bin2dec(off))];
                }
                PC_increase();
                break;
            }
            case 6:{//LB
                string se_imm;
                string bin;
                string word_value;
                long long addr=rs_dec+bin2dec(off);
                int index=addr%4; //주소 나머지//index=1,2,3
                if(auto itr=memory_data.find(dec2hex(addr))==memory_data.end()){
                    //못 찾았을 경우.
                    string value=hex2bin(memory_data[dec2hex(addr-index)]); //4Byte들어고와
                    value=unsign_ext(value);
                    word_value=value.substr(index*8,8);
                }
                else{
                    bin=memory_data[dec2hex(rs_dec+bin2dec(off))];
                    word_value=hex2bin("0x"+bin.substr(2,2));
                }
                register_file[stoi(rt,NULL,2)]=bin2hex(sign_ext(word_value));
                PC_increase();
                break;
            }
            case 7:{//ORI
                string ze_imm;
                for(int i=0;i!=16;i++) ze_imm+="0";
                register_file[stoi(rt,NULL,2)]=
                        dec2hex(((rs_dec)|bin2dec(ze_imm+off)));
                        PC_increase();
                break;
            }
            case 8:{//SLTIU
                string se_imm;
                for(int i=0; i!=16; i++){//sign-extended
                    se_imm+=off[0];
                }
                if(rs_dec<bin2dec_sign(se_imm+off)) {
                    register_file[stoi(rt,NULL,2)]="0x1";
                }
                else{
                    register_file[stoi(rt,NULL,2)]="0x0";
                }
                PC_increase();
                break;
            }
            case 9:{//SW
                long long addr=rs_dec+bin2dec(off);
                int index=addr%4;
                if(addr%4!=0){ //주소가 4의 배수를 가리키고 있지 않을 때.
                    if(auto itr=memory_data.find(dec2hex(addr-index))==memory_data.end()){
                        memory_data[dec2hex(addr-index)]="0x0";
                        memory_data[dec2hex(addr+(4-index))]="0x0";
                    }
                    string value1=unsign_ext(hex2bin(memory_data[dec2hex(addr-index)]));//32bit
                    string value2=unsign_ext(hex2bin(memory_data[dec2hex(addr+(4-index))]));//32bit
                    string rt_value=unsign_ext(hex2bin(register_file[stoi(rt,NULL,2)]));//32bit
                    value1=value1.replace(index*8,(value1.size()-(index*8)),rt_value.substr(0,(value1.size()-(index*8))));
                    value2=value2.replace(0,(4-index)*8,rt_value.substr((value1.size()-(index*8))));
                    memory_data[dec2hex(addr-index)]=bin2hex(value1);
                    memory_data[dec2hex(addr+(4-index))]=bin2hex(value2);
                }
                else{
                    memory_data[dec2hex(addr)]=
                            register_file[stoi(rt,NULL,2)];
                }
                PC_increase();
                break;
            }
            case 10:{//SB
                string bin=unsign_ext(hex2bin(register_file[stoi(rt,NULL,2)])); //rt에 있는 값.
                long long addr=rs_dec+bin2dec(off);
                int index = addr%4;
                if(addr%4!=0){ //4단위가 아닐때
                    if(auto itr=memory_data.find(dec2hex(addr-index))==memory_data.end()){
                        memory_data[dec2hex(addr-index)]="0x0";
                    }
                    string value_bin = unsign_ext(hex2bin(memory_data[dec2hex(addr-index)]));
                    value_bin=value_bin.replace(index*8,8,bin.substr(bin.size()-8));
                    memory_data[dec2hex(addr-index)]=bin2hex(value_bin);
                }
                else{
                    if(auto itr=memory_data.find(dec2hex(addr))==memory_data.end()){ //저장되어있지 않은 메모리라면, 0x0으로 초기화 후 저장
                        memory_data[dec2hex(addr)]="0x0";
                    }
                    string bin_or=unsign_ext(hex2bin(memory_data[dec2hex(addr)]));
                    memory_data[dec2hex(addr)]=
                            bin2hex(bin.substr(bin.size()-8)+bin_or.substr(8));
                }
                PC_increase();
                break;
            }
        }
    }
    //bincode 한 줄씩 처리
    void emulate(string bin_text) {
        string opcode,rs,rt,rd,shamt,offset;
        if (bin_text.substr(0, 6) == "000000") {
            //R형식
            rs=bin_text.substr(6,5);
            rt=bin_text.substr(11,5);
            rd=bin_text.substr(16,5);
            shamt=bin_text.substr(21,5);
            return R_emulate(rs,rt,rd,shamt,bin_text.substr((bin_text.size()-6)));
        }
        else if(bin_text.substr(0,6)=="000010"){ //Jformat-J
            PC=dec2hex((hex2dec(PC)&0xF0000000)|(bin2dec(bin_text.substr(6))*4));
        }
        else if(bin_text.substr(0,6)=="000011"){ //Jformat-JAL
            long long pc_dec=strtoll(PC.c_str(),NULL,0)+4;
            register_file[31]=dec2hex(pc_dec); //$ra($31)에 PC+4 저장
            PC=dec2hex(( hex2dec(PC)&0xF0000000)|(bin2dec(bin_text.substr(6))*4));
        }
        else{
            opcode=bin_text.substr(0,6);
            rs=bin_text.substr(6,5);
            rt=bin_text.substr(11,5);
            offset=bin_text.substr(16);
            return I_emulate(opcode,rs,rt,offset);
        }
        return;
    }

    void emulater()
    {
        PC="0x400000"; //PC값 초기화.
        if(option["n"]==""){ //n option없을 때
            auto itr=memory_text.find(PC);
            while(itr!=memory_text.end()){ //해당 PC값을 못찾을 때 까지 반복
                emulate(memory_text[PC]);
                output(1); //아무 num_n이나 보내 <--마무리 안되었다는 신호.
                itr=memory_text.find(PC);
            }
            if(option["d"]==""){
                output(0);
            }
        }
        else{
            unsigned int num_n=stoull(option["n"]);
            if(num_n==0) return output(num_n);
            while(num_n!=0){
                if(auto itr=memory_text.find(PC)!=memory_text.end()) {
                    emulate(memory_text[PC]);
                }
                output(num_n);
                num_n--;
            }
            if(option["d"]==""){
                output(0);
            }
        }
    }

    void output(unsigned int num_n){
        if(option["d"]!="" and num_n==0){ //처음부터 n=0이고 -d옵션이 있는 경우
            if(option["m"]!=""){
                cout<<"Memory content ["<<option["m"].substr(0,option["m"].find(":"))<<".."<<option["m"].substr(option["m"].find(":")+1)<<"]:"<<endl;
                cout<<"--------------------------------------"<<endl;
                string m1=option["m"].substr(0,option["m"].find(':'));
                string m2=option["m"].substr(option["m"].find(':')+1);
                long long m1_dec=hex2dec(m1);
                long long m2_dec=hex2dec(m2)+4;
                //첫 시작주소부터 범위벗어났을 경우
                auto itr_d=memory_data.find(dec2hex(m1_dec));
                auto itr_t=memory_text.find(dec2hex(m1_dec));
                if(hex2dec(m1)<0x400000){
                    cout<<"out of start address"<<endl;
                    return;
                }
                if(hex2dec(m2)<hex2dec(m1)){
                    cout<<"range error"<<endl;
                    return;
                }
                for(m1_dec; m1_dec!=m2_dec; m1_dec+=4){
                    auto itr_d=memory_data.find(dec2hex(m1_dec));
                    auto itr_t=memory_text.find(dec2hex(m1_dec));
                    if(itr_d!=memory_data.end()){ //m1 key값을 가진 value 발견되면,
                        cout<<itr_d->first<<": "<<itr_d->second<<endl;
                    }
                    else if(itr_t!=memory_text.end()){
                        cout<<itr_t->first<<": "<<bin2hex(itr_t->second)<<endl;
                    }
                    else{
                        cout<<dec2hex(m1_dec)<<": 0x0"<<endl;
                    }
                }cout<<endl;
            }
            return;
        }
        if(option["d"]!=""){//inst마다 d옵션 output
            cout<<"Current register values: "<<endl;
            cout<<"----------------------------------------"<<endl;
            cout<<"PC : "<<PC<<endl;
            cout<<"Registers : "<<endl;
            for(auto low=register_file.begin();low!=register_file.end();low++){
                cout<<"R"<<low->first<<": "<<low->second<<endl;
            }
            cout<<endl;
            if(option["m"]!=""){
                cout<<"Memory content ["<<option["m"].substr(0,option["m"].find(":"))<<".."<<option["m"].substr(option["m"].find(":")+1)<<"]:"<<endl;
                cout<<"--------------------------------------"<<endl;
                string m1=option["m"].substr(0,option["m"].find(':'));
                string m2=option["m"].substr(option["m"].find(':')+1);
                long long m1_dec=hex2dec(m1);
                long long m2_dec=hex2dec(m2)+4;
                //첫 시작주소부터 범위벗어났을 경우
                auto itr_d=memory_data.find(dec2hex(m1_dec));
                auto itr_t=memory_text.find(dec2hex(m1_dec));
                if(hex2dec(m1)<0x400000){
                    cout<<"out of start address"<<endl;
                    return;
                }
                if(hex2dec(m2)<hex2dec(m1)){
                    cout<<"range error"<<endl;
                    return;
                }
                for(m1_dec; m1_dec!=m2_dec; m1_dec+=4){
                    auto itr_d=memory_data.find(dec2hex(m1_dec));
                    auto itr_t=memory_text.find(dec2hex(m1_dec));
                    if(itr_d!=memory_data.end()){ //m1 key값을 가진 value 발견되면,
                        cout<<itr_d->first<<": "<<itr_d->second<<endl;
                    }
                    else if(itr_t!=memory_text.end()){
                        cout<<itr_t->first<<": "<<bin2hex(itr_t->second)<<endl;
                    }
                    else{
                        cout<<dec2hex(m1_dec)<<": 0x0"<<endl;
                    }
                }cout<<endl;
            }
        }
        else{ //최종 output
            if(num_n==0){//모든 inst처리 완료, d가 없음
                cout<<"Current register values: "<<endl;
                cout<<"----------------------------------------"<<endl;
                cout<<"PC : "<<PC<<endl;
                cout<<"Registers : "<<endl;
                for(auto low=register_file.begin();low!=register_file.end();low++){
                    cout<<"R"<<low->first<<": "<<low->second<<endl;
                }
                cout<<endl;
                if(option["m"]!=""){
                    cout<<"Memory content ["<<option["m"].substr(0,option["m"].find(":"))<<".."<<option["m"].substr(option["m"].find(":")+1)<<"]:"<<endl;
                    cout<<"--------------------------------------"<<endl;
                    string m1=option["m"].substr(0,option["m"].find(':'));
                    string m2=option["m"].substr(option["m"].find(':')+1);
                    long long m1_dec=hex2dec(m1);
                    long long m2_dec=hex2dec(m2)+4;
                    auto itr_d=memory_data.find(dec2hex(m1_dec));
                    auto itr_t=memory_text.find(dec2hex(m1_dec));
                    if(hex2dec(m1)<0x400000){
                        cout<<"out of start address"<<endl;
                        return;
                    }
                    if(hex2dec(m2)<hex2dec(m1)){
                        cout<<"range error"<<endl;
                        return;
                    }
                    for(m1_dec; m1_dec!=m2_dec; m1_dec+=4){
                        auto itr_d=memory_data.find(dec2hex(m1_dec));
                        auto itr_t=memory_text.find(dec2hex(m1_dec));
                        if(itr_d!=memory_data.end()){ //m1 key값을 가진 value 발견되면,
                            cout<<itr_d->first<<": "<<itr_d->second<<endl;
                        }
                        else if(itr_t!=memory_text.end()){
                            cout<<itr_t->first<<": "<<bin2hex(itr_t->second)<<endl;
                        }
                        else{
                            cout<<dec2hex(m1_dec)<<": 0x0"<<endl;
                        }
                    }
                }
            }
        }
    }
};//class MIPS endl



int main(int argc, char* argv[]) {
    Init();
    MIPS mips;
    int i=1;
    map<string,string>::iterator itr;
    int argc_=argc;
    while(--argc_!=0){
        const char* op=&argv[i][1]; //argv[i]는 char*, argv[i][1]는 char
        itr=mips.option.find(op); //해당 key값을 찾아
        if (itr==mips.option.end()) break; //해당 key가 없음//sample.o만나면 그만
        if (itr->first=="d") {
            i++;
            mips.option["d"]="1"; //들어왔다는 표시.
            continue; //d는 따로 숫자가 더 안옴.
        }
        itr->second=argv[++i];//해당 key의 value에 그다음 값을 넣어
        i++;
    }
    mips.read_file(argv[argc-1]);
    mips.emulater();

    return 0;
}
