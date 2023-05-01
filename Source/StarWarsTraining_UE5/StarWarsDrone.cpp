// Fill out your copyright notice in the Description page of Project Settings.


#include "StarWarsDrone.h"

// Sets default values
AStarWarsDrone::AStarWarsDrone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
/*
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName("OverlapAll");
	CollisionBox->SetNotifyRigidBodyCollision(true);
	CollisionBox->SetHiddenInGame(false);
	*/

	CollisionBall = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionBall"));
	CollisionBall->SetupAttachment(RootComponent);
	CollisionBall->SetCollisionProfileName("OverlapAll");
	CollisionBall->SetNotifyRigidBodyCollision(true);
	CollisionBall->SetHiddenInGame(false);

	UE_LOG(LogTemp, Warning, TEXT("Constructing a Drone"))

}

// Called when the game starts or when spawned
void AStarWarsDrone::BeginPlay()
{
	Super::BeginPlay();
	// attach collision components to sockets based on transformations definitions
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
/*
	CollisionBox->AttachToComponent(RootComponent, AttachmentRules, "SphereCollisionSocket");
	CollisionBox->OnComponentHit.AddDynamic(this, &AStarWarsDrone::OnAttackHit);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AStarWarsDrone::OnOverlapBegin);
*/
	CollisionBall->AttachToComponent(RootComponent, AttachmentRules, "SphereCollisionSocket");
	CollisionBall->OnComponentHit.AddDynamic(this, &AStarWarsDrone::OnAttackHit);
	CollisionBall->OnComponentBeginOverlap.AddDynamic(this, &AStarWarsDrone::OnOverlapBegin);

	XController = new PIDController(-100.0, 100.0, Kp, Kd, Ki);
	YController = new PIDController(-100.0, 100.0, Kp, Kd, Ki);
	ZController = new PIDController(-100.0, 100.0, Kp, Kd, Ki);
	UE_LOG(LogTemp, Warning, TEXT("Kp %f Kd %f Ki %f"), Kp, Kd, Ki)

	/*
	CollisionSphere->AttachToComponent(RootComponent, AttachmentRules, "SphereCollisionSocket");
	CollisionSphere->OnComponentHit.AddDynamic(this, &AStarWarsDrone::OnAttackHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AStarWarsDrone::OnOverlapBegin);
	*/
	// FCL learning network
	TArray<int> nNeur = { 2,2 };
	fcl = new FeedforwardClosedloopLearning(4, nNeur);
	UE_LOG(LogTemp, Warning, TEXT("Num layers: %d"), fcl->getNumLayers())
	fcl->setLearningRate(0.01);
	fcl->initWeights(1, 0, FCLNeuron::MAX_OUTPUT_RANDOM);
	fcl->setLearningRateDiscountFactor(1.0);
	fcl->setBias(0);
	UE_LOG(LogTemp, Warning, TEXT("Constructing FCL"))
	RootMeshComponent = Cast<UStaticMeshComponent>(RootComponent);

}

// Called every frame
void AStarWarsDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector EnemyLocation = Enemy->GetActorLocation();
	FRotator EnemyRotation = Enemy->GetActorRotation();
	FVector DroneLocation = GetActorLocation();
	FRotator DroneRotation = GetActorRotation();

	// getting the angle betweeen drone and gun
	FVector d = DroneLocation - EnemyLocation;
	float a = atan2f(d.Y, d.X);
	a = 360.0 * a / (2.0 * PI);
	a = fmod(a, 360.0);

	float hyp = sqrt(d.Y * d.Y + d.X * d.X);
	float b = atan2f(d.Z, hyp);
	// now let's subtract the gun rotation
	float drone_actor_angle_yaw = a - (EnemyRotation.Yaw + 90);
	float drone_actor_angle_roll = b - EnemyRotation.Roll;
	float cos_drone_actor_angle_yaw = cos(2 * PI * drone_actor_angle_yaw / 360.0);
	float cos_drone_actor_angle_roll = cos(2 * PI * drone_actor_angle_roll / 360.0);
	float sin_drone_actor_angle_yaw = sin(2 * PI * drone_actor_angle_yaw / 360.0);
	float sin_drone_actor_angle_roll = sin(2 * PI * drone_actor_angle_roll / 360.0);
	// hopefully that's zero if the gun is pointing at the drone?

	float XOutput = XController->update(890.0, DroneLocation.X, DeltaTime);
	float YOutput = YController->update(0.0, DroneLocation.Y, DeltaTime);
	float ZOutput = ZController->update(100.0, DroneLocation.Z, DeltaTime);

//	UE_LOG(LogTemp, Warning, TEXT("Aiming Angle Yaw: %f  Roll: %f"), drone_actor_angle_yaw, drone_actor_angle_roll);
//	UE_LOG(LogTemp, Warning, TEXT("Yaw: %f Angle: %f Enemy: %f"), drone_actor_angle_yaw, a, EnemyRotation.Yaw);
//	UE_LOG(LogTemp, Warning, TEXT("Cos Yaw: %f Sin Yaw: %f "), cos_drone_actor_angle_yaw, sin_drone_actor_angle_yaw);

	Flinch = 0.0;
// Simulate the same learning task as the LineFollower: imagine two rays at +30 degrees and -30 from where the pistol is pointing. These are the 
	// edges of the 'road'. If the drone is outside these boundaries, it receives an impulse to push it back towards where the pistol points. 
	if (sin_drone_actor_angle_yaw < -0.5)
		Flinch = -1.0;
	if (sin_drone_actor_angle_yaw > 0.5)
		Flinch = 1.0;

	input = { cos_drone_actor_angle_yaw, cos_drone_actor_angle_roll, sin_drone_actor_angle_yaw, sin_drone_actor_angle_roll };
	error = { Flinch, Flinch, Flinch, Flinch };
	fcl->doStep(input, error);
	UE_LOG(LogTemp, Warning, TEXT("%f %f %f %f %f %f"), cos_drone_actor_angle_yaw, cos_drone_actor_angle_roll,
		sin_drone_actor_angle_yaw, sin_drone_actor_angle_roll, Flinch, fcl->getOutputLayer()->getNeuron(0)->getOutput());

	FRotator NetAngle = FRotator(0.0, -90.0, 0.0);
	FVector NetVector = NetAngle.RotateVector(d);
	NetVector /= NetVector.Size();
	FVector FlinchVector = FVector(NetVector);
	NetVector *= Gain * (fcl->getOutputLayer()->getNeuron(0)->getOutput());
	FlinchVector *= Flinch * DodgeGain;
	NetVector += FlinchVector;
	RootMeshComponent->AddImpulse(FVector(
		XGain * XOutput + NetVector.X, 
		YGain * YOutput + NetVector.Y,
		ZGain * ZOutput
			));

//	NewLocation.X += XOutput;// +Gain * (fcl->getOutputLayer()->getNeuron(0)->getOutput());
//	NewLocation.Y += YOutput;// +Gain * (fcl->getOutputLayer()->getNeuron(1)->getOutput());
//	NewLocation.Z += ZOutput;// +Gain * (fcl->getOutputLayer()->getNeuron(2)->getOutput());

//	SetActorLocationAndRotation(NewLocation, NewRotation);
}

void AStarWarsDrone::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit event"));
}


void AStarWarsDrone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector NewLocation = GetActorLocation();
//	UE_LOG(LogTemp, Warning, TEXT("Overlap event"));
//	UE_LOG(LogTemp, Warning, TEXT("Actor name: %s"), *FString(OtherActor->GetName()));
	UClass* MyClass = OtherActor->GetClass();
//	UE_LOG(LogTemp, Warning, TEXT("Actor class desc: %s"), *FString(MyClass->GetDescription()));

	// todo: need a better way to test if it's a projectile, but how to include Blueprint classes?
	if (MyClass->GetDescription().StartsWith(FString("Projectile"))) {
//		UE_LOG(LogTemp, Warning, TEXT("OUCH"));
//		UE_LOG(LogTemp, Warning, TEXT("Sweep: %f %f %f"), SweepResult.ImpactPoint.X, SweepResult.ImpactPoint.Y, SweepResult.ImpactPoint.Z);
		FVector OtherActorLocation = OtherActor->GetActorLocation();
		FVector MyLocation = GetActorLocation();
		FVector RelativeImpactPoint = SweepResult.ImpactPoint - MyLocation;
		FVector Diff = MyLocation - OtherActorLocation;
		FRotator ImpactDirection = Diff.Rotation();
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Black, TEXT("Ouch!!"));
		FRotator BulletRotation = OtherActor->GetActorRotation();
		FVector BulletDirection = BulletRotation.Vector();
		BulletDirection.Normalize();
		RelativeImpactPoint.Normalize();
		float DotVectors = BulletDirection.X * RelativeImpactPoint.X + BulletDirection.Y * RelativeImpactPoint.Y;
		float DetVectors = BulletDirection.X * RelativeImpactPoint.Y - BulletDirection.Y * RelativeImpactPoint.X;
		float angle = atan2(DetVectors, DotVectors);
//		UE_LOG(LogTemp, Warning, TEXT("Bullet: Yaw: %f Roll: %f Pitch: %f"), BulletRotation.Yaw, BulletRotation.Pitch, BulletRotation.Roll);
//		UE_LOG(LogTemp, Warning, TEXT("Impact: Yaw: %f Roll: %f Pitch: %f"), ImpactDirection.Yaw, ImpactDirection.Pitch, ImpactDirection.Roll);
//		UE_LOG(LogTemp, Warning, TEXT("Cos Theta: %f Sin Theta: %f"), cos(angle), sin(angle));
		FRotator FlinchAngle;
/*		if (sin(angle) > 0.0) {
			FlinchAngle = FRotator(0.0, -90.0, 0.0);
			Flinch = -1.0;
		}
		else {
			FlinchAngle = FRotator(0.0, 90.0, 0.0);
			Flinch = 1.0;
		}
		*/
//		UE_LOG(LogTemp, Warning, TEXT("Bullet Direction: X: %f Y: %f Z: %f"), BulletRotation.Vector().X, BulletRotation.Vector().Y, BulletRotation.Vector().Z);
		FVector FlinchVector = FlinchAngle.RotateVector(BulletDirection);

		//		UE_LOG(LogTemp, Warning, TEXT("Flinch: X: %f Y: %f Z: %f"), Flinch.X, Flinch.Y, Flinch.Z);

//		RootMeshComponent->AddImpulse(DodgeGain * FlinchVector);

		/*		
		UE_LOG(LogTemp, Warning, TEXT("Vector: %f %f %f"), Diff.X, Diff.Y, Diff.Z);
		UE_LOG(LogTemp, Warning, TEXT("Impact Vector: %f %f %f"), RelativeImpactPoint.X, RelativeImpactPoint.Y, RelativeImpactPoint.Z);
		UE_LOG(LogTemp, Warning, TEXT("Impact Vector: %f %f %f"), RelativeImpactPoint.X, RelativeImpactPoint.Y, RelativeImpactPoint.Z);
	*/
	}

}