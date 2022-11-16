// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "PIDController.h"
#include "StarWarsDrone.generated.h"

UCLASS()
class STARWARSTRAINING_UE5_API AStarWarsDrone : public AActor
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		AActor* Enemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionBox;


public:
	// Sets default values for this actor's properties
	AStarWarsDrone();

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AActor> MyProjectile;

	UFUNCTION()
		void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); protected:

private:
	PIDController* XController;
	PIDController* YController;
	PIDController* ZController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
