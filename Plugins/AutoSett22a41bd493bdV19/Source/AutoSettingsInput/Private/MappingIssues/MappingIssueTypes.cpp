// Copyright Sam Bonifacio. All Rights Reserved.

#include "MappingIssues/MappingIssueTypes.h"

bool FMappingAttemptInfo::CanUnbindPreviousMappings() const
{
	// Conflict resolution strategy: Only allow unbinding if all conflicting mappings are player-mappable
	// This prevents accidentally removing developer-defined default mappings that users shouldn't modify
	return !Issues.ContainsByPredicate([](const FMappingIssue& Issue)
		{
			return !SB::AutoSettings::Input::PlayerMapping::GetMapping(Issue.Mapping.GetMappingName()).IsPlayerMappable();
		});
}
