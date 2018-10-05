// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealEventPort.h"

UnrealEventPort::UnrealEventPort()
{
}

UnrealEventPort::~UnrealEventPort()
{
}

bool UnrealEventPort::wait() {
    // If events are already pending, they will be processed and we return immediately thereafter
    // Otherwise, we block until new events arrive
    return false;
}

bool UnrealEventPort::poll() {
    // Process any pending events, but don't block
    return false;
}

void UnrealEventPort::setRunnable(bool runnable) {
    isRunnable = runnable;
}

void UnrealEventPort::run() {
    if (kjLoop)
        kjLoop->run();
}
