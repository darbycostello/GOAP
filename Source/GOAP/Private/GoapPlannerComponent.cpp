#include "GoapPlannerComponent.h"
#include "GoapPlanTask.h"
#include "GoapWorldComponent.h"
#include "Kismet/GameplayStatics.h"

UGoapPlannerComponent::UGoapPlannerComponent() {
	PrimaryComponentTick.bCanEverTick = false;
	bEnableLogging = false;
	GoapPlan = MakeShareable<FGoapPlan>(new FGoapPlan());
}

void UGoapPlannerComponent::SetGoapWorldComponent(UGoapWorldComponent* GoapWorldComponent) {
	if (!GoapWorldComponent) return;
	GoapWorld = GoapWorldComponent;
}

UGoapWorldComponent* UGoapPlannerComponent::GetGoapWorldComponent() const {
	return GoapWorld ? GoapWorld : nullptr;
}

void UGoapPlannerComponent::GetPlan(TArray<FGoapNode> &Plan) const {
	Plan = GoapPlan->Nodes;
}

void UGoapPlannerComponent::Plan(FGoapWorldState WorldState, FGoapWorldState GoalState, FGoapPlanTaskCompleteDynamicDelegate OnComplete) {
	(new FAutoDeleteAsyncTask<FGoapPlanTask>(this, WorldState, GoalState, &GoapPlan, OnComplete))->StartBackgroundTask();	
}

void UGoapPlannerComponent::BeginPlay() {
	Super::BeginPlay();

	if (!GoapWorld) {
		// Create a reference to the first UGoapWorldComponent found
		TArray<AActor*> WorldActors;
		UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), AActor::StaticClass(), WorldActors);
		if (WorldActors.Num() > 0) {
			for (auto& Actor : WorldActors) {
				UActorComponent* WorldComponent = Actor->GetComponentByClass(UGoapWorldComponent::StaticClass());
				if (WorldComponent) {
					GoapWorld = Cast<UGoapWorldComponent>(WorldComponent);	
				}
			}
		}

		if (ActionsDataTable) {
			for(const auto Row : ActionsDataTable->GetRowMap())
			{
				FGoapActionData* Data = reinterpret_cast<FGoapActionData*>(Row.Value);
				FGoapAction Action;
				Action.Name = Data->Name;
				Action.Cost = Data->Cost;
				Action.Preconditions = Data->Preconditions;
				Action.Effects = Data->Effects;
				Actions.Add(Action);
			}
		}
	}
}

void UGoapPlannerComponent::ExecutePlanner(const FGoapWorldState WorldState, const FGoapWorldState GoalState, FGoapPlanSharedPtr* Plan) {
	float CurrentFBound = static_cast<float>(WorldState.CalculateHeuristic(GoalState));
	if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Heuristic Estimate: %f"), CurrentFBound);
	TArray<FGoapNode> Nodes;
	FGoapNode StartNode;
	StartNode.WorldState = WorldState;
	Nodes.Add(StartNode);
	
	do {
		const float SmallestFNewBound = Search(GoalState, Nodes, 0, CurrentFBound);
		if (SmallestFNewBound == 0.0f) {
			if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Returning nodes"));
			FGoapNode EndNode = Nodes[Nodes.Num()-1];
			Plan->Get()->Add(EndNode);

			while (EndNode.Hash != -1) {
				EndNode = *Nodes.FindByPredicate([=](FGoapNode PathNode) {
                    return PathNode.Hash == EndNode.ParentHash;	
                });
				Plan->Get()->Insert(EndNode, 0);
			}
			return;		
		}
		CurrentFBound = SmallestFNewBound;
	} while (CurrentFBound != FLT_MAX);
}

void UGoapPlannerComponent::SimulatePlan(const FGoapWorldState WorldState, TArray<FGoapNode> Plan, FGoapWorldState &ResultWorldState) {
	ResultWorldState = FGoapWorldState(WorldState);
	for (auto& Node: Plan) {
		for (auto& Effect : Node.Action.Effects) {
			ResultWorldState.Flags.Add(Effect.Key, Effect.Value);	
		}
	}
}

float UGoapPlannerComponent::Search(
	const FGoapWorldState GoalState,
	TArray<FGoapNode> &Nodes,
	const int32 Cost,
	const int32 CurrentFBound,
	const bool bAllowRepeatActions
) {
	FGoapNode CurrentNode = Nodes[Nodes.Num()-1];
	CurrentNode.H = CurrentNode.WorldState.CalculateHeuristic(GoalState);
	CurrentNode.G = Cost;
	CurrentNode.F = Cost + CurrentNode.H;
	if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Current node %s - F: %f"), *CurrentNode.Action.Name.ToString(), CurrentNode.F);
	if (CurrentNode.F > CurrentFBound) {
		if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Current node F > Current F Bound"));
		return CurrentNode.F;
	}

	if (CurrentNode.WorldState.MeetsGoal(GoalState)) {
		if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Goal Met"));
		return 0.0f;
	}

	float MinFFound = INFINITY;
	TArray<FGoapNode> AvailableNodes = GetAvailableNodes(CurrentNode);
	if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - AvailableNodes: %d"), AvailableNodes.Num());
	for (auto& Node : AvailableNodes) {
		if (!Nodes.ContainsByPredicate([=](const FGoapNode PathNode) {
			return bAllowRepeatActions ? PathNode.Hash == Node.Hash : PathNode.Action.Name == Node.Action.Name;	
		})) {
			Nodes.Add(Node);
			if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Available Action found: %s"), *Node.Action.Name.ToString());
			const float MinFOverBound = Search(GoalState, Nodes, CurrentNode.G + Cost, CurrentFBound, bAllowRepeatActions);
			if (MinFOverBound == 0.0f) {
				// Goal met
				return 0.0f;
			}

			if (MinFOverBound < MinFFound) {
				MinFFound = MinFOverBound;
			}

			Nodes.RemoveAt(Nodes.Num()-1);
		}	
	}
	return MinFFound;
}

TArray<FGoapNode> UGoapPlannerComponent::GetAvailableNodes(FGoapNode CurrentNode) {
	TArray<FGoapNode> AvailableNodes;
	for (const auto& PotentialAction : Actions) {
		if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Potential action found: %s"), *PotentialAction.Name.ToString());
		if (PotentialAction.OperableOn(CurrentNode.WorldState)) {
			if (bEnableLogging) UE_LOG(LogTemp, Display, TEXT("GOAP - Potential action accepted: %s"), *PotentialAction.Name.ToString());
			FGoapNode Node(PotentialAction, PotentialAction.ActOn(CurrentNode.WorldState), CurrentNode.Hash);
			if (Node.Hash != CurrentNode.WorldState.GetHash()) {
				AvailableNodes.Add(Node);	
			}
		} else {
			if (bEnableLogging) UE_LOG(LogTemp, Warning, TEXT("GOAP - Potential action rejected: %s"), *PotentialAction.Name.ToString());
		}
	}
	return AvailableNodes;
}