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

	void dot(cv::Mat subImg, double* sigma_color, double* sigma_space, cv::Mat &result, int pos_y, int pos_x) {
		double value_temp = 0.0, weight_sum = 0.0;
		int center_y = (subImg.rows - 1) / 2;
		int center_x = (subImg.cols - 1) / 2;

		for (int y = 0; y < subImg.rows; y++) {//内積
			for (int x = 0; x < subImg.cols; x++) {
				double weight_temp = 0.0;
				uchar value_diff = cv::saturate_cast<uchar>(std::abs(subImg.at<uchar>(center_y, center_x) - subImg.at<uchar>(y, x)));
				weight_temp = sigma_color[value_diff];

				double weight = weight_temp * sigma_space[std::abs(center_y - y)] * sigma_space[std::abs(center_x - x)];
				value_temp += cv::saturate_cast<uchar>(subImg.at<uchar>(y, x)) * weight;
				weight_sum += weight;
			}
		}
		value_temp /= weight_sum;

		result.at<uchar>(pos_y, pos_x) = cv::saturate_cast<uchar>(value_temp);

	};

	void Bilateralfilter(cv::Mat &input, cv::Mat &output, int size, double sigma_color, double sigma_space) {
		cv::Mat temp;
		int pad = (size - 1) / 2;
		input.convertTo(temp, CV_8U);
		input.convertTo(output, CV_8U);
		cv::copyMakeBorder(temp, temp, pad, pad, pad, pad, cv::BORDER_REFLECT);

		double gauss_color_coeff = -0.5 / (sigma_color*sigma_color);
		double gauss_space_coeff = -0.5 / (sigma_space*sigma_space);

		std::vector<double> _color_weight(256);
		std::vector<double> _space_weight(pad + 1);
		double* color_weight = &_color_weight[0];
		double* space_weight = &_space_weight[0];

		for (int i = 0; i < 256; i++)
			color_weight[i] = (double)std::exp(i*i*gauss_color_coeff);

		for (int i = 0; i <= pad; i++)
			space_weight[i] = (double)std::exp(i*i*gauss_space_coeff);

		for (int y = 0; y < input.rows; y++) {
			for (int x = 0; x < input.cols; x++) {
				cv::Rect roi(cv::Point(x, y), cv::Size(size, size));//局所領域の抽出
				cv::Mat subImg = temp(roi);//局所領域の抽出,型はCV_8UC3
				dot(subImg, color_weight, space_weight, output, y, x);//フィルタ計算
			}
		}
	};

};

class PARAMETER {

public:

	char *name = NULL;					/* -- 入力画像の名前 -- */
	char *Seam = NULL;					/* -- 合計削除数 -- */
	char *Cost = NULL;					/* -- 使用したコスト関数 -- */
	char *dir_img = NULL;				/* -- 入力画像のディレクトリ -- */
	char *dir_mask = NULL;				/* -- マスク画像のディレクトリ -- */
	char dir_res[100];					/* -- リサイズ画像のディレクトリ -- */
	char mode[100];						

	int seam_band;				/* -- 帯状シーム幅(現状，水平方向，垂直方向ともに同一のパラメータに設定) -- */
	int seam_x;					/* -- 水平方向に何画素抜くか -- */
	int seam_y;					/* -- 垂直方向に何画素抜くか -- */
	int ratio_x;				/* -- アスペクト比 -- */
	int ratio_y;				/* -- アスペクト比 -- */
	int band_cut;				/* -- 帯状シーム縮小幅 -- */
	int reduction_band_size;	/* -- 帯状シーム縮小後の幅 -- */
	int xy_max;					/* -- 合計削除回数 -- */
	int band_num;				/* -- 現在の削除回数 -- */
	int selected_y = 0;			/* -- 縦方向の選択回数 -- */
	int selected_x = 0;			/* -- 横方向の選択回数 -- */

	double fe_weight;			/* -- FEの重み -- */
	double re_weight;			/* -- REの重み -- */

	std::vector<int> mask_weight;	/* -- mask画像による重要度 (0:R,1:G,2:B,3:P,4:Y,5:O,6:YG,7:W)-- */
	std::vector<double> surv_rate;	/* -- オブジェクト残存率 -- */
	int color_num;

	const int obj_num = 9;

	int kernel;
	double sigma_color;
	double sigma_space;
	bool filter_flag = true;

	std::map<std::tuple<uchar, uchar, uchar>, int> field{
		{ std::make_tuple(128, 0, 0), 1 },
		{ std::make_tuple(0, 128, 0), 2 },
		{ std::make_tuple(0, 0, 128), 3 },
		{ std::make_tuple(128, 0, 128), 4 },
		{ std::make_tuple(128, 128, 0), 5 },
		{ std::make_tuple(0, 128, 128), 6 },
		{ std::make_tuple(128, 128, 128), 7 },
		{ std::make_tuple(128, 64, 0), 8 },
		{ std::make_tuple(64, 128, 0), 9 },
	};

	PARAMETER() {
		std::cout << "PARAMETER Allocate!" << std::endl;
	}

	virtual ~PARAMETER() {
		std::cout << "PARAMETER Release!" << std::endl;
	}

	void write_param() {

		std::cout << "NAME : " << name << std::endl;
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
	bool flag = true;

	std::vector<std::vector <std::vector <int> > > r_image;
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
		size = org_height > org_width ? org_height : org_width; //大きい値をsizeとする．

		common::resize(r_image, channels, size, size);
		common::resize(process_image, size, size);

		common::Mat2Img(img, r_image);

		param->seam_x = (org_height - (org_width * param->ratio_y) / param->ratio_x) > 0 ? org_height - (org_width * param->ratio_y) / param->ratio_x : 0;
		param->seam_y = (org_width - (org_height * param->ratio_x) / param->ratio_y) > 0 ? org_width - (org_height * param->ratio_x) / param->ratio_y : 0;
		param->xy_max = (param->seam_x + param->seam_y) / param->band_cut;

		printf("Read Success!\n");

	}
	virtual ~IMG() {
		printf(" ------------------------------ Make Seam End! ------------------------------ \n\n");
	}

	/* -- 3色を輝度値に変換 -- */
	void convert(PARAMETER *param) {
		cv::Mat output_gray(r_height, r_width, CV_8U), temp;
		cv::Mat output_color(r_height, r_width, CV_8UC3);

		output_color = common::Img2Mat(r_image, r_height, r_width);
		cv::cvtColor(output_color, output_gray, CV_BGR2GRAY);

		if (param->filter_flag) {
			common::Bilateralfilter(output_gray, temp, param->kernel, param->sigma_color, param->sigma_space);
			temp.convertTo(output_gray, CV_8UC1);
		}

		common::Mat2Img(output_gray, process_image);

		channels = 1;
	}

};

class ENERGY {

public:
	/* -- 帯状シーム領域のコスト -- */
	std::vector<std::vector <std::vector <int> > > idct_luminance; //IDCTで戻した輝度値

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
	std::vector<std::vector <double> > maskweight;

	std::vector<std::vector <std::vector <int> > > obj_map;
	std::vector<int> obj_map_thr;
	std::vector<bool> obj_map_flag;

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
		common::resize(maskweight, img->size, img->size);
		common::resize(seam_path, 2, img->size);
		common::resize(seam_path_map, img->size, img->size);
		common::resize(seam_pos_log, 1000, img->size);
		common::resize(obj_map, param->color_num, img->size, img->size);
		obj_map_thr.resize(param->color_num);
		obj_map_flag.resize(param->color_num, 1);

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

// プロトタイプ宣言
void seamcarving(IMG *, PARAMETER *, ENERGY *);
void reconst_seamband(IMG *, PARAMETER *, ENERGY *);

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