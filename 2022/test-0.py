def solve(N):
    num = list(N)

    if '3' not in num:
        num.sort()
        return "".join(num)

    index = num.index('3')
    res = [n for n in num]
    num1 = sorted(res[:index])
    res[:index] = num1
    left = index
    right = len(num) - 1
    for i in range(len(num) - 1, index - 1, -1):
        if (num[i] == '2'):
            res[left] = '2'
            left += 1
        else:
            res[right] = num[i]
            right -= 1
    return ''.join(res)

T = int(input())
for _ in range(T):
    N = input()

    out_ = solve(N)
    print(out_)