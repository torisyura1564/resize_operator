#define _USE_MATH_DEFINES
#include "opencv2/opencv.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <tuple>

#define Inf 100000000

#ifndef NEW_SMC
#define NEW_SMC

namespace common {

	template<typename V>
	cv::Mat Img2Mat(std::vector<std::vector <std::vector <V> > > image, int height, int width) {

		cv::Mat output_color(height, width, CV_8UC3);

		for (int i = 0; i < 3; i++) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					output_color.at<cv::Vec3b>(y, x)[i] = cv::saturate_cast<uchar>(image[i][y][x]);
				}
			}
		}

		return output_color;
	}

	template<typename V>
	cv::Mat Img2Mat(std::vector<std::vector <V> > image, int height, int width) {

		cv::Mat output_color(height, width, CV_8UC1);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				output_color.at<uchar>(y, x) = cv::saturate_cast<uchar>(image[y][x]);
			}
		}

		return output_color;
	}

	template<typename V>
	void Mat2Img(cv::Mat mat, std::vector<std::vector <std::vector <V> > > &image) {

		for (int i = 0; i < 3; i++) {
			for (int y = 0; y < mat.size().height; y++) {
				for (int x = 0; x < mat.size().width; x++) {
					image[i][y][x] = cv::saturate_cast<uchar>(mat.at<cv::Vec3b>(y, x)[i]);
				}
			}
		}

	}

	template<typename V>
	void Mat2Img(cv::Mat mat, std::vector<std::vector <V> > &image) {
		for (int y = 0; y < mat.size().height; y++) {
			for (int x = 0; x < mat.size().width; x++) {
				image[y][x] = cv::saturate_cast<uchar>(mat.at<uchar>(y, x));
			}
		}
	}

	/* -- WRITE function -- */
	template<typename V>
	void write_img(char* name, std::vector<std::vector <std::vector <V> > > image, int height, int width) {

		cv::Mat output(height, width, CV_8UC3);

		printf("***** write function *****\n");
		printf("HEIGHT %d, WIDTH %d, NAME %s\n", height, width, name);
		printf("\n");

		output = Img2Mat(image, height, width);
		cv::imwrite(name, output);

		std::cout << "Write Success!" << std::endl;

	}
	
	template<typename V>
	void write_img(char* name, std::vector<std::vector <V> > image, int height, int width) {

		cv::Mat output(height, width, CV_8UC1);

		printf("***** write function *****\n");
		printf("HEIGHT %d, WIDTH %d, NAME %s\n", height, width, name);
		printf("\n");

		output = Img2Mat(image, height, width);
		cv::imwrite(name, output);

		std::cout << "Write Success!" << std::endl;

	}

	/* -- リサイズ -- */
	template<typename V, typename H>
	void resize(std::vector<V>& vec, const H head) {
		vec.resize(head);
	}

	template<typename V, typename H, typename ... T>
	void resize(std::vector<V>& vec, const H& head, const T ... tail) {
		vec.resize(head);
		for (auto& v : vec) resize(v, tail...);
	}

	/* -- 最小値選択 -- */
	template<typename T>
	int minidx(int num, std::vector<T> date, T *cost) {

		int idx;
		*cost = Inf;

		for (int i = 0; i < num; i++) {
			if (*cost>date[i]) {
				idx = i;
				*cost = date[i];
			}
		}

		return idx;
	}

	template<typename V>
	std::vector<V> transposition(std::vector<V> image, int height, int width) {

		cv::Mat t = Img2Mat(image, height, width), temp;
		cv::transpose(t, temp);
		Mat2Img(temp, image);

		return image;
	}

};

class PARAMETER {

public:

	char *name;					/* -- 入力画像の名前 -- */
	char *Seam;					/* -- 合計削除数 -- */
	char *Cost;					/* -- 使用したコスト関数 -- */
	char *dir_img;				/* -- 入力画像のディレクトリ -- */
	char dir_res[100];				/* -- リサイズ画像のディレクトリ -- */

	int seam_band;				/* -- 帯状シーム幅(現状，水平方向，垂直方向ともに同一のパラメータに設定) -- */
	int seam_x;					/* -- 水平方向に何画素抜くか -- */
	int seam_y;					/* -- 垂直方向に何画素抜くか -- */
	int band_cut;				/* -- 帯状シーム縮小幅 -- */
	int reduction_band_size;	/* -- 帯状シーム縮小後の幅 -- */
	int xy_max;					/* -- 合計削除回数 -- */
	int band_num;				/* -- 現在の削除回数 -- */
	int selected_y = 0;			/* -- 縦方向の選択回数 -- */
	int selected_x = 0;			/* -- 横方向の選択回数 -- */

	double fe_weight;			/* -- FEの重み -- */
	double re_weight;			/* -- REの重み -- */

	bool SEAMCARVING = false;

	PARAMETER() {
		std::cout << "PARAMETER Allocate!" << std::endl;
	}

	virtual ~PARAMETER() {
		std::cout << "PARAMETER Release!" << std::endl;
	}

	void write_param() {

		std::cout << "DIR_IMG : " << dir_img << std::endl;
		std::cout << "Seam : " << Seam << std::endl;
		std::cout << "Cost : " << Cost << std::endl;
		std::cout << "band : " << seam_band << " , cut : " << band_cut << " , reduction : " << reduction_band_size << std::endl;
		std::cout << "x : " << seam_x << " , y : " << seam_y << " , x + y : " << xy_max << std::endl;
		std::cout << "DCT_seam" << std::endl;
	}

};

class IMG{

public:
	int channels;
	int org_width, r_width;
	int org_height, r_height;
	int size;

	std::vector<std::vector <std::vector <int> > > r_image, band_image;
	std::vector<std::vector <int> > process_image;

	IMG(PARAMETER *param){

		cv::Mat img = cv::imread(param->dir_img, cv::IMREAD_UNCHANGED);
		if (img.data == NULL) {
			std::cerr << "Can\'t open !" << std::endl;
			exit(1);
		}

		printf("read image : rows = %d, cols = %d, channels = %d\n", img.rows, img.cols, img.channels());

		r_height = org_height = img.rows;
		r_width = org_width = img.cols;
		channels = img.channels();
		size = org_height > org_width ? org_height : org_width;

		common::resize(r_image, channels, size, size);
		common::resize(process_image, size, size);
		common::resize(band_image, channels, size, size);

		common::Mat2Img(img, r_image);

		printf("Read Success!\n");

	}
	virtual ~IMG() {
		printf(" ------------------------------ Make Seam End! ------------------------------ \n\n");
	}

	/* -- 3色を輝度値に変換 -- */
	void convert(PARAMETER *param) {

		cv::Mat output_gray(r_height, r_width, CV_8U);
		cv::Mat output_color(r_height, r_width, CV_8UC3);
	
		output_color = common::Img2Mat(r_image, r_height, r_width);
		cv::cvtColor(output_color, output_gray, CV_BGR2GRAY);

		common::Mat2Img(output_gray, process_image);

		channels = 1;
	}

};

class ENERGY {

public:
	/* -- 帯状シーム領域のコスト -- */
	std::vector<std::vector <std::vector <int> > > idct_luminance;

	/* -- 1次元DCTの計算用係数 -- */
	std::vector<std::vector <double> > dct_coeff;
	std::vector<std::vector <double> > idct_coeff;
	
	/* -- 削除方向 -- */
	int direction; // 1 : 縦方向，0 : 横方向

	/* -- FE -- */
	std::vector<std::vector <std::vector <double> > > fe;

	/* -- costmap -- */
	std::vector<std::vector <double> > dpmap;
	std::vector<std::vector <double> > remap;
	std::vector<std::vector <double> > femap;

	std::vector<std::vector <int> > seam_path, seam_path_map, seam_pos_log;

	int cost[2];

	ENERGY(PARAMETER *param, IMG *img) {

		common::resize(idct_luminance, img->size, img->size, param->reduction_band_size);
		common::resize(dct_coeff, param->seam_band, param->seam_band);
		common::resize(idct_coeff, param->reduction_band_size, param->reduction_band_size);
		common::resize(fe, img->size, img->size, 4);
		common::resize(dpmap, img->size, img->size);
		common::resize(femap, img->size, img->size);
		common::resize(remap, img->size, img->size);
		common::resize(seam_path, 2, img->size);
		common::resize(seam_path_map, img->size, img->size);
		common::resize(seam_pos_log, param->xy_max, img->size);

		std::cout << "ENERGY Allocate!" << std::endl;

		for (int u = 0; u < param->seam_band; u++) {
			for (int i = 0; i < param->seam_band; i++) {

				if (u == 0) {
					dct_coeff[u][i] = cos(M_PI * (2.0 * i + 1.0) * u / (2.0 * param->seam_band)) / sqrt(param->seam_band);
					if (u < param->reduction_band_size && i < param->reduction_band_size) idct_coeff[u][i] = cos(M_PI * (2.0 * i + 1.0) * u / (2.0 * param->reduction_band_size)) / sqrt(param->seam_band);
				}
				else {
					dct_coeff[u][i] = cos(M_PI * (2.0 * i + 1.0) * u / (2.0 * param->seam_band)) * sqrt(2.0) / sqrt(param->seam_band);
					if (u < param->reduction_band_size && i < param->reduction_band_size) idct_coeff[u][i] = cos(M_PI * (2.0 * i + 1.0) * u / (2.0 * param->reduction_band_size)) * sqrt(2.0) / sqrt(param->seam_band);
				}
			}
		}

		std::cout << "DCT coefficient Allocate" << std::endl;

	}
	virtual ~ENERGY() {
		std::cout << "ENERGY Release!" << std::endl;
	}

};

template<typename V>
std::vector<V> DCT_IDCT(std::vector<std::vector <V> > image, PARAMETER *param, ENERGY *emap, const int x, const int y) { // DCT

	double temp = 0.0;
	std::vector<double> RE_coeff(param->seam_band, 0.0);
	std::vector<V> IDCT_luminance;

	for (int u = 0; u < param->seam_band; u++) { //DCT係数
		for (int i = 0; i < param->seam_band; i++) {
			RE_coeff[u] += image[y][x + i] * emap->dct_coeff[u][i];
		}
	}

	for (int i = 0; i < param->reduction_band_size; i++) { //IDCTで帯状シームの縮小
		for (int u = 0; u < param->reduction_band_size; u++) {
			temp += RE_coeff[u] * emap->idct_coeff[u][i];
		}
		if (temp < 0) temp = 0.0;
		if (temp > 255) temp = 255;
		IDCT_luminance.push_back(temp + 0.5); //四捨五入
		temp = 0.0;
	}

	return IDCT_luminance;

}

#endif