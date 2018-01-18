// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ROSBridge.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"
#pragma push_macro("check")
#undef check
// include your boost headers
#include "ros/ros.h"
#include "std_msgs/String.h"
#pragma pop_macro("check")
#include <sstream>

#define LOCTEXT_NAMESPACE "FROSBridgeModule"

void FROSBridgeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("ROSBridge")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_MAC
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/roscpp/lib/libroscpp.dylib"));
#endif

	ROSCppLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/roscpp/lib/libroslib.dylib"));
    
    ROSLibLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

    
    std::map<std::string,std::string> args;
    args.insert(std::make_pair("__master","http://localhost:11311/"));
    
	if (ROSCppLibraryHandle)
	{
		// Call the test function in the third party library that opens a message box
		//ExampleLibraryFunction();
        UE_LOG(LogTemp, Warning, TEXT("libroscpp Loaded"));
        
        ros::init(args, "surrealsim", ros::init_options::NoSigintHandler);
        
        
        //ros::spinOnce();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}
}

void FROSBridgeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
    if(!(ROSCppLibraryHandle==nullptr)){
        
        ros::shutdown();
        
        FPlatformProcess::FreeDllHandle(ROSCppLibraryHandle);
    }
    ROSCppLibraryHandle = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("libroscpp Unloaded"));
    
}

void FROSBridgeModule::CreateNodeHandle(){
    ros::NodeHandle nh("unreal");
    publisher = nh.advertise<std_msgs::String>("test_string", 5);
}

void FROSBridgeModule::DestroyNodeHandle(){
    UE_LOG(LogTemp, Warning, TEXT("DestroyNodeHandle Called"));
    
    publisher.shutdown();
    
    UE_LOG(LogTemp, Warning, TEXT("DestroyNodeHandle Finished"));
    
}

void FROSBridgeModule::spinOnce(){
    ros::spinOnce();
}

void FROSBridgeModule::publishString(std::string str){
    
    std_msgs::String str_msg;
    str_msg.data = str;
    //static_cast<ros::Publisher*>(publisher)->publish(str_msg);
    publisher.publish(str_msg);
}

template<class M, class T> FROSSubscriber::FROSSubscriber(std::string& node_name, std::string& topic, uint32_t queue_size, void(T::*fp)(M), T* obj){
    ros::NodeHandle nh(node_name);
    sub = nh.subscribe(topic, queue_size, fp, obj);
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FROSBridgeModule, ROSBridge)
