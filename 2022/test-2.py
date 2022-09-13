def StringMerger (N, A):

    res = A[0]
    for s in A[1:]:
        len_l = len(res)
        len_r = len(s)
        l = min(len_l, len_r)

        for i in range(l):
            #print(i, res, s, s[:len_r - i])
            if s[len_r - i - 1] != res[-1]:
                continue

            for j in range(l - i):
                if s[len_r - i - 1 - j] != res[len_l - j - 1]:
                    break
            else:
                res += s[len_r - i:]
                print(1, res)
                break
        else:
            res += s
            print(2, res)

    return res

N = int(input())
A = input().split(" ")

print(A)
out_ = StringMerger(N, A)
print (out_)