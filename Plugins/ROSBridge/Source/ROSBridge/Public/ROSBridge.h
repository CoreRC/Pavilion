// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

#pragma push_macro("check")
#undef check
// include your boost headers
#include "ros/ros.h"
#include "std_msgs/String.h"
#pragma pop_macro("check")

class FROSBridgeModule : public IModuleInterface
{
public:    
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static inline FROSBridgeModule& Get()
    {
        return FModuleManager::LoadModuleChecked< FROSBridgeModule >("ROSBridge");
    }
    
    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("ROSBridge");
    }
    
    void CreateNodeHandle();
    void DestroyNodeHandle();
    void spinOnce();
    
    void publishString(std::string str);
private:
	/** Handle to the test dll we will load */
    void*	ROSCppLibraryHandle;
    void*   ROSLibLibraryHandle;
    //ros::NodeHandle* nh;
    ros::Publisher   publisher;
    
    std::map<std::string,void*> callbacks;
};

class FROSSubscriber
{
public:
    template<class M, class T> FROSSubscriber(std::string& node_name, std::string& topic, uint32_t queue_size, void(T::*fp)(M), T* obj);
    ~FROSSubscriber();
    
private:
    ros::Subscriber sub;
};
