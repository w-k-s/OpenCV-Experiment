//
//  main.cpp
//  OpenCVTutorial
//
//  Created by Waqqas Sheikh on 23/10/2015.
//  Copyright (c) 2015 asfour. All rights reserved.
//
// The following example is based on the source code hosted at:
// https://raw.githubusercontent.com/kylehounslow/opencv-tuts/master/object-tracking-tut/objectTrackingTut.cpp (retrieved on 24th October 2015).
//
// The source code goes along with Kyle Hounslow YouTube tutorial:
// https://www.youtube.com/watch?v=bSeFrPrqZ2A (retrieved on 24th October 2015)
//
// The code has been copied by hand in order to familiarise myself with the OpenCV library and its concepts.
// Methods and variables have been renamed according to my preference.
// Certain parts of the original code seemed redundant and have been removed from this version.

#include <iostream>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

static const std::string kFrameTitleRGB = "RGB Window";
static const std::string kFrameTitleHSV = "HSV Window";
static const std::string kFrameTitleHSVFiltered = "Filtered HSV Window";
static const std::string kFrameTitleHSVFilterControls = "HSV Filter Controls";

static const int kIDWebcam = 0;
static const int kFrameWidth = 600;
static const int kFrameHeight = 400;

static int hueMinimum = 0;
static int hueMaximum = 256;

static int saturationMinimum = 145;
static int saturationMaximum = 256;

static int valueMinimum = 156;
static int valueMaximum = 256;

void on_trackbar( int, void* )
{
    //This function gets called whenever a
    // trackbar position is changed

}

void createTrackers(){
    
    cv::namedWindow(kFrameTitleHSVFilterControls,0);
    
    cv::createTrackbar("H Min", kFrameTitleHSVFilterControls, &hueMinimum, hueMaximum, on_trackbar);
    cv::createTrackbar("H Max", kFrameTitleHSVFilterControls, &hueMaximum, hueMaximum, on_trackbar);
    
    cv::createTrackbar("S MIN", kFrameTitleHSVFilterControls, &saturationMinimum, saturationMaximum, on_trackbar);
    cv::createTrackbar("S MAX", kFrameTitleHSVFilterControls, &saturationMaximum, saturationMaximum, on_trackbar);
    
    cv::createTrackbar("V MIN", kFrameTitleHSVFilterControls, &valueMinimum, valueMaximum, on_trackbar);
    cv::createTrackbar("V MAX", kFrameTitleHSVFilterControls, &valueMaximum, valueMaximum, on_trackbar);
    
}

void executeMorphologicalOperations(cv::Mat& inputOutputMatrix){
    
    //Create structuring element that will be used to "dilate" and "erode" image.
    //The element chosen here is a 3px by 3px rectangle. Anything smaller than this will be eroded.
    //Using a rectanglular shape is less computationally expensive compared to other shapes.
    cv::Mat erodeMatrix = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    
    //Dilate elements that are larger than 8px x 8px.
    cv::Mat dilateMatrix = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8,8));
    
    //Erode and Dilate can be called multiple times if necessary (TODO: Check Why? Seems Strange!)
    cv::erode(/* IN */ inputOutputMatrix, /* OUT */ inputOutputMatrix, erodeMatrix);
    cv::dilate(/* IN */ inputOutputMatrix, /* OUT */ inputOutputMatrix, dilateMatrix);
}

int main(int argc, const char * argv[]) {
    
    createTrackers();
    
    cv::VideoCapture videoCapture{kIDWebcam};
    
    videoCapture.open(kIDWebcam);
    videoCapture.set(CV_CAP_PROP_FRAME_WIDTH, kFrameWidth);
    videoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, kFrameHeight);
    
    cv::Mat rgbMatrix;
    cv::Mat hsvMatrix;
    cv::Mat hsvFilteredMatrix;

    while (true) {
        videoCapture.read(rgbMatrix);
        cv::cvtColor(rgbMatrix, hsvMatrix, cv::COLOR_BGR2HSV);
        
        cv::inRange(hsvMatrix,
                    cv::Scalar(hueMinimum,saturationMinimum,valueMinimum),
                    cv::Scalar(hueMaximum,saturationMaximum,valueMaximum),
                    hsvFilteredMatrix);
        
        //std::cout<<"H: "<<hueMinimum<<" S: "<<saturationMinimum<<" V: "<<valueMinimum;
        //std::cout<<" - ";
        //std::cout<<"H: "<<hueMaximum<<" S: "<<saturationMaximum<<" V: "<<valueMaximum<<"\n";
        
        executeMorphologicalOperations(hsvFilteredMatrix);
        
        cv::imshow(kFrameTitleHSVFiltered, hsvFilteredMatrix);
        cv::imshow(kFrameTitleRGB, rgbMatrix);
        cv::imshow(kFrameTitleHSV, hsvMatrix);
        
        cv::waitKey(30);
    }
    
}
