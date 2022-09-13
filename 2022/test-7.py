import math

class Solution:
    def minimumSize(self, nums, maxOperations):
        l, r = 1, max(nums)
        while l < r:
            mid = (l + r) // 2
            if sum([(n - 1) // mid for n in nums]) > maxOperations:
                l = mid + 1
            else:
                r = mid
        return l

def solve(N, K, A):
    s = Solution()
    return s.minimumSize(A, K)

T = int(input())
for _ in range(T):
    N = int(input())
    K = int(input())
    A = list(map(int, input().split()))

    out_ = solve(N, K, A)
    print (out_)