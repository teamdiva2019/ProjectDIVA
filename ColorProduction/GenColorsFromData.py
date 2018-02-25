import numpy as np
import matplotlib
import matplotlib.cm as cm
from matplotlib.colors import ListedColormap, LinearSegmentedColormap
from netCDF4 import Dataset, num2date
import GenColorMap
import time
import sys
import os

# Get script directory
scriptDir = os.path.dirname(os.path.realpath(__file__))
# Read file path
filename = ''
try:
    filename = sys.argv[1]
except IndexError:
    print('No file name!')
    sys.exit(1)
data = Dataset(filename)
allVariables = data.variables
# print(allVariables)
# Sometimes we have time_bnds, lat_bnds, etc.
# Keep anything that doesn't have 'bnds'
varNames = list(filter(lambda x: 'bnds' not in x, list(allVariables.keys())))
# Remove the dimensions
varNames = list(filter(lambda x: x not in data.dimensions, varNames))
var = varNames[0]
with open('metadata.txt', 'w') as f:
    # Write the number of variables
    f.write('%d\n' % len(varNames))
    # For each variable, write the name of it
    for var in varNames:
        f.write(allVariables[var].long_name +'\n' + allVariables[var].name + '\n' + allVariables[var].units + '\n')
    # Only need to print shape once, as all variables need to have
    # same shape
    f.write(str(allVariables[var].shape) + '\n')
    # Get the dimensions of a sample variable
    # By design, the other variables must have the
    # same dimensions
    varDims = allVariables[var].dimensions
    # For each dimension, write the name,
    # the start value, the end value,
    # and the step
    for dim in varDims:
        vals = allVariables[dim][:]
        if dim == 'time':
            timeUnit = allVariables[dim].units
            try:
                timeCalendar = allVariables[dim].calendar
            except AttributeError:
                timeCalendar = u"gregorian"
            endpoints = num2date([vals[0], vals[-1]], units=timeUnit, calendar=timeCalendar)
            timeStep = num2date(vals[1], units=timeUnit, calendar=timeCalendar) - endpoints[0]
            f.write('\n'.join(map(str, [dim, endpoints[0], endpoints[1], timeStep])) + '\n')
        else:
            f.write('\n'.join(map(str, [dim, vals[0], vals[-1], vals[1] - vals[0]])) + '\n')

start = time.clock()
# The second argument is either the
# name of the color map, which may be followed by color
# stops if the map isn't available
# to generate a perceptually uniform color map

colorMap = ''
try:
    colorMap = sys.argv[2]
except IndexError:
    print('No color map!')
    sys.exit(1)
# colorMap = 'viridis'
# Check if the color map exists
if not os.path.isfile(scriptDir + './/ColorMaps//' + colorMap + '.txt'):
    # Make the colormap in another file
    GenColorMap.runScript(sys.argv[2:])

# Read the colormap
PUcols = np.loadtxt(scriptDir + './/ColorMaps//' + colorMap + '.txt')

# Create the Color map object
gendMap = ListedColormap(PUcols, N=len(PUcols))
# Set nan values as black
gendMap.set_under('black', 1)

# Check to see if the data is vectorized
if len(varNames) > 1:
    pass

# Read in all the data and fill in nans
# if necessary
data = allVariables[var][:].flatten()
if isinstance(data, np.ma.core.MaskedArray):
    data = data.filled(np.nan)
# Find the min and max
minVal = np.nanmin(data)
maxVal = np.nanmax(data)

# Normalize the data and create a ScalarMappable object.
# This allows us to efficiently map the values to colors
norm = matplotlib.colors.Normalize(vmin=minVal, vmax=maxVal)
mapper = cm.ScalarMappable(norm=norm, cmap=gendMap)

# Map the data. Take the first 3 columns as last column is alpha
colorMappedData = mapper.to_rgba(data, alpha=False, bytes=True)[:,:3]

# Save the data to a binary file to minimize size
colorMappedData.tofile('colors_' + var + '.bin')

end = time.clock()
print(end - start, 'seconds.')