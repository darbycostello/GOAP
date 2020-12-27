
#include "GoapWorldComponent.h"
#include "GoapStructs.h"

UGoapWorldComponent::UGoapWorldComponent() {
	
}

void UGoapWorldComponent::BeginPlay() {
	Super::BeginPlay();

	if (WorldDataTable) {
		for(const auto Row : WorldDataTable->GetRowMap())
		{
			FGoapWorldStateData* Data = reinterpret_cast<FGoapWorldStateData*>(Row.Value);
			WorldState.Flags.Add(Data->Tag, Data->bFlag);
		}
	}
}