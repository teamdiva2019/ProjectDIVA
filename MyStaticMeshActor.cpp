// Fill out your copyright notice in the Description page of Project Settings.

// This class has been modified from the example by unreal engine forum user JR4815 in his response to the following thread:
// https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time

#include "DynamicTextureSample.h"
#include "MyStaticMeshActor.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <Python.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>


#define RED 2
#define GREEN 1
#define BLUE 0
#define ALPHA 3

#define ALPHA_CHECK 200

// This is the name of the data file we will read. 
#define PROG_NAME "C:/Users/Erin/Documents/Unreal Projects/DynamicTextureSample-brokenbuild/ReadGeneralNC.py"
// There are 3 dimensions (lat, lon, time) and we will update the texture every .1 seconds
#define DIMS 3
#define UPDATEINTERVAL 0.05


float *temp_in;
int currz = 0;
// Temporary max and min values
float max = 108370.0f, min = 93320.0f;
size_t lat, lon, time_len;

std::vector<std::string> var_names;
std::vector<std::vector<std::string>> dim_values;
std::vector<std::vector<float>> var_values;
std::vector<std::string> shortnames;

int getIndex(int x, int y, int z) {
	return x + y * lon + z * lat * lon;
}

void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdateTextureRegionsData, FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}


AMyStaticMeshActor::AMyStaticMeshActor(const class FObjectInitializer& PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	/*// Run the python scrtipt on the given .nc file
	std::string stdOutErr =
	"import sys\n\
	class CatchOutErr:\n\
	def __init__(self):\n\
	self.value = ''\n\
	def write(self, txt):\n\
	self.value += txt\n\
	catchOutErr = CatchOutErr()\n\
	sys.stdout = catchOutErr\n\
	sys.stderr = catchOutErr\n\
	"; //this is python code to redirect stdouts/stderr*/

	UE_LOG(LogActor, Log, TEXT("Runing python script"));
	char * file = "file_path = 'C:\\Users\\Erin\\Documents\\Unreal Projects\\DynamicTextureSample-brokenbuild\\mslp.2002.nc'\n";
	Py_Initialize();
	PyObject *pModule = PyImport_AddModule("__main__"); //create main module
														//PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect
	PyRun_SimpleString(file);
	PyObject* PyFileObject = PyFile_FromString(PROG_NAME, "r");
	int python_result = PyRun_SimpleFileEx(PyFile_AsFile(PyFileObject), PROG_NAME, 0);
	//PyObject *catcher = PyObject_GetAttrString(pModule,"catchOutErr"); //get our catchOutErr created above
	//PyErr_Print(); //make python print any errors
	//PyObject *output = PyObject_GetAttrString(catcher,"value"); //get the stdout and stderr from our catchOutErr object
	//char * str = PyString_AsString(output);
	//FString Fs(str);
	//UE_LOG(LogActor, Error, TEXT("result = %s"), *Fs);
	UE_LOG(LogActor, Log, TEXT("Script exited: %d"), python_result);
	UE_LOG(LogActor, Log, TEXT("Finalized; reading metadata"));

	std::ifstream metadata;
	std::string line;
	metadata.open("./data/metadata.txt");
	var_values.clear();
	var_names.clear();
	shortnames.clear();
	while (getline(metadata, line)) {
		if (line.substr(0, 5) == "Name:") {
			// Read in the name of a variable
			line = line.substr(6);
			var_names.push_back(line);
			getline(metadata, line);
			shortnames.push_back(line);
		}
		else {
			// Read in the name, start, stop, and step of a variable
			std::string s;
			std::istringstream f(line);
			std::vector<std::string> param_list;
			while (getline(f, s, ' ')) {
				param_list.push_back(s);
			}
			if (param_list.at(0) == "time") {
				time_len = (size_t)int(abs((atof(param_list[2].c_str()) - atof(param_list[1].c_str())) / atof(param_list[3].c_str())));
				UE_LOG(LogTemp, Log, TEXT("time len: %d"), time_len);
			}
			if (param_list.at(0) == "lat") {
				lat = (size_t)int(abs((atof(param_list[2].c_str()) - atof(param_list[1].c_str())) / atof(param_list[3].c_str())));
				UE_LOG(LogTemp, Log, TEXT("lat len: %d"), lat);
			}
			if (param_list.at(0) == "lon") {
				lon = (size_t)int(abs((atof(param_list[2].c_str()) - atof(param_list[1].c_str())) / atof(param_list[3].c_str())));
				UE_LOG(LogTemp, Log, TEXT("lon len: %d"), lon);
			}
			dim_values.push_back(param_list);
		}
	}
	UE_LOG(LogActor, Log, TEXT("Metadata read"));
	

	mDynamicColors = nullptr;
	mUpdateTextureRegion = nullptr;


}

void AMyStaticMeshActor::BeginPlay()
{
	/*
	// There will be netCDF IDs for the file and variables
	int ncid, varid;
	// Vectors used when reading in a varialbe
	size_t start[DIMS], count[DIMS];
	// Loop index and error handling
	int retval;
	// Open the file. NC_NOWRITE tells netCDF we want read-only access to the file
	retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	// Allocate space for array to read in
	temp_in = (float *)malloc(time*lat*lon * sizeof(float));
	// Get the varid of the mean sea level pressure data variable, based on its name
	retval = nc_inq_varid(ncid, "TMAX_2maboveground", &varid);
	if (retval)
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get variable"));

	// Read the data
	// Set vectors for reading data
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	count[0] = time;
	count[1] = lat;
	count[2] = lon;

	// Read in the data
	retval = nc_get_vara_float(ncid, varid, start, count, temp_in);
	if (retval)
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));

	// Close the file
	retval = nc_close(ncid);
	if (retval) {
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}
	else {
	UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
	UE_LOG(LogTemp, Log, TEXT("MIN: %f"), min);
	UE_LOG(LogTemp, Log, TEXT("MAX: %f"), max);
	}
	*/

	UE_LOG(LogActor, Error, TEXT("Reading binary data"));
	// Read in the data for each variable
	for (int i = 0; i < var_names.size(); i++) {
		std::ifstream vardata;
		std::string curr_filename = std::string("./data/data_") + shortnames.at(i) + std::string(".bin");
		vardata.open(curr_filename.c_str(), std::ios::in | std::ios::binary);
		std::vector<float> temp;
		while (vardata) {
			float curr;
			vardata.read(reinterpret_cast<char*>(&curr), sizeof(float));
			temp.push_back(curr);
		}
		var_values.push_back(temp);
	}
	UE_LOG(LogActor, Error, TEXT("Done"));

	Super::BeginPlay();
}

void AMyStaticMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete[] mDynamicColors; mDynamicColors = nullptr;
	delete mUpdateTextureRegion; mUpdateTextureRegion = nullptr;

	Py_Finalize();
	// Free the data
	//free(temp_in);

	Super::EndPlay(EndPlayReason);
}

void AMyStaticMeshActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetupTexture();
}

void AMyStaticMeshActor::SetupTexture()
{
	if (mDynamicColors) delete[] mDynamicColors;
	if (mUpdateTextureRegion) delete mUpdateTextureRegion;

	mDynamicMaterials.Empty();
	mDynamicMaterials.Add(GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0));
	mDynamicTexture = UTexture2D::CreateTransient(lon, lat);
	mDynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	mDynamicTexture->SRGB = 0;
	mDynamicTexture->Filter = TextureFilter::TF_Nearest;
	mDynamicTexture->AddToRoot();
	mDynamicTexture->UpdateResource();

	mUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, lon, lat);

	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);

	mDataSize = lon * lat * 4;
	mDataSqrtSize = lon * 4;
	mArraySize = lon * lat;
	mArrayRowSize = lon;

	mDynamicColors = new uint8[mDataSize];

	memset(mDynamicColors, 0, mDataSize);
}

void AMyStaticMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float t = GetWorld()->GetTimeSeconds();

	// Only update the texture every update interval
	if (t - lastTick > UPDATEINTERVAL)
	{
		lastTick = t;

		uint32 x, y;
		for (y = 0; y < lat; y++) {
			for (x = 0; x < lon; x++) {
				int indx = getIndex(x, lat - y, 0) * 4;
				float curr = var_values.at(0).at(getIndex(x, y, currz));
				curr = (curr < 0) ? curr * -1 : curr;
				// Color scaling algo from:
				//http://stackoverflow.com/questions/2374959/algorithm-to-convert-any-positive-integer-to-an-rgb-value by Martin Beckett
				// Get the midpoint of your data values
				float mid = ((max)+(min)) / 2;
				//Get the range of your data values
				short range = (max)-(min);
				// The red value is a fraction between -1 and 1 where 0 is the midpoint
				float redvalue = 2 * (curr - mid) / range;
				// If the red value is positive, the value is > midpoint
				if (redvalue > 0 && redvalue < 1) {
					// red value is a fraction of the max value, 255 and green is the remaining fraction
					mDynamicColors[indx + RED] = redvalue * 255;
					mDynamicColors[indx + GREEN] = 255 - (redvalue * 255);
					mDynamicColors[indx + BLUE] = 0;
				}
				// If the red value is negative, the value is < midpoint
				else if (redvalue < 0 && redvalue > -1) {
					// inverse of red value becomes the new blue value, and the remainig fraction is green
					mDynamicColors[indx + RED] = 0;
					mDynamicColors[indx + GREEN] = 255 - (redvalue * -1 * 255);
					mDynamicColors[indx + BLUE] = redvalue * -1 * 255;
				}
				// Positive but off the scale
				else if (redvalue > 0) {
					mDynamicColors[indx + RED] = 255;
					mDynamicColors[indx + GREEN] = 0;
					mDynamicColors[indx + BLUE] = 0;
				}
				// Negative but off the scale
				else {
					mDynamicColors[indx + RED] = 0;
					mDynamicColors[indx + GREEN] = 0;
					mDynamicColors[indx + BLUE] = 255;
				}
			}
		}

		// Once each pixel has had its RGB channels updated, update the texture and increment the timestep
		UpdateTexture();
		currz++;
		// Loop timestep if animation is over
		if (currz >= time_len) {
			currz = 0;
		}
	}
}

void AMyStaticMeshActor::UpdateTexture()
{
	UpdateTextureRegions(mDynamicTexture, 0, 1, mUpdateTextureRegion, mDataSqrtSize, (uint32)4, mDynamicColors, false);
	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
}

/*// Fill out your copyright notice in the Description page of Project Settings.

// This class has been modified from the example by unreal engine forum user JR4815 in his response to the following thread:
// https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time

#include "DynamicTextureSample.h"
#include "MyStaticMeshActor.h"
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <iostream>
#include <string>


#define RED 2
#define GREEN 1
#define BLUE 0
#define ALPHA 3

#define ALPHA_CHECK 200

// This is the name of the data file we will read.
#define FILE_NAME "C:\\Users\\Erin\\Documents\\Unreal Projects\\DynamicTextureSample\\mslp.2002.nc"
// There are 3 dimensions (lat, lon, time) and we will update the texture every .05 seconds
#define DIMS 3
#define UPDATEINTERVAL 0.05


short *mslp_in;
int currz = 0, min = 16000, max = -40000;
size_t lat, lon, time;


int getIndex(int x, int y, int z) {
return x + y * lon + z * lat * lon;
}

void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
if (Texture && Texture->Resource)
{
struct FUpdateTextureRegionsData
{
FTexture2DResource* Texture2DResource;
int32 MipIndex;
uint32 NumRegions;
FUpdateTextureRegion2D* Regions;
uint32 SrcPitch;
uint32 SrcBpp;
uint8* SrcData;
};

FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
RegionData->MipIndex = MipIndex;
RegionData->NumRegions = NumRegions;
RegionData->Regions = Regions;
RegionData->SrcPitch = SrcPitch;
RegionData->SrcBpp = SrcBpp;
RegionData->SrcData = SrcData;

ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
UpdateTextureRegionsData,
FUpdateTextureRegionsData*, RegionData, RegionData,
bool, bFreeData, bFreeData,
{
for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
{
int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
if (RegionData->MipIndex >= CurrentFirstMip)
{
RHIUpdateTexture2D(
RegionData->Texture2DResource->GetTexture2DRHI(),
RegionData->MipIndex - CurrentFirstMip,
RegionData->Regions[RegionIndex],
RegionData->SrcPitch,
RegionData->SrcData
+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
);
}
}
if (bFreeData)
{
FMemory::Free(RegionData->Regions);
FMemory::Free(RegionData->SrcData);
}
delete RegionData;
});
}
}


AMyStaticMeshActor::AMyStaticMeshActor(const class FObjectInitializer& PCIP)
{
PrimaryActorTick.bCanEverTick = true;

// ID variables and error handling
int ncid, latid, lonid, timeid, retval;
// Open the file. NC_NOWRITE tells netCDF we want read-only access to the file
retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
// Get the dimension names
retval = nc_inq_dimid(ncid, "lat", &latid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latid"));
retval = nc_inq_dimid(ncid, "lon", &lonid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: lonid"));
retval = nc_inq_dimid(ncid, "time", &timeid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timeid"));
// Get the dimension lengths; these are used for initialization later on
retval = nc_inq_dimlen(ncid, latid, &lat);
UE_LOG(LogTemp, Error, TEXT("LAT: %d"), lat);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latdim"));
retval = nc_inq_dimlen(ncid, lonid, &lon);
UE_LOG(LogTemp, Error, TEXT("LON: %d"), lon);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: londim"));
retval = nc_inq_dimlen(ncid, timeid, &time);
UE_LOG(LogTemp, Error, TEXT("TIME: %d"), time);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timedim"));
// Close file for now
retval = nc_close(ncid);
if (retval) {
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
}

mDynamicColors = nullptr;
mUpdateTextureRegion = nullptr;


}

void AMyStaticMeshActor::BeginPlay()
{
// There will be netCDF IDs for the file and variables
int ncid, mslpvarid;
// Vectors used when reading in a varialbe
size_t start[DIMS], count[DIMS];
// Loop index and error handling
int retval;
// Open the file. NC_NOWRITE tells netCDF we want read-only access to the file
retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
// Allocate space for array to read in
mslp_in = (short *)malloc(time*lat*lon * sizeof(short));
// Get the varid of the mean sea level pressure data variable, based on its name
retval = nc_inq_varid(ncid, "mslp", &mslpvarid);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get variable"));

// Read the data
// Set vectors for reading data
start[0] = 0;
start[1] = 0;
start[2] = 0;
count[0] = time;
count[1] = lat;
count[2] = lon;

// Read in the data
retval = nc_get_vara_short(ncid, mslpvarid, start, count, mslp_in);
if (retval)
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));

// Close the file
retval = nc_close(ncid);
// Temp max and min values because linear scaling isn't very pretty
min = 11500;
max = 6500;
if (retval) {
UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
}
else {
UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
UE_LOG(LogTemp, Log, TEXT("MIN: %d"), min);
UE_LOG(LogTemp, Log, TEXT("MAX: %d"), max);
}


Super::BeginPlay();
}

void AMyStaticMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
delete[] mDynamicColors; mDynamicColors = nullptr;
delete mUpdateTextureRegion; mUpdateTextureRegion = nullptr;

free(mslp_in);

Super::EndPlay(EndPlayReason);
}

void AMyStaticMeshActor::PostInitializeComponents()
{
Super::PostInitializeComponents();

SetupTexture();
}

void AMyStaticMeshActor::SetupTexture()
{
if (mDynamicColors) delete[] mDynamicColors;
if (mUpdateTextureRegion) delete mUpdateTextureRegion;

mDynamicMaterials.Empty();
mDynamicMaterials.Add(GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0));
mDynamicTexture = UTexture2D::CreateTransient(lon, lat);
mDynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
mDynamicTexture->SRGB = 0;
mDynamicTexture->Filter = TextureFilter::TF_Nearest;
mDynamicTexture->AddToRoot();
mDynamicTexture->UpdateResource();

mUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, lon, lat);

mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);

mDataSize = lon * lat * 4;
mDataSqrtSize = lon * 4;
mArraySize = lon * lat;
mArrayRowSize = lon;

mDynamicColors = new uint8[mDataSize];

memset(mDynamicColors, 0, mDataSize);
}

void AMyStaticMeshActor::Tick(float DeltaTime)
{
Super::Tick(DeltaTime);

float t = GetWorld()->GetTimeSeconds();

// Only update the texture every update interval
if (t - lastTick > UPDATEINTERVAL)
{
lastTick = t;

// Go through each pixel and set RGB values
uint32 x, y;
for (y = 0; y < lat; y++) {
for (x = 0; x < lon; x++) {
int indx = getIndex(x, y, 0) * 4;
float curr = mslp_in[getIndex(x, y, currz)];
curr = (curr < 0) ? curr * -1 : curr;
// Color scaling algo from:
//http://stackoverflow.com/questions/2374959/algorithm-to-convert-any-positive-integer-to-an-rgb-value by Martin Beckett
// Get the midpoint of your data values
float mid = ((max)+(min)) / 2;
//Get the range of your data values
short range = (max)-(min);
// The red value is a fraction between -1 and 1 where 0 is the midpoint
float redvalue = 2 * (curr - mid) / range;
// If the red value is positive, the value is > midpoint
if (redvalue > 0 && redvalue < 1) {
// red value is a fraction of the max value, 255 and green is the remaining fraction
mDynamicColors[indx + RED] = redvalue * 255;
mDynamicColors[indx + GREEN] = 255 - (redvalue * 255);
mDynamicColors[indx + BLUE] = 0;
}
// If the red value is negative, the value is < midpoint
else if (redvalue < 0 && redvalue > -1) {
// inverse of red value becomes the new blue value, and the remainig fraction is green
mDynamicColors[indx + RED] = 0;
mDynamicColors[indx + GREEN] = 255 - (redvalue * -1 * 255);
mDynamicColors[indx + BLUE] = redvalue * -1 * 255;
}
// Positive but off the scale
else if (redvalue > 0) {
mDynamicColors[indx + RED] = 255;
mDynamicColors[indx + GREEN] = 0;
mDynamicColors[indx + BLUE] = 0;
}
// Negative but off the scale
else {
mDynamicColors[indx + RED] = 0;
mDynamicColors[indx + GREEN] = 0;
mDynamicColors[indx + BLUE] = 255;
}
}
}

// Once each pixel has had its RGB channels updated, update the texture and increment the timestep
UpdateTexture();
currz++;
// Loop timestep if animation is over
if (currz >= time) {
currz = 0;
}
}
}

void AMyStaticMeshActor::UpdateTexture()
{
UpdateTextureRegions(mDynamicTexture, 0, 1, mUpdateTextureRegion, mDataSqrtSize, (uint32)4, mDynamicColors, false);
mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
}*/
