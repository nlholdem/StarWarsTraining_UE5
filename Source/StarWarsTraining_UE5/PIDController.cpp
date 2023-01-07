// Fill out your copyright notice in the Description page of Project Settings.


#include "PIDController.h"

PIDController::PIDController(double _min, double _max, double _Kp, double _Kd, double _Ki)
{
	min = _min;
	max = _max;
	Kp = _Kp;
	Kd = _Kd;
	Ki = _Ki;
	error_old = 0.0;
	integral = 0.0;
	UE_LOG(LogTemp, Warning, TEXT("Constructing PIDController... Kp %f Kd %f Ki %f "), Kp, Kd, Ki);

}

PIDController::~PIDController()
{
}

double PIDController::update(double setpoint, double state, double dt)
{


    // Calculate error
    double error = setpoint - state;

    // Proportional term
    double Pout = Kp * error;

    // Integral term
    integral += error * dt;
    double Iout = Ki * integral;

    // Derivative term
    double derivative = (error - error_old) / dt;
    double Dout = Kd * derivative;

    // Calculate total output
    double output = Pout + Iout + Dout;

    // Restrict to max/min
    if (output > max)
        output = max;
    else if (output < min)
        output = min;

    // Save error to previous error
    error_old = error;
//    UE_LOG(LogTemp, Warning, TEXT("CONTROLLER......  setpt %f state %f err %f out %f"), setpoint, state, error, output);

    return output;
}

