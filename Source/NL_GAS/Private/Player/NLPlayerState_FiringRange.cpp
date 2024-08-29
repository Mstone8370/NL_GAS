// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerState_FiringRange.h"

#include "NLGameplayTags.h"

void ANLPlayerState_FiringRange::SetPlayerStat(FGameplayTag StatTag, int32 Value)
{
    if (StatTag.IsValid())
    {
        if (StatTag.MatchesTagExact(Stat_Fire))
        {
            FireCount = FMath::Max(0, Value);
        }
        if (StatTag.MatchesTagExact(Stat_Hit))
        {
            HitCount = FMath::Max(0, Value);
        }
        if (StatTag.MatchesTagExact(Stat_Hit_Critical))
        {
            CriticalHitCount = FMath::Max(0, Value);
        }
        if (StatTag.MatchesTagExact(Stat_Kill))
        {
            KillCount = FMath::Max(0, Value);
        }
        if (StatTag.MatchesTagExact(Stat_Death))
        {
            DeathCount = FMath::Max(0, Value);
        }
        OnPlayerStatUpdated.Broadcast(this, StatTag, Value);
    }
}

void ANLPlayerState_FiringRange::AddPlayerStat(FGameplayTag StatTag, int32 ValueAdded)
{
    SetPlayerStat(StatTag, GetPlayerStat(StatTag) + ValueAdded);
}

int32 ANLPlayerState_FiringRange::GetPlayerStat(FGameplayTag StatTag) const
{
    if (StatTag.IsValid())
    {
        if (StatTag.MatchesTagExact(Stat_Fire))
        {
            return FireCount;
        }
        if (StatTag.MatchesTagExact(Stat_Hit))
        {
            return HitCount;
        }
        if (StatTag.MatchesTagExact(Stat_Hit_Critical))
        {
            return CriticalHitCount;
        }
        if (StatTag.MatchesTagExact(Stat_Kill))
        {
            return KillCount;
        }
        if (StatTag.MatchesTagExact(Stat_Death))
        {
            return DeathCount;
        }
    }
    return 0;
}

void ANLPlayerState_FiringRange::BroadcastAllPlayerStats() const
{
    OnPlayerStatUpdated.Broadcast(this, Stat_Fire, FireCount);
    OnPlayerStatUpdated.Broadcast(this, Stat_Hit, HitCount);
    OnPlayerStatUpdated.Broadcast(this, Stat_Hit_Critical, CriticalHitCount);
    OnPlayerStatUpdated.Broadcast(this, Stat_Kill, KillCount);
    OnPlayerStatUpdated.Broadcast(this, Stat_Death, DeathCount);
}

void ANLPlayerState_FiringRange::ResetPlayerStats()
{
    SetPlayerStat(Stat_Fire, 0);
    SetPlayerStat(Stat_Hit, 0);
    SetPlayerStat(Stat_Hit_Critical, 0);
    SetPlayerStat(Stat_Kill, 0);
    SetPlayerStat(Stat_Death, 0);
}
