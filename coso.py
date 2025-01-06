import numpy as np


coso = np.array([
                          0,  2,  2,  0,  0,  2,  4,
                          1,  0,  1,  0,  1,  0,  1,
                          4,  3,  0,  0,  2,  1,  0,
                          0,  0,  0,  0,  0,  0,  0,
                          0,  4,  4,  0,  0,  4,  2,
                          3,  0,  3,  0,  3,  0,  3,
                          2,  1,  0,  0,  4,  3,  0,
])
coso = coso.reshape((7,7))

for i in coso-1:
    print(', '.join([str(x) for x in i]))



