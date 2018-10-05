// Copyright 2018 Fan Jiang <i@fanjiang.me> All Rights Reserved. This source file is subject to multiple Chinese and International Copyright Laws and Patents. Unauthorized Redistribution is strictly prohibited.

#include "RobotMeshComponent.h"
#include "RuntimeMeshShapeGenerator.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RenderUtils.h"

#include "ThirdParty/VHACD/public/VHACD.h"

URobotMeshComponent::URobotMeshComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    //GetBodySetup()->bGenerateMirroredCollision = true;
    static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/PavilionPlugin/BasicRobotPartMaterial.BasicRobotPartMaterial"));
    if (FoundMaterial.Succeeded())
    {
        BaseMaterial = FoundMaterial.Object;
    }

    this->SetMobility(EComponentMobility::Movable);

}

void URobotMeshComponent::InitializeComponent()
{
    Super::InitializeComponent();
    // your code
    
}

void URobotMeshComponent::OnRegister()
{
    // your code
    Super::OnRegister();
    
    Super::SetCollisionUseComplexAsSimple(false);
    
    clear();
    int32 secCount;
    FString errCode;

    Super::ClearAllMeshSections();
    Super::ClearAllMeshCollisionSections();
    
    switch (MeshType) {
        case ERobotMeshType::Mesh:
            if(ComponentType != ERobotComponentType::Collision && !ModelPath.IsEmpty() && openMesh(ModelPath, secCount, errCode)){
                Super::CreateMeshSection(0, _vertices[0], _indices[0], _normals[0], _uvs[0], _vertexColors[0], _tangents[0], false, EUpdateFrequency::Average, ESectionUpdateFlags::None, false, false);
                
                //Super::ContainsPhysicsTriMeshData(true);
                
                if(_materials.Num() > 0){
                    UE_LOG(LogTemp, Warning, TEXT("Material Count %d"), _materials.Num());
                    Super::SetMaterial(0, _materials[0]);
                }
            } else {
                FRuntimeMeshDataPtr Data = GetOrCreateRuntimeMesh()->GetRuntimeMeshData();
                Data->EnterSerializedMode();
                
                Data->CreateMeshSection(0, false, false, 1, false, false, EUpdateFrequency::Average);
                
                auto Section = Data->BeginSectionUpdate(0);
                
                URuntimeMeshShapeGenerator::CreateBoxMesh(FVector(0.1,0.1,0.1), *Section.Get());
                
                Section->Commit();
                UE_LOG(LogTemp, Warning, TEXT("Cannot read visual model: %s"), *errCode);
            }
            
            this->SetSimulatePhysics(true);
            this->SetCollisionObjectType(ECC_PhysicsBody);
            this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            
            if(ComponentType != ERobotComponentType::Visual && openCollisionMesh(CollisionPath, errCode)){
                
                auto name = this->GetName();
                UE_LOG(LogTemp, Warning, TEXT("Collision loaded: %s"), *name);
            } else {
                this->SetSimulatePhysics(false);
                this->SetCollisionObjectType(ECC_PhysicsBody);
                this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            }
            break;
        case ERobotMeshType::Cylinder:
            if(ComponentType != ERobotComponentType::Collision) {
                FRuntimeMeshDataPtr Data = GetOrCreateRuntimeMesh()->GetRuntimeMeshData();
                Data->EnterSerializedMode();
                
                Data->CreateMeshSection(0, false, false, 1, false, false, EUpdateFrequency::Average);
                
                auto Section = Data->BeginSectionUpdate(0);
                
                URuntimeMeshShapeGenerator::CreateBoxMesh(FVector(100 * CylinderSize.X, 100 * CylinderSize.X, 100 * CylinderSize.Y), *Section.Get());
                
                Section->Commit();
                UE_LOG(LogTemp, Warning, TEXT("Cannot read visual model: %s"), *errCode);
            }
            
            this->SetSimulatePhysics(true);
            this->SetCollisionObjectType(ECC_PhysicsBody);
            this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            
            if(ComponentType != ERobotComponentType::Visual){
                
                auto name = this->GetName();
                UE_LOG(LogTemp, Warning, TEXT("Collision loaded: %s"), *name);
            } else {
                this->SetSimulatePhysics(false);
                this->SetCollisionObjectType(ECC_PhysicsBody);
                this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            }
            break;
        case ERobotMeshType::Box:
            if(ComponentType != ERobotComponentType::Collision) {
                FRuntimeMeshDataPtr Data = GetOrCreateRuntimeMesh()->GetRuntimeMeshData();
                Data->EnterSerializedMode();
                
                Data->CreateMeshSection(0, false, false, 1, false, false, EUpdateFrequency::Average);
                
                auto Section = Data->BeginSectionUpdate(0);
                
                URuntimeMeshShapeGenerator::CreateBoxMesh(BoxSize, *Section.Get());
                
                Section->Commit();
                UE_LOG(LogTemp, Warning, TEXT("Cannot read visual model: %s"), *errCode);
            }
            
            this->SetSimulatePhysics(true);
            this->SetCollisionObjectType(ECC_PhysicsBody);
            this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            
            if(ComponentType != ERobotComponentType::Visual){
                
                auto name = this->GetName();
                UE_LOG(LogTemp, Warning, TEXT("Collision loaded: %s"), *name);
            } else {
                this->SetSimulatePhysics(false);
                this->SetCollisionObjectType(ECC_PhysicsBody);
                this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                this->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            }
            break;
    }
    
}

void URobotMeshComponent::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent){
    
    
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool URobotMeshComponent::openCollisionMesh(FString path, FString& ErrorCode){
    Assimp::Importer importer;
    std::string filename(TCHAR_TO_UTF8(*path));
    const aiScene* scene;
    
    if(MakeLeftHanded) {
        scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality |
                                             aiProcess_FlipUVs |
                                             aiProcess_MakeLeftHanded);
    } else {
        scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality |
                                  aiProcess_FlipUVs);
    }
    
    if (!scene)
    {
        ErrorCode = importer.GetErrorString();
        UE_LOG(LogTemp, Warning, TEXT("Collision Load Error: %s"), *ErrorCode);
        return false;
    }
    
    Super::ClearAllConvexCollisionSections();
    
    FTransform transV;
    transV.SetScale3D(CollisionScaleTransform);
    
    UE_LOG(LogTemp, Warning, TEXT("Number of Collision Meshes: %u"), scene->mNumMeshes);
    for (uint32 i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        TArray<FVector> vertices;
        TArray<uint32> indices;
        
        vertices.Empty();
        vertices.Reserve(mesh->mNumVertices);
        
        indices.Empty();
        indices.Reserve(mesh->mNumFaces);
        
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            FVector vertex, normal;
            // process vertex positions, normals and UVs
            vertex.X = mesh->mVertices[j].x;
            vertex.Y = mesh->mVertices[j].y;
            vertex.Z = mesh->mVertices[j].z;
            
            if(ApplyTransformation){
                vertex = transV.TransformVector(vertex);
            }
            vertices.Add(vertex);
        }
        
        for (uint32 j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            if(ApplyTransformation){
                indices.Add(face.mIndices[0]);
                indices.Add(face.mIndices[1]);
                indices.Add(face.mIndices[2]);
            } else {
                indices.Add(face.mIndices[2]);
                indices.Add(face.mIndices[1]);
                indices.Add(face.mIndices[0]);
            }
        }
        // Super::AddConvexCollisionSection(vertices);
        DecomposeMeshToHulls(_convexCollision, vertices, indices, 0.001, 64);
        UE_LOG(LogTemp, Warning, TEXT("Decomposing Mesh to Hulls: %d to %d"), mesh->mNumVertices, _convexCollision.Num());
        for(auto c: _convexCollision){
            UE_LOG(LogTemp, Warning, TEXT("Hull Added: %d"), c.Num());
            //Super::CreateMeshSection(2, _vertices[0], _indices[0], _normals[0], _uvs[0], _vertexColors[0], _tangents[0], false, EUpdateFrequency::Average, ESectionUpdateFlags::None, false, false);
            Super::AddConvexCollisionSection(c);
        }
//        for(auto c: _convexCollision){
//            UE_LOG(LogTemp, Warning, TEXT("Hull Added"));
//
//            Super::AddConvexCollisionSection(c);
//            FRuntimeMeshDataPtr Data = GetOrCreateRuntimeMesh()->GetRuntimeMeshData();
//            Data->EnterSerializedMode();
//            Data->SetCollisionConvexMeshes(_convexCollision);
//            //Data->CreateMeshSection(0, false, false, 1, false, true, EUpdateFrequency::Average);
//
//            //auto Section = Data->BeginSectionUpdate(0);
//
//            URuntimeMeshShapeGenerator::CreateBoxMesh(FVector(0.1,0.1,0.1), *Section.Get());
//
//            Section->Commit();
//        }
    }
    UE_LOG(LogTemp, Warning, TEXT("CollisionMesh Loaded"));
    return true;
}

void URobotMeshComponent::OnCreatePhysicsState() {
//    FRuntimeMeshDataPtr Data = GetOrCreateRuntimeMesh()->GetRuntimeMeshData();
//    Data->SetCollisionConvexMeshes(_convexCollision);
    
    Super::OnCreatePhysicsState();
}

bool URobotMeshComponent::openMesh(FString path, int32& SectionCount, FString& ErrorCode)
{
    Assimp::Importer importer;
    std::string filename(TCHAR_TO_UTF8(*path));
    const aiScene* scene;
    
    if(MakeLeftHanded) {
        scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality |
                                  aiProcess_FlipUVs |
                                  aiProcess_MakeLeftHanded |
                                  aiProcess_GlobalScale);
    } else {
        scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality |
                        aiProcess_FlipUVs | aiProcess_GlobalScale
        );
    }

    if (!scene)
    {
        ErrorCode = importer.GetErrorString();
        return false;
    }
    _meshCurrentlyProcessed = 0;
    
//    if(ApplyTransformation){
//        UE_LOG(LogTemp, Warning, TEXT("Trying to apply transform"));
//        aiMatrix4x4 scalingMat;
//        aiVector3D scale(10.f, -10.f, 10.f); // Assimp -> Unreal
//        aiMatrix4x4::Scaling(scale, scalingMat);
//        if(scene->mRootNode->mChildren[0]){
//            aiMatrix4x4 rootMat(scene->mRootNode->mChildren[0]->mTransformation);
//
//            scene->mRootNode->mChildren[0]->mTransformation = scalingMat * rootMat;
//        }
//    }
    
    processNode(scene->mRootNode, scene);
    SectionCount = _meshCurrentlyProcessed;
    
    if (scene->HasMaterials())
    {
        for (unsigned int ii = 0; ii < scene->mNumMaterials; ++ii)
        {
//            if(_materials.Num() < scene->mNumMaterials){
//                _materials.AddZeroed();
//                _textures.AddZeroed();
//            }
            
            processMaterial(scene->mMaterials[ii], scene);
        }
    }
    
    return true;
}

bool URobotMeshComponent::getSection(int32 index, TArray<FVector>& Vertices, TArray<int32>& Faces, TArray<FVector>& Normals, TArray<FVector2D>& UV, TArray<FRuntimeMeshTangent>& Tangents)
{
    if (index>=_meshCurrentlyProcessed)
    {
        return false;
    }
    Vertices = _vertices[index];
    Faces = _indices[index];
    Normals = _normals[index];
    UV = _uvs[index];
    Tangents = _tangents[index];
    return true;
}

void URobotMeshComponent::clear()
{
    _vertices.Empty();
    _indices.Empty();
    _normals.Empty();
    _uvs.Empty();
    _tangents.Empty();
    _vertexColors.Empty();
    _materials.Empty();
    _textures.Empty();
    _tnormals.Empty();
    _meshCurrentlyProcessed = 0;
}

void URobotMeshComponent::processNode(aiNode* node, const aiScene* scene)
{
    for (uint32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, node, scene);
        ++_meshCurrentlyProcessed;
        UE_LOG(LogTemp, Warning, TEXT("processNode got mesh %d on %d"), i, node->mName.length);
    }
    uint32 nodes = node->mNumMeshes;
    // do the same for all of its children
    for (uint32 i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void URobotMeshComponent::processMesh(aiMesh* mesh, const aiNode* node, const aiScene* scene)
{
    
    // the very first time this method runs, we'll need to create the empty arrays
    // we can't really do that in the class constructor because we don't know how many meshes we'll read, and this data can change between imports
    if (_vertices.Num() <= _meshCurrentlyProcessed) {
        _vertices.AddZeroed();
        _normals.AddZeroed();
        _uvs.AddZeroed();
        _tangents.AddZeroed();
        _vertexColors.AddZeroed();
        _indices.AddZeroed();
    }
    
    // we check whether the current data to read has a different amount of vertices compared to the last time we generated the mesh
    // if so, it means we'll need to recreate the mesh and resupply new indices.
    if (mesh->mNumVertices != _vertices[_meshCurrentlyProcessed].Num())
        _requiresFullRecreation = true;
    
    // we reinitialize the arrays for the new data we're reading
    _vertices[_meshCurrentlyProcessed].Empty();
    _normals[_meshCurrentlyProcessed].Empty();
    _uvs[_meshCurrentlyProcessed].Empty();
    // this if actually seems useless, seeing what it does without it
    //if (_requiresFullRecreation) {
    _tangents[_meshCurrentlyProcessed].Empty();
    _vertexColors[_meshCurrentlyProcessed].Empty();
    _indices[_meshCurrentlyProcessed].Empty();
    //}

    FTransform transV;
    FTransform transN;
    
    transV.SetScale3D(ModelScaleTransform); // Assimp -> Unreal
    transN.SetScale3D(NormalTransform);

    // HACK!
    aiMatrix4x4 nodeTransform = node->mTransformation;

    UE_LOG(LogTemp, Warning, TEXT("NODE SCALE: %f"), nodeTransform.Determinant());

    if(abs(nodeTransform.Determinant()-1.0)<0.001){
        // TODO: Investigate the real cause, now a hack
        transV.SetScale3D(10.0 * transV.GetScale3D());
        transN.SetScale3D(10.0 * transN.GetScale3D());
    }

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        FVector vertex, normal;
        // process vertex positions, normals and UVs
        vertex.X = mesh->mVertices[i].x;
        vertex.Y = mesh->mVertices[i].y;
        vertex.Z = mesh->mVertices[i].z;
        
        
        normal.X = mesh->mNormals[i].x;
        normal.Y = mesh->mNormals[i].y;
        normal.Z = mesh->mNormals[i].z;
        
        if(ApplyTransformation){
            //UE_LOG(LogTemp, Warning, TEXT("Trying to apply transform on mesh"));
            vertex = transV.TransformVector(vertex);
            normal = transN.TransformVector(normal);
            //trans.InverseTr
        }
        
        // if the mesh contains tex coords
        if (mesh->mTextureCoords[0]) {
            FVector2D uvs;
            uvs.X = mesh->mTextureCoords[0][i].x;
            uvs.Y = mesh->mTextureCoords[0][i].y;
            _uvs[_meshCurrentlyProcessed].Add(uvs);
        }
        else {
            _uvs[_meshCurrentlyProcessed].Add(FVector2D(0.f, 0.f));
        }
        _vertices[_meshCurrentlyProcessed].Add(vertex);
        _normals[_meshCurrentlyProcessed].Add(normal);
    }
    
    _requiresFullRecreation = true;
    
    if (_requiresFullRecreation) {
        // process indices
        for (uint32 i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            if(ApplyTransformation){
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[0]);
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[1]);
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[2]);
            } else {
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[2]);
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[1]);
                _indices[_meshCurrentlyProcessed].Add(face.mIndices[0]);
            }
        }
    }
}

void URobotMeshComponent::processMaterial(aiMaterial* material, const aiScene* scene){
    if(BaseMaterial == nullptr) return;
    
    UMaterialInstanceDynamic* unrealMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    aiString diffuse_map, normal_map;
    UE_LOG(LogTemp, Warning, TEXT("Loading Material"));
    if((material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_map))==aiReturn_SUCCESS){
        FString TextureName = diffuse_map.data;
        FString LongTextureName = FPaths::ConvertRelativePathToFull(FPaths::GetPath(ModelPath), TextureName);
        UE_LOG(LogTemp, Warning, TEXT("Located Texture at %s"), *TextureName);
        UE_LOG(LogTemp, Warning, TEXT("Raw Path at %s"), *LongTextureName);
        
        UTexture* unrealTexture = LoadImageFromDisk(this, LongTextureName);
        if(unrealTexture!=nullptr) {
            _textures.Add(unrealTexture);
            UE_LOG(LogTemp, Warning, TEXT("Setting Texture BaseColorParam"));
            unrealMaterial->SetTextureParameterValue("BaseColorParam", unrealTexture);
        } else {
            UE_LOG(LogTemp, Error, TEXT("LoadImageFromDisk failed"));
        }
        
        if((material->GetTexture(aiTextureType_NORMALS, 0, &normal_map))==aiReturn_SUCCESS){
            FString NormalName = normal_map.data;
            UTexture* unrealNormal = LoadImageFromDisk(this, FPaths::ConvertRelativePathToFull(FPaths::GetPath(ModelPath), NormalName));
            if(unrealNormal!=nullptr) {
                _tnormals.Add(unrealNormal);
                UE_LOG(LogTemp, Warning, TEXT("Setting NormalParam"));
                unrealMaterial->SetTextureParameterValue("NormalParam", unrealNormal);
            } else {
                UE_LOG(LogTemp, Error, TEXT("LoadImageFromDisk failed"));
            }
        }
        
        _materials.Add(unrealMaterial);
    } else {
        _materials.Add(unrealMaterial);
    }
}

UTexture2D* URobotMeshComponent::LoadTextureFromPath(const FString& Path)
{
    if (Path.IsEmpty()) return NULL;
    
    return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(Path)));
}

UTexture2D* URobotMeshComponent::LoadImageFromDisk(UObject* Outer, const FString& ImagePath)
{
    // Check if the file exists first
    if (!FPaths::FileExists(ImagePath))
    {
        UE_LOG(LogTemp, Error, TEXT("File not found: %s"), *ImagePath);
        return nullptr;
    }
    
    // Load the compressed byte data from the file
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *ImagePath);
        return nullptr;
    }
    
    // Detect the image type using the ImageWrapper module
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    
    EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
    if (ImageFormat == EImageFormat::Invalid)
    {
        UE_LOG(LogTemp, Error, TEXT("Unrecognized image file format: %s"), *ImagePath);
        return nullptr;
    }
    
    // Create an image wrapper for the detected image format
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
    if (!ImageWrapper.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create image wrapper for file: %s"), *ImagePath);
        return nullptr;
    }
    
    // Decompress the image data
    const TArray<uint8>* RawData = nullptr;
    ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num());
    ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData);
    if (RawData == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decompress image file: %s"), *ImagePath);
        return nullptr;
    }
    
    // Create the texture and upload the uncompressed image data
    FString TextureBaseName = TEXT("Texture_") + FPaths::GetBaseFilename(ImagePath);
    return CreateTexture(Outer, *RawData, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), EPixelFormat::PF_B8G8R8A8, FName(*TextureBaseName));
}

UTexture2D* URobotMeshComponent::CreateTexture(UObject* Outer, const TArray<uint8>& PixelData, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat, FName BaseName)
{
    // Shamelessly copied from UTexture2D::CreateTransient with a few modifications
    if (InSizeX <= 0 || InSizeY <= 0 ||
        (InSizeX % GPixelFormats[InFormat].BlockSizeX) != 0 ||
        (InSizeY % GPixelFormats[InFormat].BlockSizeY) != 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid parameters specified for UImageLoader::CreateTexture()"));
        return nullptr;
    }
    
    // Most important difference with UTexture2D::CreateTransient: we provide the new texture with a name and an owner
    FName TextureName = MakeUniqueObjectName(Outer, UTexture2D::StaticClass(), BaseName);
    UTexture2D* NewTexture = NewObject<UTexture2D>(Outer, TextureName, RF_Transient);
    
    NewTexture->PlatformData = new FTexturePlatformData();
    NewTexture->PlatformData->SizeX = InSizeX;
    NewTexture->PlatformData->SizeY = InSizeY;
    NewTexture->PlatformData->PixelFormat = InFormat;
    
    // Allocate first mipmap and upload the pixel data
    int32 NumBlocksX = InSizeX / GPixelFormats[InFormat].BlockSizeX;
    int32 NumBlocksY = InSizeY / GPixelFormats[InFormat].BlockSizeY;
    FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
    Mip->SizeX = InSizeX;
    Mip->SizeY = InSizeY;
    Mip->BulkData.Lock(LOCK_READ_WRITE);
    void* TextureData = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
    FMemory::Memcpy(TextureData, PixelData.GetData(), PixelData.Num());
    Mip->BulkData.Unlock();
    
    NewTexture->UpdateResource();
    return NewTexture;
}

using namespace VHACD;

class FMyVHACDProgressCallback : public IVHACD::IUserCallback
{
public:
    FMyVHACDProgressCallback(void) {}
    ~FMyVHACDProgressCallback() {};
    
    void Update(const double overallProgress, const double stageProgress, const double operationProgress, const char * const stage,    const char * const    operation)
    {
        //FString StatusString = FString::Printf(TEXT("Processing [%s]..."), ANSI_TO_TCHAR(stage));
        
        //GWarn->StatusUpdate(stageProgress*10.f, 1000, FText::FromString(StatusString));
        //GWarn->StatusUpdate(overallProgress*10.f, 1000, FText::FromString(StatusString));
    };
};

void URobotMeshComponent::DecomposeMeshToHulls(TArray< TArray<FVector> >& ConvexMeshes, const TArray<FVector>& InVertices, const TArray<uint32>& InIndices, float InAccuracy, int32 InMaxHullVerts)
{
    
    bool bSuccess = false;
    
    // Validate input by checking bounding box
    FBox VertBox(ForceInit);
    for (FVector Vert : InVertices)
    {
        VertBox += Vert;
    }
    
    // If box is invalid, or the largest dimension is less than 1 unit, or smallest is less than 0.1, skip trying to generate collision (V-HACD often crashes...)
    if (VertBox.IsValid == 0 || VertBox.GetSize().GetMax() < 0.1f || VertBox.GetSize().GetMin() < 0.1f)
    {
        UE_LOG(LogTemp, Error, TEXT("Object too small for V-HACD: %f, %f"), VertBox.GetSize().GetMax(), VertBox.GetSize().GetMin());
        return;
    }
    
    FMyVHACDProgressCallback VHACD_Callback;
    
    IVHACD::Parameters VHACD_Params;
    VHACD_Params.m_resolution = 100000; // Maximum number of voxels generated during the voxelization stage (default=100,000, range=10,000-16,000,000)
    VHACD_Params.m_maxNumVerticesPerCH = InMaxHullVerts; // Controls the maximum number of triangles per convex-hull (default=64, range=4-1024)
    VHACD_Params.m_concavity = 0.3f * (1.f - FMath::Clamp(InAccuracy, 0.f, 1.f)); // Maximum allowed concavity (default=0.0025, range=0.0-1.0)
    VHACD_Params.m_callback = &VHACD_Callback;
    VHACD_Params.m_oclAcceleration = true;
    VHACD_Params.m_minVolumePerCH = 0.003f; // this should be around 1 / (3 * m_resolution ^ (1/3))
    
    
    IVHACD* InterfaceVHACD = CreateVHACD();
    
    const float* const Verts = (float*)InVertices.GetData();
    const unsigned int NumVerts = InVertices.Num();
    const uint32_t* const Tris = (uint32_t*)InIndices.GetData();
    const unsigned int NumTris = InIndices.Num() / 3;
    
    bSuccess = InterfaceVHACD->Compute(Verts, NumVerts, Tris, NumTris, VHACD_Params);
    
    if(bSuccess)
    {
        // Clean out old hulls
        ConvexMeshes.Empty();
        
        // Iterate over each result hull
        int32 NumHulls = InterfaceVHACD->GetNConvexHulls();
        
        ConvexMeshes.AddDefaulted(NumHulls);
        for(int32 HullIdx=0; HullIdx<NumHulls; HullIdx++)
        {
            IVHACD::ConvexHull Hull;
            InterfaceVHACD->GetConvexHull(HullIdx, Hull);
            
            
            ConvexMeshes[HullIdx].Reserve(Hull.m_nPoints);
            
            for (uint32 VertIdx = 0; VertIdx < Hull.m_nPoints; VertIdx++)
            {
                FVector V;
                V.X = (float)(Hull.m_points[(VertIdx * 3) + 0]);
                V.Y = (float)(Hull.m_points[(VertIdx * 3) + 1]);
                V.Z = (float)(Hull.m_points[(VertIdx * 3) + 2]);
                
                ConvexMeshes[HullIdx].Add(V);
            }
        }
    }
    
    
    InterfaceVHACD->Clean();
    InterfaceVHACD->Release();
}
