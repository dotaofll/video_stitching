#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <array>
#include <string_view>
#include <mutex>
#include <future>
#include <execution>
#include <algorithm>
#include <functional>
#include "./Thread/ThreadPool.h"
#include "RadialStitcher.h"

constexpr auto IMAGEWIDTH = 640;
constexpr auto IMAGEHEIGHT = 480;
constexpr auto NUM_IMAGES = 3;

using namespace std;
using namespace cv;

static std::array<std::string_view, 4> cam_idx{
	"/home/eksan/Desktop/test.mp4",
	"/home/eksan/Desktop/test.mp4",
	"/home/eksan/Desktop/test.mp4",
	"/home/eksan/Desktop/test.mp4"};

VideoCapture read_camera(string &cam_id)
{
	//读取视频或摄像头
	VideoCapture capture(cam_id);
	capture.set(cv::CAP_PROP_FPS, 1);
	return capture;
}

//这里需要写一个线程　用队列直接传frames 4个队列 每个队列里面有一个线程用于拼接
// python 样例　imgs_original = [q.get() for q in queue_list]

int main()
{
	stiching::ThreadPool pool(NUM_IMAGES);
	std::mutex mut_a, mut_b;
	vector<VideoCapture> captures(4);
	vector<Mat> frameVec(4, Mat());
	vector<Mat> img_list;

	auto index{0};
	auto fuck(std::bind(read_camera, placeholders::_1));

	for_each(execution::par, captures.begin(), captures.end(),
			 [&](VideoCapture &_this) {
				 std::scoped_lock{mut_a, mut_b};
				 auto pos = index++;
				 string str(cam_idx[pos].data, cam_idx[pos].size());
				 _this = fuck(str);
			 });

	for_each(execution::par, captures.begin(), captures.end(),
			 [&](VideoCapture &_this) {
				 std::scoped_lock{mut_a, mut_b};
				 Mat frame;
				 //TODO
				 //VideoCapture2Mat interface! And
				 //Push_back to frameVec;
			 });

	std::shared_ptr<RadialStitcher> rs{new RadialStitcher{NUM_IMAGES}};
	Rect rect(19, 0, 602, 480);
	vector<Mat> cylinder_maps = rs->cylinder_projection_map(640, 480, 720); //calculate the mapping of the cylinder

	for (int i = 0; i < 4; i++)
	{
		Mat cylinder;
		cv::remap(frameVec[i], cylinder,
				  cylinder_maps[0], cylinder_maps[1],
				  cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
		cylinder = cylinder(rect); //remove the blank edge of the cylinder image
		img_list.push_back(cylinder);
	}

	Mat result = rs->Stitch(img_list);
	imshow("result", result);
	img_list.clear();
	if ((cv::waitKey(25) & 0XFF) == 27)
		exit(1);

	cv::destroyAllWindows();
	for (int i = 0; i < 4; ++i)
	{
		captures[i].release();
	}

	return 0;
}
