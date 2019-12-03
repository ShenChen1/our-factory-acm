def count_right_numbers(s):
    z = int
    r = 0
    u = [-1]
    N = [[0]*11 for i in s]
    def f(a, i):
        n = N[i][a]
        if n < 1:
            n = 1
            j = i + 1
            if j < len(s):
                w = z(s[j])
                t = a + w
                m = t // 2
                n = f(m, j) + t % 2 * f(m + 1, j)
                if abs(w - z(s[i])) > 1:
                    u[0] = 0
        N[i][a] = n
        return n
    for n in range(10):
        r += f(n, 0)
    return r + u[0]