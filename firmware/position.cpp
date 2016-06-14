/*
   position.hpp : Position class header

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __arm__
extern "C" {
#endif

#include "mw.hpp"

void Position::init(Baro * _baro, IMU * _imu)
{
    this->baro = _baro;
    this->imu  = _imu;

    this->previousTime = 0;
    this->accZ_old = 0;
    this->accelVel = 0;
    this->FusedBarosonarAlt = 0;
    this->FusedBarosonarAlt = 0;
    this->baroAlt = 0;
    this->baroAltBaseline = 0;
    this->accelAlt = 0;
    this->wasArmed = 0;
    this->baroAlt_offset = 0;
    this->sonarTransition = 0;
}

void Position::getAltitude(bool armed, uint32_t currentTime, int32_t & estAltOut)
{
    /*
    uint32_t dTime = currentTime - this->previousTime;

    this->previousTime = currentTime;

    // Avoid estimating altitude too often
    if (dTime < CONFIG_ACCEL_UPDATE_PERIOD_USEC) 
        return;

    // Calculates height from ground in cm via baro pressure
    // See: https://github.com/diydrones/ardupilot/blob/master/libraries/AP_Baro/AP_Baro.cpp#L140
    int32_t this->baroAltRaw = lrintf((1.0f - powf((float)(baroPressureSum / (CONFIG_BARO_TAB_SIZE - 1)) 
    / 101325.0f, 0.190295f)) * 4433000.0f);

    // Grab baro baseline on arming
    if (armed) {
    if (!this->wasArmed) {
    this->baroAltBaseline = this->baroAltRaw;
    this->accelVel = 0;
    this->accelAlt = 0;
    }
    this->baroAlt = this->baroAltRaw - this->baroAltBaseline;
    }
    else {
    this->baroAlt = 0;
    }
    this->wasArmed = armed;

    // Calculate sonar altitude only if the sonar is facing downwards(<25deg)
    int16_t tiltAngle = max(abs(this->imu->angle[ROLL]), abs(this->imu->angle[PITCH]));

    sonarAlt = (tiltAngle > 250) ? -1 : sonarAlt * (900.0f - tiltAngle) / 900.0f;

    // Fuse sonarAlt and this->baroAlt
    if (sonarInRange()) {
    this->baroAlt_offset = this->baroAlt - sonarAlt;
    this->FusedBarosonarAlt = sonarAlt;
    } else {
    this->baroAlt = this->baroAlt - this->baroAlt_offset;
    if (sonarAlt > 0) {
    this->sonarTransition = (300 - sonarAlt) / 100.0f;
    this->FusedBarosonarAlt = cfilter(sonarAlt, this->baroAlt, this->sonarTransition); 
    }
    }

    // delta acc reading time in seconds
    float dt = accTimeSum * 1e-6f; 

    // Integrator - velocity, cm/sec
    float accZ_tmp = (float)accSum[2] / (float)accSumCount;
    float vel_acc = accZ_tmp * accVelScale * (float)accTimeSum;

    // integrate accelerometer velocity to get distance (x= a/2 * t^2)
    this->accelAlt += (vel_acc * 0.5f) * dt + this->accelVel * dt;                                         
    this->accelVel += vel_acc;

    // complementary filter for altitude estimation (baro & acc)
    //this->accelAlt = cfilter(this->accelAlt, this->FusedBarosonarAlt, CONFIG_BARO_CF_ALT);

    estAlt = sonarInRange() ? this->FusedBarosonarAlt : this->accelAlt;

    // reset acceleromter sum
    accSum[0] = 0;
    accSum[1] = 0;
    accSum[2] = 0;
    accSumCount = 0;
    accTimeSum = 0;

    int32_t fusedBaroSonarVel = (this->FusedBarosonarAlt - lastthis->FusedBarosonarAlt) * 1000000.0f / dTime;
    lastthis->FusedBarosonarAlt = this->FusedBarosonarAlt;

    fusedBaroSonarVel = constrain(fusedBaroSonarVel, -1500, 1500);    // constrain baro velocity +/- 1500cm/s
    fusedBaroSonarVel = applyDeadband(fusedBaroSonarVel, 10);         // to reduce noise near zero

    // Apply complementary filter to keep the calculated velocity based on baro velocity (i.e. near real velocity).
    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, 
    // i.e without delay
    this->accelVel = cfilter(this->accelVel, fusedBaroSonarVel, CONFIG_BARO_CF_VEL);
    int32_t vel_tmp = lrintf(this->accelVel);

    // set vario
    vario = applyDeadband(vel_tmp, 5);

    if (tiltAngle < 800) { // only calculate pid if the copters thrust is facing downwards(<80deg)

        int32_t setVel = setVelocity;

        // Altitude P-Controller
        if (!velocityControl) {
            int32_t error = constrain(altHold - estAlt, -500, 500);
            error = applyDeadband(error, 10);       // remove small P parametr to reduce noise near zero position
            setVel = constrain((CONFIG_ALT_P * error / 128), -300, +300); // limit velocity to +/- 3 m/s
        } 

        // Velocity PID-Controller
        // P
        int32_t error = setVel - vel_tmp;
        altPID = constrain((CONFIG_VEL_P * error / 32), -300, +300);

        // I
        errorVelocityI += (CONFIG_VEL_I * error);
        errorVelocityI = constrain(errorVelocityI, -(8196 * 200), (8196 * 200));
        altPID += errorVelocityI / 8196;     // I in the range of +/-200

        // D
        altPID -= constrain(CONFIG_VEL_D * (accZ_tmp + this->accZ_old) / 512, -150, 150);

    } else {
        altPID = 0;
    }

    this->accZ_old = accZ_tmp;

    estAltOut = estAlt;
    */
}

#ifdef __arm__
} // extern "C"
#endif