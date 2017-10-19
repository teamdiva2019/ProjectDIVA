from netCDF4 import Dataset
import numpy as np
import time
import sys
import os


file_path = './tmax.nc'
data = Dataset(file_path)
# limit is currently 500 MB
# limit = 1024 ** 3 / 2
allVariables = data.variables
# Sometimes we have time_bnds, lat_bnds, etc.
# Keep anything that doesn't have 'bnds'
varNames = list(filter(lambda x: 'bnds' not in x, list(allVariables.keys())))
# Remove the dimensions
varNames = list(filter(lambda x: x not in data.dimensions, varNames))
var = varNames[0]

if not os.path.exists(os.path.dirname('./data/metadata.txt')):
    os.makedirs(os.path.dirname('./data/metadata.txt'))
with open('./data/metadata.txt', 'w') as f:
    for var in varNames:
        f.write('Name: ' + allVariables[var].long_name + '\n')
        f.write(allVariables[var].name + '\n')
    varDims = allVariables[var].dimensions
    for dim in varDims:
        vals = allVariables[dim][:]
        f.write(' '.join(map(str, [dim, vals[0], vals[-1], vals[1] - vals[0]])) + '\n')
for var in varNames:
    outfile = './data/data_' + allVariables[var].name + '.bin'
    if not os.path.exists(os.path.dirname(outfile)):
        os.makedirs(os.path.dirname(outfile))
    data = allVariables[var][:].reshape(-1, np.prod(allVariables[var].shape[1:]))
    if isinstance(data, np.ma.core.MaskedArray):
        data.filled(np.nan).tofile(outfile)
    else:
        data.tofile(outfile)



