#include <bits/stdc++.h>

using namespace std;

const int maxN = 1e7;
int n,m, a[maxN];
int getmax(int l, int r){
    int res = a[l-1];
    for(int i = l-1; i < r; i++){
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
        if(cmp == "get-max"){
            int i,j; cin>>i>>j;
            cout << getmax(i, j) << endl;
        }
        if(cmp == "update"){
            int i,v; cin>>i>>v;
            a[i-1] = v;
        }
    }
    return 0;
}
