#include<bits/stdc++.h>

using namespace std;

int Router_ID = 0;
const int inf = 100; 

class Router{
    public:
        int id;
        vector<pair<int, int>>Neighbour;
        vector<pair<int, int>>DVR;

        Router(){
            id = Router_ID ;
            Router_ID++;
            DVR.push_back({-1, -1}) ; // To start the indexing from 1 we are putting a dummy entry at index 0 
        }

        bool is_invalid(){
            if(id==-1){
                return true;
            }
            else{
                return false;
            }
        }
};

int runDVR_split_horizon(Router Routers[], int N){
    int itr = 0;
    bool change = true;

    while(change){
        itr++;
        change = false;
        
        vector<pair<int ,int>> Sent[N+1]; // to store the Distance Vectors each router sent to its neighbours ...
        
        for(int i=1; i<=N; i++){
            for(int j=0; j<Routers[i].DVR.size(); j++){
                Sent[i].push_back(Routers[i].DVR[j]);
            }
        }

        for(int i=1; i<=N; i++){
            for(int j=1; j<=N; j++){
                int y = Routers[i].DVR[j].first;
                if(i==j){
                    Routers[i].DVR[j].first=0;
                    Routers[i].DVR[j].second=i;
                }
                else{
                    Routers[i].DVR[j].first=100;
                }

                for(int k = 0 ; k <Routers[i].Neighbour.size() ; k++){
                    if(Routers[i].Neighbour[k].first==-1){
                        continue;
                    }
                    
                    if(Sent[Routers[i].Neighbour[k].first][j].second==i){
                        continue;
                    }
                    
                    if(Routers[i].DVR[j].first > (Routers[i].Neighbour[k].second + Sent[Routers[i].Neighbour[k].first][j].first)){
                        Routers[i].DVR[j].first = Routers[i].Neighbour[k].second + Sent[Routers[i].Neighbour[k].first][j].first;
                        Routers[i].DVR[j].second = Routers[i].Neighbour[k].first;
                    }
                }

                if(y!=Routers[i].DVR[j].first){
                    change = true;
                }
            }
        }
    }
    return itr;
}

int main(){
    //initalization of routers
    int N, M;
    cout << "Enter no of routers in network" << endl;
    cin >> N;
    cout << "Number of edges in network" << endl;
    cin >> M;
    
    Router Routers[N+1];
    
    for(int i=1; i<N+1; i++){
        cout << "Routers having ID" << Routers[i].id << endl;
    }
    
    for(int i=0; i<M; i++){
        int A, B, cost;
        cout << "Enters two nodes and the path cost" << endl;
        cin >> A >> B >> cost;
        Routers[A].Neighbour.push_back({B, cost});
        Routers[B].Neighbour.push_back({A, cost});
    }
    
    //DV initialization 
    for(int i=1; i<=N; i++){
        for(int j=0; j<=N; j++){
            Routers[i].DVR.push_back({inf, -1});
        }
    }

    for(int i=1; i<=N; i++){
        Routers[i].DVR[i]={0, i};
        for(int j=0; j<Routers[i].Neighbour.size(); j++){
            Routers[i].DVR[Routers[i].Neighbour[j].first] = {Routers[i].Neighbour[j].second, Routers[i].Neighbour[j].first};
        }
    }

    //DVR algorithm
    int iter = runDVR_split_horizon(Routers ,N);
    //printing the DVR for showing output
    for(int i=1; i<=N; i++){
        cout << "dest             cost     nexthop" << endl;
        for(int j=1; j<=N; j++){
            cout << j << "                 " << Routers[i].DVR[j].first << "        " << Routers[i].DVR[j].second << endl;
        }
    }

    cout << "No of iterataions to reach stability " << iter << endl;
    //Simulate link failure we have to ask for which link failed in the real case it will be entered automatically through some additional hardware or software 
    //here will manualy enter 
    cout<<"enter which link failed in format (a , b ) if link between a , b failed "<<endl;
    int a, b;
    cin >> a >> b;

    //update the link failure in the router memory 
    //delete entries from neighbor vector(mark it invalid entry instead from deleting)
    for(int i=0; i<Routers[a].Neighbour.size(); i++){
        if(Routers[a].Neighbour[i].first==b){
            Routers[a].Neighbour[i].first=-1; // I just made it an invalid entry instead of deleting it becuase it will be faster 
        }
    }
    
    for(int i=0; i<Routers[b].Neighbour.size(); i++){
        if(Routers[b].Neighbour[i].first==a){
            Routers[b].Neighbour[i].first=-1; // I just made it an invalid entry instead of deleting it becuase it will be faster 
        }
    }

    //update value in DVR 
    Routers[a].DVR[b] = {inf, -1};
    Routers[b].DVR[a] = {inf, -1};
    
    // run algorithm again that is what happens in real life also they send their new updates to the adjacent nodes and things get recalculated
    int ret = runDVR_split_horizon(Routers, N);
    for(int i=1; i<=N;i++){
        cout << "dest             cost     nexthop" << endl;
        for(int j=1; j<=N; j++){
            cout << j << "                 " << Routers[i].DVR[j].first << "        " << Routers[i].DVR[j].second << endl;
        }
    }

    cout << "No of iterataions to reach stability " << ret << endl;
}
