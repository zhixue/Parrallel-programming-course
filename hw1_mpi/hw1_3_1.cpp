//
//  small files
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <mpi.h>
#include <string>
#include <map>
#include <vector>

using namespace std;

void split(const string& s, std::vector<string>& elems, char splitchar = ' '){
    std::stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss,item,splitchar)){
        elems.push_back(item);
    }
}


bool isword(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}


void strip(string& s, string& strip_s){
    // remove the char like {}[]()*&
    for (int i = 0;i < s.size();i++){
        if (isword(s[i])){
            strip_s += s[i];
        }
    }
}

void downletter(string& raw_s, string& down_s){
    //
    for (int i = 0;i < raw_s.size();i++){
        down_s += tolower(raw_s[i]);
    }
}

void add_string_to_map(std::map<string,int>& wordcount, const string& key_value){
    std::vector<string> temp_key_value;
    split(key_value, temp_key_value);
    string key = temp_key_value[0];
    string value = temp_key_value[1];
    //if (atoi(value.c_str())>100) {
    //    cout << key << "  " << value << endl;
    //}
    std::map<string,int>::iterator iter = wordcount.find(key);
    if (iter != wordcount.end()){
        wordcount[key] += atoi(value.c_str());
    } else {
        wordcount[key] = atoi(value.c_str());
    }
}

void wordcount_file(std::map<string,int>& container, const string& filename){
    string word = "";
    string cleanword;
    string lowercleanword;
    ifstream fin;
    fin.open(filename);
    if (!fin.is_open()){
        cout<<"Open error!"<<endl;
        exit(0);
    }
    while (!fin.eof()){
        cleanword = "";
        lowercleanword = "";
        fin >> word;
        strip(word, cleanword);
        downletter(cleanword,lowercleanword);
        if (lowercleanword != ""){
            std::map<string,int>::iterator iter = container.find(lowercleanword);
            if (iter != container.end()){
                container[lowercleanword] += 1;
            } else{
                container[lowercleanword] = 1;
            }
        }
    }
    fin.close();
}

int min(int a,int b){
    if (a<=b)
        return a;
    else
        return b;

}

int main(int argc, char *argv[]) {
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // small files id:100~199
    if (world_rank != 0){
        // read files on other processors
        int blockn = 100/(world_size-1)+1;
        std::map<string,int> temp_container;

        // the last processor may read less than blockn files
        for (int file_id = 100+(world_rank-1)*blockn; file_id < min(100+world_rank*blockn,199); file_id++){
            stringstream file_idname;
            file_idname << file_id;
            string open_file_name = "./Small_file/tmp/small_" + file_idname.str() + ".txt";
            wordcount_file(temp_container, open_file_name);
        }
        // send map key number
        int temp_container_num = temp_container.size();
        MPI_Send(&temp_container_num,1,MPI_INT, 0, 0,MPI_COMM_WORLD);

        for(std::map<string,int>::iterator iter = temp_container.begin(); iter != temp_container.end(); iter++) {
            // map obeject{key:value} to char *{key value}
            // map to string
            stringstream strnum;
            stringstream strname;
            strnum << iter->second;
            strname << iter->first;
            string st_key_value = strname.str() + " " + strnum.str();
            int st_length = st_key_value.size();
            //string to char *
            char *char_key_value = new char[st_length];
            for(int i=0;i<st_length;i++){
                char_key_value[i] = st_key_value[i];
            }
            // send keyvalue length & content
            MPI_Send(&st_length, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(char_key_value,st_length,MPI_BYTE,0 ,2, MPI_COMM_WORLD);
        }





    } else {
        // summary on the first processor
        map<string,int> wordmap;
        for (int proc=1;proc<world_size;proc++){
            int container_size;
            MPI_Recv(&container_size,1,MPI_INT,proc,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);


            for (int k=0;k<container_size;k++){
                int stlength;
                MPI_Recv(&stlength,1,MPI_INT,proc,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                //cout<<stlength<<endl;
                char *st = new char[stlength];
                MPI_Recv(st,stlength,MPI_BYTE,proc,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                string str(st);
                //cout<< st<<endl;
                add_string_to_map(wordmap, st);
            }

        }
        // print result
        cout<<"Small files result:"<<endl;
        for(std::map<string,int>::iterator iter = wordmap.begin(); iter != wordmap.end(); iter++){
            cout<<iter->first<<": "<<iter->second<<endl;
        }
    }
    MPI_Finalize();
}

