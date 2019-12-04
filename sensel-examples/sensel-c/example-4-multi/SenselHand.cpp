#include "SenselHand.h"
#include <iostream>
#include <vector>

SenselHand::SenselHand()
{
    _isInit = false;
    _curEvent = KeyEvent();
    _idleCount = 0;
}


SenselHand::~SenselHand()
{
}

void SenselHand::init(int deviceid, SenselFrameData & curFrame)
{
    // treat only fingers as reasonable fingers
    if (!_isInit && curFrame.n_contacts >= 5) {
        int nFinger = 0;
        std::vector<int> indices;
        for (int i = 0; i < curFrame.n_contacts; i++) {
            if (curFrame.contacts[i].area > kAreaThres && curFrame.contacts[i].area < kFingerArea) {
                ++nFinger;
                indices.push_back(i);
            }                
        }
        // fill the field
        if (nFinger == 5) {
            int index = 0;
            for (int i : indices) {
                _fingers[index++] = SenselFinger(curFrame.contacts[i].id, curFrame.contacts[i].x_pos, curFrame.contacts[i].y_pos, curFrame.contacts[i].total_force, curFrame.contacts[i].area);
            }
            sortFingers();
            _deviceID = deviceid;
            _isInit = true;
        }
        
    }
}

void SenselHand::reset(int deviceid, SenselFrameData & curFrame)
{
    // if there is no pressure on the device for 10 frames
    // reset the fields    
    if (++_idleCount > kIdleTime) {
        // reset
        _isInit = false;
        _idleCount = 0;
        std::cout << "reset\n";
    }    
}

void SenselHand::track(int deviceid, SenselFrameData & curFrame)
{
    // check the device id first
    if (_isInit && deviceid == _deviceID) {
        if (curFrame.n_contacts == 0) {
            std::cout << "idle time: " << _idleCount << "\n";
            reset(deviceid, curFrame);
            return;
        }
        for (int i = 0; i < curFrame.n_contacts; i++) {
            SenselContact sc = curFrame.contacts[i];
            // go through the id first
            //
            //std::cout << "id" << _fingers[0]._id << "\n";
            //bool sameid = false;
            //for (int j = 0; j < 5; j++) {
            //    if (sc.id == _fingers[j]._id) {
            //        // they must be the same contact
            //        // update the fields directly
            //        _fingers[j] = sc;
            //        sameid = true;
            //        break;
            //    }
            //}
            //if (!sameid) {
                // failed to find the contact with the same id
                // use x and y pos
                for (int j = 0; j < 5; j++) {
                    float dis = pow((sc.x_pos - _fingers[j]._pos_x), 2) + pow(sc.y_pos - _fingers[j]._pos_y, 2);
                    //std::cout << "dis " << dis << "\n";
                    if (pow((sc.x_pos - _fingers[j]._pos_x),2) + pow(sc.y_pos-_fingers[j]._pos_y,2) < kDisThres) {
                        // they are supposed to be the same contact
                        // update the fields directly
                        _fingers[j] = sc;
                        break;
                    }
                }
            //}
        }
    }
    // print the current indice
    //for (int j = 0; j < 5; j++) {
    //    std::cout << "\t" << j << ":" << _fingers[j]._total_force;
    //}
}

std::string SenselHand::toString()
{
    if (!_isInit)
        return "";
    // check the largest one, usually we only support one key at a time. if it is larger than 100, then it is a key down event
    // if all of them reaches smaller than 50, that is a key up
    // if another key down event happens before a key up
    // we send two events at the same time
    KeyEvent prevEvent = _curEvent;
    _curEvent.type = "N";
    std::string str;
    
    int maxforce = _fingers[0]._total_force;
    int downindex = 0;
    for (int i = 1; i < 5; i++) {
        if (_fingers[i]._total_force > maxforce) {
            maxforce = _fingers[i]._total_force;
            downindex = i;
        }
    }
    if (maxforce > kForceDownThres) {
        _curEvent = KeyEvent("D", kMapping[_direction == "L" ? downindex : downindex + 5]);
    }
    else{
        _curEvent = KeyEvent("U", prevEvent.key);
    }
    
    // D D
    if (prevEvent.type == "D" && _curEvent.type == "D") {
        if (prevEvent.key != _curEvent.key) {
            str = "U " + std::string(1, prevEvent.key) + " D " + std::string(1, _curEvent.key);
            std::cout << str << "\n";
            str = std::string(1, _curEvent.key);
        }
    }
    else if (prevEvent.type == "D" && _curEvent.type == "U") {
        str = "U " + std::string(1, prevEvent.key);
        std::cout << str << "\n";
        str = "";
    }
    else if (prevEvent.type != "D" && _curEvent.type == "D") {
        str = "D " + std::string(1, _curEvent.key);
        std::cout << str << "\n";
        str = std::string(1, _curEvent.key);
    }
    return str;
}

void SenselHand::sortFingers()
{
    // figure out it is left hand or right hand
    // if the contact with lowest y is at rightmost, that is left hand
    // things changed if I put it into vertical way
    float maxy = _fingers[0]._pos_y;
    int thumbindex = 0;
    float sumx = _fingers[0]._pos_x;
    float thumbx = sumx;
    for (int i = 1; i < 5; i++) {
        if (_fingers[i]._pos_y > maxy) {
            maxy = _fingers[i]._pos_y;
            thumbindex = i;
            thumbx = _fingers[i]._pos_x;
        }
        sumx += _fingers[i]._pos_x;
    }
    sumx /= 5;
    if (thumbx < sumx) {
        //thumb is at left of the middle of palm
        _direction = "R";
    }
    else
        _direction = "L";

    // order the fingers from left to right
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (_fingers[i]._pos_x > _fingers[j]._pos_x)
                std::swap(_fingers[i], _fingers[j]);
        }
    }
}