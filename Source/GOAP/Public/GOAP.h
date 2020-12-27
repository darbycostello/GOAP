
#pragma once

#include "CoreMinimal.h"

class FGOAP : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

