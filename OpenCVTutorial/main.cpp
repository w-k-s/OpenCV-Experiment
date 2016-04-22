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
#include <sstream>
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
static const int kFontFace = 2;
static const int kFontScale = 1;
static const cv::Scalar kColorGreen = cv::Scalar(0,255,0);

static int hueMinimum = 0;
static int hueMaximum = 256;

static int saturationMinimum = 145;
static int saturationMaximum = 256;

static int valueMinimum = 156;
static int valueMaximum = 256;


using Contours = std::vector<cv::Point>;

std::string intToString(int number){
    std::stringstream ss;
    ss << number;
    return ss.str();
}

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


//Draws a target icon (A circle with a + in the center) at the given coordinates.
void drawTargetIconAtCoordinates(int x, int y,cv::Mat& cameraFeed){
    
    const int thickness = 2;
    cv::circle(cameraFeed,cv::Point(x,y),20,kColorGreen,2);
    
    if(y -25 >0 ){
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x,y-25),kColorGreen,thickness);
    }else{
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x,0),kColorGreen,thickness);
    }
    
    if(y+25<kFrameHeight){
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x,y+25),kColorGreen,thickness);
    }else{
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x,kFrameHeight),kColorGreen,thickness);
    }
    
    if(x-25>0){
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x-25,y),kColorGreen,thickness);
    }
    else{
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(0,y),kColorGreen, thickness);
    }
    
    if(x+25<kFrameWidth){
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(x+25,y),kColorGreen, thickness);
    }else{
        cv::line(cameraFeed,cv::Point(x,y),cv::Point(kFrameWidth,y),kColorGreen,thickness);
    }
    
    cv::putText(cameraFeed,intToString(x)+","+intToString(y),cv::Point(x,y+30),kFontFace,kFontScale,kColorGreen);
    
}

void trackObjects(const cv::Mat& trackMatrix,cv::Mat& cameraFeed, int& outX, int& outY){
    
    cv::Mat _trackMatrix;
    trackMatrix.copyTo(_trackMatrix);
    
    std::vector<Contours> contours;
    
    //Hierarchy is the relationship of objects within the image.
    //For example, an image could have 3 objects: 1, 2 and 3; where object 3 is nested inside 2.
    //In this case, object 1 and 2 are in the same hierarchy level while 3 is a child of 2 (and thus, 2 is the parent of 3.)
    //The 'hierarchy' vector below describes the hierarchy of each contour in the 'contours' vector above.
    //In other words, 'contours' and 'hierarchy' have the same length.
    //The 'hierarchy' vector has a type Vec4i (a vector of 4 ints). These ints are:
    // Int at Index 0: Index of next contour in the same level (in the 'contours' vector).
    // Int at Index 1: Index of previous contour in the same level (in the 'contours' vector).
    // Int at Index 2: Index of first-child contour (in the 'contours' vector)
    // Int at Index 3: Index of parent contour (in the 'contours' vector)
    std::vector<cv::Vec4i> hierarchy;
    
    //CV_RETR_CCOMP means arrange contours in a 2-level hierarchy The first level contains outer (white) objects. The second level contains all inner (black) objects.
    //CV_RETR_LIST means ignore hierarchy and only list all contours
    //CV_RETR_EXTERNAL means return Only the outer-most contours and ignore child contours.
    //CV_RETR_TREE means arrange contours in a proper (as-many-levels-as-necessary) hierarchy.
    //Reference: http://docs.opencv.org/3.1.0/d9/d8b/tutorial_py_contours_hierarchy.html#gsc.tab=0
    cv::findContours(_trackMatrix, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numberOfObjects = hierarchy.size();
        //If number of Objects > 10, the filter is probably noisy.
        if (numberOfObjects < 10) {
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                objectFound = false;
                //Use moments method to distinguish filtered object
                cv::Moments moment = cv::moments((cv::Mat) contours[index]);
                double area = moment.m00;
                
                //1. If the area is less than 20px x 20px, it's likelt just noise
                //2. If the area is 1.5 the size of the screen, It's likely to be caused by a bad filter.
                //3. refArea stores the largest valid area as a standard-of-reference for future iterations
                
                if (area > 400 && area<((kFrameWidth * kFrameWidth)/1.5) && area > refArea) {
                    printf("Object Found.\nArea: %f\n",area);
                    outX = moment.m10/area;
                    outY = moment.m01/area;
                    objectFound = true;
                }
            }
            
            if (objectFound) {
                cv::putText(cameraFeed, "Tracking Object", cv::Point(0,50), kFontFace, kFontScale, kColorGreen,false);
                drawTargetIconAtCoordinates(outX,outY,cameraFeed);
            }
        }
    }
    
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
        int x,y;
        trackObjects(hsvFilteredMatrix, rgbMatrix, x, y);
        
        cv::imshow(kFrameTitleHSVFiltered, hsvFilteredMatrix);
        cv::imshow(kFrameTitleRGB, rgbMatrix);
        cv::imshow(kFrameTitleHSV, hsvMatrix);
        
        cv::waitKey(30);
    }
    
}
