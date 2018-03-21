// Fill out your copyright notice in the Description page of Project Settings.

// This class has been modified from the example by unreal engine forum user JR4815 in his response to the following thread:
// https://forums.unrealengine.com/showthread.php?84856-Drawing-on-textures-in-real-time

#include <Python.h>
#include "DynamicTextureSample.h"
#include "MyStaticMeshActor.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>


// Color channel references
#define RED 2
#define GREEN 1
#define BLUE 0

// This is the name of the python script to execute for preprocessing. 
#define PROG_NAME "C:/Users/teamd/Documents/Unreal Projects/DynamicTextureSample/ColorProduction/GenColorsFromData.py"
// This is the name of the data file we will read. 
#define FILE_NAME "C:/Users/teamd/Documents/Unreal Projects/DynamicTextureSample/mslp.2002.nc"
// This is the name of the color map to use
#define MAP_NAME "magma"
// We will update the texture every .05 seconds
#define UPDATEINTERVAL 0.05

int currz = 0;
size_t lat, lon, time_len;
int currvar = 0;

std::vector<std::string> var_names;
std::vector<std::vector<std::string>> dim_values;
std::vector<std::vector<std::vector<uint8>>> var_values;
std::vector<std::string> shortnames;
std::vector<std::string> units;
std::vector<int> shape;

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

	UE_LOG(LogActor, Log, TEXT("Running python script for preprocessing"));
	int argc = 3;
	wchar_t *argv[3] = {
		L"" PROG_NAME,
		L"" FILE_NAME,
		L"" MAP_NAME,
	};
	Py_SetProgramName(argv[0]);
	Py_Initialize();
	PySys_SetArgv(argc, argv);
	FILE * python_prog = fopen(PROG_NAME, "r");
	int result = PyRun_AnyFile(python_prog, PROG_NAME);
	UE_LOG(LogActor, Error, TEXT("Ran preprocessing: result = %d"), result);

	UE_LOG(LogActor, Log, TEXT("Reading metadata"));
	std::ifstream metadata;
	std::string line;
	metadata.open("./metadata.txt");
	var_values.clear();
	var_names.clear();
	dim_values.clear();
	shortnames.clear();
	units.clear();
	shape.clear();

	if (getline(metadata, line)) {
		int num_vars = atoi(line.c_str());
		for (int i = 0; i < num_vars; i++) {
			// Read variable long name
			getline(metadata, line);
			var_names.push_back(line);
			// Read variable short name
			getline(metadata, line);
			shortnames.push_back(line);
			// Read the units of the variable
			getline(metadata, line);
			units.push_back(line);
		}
		// Shape of data
		getline(metadata, line);
		line = line.substr(1, line.size() - 2);
		line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
		std::string s;
		std::istringstream f(line);
		while (getline(f, s, ',')) {
			shape.push_back(atoi(s.c_str()));
		}
		time_len = shape.at(0);
		lat = shape.at(1);
		lon = shape.at(2);
		UE_LOG(LogTemp, Log, TEXT("time len: %d"), time_len);
		UE_LOG(LogTemp, Log, TEXT("lat len: %d"), lat);
		UE_LOG(LogTemp, Log, TEXT("lon len: %d"), lon);

		// Read in dimensions' start, stop, and step
		while (getline(metadata, line)) {
			std::vector<std::string> param_list;
			// Name of dimension
			getline(metadata, line);
			param_list.push_back(line);
			// Start
			getline(metadata, line);
			param_list.push_back(line);
			// Stop
			getline(metadata, line);
			param_list.push_back(line);
			// Step
			getline(metadata, line);
			param_list.push_back(line);

			dim_values.push_back(param_list);
		}
	}
	UE_LOG(LogActor, Log, TEXT("Metadata read"));
	

	mDynamicColors = nullptr;
	mUpdateTextureRegion = nullptr;
}

void AMyStaticMeshActor::BeginPlay()
{
	anim_play = true;
	UE_LOG(LogActor, Error, TEXT("Reading binary data"));
	// Read in the data for each variable
	for (int i = 0; i < var_names.size(); i++) {
		std::ifstream vardata;
		std::string curr_filename = std::string("./colors_") + shortnames.at(i) + std::string(".bin");
		vardata.open(curr_filename.c_str(), std::ios::in | std::ios::binary);
		std::vector<uint8> temp1;
		std::vector<std::vector<uint8>> temp2;
		while (vardata) {
			uint8 curr;
			for (int j = 0; j < 3; j++) {
				vardata.read(reinterpret_cast<char*>(&curr), sizeof(uint8));
				//UE_LOG(LogActor, Error, TEXT("%d"), curr);
				temp1.push_back(curr);
			}
			temp2.push_back(temp1);
			temp1.clear();
		}
		var_values.push_back(temp2);
		temp2.clear();
	}
	UE_LOG(LogActor, Error, TEXT("Done"));

	Super::BeginPlay();
}

void AMyStaticMeshActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete[] mDynamicColors; mDynamicColors = nullptr;
	delete mUpdateTextureRegion; mUpdateTextureRegion = nullptr;

	Py_Finalize();

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
	if (t - lastTick > UPDATEINTERVAL && anim_play)
	{
		lastTick = t;

		uint32 x, y;
		for (y = 0; y < lat; y++) {
			for (x = 0; x < lon; x++) {
				int indx = getIndex(x, lat - y, 0) * 4;
				mDynamicColors[indx + RED] = var_values.at(currvar).at(getIndex(x, y, currz)).at(0);
				mDynamicColors[indx + GREEN] = var_values.at(currvar).at(getIndex(x, y, currz)).at(1);
				mDynamicColors[indx + BLUE] = var_values.at(currvar).at(getIndex(x, y, currz)).at(2);
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
