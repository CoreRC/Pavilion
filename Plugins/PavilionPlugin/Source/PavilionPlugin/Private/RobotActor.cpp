// Copyright 2018 Fan Jiang <i@fanjiang.me> All Rights Reserved. This source file is subject to multiple Chinese and International Copyright Laws and Patents. Unauthorized Redistribution is strictly prohibited.

#include "RobotActor.h"

class LStream : public std::stringbuf{
protected:
    int sync() {
        UE_LOG(LogTemp, Log, TEXT("%s"), *FString(str().c_str()));
        str("");
        return std::stringbuf::sync();
    }
};

FRotator ARobotActor::URDFRotationToUnreal(ignition::math::Quaterniond quat) {
    return FRotator(FQuat(quat.X(),quat.Y(),quat.Z(),quat.W()));
}

FVector ARobotActor::URDFPositionToUnreal(ignition::math::Vector3d vec) {
    return FVector(vec.X() * 100.0, vec.Y() * -100.0, vec.Z() * 100.0);
}

FString ARobotActor::GetRealPathFromURI(std::string uri){
    // NOTE: URI looks like model://pr2/meshes/shoulder_v0/shoulder_pan.dae
    if(uri.find("model://") != 0){
        return FString();
    }

    return FPaths::ConvertRelativePathToFull(FPaths::ConvertRelativePathToFull(FPaths::GetPath(URDFPath),".."), uri.substr(8,uri.length()-8).c_str());
}

// Sets default values
ARobotActor::ARobotActor(const FObjectInitializer& ObjectInitializer ): Super(ObjectInitializer)
{
    
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("UnrealRoot");

    UE_LOG(LogTemp, Warning, TEXT("ARobotActor ctor(ObjectInitializer)"));

    LoadModelOnCreation(ObjectInitializer);
}

void ARobotActor::LoadModelOnCreation(const FObjectInitializer& ObjectInitializer){
    LStream Stream;
    std::cout.rdbuf(&Stream);

    std::string filename(TCHAR_TO_UTF8(*URDFPath));

    sdf::SDFPtr sdfElement(new sdf::SDF());
    sdf::init(sdfElement);
    if (!sdf::readFile(filename, sdfElement))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not a valid SDF file!"), *URDFPath);
        return;
    }

    // start parsing model
    const sdf::ElementPtr rootElement = sdfElement->Root();
    if (!rootElement->HasElement("model"))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not a valid model SDF file!"), *URDFPath);
        return;
    }

    // Now we have a good model
    // Let's begin the initialization

    const sdf::ElementPtr modelElement = rootElement->GetElement("model");
    const std::string modelName = modelElement->Get<std::string>("name");
    std::cout <<  "Found " << modelName << " model!" << std::endl;

    // parse model links
    sdf::ElementPtr linkElement = modelElement->GetElement("link");
    int linkCount = 0;
    while (linkElement)
    {
        const std::string linkName = linkElement->Get<std::string>("name");
        std::cout << "Found " << linkName << " link in "
                  << modelName << " model!" << std::endl;

        auto linkPose = linkElement->Get<ignition::math::Pose3d>("pose");
        std::cout << "Pose: " << linkPose << std::endl;

        {
            auto rotString = URDFRotationToUnreal(linkPose.Rot()).ToString();
            auto locString = URDFPositionToUnreal(linkPose.Pos()).ToString();
            std::cout << "Unreal Pose:" << TCHAR_TO_UTF8(*rotString) << "," << TCHAR_TO_UTF8(*locString) << std::endl;
        }


        auto linkObject = CreateDefaultSubobject<USphereComponent>(FName(linkName.c_str()));
//        static ConstructorHelpers::FObjectFinder< UStaticMesh > meshFinder( TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere") );
//        linkObject->SetStaticMesh( meshFinder.Object );


        // Replace the Unreal RootComponent
        if(linkCount != 0) {
            linkObject->SetupAttachment(RootComponent);
        } else {
            RootComponent = linkObject;
        }

        linkObject->InitSphereRadius(0.1f);
        linkObject->SetMobility(EComponentMobility::Movable);
        linkObject->SetSimulatePhysics(true);
        linkObject->SetVisibility(false, false);

        if(linkCount != 0) {
            //linkObject->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
            linkObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            linkObject->SetCollisionResponseToAllChannels(ECR_Block);
        } else {
            linkObject->InitSphereRadius(.1f);
            linkObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            linkObject->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        }

        linkObject->RelativeLocation = URDFPositionToUnreal(linkPose.Pos());
        linkObject->RelativeRotation = URDFRotationToUnreal(linkPose.Rot());

        {
            sdf::ElementPtr visualElement = linkElement->GetElement("visual");
            while (visualElement) {

                std::cout << "Got visual " << visualElement->GetAttribute("name")->GetAsString() << " on it"
                          << std::endl;

                if (visualElement->HasElement("geometry")) {
                    sdf::ElementPtr geometryElement = visualElement->GetElement("geometry");
                    if (geometryElement->HasElement("mesh")) {
                        auto uri = geometryElement->GetElement("mesh")->Get<std::string>("uri");
                        std::cout << "Got mesh " << uri << " on it" << std::endl;

                        {
                            auto geometryName = FName(visualElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);

                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }
                            // geometryObject->SetupAttachment(linkObject);
                            geometryObject->ModelPath = GetRealPathFromURI(uri);
                            if (geometryElement->GetElement("mesh")->HasElement("scale")) {
                                auto meshScale = geometryElement->GetElement("mesh")->Get<ignition::math::Vector3d>("scale");
                                geometryObject->ModelScaleTransform *= FVector(meshScale.X(), meshScale.Y(), meshScale.Z());
                            }
                            geometryObject->CollisionPath = FString();
                            if (visualElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }
                        }
                    }
                    if (geometryElement->HasElement("cylinder")) {
                        auto cyl = geometryElement->GetElement("cylinder");
                        std::cout << "Got cylinder rad:" << cyl->Get<double>("radius") << " , len:"
                                  << cyl->Get<double>("length") << " on it" << std::endl;
                        {
                            auto geometryName = FName(visualElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                            
                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }
                            geometryObject->MeshType = ERobotMeshType::Cylinder;
                            geometryObject->CylinderSize = FVector2D(cyl->Get<double>("radius"),cyl->Get<double>("length"));
                            
                            if (visualElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }
                        }
                    }
                    if (geometryElement->HasElement("box")) {
                        auto box = geometryElement->GetElement("box");
                        std::cout << "Got box:" << box->Get<ignition::math::Vector3d>("size") << " on it" << std::endl;
                        {
                            auto geometryName = FName(visualElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                            
                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }
                            geometryObject->MeshType = ERobotMeshType::Box;
                            auto boxSize = box->Get<ignition::math::Vector3d>("size");
                            geometryObject->BoxSize = FVector(boxSize.X(), boxSize.Y(), boxSize.Z());
                            
                            if (visualElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }
                        }
                    }
                }
                visualElement = visualElement->GetNextElement("visual");
            }
        }

        {
            sdf::ElementPtr collisionElement = linkElement->GetElement("collision");
            while (collisionElement) {

                std::cout << "Got collision " << collisionElement->GetAttribute("name")->GetAsString() << " on it"
                          << std::endl;
                if (collisionElement->HasElement("geometry")) {
                    sdf::ElementPtr geometryElement = collisionElement->GetElement("geometry");

                    if (geometryElement->HasElement("mesh")) {
                        auto uri = geometryElement->GetElement("mesh")->Get<std::string>("uri");
                        std::cout << "Got collision mesh " << uri << " on it" << std::endl;

                        {
                            auto geometryName = FName(collisionElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            geometryObject->CollisionPath = GetRealPathFromURI(uri);
                            if (geometryElement->GetElement("mesh")->HasElement("scale")) {
                                auto meshScale = geometryElement->GetElement("mesh")->Get<ignition::math::Vector3d>("scale");
                                geometryObject->CollisionScaleTransform *= FVector(meshScale.X(), meshScale.Y(), meshScale.Z());
                            }
                            geometryObject->ModelPath = FString();
                            // geometryObject->SetupAttachment(linkObject);

                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                            geometryObject->WeldTo(linkObject);

                            UE_LOG(LogTemp, Warning, TEXT("Attaching collision to %s"), *(linkObject->GetName()));

                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }

                            if (collisionElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }

                            {
                                //set up the constraint instance with all the desired values
                                FConstraintInstance ConstraintInstance;

                                //set values here, see functions I am sharing with you below
                                ConstraintInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
                                ConstraintInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
                                ConstraintInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);

                                ConstraintInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
                                ConstraintInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
                                ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);

                                ConstraintInstance.SetLinearBreakable(false, 0);
                                ConstraintInstance.SetAngularBreakable(false, 0);

                                ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = 0;
                                ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = 0;
                                ConstraintInstance.SetDisableCollision(true);

                                auto constraintName = FName((collisionElement->GetAttribute("name")->GetAsString() + "_" + linkName).c_str());

                                UPhysicsConstraintComponent* ConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(constraintName);
                                if(!ConstraintComp)
                                {
                                    //UE_LOG constraint UObject could not be created!
                                    return;
                                }

                                ConstraintComp->ConstraintInstance = ConstraintInstance;

                                //Attach to Root!
                                ConstraintComp->SetupAttachment(RootComponent);

                                //~~~ Init Constraint ~~~
                                ConstraintComp->SetConstrainedComponents(linkObject, NAME_None, geometryObject, NAME_None);

                            }

                        }
                    }
                    if (geometryElement->HasElement("cylinder")) {
                        auto cyl = geometryElement->GetElement("cylinder");
                        std::cout << "Got cylinder rad:" << cyl->Get<double>("radius") << " , len:"
                        << cyl->Get<double>("length") << " on it" << std::endl;
                        {
                            auto geometryName = FName(collisionElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                            
                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }
                            geometryObject->MeshType = ERobotMeshType::Cylinder;
                            geometryObject->CylinderSize = FVector2D(cyl->Get<double>("radius"),cyl->Get<double>("length"));
                            
                            if (collisionElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }
                        }
                    }
                    if (geometryElement->HasElement("box")) {
                        auto box = geometryElement->GetElement("box");
                        std::cout << "Got box:" << box->Get<ignition::math::Vector3d>("size") << " on it" << std::endl;
                        {
                            auto geometryName = FName(collisionElement->GetAttribute("name")->GetAsString().c_str());
                            auto geometryObject = CreateDefaultSubobject<URobotMeshComponent>(geometryName);
                            bool attachSuccess = geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                            
                            if(!attachSuccess){
                                UE_LOG(LogTemp, Error, TEXT("ATTACHMENT FAILED for %s"), *(geometryName.ToString()));
                            }
                            
                            geometryObject->MeshType = ERobotMeshType::Box;
                            auto boxSize = box->Get<ignition::math::Vector3d>("size");
                            geometryObject->BoxSize = FVector(boxSize.X(), boxSize.Y(), boxSize.Z());
                            
                            if (collisionElement->HasElement("pose")) {
                                geometryObject->RelativeLocation = URDFPositionToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Pos());
                                geometryObject->RelativeRotation = URDFRotationToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Rot());
                            }
                        }
                    }
                }
                collisionElement = collisionElement->GetNextElement("collision");
            }
        }

        linkCount++;
        linkElement = linkElement->GetNextElement("link");
    }

    // parse model joints
    sdf::ElementPtr jointElement = modelElement->GetElement("joint");
    while (jointElement)
    {
        const std::string jointName = jointElement->Get<std::string>("name");
        std::cout << "Found " << jointName << " joint in "
                  << modelName << " model!" << std::endl;

        const sdf::ElementPtr parentElement = jointElement->GetElement("parent");
        const std::string parentLinkName = parentElement->Get<std::string>();

        const sdf::ElementPtr childElement = jointElement->GetElement("child");
        const std::string childLinkName = childElement->Get<std::string>();

        std::cout << "Joint " << jointName << " connects " << parentLinkName
                  << " link to " << childLinkName << " link" << std::endl;

        jointElement = jointElement->GetNextElement("joint");
    }
}

void ARobotActor::LoadModel(const FTransform &Transform){
    LStream Stream;
    std::cout.rdbuf(&Stream);

    std::string filename(TCHAR_TO_UTF8(*URDFPath));

    sdf::SDFPtr sdfElement(new sdf::SDF());
    sdf::init(sdfElement);
    if (!sdf::readFile(filename, sdfElement))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not a valid SDF file!"), *URDFPath);
        return;
    }

    // start parsing model
    const sdf::ElementPtr rootElement = sdfElement->Root();
    if (!rootElement->HasElement("model"))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is not a valid model SDF file!"), *URDFPath);
        return;
    }

    // Now we have a good model
    // Let's begin the initialization

    const sdf::ElementPtr modelElement = rootElement->GetElement("model");
    const std::string modelName = modelElement->Get<std::string>("name");
    std::cout <<  "Found " << modelName << " model!" << std::endl;

    // parse model links
    sdf::ElementPtr linkElement = modelElement->GetElement("link");
    int linkCount = 0;
    while (linkElement)
    {
        const std::string linkName = linkElement->Get<std::string>("name");
        std::cout << "Found " << linkName << " link in "
                  << modelName << " model!" << std::endl;

        auto linkPose = linkElement->Get<ignition::math::Pose3d>("pose");
        std::cout << "Pose: " << linkPose << std::endl;

        {
            auto rotString = URDFRotationToUnreal(linkPose.Rot()).ToString();
            auto locString = URDFPositionToUnreal(linkPose.Pos()).ToString();
            std::cout << "Unreal Pose:" << TCHAR_TO_UTF8(*rotString) << "," << TCHAR_TO_UTF8(*locString) << std::endl;
        }


        auto linkObject = NewObject<USphereComponent>(this, FName(linkName.c_str()));
        linkObject->RegisterComponent();

        if(linkCount != 0) {
            linkObject->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        } else {
            if(RootComponent){
                RootComponent->UnregisterComponent();
            }
            RootComponent = linkObject;
            RootComponent->SetWorldTransform(Transform);
        }

        linkObject->InitSphereRadius(1.f);
        linkObject->SetMobility(EComponentMobility::Movable);
        linkObject->SetSimulatePhysics(true);
        linkObject->SetVisibility(false, false);

        if(linkCount != 0) {
            linkObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            linkObject->SetCollisionResponseToAllChannels(ECR_Ignore);
            linkObject->SetRelativeLocation(URDFPositionToUnreal(linkPose.Pos()));
            linkObject->SetRelativeRotation(URDFRotationToUnreal(linkPose.Rot()));
        } else {
            linkObject->InitSphereRadius(.1f);
            linkObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            linkObject->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        }

        // End link process

        sdf::ElementPtr visualElement = linkElement->GetElement("visual");
        while(visualElement){

            std::cout << "Got visual " << visualElement->GetAttribute("name")->GetAsString() << " on it" << std::endl;

            if(visualElement->HasElement("geometry")){
                sdf::ElementPtr geometryElement = visualElement->GetElement("geometry");
                if(geometryElement->HasElement("mesh")){
                    auto uri = geometryElement->GetElement("mesh")->Get<std::string>("uri");
                    std::cout << "Got mesh " << uri << " on it" << std::endl;

                    {
                        auto geometryName = FName(visualElement->GetAttribute("name")->GetAsString().c_str());
                        auto geometryObject = NewObject<URobotMeshComponent>(this, geometryName);
                        geometryObject->ModelPath = GetRealPathFromURI(uri);
                        if(geometryElement->GetElement("mesh")->HasElement("scale")){
                            auto meshScale = geometryElement->GetElement("mesh")->Get<ignition::math::Vector3d>("scale");
                            geometryObject->ModelScaleTransform *= FVector(meshScale.X(), meshScale.Y(), meshScale.Z());
                        }
                        geometryObject->CollisionPath = FString();
                        geometryObject->AttachToComponent(linkObject, FAttachmentTransformRules::KeepRelativeTransform);
                        if(visualElement->HasElement("pose")){
                            geometryObject->SetRelativeLocation(URDFPositionToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Pos()));
                            geometryObject->SetRelativeRotation(URDFRotationToUnreal(visualElement->Get<ignition::math::Pose3d>("pose").Rot()));
                        }
                        geometryObject->RegisterComponent();
                    }
                }
                if(geometryElement->HasElement("cylinder")){
                    auto cyl = geometryElement->GetElement("cylinder");
                    std::cout << "Got cylinder rad:" << cyl->Get<double>("radius") << " , len:" << cyl->Get<double>("length") << " on it" << std::endl;
                }
                if(geometryElement->HasElement("box")){
                    auto box = geometryElement->GetElement("box");
                    std::cout << "Got box:" << box->Get<ignition::math::Vector3d>("size") << " on it" << std::endl;
                }
            }
            visualElement = visualElement->GetNextElement("visual");
        }


        sdf::ElementPtr collisionElement = linkElement->GetElement("collision");
        while(collisionElement){

            std::cout << "Got collision " << collisionElement->GetAttribute("name")->GetAsString() << " on it" << std::endl;
            if(collisionElement->HasElement("geometry")) {
                sdf::ElementPtr geometryElement = collisionElement->GetElement("geometry");

                if (geometryElement->HasElement("mesh")) {
                    auto uri = geometryElement->GetElement("mesh")->Get<std::string>("uri");
                    std::cout << "Got mesh " << uri << " on it" << std::endl;

                    {
                        auto geometryName = FName(collisionElement->GetAttribute("name")->GetAsString().c_str());
                        auto geometryObject = NewObject<URobotMeshComponent>(this, geometryName);
                        geometryObject->CollisionPath = GetRealPathFromURI(uri);
                        if (geometryElement->GetElement("mesh")->HasElement("scale")) {
                            auto meshScale = geometryElement->GetElement("mesh")->Get<ignition::math::Vector3d>("scale");
                            geometryObject->CollisionScaleTransform *= FVector(meshScale.X(), meshScale.Y(), meshScale.Z());
                        }
                        geometryObject->ModelPath = FString();

                        auto rule = FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false);

                        geometryObject->AttachToComponent(linkObject, rule);
                        geometryObject->RegisterComponent();

                        if (collisionElement->HasElement("pose")) {
                            geometryObject->SetRelativeLocation(URDFPositionToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Pos()));
                            geometryObject->SetRelativeRotation(URDFRotationToUnreal(collisionElement->Get<ignition::math::Pose3d>("pose").Rot()));
                        }

                        {
                            //set up the constraint instance with all the desired values
                            FConstraintInstance ConstraintInstance;

                            //set values here, see functions I am sharing with you below
                            ConstraintInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
                            ConstraintInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
                            ConstraintInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);

                            ConstraintInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
                            ConstraintInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
                            ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);

                            ConstraintInstance.SetLinearBreakable(false, 0);
                            ConstraintInstance.SetAngularBreakable(false, 0);

                            ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = 0;
                            ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = 0;
                            ConstraintInstance.SetDisableCollision(true);

                            auto constraintName = FName((collisionElement->GetAttribute("name")->GetAsString() + "_" + linkName).c_str());

                            UPhysicsConstraintComponent* ConstraintComp = NewObject<UPhysicsConstraintComponent>(this, constraintName);
                            if(!ConstraintComp)
                            {
                                //UE_LOG constraint UObject could not be created!
                                return;
                            }

                            ConstraintComp->ConstraintInstance = ConstraintInstance;

                            //Attach to Root!
                            ConstraintComp->SetupAttachment(linkObject);

                            //~~~ Init Constraint ~~~
                            ConstraintComp->SetConstrainedComponents(linkObject, NAME_None, geometryObject, NAME_None);

                        }
                    }
                }
                if (geometryElement->HasElement("cylinder")) {
                    auto cyl = geometryElement->GetElement("cylinder");
                    std::cout << "Got cylinder rad:" << cyl->Get<double>("radius") << " , len:"
                              << cyl->Get<double>("length") << " on it" << std::endl;
                }
                if (geometryElement->HasElement("box")) {
                    auto box = geometryElement->GetElement("box");
                    std::cout << "Got box:" << box->Get<ignition::math::Vector3d>("size") << " on it" << std::endl;
                }
            }
            collisionElement = collisionElement->GetNextElement("collision");
        }

        linkCount++;
        linkElement = linkElement->GetNextElement("link");
    }

    // parse model joints
    sdf::ElementPtr jointElement = modelElement->GetElement("joint");
    while (jointElement)
    {
        const std::string jointName = jointElement->Get<std::string>("name");
        std::cout << "Found " << jointName << " joint in "
                  << modelName << " model!" << std::endl;

        const sdf::ElementPtr parentElement = jointElement->GetElement("parent");
        const std::string parentLinkName = parentElement->Get<std::string>();

        const sdf::ElementPtr childElement = jointElement->GetElement("child");
        const std::string childLinkName = childElement->Get<std::string>();

        std::cout << "Joint " << jointName << " connects " << parentLinkName
                  << " link to " << childLinkName << " link" << std::endl;

        jointElement = jointElement->GetNextElement("joint");
    }
}

void ARobotActor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent){

    Super::PostEditChangeProperty(PropertyChangedEvent);

    if(PropertyChangedEvent.GetPropertyName() == FName("URDFPath")){
        {
            TInlineComponentArray<UActorComponent*> PrimComponents;
            this->GetComponents(PrimComponents);
            for(auto comp: PrimComponents){
                // if(comp==RootComponent) continue;

                comp->UnregisterComponent();
                comp->ConditionalBeginDestroy();
            }
        }
        LoadModel(GetTransform());
    }
}

void ARobotActor::OnConstruction(const FTransform &Transform){
    
    Super::OnConstruction(Transform);

//    {
//        TInlineComponentArray<UActorComponent*> PrimComponents;
//        this->GetComponents(PrimComponents);
//        for(auto comp: PrimComponents){
//            // if(comp==RootComponent) continue;
//
//            comp->UnregisterComponent();
//            comp->ConditionalBeginDestroy();
//        }
//    }
//
//    LoadModel(Transform);
}

// Called when the game starts or when spawned
void ARobotActor::BeginPlay()
{
	Super::BeginPlay();
    
}

// Called every frame
void ARobotActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

