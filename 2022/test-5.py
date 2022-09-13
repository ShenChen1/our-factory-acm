import threading
import time
import sys
import os

class Solution:
    def __init__(self, N, M, K, A, L, R, V):
        self.min_cost = 1
        self.n_total = M
        self.n_required = K
        self.shop_money = []
        self.shop_cap = []
        self.shop_efficiency = []

        self.n_shop = 0
        self.shop_pool = []
        for i in range(N):
            if V[i] is False:
                self.shop_money.append(A[i])
                self.shop_cap.append([n for n in range(L[i], R[i] + 1)])
                self.shop_efficiency.append((self.n_shop, A[i] / (R[i] + 1 - L[i])))
                self.shop_pool.append(self.n_shop)
                self.n_shop += 1
                self.min_cost += A[i]
        self.oom = self.min_cost
        self.cache_path = []
        self.cache_res_money = []

    def update(self, shop, cur_caps, cur_efficiencies):
        new_shop_efficiency = []
        new_shop_caps = cur_caps.copy()
        for (s, _) in cur_efficiencies:
            new_shop_caps[s] = new_shop_caps[s].copy()
            for sweet in cur_caps[shop]:
                if sweet in new_shop_caps[s]:
                    new_shop_caps[s].remove(sweet)

            if len(new_shop_caps[s]):
                new_shop_efficiency.append((s, self.shop_money[s] / len(new_shop_caps[s])))

        return new_shop_caps, new_shop_efficiency

    def pick_one(self, cur_pool, cur_cost, cur_caps, cur_efficiencies, path):
        #print("path", path)
        #print(cur_pool, cur_cost)
        #print(cur_caps)
        #print(self.shop_cap)
        #print(cur_efficiencies)
        #print(self.shop_efficiency)

        if len(cur_pool) >= self.n_required:
            self.min_cost = min(self.min_cost, cur_cost)
            return cur_cost

        if len(cur_efficiencies) == 0:
            return self.oom

        path.sort()
        if path in self.cache_path:
            index = self.cache_path.index(path)
            return cur_cost + self.cache_res_money[index]

        cur_efficiencies = sorted(cur_efficiencies, key=lambda item:item[1], reverse=False)
        (shop, _) = cur_efficiencies[0]
        new_shop_caps, new_shop_efficiency = self.update(shop, cur_caps, cur_efficiencies)

        money = self.pick_one(cur_pool + cur_caps[shop], cur_cost + self.shop_money[shop], new_shop_caps, new_shop_efficiency, path + [shop])
        self.cache_path.append(path)
        self.cache_res_money.append(money - cur_cost)
        return money

    def run(self):
        self.shop_efficiency = sorted(self.shop_efficiency, key=lambda item:item[1], reverse=False)
        while len(self.shop_pool):
            shop = self.shop_pool.pop()
            cur_pool = []
            cur_cost = 0
            cur_efficiencies = self.shop_efficiency.copy()
            cur_caps = self.shop_cap.copy()
            new_shop_caps, new_shop_efficiency = self.update(shop, cur_caps, cur_efficiencies)
            res = self.pick_one(cur_pool + cur_caps[shop], cur_cost + self.shop_money[shop], new_shop_caps, new_shop_efficiency, [shop])
            if self.oom == res:
                self.min_cost = -1
                break
        return self.min_cost

def minimumCost (N, M, K, A, L, R):
    V = [False for _ in range(N)]

    for i in range(N):
        a = [n for n in range(L[i], R[i] + 1)]
        for j in range(N):
            if i == j:
                continue
            b = [n for n in range(L[j], R[j] + 1)]
            if a == b and V[j]:
                continue
            if set(a).issubset(b) and A[i] >= A[j]:
                V[i] = True
                break

    #print(A)
    #print(V)

    s = Solution(N, M, K, A, L, R, V)
    return s.run()

N = 9
M = 9
K = 4
A = [1, 2, 1, 3, 1, 1, 1, 1, 1]
L = [4, 2, 2, 2, 4, 6, 7, 8, 9]
R = [4, 4, 4, 4, 4, 6, 7, 8, 9]

N = 1
M = 9
K = 1
A = [1]
L = [1]
R = [4]


N = 8
M = 16
K = 10
A = [36, 87, 93, 50, 22, 63, 28, 91]
L = [3, 4, 7, 7, 8, 9, 10, 11]
R = [5, 12, 13, 14, 9, 12, 16, 15]
'''
N = 12
M = 16
K = 10
A = [28, 91, 60, 64, 27, 41, 27, 73, 37, 12, 69, 68]
L = [2, 3, 4, 5, 6, 6, 7, 7, 8, 10, 10, 11]
R = [8, 14, 9, 12, 11, 14, 11, 14, 9, 15, 16, 15]
'''

out_ = minimumCost(N, M, K, A, L, R)
print (out_)