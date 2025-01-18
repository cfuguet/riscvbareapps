import random
import sys

m = int(sys.argv[1])
n = int(sys.argv[2])
nnz_ratio = float(sys.argv[3])

idx = []
p = [0]
for i in range(0, m):
  for j in range (0, n):
    if random.random() < nnz_ratio:
      idx.append(j)
  p.append(len(idx))

nnz = len(idx)
v = [random.random()*1000 for i in range (n)]
d = [random.random()*1000 for i in range (nnz)]

def spmv(p, d, idx, v):
  y = []
  for i in range(len(p)-1):
    yi = 0
    for k in range(p[i], p[i+1]):
      yi = yi + d[k]*v[idx[k]]

    y.append(yi)

  return y

def printCVec(dtype, name, values):
  outs = dtype + ' ' + name + '[] = {'
  outs += str(values[0])
  for i in values[1:]:
    outs += ', ' + str(i)
  outs += '};'
  print(outs)

print ('const int R = ' + str(n) + ';')
print ()
print ('const int nnz = ' + str(nnz) + ';')
print ()
printCVec('double', 'val', d)
print ()
printCVec('double', 'x', v)
print ()
printCVec('int', 'idx', idx)
print ()
printCVec('int', 'ptr', p)
print ()
printCVec('double', 'verify_data', spmv(p, d, idx, v))
