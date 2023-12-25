
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
				SF = 1.0;
			}
			else
			{
				SF = BrakeSet;
			}

			// check deadband
			if (abs(RateError) > Deadband * Sensor[ID].TargetUPM)
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

	LastPWM[ID] = Result;
	return (int)Result;
}

/*
int PIDvalve(byte ID)
{
    float Result = 0;
    if (Sensor[ID].FlowEnabled && Sensor[ID].RateSetting > 0)
    {
        float ErrorPercent = Sensor[ID].RateError / Sensor[ID].RateSetting;

        if (abs(ErrorPercent) > (float)Sensor[ID].Deadband / 100.0)
        {
            Result = Sensor[ID].MinPWM;
            if(ErrorPercent<0) Result = -Sensor[ID].MinPWM;
            
            Result += Sensor[ID].KP * ErrorPercent; //Errorpercent -0.19 -> 19%

            unsigned long elapsedTime = millis() - CurrentAdjustTime[ID];
            CurrentAdjustTime[ID] = millis();

            ErrorPercentCum[ID] += ErrorPercent * (elapsedTime * 0.001);

            Integral[ID] += Sensor[ID].KI * ErrorPercentCum[ID];
            if (Integral[ID] > 10) Integral[ID] = 10;
            if (Integral[ID] < -10) Integral[ID] = -10;
            if (Sensor[ID].KI == 0)
            {
                Integral[ID] = 0;
                ErrorPercentCum[ID] = 0;
            }
            

            Result += Integral[ID];

            Result += (float)Sensor[ID].KD * (ErrorPercent - ErrorPercentLast[ID]) / (elapsedTime * 0.001) * 0.001;

            ErrorPercentLast[ID] = ErrorPercent;

            bool IsPositive = (Result > 0);
            Result = abs(Result);

            if (Result < Sensor[ID].MinPWM)
            {
                Result = Sensor[ID].MinPWM;
            }
            else
            {
                if (abs(ErrorPercent) < (float)Sensor[ID].BrakePoint / 100.0)
                {
                    if (Result > Sensor[ID].MinPWM * 3.0) Result = Sensor[ID].MinPWM * 3.0;
                }
                if (Result > Sensor[ID].MaxPWM) Result = Sensor[ID].MaxPWM;
            }

            if (!IsPositive) Result *= -1;
        }
        else
        {
            Integral[ID] = 0;
        }
    }
    return (int)Result;
}
*/

int PIDvalve(byte ID) //TODO
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
				IntegralSum[ID] += Sensor[ID].KI * RateError;
				IntegralSum[ID] *= (Sensor[ID].KI > 0);	// zero out if not using KI

				DifValue = Sensor[ID].KD * (LastUPM[ID] - Sensor[ID].UPM);
				Result = Sensor[ID].KP * SF * RateError + IntegralSum[ID] + DifValue;

				bool IsPositive = (Result > 0);
				Result = abs(Result);

				if (Result > Sensor[ID].MaxPWM * SF) Result = Sensor[ID].MaxPWM * SF;
				if (Result < Sensor[ID].MinPWM) Result = Sensor[ID].MinPWM;

				if (!IsPositive) Result *= -1;
			}
			else
			{
				Result = 0;
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

