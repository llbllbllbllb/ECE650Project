#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <pthread.h>
#include <time.h>
#include <fstream>
// mac
//#include <mach/mach_init.h>
//#include <mach/thread_act.h>

#define NUM_THREADS 4
#define MAX_TIME 15
pthread_t threads[NUM_THREADS];
pthread_t time_id;


//ministat
// defined std::unique_ptr
#include <memory>
// defines Var and Lit
// pthread_mutex_t mutex1;
//pthread_mutex_t mutex2 = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t mutex3 = PTHREAD_COND_INITIALIZER;

#include "minisat/core/SolverTypes.h"
// defines Solver
#include "minisat/core/Solver.h"

using namespace std;


class vertex
{
public:
    int vertexNumber;   // vertex number
    int color;  // 0: white, 1: gray, 2: black
    int distance;   // -1 for infinity
    int parent; // parent of vertex number, -1 for no parent
};

string ss,ss2,ss3;
// long double CNF_SAT_VC_time, APPROX_VC_1_time, APPROX_VC_2_time;
int CNF_SAT_VC_size,APPROX_VC_1_size,APPROX_VC_2_size;



ofstream output_file;
ifstream input_file;

void readCommand(string command, int &vertexNumber, int &startNum, int &endNum, vector<int> &dataInt){

    char firstLetter = command[0];
    if(firstLetter == 'V'){
        string vertexNumberStr;
        vertexNumberStr = command.substr(2,command.size()-1);
        vertexNumber = stoi(vertexNumberStr);
    }
    else if(firstLetter == 'E'){

        // check if have not input vertex number yet
        if(vertexNumber == -1){
            cout<<"Error: You haven't input number of vertex yet."<<endl;
        }
        else{
            string dataString = command.substr(2,command.size()-1); // remove "E " from command
            regex re("<[0-9]+,[0-9]+>"); //regular expression for vertex pair,
            sregex_iterator next(dataString.begin(), dataString.end(), re); // initialize iterator to find all available vertex pair
            sregex_iterator end;

            while (next != end) {
                //this while loop extract the integer information and store them sequentially to dataInt
                smatch match = *next;
                string ss = match.str();
                ss = ss.substr(1,ss.size()-2);
                string firstNumber, secondNumber;
                string delimiter = ",";

                firstNumber = ss.substr(0,ss.find(delimiter));
                secondNumber = ss.substr(ss.find(delimiter)+1, ss.size());

                dataInt.push_back(stoi(firstNumber));
                dataInt.push_back(stoi(secondNumber));
                //need to check if valid: smaller than vertex number
                next++;
            }

            // check if the data are smaller than vertex number
            bool dataLargerThanVertexNum = false;
            for(int j=0; j<dataInt.size();j++){
                if(dataInt[j]>=vertexNumber){
                    dataLargerThanVertexNum = true;
                }
            }
            if(dataLargerThanVertexNum){  // some vertex larger or equal to vertex number, invalid input, clear dataInt
                cout<<"Error: Your edge list contains number(s) larger than the vertex number, please type again"<<endl;
                dataInt.clear();
            }//else, edge input is valid, dataInt will be passed to createAdjList for further processing

        }
    }
    else{
        cout<<"Error: Your command is not valid, please input correct command."<<endl;
    }

}


void createAdjList(vector<int> dataInt, int vertexNumber, vector<list<int> > &adj_list){
    adj_list.clear();
    for(int i = 0; i<vertexNumber; i++){
        list<int> tmpList;
        adj_list.push_back(tmpList);
    }
    // first push back empty list
    // run the program that convert the dataInt into adjacency list for later processing
    for(int j=0; j<=(dataInt.size()/2-1);j++){
        int firstNumberIndex = j*2;
        int secondNumberIndex = j*2+1;

        int firstNum = dataInt[firstNumberIndex];
        int secondNum = dataInt[secondNumberIndex];

        if(find(adj_list[firstNum].begin(), adj_list[firstNum].end(), secondNum) == adj_list[firstNum].end()) {
            /* v does not contain x */
            adj_list[firstNum].push_back(secondNum);
        }
        if(find(adj_list[secondNum].begin(), adj_list[secondNum].end(), firstNum) == adj_list[secondNum].end()) {
            /* v does not contain x */
            adj_list[secondNum].push_back(firstNum);
        }
    }


}

//double threadTime(/*int& rc*/){
////    mach_port_t thread = mach_thread_self();
////    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
////    thread_basic_info_data_t info;
////    kern_return_t kr = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);
////    if (kr != KERN_SUCCESS || (info.flags & TH_FLAGS_IDLE) != 0)
////        return 0.0;
////    return info.user_time.microseconds;
//
//    struct timespec ts;
//    clockid_t cid;
//    pthread_getcpuclockid(pthread_self(), &cid);
//    clock_gettime(cid, &ts);
//    return ts.tv_nsec;
//}

// vector<int>& dataInt, int vertexNumber

// time thread
void * time_thread(void* input){
  int count = 0;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  while (MAX_TIME > count){
    pthread_testcancel();
    count++;
    sleep(1);
  }
  (*(std::unique_ptr<Minisat::Solver>*) input) -> interrupt();
  return NULL;
}


void *APPROX_VC_1(void* input){
    // vector<int> dataInt = ((struct args*)input) -> dataInt;
    // int vertexNumber = ((struct args*)input) -> vertexNumber;

    // time start
    // clockid_t clock_id;
    // pthread_getcpuclockid(pthread_self(), &clock_id);
    // struct timespec ts_start;
    // clock_gettime(clock_id, &ts_start);

    vector<int> dataInt = *(vector <int>*)input;
    int vertexNumber = dataInt[dataInt.size()-1];
    dataInt.pop_back();

    vector<int> res;
    vector<int> vertexByRate(vertexNumber);
    vector<int> occurRate(vertexNumber,0);
    vector<bool> deleted(dataInt.size()/2,false);

    for(auto num:dataInt){
        occurRate[num]++;
    }
    for(int i=0; i<vertexByRate.size();i++){
        vertexByRate[i] = i;
    }
    sort(vertexByRate.begin(), vertexByRate.end(),[&occurRate](size_t i1, size_t i2) {return occurRate[i1] < occurRate[i2];});


//    cout<<"vertexByRate: "<<endl;
//    for(auto num:vertexByRate){
//        cout<<num<<", ";
//    }
//    cout<<endl;

    bool allDone = false;
    while(!allDone){
        res.push_back(vertexByRate[vertexByRate.size()-1]);
//        cout<<"res has push back: "<<vertexByRate[vertexByRate.size()-1]<<endl;
        for(int i=0; i<dataInt.size(); i+=2){
            if(dataInt[i] == vertexByRate[vertexByRate.size()-1] || dataInt[i+1] == vertexByRate[vertexByRate.size()-1]){
                //delete this
                deleted[i/2] = true;
            }
        }
        vertexByRate.pop_back();
        if(find(deleted.begin(),deleted.end(),false) == deleted.end()){
            allDone = true;
        }

    }

    sort(res.begin(),res.end());

    APPROX_VC_1_size = res.size();


    // time end
    // struct timespec ts_end;
    // clock_gettime(clock_id, &ts_end);

    // APPROX_VC_1_time  = ((long double)ts_end.tv_sec*1000000 + (long double)ts_end.tv_nsec/1000.0) - ((long double)ts_start.tv_sec*1000000 + (long double)ts_start.tv_nsec/1000.0);

    ss.clear();
    ss ="APPROX-VC-1: ";
    for(int i=0; i<res.size(); i++){
        if(i != res.size()-1){ss = ss + to_string(res[i]) + ",";}
        else{ss+=to_string(res[i]);}

    }
    ss+="\n";

return NULL;
}

void *APPROX_VC_2(void* input)
{
    // create the map to store the element of dataInt, make it unvisited
//    vertexCoverA2.clear();

    // time start
    // clockid_t clock_id;
    // pthread_getcpuclockid(pthread_self(), &clock_id);
    // struct timespec ts_start;
    // clock_gettime(clock_id, &ts_start);

    vector<int> dataInt = *(vector <int>*)input;
    int vertexNumber = dataInt[dataInt.size()-1];
    dataInt.pop_back();
    vector<int> vertexCoverA2;
    map<int, bool> myMap;
    for (int i = 0; i < dataInt.size(); i++)
    {
        myMap[dataInt[i]] = false;
    }

    for (int i = 0; i < dataInt.size(); i=i+2)
    {
        if (myMap[dataInt[i]] == true || myMap[dataInt[i+1]] == true)
        {
            continue;
        }
        vertexCoverA2.push_back(dataInt[i]);
        vertexCoverA2.push_back(dataInt[i+1]);
        myMap[dataInt[i]] = true;
        myMap[dataInt[i+1]] = true;
    }

//    cout << "size is " << vertexCoverA2.size() << endl;
    sort(vertexCoverA2.begin(), vertexCoverA2.end());

    APPROX_VC_2_size = vertexCoverA2.size();

    // time end
    // struct timespec ts_end;
    // clock_gettime(clock_id, &ts_end);

    // APPROX_VC_2_time = ((long double)ts_end.tv_sec*1000000 + (long double)ts_end.tv_nsec/1000.0) - ((long double)ts_start.tv_sec*1000000 + (long double)ts_start.tv_nsec/1000.0);

    ss2.clear();
    ss2 = "APPROX-VC-2: ";
    for (int i = 0; i < vertexCoverA2.size(); i++)
    {
        if(i != vertexCoverA2.size()-1){ss2 = ss2 + to_string(vertexCoverA2[i]) + ",";}
        else{ss2 += to_string(vertexCoverA2[i]);}

    }
    ss2+="\n";

return NULL;
}

void *IO(void*) {
    cout << ss3;
    cout << ss;
    cout << ss2;
    return NULL;
    // cout<<"CNF_SAT_VC_time: "<<CNF_SAT_VC_time<<endl;
    // cout<<"APPROX_VC_1_time: "<<APPROX_VC_1_time<<endl;
    // cout<<"APPROX_VC_2_time: "<<APPROX_VC_2_time<<endl;
    // cout<<"CNF_SAT_VC_size: "<<CNF_SAT_VC_size<<endl;
    // cout<<"APPROX_VC_1_size: "<<APPROX_VC_1_size<<endl;
    // cout<<"APPROX_VC_2_size: "<<APPROX_VC_2_size<<endl;
    // output_file<<CNF_SAT_VC_time<<",";
    // output_file<<APPROX_VC_1_time<<",";
    // output_file<<APPROX_VC_2_time<<",";
    // output_file<<CNF_SAT_VC_size<<",";
    // output_file<<APPROX_VC_1_size<<",";
    // output_file<<APPROX_VC_2_size<<",";
    // output_file<<endl;



//    CNF_SAT_VC_size,APPROX_VC_1_size,APPROX_VC_2_size;

//    CNF_SAT_VC_time, APPROX_VC_1_time, APPROX_VC_2_time;
}

void *CNF_SAT_VC(void* input){
    // create time threads
    // int rcTime = pthread_create(&time_id, NULL, &time_thread, &threads[2]);
    // if (rcTime)
    // {
    //   cerr << "Error: unable to create thread" << endl;
    //   exit(-1);
    // }
    // enable cancellation
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // time start
    // clockid_t clock_id;
    // pthread_getcpuclockid(pthread_self(), &clock_id);
    // struct timespec ts_start;
    // clock_gettime(clock_id, &ts_start);

    vector<int> dataInt = *(vector <int>*)input;
    int vertexNumber = dataInt[dataInt.size()-1];
    dataInt.pop_back();
    // -- allocate on the heap so that we can reset later if needed
    std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());

    int rcTime = pthread_create(&time_id, NULL, &time_thread, &solver);
    if (rcTime)
    {
      cerr << "Error: unable to create thread" << endl;
      exit(-1);
    }

    Minisat::vec<Minisat::Lit> dummy;
    Minisat::lbool ret = solver->solveLimited(dummy);

    bool res = false;
    int n = vertexNumber;
    int k = 1; // k starts from 1 and ascending
    while(!res){
        if (ret != Minisat::l_True && ret != Minisat::l_False) {
          ss3.clear();
          ss3 = "CNF-SAT-VC: timeout\n";
          break;
        }

        // pthread_testcancel();
        //create n*k atomic propositions x(i-1,j-1)
        vector<vector<Minisat::Lit>> litArray;
        for(int i =0; i<n;i++){
            vector<Minisat::Lit> tmpVector;
            for(int j =0; j<k; j++){
                pthread_testcancel();
                Minisat::Lit tmp;
                tmp = Minisat::mkLit(solver->newVar());
                tmpVector.push_back(tmp);
            }
            litArray.push_back(tmpVector);
        }

        // add clauses
        // 1. At least one vertex is the ith vertex in the vertex cover
        for(int m =0;m<k;m++){
            Minisat::vec<Minisat::Lit> tmp;
            for(int a=0;a<n;a++){
                // pthread_testcancel();
                tmp.push(litArray[a][m]);
            }
            solver->addClause(tmp);
        }

        // 2. No one vertex can appear twice in a vertex cover
        for(int m=0;m<n;m++){
            for(int q=0;q<k;q++){
                for(int p=0;p<q;p++){
                    // pthread_testcancel();
                    solver->addClause(~litArray[m][p],~litArray[m][q]);
                }
            }
        }

        // 3. No more than one vertex appears in the mth position of the vertex cover
        for(int m=0;m<k;m++){
            for(int q=0;q<n;q++){
                for(int p=0;p<q;p++){
                    // pthread_testcancel();
                    solver->addClause(~litArray[p][m],~litArray[q][m]);
                }
            }
        }

        // 4. Every edge is incident to at least one vertex in the vertex cover
        for(int i=0;i<dataInt.size();i+=2){
            Minisat::vec<Minisat::Lit> tmp;
            for(int a=0;a<k;a++){
                // pthread_testcancel();
                tmp.push(litArray[dataInt[i]][a]);
                tmp.push(litArray[dataInt[i+1]][a]);
            }
            solver->addClause(tmp);
        }
        ret = solver->solveLimited(dummy);
        res = solver->solve();


//                std::cout << "The result is: " << res << "\n";


        // output corresponding vertex
        vector<int> vertexOutput;
        if(res){
            for(int i =0; i<n;i++){
                for(int j =0; j<k; j++){
//                            cout<<"("<<i<<","<<j<<"):"<<Minisat::toInt(solver->modelValue(litArray[i][j]))<<endl;
                    if(Minisat::toInt(solver->modelValue(litArray[i][j])) == 0){

                        vertexOutput.push_back(i);
                    }
                }
            }
            sort(vertexOutput.begin(),vertexOutput.end());

            CNF_SAT_VC_size  = vertexOutput.size();


            // time end
            // struct timespec ts_end;
            // clock_gettime(clock_id, &ts_end);
            //
            // CNF_SAT_VC_time = ((long double)ts_end.tv_sec*1000000 + (long double)ts_end.tv_nsec/1000.0) - ((long double)ts_start.tv_sec*1000000 + (long double)ts_start.tv_nsec/1000.0);

            ss3.clear();
            ss3 = "CNF-SAT-VC: ";
            for(int i=0;i<vertexOutput.size();i++){
                if(i != vertexOutput.size()-1){
                    ss3 = ss3+ to_string(vertexOutput[i])+ ",";
                }
                else{
                    ss3+=to_string(vertexOutput[i]);
                }

            }
            ss3+="\n";
        }

        solver.reset (new Minisat::Solver());

        k++;
//                cout<<"current k is: "<<k<<endl;
    }
    pthread_cancel(time_id);
return NULL;
}



int main(int argc, char **argv) {


    // if(pthread_mutex_init(&mutex1,NULL) != 0){
    //     return 1;
    // }

    /* in V, store Vertex number */
    int vertexNumber = -1;


    /* in s, store start number and end number */
    int startNum = -1;
    int endNum = -1;

    vector<int> dataInt; /* in E store output of regex */
    vector<int> existVertex; // to create color list and parent list
    vector<list<int> > adj_list;
    vector<vertex> vertexList;


    // output_file.open ("../output.txt");
    // input_file.open("../input2.txt");




    while(true){
        string command;
       getline(cin,command);
        // getline(input_file,command);

       if (std::cin.eof()){
           break;
       }
        // if (input_file.eof()){
        //     break;
        // }

        readCommand(command, vertexNumber, startNum, endNum, dataInt);

        // if dataInt not empty(E input is valid), create adjList
        if(!dataInt.empty()){
            // pthread_t threads[NUM_THREADS];
            // pthread_t time_id;

            int rc1, rc2, rc3, rcIO;
            void * res;

//            clock time
            // clockid_t cid1;
            // clockid_t cid2;
            // clockid_t cid3;

            vector<int> newDataInt;
            newDataInt = dataInt;
            newDataInt.push_back(vertexNumber);

              rc1 = pthread_create(&threads[0], NULL, &APPROX_VC_1, &newDataInt);
              rc2 = pthread_create(&threads[1], NULL, &APPROX_VC_2, &newDataInt);
              rc3 = pthread_create(&threads[2], NULL, &CNF_SAT_VC, &newDataInt);

              // int count = 0;
              // while (count < MAX_TIME) {
              //   count ++;
              //   sleep(1);
              //
              // }

              if (rc1 || rc2 || rc3)
              // if (rc1 || rc2)
              {
                cerr << "Error: unable to create thread" << endl;
                exit(-1);
              }
//            CNF_SAT_VC_time = threadTime(rc3);
//            APPROX_VC_1_time = threadTime(rc1);
//            APPROX_VC_2_time = threadTime(rc2);

              pthread_join(threads[2], &res);
              pthread_join(threads[0],NULL);
              pthread_join(threads[1],NULL);
              pthread_join(time_id,NULL);

              rcIO = pthread_create(&threads[3], NULL, IO, NULL);


              if (rcIO)
              {
                  cerr << "Error: unable to create thread" << endl;
                  exit(-1);
              }
            pthread_join(threads[3], NULL);


//              cout<<ss3;
//              cout<<ss;
//              cout<<ss2;


            // CNF_SAT_VC(dataInt,vertexNumber);
            // APPROX_VC_1(dataInt,vertexNumber);
            // APPROX_VC_2(vertexNumber,dataInt);



            //create adj_list
            createAdjList(dataInt, vertexNumber, adj_list);

            // fill in existVertex, dataInt might contain multiple same number, existVertex remove duplicates
            existVertex = dataInt;
            sort( existVertex.begin(), existVertex.end() );
            existVertex.erase( unique( existVertex.begin(), existVertex.end() ), existVertex.end() );

            //clear dataInt
            dataInt.clear();
        }


    }

    // output_file.close();
    // input_file.close();


    return 0;
}
