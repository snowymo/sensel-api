#include "SenselOpenCV.h"

SenselOpenCV::SenselOpenCV()
{
    
}

SenselOpenCV::~SenselOpenCV()
{
}

void SenselOpenCV::addDevice(int width, int height)
{
    cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC1);
    pressureMats.push_back(mat);
}

void SenselOpenCV::drawPressure(int index, float * forces)
{
    memcpy_s(pressureMats[index].data, pressureMats[index].cols * pressureMats[index].rows * sizeof(float), forces, pressureMats[index].cols * pressureMats[index].rows * sizeof(float));
    
}

void SenselOpenCV::drawContact(int index, SenselContact * senselContact)
{
    cv::circle(pressureMats[index], cv::Point(senselContact->x_pos, senselContact->y_pos), sqrt(senselContact->area), cv::Scalar(1, 255, 255));
}

void SenselOpenCV::showImage()
{
    for (int i = 0; i < pressureMats.size(); i++)
        cv::imshow("forces " + std::to_string(i), pressureMats[i]);
    cv::waitKey(1);
}

void SenselOpenCV::resetImage()
{
    for(cv::Mat mat : pressureMats)
        mat = cv::Mat::zeros(mat.size(), mat.type());
}
