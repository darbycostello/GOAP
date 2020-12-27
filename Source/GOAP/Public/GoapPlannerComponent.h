
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoapStructs.h"
#include "GoapWorldComponent.h"
#include "GoapPlannerComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE(FGoapPlanTaskCompleteDynamicDelegate);

/**
* Builds a plan of available actions to transform a given world state into a goal state, using IDA*
*/
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class GOAP_API UGoapPlannerComponent : public UActorComponent {
	GENERATED_BODY()

public:	
	UGoapPlannerComponent();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnableLogging;

	// A data table from which to load all actions available to this planner
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UDataTable* ActionsDataTable;

	// Set the component containing the world state
	UFUNCTION(BlueprintCallable, Category="GOAP")
	void SetGoapWorldComponent(UGoapWorldComponent* GoapWorldComponent);

	// Get the component containing the world state
	UFUNCTION(BlueprintCallable, Category="GOAP")
    UGoapWorldComponent* GetGoapWorldComponent() const;

	// Get the last plan created by this component
	UFUNCTION(BlueprintCallable, Category="GOAP")
    void GetPlan(TArray<FGoapNode> &Plan) const;

	// Finds a list of actions to transform the supplied world state into the supplied goal state. Runs as a background task. 
	UFUNCTION(BlueprintCallable, Category="GOAP")
	void Plan(FGoapWorldState WorldState, FGoapWorldState GoalState, FGoapPlanTaskCompleteDynamicDelegate OnComplete);
	void ExecutePlanner(FGoapWorldState WorldState, FGoapWorldState GoalState, FGoapPlanSharedPtr* Plan);

	// Returns a world state modified by the plan, including all effects.
	UFUNCTION(BlueprintCallable, Category="GOAP")
	static void SimulatePlan(FGoapWorldState WorldState, TArray<FGoapNode> Plan, FGoapWorldState &ResultWorldState);

protected:

	FGoapPlanSharedPtr GoapPlan;
	virtual void BeginPlay() override;

private:
	UPROPERTY()
    TArray<FGoapAction> Actions;

	UPROPERTY()
    UGoapWorldComponent* GoapWorld;
	
	TArray<FGoapNode> GetAvailableNodes(FGoapNode CurrentNode);

	float Search(
		FGoapWorldState GoalState,
		TArray<FGoapNode> &Nodes,
		int32 Cost,
		int32 CurrentFBound,
		bool bAllowRepeatActions = false
	);
};