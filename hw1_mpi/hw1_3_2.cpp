//
// big file
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

int get_fileline(const string& file) {
    int countline = 0;
    string line;
    ifstream fin;
    fin.open(file);
    if (!fin.is_open()){
        cout<<"Open error!"<<endl;
        exit(0);
    }
    while(!fin.eof()){
        countline++;
        getline(fin,line);
    }
    fin.close();
    return countline;
}


void wordcount_file(std::map<string,int>& container, const string& filename, int startline, int endline){
    int currentline = 0;
    string line;
    std::vector<string> words;
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
        currentline++;
        getline(fin,line);
        if (currentline < startline)
            continue;
        if (currentline > endline)
            break;
        split(line,words);
        int words_size = words.size();
        for (std::vector<string>::iterator iter=words.begin();iter!=words.end();iter++){
            word = *iter;
            cleanword = "";
            lowercleanword = "";
            strip(word, cleanword);
            downletter(cleanword,lowercleanword);
            //cout<<lowercleanword<<endl;
            if (lowercleanword != ""){
                std::map<string,int>::iterator iter = container.find(lowercleanword);
                if (iter != container.end()){
                    container[lowercleanword] += 1;
                } else{
                    container[lowercleanword] = 1;
                }
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

    // big files id:100
    if (world_rank != 0){
        // read files on other processors
        std::map<string,int> temp_container;

        int file_id = 100;
        stringstream file_idname;
        file_idname << file_id;
        string open_file_name = "./Big_file/big_" + file_idname.str() + ".txt";
        //cout<<open_file_name<<endl;
        int linecount = get_fileline(open_file_name);
        //cout<<linecount<<" file size"<<endl;

        int blockn = linecount/world_size + 1; // the last proccessor may read less lines than others
        wordcount_file(temp_container, open_file_name, blockn*(world_rank-1), min(blockn*world_rank,linecount));

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
        cout<<"Big files result:"<<endl;
        for(std::map<string,int>::iterator iter = wordmap.begin(); iter != wordmap.end(); iter++){
            cout<<iter->first<<": "<<iter->second<<endl;
        }
    }
    MPI_Finalize();
}

