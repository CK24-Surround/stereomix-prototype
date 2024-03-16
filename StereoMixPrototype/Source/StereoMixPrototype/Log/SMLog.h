// 네트워크용 로그 매크로입니다.

#pragma once

DECLARE_LOG_CATEGORY_CLASS(LogSMNetwork, Log, All);

#define NET_LOG(CategoryName, Verbosity, Format, ...)\
{\
	const ENetMode Macro_CachedNetMode = GetNetMode();\
	const FString Macro_NetModeInfo = FString(Macro_CachedNetMode == NM_Standalone ? TEXT("Standalone") : Macro_CachedNetMode == NM_DedicatedServer ? TEXT("Server") : Macro_CachedNetMode == NM_ListenServer ? TEXT("ListenServer") : Macro_CachedNetMode == NM_Client ? FString::Printf(TEXT("Client%d"), static_cast<int32>(GPlayInEditorID)) : TEXT("Unknown"));\
	\
	const FString Macro_LocalRole = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole());\
	\
	const FString Macro_RemoteRole = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole());\
	\
	const FString Macro_FunctionInfo = FString(__FUNCTION__);\
	\
	UE_LOG(CategoryName, Verbosity, TEXT("[%s] (L: %s / R: %s)\t [%s]: %s"), *Macro_NetModeInfo, *Macro_LocalRole, *Macro_RemoteRole, *Macro_FunctionInfo, *FString::Printf(Format, ##__VA_ARGS__));\
}

#define NET_ANIM_LOG(CategoryName, Verbosity, Format, ...)\
{\
	const AActor* Macro_CachedOwningActor = GetOwningActor();\
	if (Macro_CachedOwningActor)\
	{\
		const ENetMode Macro_CachedNetMode = GetOwningActor()->GetNetMode();\
		const FString Macro_NetModeInfo = FString(Macro_CachedNetMode == NM_Standalone ? TEXT("Standalone") : Macro_CachedNetMode == NM_DedicatedServer ? TEXT("Server") : Macro_CachedNetMode == NM_ListenServer ? TEXT("ListenServer") : Macro_CachedNetMode == NM_Client ? FString::Printf(TEXT("Client%d"), static_cast<int32>(GPlayInEditorID)) : TEXT("Invalid"));\
		\
		const FString Macro_LocalRoleInfo = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Macro_CachedOwningActor->GetLocalRole());\
		\
		const FString Macro_RemoteRoleInfo = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Macro_CachedOwningActor->GetRemoteRole());\
		\
		const FString Macro_FunctionInfo = FString(__FUNCTION__);\
		\
		UE_LOG(CategoryName, Verbosity, TEXT("[%s] (L: %s / R: %s)\t [%s]: %s"), *Macro_NetModeInfo, *Macro_LocalRoleInfo, *Macro_RemoteRoleInfo, *Macro_FunctionInfo, *FString::Printf(Format, ##__VA_ARGS__));\
	}\
}

#define NET_COMP_LOG(CategoryName, Verbosity, Format, ...)\
{\
	const AActor* Macro_CachedOwningActor = GetOwner();\
	if (Macro_CachedOwningActor)\
	{\
		const ENetMode Macro_CachedNetMode = Macro_CachedOwningActor->GetNetMode();\
		const FString Macro_NetModeInfo = FString(Macro_CachedNetMode == NM_Standalone ? TEXT("Standalone") : Macro_CachedNetMode == NM_DedicatedServer ? TEXT("Server") : Macro_CachedNetMode == NM_ListenServer ? TEXT("ListenServer") : Macro_CachedNetMode == NM_Client ? FString::Printf(TEXT("Client%d"), static_cast<int32>(GPlayInEditorID)) : TEXT("Invalid"));\
		\
		const FString Macro_LocalRoleInfo = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Macro_CachedOwningActor->GetLocalRole());\
		\
		const FString Macro_RemoteRoleInfo = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Macro_CachedOwningActor->GetRemoteRole());\
		\
		const FString Macro_FunctionInfo = FString(__FUNCTION__);\
		\
		UE_LOG(CategoryName, Verbosity, TEXT("[%s] (L: %s / R: %s)\t [%s]: %s"), *Macro_NetModeInfo, *Macro_LocalRoleInfo, *Macro_RemoteRoleInfo, *Macro_FunctionInfo, *FString::Printf(Format, ##__VA_ARGS__));\
	}\
}
