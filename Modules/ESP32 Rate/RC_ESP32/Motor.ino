
void AdjustFlow()
{
    for (int i = 0; i < MDL.SensorCount; i++)
    {
        // b9threlay: skip sensor 1 (motor channel used as 9th relay)
        if (b9threlay && i == 1) continue;

        float clamped = constrain(Sensor[i].PWM, -255.0f, 255.0f);

        switch (Sensor[i].ControlType)
        {
        case StandardValve_ct:
            SetPWM(i, SensorConnected[i] ? clamped : 0.0f);
            break;

        case Motor_ct:
        case Fan_ct:
            SetPWM(i, (SensorConnected[i] && Applying[i]) ? clamped : 0.0f);
            break;

        case ComboClose_ct:
        case TimedCombo_ct:
            // fast close valve or combo close timed, used for flow control and on/off
            SetPWM(i, SensorConnected[i] && Applying[i] ? clamped : -255.0f);
            break;

        default:
            break;
        }
    }
}

void SetPWM(byte ID, float pwmVal)
{
    const int maxDuty = (1 << PWM_BITS) - 1;
    int duty = (int)floorf(fabsf(pwmVal) * maxDuty / 255.0f);

    bool Increase = (pwmVal >= 0.0f);
    if (MDL.InvertFlow) Increase = !Increase;

#if PWM_BITS == 8
    duty = ditherAdjust(duty, fabsf(pwmVal));
#endif


#if defined(ESP32)
    if (Increase)
    {
        ledcWrite(ID * 2, duty);      // IN1 channel
        ledcWrite(ID * 2 + 1, 0);     // IN2 channel
    }
    else
    {
        ledcWrite(ID * 2, 0);         // IN1 channel
        ledcWrite(ID * 2 + 1, duty);  // IN2 channel
    }

#else
    digitalWrite(Sensor[ID].DirPin, Increase);
    analogWrite(Sensor[ID].PWMPin, duty);
#endif
}

#if PWM_BITS == 8
int ditherAdjust(int base, float val255)
{
    const int maxDuty = 255;
    float exactDuty = val255 * maxDuty / 255.0f;
    float frac = exactDuty - base;

    ditherCounter = (ditherCounter + 1) & 0x0F; // 16 step cycle
    if (frac > 0 && ditherCounter < (uint8_t)(frac * 16)) {
        base = min(base + 1, maxDuty);
    }
    return base;
}
#endif
