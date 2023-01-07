// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Vector.h"
#include "PIDController.h"
#include "fcl.h"
#include "StarWarsDrone.generated.h"

UCLASS()
class STARWARSTRAINING_UE5_API AStarWarsDrone : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		AActor* Enemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		double Kp;

	UPROPERTY(EditAnywhere)
		double Kd;

	UPROPERTY(EditAnywhere)
		double Ki;

	UPROPERTY(EditAnywhere)
		double Gain;

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
	float XGain = 100.0;
	float YGain = 100.0;
	float ZGain = 100.0;
	FVector Flinch = FVector(0, 0, 0);

	FeedforwardClosedloopLearning* fcl;
	UStaticMeshComponent* RootMeshComponent;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
