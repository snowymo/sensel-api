#include "SenselOpenCV.h"

SenselOpenCV::SenselOpenCV()
{
    
}

SenselOpenCV::~SenselOpenCV()
{
}

void SenselOpenCV::addDevice(int width, int height, int num_cols, int num_rows)
{
    cv::Mat mat = cv::Mat::zeros(num_rows, num_cols, CV_32F);
    pressureMats.push_back(mat);
    cv::Mat mat2 = cv::Mat::zeros(height, width, CV_8U);
    contactMats.push_back(mat2);
}

void SenselOpenCV::drawPressure(int index, float * forces)
{
    memcpy_s(pressureMats[index].data, 
        pressureMats[index].cols * pressureMats[index].rows * sizeof(float), 
        forces, 
        pressureMats[index].cols * pressureMats[index].rows * sizeof(float));
    
}

void SenselOpenCV::drawContact(int index, SenselContact * senselContact)
{
    cv::circle(contactMats[index], cv::Point(senselContact->x_pos, senselContact->y_pos), sqrt(senselContact->area), cv::Scalar(255, 255, 255));
}

void SenselOpenCV::showImage()
{
    for (int i = 0; i < pressureMats.size(); i++) {
        cv::imshow("forces " + std::to_string(i), pressureMats[i]);
        cv::imshow("contacts " + std::to_string(i), contactMats[i]);        
    }        
    cv::waitKey(1);
}

void SenselOpenCV::resetImage()
{
    for (cv::Mat mat : pressureMats) {
        mat = cv::Mat::zeros(mat.size(), mat.type());
    }
    for (cv::Mat mat : contactMats) {
        mat = cv::Mat::zeros(mat.size(), mat.type());
    }
        
}
