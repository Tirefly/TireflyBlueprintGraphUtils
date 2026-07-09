// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/BlueprintExtension.h"

#include "TireflyBlueprintParamMetaDataExtension.generated.h"


class UBlueprint;


// 单个函数参数的 MetaData 条目。
USTRUCT()
struct FTireflyParamMetaDataEntry
{
	GENERATED_BODY()

	// 参数名，对应 FUserPinInfo::PinName。
	UPROPERTY()
	FName ParamName = NAME_None;

	// 该参数的 MetaData Key/Value 集合。
	UPROPERTY()
	TMap<FName, FString> MetaData;
};


// 单个函数的参数 MetaData 集合。
USTRUCT()
struct FTireflyFunctionParamMetaData
{
	GENERATED_BODY()

	// 函数名（UFunction 名），作为 sidecar 查找的主键。
	UPROPERTY()
	FName FunctionName = NAME_None;

	// 函数级 MetaData（仅用于没有 FKismetUserDeclaredFunctionMetadata 的目标，如 Custom Event）。
	UPROPERTY()
	TMap<FName, FString> FunctionMetaData;

	// 该函数各参数的 MetaData 列表。
	UPROPERTY()
	TArray<FTireflyParamMetaDataEntry> ParamMetaData;
};


/**
 * 蓝图函数参数 MetaData 的 sidecar 持久化扩展。
 * 作为 UBlueprint 的子对象自动序列化到资产中。
 */
UCLASS()
class TIREFLYBLUEPRINTGRAPHUTILS_API UTireflyBlueprintParamMetaDataExtension : public UBlueprintExtension
{
	GENERATED_BODY()


#pragma region FunctionLevelCRUD

public:
	/**
	 * 获取或创建指定函数的参数 MetaData 条目。
	 *
	 * @param FunctionName 函数名（UFunction 名），作为主键。
	 * @return 指向函数条目的指针。
	 */
	FTireflyFunctionParamMetaData* FindOrAddFunction(FName FunctionName);

	/**
	 * 查找指定函数的参数 MetaData 条目（只读）。
	 *
	 * @param FunctionName 函数名。
	 * @return 指向函数条目的指针，未找到返回 nullptr。
	 */
	const FTireflyFunctionParamMetaData* FindFunction(FName FunctionName) const;

	/**
	 * 删除指定函数的参数 MetaData 条目。
	 *
	 * @param FunctionName 函数名。
	 */
	void RemoveFunction(FName FunctionName);

	/**
	 * 清理不存在于当前蓝图中的孤儿函数条目。
	 *
	 * @param ValidFunctionNames 当前蓝图中有效的函数名列表。
	 */
	void PurgeOrphanedFunctions(const TArray<FName>& ValidFunctionNames);

#pragma endregion


#pragma region ParamLevelCRUD

public:
	/**
	 * 获取或创建指定参数的 MetaData 条目。
	 *
	 * @param FunctionEntry 所属函数条目。
	 * @param ParamName 参数名。
	 * @return 指向参数条目的指针。
	 */
	FTireflyParamMetaDataEntry* FindOrAddParam(FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName);

	/**
	 * 删除指定参数的 MetaData 条目。
	 *
	 * @param FunctionEntry 所属函数条目。
	 * @param ParamName 参数名。
	 */
	void RemoveParam(FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName);

	/**
	 * 参数重命名时迁移 MetaData。
	 *
	 * @param FunctionName 所属函数名。
	 * @param OldParamName 旧参数名。
	 * @param NewParamName 新参数名。
	 */
	void RenameParam(FName FunctionName, FName OldParamName, FName NewParamName);

	/**
	 * 清理不存在于当前函数参数列表中的孤儿条目。
	 *
	 * @param FunctionName 所属函数名。
	 * @param ValidParamNames 当前函数中有效的参数名列表。
	 */
	void PurgeOrphanedParams(FName FunctionName, const TArray<FName>& ValidParamNames);

#pragma endregion


#pragma region Utility

public:
	/**
	 * 获取或创建蓝图的参数 MetaData sidecar extension。
	 * 如果蓝图上还没有该 extension，则创建并添加。
	 *
	 * @param Blueprint 目标蓝图。
	 * @return 指向 extension 的指针。
	 */
	static UTireflyBlueprintParamMetaDataExtension* GetOrCreate(UBlueprint* Blueprint);

	/**
	 * 查找蓝图上已有的参数 MetaData sidecar extension（只读，不创建）。
	 *
	 * @param Blueprint 目标蓝图。
	 * @return 指向 extension 的指针，不存在则返回 nullptr。
	 */
	static const UTireflyBlueprintParamMetaDataExtension* Find(const UBlueprint* Blueprint);

public:
	/**
	 * 获取所有函数参数 MetaData 的只读引用。
	 *
	 * @return 函数参数 MetaData 列表的只读引用。
	 */
	const TArray<FTireflyFunctionParamMetaData>& GetAllFunctionParamMetaData() const { return FunctionParamMetaData; }

#pragma endregion


#pragma region Data

private:
	// 所有函数的参数 MetaData 列表。
	UPROPERTY()
	TArray<FTireflyFunctionParamMetaData> FunctionParamMetaData;

#pragma endregion
};
