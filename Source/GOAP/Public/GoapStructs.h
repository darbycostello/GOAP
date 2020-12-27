#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GoapStructs.generated.h"

/**
* Stores a given world state for a single user
*/
USTRUCT(BlueprintType)
struct GOAP_API FGoapWorldState {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, bool> Flags;

	FGoapWorldState() {}

	uint32 GetHash() {
		uint32 HashValue = 0;
		for (auto& Flag: Flags) {
			HashValue = HashCombine(HashValue, GetTypeHash(Flag));
		}
		return HashValue;
	}

	bool MeetsGoal(const FGoapWorldState& GoalState) const {
		for (const auto& Flag : GoalState.Flags) {
			if (GoalState.Flags.Contains(Flag.Key) && Flags.Contains(Flag.Key)) {
				if (GoalState.Flags[Flag.Key] != Flags[Flag.Key]) {
					return false;
				}
			} else {
				return false;
			}
		}
		return true;
	}

	int CalculateHeuristic(const FGoapWorldState& GoalState) const {
		int Distance = 0;
		const int Last = Flags.Num() - 1;
		int i = 0;
		for (const auto& KeyValue : GoalState.Flags) {
			if (Flags.Contains(KeyValue.Key)) {
				if (i == Last || Flags[KeyValue.Key] != KeyValue.Value) {
					Distance++;
				}
			} else {
				Distance++;
			}
			i++;
		}
		return Distance;
	}
};

/**
 * Stores the preconditions, cost and effects for a given action
 */
USTRUCT(BlueprintType)
struct GOAP_API FGoapAction {

	GENERATED_BODY()
	
    UPROPERTY(BlueprintReadWrite)
    FName Name;

	UPROPERTY(BlueprintReadWrite)
    int32 Cost;

	UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, bool> Preconditions;

	UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, bool> Effects;
	
	bool OperableOn(const FGoapWorldState& WorldState) const {
		for (const auto& Precondition : Preconditions) {
			if (WorldState.Flags.Contains(Precondition.Key)) {
				if (WorldState.Flags[Precondition.Key] != Precondition.Value) {
					return false;
				}
			} else {
				return false;
			}
		}
		return true;
	}

	FGoapWorldState ActOn(const FGoapWorldState& WorldState) const {
		FGoapWorldState TestWorldState(WorldState);
		for (const auto& Effect : Effects) {
			TestWorldState.Flags.Add(Effect.Key, Effect.Value);
		}
		return TestWorldState;
	}
};

/**
* Stores the world state and the action that creates it as a single data structure
*/
USTRUCT(BlueprintType)
struct GOAP_API FGoapNode {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGoapWorldState WorldState;

	UPROPERTY(BlueprintReadWrite)
	FGoapAction Action;

	uint32 Hash = -1;
    uint32 ParentHash = -1; 
	float H = FLT_MIN;
	float G = 0.0f;
	float F = 0.0f;

	FGoapNode() {
		Action.Name = FName();
		Action.Cost = 0;
	}

	FGoapNode(FGoapAction Action, const FGoapWorldState WorldState, const uint32 ParentHash) {
		this->Action = Action;
		this->WorldState = WorldState;
		this->Hash = this->WorldState.GetHash();
		this->ParentHash = ParentHash;
	}
};

/**
* Stores the nodes making up a GOAP plan
*/
USTRUCT(BlueprintType)
struct GOAP_API FGoapPlan {
	GENERATED_BODY()

	TArray<FGoapNode> Nodes;

	FGoapPlan(){}

	void Add(const FGoapNode Node) {
		Nodes.Add(Node);
	}

	void Insert(const FGoapNode Node, const int32 Index) {
		Nodes.Insert(Node, Index);
	}

	void Clear() {
		Nodes.Empty();
	}
};

/**
* Defines a shared pointer to the FGoapPlan struct
*/
typedef TSharedPtr<struct FGoapPlan, ESPMode::ThreadSafe> FGoapPlanSharedPtr;

/**
* The table row structure for storing all of the accessible flags for a given world state 
*/
USTRUCT(BlueprintType)
struct GOAP_API FGoapWorldStateData : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(BlueprintReadWrite)
	bool bFlag;
};

/**
* The table row structure for storing all of the accessible actions for a given planner 
*/
USTRUCT(BlueprintType)
struct GOAP_API FGoapActionData : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
    FName Name;

	UPROPERTY(BlueprintReadWrite)
    TArray<FString> Tags;

	UPROPERTY(BlueprintReadWrite)
    int32 Cost;
	
	UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, bool> Preconditions;
	
	UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, bool> Effects;
};