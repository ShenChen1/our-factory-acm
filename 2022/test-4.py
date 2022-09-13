class Solution:
    def egcd(self, a, b):
        if a == 0:
            return (b, 0, 1)
        else:
            g, y, x = self.egcd(b % a, a)
            return (g, x - (b // a) * y, y)

    def modinv(self, a, m):
        g, x, y = self.egcd(a, m)
        if g != 1:
            raise Exception('modular inverse does not exist')
        else:
            return x % m

    def __init__(self, N, A):
        self.n_nodes = N

        self.edges = []
        self.n_edges = 0
        self.cache = []

        self.graph = [[None for i in range(j+1)] for j in range(N+1)]
        for e in A:
            (start, end, cost) = e
            # exchange
            if start < end:
                start, end = end, start

            # the same edge
            if self.graph[start][end] is not None:
                index = self.graph[start][end]
                v = min(self.edges[index][2], cost)
                self.edges[index] = (start, end, v)
                continue

            self.edges.append((start, end, cost))
            self.n_edges += 1

        # sort
        self.edges = sorted(self.edges, key=lambda item:item[2], reverse=False)
        for i in range(self.n_edges):
            (start, end, cost) = self.edges[i]
            self.graph[start][end] = i

    def find(self, fa, x):
        if x != fa[x]:
            fa[x] = self.find(fa, fa[x])
        return fa[x]

    def kruscal(self, edges):
        path = []
        fa = [n for n in range(self.n_nodes + 1)]
        total = 0
        num = self.n_nodes
        for e in edges:
            (start, end, cost) = e
            t1 = self.find(fa, start)
            t2 = self.find(fa, end)
            if t1 == t2:
                continue
            fa[t1] = t2
            total += cost
            #print("cost", cost)
            path.append((start, end))
            num -= 1
            if num == 1:
                break
        #print(total)
        return (total, path)

    def run(self):
        ans = 0
        time = 0

        for i in range(1, self.n_nodes + 1):
            for j in range(1, i):
                e = None
                if self.graph[i][j] is not None:
                    e = self.edges.pop(self.graph[i][j])
                self.edges.insert(0, (i, j, 0))

                print(self.edges)
                res, _ = self.kruscal(self.edges)
                print(res, _)
                ans += res
                time += 1

                self.edges.pop(0)
                if e is not None:
                    self.edges.insert(self.graph[i][j], e)

        m = 1000000007
        a = ans
        b = time
        b1 = self.modinv(b, m)
        return (a * b1) % m

def solve(N, M, A):
    s = Solution(N, A)
    return s.run()

N = int(input())
M = int(input())
A = [list(map(int, input().split())) for i in range(M)]

out_ = solve(N, M, A)
print (out_)
