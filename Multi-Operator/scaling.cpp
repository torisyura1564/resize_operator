#include <iostream>
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"

int main(int argc, char **argv)
{
	cv::Mat src, dst;

	src = imread(argv[1], cv::IMREAD_UNCHANGED);
	resize(src, dst, cv::Size(atoi(argv[3]), atoi(argv[4])), cv::INTER_CUBIC);
	
	char filename[100];
	sprintf(filename, "./resize/scaling/%s.ppm", argv[2]);
	cv::imwrite(filename, dst);

	return 0;
}
