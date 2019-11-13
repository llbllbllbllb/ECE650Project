#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <vector>
#include <algorithm>
#include <list>

//ministat
// defined std::unique_ptr
#include <memory>
// defines Var and Lit
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
    else if (firstLetter == 's'){
        // check vertex number
        if(vertexNumber==-1){
            cout<<"Error: You haven't input number of vertex yet."<<endl;
        }
        else{
            // parse input string
            string ss;
            regex re("-?[0-9]+");
            ss = command.substr(2,ss.size()-1); // remove "s " at the beginning

            sregex_iterator next(ss.begin(), ss.end(), re);
            sregex_iterator end;
            vector<int> startEndNumber;
            while (next != end) {
                //this while loop extract the integer information and store them sequentially to startEndNumber
                smatch match = *next;
                string stringNum = match.str();
                int extractedNum = stoi(stringNum);
                startEndNumber.push_back(extractedNum);
                //need to check if valid: smaller than vertex number
                next++;
            }
            startNum = startEndNumber[0];
            endNum = startEndNumber[1];
            if(startNum >= vertexNumber || endNum >= vertexNumber || startNum < 0 || endNum < 0){
                cout<<"Error: Your start number or end number is invalid"<<endl;
                startNum = -1;
                endNum = -1;
            }

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

void calculateShortestPath(int startNum, int endNum, vector<list<int> > adj_list, vector<vertex> &vertexList, vector<int> existVertex){
    /* This function calculate the distance and parent of each vertex given the start and end vertex */

    vertexList.clear(); // first clean the vertex list,
    //initialize vertexList
    for(int i = 0; i<existVertex.size(); i++){
        vertex tmpVertex;
        tmpVertex.vertexNumber = existVertex[i];
        tmpVertex.color = 0; // initialize: color is white
        tmpVertex.parent = -1; // initialize: no parent yet
        tmpVertex.distance = -1; // initialize: infinite distance
        vertexList.push_back(tmpVertex);
    }

    // first change start vertex property:
    for(int i =0; i<vertexList.size(); i++){
        if(vertexList[i].vertexNumber == startNum){
            vertexList[i].color = 1; //change color to gray
            vertexList[i].distance = 0; //change distance to 0
            vertexList[i].parent = -1; // change parent to null
        }
    }


    // create a queue
    vector<int> queue;
    queue.push_back(startNum);
    while(!queue.empty()){
        int processVertexNum = queue[0];

        queue.erase(queue.begin()); // delete first element

        //get distance of the processVertex
        int processVerDistance;
        for(int i = 0; i<vertexList.size(); i++){
            if(vertexList[i].vertexNumber == processVertexNum){//found process vertex
                processVerDistance = vertexList[i].distance;
            }
        }

        list<int>::iterator it; // declare a iterator for list
        for(it = adj_list[processVertexNum].begin();it!=adj_list[processVertexNum].end();it++){ // for every vertex in adj_list
            int adjNum = *it;

            for(int j = 0; j<vertexList.size(); j++){
                if(vertexList[j].vertexNumber == adjNum){ // found that adj vertex in vertexList

                    if(vertexList[j].color == 0){//check if color is white

                        vertexList[j].color = 1 ;// color is white, change it to gray
                        vertexList[j].distance = processVerDistance +1;
                        vertexList[j].parent = processVertexNum;

                        queue.push_back(vertexList[j].vertexNumber);


                    }
                }
            }
        }
        for(int i = 0; i<vertexList.size(); i++){
            if(vertexList[i].vertexNumber == processVertexNum){
                vertexList[i].color = 2; // change color to black
            }
        }


    }

}


void printPath(vector<vertex> vertexList, int startNum, int endNum){
    if(startNum == endNum){
        cout<<startNum;
    }
        // find endNum vertex and check if its parent is -1(Null)
    else{
        for(int i =0; i<vertexList.size();i++){
            if(vertexList[i].vertexNumber == endNum){//found endNum vertex in the list
                if(vertexList[i].parent == -1){ //end Num no parent
                    cout<<"Error: no path from "<<startNum<<" to "<<endNum<<" exists";

                }
                else{
                    printPath(vertexList,startNum,vertexList[i].parent);
                    cout<<"-"<<vertexList[i].vertexNumber;
                }
            }
        }
    }


}



int main(int argc, char **argv) {

    /* in V, store Vertex number */
    int vertexNumber = -1;


    /* in s, store start number and end number */
    int startNum = -1;
    int endNum = -1;

    vector<int> dataInt; /* in E store output of regex */
    vector<int> existVertex; // to create color list and parent list
    vector<list<int> > adj_list;
    vector<vertex> vertexList;


    while(true){
        string command;
        getline(cin,command);

        if (std::cin.eof()){
            break;
        }

        readCommand(command, vertexNumber, startNum, endNum, dataInt);

        // if dataInt not empty(E input is valid), create adjList
        if(!dataInt.empty()){

            // -- allocate on the heap so that we can reset later if needed
            std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());

//            cout<<"Edge List: "<<endl;
//            for(int i =0; i<dataInt.size();i++){
//                cout<<dataInt[i]<<",";
//            }
//            cout<<endl;

            bool res = false;
            int n = vertexNumber;
            int k = 1; // k starts from 1 and ascending
            while(!res){
                //create n*k atomic propositions x(i-1,j-1)
                vector<vector<Minisat::Lit>> litArray;
                for(int i =0; i<n;i++){
                    vector<Minisat::Lit> tmpVector;
                    for(int j =0; j<k; j++){
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
                        tmp.push(litArray[a][m]);
                    }
                    solver->addClause(tmp);
                }

                // 2. No one vertex can appear twice in a vertex cover
                for(int m=0;m<n;m++){
                    for(int q=0;q<k;q++){
                        for(int p=0;p<q;p++){
                            solver->addClause(~litArray[m][p],~litArray[m][q]);
                        }
                    }
                }

                // 3. No more than one vertex appears in the mth position of the vertex cover
                for(int m=0;m<k;m++){
                    for(int q=0;q<n;q++){
                        for(int p=0;p<q;p++){
                            solver->addClause(~litArray[p][m],~litArray[q][m]);
                        }
                    }
                }

                // 4. Every edge is incident to at least one vertex in the vertex cover
                for(int i=0;i<dataInt.size();i+=2){
                    Minisat::vec<Minisat::Lit> tmp;
                    for(int a=0;a<k;a++){
                        tmp.push(litArray[dataInt[i]][a]);
                        tmp.push(litArray[dataInt[i+1]][a]);
                    }
                    solver->addClause(tmp);
                }
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
                    for(int i=0;i<vertexOutput.size();i++){
                        cout<<vertexOutput[i]<<" ";
                    }
                    cout<<endl;
                }


                solver.reset (new Minisat::Solver());

                k++;
//                cout<<"current k is: "<<k<<endl;
            }



            //create adj_list
            createAdjList(dataInt, vertexNumber, adj_list);

            // fill in existVertex, dataInt might contain multiple same number, existVertex remove duplicates
            existVertex = dataInt;
            sort( existVertex.begin(), existVertex.end() );
            existVertex.erase( unique( existVertex.begin(), existVertex.end() ), existVertex.end() );

            //clear dataInt
            dataInt.clear();
        }

        if(startNum != -1 && endNum != -1){

            if((find(existVertex.begin(), existVertex.end(), startNum) != existVertex.end()) && (find(existVertex.begin(), existVertex.end(), endNum) != existVertex.end())) {
                /* both startNum and endNum in existVertex */
                calculateShortestPath(startNum, endNum, adj_list, vertexList, existVertex);
                if(startNum == endNum){
                    cout<<"Error: Your start or end vertex are same";
                }
                else{
                    printPath(vertexList, startNum, endNum);
                }

                cout<<endl;
            } else {
                /* one or both not in existVertex */
                cout<<"Error: Your start or end vertex does not exist"<<endl;
            }

            startNum = -1;
            endNum = -1;
        }

    }

    return 0;
}


