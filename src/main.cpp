// main.cpp

#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace std;

int main(int argc, char** argv) {

    // if (argc != 2) {
        // cout << "Expecting a image file to be passed to program" << endl;
        // return -1;
    // }
    
    // cv::Mat img = cv::imread(argv[1]);
    
    // if (img.empty()) {
        // cout << "Not a valid image file" << endl;
        // return -1;
    // }
    
    // cv::namedWindow("Simple Demo", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Simple Demo", img);

    // Configure ArUco detector
    cv::Mat markerImage;
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000);
    cv::aruco::generateImageMarker(dictionary, 202, 200, markerImage, 1);
    cv::imwrite("marker202.png", markerImage);

    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);
    
    cv::VideoCapture cap;

    cap.open(0);

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cv::Mat frameIn;
    for (;;) {

        cap.read(frameIn);

        if (frameIn.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        cv::Mat frameOut;
        frameIn.copyTo(frameOut);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners, rejected;
        detector.detectMarkers(frameIn, corners, ids, rejected);

        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(frameOut, corners, ids);
        }

        // show live and wait for a key with timeout long enough to show images
        cv::imshow("Live", frameOut);

        int q = 'q';

        if ((cv::waitKey(1) & 0xFF) == q) {
            cv::destroyAllWindows();
            return 0;
        }
    }

    return 0;
}

