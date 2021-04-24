#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <algorithm> //toupper
#include <bitset>
#include <iomanip>

using namespace std;

static map<string,int> instructions;
void Init(){
    instructions["ADDIU"] = 0;
    instructions["ADDU"] = 1;
    instructions["AND"] = 2;
    instructions["ANDI"] = 3;
    instructions["BEQ"] = 4;
    instructions["BNE"] = 5;
    instructions["J"] = 6;
    instructions["JAL"] = 7;
    instructions["JR"] = 8;
    instructions["LUI"] = 9;
    instructions["LW"] = 10;
    instructions["LA"] = 11;
    instructions["NOR"] = 12;
    instructions["OR"] = 13;
    instructions["ORI"] = 14;
    instructions["SLTIU"] = 15;
    instructions["SLTU"] = 16;
    instructions["SLL"] = 17;
    instructions["SRL"] = 18;
    instructions["SW"] = 19;
    instructions["SUBU"] = 20;
    instructions["LB"] = 21;
    instructions["SB"] = 22;
}


class MIPS{
    map<string,tuple<int,string>> memory_data; //<16진수,(int,16진수 string)>
    map<string,string> memory_text; //<16진수, instruction>
    map<string,string> label; // <label,16진수>
    string PC="0";

public:
    //constructor
    MIPS(){};
    //destructor
    ~MIPS(){};
    void data_store(string word){
        stringstream hex_address_data;
        unsigned int address_data;
        if(PC=="0"){ //.data가 .text보다 앞에있다고 가정.
            PC="0x10000000";
            const char* base_address=PC.c_str(); //16진수로 주소저장.
            address_data=(unsigned int)strtoul(base_address,NULL,0);
        }
        else{
            const char* base_address=PC.c_str();
            address_data=(unsigned int)strtoul(base_address,NULL,0)+4;
        }
        hex_address_data<<std::hex<<address_data;//10진수->16진수로 변환 후 저장
        PC="0x"+hex_address_data.str();

        if(word[1]=='x'){ //16진수일 때
            memory_data[PC]=make_tuple(NULL,word); //16진수 data string형식으로 저장
        }
        else { //10진수일 때
            memory_data[PC]=make_tuple(atoi(word.c_str()),"");//10진수로 저장
        }
    }
    void text_store(string word){
        stringstream hex_address_data;
        unsigned int address_data;
        if(memory_text.size()==0){
            PC="0x00400000";
            const char* base_address=PC.c_str(); //16진수로 주소저장.
            address_data=(unsigned int)strtoul(base_address,NULL,0);
        }
        else{
            const char* base_address=PC.c_str(); //16진수로 주소저장.
            address_data=(unsigned int)strtoul(base_address,NULL,0)+4; //4byte 증가,10진수
        }
        hex_address_data<<std::hex<<address_data;//10진수->16진수로 변환 후 저장
        PC="0x"+hex_address_data.str();
        memory_text[PC]=word; //한 줄 통째로 우선 저장
    }
    void text_store_label(string tt_label, string low){
        if(tt_label=="exit:"){
            const char* PC_next_char=PC.c_str();
            unsigned int PC_next=(unsigned int)strtoul(PC_next_char,NULL,0)+4; //다음주소 10진수
            stringstream hex_address_data;
            hex_address_data<<std::hex<<PC_next;//10진수->16진수로 변환 후 저장 //10진수 int-->16진수 string
            PC="0x"+hex_address_data.str();
            label["exit"]=PC;
        }
        else {
            text_store(low);
            tt_label=tt_label.substr(0,tt_label.size()-1);
            label[tt_label]=PC;
        }
    }
    void data_store_label(string lb, string word){
        data_store(word);
        label[lb]=PC; //<string, int>
    }
    void data_store_text(string lb, string text){
        text_store(text);
    }

    //파일 읽기 함수
    void read_file(string f_name) {
        ifstream fileopen;
        fileopen.open(f_name, ios::in);
        char low[256];
        string directive = "";
        string tt_label="";

        if (fileopen.is_open()) {
            while (!fileopen.eof()) //파일 끝까지 읽었는지 확인
            {
                fileopen.getline(low, 256); //한줄씩 읽어오기 , 이전에 읽었던 줄은 날아가버려
                string word;
                stringstream sample_stream;
                sample_stream.str(low);
                sample_stream >> word;
                if (word == ".data") {
                    directive = ".data";
                    continue; //다음 줄 받아오게 만들어
                }
                if (directive == ".data") {
                    if (word[word.size() - 1] == ':') {
                        string lb = word.substr(0, word.size() - 1); //lb=array저장
                        sample_stream >> word; //.word
                        sample_stream >> word; //3
                        data_store_label(lb, word);
                    }
                    if (word == ".word") { //else로 바꿔도 됨!!
                        sample_stream >> word;
                        data_store(word);
                    }
                }
                if (word == ".text") {
                    directive = ".text";
                    continue; //다음 줄 받아오게 만들어
                }
                if (directive == ".text") {
                    if (word[word.size() - 1] == ':') {
                        if(word=="exit:") {
                            tt_label="exit:";
                            text_store_label(tt_label,low);
                            break;
                        }
                        //sample_stream>>word; //다시 main: 꺼내와
                        string temp=word;// main: 저장
                        sample_stream>>word; //slitiu
                        if(temp==word){
                            tt_label=word;
                            continue; //while문을 다시 돌려
                        } //main: 과 한 줄에 있을 때.
                        tt_label=temp;
                        text_store_label(tt_label,low);
                        tt_label=""; //label을 처리했으니 reset시켜야지
                    }
                    else if(tt_label!=""){//main: 과 다른 줄에 있을 때.
                        text_store_label(tt_label,low);
                        tt_label="";
                    }
                    else{ //평소 줄
                        if(word=="") continue;//무시
                        //if(word=="la") text_store(low); //la면 두개만들어
                        text_store(low);
                    }
                }
            }
             //제일 마지막줄은 그냥 \n임.
        }//if(file open) end
        fileopen.close();
        changetobinary(); //changetobinary() 함수 자동호출
    }//read_file end
    //switch 구문
    string find_instruction(string key,string low){
        string text_key=key;
        stringstream ins;
        string w;
        string p1,p2,p3,p4;
        ins.str(low);
        ins>>w>>p1>>p2>>p3; //addiu //$2, // p는 register num(주소)
        //ins>>low; //         addiu   $2, $0, 1024
        if(w[w.size()-1]==':'){
            ins>>p4;
            w=p1;
            p1=p2;
            p2=p3;
            p3=p4;
        } //p3가 없을 수도 있음. 공백으로 저장됨.
        transform(w.begin(),w.end(),w.begin(), ::toupper); //instruction 대문자로
        string op,rt,rs,rd,imm,bin;
        unsigned int bin_label; //이 전에 쓰던 값으로 초기화되어있으니 잘 사용했는지 확인하기!!!!
        switch(instructions[w]) {
            case 0: {//ADDIU
                op = "001001";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());
                    imm = std::bitset<16>(nimm).to_string();
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<16>(p3_dec).to_string(); //확인해보기!!!
                }
                bin = op + rs + rt + imm;
                break;
            }
            case 1: { //ADDU
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //3 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3 = p3.substr(1, p3.size() - 1);
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op + rs + rt + rd + "00000100001";
                break;
            }
            case 2: { //AND
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3=p3.substr(1,p3.size()-1);
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op + rs + rt + rd + "00000100100";
                break;
            }
            case 3: { //ANDI
                op = "001100";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());
                    imm = std::bitset<16>(nimm).to_string();
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<16>(p3_dec).to_string(); //확인해보기!!!
                }
                bin = op + rs + rt + imm;
                break;
            }
            case 4: { //BEQ
                op = "000100";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rs = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rt = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                //여기들어온 low의 key값이 BEQ(현재) PC값임.
                //PC
                const char *p3_ch = label[p3].c_str();
                bin_label = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수->10진수
                const char *text_memory_key = text_key.c_str();
                unsigned int dec_key = (unsigned int) strtoul(text_memory_key, NULL, 0);
                unsigned int offset = (bin_label - dec_key - 4) / 4;
                imm = std::bitset<16>(offset).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm=offset
                break;
            }
            case 5: { //BNE
                op = "000101";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rs = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2);
                int np2 = atoi(p2.c_str());
                rt = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                //여기들어온 low의 key값이 BEQ(현재) PC값임.
                //PC
                const char *p3_ch = label[p3].c_str();
                bin_label = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수->10진수
                const char *text_memory_key = text_key.c_str(); //text_key는 해당 ins의 key값
                unsigned int dec_key = (unsigned int) strtoul(text_memory_key, NULL, 0);
                unsigned int offset = (bin_label - dec_key - 4) / 4;
                imm = std::bitset<16>(offset).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm=offset
                break;
            }
            case 6: { //J
                op = "000010";
                const char *p1_ch = label[p1].c_str();
                bin_label = ((unsigned int) strtoul(p1_ch, NULL, 0) / 4);
                imm = std::bitset<26>(bin_label).to_string();
                bin = op + imm;//imm==target
                break;
            }
            case 7: { //JAL
                op = "000011";
                const char *p1_ch = label[p1].c_str();
                bin_label = ((unsigned int) strtoul(p1_ch, NULL, 0) / 4);
                imm = std::bitset<26>(bin_label).to_string();
                bin = op + imm;//imm==target
                break;
            }
            case 8: { //JR
                op = "000000";
                p1 = p1.substr(1); //$제거
                int np = atoi(p1.c_str());
                rs = std::bitset<5>(np).to_string();
                bin = op + rs + "000000000000000001000";//imm==target
                break;
            }
            case 9: { //LUI
                op = "001111";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string();
                if (p2.substr(0, 2) != "0x") {
                    int np2 = atoi(p2.c_str());
                    imm = std::bitset<16>(np2).to_string();
                } else if (p2.substr(0, 2) == "0x") {
                    const char *p2_ch = p2.c_str();
                    unsigned int p2_dec = (unsigned int) strtoul(p2_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<16>(p2_dec).to_string(); //확인해보기!!!
                }
                bin = op + "00000" + rt + imm;
                break;
            }
            case 10: { //LW
                op = "100011";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string();
                string rs;
                rs = p2.substr(p2.find('$')); //원래 P2 : 0($8)-->$8)
                rs=rs.substr(1);//8)
                rs=rs.substr(0,rs.size()-1);//8
                int np2 = atoi(rs.c_str());
                rs = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                p3 = p2.substr(0, p2.find('('));//offset //p2.find()는 해당 index를 줌.
                int np3 = atoi(p3.c_str());
                imm = std::bitset<16>(np3).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm==offset
                break;
            }
            case 11: { //LA
                //p2로 주소값이 온다.
                string last;
                last = label[p2]; //substr은 뒤의 index포함x
                last = last.substr(last.size() - 4, last.size());//하위 4개 숫자
                if (last == "0000") {
                    //lui만
                    op = "001111";
                    p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                    int np = atoi(p1.c_str());
                    rt = std::bitset<5>(np).to_string(); //8->2진수 01000
                    //상수(address 직접적으로) 올수도, label이 올수도..!
                    string temp;
                    temp = label[p2].substr(0, 6); //var 주소 0x1000 //data주소는 0x1000부터라서 0x004걱정안해도 됨.
                    const char *t = temp.c_str();
                    unsigned int hexnum = (unsigned int) strtoul(t, NULL, 0); //주소 16진수 --> 10진수변환
                    imm = std::bitset<16>(hexnum).to_string();//10진수-->2진수 변환
                    bin = op + "00000" + rt + imm;//rs=00000
                } else {
                    //lui
                    op = "001111";
                    p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                    int np = atoi(p1.c_str());
                    rt = std::bitset<5>(np).to_string(); //8->2진수 01000
                    //상수(address 직접적으로) 올수도, label이 올수도..!
                    string temp;
                    temp = label[p2].substr(0, 6); //var 주소 0x1000 //data주소는 0x1000부터라서 0x004걱정안해도 됨.
                    const char *t = temp.c_str();
                    unsigned int hexnum = (unsigned int) strtoul(t, NULL, 0); //주소 16진수 --> 10진수변환
                    imm = std::bitset<16>(hexnum).to_string();//10진수-->2진수 변환
                    bin = "/" + op + "00000" + rt + imm;//rs=00000
                    //ori
                    op = "001101";
                    rs = rt;
                    temp = "0x" +label[p2].substr(6, label[p2].size()); //var 주소 0x1000 //data주소는 0x1000부터라서 0x004걱정안해도 됨.
                    t = temp.c_str();
                    hexnum = (unsigned int) strtoul(t, NULL, 0); //주소 16진수 --> 10진수변환
                    imm = std::bitset<16>(hexnum).to_string();//10진수-->2진수 변환
                    bin += op + rs + rt + imm;//rs=00000
                }
                break;
            }
            case 12: { //NOR
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2);
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3 = p3.substr(1, p3.size() - 1);//2
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op + rs + rt + rd + "00000100111";
                break;
            }
            case 13: { //OR
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //갯수를 들고오는데 앞에 $빼고 뒤에 , 빼니까 2를 빼야해!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2);
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3 = p3.substr(1, p3.size()-1);//2
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op + rs + rt + rd + "00000100101";
                break;
            }
            case 14:{//ORI //imm에 label이 안들어올거라고 가정.
                op = "001101";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());
                    imm = std::bitset<16>(nimm).to_string();
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<16>(p3_dec).to_string(); //확인해보기!!!
                }
                bin = op + rs + rt + imm;
                break;
            }
            case 15:{//SLTIU //imm에 label이 안들어올거라고 가정.
                op = "001011";
                p1 = p1.substr(1, p1.size() - 2); //2 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());
                    imm = std::bitset<16>(nimm).to_string();
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<16>(p3_dec).to_string(); //확인해보기!!!
                }
                bin = op + rs + rt + imm;
                break;
            }
            case 16:{ //SLTU
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //3 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3 = p3.substr(1, p3.size() - 1);//2
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op + rs + rt + rd + "00000101011";
                break;
            }
            case 17:{//SLL
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //3 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rt = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());//음수도 처리완료
                    imm = std::bitset<5>(nimm).to_string(); //imm=shamt
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<5>(p3_dec).to_string(); //확인해보기!!!
                } else { //imm 주소값

                }
                bin = op +"00000" + rt + rd+imm + "000000";
                break;
            }
            case 18:{//SRL
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //3 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rt = std::bitset<5>(np2).to_string();
                if (p3.substr(0, 2) != "0x") { //imm 10진수
                    int nimm = atoi(p3.c_str());
                    imm = std::bitset<5>(nimm).to_string(); //imm=shamt
                } else if (p3.substr(0, 2) == "0x") { //imm 16진수
                    const char *p3_ch = p3.c_str();
                    unsigned int p3_dec = (unsigned int) strtoul(p3_ch, NULL, 0); //16진수 -->10진수
                    imm = std::bitset<5>(p3_dec).to_string(); //확인해보기!!!
                } else { //imm 주소값

                }
                bin = op +"00000" + rt + rd+imm + "000010";
                break;
            }
            case 19:{ //SW
                op = "101011";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string();
                string rs;
                rs = p2.substr(p2.find('$')); //원래 P2 : 0($8)-->8
                rs=rs.substr(1);
                rs=rs.substr(0,rs.size()-1);
                int np2 = atoi(rs.c_str());
                rs = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                p3 = p2.substr(0, p2.find('('));//offset
                int np3 = atoi(p3.c_str());
                imm = std::bitset<16>(np3).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm==offset
                break;
            }
            case 20:{//SUBU
                op = "000000";
                p1 = p1.substr(1, p1.size() - 2); //3 //왜 -2지?!!!!!
                int np = atoi(p1.c_str());
                rd = std::bitset<5>(np).to_string(); //00010
                p2 = p2.substr(1, p2.size() - 2); //2 //왜 -2지?!!!!!
                int np2 = atoi(p2.c_str());
                rs = std::bitset<5>(np2).to_string();
                p3 = p3.substr(1, p3.size() - 1);//2
                int nimm = atoi(p3.c_str());
                rt = std::bitset<5>(nimm).to_string();
                bin = op +rs+rt+rd+"00000100011";
                break;
            }
            case 21:{ //LB
                op = "100000";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string();
                string rs;
                rs = p2.substr(p2.find('$')); //원래 P2 : 0($8)-->8
                rs=rs.substr(1);
                rs=rs.substr(0,rs.size()-1);
                int np2 = atoi(rs.c_str());
                rs = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                p3 = p2.substr(0, p2.find('('));//offset
                int np3 = atoi(p3.c_str());
                imm = std::bitset<16>(np3).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm==offset
                break;
            }
            case 22:{ //SB
                op = "101000";
                p1 = p1.substr(1, p1.size() - 2);
                int np = atoi(p1.c_str());
                rt = std::bitset<5>(np).to_string();
                string rs;
                rs = p2.substr(p2.find('$')); //원래 P2 : 0($8)-->8
                rs=rs.substr(1);
                rs=rs.substr(0,rs.size()-1);
                int np2 = atoi(rs.c_str());
                rs = std::bitset<5>(np2).to_string();
                //라벨 찾아야해
                p3 = p2.substr(0, p2.find('('));//offset
                int np3 = atoi(p3.c_str());
                imm = std::bitset<16>(np3).to_string(); // imm=ofset!!!
                bin = op + rs + rt + imm;//imm==offset
                break;
            }
        }
        return bin;

    }

    //memory_text :instruction -->Binary
    void changetobinary(){
        string born_key,add,born,born_val;
        for(auto low=memory_text.begin(); low!=memory_text.end();low++){
            string bincode=find_instruction(low->first,low->second); //bincode에 bin리턴
            //lA는 bin값이 두개합쳐져있음.
            if(bincode[0]=='/'){ //la의 하위 ori만집중한것
                bincode=bincode.substr(1,bincode.size());
                memory_text[low->first]=bincode.substr(0,32); //32bit
                low++;//다음 low로//현재 iterator의 뒷부분부터 PC값 수정(뒤로 +4씩)
                born_key=low->first; //원래 현재 key
                born=low->second; //원래 현재 value //born에 저장되어있어!!
                low->second=bincode.substr(32,bincode.size());
                low++;
                for(auto after_low=low;after_low!=memory_text.end();after_low++){
                    born_val=after_low->second; //원래 value저장
                    memory_text[after_low->first]=born;
                    born=born_val;//여기서 after_low는 add(bin)으로 바뀐상황.
                }
                text_store(born);
                low--;
                low--;
                //label 주소도 한칸씩 빌려나야해! la뒤에 등장하는 label들만!
                for(auto low_lb=label.begin(); low_lb!=label.end();low_lb++){
                    //16진수 주소 10진법으로 바꾸기
                    unsigned int lb_key=(unsigned int)strtoul(low_lb->second.c_str(),NULL,0);
                    unsigned int la_key=(unsigned int)strtoul(low->first.c_str(),NULL,0);
                    if(lb_key>la_key){ // 같을때는 생각ㄴㄴ //lb와 주소가 크면!! lb뒤에 저장된거니까!
                        lb_key+=4;
                        stringstream hex_lb;
                        hex_lb<<std::hex<<lb_key;
                        //label.erase(low_lb->first); //value값(PC값)을 변경하는 것이므로 삭제할 필요 x.
                        label[low_lb->first]="0x"+hex_lb.str();
                    }
                }
                low++; //la뒤에꺼는 넘겨야하니까!
            }
            else {
                memory_text[low->first]=bincode;
            }

        }
    }

    //file 생성
    void output_file(string filename){
        ofstream fileout(filename.substr(0,filename.size()-2)+".o");
        string word;
        if(fileout.is_open()){
            //1.<text section size>
            int size=memory_text.size()*4; // 10진수
            stringstream hex;
            hex<<std::hex<<size; //10진수-->16진수 //2진수로 변환거치지않고 바로
            string size_hex="0x"+hex.str();
            fileout<<size_hex<<"\n"; // <<operantor는 stringstream안받아
            //2.<data section size>
            size=memory_data.size()*4;
            stringstream hex2;
            hex2<<std::hex<<size;
            size_hex="0x"+hex2.str();
            fileout<<size_hex<<"\n";
            //3.instruction
            for(auto low=memory_text.begin(); low!=memory_text.end();low++){
                string binary=low->second;//2진수
                unsigned int bin_dec=(unsigned int)strtoul(binary.c_str(),NULL,2);//10진수
                stringstream hex3;
                hex3<<std::hex<<bin_dec;
                size_hex="0x"+hex3.str();
                fileout<<size_hex<<"\n";
            }
            for(auto low=memory_data.begin(); low!=memory_data.end();low++){
                if(get<1>(low->second)==""){ //10진수인 경우
                    stringstream dec;
                    dec<<std::setbase(16)<<get<0>(low->second);
                    fileout<<"0x"<<dec.str()<<"\n";
                    continue; //십진수가 0으로 저장되었을 경우 밑에를 통과할수있으니 넘겨버려!
                }
                else{ //16진수일 경우, 그대로 넣어 //==0으로 바꿔야하나?!
                    stringstream hex_data;
                    string bin_data=get<1>(low->second);
                    unsigned int data_dec=(unsigned int)strtoul(bin_data.c_str(),NULL,0);
                    hex_data<<"0x"<<std::hex<<data_dec;
                    fileout<<hex_data.str()<<"\n";
                }
            }
        }//if end
        fileout.close();
    }
};


int main(int argc, char* argv[]) {
    Init(); //instruction setting
    MIPS sample_mips;
    sample_mips.read_file(argv[1]);//나중에 param으로 파일명전달!!!!
    sample_mips.output_file(argv[1]);
    cout<<"success"<<endl;
    return 0;
}
