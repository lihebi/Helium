#!/usr/bin/env python3
from copy import copy
class A:
    dd=1
    d=dict()
    def __init__(self):
        print("Init")
        self.d=dict()
        print(self.d.keys())
        pass
    # def __copy__(self):
    #     cls = self.__class__
    #     result=cls.__new__(cls)
    #     # result.d=dict()
    #     return result

a = A();
lst=[]
a.dd=2;
a.d[1]=2
lst.append(a)
print(a.dd)
print(a.d[1])
a = A();
print(a.dd)
# print(a.d[1])
a.dd=3;
a.d[1] = 8
lst.append(a)

print("===")
for a in lst:
#    print(a.dd)
    print(a.d[1])
