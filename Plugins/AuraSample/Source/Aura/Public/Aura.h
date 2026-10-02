// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// 自定义的碰撞通道
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1

#define PRINT_TO_SCREEN(Str) \
do \
{ \
    if (GEngine && !IsRunningCommandlet()) \
    { \
        static int32 DebugMsgKey = 0; \
        GEngine->AddOnScreenDebugMessage( \
            DebugMsgKey++,  /* 自动递增Key，避免覆盖之前的打印 */ \
            5.0f,           /* 显示时长：5秒（可根据需求修改） */ \
            FColor::Cyan,   /* 文字颜色：青色（醒目且不刺眼） */ \
            Str             /* 传入的字符串参数 */ \
        ); \
        /* 超过100个Key后重置，避免Key值过大 */ \
        if (DebugMsgKey >= 100) { DebugMsgKey = 0; } \
    } \
} while(0)