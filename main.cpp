#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <array>
#include <string_view>
#include "./Thread/ThreadPool.h"
#include "RadialStitcher.h"

constexpr auto IMAGEWIDTH = 640;
constexpr auto IMAGEHEIGHT = 480;
constexpr auto NUM_IMAGES = 3;


using namespace std;
using namespace cv;




VideoCapture read_camera(string cam_id) {
	//读取视频或摄像头
	VideoCapture capture(cam_id);
	capture.set(cv::CAP_PROP_FPS, 1);
	return  capture;
}

//这里需要写一个线程　用队列直接传frames 4个队列 每个队列里面有一个线程用于拼接
// python 样例　imgs_original = [q.get() for q in queue_list]

int main()
{
	std::array<string_view, 4> cam_idx{ "/home/eksan/Desktop/test.mp4",
		"/home/eksan/Desktop/test.mp4",
		"/home/eksan/Desktop/test.mp4",
		"/home/eksan/Desktop/test.mp4" };

	stiching::ThreadPool pool(NUM_IMAGES);

	vector<VideoCapture> captures;
	vector<Mat> frameVec(4, Mat());
	vector<Mat> img_list;

	for (int i = 0; i < 4; i++) {
		pool.async([&]() {
			captures.push_back(std::move
			(read_camera
			(cam_idx[i].data())));
			});
	}

	for (int i = 0; i < 4; i++) {
		pool.async([&]() {
			Mat frame;
			captures[i] >> frame;
			frameVec.push_back(frame);
			});
	}

	std::shared_ptr<RadialStitcher> rs = make_shared<RadialStitcher>
		(new RadialStitcher(NUM_IMAGES));
	Rect rect(19, 0, 602, 480);
	vector<Mat> cylinder_maps = rs->cylinder_projection_map(640, 480, 720);  //calculate the mapping of the cylinder

	for (int i = 0; i < 4; i++) {
		pool.async([&]() {
			Mat cylinder;
			cv::remap(frameVec[i], cylinder,
				cylinder_maps[0], cylinder_maps[1],
				cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
			cylinder = cylinder(rect); //remove the blank edge of the cylinder image
			img_list.push_back(cylinder);
			});
	}

	Mat result = rs->Stitch(img_list);
	imshow("result", result);
	img_list.clear();
	if ((cv::waitKey(25) & 0XFF) == 27) exit(1);

	cv::destroyAllWindows();
	for (int i = 0; i < 4; ++i) {
		captures[i].release();
	}

	return  0;
}

