// Copyright 2018 Fan Jiang <i@fanjiang.me> All Rights Reserved. This source file is subject to multiple Chinese and International Copyright Laws and Patents. Unauthorized Redistribution is strictly prohibited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "RobotMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "DrawDebugHelpers.h"
#include <iostream>
#include <sdf/sdf.hh>
#include "RobotActor.generated.h"

UCLASS()
class PAVILIONPLUGIN_API ARobotActor : public AActor
{
    GENERATED_BODY()
	
public:	
    // Sets default values for this actor's properties
    ARobotActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void LoadModel(const FTransform &Transform);
    void LoadModelOnCreation(const FObjectInitializer& ObjectInitializer);
    FRotator URDFRotationToUnreal(ignition::math::Quaterniond);
    FVector URDFPositionToUnreal(ignition::math::Vector3d);

    FString GetRealPathFromURI(std::string uri);
public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    virtual void OnConstruction(const FTransform &Transform) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;

    UPROPERTY(EditAnywhere)
    FString URDFPath = TEXT("/Users/proffan/Notebooks/Robotics/SDF/pr2/model.sdf");
	
	
};
