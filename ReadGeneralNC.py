from netCDF4 import Dataset
import numpy as np
import dask.array as da
import gc
import time
import sys
import os

# Read file path
#file_path = 'C:/Users/Erin/Documents/Unreal Projects/DynamicTextureSample/mslp.2002.nc'
# Default colors is blue for low and red for high
lowColor = np.array([0, 0, 255], dtype=int)
highColor = np.array([255, 0, 0], dtype=int)
# If there's still more then attempt to read colors
#if len(sys.argv) > 2:
#    lowColor = np.array(sys.argv[2:5], dtype=int)
#    highColor = np.array(sys.argv[5:8], dtype=int)
#print(lowColor)
#print(highColor)

data = Dataset(file_path)
allVariables = data.variables
# Sometimes we have time_bnds, lat_bnds, etc.
# Keep anything that doesn't have 'bnds'
varNames = list(filter(lambda x: 'bnds' not in x, list(allVariables.keys())))
# Remove the dimensions
varNames = list(filter(lambda x: x not in data.dimensions, varNames))
var = varNames[0]
if not os.path.exists(os.path.join(os.getcwd(), 'data')):
    os.makedirs(os.path.join(os.getcwd(), 'data'))
with open('./data/metadata.txt', 'w') as f:
    # For each variable, write the name of it
    for var in varNames:
        f.write('Name: ' + allVariables[var].long_name + '\n')
        f.write(allVariables[var].name + '\n')
    # Get the dimensions of a sample variable
    # By design, the other variables must have the
    # same dimensions
    varDims = allVariables[var].dimensions
    # For each dimension, write the name,
    # the start value, the end value,
    # and the step
    for dim in varDims:
        vals = allVariables[dim][:]
        f.write(' '.join(map(str, [dim, vals[0], vals[-1], vals[1] - vals[0]])) + '\n')
start = time.clock()
# For each variable
for var in varNames:
    # data = allVariables[var][:].reshape(-1, np.prod(allVariables[var].shape[1:]))
    flattened = allVariables[var][:].flatten()
    origShape = allVariables[var].shape
    print(flattened.shape, origShape)
    if isinstance(flattened, np.ma.core.MaskedArray):
        flattened = flattened.filled(np.nan)
    # Find the minimum value and the range of values.
    # Using these two we can make a percentage of how
    # far 'up' each value and simply convert colors
    # based on that. Because there's a chance of the data
    # having NaNs, I can't use ptp().
    lowVal = np.nanmin(flattened)
    ptp = np.nanmax(flattened) - lowVal
    # Subtract the min from each value and divide by ptp
    # and add a dimension for dot product later.
    percents = ((flattened - lowVal) / ptp)[np.newaxis, :]
    flattened = None
    gc.collect()
    # Calculate the color difference
    diff = (highColor - lowColor)[np.newaxis, :].T
    # Do the dot product to create a list of colors
    # Transpose so each color is each row. Also
    # add the low color
    colors = lowColor + np.dot(diff, percents).T
    # Round each value and cast to uint8 and finally reshape to
    # the original data
    colors = np.round(colors).astype(np.uint8)
    # Note: the color for missing data will be black
    # Uncomment line below for white
    # colors[np.all(colors == 0, axis=1)] = np.array([255, 255, 255])
    colors = colors.reshape(origShape + (3,))
    colors.tofile('./data/colors_' + allVariables[var].name + '.bin')
end = time.clock()
print(end - start, 'seconds.')
