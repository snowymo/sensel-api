#include "SenselHand.h"
#include <iostream>
#include <vector>

SenselHand::SenselHand()
{
    _isInit = false;
    _curEvent = KeyEvent();
    _idleCount = 0;
    _deviceOrientation = DEVICE_ORIEN::horizontal;
}


SenselHand::~SenselHand()
{
}

void SenselHand::init(int deviceid, SenselFrameData & curFrame)
{
    // treat only fingers as reasonable fingers
    if (!_isInit || curFrame.n_contacts >= 5) {
        int nFinger = 0;
        std::vector<int> indices;
        for (int i = 0; i < curFrame.n_contacts; i++) {
            
            if (curFrame.contacts[i].area > kAreaThres && curFrame.contacts[i].area < kFingerArea) {
                ++nFinger;
                indices.push_back(i);
            }
            else {
                std::cout << "area: " << curFrame.contacts[i].area << "\n";
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
        // first mode, we want to put the hands on the device all the time
        // but it is not comfortable
        //trackVersion1(deviceid, curFrame);
        //
        trackVersion2(deviceid, curFrame);
    }
    // print the current indice
    //for (int j = 0; j < 5; j++) {
    //    std::cout << "\t" << j << ":" << _fingers[j]._total_force;
    //}
}

void SenselHand::trackVersion1(int deviceid, SenselFrameData & curFrame)
{
    if (curFrame.n_contacts == 0) {
        std::cout << "idle time: " << _idleCount << "\n";
        reset(deviceid, curFrame);
        return;
    }
    for (int i = 0; i < curFrame.n_contacts; i++) {
        _idleCount = 0;
        SenselContact sc = curFrame.contacts[i];
        // go through the id first
        // failed to find the contact with the same id
        // use x and y pos
        for (int j = 0; j < 5; j++) {
            float dis = pow((sc.x_pos - _fingers[j]._pos_x), 2) + pow(sc.y_pos - _fingers[j]._pos_y, 2);
            //std::cout << "dis " << dis << "\n";
            if (pow((sc.x_pos - _fingers[j]._pos_x), 2) + pow(sc.y_pos - _fingers[j]._pos_y, 2) < kDisThres) {
                // they are supposed to be the same contact
                // update the fields directly
                _fingers[j] = sc;
                break;
            }
        }
    }
}

void SenselHand::trackVersion2(int deviceid, SenselFrameData & curFrame)
{
    // 0.1 only continue when there is one contact
    // 0.2 filter noise if there are more than 1 contacts
    if (curFrame.n_contacts == 10) {
        std::cout << "idle time: " << _idleCount << "\n";
        reset(deviceid, curFrame);
        return;
    }
    for (int j = 0; j < 5; j++) {
        _fingers[j]._total_force = 0;
    }
    if (curFrame.n_contacts == 1) {
        // figure out which finger it is
        SenselContact sc = curFrame.contacts[0];
        // go through the id first
        // failed to find the contact with the same id
        // use x and y pos
        float minDis = 9999;
        int fingerIndex = 0;
        for (int j = 0; j < 5; j++) {
            float dis = pow((sc.x_pos - _fingers[j]._pos_x), 2) + pow(sc.y_pos - _fingers[j]._pos_y, 2);
            //std::cout << "dis " << dis << "\n";
            if (dis < minDis) {
                // they are supposed to be the same contact
                // update the fields directly
                minDis = dis;
                fingerIndex = j;
            }
        }
        _fingers[fingerIndex] = sc;
        // send out press down
        //currentAction = kMapping[_direction == "L" ? fingerIndex : fingerIndex + 5];
    }
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

std::string SenselHand::toString2()
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
    if (maxforce > 0) {
        _curEvent = KeyEvent("D", kMapping[_direction == "L" ? downindex : downindex + 5]);
    }
    else {
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

void SenselHand::setOrientation(DEVICE_ORIEN devOri)
{
    _deviceOrientation = devOri;
}

std::string SenselHand::outputCurrentAction() { 
    if(currentAction != "")
        std::cout << currentAction << "\n";  
    std::string ret = currentAction;
    currentAction = "";
    return ret; 
}

void SenselHand::sortFingers()
{
    // figure out it is left hand or right hand
    // if the contact with lowest y is at rightmost, that is left hand
    // things changed if I put it into vertical way
    float maxy = _fingers[0]._pos_y, maxx = _fingers[0]._pos_x, minx = _fingers[0]._pos_x;
    int thumbindex = 0, maxindex = 0, minindex = 0;
    float sumx = maxx, sumy = maxy;
    float thumbx = sumx, thumby = sumy;

    switch (_deviceOrientation) {
    case DEVICE_ORIEN::horizontal:
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
        break;
    case DEVICE_ORIEN::vertical:
    {
        // check edge first: if average x is larget than width/2, edge on the left
        bool edgeLeft = false;
        for (int i = 1; i < 5; i++) {
            sumx += _fingers[i]._pos_x;
        }
        if ((sumx / 5) > (240 / 2)) {
            // edge on the left
            edgeLeft = true;
        }
        for (int i = 1; i < 5; i++) {
            // edge on the right: find the largest x, check if its y larger than average of y, if true then "L"
            // edge on the left: find the smallest x, check if its y larger than average of y, if true then "L"
            if (_fingers[i]._pos_x > maxx) {
                maxx = _fingers[i]._pos_x;
                maxindex = i;
            }
            if (_fingers[i]._pos_x < minx) {
                minx = _fingers[i]._pos_x;
                minindex = i;
            }
            sumy += _fingers[i]._pos_y;
        }
        sumy /= 5;
        if (edgeLeft && _fingers[minindex]._pos_y < sumy) {
            //thumb is at left of the middle of palm
            _direction = "R";
        }
        else if (edgeLeft && _fingers[minindex]._pos_y >= sumy) {
            //thumb is at left of the middle of palm
            _direction = "L";
        }
        else if (!edgeLeft && _fingers[maxindex]._pos_y > sumy) {
            _direction = "R";
        }
        else
            _direction = "L";
        std::cout << "direction:" << _direction << "\n";

        // order the fingers from left to right
        for (int i = 0; i < 5; i++) {
            for (int j = i + 1; j < 5; j++) {
                if (edgeLeft) {
                    if (_fingers[i]._pos_y > _fingers[j]._pos_y)
                        std::swap(_fingers[i], _fingers[j]);
                }
                else {
                    if (_fingers[i]._pos_y < _fingers[j]._pos_y)
                        std::swap(_fingers[i], _fingers[j]);
                }
            }
        }
        break;
    }
    default:
        break;
    }
}