#pragma once

// SenselHand will be created when there are five valid contacts
// Since then, all the contacts info of new frame will be treated as additive information to original five fingers

#include "sensel.h"
#include "sensel_device.h"
#include <string>
#include <iostream>

struct SenselFinger {
    int _id;
    float _pos_x;
    float _pos_y;
    float _total_force;
    float _area;
    SenselFinger() {}
    SenselFinger(int id, float pos_x, float pos_y, float total_force, float area) {
        _id = id;
        _pos_x = pos_x;
        _pos_y = pos_y;
        _total_force = total_force;
        _area = area;
    }
    SenselFinger& operator = (const SenselContact & rhs) {
        this->_area = rhs.area;
        this->_pos_x = rhs.x_pos;
        this->_pos_y = rhs.y_pos;
        this->_total_force = rhs.total_force;
        return *this;
    }
};

struct KeyEvent {
    std::string type;
    char key;
    KeyEvent() { type = "N"; }
    KeyEvent(std::string t, char k) { type = t; key = k; }
};

enum DEVICE_ORIEN { horizontal, vertical };

class SenselHand
{
public:
    SenselHand();
    ~SenselHand();

public:
    void init(int deviceid, SenselFrameData& curFrame);
    void reset(int deviceid, SenselFrameData& curFrame);
    void track(int deviceid, SenselFrameData& curFrame);
    void trackVersion1(int deviceid, SenselFrameData& curFrame);
    void trackVersion2(int deviceid, SenselFrameData& curFrame);
    std::string toString();
    std::string toString2();
    void setOrientation(DEVICE_ORIEN devOri);
    std::string outputCurrentAction();

private:
    // we need to support both vertical and horizontal version
    void sortFingers();

private:
    std::string _direction;
    SenselFinger _fingers[5];
    bool _isInit;
    const float kAreaThres = 22;
    const float kDisThres = 10;
    int _deviceID;
    const std::string kMapping = "q34tbnu90[";
    KeyEvent _curEvent;
    const float kForceDownThres = 70;
    const float kForceUpThres = 50;
    int _idleCount;
    const int kIdleTime = 10;
    const float kFingerArea = 500;
    DEVICE_ORIEN _deviceOrientation;
    std::string currentAction;
};

