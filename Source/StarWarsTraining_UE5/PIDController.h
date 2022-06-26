// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class STARWARSTRAINING_UE5_API PIDController
{
public:
	PIDController(double _min, double _max, double _Kp, double _Kd, double _Ki);
	~PIDController();
	double update(double setpoint, double state, double dt);

private:
	double max;
	double min;
	double Kp;
	double Kd;
	double Ki;
	double error_old;
	double integral;

};

