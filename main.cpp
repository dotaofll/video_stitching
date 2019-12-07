#include  <opencv2/opencv.hpp>
#include  <iostream>
#include "RadialStitcher.h"

#define IMAGEWIDTH 640
#define IMAGEHEIGHT 480
#define NUM_IMAGES 3   // 线程数量

using namespace std;
using namespace cv;




VideoCapture read_camera(string cam_id){
    //读取视频或摄像头
    VideoCapture capture(cam_id);
    capture.set(cv::CAP_PROP_FPS, 1);
    return  capture;
}

//这里需要写一个线程　用队列直接传frames 4个队列 每个队列里面有一个线程用于拼接
// python 样例　imgs_original = [q.get() for q in queue_list]
void collect_capture(vector<VideoCapture> captures){
    vector<Mat> img_list;
    auto *rs = new RadialStitcher(NUM_IMAGES);
    Rect rect(19, 0, 602, 480);
    vector<Mat> cylinder_maps = rs->cylinder_projection_map(640, 480, 720);  //calculate the mapping of the cylinder
    while (true)
    {
        for (int i = 0; i < 4; ++i) {
            Mat frame; Mat cylinder;
            captures[i] >> frame;
            cv::remap(frame, cylinder, cylinder_maps[0], cylinder_maps[1],cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
            cylinder = cylinder(rect); //remove the blank edge of the cylinder image
            img_list.push_back(cylinder);
        }
        Mat result = rs->Stitch(img_list);
        imshow("result", result);
        img_list.clear();
        if ((cv::waitKey(25) & 0XFF) == 27) break;
    }
    destroyAllWindows();
    for (int i = 0; i < 4; ++i) {
        captures[i].release();
    }
}

int main()
{

    string cam_idx[4] = {"/home/eksan/Desktop/test.mp4", "/home/eksan/Desktop/test.mp4", "/home/eksan/Desktop/test.mp4", "/home/eksan/Desktop/test.mp4"};
//      string cam_idx[4] = { "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:3:1.0-video-index0",
//                          "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:5:1.0-video-index0",
//                          "/dev/v4l/by-path/pci-0000:05:00.0-usb-0:1:1.0-video-index0",
//                          "/dev/v4l/by-path/pci-0000:00:14.0-usb-0:4:1.0-video-index0",};

    VideoCapture capture1 = read_camera(cam_idx[0]);
    VideoCapture capture2 = read_camera(cam_idx[1]);
    VideoCapture capture3 = read_camera(cam_idx[2]);
    VideoCapture capture4 = read_camera(cam_idx[3]);
    vector<VideoCapture> captures; captures.push_back(capture1); captures.push_back(capture2); captures.push_back(capture3); captures.push_back(capture4);
    collect_capture(captures);



       return  0;
}

