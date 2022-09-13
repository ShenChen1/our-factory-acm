#include <bits/stdc++.h>
using namespace std;

size_t Solve(int N, vector<int> arr)
{
    // Write your code here
    unordered_map<int, vector<int>> u = {};
    for (size_t i = 0; i < N; ++i) u[arr[i]].emplace_back(i);

    size_t res = 1000000000;
    for (const auto & [k, v] : u) {
        size_t offset = v.size() - 1;
        if (offset == 0) {
            continue;
        }

        size_t d = v[offset] - v[0];
        res = min(res, d);
    }
    return res;
}

int main()
{
    ios::sync_with_stdio(0);
    cin.tie(0);
    int N;
    cin >> N;
    vector<int> arr(N);
    for (int i_arr = 0; i_arr < N; i_arr++) {
        cin >> arr[i_arr];
    }

    size_t out_;
    out_ = Solve(N, arr);
    cout << out_;
}