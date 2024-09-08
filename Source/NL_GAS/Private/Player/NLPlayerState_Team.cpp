// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerState_Team.h"

#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"

void ANLPlayerState_Team::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState_Team, KillCount, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState_Team, DeathCount, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState_Team::OnRep_KillCount()
{
    PlayerStatUpdated.Broadcast(this, Stat_Kill, KillCount);
}

void ANLPlayerState_Team::OnRep_DeathCount()
{
    PlayerStatUpdated.Broadcast(this, Stat_Death, DeathCount);
}

void ANLPlayerState_Team::SetPlayerStat(FGameplayTag StatTag, int32 Value)
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
        PlayerStatUpdated.Broadcast(this, StatTag, Value);
    }
}

void ANLPlayerState_Team::AddPlayerStat(FGameplayTag StatTag, int32 ValueAdded)
{
    SetPlayerStat(StatTag, GetPlayerStat(StatTag) + ValueAdded);
}

int32 ANLPlayerState_Team::GetPlayerStat(FGameplayTag StatTag) const
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

void ANLPlayerState_Team::BroadcastPlayerAllStats() const
{
    PlayerStatUpdated.Broadcast(this, Stat_Fire, FireCount);
    PlayerStatUpdated.Broadcast(this, Stat_Hit, HitCount);
    PlayerStatUpdated.Broadcast(this, Stat_Hit_Critical, CriticalHitCount);
    PlayerStatUpdated.Broadcast(this, Stat_Kill, KillCount);
    PlayerStatUpdated.Broadcast(this, Stat_Death, DeathCount);
}

void ANLPlayerState_Team::ResetPlayerStats()
{
    SetPlayerStat(Stat_Fire, 0);
    SetPlayerStat(Stat_Hit, 0);
    SetPlayerStat(Stat_Hit_Critical, 0);
    SetPlayerStat(Stat_Kill, 0);
    SetPlayerStat(Stat_Death, 0);
}