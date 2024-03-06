// 네트워크용 로그 매크로입니다.

#pragma once

DECLARE_LOG_CATEGORY_CLASS(LogSMNetwork, Log, All);

#define LocalRoleInfo UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole())
#define RemoteRoleInfo UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole())
#define CallFunctionInfo FString(__FUNCTION__)

#define NET_LOG(LogCategory, Verbosity, Format, ...)\
{\
ENetMode Macro_CachedNetMode = GetNetMode();\
FString Macro_NetModeInfo = \
FString(Macro_CachedNetMode == NM_Standalone ? TEXT("Standalone") : \
Macro_CachedNetMode == NM_DedicatedServer ? TEXT("DedicatedServer") : \
Macro_CachedNetMode == NM_ListenServer ? TEXT("ListenServer") : \
Macro_CachedNetMode == NM_Client ? FString::Printf(TEXT("Client%d"), static_cast<int32>(GPlayInEditorID)) : TEXT("Unknown"));\
UE_LOG(LogCategory, Verbosity, TEXT("[%s] [Local: %s / Remote: %s] [%s]: %s"), *Macro_NetModeInfo, *LocalRoleInfo, *RemoteRoleInfo, *CallFunctionInfo, *FString::Printf(Format, ##__VA_ARGS__))\
}
