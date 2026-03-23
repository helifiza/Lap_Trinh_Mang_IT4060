#include <bits/stdc++.h>

using namespace std;

const int maxN = 1e7;
int n,m, a[maxN];
int getmax(int l, int r){
    int res = INT_MIN;
    for(int i = l; i <= r; i++){
        res = max(res, a[i]);
    }
    return res;
}
int main(){
    cin>>n;
    for(int i = 0; i < n; i++){
        cin>>a[i];
    }
    cin>>m;
    while(m--){
        string cmp; cin>>cmp;
        int i,j; cin>>i>>j;
        if(cmp == "get-max"){
            cout << getmax(i, j) << endl;
        }
        if(cmp == "update"){
            int i,v; cin>>i>>v;
            a[i] = v;
        }
    }
}
