#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <algorithm> //toupper
#include <bitset>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
using namespace std;

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
    for(int i=0; i!=hex.length(); i++) {
        bin+=hex2bin_char(hex[i]);
    }
    return bin;
}
string _ext(string bin){
    string bin_;
    if(bin.size()!=64){
        for(int i=0; i!=64-bin.size();i++){
            bin_+=bin[0];
        }
        bin_+=bin;
    }
    return bin_;
}
string dec2hex(long long dec){
    stringstream hex; //양, 음 구분해서 받아
    hex<<"0x"<<std::hex<<dec;
    return hex.str();
}

int main(int argc, char* argv[]) {
    int dirty,num_bloffset,num_index,num_tag,capacity,way,bl_size,t_acc,rd_acc,wr_acc,rd_miss,wr_miss,c_evict,d_evict = 0;
    map<unsigned long long,map<unsigned long long,int>> cache; //<set index<tag,itr(LRU)+ +dirtybit>>>
    map<string, int> cache_ways; //<tag,dirty>
    string filename=argv[argc-1];
    string address;//binary
    if (argc==8){
        capacity=atoi(argv[2]);
        way=atoi(argv[4]);
        bl_size=atoi(argv[6]);
        num_index=log((capacity*pow(2,10))/(bl_size*way))/log(2);
        num_bloffset=(log(bl_size)/log(2));
        num_tag=64-num_index-num_bloffset;
        ifstream fileopen;
        fileopen.open(filename, ios::in);
        char low[256];
        int iter=0;
        rd_miss=0;
        wr_miss=0;
        unsigned long long min_tag;
        stringstream ss;
        string w;
        int min_dirty;
        int min;
        wr_acc=0;
        rd_acc=0;
        string word_;
        int recent;
        if (fileopen.is_open()) {
            while (!fileopen.eof())
            {
                fileopen.getline(low, 256);
                string word;
                stringstream sample_stream;
                sample_stream.str(low);
                sample_stream >> word;
                unsigned long long index;
                unsigned long long tag;
                if(word=="W"){
                    wr_acc++;
                    sample_stream>>word;
                    address=_ext(hex2bin(word));
                    index=stoull(address.substr(num_tag,num_index),NULL,2);
                    tag=stoull(address.substr(0,num_tag),NULL,2);
                    if(cache[index].find(tag)==cache[index].end()) {
                        wr_miss++; //write miss
                        if(cache[index].size()>=way){
                            int j=0;
                            for (auto __tag=cache[index].begin(); __tag!=cache[index].end(); __tag++)
                            {
                                recent=__tag->second; //LRU(iter)
                                if(j==0){
                                    min=recent;
                                    min_tag=__tag->first;
                                    j++;
                                    continue;
                                }
                                if(min>recent) {
                                    min=recent;
                                    min_tag=__tag->first;
                                }
                            }
                            if(cache_ways[to_string(min_tag)+to_string(index)]==1) d_evict++;
                            else if (cache_ways[to_string(min_tag)+to_string(index)]==0) c_evict++;
                            cache[index].erase(min_tag); //erase 수정해야해
                            cache_ways.erase(to_string(min_tag)+to_string(index));
                        }
                    }
                    cache[index][tag]=iter; //최근 사용으로 변경
                    cache_ways[to_string(tag)+to_string(index)]=1;
                }
                else if(word=="R"){
                    rd_acc++;
                    sample_stream>>word;
                    address=_ext(hex2bin(word));
                    index=stoull(address.substr(num_tag,num_index),NULL,2);
                    tag=stoull(address.substr(0,num_tag),NULL,2);
                    if(cache[index].find(tag)==cache[index].end()) {
                        rd_miss++;
                        if(cache[index].size()>=way){ //꽉찼다는 거니까
                            int j=0;
                            for (auto __tag=cache[index].begin(); __tag!=cache[index].end(); __tag++)
                            {
                                recent=__tag->second;
                                if(j==0){
                                    min=recent;
                                    min_tag=__tag->first;
                                    j++;
                                    continue;
                                }
                                if(min>recent) {
                                    min=recent;
                                    min_tag=__tag->first;
                                }
                            }
                            if(cache_ways[to_string(min_tag)+to_string(index)]==1) d_evict++;
                            else if (cache_ways[to_string(min_tag)+to_string(index)]==0) c_evict++;
                            cache[index].erase(min_tag); //erase 수정해야해
                            cache_ways.erase(to_string(min_tag)+to_string(index));
                        }
                        cache[index][tag]=iter;
                        cache_ways[to_string(tag)+to_string(index)]=0;
                    }
                    else cache[index][tag]=iter;
                }
                iter++;
            }
        }
        unsigned int checksum = 0;
        unsigned long long cs_tag;
        unsigned long long cs_index;
        unsigned long long cs_dirty_bit;
        for (auto _index = cache.begin(); _index != cache.end(); _index++ )
        {
            cs_index = _index->first;
            for (auto _tag=_index->second.begin(); _tag!=_index->second.end(); _tag++){
                cs_tag = _tag->first;
                cs_dirty_bit=cache_ways[to_string(cs_tag)+to_string(cs_index)];
                checksum = checksum ^ (((cs_tag^cs_index) << 1) | cs_dirty_bit);
            }
        }
        ofstream fileout(filename.substr(0,filename.size()-4)+"_"+to_string(capacity)+"_"+to_string(way)+"_"+to_string(bl_size)+".out");
        string word;
        if(fileout.is_open()) {
            fileout<<"-- General Stats --"<<endl;
            fileout<<"Capacity: "<<capacity<<endl;
            fileout<<"Way: "<<way<<endl;
            fileout<<"Block size: "<<bl_size<<endl;
            fileout<<"Total accesses: "<<(rd_acc)+(wr_acc)<<endl; //iter로 바꾸기
            fileout<<"Read accesses: "<<rd_acc<<endl;
            fileout<<"Write accesses: "<<wr_acc<<endl;
            fileout<<"Read misses: "<<rd_miss<<endl;
            fileout<<"Write misses: "<<wr_miss<<endl;
            fileout<<"Read miss rate: "<<(float(rd_miss)/float(rd_acc))*100<<"%"<<endl;
            fileout<<"Write miss rate: "<<(float(wr_miss)/float(wr_acc))*100<<"%"<<endl;
            fileout<<"Clean evictions: "<<c_evict<<endl;
            fileout<<"Dirty evictions: "<<d_evict<<endl;
            fileout<<"Checksum: "<<dec2hex(checksum)<<endl;
        }
        fileout.close();
    }
    else{
        cout<<"Need more Execution Command Option!"<<endl;
    }
    return 0;
}
