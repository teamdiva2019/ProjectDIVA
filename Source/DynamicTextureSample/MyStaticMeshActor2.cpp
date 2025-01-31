// Fill out your copyright notice in the Description page of Project Settings.

// This class has been modified from the example by unreal engine forum user JR4815 in his response to the following thread:
// https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time

#include "DynamicTextureSample.h"
#include "MyStaticMeshActor2.h"
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <string.h>
#include <iostream>
#include <string>


#define RED 2
#define GREEN 1
#define BLUE 0
#define ALPHA 3

#define ALPHA_CHECK 200

// This is the name of the data file we will read. 
#define FILE_NAME "C:\\Users\\teamd\\Documents\\Unreal Projects\\DynamicTextureSample\\raw_data\\chesapeake_temp_"
#define FIRST_FILE_NAME "C:\\Users\\teamd\\Documents\\Unreal Projects\\DynamicTextureSample\\raw_data\\chesapeake_temp_229.nc4"
// There are 3 dimensions (lat, lon, time) and we will update the texture every .05 seconds 
#define DIMS 3
#define UPDATEINTERVAL 0.1


static short *mslp_in;
static int currz = 0, min = 2000, max = 3000;
static size_t lat = 358, lon = 243, time = 18;


static int getIndex(int x, int y, int z) {
	return x + y * lon + z * lat * lon;
}


static void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
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

	UE_LOG(LogTemp, Log, TEXT("UPDATETEXTUREREGIONS()"));
}


AMyStaticMeshActor2::AMyStaticMeshActor2(const class FObjectInitializer& PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	// ID variables and error handling
	int ncid, latid, lonid, retval;
	// Open the file. NC_NOWRITE tells netCDF we want read-only access to the file
	retval = nc_open(FIRST_FILE_NAME, NC_NOWRITE, &ncid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));
	// Get the dimension names
	retval = nc_inq_dimid(ncid, "lat", &latid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latid"));
	retval = nc_inq_dimid(ncid, "lon", &lonid);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: lonid"));
	/*retval = nc_inq_dimid(ncid, "time", &timeid);
	if (retval)
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timeid"));*/
	// Get the dimension lengths; these are used for initialization later on
	retval = nc_inq_dimlen(ncid, latid, &lat);
	UE_LOG(LogTemp, Error, TEXT("LAT: %d"), lat);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: latdim"));
	retval = nc_inq_dimlen(ncid, lonid, &lon);
	UE_LOG(LogTemp, Error, TEXT("LON: %d"), lon);
	if (retval)
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: londim"));
	/*retval = nc_inq_dimlen(ncid, timeid, &time);
	UE_LOG(LogTemp, Error, TEXT("TIME: %d"), time);
	if (retval)
	UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: timedim"));*/
	// Close file for now
	retval = nc_close(ncid);
	if (retval) {
		UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
	}

	mDynamicColors = nullptr;
	mUpdateTextureRegion = nullptr;


}

void AMyStaticMeshActor2::BeginPlay()
{
	// There will be netCDF IDs for the file and variables
	int ncid, mslpvarid;
	int start_day = 229;
	int stop_day = 246;
	int i;
	// Vectors used when reading in a varialbe
	size_t start[4], count[4];
	// Loop index and error handling
	int retval;
	short *mslp_temp;
	char file_name[100];
	char number_s[4];

	// Allocate space for array to read in
	mslp_in = (short *)malloc(time*lat*lon * sizeof(short));
	// Get the varid of the mean sea level pressure data variable, based on its name

	// Read the data
	// Set vectors for reading data
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	start[3] = 0;
	count[0] = 1;
	count[1] = 1;
	count[2] = lat;
	count[3] = lon;

	mslp_temp = mslp_in;

	for (i = start_day; i <= stop_day; i++) {

		strcpy(file_name, FILE_NAME);
		itoa(i, number_s, 10);
		strcat(file_name, number_s);
		strcat(file_name, ".nc4");

		UE_LOG(LogTemp, Error, TEXT("FileName: %s"), *FString(file_name));

		// Open the file. NC_NOWRITE tells netCDF we want read-only access to the file
		retval = nc_open(file_name, NC_NOWRITE, &ncid);
		if (retval)
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Open file"));

		retval = nc_inq_varid(ncid, "sst", &mslpvarid);
		if (retval)
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Get variable"));

		// Read in the data
		retval = nc_get_vara_short(ncid, mslpvarid, start, count, mslp_temp);
		if (retval)
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Read variable"));

		// Close the file  
		retval = nc_close(ncid);
		//// Temp max and min values because linear scaling isn't very pretty
		//min = 11500;
		//max = 6500;
		if (retval) {
			UE_LOG(LogTemp, Error, TEXT("ndfCDF Error: Close file"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("SUCCESS"));
			UE_LOG(LogTemp, Log, TEXT("MIN: %d"), min);
			UE_LOG(LogTemp, Log, TEXT("MAX: %d"), max);
		}

		mslp_temp += lat * lon;

	}

	UE_LOG(LogTemp, Log, TEXT("BEGINPLAY()"));
	Super::BeginPlay();
}

void AMyStaticMeshActor2::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete[] mDynamicColors; mDynamicColors = nullptr;
	delete mUpdateTextureRegion; mUpdateTextureRegion = nullptr;

	free(mslp_in);

	Super::EndPlay(EndPlayReason);
}

void AMyStaticMeshActor2::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetupTexture();
}

void AMyStaticMeshActor2::SetupTexture()
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
	UE_LOG(LogTemp, Log, TEXT("SETUPTEXTURE()"));
}

void AMyStaticMeshActor2::Tick(float DeltaTime)
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
				// Color scaling algo from:
				//http://stackoverflow.com/questions/2374959/algorithm-to-convert-any-positive-integer-to-an-rgb-value by Martin Beckett
				// Get the midpoint of your data values
				float mid = ((max)+(min)) / 2;
				//Get the range of your data values
				short range = (max)-(min);
				// The red value is a fraction between -1 and 1 where 0 is the midpoint
				float redvalue = 2 * (curr - mid) / range;
				// If the red value is positive, the value is > midpoint
				if (curr != -32768) {
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
		}

		// Once each pixel has had its RGB channels updated, update the texture and increment the timestep
		UpdateTexture();
		currz++;
		// Loop timestep if animation is over
		if (currz >= time) {
			currz = 0;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("TICK()"));
}

void AMyStaticMeshActor2::UpdateTexture()
{
	UpdateTextureRegions(mDynamicTexture, 0, 1, mUpdateTextureRegion, mDataSqrtSize, (uint32)4, mDynamicColors, false);
	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
	UE_LOG(LogTemp, Log, TEXT("UPDATETEXTURE()"));
}
