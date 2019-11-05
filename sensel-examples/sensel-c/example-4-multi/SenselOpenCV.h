#pragma once

#include<vector>

#include "sensel.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;

class SenselOpenCV
{
public:
    SenselOpenCV();
    ~SenselOpenCV();

public:
    void addDevice(int width = 640, int height = 480);
    void drawPressure(int index, float* forces);
    void drawContact(int index, SenselContact* senselContact);
    void showImage();
    void resetImage();
private:
    std::vector<cv::Mat> pressureMats;
};

