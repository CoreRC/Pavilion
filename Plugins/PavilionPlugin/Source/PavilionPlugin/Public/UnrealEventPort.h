// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#include <kj/async.h>
#pragma clang diagnostic pop

/**
 * 
 */
class PAVILIONPLUGIN_API UnrealEventPort : public kj::EventPort
{
public:
	UnrealEventPort();
	~UnrealEventPort();
    void setLoop(kj::EventLoop* loop) {
        // Store a pointer to the KJ event loop to call run() on when it needs to process events. The UnrealEventPort does
        // not take ownership of kjLoop. Make sure to call setLoop(nullptr) or destroy the UnrealEventPort prior to
        // deallocating kjLoop.
        this->kjLoop = loop;
    }
    // EventPort API
    virtual bool wait();
    virtual bool poll();
    virtual void setRunnable(bool runnable);
    void run();
private:
    bool isRunnable = false;
    kj::EventLoop* kjLoop = nullptr;
    
};
