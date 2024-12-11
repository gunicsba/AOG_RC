
// based off of https://github.com/br3ttb/Arduino-PID-Library

uint32_t LastCheck[MaxProductCount];
const double SampleTime = 50;
const double Deadband = 0.04;		// % error below which no adjustment is made
const double BrakePoint = 0.20;		// % error below which reduced adjustment is used
const double BrakeSet = 0.75;		// low adjustment rate
double SF;							// Settings Factor used to reduce adjustment when close to set rate
double DifValue;					// differential value on UPM

double RateError;
double LastPWM[MaxProductCount];
double IntegralSum[MaxProductCount];
double LastUPM[MaxProductCount];

const byte AdjustTime = 15;
const byte PauseTime = 250;
bool PauseAdjust[MaxProductCount];
uint32_t ComboTime[MaxProductCount];

void SetPWM()
{
	if (AutoOn)
	{
		// auto control
		for (int i = 0; i < MDL.SensorCount; i++)
		{
			switch (Sensor[i].ControlType)
			{
			case 2:
			case 3:
			case 4:
				// motor control
				Sensor[i].PWM = PIDmotor(i);
				break;

			case 5:
				// combo close timed adjustment
				Sensor[i].PWM = TimedCombo(i, false);
				break;

			default:
				// valve control
				Sensor[i].PWM = PIDvalve(i);
				break;
			}
		}
	}
	else
	{
		// manual control
		for (int i = 0; i < MDL.SensorCount; i++)
		{
			switch (Sensor[i].ControlType)
			{
			case 5:
				// combo close timed adjustment
				Sensor[i].PWM = TimedCombo(i, true);
				break;

			default:
				Sensor[i].PWM = Sensor[i].ManualAdjust;
				break;
			}
		}
	}
}

int PIDmotor(byte ID)
{
	double Result = 0;
	if (Sensor[ID].FlowEnabled && Sensor[ID].TargetUPM > 0)
	{
		Result = LastPWM[ID];
		if (millis() - LastCheck[ID] >= SampleTime)
		{
			LastCheck[ID] = millis();

			RateError = Sensor[ID].TargetUPM - Sensor[ID].UPM;

			if (abs(RateError) > Sensor[ID].TargetUPM) //Not sure if this will ever run
			{
				if (RateError > 0)
				{
					RateError = Sensor[ID].TargetUPM;
				}
				else
				{
					RateError = Sensor[ID].TargetUPM * -1;
				}
			}

			// check brakepoint
			if (abs(RateError) > BrakePoint * Sensor[ID].TargetUPM)
			{
				SF = 1.0;
			}
			else
			{
				SF = BrakeSet;
			}

			// check deadband
//			if (abs(RateError) > Deadband * Sensor[ID].TargetUPM)
			{
				IntegralSum[ID] += Sensor[ID].KI * RateError;
				IntegralSum[ID] *= (Sensor[ID].KI > 0);	// zero out if not using KI

				DifValue = Sensor[ID].KD * (LastUPM[ID] - Sensor[ID].UPM);
				Result += Sensor[ID].KP * SF * RateError + IntegralSum[ID] + DifValue;

				if (Result > Sensor[ID].MaxPWM) Result = Sensor[ID].MaxPWM;
				if (Result < Sensor[ID].MinPWM) Result = Sensor[ID].MinPWM;
			}
			LastUPM[ID] = Sensor[ID].UPM;
		}
	}
	else
	{
		IntegralSum[ID] = 0;
	}

	if (Result != 0) LastPWM[ID] = Result;
	return (int)Result;
}

int PIDvalve(byte ID)
{
	double Result = 0;
	if (Sensor[ID].FlowEnabled && Sensor[ID].TargetUPM > 0)
	{
		Result = LastPWM[ID];
		if (millis() - LastCheck[ID] >= SampleTime)
		{
			LastCheck[ID] = millis();

			RateError = Sensor[ID].TargetUPM - Sensor[ID].UPM;
			if (abs(RateError) > Sensor[ID].TargetUPM)
			{
				if (RateError > 0)
				{
					RateError = Sensor[ID].TargetUPM;
				}
				else
				{
					RateError = Sensor[ID].TargetUPM * -1;
				}
			}

			// check brakepoint
			if (abs(RateError) > BrakePoint * Sensor[ID].TargetUPM)
			{
				SF = 1;
			}
			else
			{
				SF = BrakeSet;
			}

			// check deadband
			if (abs(RateError) > Deadband * Sensor[ID].TargetUPM)
			{
        if( (RateError > 0) != (Result > 0) ) IntegralSum[ID] = 0; //if we shoot over reset IntegralSum
				IntegralSum[ID] += Sensor[ID].KI * abs(RateError) * SF;
        if(IntegralSum[ID] > 50) IntegralSum[ID] = 100;
				IntegralSum[ID] *= (Sensor[ID].KI > 0);	// zero out if not using KI
				DifValue = Sensor[ID].KD * (LastUPM[ID] - Sensor[ID].UPM);
				Result = Sensor[ID].MinPWM + Sensor[ID].KP * SF * abs(RateError) + IntegralSum[ID] + DifValue;

				bool IsPositive = (RateError > 0);
				Result = abs(Result);

				if (Result > Sensor[ID].MaxPWM * SF) Result = Sensor[ID].MaxPWM * SF;
				if (Result < Sensor[ID].MinPWM) Result = Sensor[ID].MinPWM;

				if (!IsPositive) Result *= -1.0;
			}
			else
			{
				Result = 0;
        IntegralSum[ID] = 0;
			}
			LastUPM[ID] = Sensor[ID].UPM;
		}
	}
	else
	{
		IntegralSum[ID] = 0;
	}

	LastPWM[ID] = Result;
	return (int)Result;
}

String getDebugPID(byte ID){
  String fr = "";
  fr += "PID Debug for valve:";
  fr += ID;
  fr += "<br> MaxPWM: ";
  fr += Sensor[ID].MaxPWM;
  fr += " MinPWM: ";
  fr += Sensor[ID].MinPWM;
  fr += "<br> FlowEnabled? ";
  fr += Sensor[ID].FlowEnabled;
  fr += "<br> TargetUPM: ";
  fr += Sensor[ID].TargetUPM;
  fr += " UPM: ";
  fr += Sensor[ID].UPM;
  fr += " RateError: ";
  fr += (Sensor[ID].TargetUPM - Sensor[ID].UPM);
  fr += "<br> LastCheck: ";
  fr += LastCheck[ID];
  fr += " SampleTime: ";
  fr += SampleTime;
  fr += "<br> BrakePoint: ";
  fr += BrakePoint;
  fr += "<br> IntegralSum[ID]: "; 
  fr += IntegralSum[ID];
  fr += " KD: ";
  fr += Sensor[ID].KD;
  fr += " KP: ";
  fr += Sensor[ID].KP;
  fr += " IntegralSum: ";
  fr += IntegralSum[ID];
  fr += "<br> LastPWM ";
  fr += LastPWM[ID];
  fr += "<br><br> DifValue = Sensor[ID].KD * (LastUPM[ID] - Sensor[ID].UPM) ";
  fr += (Sensor[ID].KD * (LastUPM[ID] - Sensor[ID].UPM));
  fr += "<br><br> Sensor[ID].KP * SF * RateError + IntegralSum[ID] + DifValue) ";
  fr += Sensor[ID].KP * SF * RateError + IntegralSum[ID];
 return fr;
}

int TimedCombo(byte ID, bool ManualAdjust = false)
{
	int Result = 0;
	if (PauseAdjust[ID])
	{
		// pausing state
		if (millis() - ComboTime[ID] > PauseTime)
		{
			// switch state
			ComboTime[ID] = millis();
			PauseAdjust[ID] = !PauseAdjust[ID];
		}
	}
	else
	{
		// adjusting state
		if (millis() - ComboTime[ID] > AdjustTime)
		{
			// switch state
			ComboTime[ID] = millis();
			PauseAdjust[ID] = !PauseAdjust[ID];
		}
		else
		{
			if (ManualAdjust)
			{
				Result = 255;
			}
			else
			{
				// auto adjust, check deadband
				if (Sensor[ID].TargetUPM > 0)
				{
					RateError = Sensor[ID].TargetUPM - Sensor[ID].UPM;
					if (abs(RateError / Sensor[ID].TargetUPM) > Deadband)  Result = 255;
				}
			}
		}
	}
	return Result;
}

