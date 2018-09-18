import sys

a = []
b = a
print(sys.getrefcount(a)) # 3: a, b, and the argument passed to sys.getrefcount()
