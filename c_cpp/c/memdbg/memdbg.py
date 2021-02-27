#!/usr/bin/python3

fp = open('memdbg.log', 'r')

def Diff(li1, li2): 
    li_dif = [i for i in li1 + li2 if i not in li1 or i not in li2] 
    return li_dif 

memdbg_alloc = []
memdbg_free = []
memdbg_alloc_dict = dict()

line = fp.readline()
while line:
	line = fp.readline()
	arr = line.split()
	if len(arr) > 2:
		allocfree = arr[0]
		addr = arr[1]
		if allocfree == 'memdbg_alloc':
			memdbg_alloc.append(addr)
			memdbg_alloc_dict[addr] = line
		elif allocfree == 'memdbg_free':
			memdbg_free.append(addr)
		else:
			continue

alcal_diff = Diff(memdbg_alloc, memdbg_free) 
print('alloc/free diff:')
print(alcal_diff)

size = 0
for k in alcal_diff:
	if k in memdbg_alloc_dict:
		line = memdbg_alloc_dict[k]
		arr = line.split()
		isize = int( arr[2] )
		print(arr[4], isize)
		size = size + isize
print('total diff =', size)
