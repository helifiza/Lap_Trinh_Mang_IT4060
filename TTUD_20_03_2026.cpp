#include <bits/stdc++.h>
using namespace std;

const int N = 30;
const int INF = 1e9;
int n, k, fCcur = 0, fopt = INF;
int c[N][N];
int x[N], xOpt[N], load = 0;
vector<int> visited(N, false);

void input(){
    cin>> n >> k;
    for(int i =0; i<=2*n+1; i++){
        for(int j = 0; j<=2*n+1;j++){
            cin>> c[i][j];
            if(i!=j) cmin = min(cmin, c[i][j]);
        }
    }
}

bool check(int v, int k){
    if visited[v] = true return false;
    if(v>n){
        if(visited[v-n] == false) return false;
    }else{
        if(load == k) return false;
    }
    return true;
}

void updateSol(){
    if(fcur + c[x[2*n+1]][0] < fopt){
        fopt = fcur + c[x[2*n+1]][0];
        for(int i =0; i<= 2*n+1; i++){
            xOpt[i] = x[i];
        }
    }
}

void TRY(int i){
    for(int v= 1; v <=2*n; v++){
        if(check(v,k)){
            x[k] =v;
            visited[v] = true;
            if(v <= n) load++;
            else load--;
            fcur += c[x[k-1]][v];
            if(k == 2*n+1) updateSol();
            else{
                if(fcur + (2*n+1-k)*cmin <fopt) TRY(k+1);
            }
            fcur -= c[x[k-1]][v];
            if(v <= n) load--; else load++;
            visited[v] = false;
        }
    }
}

int main()
{
    if(fopen("data.inp", "r") != NULL){
        freopen("data.inp", "r", stdin);
    }
    ios_base::sync_with_stdio(false);
    cin.tie(0);

    input();
    x[0] = 0;
    TRY(1);
    cout<< fopt<<endl;
    return 0;
}