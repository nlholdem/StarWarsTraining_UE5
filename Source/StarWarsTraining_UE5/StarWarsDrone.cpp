// Fill out your copyright notice in the Description page of Project Settings.


#include "StarWarsDrone.h"

// Sets default values
AStarWarsDrone::AStarWarsDrone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	XController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	YController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	ZController = new PIDController(-100.0, 100.0, 0.01, 0.001, 0.05);
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName("OverlapAll");
	CollisionBox->SetNotifyRigidBodyCollision(true);
	CollisionBox->SetHiddenInGame(false);

	UE_LOG(LogTemp, Warning, TEXT("Constructing a Drone"))

}

// Called when the game starts or when spawned
void AStarWarsDrone::BeginPlay()
{
	Super::BeginPlay();
	// attach collision components to sockets based on transformations definitions
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	CollisionBox->AttachToComponent(RootComponent, AttachmentRules, "SphereCollisionSocket");
	CollisionBox->OnComponentHit.AddDynamic(this, &AStarWarsDrone::OnAttackHit);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AStarWarsDrone::OnOverlapBegin);

}

// Called every frame
void AStarWarsDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector EnemyLocation = Enemy->GetActorLocation();
	FRotator EnemyRotation = Enemy->GetActorRotation();
	FVector NewLocation = GetActorLocation();
	FRotator NewRotation = GetActorRotation();

	// getting the angle betweeen drone and gun
	FVector d = NewLocation - EnemyLocation;
	float a = atan2f(d.Y, d.X);
	float hyp = sqrt(d.Y * d.Y + d.X * d.X);
	float b = atan2f(d.Z, hyp);
	// now let's subtract the gun rotation
	float drone_actor_angle_yaw = a - EnemyRotation.Yaw + 90;
	float drone_actor_angle_roll = b - EnemyRotation.Roll;
	// hopefully that's zero if the gun is pointing at the drone?

	NewLocation.X += XController->update(-200.0, NewLocation.X, DeltaTime);
	NewLocation.Y += YController->update(0.0, NewLocation.Y, DeltaTime);
	NewLocation.Z += ZController->update(90.0, NewLocation.Z, DeltaTime);
	//	UE_LOG(LogTemp, Warning, TEXT("Aiming Angle Yaw: %f  Roll: %f"), drone_actor_angle_yaw, drone_actor_angle_roll);

	SetActorLocationAndRotation(NewLocation, NewRotation);
}

void AStarWarsDrone::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit event"));
}


void AStarWarsDrone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector NewLocation = GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("Overlap event"));
	UE_LOG(LogTemp, Warning, TEXT("Actor name: %s"), *FString(OtherActor->GetName()));
	UClass* MyClass = OtherActor->GetClass();
	UE_LOG(LogTemp, Warning, TEXT("Actor class desc: %s"), *FString(MyClass->GetDescription()));

	// todo: need a better way to test if it's a projectile, but how to include Blueprint classes?
	if (MyClass->GetDescription().StartsWith(FString("Projectile"))) {
		UE_LOG(LogTemp, Warning, TEXT("OUCH"));
		UE_LOG(LogTemp, Warning, TEXT("Sweep: %f %f %f"), SweepResult.ImpactPoint.X, SweepResult.ImpactPoint.Y, SweepResult.ImpactPoint.Z);
		FVector OtherActorLocation = OtherActor->GetActorLocation();
		FVector MyLocation = GetActorLocation();
		FVector Diff = MyLocation - OtherActorLocation;
		UE_LOG(LogTemp, Warning, TEXT("Vector: %f %f %f"), Diff.X, Diff.Y, Diff.Z);

		NewLocation.X = -60.0;
		NewLocation.Y = 0.0;
		NewLocation.Z = 50.0;
		SetActorLocation(NewLocation);
	}

}