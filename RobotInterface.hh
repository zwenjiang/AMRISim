/* 

  Copyright (C) 2005, ActivMedia Robotics, LLC
  Copyright (C) 2006-2010 MobileRobots, Inc.
  Copyright (C) 2011-2015 Adept Technology
  Copyright (C) 2016-2017 Omron Adept Technologies

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#ifndef EP_ROBOT_INTERFACE_HH_
#define EP_ROBOT_INTERFACE_HH_

#include "MobileSim.hh"
#include "MapLoader.hh"

#include <string>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <functional>
#include <assert.h>

#include "ArFunctor.h"

#include "ArFunctor.h"

#include "ArRobotPacket.h"


// MS Visual C++ does not have vsnprintf, but it does have _vsnprintf. Gee.
#ifdef _MSC_VER
#define vsnprintf(buf, sz, fmt, args)  _vsnprintf(buf, sz, fmt, args)
#endif

class RobotParams;

/** Abstracts a robot interface, such as Player or Stage, for EmulatePioneer to
 * use.
 *  Subclasses implement this interface for a particular robot or simulator.
 *  In general, if a method in this class is not pure virtual, and you override
 *  it, you should also call the method in this class.
 *  The purpose of this class is to collect all of a target robot or simulator's
 *  objects and functions into a single unified set of methods specifically for
 *  EmulatePioneer's needs.
 */
class RobotInterface : public LogInterface {
  private:
    bool requestedOpenSonar;
    std::string robotName;
    float batteryVoltage;
    bool tempWarning;
    bool haveTemp;
    double temperature;
    float stateOfCharge;
  public:
    RobotInterface(std::string init_robotName) :
      LogInterface(init_robotName),
      requestedOpenSonar(false), robotName(init_robotName), batteryVoltage(0.0),
      tempWarning(false), haveTemp(false), temperature(0.0), stateOfCharge(0.0)
    {
    }

    virtual ~RobotInterface() {
      //removeStoredRobotInterface(this);
    }
 
    std::string getRobotName() const { return robotName; }

    /** Use the given params struct. The implementation of this function might
     change them based on values read for this robot type from the simulator.
     You should call this before accepting a connection with a client, as this function
     may connect to the robot/sim and retrieve configuration from the robot/sim 
     you will need for the client session.
    */
    virtual void connect(RobotParams* params) = 0;

    /// Cleanup from connect(), and also clear any session state such as odometry etc.
    virtual void disconnect() = 0;


    // commands:
    virtual void enableMotors(bool e) = 0;
    virtual void transVel(int v) = 0;  ///< mm/s
    virtual void latVel(int v) = 0;   ///< mm/s for omnidirectionals only
    virtual void rotVel(int v) = 0;    ///< deg/s
    virtual void move(int m) = 0;      ///< mm
    virtual void heading(int h) = 0;   ///< deg
    virtual void deltaHeading(int h) = 0; ///< deg
    virtual void stop() = 0;
    virtual void stop(int /*transDecel*/, int /*rotDecel*/) { stop(); }
    virtual void setAccel(int a) = 0;   ///< mm/s/s
    virtual void setDecel(int d) = 0;   ///< mm/s/s
    virtual void setRotAccel(int a) = 0;///< deg/s/s
    virtual void setRotDecel(int d) = 0;///< deg/s/s
    virtual void setLatAccel(int a) = 0;
    virtual void setLatDecel(int d) = 0;
    virtual void setMaxVel(int v) = 0 ; ///< mm/s
    virtual void setMaxRotVel(int v) = 0;///< mm/s
    virtual void setDefaultVel(int v) = 0; ///< m/s for move()
    virtual void setDefaultRotVel(int v) = 0; ///< deg/s for heading() and deltaHeading()
    virtual void setOdom(int x, int y, int theta) = 0;
    virtual void setSimulatorPose(long int x, long int y, long int z, int theta) = 0;
    virtual void resetSimulatorPose() = 0;

    /// Try to open the sonar
    virtual void openSonar() { 
      requestedOpenSonar = true;
    }

    /// Close the sonar if open
    virtual void closeSonar() {
      requestedOpenSonar = false;
    } 

    /** Whether sonar was requested to open with openSonar() (regardless of
     * whether it was successfully opened or not)
     */
    virtual bool sonarOpenRequested() {
      return requestedOpenSonar;
    }

    /// Whether sonar is open and (potentially) generating data. 
    virtual bool sonarOpen() = 0;

    virtual void openLaser(size_t i = 0) = 0;
    virtual void closeLaser(size_t i = 0) = 0;
    virtual bool laserOpen(size_t i = 0) = 0;
    virtual void stall(bool stalled) = 0;



    // data:
    virtual void getMotionState(int &x, int &y, int &theta, int &transVel, int &rotVel, bool &stallflag, bool &motorsEnabled) = 0;
    virtual void getPosition(int &x, int &y, int &theta) = 0;
    virtual void getVelocity(int &x, int &y, int &theta) = 0;
    virtual bool havePositionData() { return false; }
    virtual bool haveGripper()  { return false; }
    virtual bool haveFrontSonar() { return false; }
    virtual bool haveRearSonar() { return false; }
    virtual bool haveSonar() { return haveFrontSonar() || haveRearSonar(); }
    virtual bool haveLaser(size_t /*i*/) { return false; }
    virtual size_t numLasers() { return 0; }
    virtual int xpos() = 0;   ///< odometric x
    virtual int ypos() = 0;   ///< odometric y
    virtual int theta() = 0;  ///< odometric theta
    virtual int xspeed() = 0;
    virtual int rotspeed() = 0;
    virtual int yspeed() = 0;
    virtual long getSimulatorPoseX() = 0;   ///< true x
    virtual long getSimulatorPoseY() = 0;   ///< true y
    virtual int getSimulatorPoseTheta() = 0;  ///< true theta
    virtual long getSimulatorPoseZ() { return 0; }
    virtual void getSimulatorPose(long &x, long &y, long &z, int &theta) = 0;
    virtual float getBatteryVoltage() { return batteryVoltage; }
    virtual void setBatteryVoltage(float val) { batteryVoltage = val; }
    virtual bool getTempWarning() { return tempWarning; }
    virtual void setTempWarning(bool val) { tempWarning = val; }
    virtual bool haveTemperature() { return haveTemp; }
    virtual void setHaveTemperature(bool val) { haveTemp = val; }
    virtual double getTemperature() { return temperature; }
    virtual void setTemperature(double temp) { temperature = temp; haveTemp = true; }
    virtual bool stalled() = 0;
    virtual char gripperState() = 0;
    virtual void updateStateOfCharge() = 0;
    virtual float getStateOfCharge() { return stateOfCharge; }
    virtual void setStateOfCharge(float val) { stateOfCharge = val; }
    virtual bool haveStateOfCharge() { return false; }
    virtual float getSimGPSDOP() { return 1.0; }
    virtual double getSimulatorOdomErrorX() { return 0.0; }
    virtual double getSimulatorOdomErrorY() { return 0.0; }
    virtual double getSimulatorOdomErrorTh() { return 0.0; }
    virtual bool haveSimulatorOdomError() { return false; }

    virtual size_t numSonarReadings() = 0;
    virtual int getSonarReading(int i) = 0;

    class SonarReadingFunc : public std::unary_function<int, bool>
    {
    public:
      virtual bool operator()(int r) = 0;
      virtual ~SonarReadingFunc(){}
    };

    /** Locks the sonar if neccesary, and applies @a func for each sonar
     * reading, starting with reading number @a start.
     *  If @a func returns false, stop iterating over sonar readings.
     *  @return number of sonar readings @a func was applied to.
     *
     *  Example of a class you can use for @a func:
     *  @code
     *  class PrintReadingFunc : public SonarReadingFu {
     *  public:
     *    virtual bool operator()(int r) {
     *      std::cout << "Sonar reading: " << r << std::endl;
     *    }
     *  };
     *  @endcode
     *
     *  @todo maybe the functor should be binary and accept (i, range)
     *  @todo define custom iterators instead?
     */
    virtual size_t forEachSonarReading(SonarReadingFunc &func, const size_t &start = 0) = 0;

    virtual size_t numLaserReadings(size_t lasernum) = 0;
    virtual int getLaserReading(size_t lasernum, int i) = 0;
    virtual int getLaserReflectance(size_t lasernum, int i) = 0;

    class LaserReadingFunc : public std::binary_function<int, int, bool>
    {
    public:
      virtual bool operator()(int range, int ref) = 0;
      virtual ~LaserReadingFunc() {}
    };

    /** Locks the laser if neccesary, and applies @a func for each laser reading and reflectance value/flags, starting with reading number @a start.
     * If @a func returns false, stop iterating over readings.
     * @return number of laser readings @a func was applied to.
     *
     *  Example of a class you can use for @a func:
     *  @code
     *  class PrintReadingFunc : binary_function<int, int, bool> {
     *  public:
     *    bool operator()(int range, int ref) {
     *      std::cout << "Laser reading: " << range << " (" << ref << ")" << std::endl;
     *    }
     *  };
     *  @endcode
     *
     *  @todo maybe the functor should be tertiary and accept (i, range, ref),
     *  or binary and accept (i, struct {range, ref}).
     *  @todo define custom iterators instead?
     */
    virtual size_t forEachLaserReading(size_t lasernum, LaserReadingFunc &func, const size_t &start = 0) = 0;

    virtual double getLaserResolution(size_t lasernum) = 0;  ///< Degrees between readings
    virtual void setLaserResolution(size_t lasernum, double deg) = 0;  ///< Degrees between readings
    virtual double getLaserFOV(size_t lasernum) = 0; ///< Total laser FOV
    virtual void setLaserFOV(size_t lasernum, double deg) = 0; ///< Total laser FOV
    virtual double getLaserStartAngle(size_t lasernum) = 0;  ///< Angle of first reading relative to robot
    virtual double getLaserEndAngle(size_t lasernum) = 0;  ///< Angle of first reading relative to robot
    virtual void setLaserAngles(size_t lasernum, double first, double last) = 0; ///< Angle of first and last readings relative to robot
    virtual void shutdown(int errorcode = 0) = 0;
    virtual int getLastInterval() = 0; ///< How much real time the last simulation loop took
    virtual int getSimInterval() = 0; ///< How much simulated time each update loop should take
    virtual int getRealInterval() = 0; ///< How much real time each update loop ought to take (if possible)
    


    /** Stores identifying information about a device. */
    class DeviceInfo {
    public:
      std::string name;
      std::string type;
      std::string basetype;
      unsigned int which;
      unsigned char status;
    };

    /** Get a list of device names (with index for multiple devices of the same 
        kind). If robot doesn't know about it's devices, an empty list may be
        returned.
    */
    virtual std::vector<DeviceInfo> getDeviceInfo()
    {
      std::vector<DeviceInfo> empty;
      return empty;
    }

    virtual std::string laserType(size_t i) = 0;
    virtual std::string laserName(size_t i) = 0;

    virtual void logState()
    {
      int x, y, th, tv, rv;
      bool stall, motorsEnabled;
      getMotionState(x, y, th, tv, rv, stall, motorsEnabled);
      log("Times: last=%d, sim=%d, real=%d", getLastInterval(), getSimInterval(), getRealInterval());
      log("State config: requestedOpenSonar=%d, sonarOpen=%d, laserOpen=%d, motorsEnabled=%d, stalled=%d", 
              sonarOpenRequested(), sonarOpen(), laserOpen(), motorsEnabled, stall);
      size_t numlasers = numLasers();
      log(" havePosition=%d, haveGripper=%d, haveFrontSonar=%d, haveRearSonar=%d, haveSonar=%d, numSonarReadings=%d, numLasers=%u",
        havePositionData(), haveGripper(), haveFrontSonar(), haveRearSonar(), haveSonar(), numSonarReadings(), numlasers);
      for(size_t i = 0; i < numlasers; ++i)
      {
        log(" laser %u: name=%s, type=%s, numLaserReadings=%u, laserRes=%0.3f, laserFOV=%0.3f, laserStart=%0.3f, laserEnd=%0.3f",
          i, laserName(i).c_str(), laserType(i).c_str(), numLaserReadings(i), getLaserResolution(i), getLaserFOV(i), getLaserStartAngle(i), getLaserEndAngle(i));
      }
      log(" battery=%0.2f, tempWarning=%d, haveTemp=%d, temperature=%0.2f", getBatteryVoltage(), getTempWarning(), haveTemperature(), getTemperature());
      log(" map=%s", getMapName().c_str());
    }


    /// Set whether this robot can be seen by other robots' sensors (laser,
    //sonar), if possible
  virtual void setInvisible(bool s) = 0;

    /// Set whether other robots can collide with this robot, if possible
  virtual void setEphemeral(bool s) = 0;

  unsigned char digoutState;
  unsigned char diginState;
  virtual void setDigoutState(unsigned char state)
  {
    digoutState = state;
  }
  virtual void setDiginState(unsigned char state)
  {
    diginState = state;
  }
  virtual unsigned char getDigoutState()
  {
    return digoutState;
  }
  virtual unsigned char getDiginState()
  {
    return diginState;
  }
};

#endif
