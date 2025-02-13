// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/NavQueryFilter_AvoidCharacters.h"
#include "Characters/CharacterBase.h"
#include "NavAreas/NavArea.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"

UNavQueryFilter_AvoidCharacters::UNavQueryFilter_AvoidCharacters()
{
    FNavigationFilterArea AreaCost1;
    AreaCost1.AreaClass = UNavArea_Null::StaticClass(); // NavArea_Null을 피하고자 할 때
    AreaCost1.TravelCostOverride = 10000.0f; // 매우 높은 비용 설정
    AreaCost1.bOverrideTravelCost = true;
    AreaCost1.bOverrideEnteringCost = true;

    //FNavigationFilterArea AreaCost2;
    //AreaCost2.AreaClass = ACharacterBase::StaticClass(); // NavArea_Null을 피하고자 할 때
    //AreaCost2.TravelCostOverride = 10000.0f; // 매우 높은 비용 설정
    //AreaCost2.bOverrideTravelCost = true;
    //AreaCost2.bOverrideEnteringCost = true;

    // 이 필터를 NavQueryFilter에 추가
    Areas.Add(AreaCost1);
    //Areas.Add(AreaCost2);
}