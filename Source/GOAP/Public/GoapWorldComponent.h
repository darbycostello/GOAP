
#pragma once

#include "CoreMinimal.h"
#include "GoapStructs.h"
#include "Engine/DataTable.h"
#include "GoapWorldComponent.generated.h"

/**
*  Stores the world state for a GOAP planner to search and modify
*/
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class GOAP_API UGoapWorldComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UGoapWorldComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UDataTable* WorldDataTable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGoapWorldState WorldState;
	
protected:
	virtual void BeginPlay() override;
};