// Copyright 2018 Fan Jiang <i@fanjiang.me> All Rights Reserved. This source file is subject to multiple Chinese and International Copyright Laws and Patents. Unauthorized Redistribution is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
//#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/Paths.h"
#include "PixelFormat.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "assimp/postprocess.h"
#include "RobotMeshComponent.generated.h"

UENUM(BlueprintType)
enum class ERobotMeshType : uint8
{
    Mesh = 0,
    Cylinder,
    Box
};

UENUM(BlueprintType)
enum class ERobotComponentType : uint8
{
    Visual = 0,
    Collision,
    Both
};

/**
 * 
 */
UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class PAVILIONPLUGIN_API URobotMeshComponent : public URuntimeMeshComponent
{
	GENERATED_UCLASS_BODY()
	
public:
    
    UFUNCTION(BlueprintCallable, Category = "Assimp")
    bool openMesh(FString path, int32& SectionCount, FString& ErrorCode);
    
    UFUNCTION(BlueprintCallable, Category = "Assimp")
    bool openCollisionMesh(FString path, FString& ErrorCode);
    
    UFUNCTION(BlueprintCallable, Category = "Assimp")
    bool getSection(int32 index, TArray<FVector>& Vertices, TArray<int32>& Faces, TArray<FVector>& Normals, TArray<FVector2D>& UV, TArray<FRuntimeMeshTangent>& Tangents);
    
    UFUNCTION(BlueprintCallable, Category = "Assimp")
    void clear();
    
    virtual void InitializeComponent() override;
    
    virtual void OnRegister() override;
    
    virtual void OnCreatePhysicsState() override;
    
    virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
    
    UPROPERTY(EditAnywhere)
    FString ModelPath = TEXT("/Users/proffan/Notebooks/Robotics/SDF/pr2/meshes/gripper_v0/l_finger_tip.dae");
    
    UPROPERTY(EditAnywhere)
    FString CollisionPath = TEXT("/Users/proffan/Notebooks/Robotics/SDF/pr2/meshes/gripper_v0/l_finger_tip.stl");
    
    UPROPERTY(EditAnywhere)
    UMaterialInterface* BaseMaterial;
    
    UPROPERTY(EditAnywhere)
    bool MakeLeftHanded = false;
    
    UPROPERTY(EditAnywhere)
    bool ApplyTransformation = true;
    
    UPROPERTY(EditAnywhere)
    FVector ModelScaleTransform = FVector(10.f, -10.f, 10.f);
    
    UPROPERTY(EditAnywhere)
    FVector CollisionScaleTransform = FVector(100.f, -100.f, 100.f);
    
    UPROPERTY(EditAnywhere)
    FVector NormalTransform = FVector(0.1f, -0.1f, 0.1f);
    
    UPROPERTY(EditAnywhere)
    ERobotMeshType MeshType = ERobotMeshType::Mesh;
    
    UPROPERTY(EditAnywhere)
    ERobotComponentType ComponentType = ERobotComponentType::Both;
    
    UPROPERTY(EditAnywhere)
    FVector2D CylinderSize = FVector2D(0.f, 0.f);
    
    UPROPERTY(EditAnywhere)
    FVector BoxSize = FVector(0.f, 0.f, 0.f);
private:
    int32 _selectedVertex;
    int32 _meshCurrentlyProcessed;
    bool _addModifier;
    int _lastModifiedTime;
    bool _requiresFullRecreation;
    
    TArray<TArray<FVector>> _vertices;
    TArray<TArray<int32>> _indices;
    TArray<TArray<FVector>> _normals;
    TArray<TArray<FVector2D>> _uvs;
    TArray<TArray<FRuntimeMeshTangent>> _tangents;
    TArray<TArray<FColor>> _vertexColors;
    TArray<UMaterialInstanceDynamic*> _materials;
    TArray<UTexture*> _textures;
    TArray<UTexture*> _tnormals;
    TArray<TArray<FVector>> _convexCollision;
    
    void processMesh(aiMesh* mesh, const aiNode* node, const aiScene* scene);
    void processNode(aiNode* node, const aiScene* scene);
    void processMaterial(aiMaterial* material, const aiScene* scene);
    UTexture2D* LoadTextureFromPath(const FString& Path);
    UTexture2D* LoadImageFromDisk(UObject* Outer, const FString& ImagePath);
    UTexture2D* CreateTexture(UObject* Outer, const TArray<uint8>& PixelData, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat, FName BaseName);
    
    void DecomposeMeshToHulls(TArray< TArray<FVector> >& ConvexMeshes, const TArray<FVector>& InVertices, const TArray<uint32>& InIndices, float InAccuracy, int32 InMaxHullVerts);
};
