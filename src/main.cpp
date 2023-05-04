// main.cpp

#define GL_SILENCE_DEPRECATION

// Without this gl.h gets included instead of gl3.h
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <OpenGL/gl.h>
// #include "GL/glew.h"
// #include <GL/glew.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

// For includes related to OpenGL, make sure their are included after glfw3.h
#include <OpenGL/gl3.h>

#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "CGL/CGL.h"

using namespace std;
using namespace CGL;

/*
  GLOBAL VARIABLES
*/

cv::Mat cameraMatrix;
cv::Mat distCoeffs;
cv::Mat inverter;
cv::VideoCapture cap;
cv::aruco::ArucoDetector detector;

GLuint texture_background;

void initCV() {
    // float mtx_data[3][3] = {{9.77893519e+02, 0.00000000e+00, 6.58731620e+02},
                        // {0.00000000e+00, 1.04158258e+03, 3.13113827e+02},
                        // {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};

    // float mtx_data[3][3] = {{625.51823121,   0,         629.92259976,},
                        // {0,         456.00878219, 282.58074249,},
                        // {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};
    float mtx_data[3][3] = {{625.51823121,   0,         629.92259976,},
                        {0,         456.00878219, 282.58074249,},
                        {0.00000000e+00, 0.00000000e+00, 1.00000000e+00}};
    cameraMatrix = cv::Mat(3, 3, CV_32F, mtx_data);

    // float dist_data[1][5] =  {{-1.61180357e-01, 4.15458974e+00, -2.53542144e-02, 3.72820230e-03, -1.98594926e+01}};
    // float dist_data[1][5] =  {{-0.0064325,   0.79430384,  0.00730585, -0.0083165,  -1.53361965}};
    float dist_data[1][5] =  {{0.17423076, -0.43449858, -0.04052579,  0.03844378, -0.2205249}};
    distCoeffs = cv::Mat(1, 5, CV_32F, dist_data);

    float inv_data[4][4] = {{ 1.,  1.,  1.,  1.},
                            {-1., -1., -1., -1.},
                            {-1., -1., -1., -1.},
                            { 1.,  1.,  1.,  1.}};
    inverter = cv::Mat(4, 4, inv_data);
                        

    cap.open(0);
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000);
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    detector = cv::aruco::ArucoDetector(dictionary, detectorParams);
}

void initGL() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glCLearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // Set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,1,0.2,100000);

    glLightfv(GL_LIGHT0, GL_POSITION,  (-40, 300, 200, 0.0));
    glLightfv(GL_LIGHT0, GL_AMBIENT, (0.2, 0.2, 0.2, 1.0));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (0.5, 0.5, 0.5, 1.0));
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_TEXTURE_2D)
    glGenTextures(1, &texture_background);
}

cv::Mat getPoseFromImage(cv::Mat *frameIn) {

    if (*frameIn.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        return NULL;
    }

    cv::Mat frameOut;
    *frameIn.copyTo(frameOut);

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    detector.detectMarkers(*frameIn, corners, ids, rejected);

    if (ids.size() > 0) {
        cv::aruco::drawDetectedMarkers(frameOut, corners, ids);


        // Set coordinate system
        float squareWidth = 10.;
        float objPointData[4][3] = {{-squareWidth/2,  squareWidth/2, 0},
                                        {squareWidth/2,  squareWidth/2, 0},
                                        {squareWidth/2, -squareWidth,   0},
                                    {-squareWidth/2, -squareWidth/2, 0}};

        cv::Mat objPoints(4, 1, CV_32FC3, objPointData);

        // Calculate pose (we assume only one marker is visible)
        if (ids.size() > 0) {
            cv::Vec3d rvec = cv::Vec3d();
            cv::Vec3d tvec = cv::Vec3d();

            solvePnP(objPoints, corners.at(0), cameraMatrix, distCoeffs, rvec, tvec, cv::SOLVEPNP_IPPE_SQUARE);
            // cout << rvecs.at(i) << endl;
            // cout << tvecs.at(i) << endl;

            cv::drawFrameAxes(frameOut, cameraMatrix, distCoeffs, rvec, tvec, 0.1);

            cv::Mat rmat;
            cv::Rodrigues(rvec, rmat);

            float trans_data[4][4] = {{rmat[0][0], rmat[0][1], rmat[0][2], tvec[0]},
                                    {rmat[1][0], rmat[1][1], rmat[1][2], tvec[1]},
                                    {rmat[2][0], rmat[2][1], rmat[2][2], tvec[2]},
                                    {0., 0., 0., 1.},
            };
            cv::Mat transformation(4, 4, trans_data);

            *frameIn = frameOut;
            cv::imshow("cv frame", frameOut);
            cv::waitKey(1);

            return cv::transpose(transformation * inverter);

        } else {
            return NULL;
        }
    }
}

void update() {
    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cv::Mat image;

    cap.read(image);

    cv::Mat transformation = getPoseFromImage(image);

    // if (transformation != NULL) {
    //     glPushMatrix();

    //     glCallList(model.gl_list)
    //     glLoadMatrixd(transformation)

    //     glPopMatrix();
    // }

    // convert image to OpenGL texture format
    int shape[3] = image.shape();
    cv::Mat bg_image = cv::flip(image, 0);
    byte image_data[shape[0]][shape[1]][3];

    for (int i = 0; i < shape[0]; i++) {
        for (int j = 0; j < shape[1]; j++) {
            for (int c = 0; c < 3; c++) {
                image_data[i][j][c] = (byte) bg_image[i][j][c];
            }
        }
    }

    // create background texture
    glBindTexture(GL_TEXTURE_2D, self.texture_background);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, shape[1], shape[0], 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        
    // draw background
    glBindTexture(GL_TEXTURE_2D, texture_background);
    glPushMatrix();
    glTranslatef(0.0,0.0,-10.0);
    drawBackground();
    glPopMatrix();

    glutSwapBuffers();

    
}

void drawBackground() {
    // draw background
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0); glVertex3f(-4.0, -3.0, 0.0)
        glTexCoord2f(1.0, 1.0); glVertex3f( 4.0, -3.0, 0.0)
        glTexCoord2f(1.0, 0.0); glVertex3f( 4.0,  3.0, 0.0)
        glTexCoord2f(0.0, 0.0); glVertex3f(-4.0,  3.0, 0.0)
    glEnd();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);                 // Initialize GLUT
    glutInitDisplayMode(GLUT_RBGA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(640, 480);   // Set the window's initial width & height
    glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
    glutCreateWindow("[Group 74] CS 184 Final Project"); // Create a window with the given title
    glutDisplayFunc(update); // Register display callback handler for window re-paint
    glutIdleFunc(update);
    initCV();
    initGL();
    glutMainLoop();           // Enter the infinitely event-processing loop

    


// Configure ArUco detector
    cv::Mat markerImage;
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000);
    // cv::aruco::generateImageMarker(dictionary, 202, 200, markerImage, 1);
    // cv::imwrite("marker202.png", markerImage);

    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);
    
    // cv::VideoCapture cap;

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cv::Mat frameIn;

    bool paused = false;
    int ctr = 0;


    for (;;) {

        

        // show live and wait for a key with timeout long enough to show images
        if (!paused) {
            cv::imshow("Live", frameOut);
        }

        char key = (char)(cv::waitKey(1) & 0xFF);

        if (key == 'q') {
            cv::destroyAllWindows();
            break;
        } else if (key == 'p') {
            paused = !paused;
        } else if (key == 'c') {
            cv::imwrite("capture_" + std::to_string(ctr++) + ".jpg", frameOut);
        }
    }

    
    // glutTimerFunc(100, update, 0);


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

    

    return 0;
}

