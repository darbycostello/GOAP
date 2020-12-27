#pragma once

#include "GoapPlannerComponent.h"
#include "GoapStructs.h"
#include "Async/AsyncWork.h"

class FGoapPlanTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FGoapPlanTask>;

public:
	FGoapPlanTask(
		UGoapPlannerComponent* Planner,
		const FGoapWorldState WorldState,
		const FGoapWorldState GoalState,
		FGoapPlanSharedPtr* Plan,
		FGoapPlanTaskCompleteDynamicDelegate Complete
	):
		GoapPlanner(Planner),
		TaskWorldState(WorldState),
		TaskGoalState(GoalState),
		GoapPlan(Plan),
		TaskComplete(Complete)
	{}

protected:
	UGoapPlannerComponent* GoapPlanner;
	FGoapWorldState TaskWorldState;
	FGoapWorldState TaskGoalState;
	FGoapPlanSharedPtr* GoapPlan;
	FGoapPlanTaskCompleteDynamicDelegate TaskComplete;

	void DoWork() const {
		GoapPlanner->ExecutePlanner(TaskWorldState, TaskGoalState, GoapPlan);
		TaskComplete.Execute();
	};

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGoapPlanTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};