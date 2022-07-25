// Fill out your copyright notice in the Description page of Project Settings.


#include "StarWarsDrone.h"
#include "FCLWithFilterbank.h"

// Sets default values
AStarWarsDrone::AStarWarsDrone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	XController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	YController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	ZController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	UE_LOG(LogTemp, Warning, TEXT("Constructing a Drone"))

}

// Called when the game starts or when spawned
void AStarWarsDrone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStarWarsDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector EnemyLocation =  Enemy->GetActorLocation();
	FRotator EnemyRotation = Enemy->GetActorRotation();
	FVector NewLocation = GetActorLocation();
	FRotator NewRotation = GetActorRotation();

	// getting the angle betweeen drone and gun
	FVector d = NewLocation - EnemyLocation;
	float a = atan2f(d.Y, d.X);
	// now let's subtract the gun rotation
	float drone_actor_angle = a - EnemyRotation.Yaw + 90;
	// hopefully that's zero if the gun is pointing at the drone?

	NewLocation.X += XController->update(-200.0, NewLocation.X, DeltaTime);
	NewLocation.Y += YController->update(0.0, NewLocation.Y, DeltaTime);
	NewLocation.Z += ZController->update(90.0, NewLocation.Z, DeltaTime);

	SetActorLocationAndRotation(NewLocation, NewRotation);

	UE_LOG(LogTemp, Warning, TEXT("Enemy location: %f %f %f"), EnemyLocation.X, EnemyLocation.Y, EnemyLocation.Z);
	UE_LOG(LogTemp, Warning, TEXT("drone_actor_angle: %f"), drone_actor_angle);
}

