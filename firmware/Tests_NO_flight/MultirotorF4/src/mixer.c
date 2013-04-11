#include "board.h"
#include "mw.h"

static uint8_t numberMotor = 0;
uint8_t useServo = 0;
int16_t motor[MAX_MOTORS];
int16_t servo[8] = { 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500 };

static motorMixer_t currentMixer[MAX_MOTORS];

static const motorMixer_t mixerTri[] = {
    { 1.0f,  0.0f,  1.333333f,  0.0f },     // REAR
    { 1.0f, -1.0f, -0.666667f,  0.0f },     // RIGHT
    { 1.0f,  1.0f, -0.666667f,  0.0f },     // LEFT
};

static const motorMixer_t mixerQuadP[] = {
    { 1.0f,  0.0f,  1.0f, -1.0f },          // REAR
    { 1.0f, -1.0f,  0.0f,  1.0f },          // RIGHT
    { 1.0f,  1.0f,  0.0f,  1.0f },          // LEFT
    { 1.0f,  0.0f, -1.0f, -1.0f },          // FRONT
};

static const motorMixer_t mixerQuadX[] = {
    { 1.0f, -1.0f,  1.0f, -1.0f },          // REAR_R
    { 1.0f, -1.0f, -1.0f,  1.0f },          // FRONT_R
    { 1.0f,  1.0f,  1.0f,  1.0f },          // REAR_L
    { 1.0f,  1.0f, -1.0f, -1.0f },          // FRONT_L
};

static const motorMixer_t mixerBi[] = {
    { 1.0f,  1.0f,  0.0f,  0.0f },          // LEFT
    { 1.0f, -1.0f,  0.0f,  0.0f },          // RIGHT
};

static const motorMixer_t mixerY6[] = {
    { 1.0f,  0.0f,  1.333333f,  1.0f },     // REAR
    { 1.0f, -1.0f, -0.666667f, -1.0f },     // RIGHT
    { 1.0f,  1.0f, -0.666667f, -1.0f },     // LEFT
    { 1.0f,  0.0f,  1.333333f, -1.0f },     // UNDER_REAR
    { 1.0f, -1.0f, -0.666667f,  1.0f },     // UNDER_RIGHT
    { 1.0f,  1.0f, -0.666667f,  1.0f },     // UNDER_LEFT
};

static const motorMixer_t mixerHex6P[] = {
    { 1.0f, -1.0f,  0.866025f,  1.0f },     // REAR_R
    { 1.0f, -1.0f, -0.866025f, -1.0f },     // FRONT_R
    { 1.0f,  1.0f,  0.866025f,  1.0f },     // REAR_L
    { 1.0f,  1.0f, -0.866025f, -1.0f },     // FRONT_L
    { 1.0f,  0.0f, -0.866025f,  1.0f },     // FRONT
    { 1.0f,  0.0f,  0.866025f, -1.0f },     // REAR
};

static const motorMixer_t mixerY4[] = {
    { 1.0f,  0.0f,  1.0f, -1.0f },          // REAR_TOP CW
    { 1.0f, -1.0f, -1.0f,  0.0f },          // FRONT_R CCW
    { 1.0f,  0.0f,  1.0f,  1.0f },          // REAR_BOTTOM CCW
    { 1.0f,  1.0f, -1.0f,  0.0f },          // FRONT_L CW
};

static const motorMixer_t mixerHex6X[] = {
    { 1.0f, -0.866025f,  1.0f,  1.0f },     // REAR_R
    { 1.0f, -0.866025f, -1.0f,  1.0f },     // FRONT_R
    { 1.0f,  0.866025f,  1.0f, -1.0f },     // REAR_L
    { 1.0f,  0.866025f, -1.0f, -1.0f },     // FRONT_L
    { 1.0f, -0.866025f,  0.0f, -1.0f },     // RIGHT
    { 1.0f,  0.866025f,  0.0f,  1.0f },     // LEFT
};

static const motorMixer_t mixerOctoX8[] = {
    { 1.0f, -1.0f,  1.0f, -1.0f },          // REAR_R
    { 1.0f, -1.0f, -1.0f,  1.0f },          // FRONT_R
    { 1.0f,  1.0f,  1.0f,  1.0f },          // REAR_L
    { 1.0f,  1.0f, -1.0f, -1.0f },          // FRONT_L
    { 1.0f, -1.0f,  1.0f,  1.0f },          // UNDER_REAR_R
    { 1.0f, -1.0f, -1.0f, -1.0f },          // UNDER_FRONT_R
    { 1.0f,  1.0f,  1.0f, -1.0f },          // UNDER_REAR_L
    { 1.0f,  1.0f, -1.0f,  1.0f },          // UNDER_FRONT_L
};

static const motorMixer_t mixerOctoFlatP[] = {
    { 1.0f,  0.707107f, -0.707107f,  1.0f },    // FRONT_L
    { 1.0f, -0.707107f, -0.707107f,  1.0f },    // FRONT_R
    { 1.0f, -0.707107f,  0.707107f,  1.0f },    // REAR_R
    { 1.0f,  0.707107f,  0.707107f,  1.0f },    // REAR_L
    { 1.0f,  0.0f, -1.0f, -1.0f },              // FRONT
    { 1.0f, -1.0f,  0.0f, -1.0f },              // RIGHT
    { 1.0f,  0.0f,  1.0f, -1.0f },              // REAR
    { 1.0f,  1.0f,  0.0f, -1.0f },              // LEFT
};

static const motorMixer_t mixerOctoFlatX[] = {
    { 1.0f,  1.0f, -0.5f,  1.0f },          // MIDFRONT_L
    { 1.0f, -0.5f, -1.0f,  1.0f },          // FRONT_R
    { 1.0f, -1.0f,  0.5f,  1.0f },          // MIDREAR_R
    { 1.0f,  0.5f,  1.0f,  1.0f },          // REAR_L
    { 1.0f,  0.5f, -1.0f, -1.0f },          // FRONT_L
    { 1.0f, -1.0f, -0.5f, -1.0f },          // MIDFRONT_R
    { 1.0f, -0.5f,  1.0f, -1.0f },          // REAR_R
    { 1.0f,  1.0f,  0.5f, -1.0f },          // MIDREAR_L
};

static const motorMixer_t mixerVtail4[] = {
    { 1.0f,  0.0f,  1.0f,  1.0f },          // REAR_R
    { 1.0f, -1.0f, -1.0f,  0.0f },          // FRONT_R
    { 1.0f,  0.0f,  1.0f, -1.0f },          // REAR_L
    { 1.0f,  1.0f, -1.0f, -0.0f },          // FRONT_L
};

/*
 motor[0] = PIDMIX(+5/4, -1,  +5/4); //FRONT_L
 motor[1] = PIDMIX(-1 ,   0,  +1 ); //MID_R
 motor[2] = PIDMIX(+3/4, +1,  +3/4); //REAR_L
 motor[3] = PIDMIX(+1 ,   0,  -1 ); //MID_L
 motor[4] = PIDMIX(-5/4, -1,  -5/4); //FRONT_R
 motor[5] = PIDMIX(-3/4, +1,  -3/4); //REAR_R
 */
static const motorMixer_t mixerHexV6[] = {
    { 1.0f,  1.25f,  1.0f,  1.25f },	// front l
    { 1.0f, -1.0f,   0.0f,  1.0 },		// mid r
    { 1.0f,  0.75f,  1.0f,  0.75f },	// REAR_L
    { 1.0f,  1.0f,   0.0f, -1.0f },		// mid_L
    { 1.0f,  1.25f, -1.0f, -1.25f },	// front r
    { 1.0f,  0.75f,  1.0f,  -0.75f },	// rear l
};

// Keep this synced with MultiType struct in mw.h!
const mixer_t mixers[] = {
//    Mo Se Mixtable
    { 0, 0, NULL },                // entry 0
    { 3, 1, mixerTri },            // MULTITYPE_TRI
    { 4, 0, mixerQuadP },          // MULTITYPE_QUADP
    { 4, 0, mixerQuadX },          // MULTITYPE_QUADX
    { 2, 1, mixerBi },             // MULTITYPE_BI
    { 0, 1, NULL },                // * MULTITYPE_GIMBAL
    { 6, 0, mixerY6 },             // MULTITYPE_Y6
    { 6, 0, mixerHex6P },          // MULTITYPE_HEX6
    { 1, 1, NULL },                // * MULTITYPE_FLYING_WING
    { 4, 0, mixerY4 },             // MULTITYPE_Y4
    { 6, 0, mixerHex6X },          // MULTITYPE_HEX6X
    { 8, 0, mixerOctoX8 },         // MULTITYPE_OCTOX8
    { 8, 0, mixerOctoFlatP },      // MULTITYPE_OCTOFLATP
    { 8, 0, mixerOctoFlatX },      // MULTITYPE_OCTOFLATX
    { 1, 1, NULL },                // * MULTITYPE_AIRPLANE
    { 0, 1, NULL },                // * MULTITYPE_HELI_120_CCPM
    { 0, 1, NULL },                // * MULTITYPE_HELI_90_DEG
    { 4, 0, mixerVtail4 },         // MULTITYPE_VTAIL4
    { 4, 0, mixerHexV6 },          // MULTITYPE_HEXV6
    { 0, 0, NULL },                // MULTITYPE_CUSTOM
};

void mixerInit(void)
{
    int i;

    // enable servos for mixes that require them. note, this shifts motor counts.
    useServo = mixers[cfg.mixerConfiguration].useServo;
    // if we want camstab/trig, that also enables servos, even if mixer doesn't
    if (getFeature(FEATURE_SERVO_TILT))
        useServo = 1;

    if (cfg.mixerConfiguration == MULTITYPE_CUSTOM) {
        // load custom mixer into currentMixer
        for (i = 0; i < MAX_MOTORS; i++) {
            // check if done
            if (cfg.customMixer[i].throttle == 0.0f)
                break;
            currentMixer[i] = cfg.customMixer[i];
            numberMotor++;
        }
    } else {
        numberMotor = mixers[cfg.mixerConfiguration].numberMotor;
        // copy motor-based mixers
        if (mixers[cfg.mixerConfiguration].motor) {
            for (i = 0; i < numberMotor; i++)
                currentMixer[i] = mixers[cfg.mixerConfiguration].motor[i];
        }
    }
}

void mixerLoadMix(int index)
{
    int i;

    // we're 1-based
    index++;
    // clear existing
    for (i = 0; i < MAX_MOTORS; i++)
        cfg.customMixer[i].throttle = 0.0f;

    // do we have anything here to begin with?
    if (mixers[index].motor != NULL) {
        for (i = 0; i < mixers[index].numberMotor; i++)
            cfg.customMixer[i] = mixers[index].motor[i];
    }
}

void writeServos(void)
{
    if (!useServo)
        return;

    switch (cfg.mixerConfiguration) {
        case MULTITYPE_BI:
            pwmWriteServo(0, servo[3]);
            pwmWriteServo(1, servo[4]);
            break;

        case MULTITYPE_TRI:
            pwmWriteServo(0, servo[4]);
            break;

        case MULTITYPE_AIRPLANE:
            pwmWriteServo(0, servo[3]);
            pwmWriteServo(1, servo[4]);
            pwmWriteServo(2, servo[5]);
            pwmWriteServo(3, servo[6]);
            break;

        case MULTITYPE_FLYING_WING:
        case MULTITYPE_GIMBAL:
            pwmWriteServo(0, servo[0]);
            pwmWriteServo(1, servo[1]);
            break;

        default:
            // Two servos for SERVO_TILT, if enabled
            if (getFeature(FEATURE_SERVO_TILT)) {
                pwmWriteServo(0, servo[0]);
                pwmWriteServo(1, servo[1]);
            }
            break;
    }
}

extern uint8_t cliMode;

void writeMotors(void)
{
    uint8_t i;

    for (i = 0; i < numberMotor; i++)
        pwmWriteMotor(i, motor[i]);
}

void writeAllMotors(int16_t mc)
{
    uint8_t i;

    // Sends commands to all motors
    for (i = 0; i < numberMotor; i++)
        motor[i] = mc;
    writeMotors();
}


///////////////////////////////////////////////////////////////////////////////
#define yawTransfer		0.4f
#define minVal			100.0f
#define maxVal			800.0f
#define rollTransfer	0.4f

float getScaledValue(float minScale, float maxScale, float valueToScale)
{
	// for low speed speeds we need more 'mix' as with high speed
	float scaler = (1.0f - valueToScale / (maxVal - minVal));
	return constrain(scaler, minScale, maxScale);
}

///////////////////////////////////////////////////////////////////////////////

void rudderAileronMixer(void)
{
	float deltaRoll = 0.0;
	float deltaYaw = 0.0;
float yawing = 0.0f;
float rolling = 0.0f;
float throttle = 0.0f;	
	// heading control (autopilot)
	// - add yaw to mantain the course, not only 'yaw'
  yawing = rcCommand[YAW];
	if (yawing != 0.0f)
	{
		// add a fraction to the roll command
		// todo: move to the top
		deltaRoll = yawTransfer * yawing;
	}
	// coordinated turns
	// - add rudder when the aileron has been commanded
  rolling = rcCommand[ROLL];
	if (rolling != 0.0f)
	{
		// rudder mixing depends on the true air speed
		// no sensor yet
	 throttle = rcCommand[THROTTLE];
		if (throttle != 0.0f)
		{

			deltaYaw = getScaledValue(minVal, maxVal, throttle);
		}
		else
		{
			// fixed transfer
			deltaYaw = rollTransfer * rolling;
		}
	}
	rcCommand[ROLL] += deltaRoll;
	rcCommand[YAW] += deltaYaw;
}

static void airplaneMixer(void)
{
	 motor[0] = rcCommand[THROTTLE];
	if (flightMode.PASSTHRU_MODE) {
		// do not use sensors for correction
		servo[3]  = cfg.roll_direction_l * (rcData[ROLL] - cfg.midrc);
		servo[4]  = cfg.roll_direction_r * (rcData[ROLL] - cfg.midrc);
	servo[5] = cfg.yaw_direction * (rcData[YAW] - cfg.midrc);
		servo[6] = cfg.pitch_direction * (rcData[PITCH] - cfg.midrc);
	} else {
		// use sensors to correct (gyro only or gyro + acc)
		servo[3]  = cfg.roll_direction_l * (axisPID[ROLL] + (rcData[ROLL] - cfg.midrc));
		servo[4]  = cfg.roll_direction_r * (axisPID[ROLL] + (rcData[ROLL] - cfg.midrc));
	servo[5] = cfg.yaw_direction * axisPID[YAW];
	servo[6] = cfg.pitch_direction * (axisPID[PITCH] + (rcData[PITCH] - cfg.midrc));
	}

	if (cfg.vtail == 1) { // V-Tail mixer enabled
		uint16_t tail_servo[2] = { 0, 0 };
		tail_servo[0] = servo[5];
		tail_servo[1] = servo[6];
	servo[5] = (tail_servo[1] + tail_servo[0]);
	servo[6] = (tail_servo[1] - tail_servo[0]);
	}

	if (cfg.flaperons == 1) { // Flaperon mix enabled
		servo[3] += rcData[cfg.flap_aux] - cfg.midrc;
		servo[4] += rcData[cfg.flap_aux] - cfg.midrc;
	}

	servo[3] = constrain(servo[3] + cfg.wing_left_mid, cfg.wing_left_min, cfg.wing_left_max);
	servo[4] = constrain(servo[4] + cfg.wing_right_mid, cfg.wing_right_min, cfg.wing_right_max);
	servo[5] = constrain(servo[5] + cfg.yaw_mid, cfg.yaw_min, cfg.yaw_max);
	servo[6] = constrain(servo[6] + cfg.pitch_mid, cfg.pitch_min, cfg.pitch_max);
}

void mixTable(void)
{
	int delta = 0;
	int16_t maxMotor;
	int16_t minMotor;
    uint32_t i;

    if (numberMotor > 3) {
        // prevent "yaw jump" during yaw correction
        axisPID[YAW] = constrain(axisPID[YAW], -100 - abs(rcCommand[YAW]), +100 + abs(rcCommand[YAW]));
    }

    // motors for non-servo mixes
	// TODO: reduce yaw command when it causes to exceed the max motor output
    if (numberMotor > 1)
        for (i = 0; i < numberMotor; i++)
            motor[i] = rcCommand[THROTTLE] * currentMixer[i].throttle + axisPID[PITCH] * currentMixer[i].pitch + axisPID[ROLL] * currentMixer[i].roll + cfg.yaw_direction * axisPID[YAW] * currentMixer[i].yaw;

    // airplane / servo mixes
    switch (cfg.mixerConfiguration) {
        case MULTITYPE_BI:
            servo[4] = constrain(1500 + (cfg.yaw_direction * axisPID[YAW]) + axisPID[PITCH], 1020, 2000);   //LEFT
            servo[5] = constrain(1500 + (cfg.yaw_direction * axisPID[YAW]) - axisPID[PITCH], 1020, 2000);   //RIGHT
            break;

        case MULTITYPE_TRI:
            servo[5] = constrain(cfg.tri_yaw_middle + cfg.yaw_direction * axisPID[YAW], cfg.tri_yaw_min, cfg.tri_yaw_max); //REAR
            break;

        case MULTITYPE_GIMBAL:
            servo[0] = constrain(cfg.gimbal_pitch_mid + cfg.gimbal_pitch_gain * angle[PITCH] / 16 + rcCommand[PITCH], cfg.gimbal_pitch_min, cfg.gimbal_pitch_max);
            servo[1] = constrain(cfg.gimbal_roll_mid + cfg.gimbal_roll_gain * angle[ROLL] / 16 + rcCommand[ROLL], cfg.gimbal_roll_min, cfg.gimbal_roll_max);
            break;

        case MULTITYPE_AIRPLANE:
            airplaneMixer();
            break;

        case MULTITYPE_FLYING_WING:
            motor[0] = rcCommand[THROTTLE];
            if (flightMode.PASSTHRU_MODE) {
                // do not use sensors for correction, simple 2 channel mixing
                servo[0]  = cfg.pitch_direction_l * (rcData[PITCH] - cfg.midrc) + cfg.roll_direction_l * (rcData[ROLL] - cfg.midrc);
                servo[1]  = cfg.pitch_direction_r * (rcData[PITCH] - cfg.midrc) + cfg.roll_direction_r * (rcData[ROLL] - cfg.midrc);
            } else {
                // use sensors to correct (gyro only or gyro + acc)
                servo[0]  = cfg.pitch_direction_l * axisPID[PITCH] + cfg.roll_direction_l * axisPID[ROLL];
                servo[1]  = cfg.pitch_direction_r * axisPID[PITCH] + cfg.roll_direction_r * axisPID[ROLL];
            }
            servo[0] = constrain(servo[0] + cfg.wing_left_mid, cfg.wing_left_min, cfg.wing_left_max);
            servo[1] = constrain(servo[1] + cfg.wing_right_mid, cfg.wing_right_min, cfg.wing_right_max);
            break;
    }

    // do camstab
    if (getFeature(FEATURE_SERVO_TILT)) {
        uint16_t aux[2] = { 0, 0 };

        if ((cfg.gimbal_flags & GIMBAL_NORMAL) || (cfg.gimbal_flags & GIMBAL_TILTONLY))
            aux[0] = rcData[AUX3] - cfg.midrc;
        if (!(cfg.gimbal_flags & GIMBAL_DISABLEAUX34))
            aux[1] = rcData[AUX4] - cfg.midrc;

        servo[0] = cfg.gimbal_pitch_mid + aux[0];
        servo[1] = cfg.gimbal_roll_mid + aux[1];

        if (rcOptions[BOXCAMSTAB]) {
            if (cfg.gimbal_flags & GIMBAL_MIXTILT) {
                servo[0] -= (-cfg.gimbal_pitch_gain) * angle[PITCH] / 16 - cfg.gimbal_roll_gain * angle[ROLL] / 16;
                servo[1] += (-cfg.gimbal_pitch_gain) * angle[PITCH] / 16 + cfg.gimbal_roll_gain * angle[ROLL] / 16;
            } else {
                servo[0] += cfg.gimbal_pitch_gain * angle[PITCH] / 16;
                servo[1] += cfg.gimbal_roll_gain * angle[ROLL]  / 16;
            }
        }

        servo[0] = constrain(servo[0], cfg.gimbal_pitch_min, cfg.gimbal_pitch_max);
        servo[1] = constrain(servo[1], cfg.gimbal_roll_min, cfg.gimbal_roll_max);
    }

    if (cfg.gimbal_flags & GIMBAL_FORWARDAUX) {
        int offset = 0;
        if (getFeature(FEATURE_SERVO_TILT))
            offset = 2;
        for (i = 0; i < 4; i++)
            pwmWriteServo(i + offset, rcData[AUX1 + i]);
    }

    maxMotor = motor[0];
    minMotor = motor[0];
    for (i = 1; i < numberMotor; i++){
        if (motor[i] > maxMotor)
            maxMotor = motor[i];
        if (motor[i] < minMotor)
        	minMotor = motor[i];
    }
    delta = 0;
    if (flightMode.ARMED) {
        // check for values out of bound
		if (maxMotor > cfg.maxthrottle)
			delta = maxMotor - cfg.maxthrottle;
		if (minMotor < cfg.minthrottle)
			delta = minMotor - cfg.minthrottle;
    }

    for (i = 0; i < numberMotor; i++) {
//        if (maxMotor > cfg.maxthrottle)     // this is a way to still have good gyro corrections if at least one motor reaches its max.
//            motor[i] -= maxMotor - cfg.maxthrottle;
    	// new
    	if (delta) {
    		motor[i] -= delta;
    	}
        motor[i] = constrain(motor[i], cfg.minthrottle, cfg.maxthrottle);
        if ((rcData[THROTTLE]) < cfg.mincheck) {
            if (!getFeature(FEATURE_MOTOR_STOP))
                motor[i] = cfg.minthrottle;
            else
                motor[i] = cfg.mincommand;
        }
        if (!flightMode.ARMED)
            motor[i] = cfg.mincommand;
    }
}
