// main.cpp

#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace std;

/*
  Intrinsic camera parameters
*/

// float mtx_data[3][3] = {{9.77893519e+02, 0.00000000e+00, 6.58731620e+02},
                    // {0.00000000e+00, 1.04158258e+03, 3.13113827e+02},
                    // {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};

// float mtx_data[3][3] = {{625.51823121,   0,         629.92259976,},
                    // {0,         456.00878219, 282.58074249,},
                    // {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};

float mtx_data[3][3] = {{625.51823121,   0,         629.92259976,},
                    {0,         456.00878219, 282.58074249,},
                    {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};

cv::Mat cameraMatrix(3, 3, CV_32F, mtx_data);

// float dist_data[1][5] =  {{-1.61180357e-01, 4.15458974e+00, -2.53542144e-02, 3.72820230e-03, -1.98594926e+01}};
// float dist_data[1][5] =  {{-0.0064325,   0.79430384,  0.00730585, -0.0083165,  -1.53361965}};
float dist_data[1][5] =  {{0.17423076, -0.43449858, -0.04052579,  0.03844378, -0.2205249}};

cv::Mat distCoeffs(1, 5, CV_32F, dist_data);

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

    bool paused = false;
    int ctr = 0;
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

            std::vector<cv::Vec3d> rvecs, tvecs;

            

            // Set coordinate system
            float objPointData[4][3] = {{ -0.5,  0.5, 0},
                                        {0.5,  0.5, 0},
                                        {0.5, -0.5, 0},
                                       {-0.5, -0.5, 0}};

            cv::Mat objPoints(4, 1, CV_32FC3, objPointData);

            cout << objPoints << endl;


            // Calculate pose for each marker
            for (int i = 0; i < ids.size(); i++) {
                rvecs.push_back(cv::Vec3d());
                tvecs.push_back(cv::Vec3d());
                solvePnP(objPoints, corners.at(i), cameraMatrix, distCoeffs, rvecs.at(i), tvecs.at(i), cv::SOLVEPNP_IPPE_SQUARE);
                cout << rvecs.at(i) << endl;
                cout << tvecs.at(i) << endl;

                for (int i = 0; i < rvecs.size(); ++i) {
                    auto rvec = rvecs[i];
                    auto tvec = tvecs[i];
                    cv::drawFrameAxes(frameOut, cameraMatrix, distCoeffs, rvec, tvec, 0.1);
                }
            }

        }

        // show live and wait for a key with timeout long enough to show images
        if (!paused) {
            cv::imshow("Live", frameOut);
        }

        char key = (char)(cv::waitKey(1) & 0xFF);

        if (key == 'q') {
            cv::destroyAllWindows();
            return 0;
        } else if (key == 'p') {
            paused = !paused;
        } else if (key == 'c') {
            cv::imwrite("capture_" + std::to_string(ctr++) + ".jpg", frameOut);
        }
    }

    return 0;
}
