import bisect

def find_lt(a, x):
    'Find rightmost value less than x'
    i = bisect.bisect_left(a, x)
    if i:
        return i-1
    return False

def find_ge(a, x):
    'Find leftmost item greater than or equal to x'
    i = bisect.bisect_left(a, x)
    if i != len(a):
        return i
    return False

def find_gt(a, x):
    'Find leftmost value greater than x'
    i = bisect.bisect_right(a, x)
    if i != len(a):
        return i
    return False

def maximizethenumber(length, arr):
    if length == 1:
        return arr[0]

    arr.sort()
    while len(arr) > 1:
        #print(len(arr), arr)
        negative = []
        n = find_lt(arr, 0)
        if n is not False:
            negative = arr[:n+1]

        positive = []
        p = find_ge(arr, 0)
        if p is not False:
            positive = sorted(arr[p:], reverse=True)

        #print("n", len(negative), negative)
        #print("p", len(positive), positive)

        len_l = len(negative)
        len_r = len(positive)
        len_c = min(len_l, len_r)

        if len_l == 1:
            return sum(positive) - negative[0]

        if len_r == 1:
            return positive[0] - sum(negative)

        negative_new = []
        positive_new = []
        for i in range(len_c):
            if i % 2:
                v = negative[i] - positive[i]
                negative_new.append(v)
            else:
                v = positive[i] - negative[i]
                positive_new.append(v)

        negative = sorted(negative[len_c:] + negative_new)
        positive = sorted(positive[len_c:] + positive_new)

        #print("n1", len(negative), negative)
        #print("p1", len(positive), positive)

        if len(negative) == 0 and len(positive) > 1:
            v = positive[0] - positive[-1]
            positive.pop(-1)
            positive.pop(0)
            negative.append(v)

        if len(positive) == 0 and len(negative) > 1:
            v = negative[-1] - negative[0]
            negative.pop(-1)
            negative.pop(0)
            positive.append(v)

        arr = negative + positive
    return abs(arr[0])

T = int(input())
for _ in range(T):
    n = int(input())
    arr = list(map(int, input().split()))

    out_ = maximizethenumber(n, arr)
    print(out_)