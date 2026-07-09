// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintCompilerExtension.h"

#include "TireflyParamMetaDataCompilerExtension.generated.h"


class UFunction;


/**
 * 蓝图编译期参数 MetaData 注入扩展。
 * 在蓝图编译完成（类布局已生成、字节码未生成）时，
 * 将 sidecar 中的参数 MetaData 注入到 UFunction 参数 FProperty 上。
 */
UCLASS()
class TIREFLYBLUEPRINTGRAPHUTILS_API UTireflyParamMetaDataCompilerExtension : public UBlueprintCompilerExtension
{
	GENERATED_BODY()


#pragma region Compilation

protected:
	/**
	 * 蓝图编译完成后回调。
	 * 此时 UFunction 和参数 FProperty 已创建并链接，但字节码尚未生成。
	 *
	 * @param CompilationContext 编译器上下文，包含 Blueprint、NewClass 等。
	 * @param Data 编译数据，包含中间图列表。
	 */
	virtual void ProcessBlueprintCompiled(
		const FKismetCompilerContext& CompilationContext,
		const FBlueprintCompiledData& Data) override;

#pragma endregion


#pragma region Injection

private:
	/**
	 * 将 sidecar 函数级 MetaData 注入到指定 UFunction 上。
	 *
	 * @param Function 目标 UFunction。
	 * @param FuncMeta sidecar 中该函数的 MetaData。
	 */
	static void InjectFunctionMetaData(UFunction* Function, const FTireflyFunctionParamMetaData& FuncMeta);

	/**
	 * 将 sidecar 参数 MetaData 注入到指定 UFunction 的参数 FProperty 上。
	 *
	 * @param Function 目标 UFunction。
	 * @param FuncMeta sidecar 中该函数的参数 MetaData。
	 */
	static void InjectParamMetaData(UFunction* Function, const struct FTireflyFunctionParamMetaData& FuncMeta);

#pragma endregion
};
