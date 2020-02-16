#include <chrono>
#include "new_smc.h"

void TRANSPOSITION(IMG *img, PARAMETER *param, ENERGY *emap) {

	img->r_image = common::transposition(img->r_image, img->r_height, img->r_width);

	if (emap->direction) img->band_image = common::transposition(img->band_image, img->r_height, img->r_width);

	int height = img->r_height;
	int width = img->r_width;

	img->r_height = width;
	img->r_width = height;

}

void calc_RE(IMG *img, PARAMETER *param, ENERGY *emap) {

	for (int y = 0; y < img->r_height; y++) {
		for (int x = 1; x < img->r_width - param->seam_band; x++) {

			double temp = 0.0;
			std::vector<double> RE_coeff(param->seam_band, 0.0);

			for (int u = 0; u < param->seam_band; u++) {
				for (int i = 0; i < param->seam_band; i++) {
					RE_coeff[u] += img->process_image[y][x + i] * emap->dct_coeff[u][i];
				}
			}

			for (int i = 0; i < param->reduction_band_size; i++) { //IDCTで帯状シームの縮小
				for (int u = 0; u < param->reduction_band_size; u++) {
					temp += RE_coeff[u] * emap->idct_coeff[u][i];
				}
				emap->idct_luminance[y][x][i] = temp + 0.5; //四捨五入
				temp = 0.0;
			}

			for (int i = param->reduction_band_size; i < param->seam_band; i++) temp += pow(RE_coeff[i], 2);
			emap->remap[y][x] = temp / (double)param->band_cut;

		}
	}
}

void make_dct_emap(IMG *img, PARAMETER *param, ENERGY *emap) {

	img->convert(param);

	calc_RE(img, param, emap);

	for (int y = 1; y < img->r_height; y++) {//calc_FE
		for (int x = 1; x < img->r_width - param->seam_band - 1; x++) {

			int xback = x - 1; int xfor = x + 1; int xfor_n = x + param->seam_band; int yback = y - 1;

			double fe1 = img->process_image[y][xback] - emap->idct_luminance[y][x][0];
			double fe2 = img->process_image[y][xfor_n] - emap->idct_luminance[y][x][param->reduction_band_size - 1];

			double fe3 = img->process_image[y][xback] - emap->idct_luminance[yback][xback][0];
			double fe4 = img->process_image[yback][xfor_n - 1] - emap->idct_luminance[y][x][param->reduction_band_size - 1];

			double fe7 = img->process_image[yback][x] - emap->idct_luminance[y][x][0];
			double fe8 = img->process_image[y][xfor_n] - emap->idct_luminance[yback][xfor][param->reduction_band_size - 1];

			emap->fe[y][x][0] = (pow(fe1, 2) + pow(fe2, 2)) / 2;//共通
			emap->fe[y][x][1] = (pow(fe3, 2) + pow(fe4, 2)) / 2;//左上から
			emap->fe[y][x][2] = 0;//真上から
			emap->fe[y][x][3] = (pow(fe7, 2) + pow(fe8, 2)) / 2;//右上から

		}
	}

}

void make_seam_emap(IMG *img, PARAMETER *param, ENERGY *emap) {

	img->convert(param);

	cv::Mat input, sobel_x, sobel_y, edge;
	input = common::Img2Mat(img->process_image, img->r_height, img->r_width);
	cv::Sobel(input, sobel_x, CV_8UC1, 1, 0);
	cv::Sobel(input, sobel_y, CV_8UC1, 0, 1);
	edge = abs(sobel_x) + abs(sobel_y);
	common::Mat2Img(edge, emap->remap);

	for (int y = 1; y < img->r_height; y++) {
		for (int x = 1; x < img->r_width - 1; x++) {

			int xback = x - 1; int xfor = x + 1; int yback = y - 1;

			double fe1 = img->process_image[y][xback] - img->process_image[y][xfor];
			double fe3 = img->process_image[y][xback] - img->process_image[yback][x];
			double fe7 = img->process_image[y][xfor] - img->process_image[yback][x];

			emap->fe[y][x][0] = std::abs(fe1);
			emap->fe[y][x][1] = std::abs(fe3);
			emap->fe[y][x][2] = 0;
			emap->fe[y][x][3] = std::abs(fe7);

		}
	}

}

void costmap_dp(IMG *img, PARAMETER *param, ENERGY *emap) {

	for (int y = 0; y < img->r_height; y++) {
		for (int x = 0; x < img->r_width - param->seam_band; x++) {
			if (x == 0 || x == img->r_width - param->seam_band - 1) {//画面端は選択しない
				emap->dpmap[y][x] = Inf;
				continue;
			}
			emap->dpmap[y][x] = emap->remap[y][x] * param->re_weight;
		}
	}

	for (int y = 1; y < img->r_height; y++) {
		for (int x = 1; x < img->r_width - param->seam_band - 1; x++) {

			std::vector<double> candidate(3, 0.0);
			double cost_pixel = 0.0;

			candidate[0] = emap->dpmap[y - 1][x - 1] + (emap->fe[y][x][0] + emap->fe[y][x][1]) * param->fe_weight;
			candidate[1] = emap->dpmap[y - 1][x] + (emap->fe[y][x][0] + emap->fe[y][x][2]) * param->fe_weight;
			candidate[2] = emap->dpmap[y - 1][x + 1] + (emap->fe[y][x][0] + emap->fe[y][x][3]) * param->fe_weight;

			int idx = common::minidx(3, candidate, &cost_pixel);
			emap->dpmap[y][x] += cost_pixel;
			emap->seam_path_map[y][x] = idx - 1;

			emap->femap[y][x] = (emap->fe[y][x][0] + emap->fe[y][x][idx + 1]) * param->fe_weight;

		}
	}

}

std::vector<int> seam_dp(IMG *img, PARAMETER *param, ENERGY *emap) {

	emap->cost[emap->direction] = Inf;
	int x = 1;
	std::vector<int> seam_path_temp(img->r_height, 0);

	for (int i = 1; i < img->r_width - param->seam_band - 1; i++) {
		if (emap->dpmap[img->r_height - 1][i] < emap->cost[emap->direction]) {
			emap->cost[emap->direction] = emap->dpmap[img->r_height - 1][i];
			x = i;
		}
	}

	seam_path_temp[img->r_height - 1] = x;
	double avgfe = emap->femap[img->r_height - 1][x];
	double avgre = emap->remap[img->r_height - 1][x];

	for (int y = img->r_height - 2; y >= 0; y--) {
		seam_path_temp[y] = seam_path_temp[y + 1] + emap->seam_path_map[y + 1][seam_path_temp[y + 1]];
		avgfe += emap->femap[y][seam_path_temp[y]];
		avgre += emap->remap[y][seam_path_temp[y]];
	}

	printf("\n------ BEST COST --------------------------------------------------------\n");
	printf("  direction --->	%s\n", emap->direction == true ? "y" : "x");
	printf("  start --->	%d\n", seam_path_temp[0]);
	printf("  FE best cost average --->	%.3f\n", avgfe / (double)(img->r_height - 1));
	printf("  RE best cost average --->	%.3f\n", avgre / (double)img->r_height);
	printf("  ALL best cost average ---> %.3f\n", (double)emap->cost[emap->direction] / (double)img->r_height);
	printf("-------------------------------------------------------------------------\n");

	return seam_path_temp;

}

void FindSeam(IMG *img, PARAMETER *param, ENERGY *emap) {

	if (param->SEAMCARVING) make_seam_emap(img, param, emap);
	else make_dct_emap(img, param, emap);
	costmap_dp(img, param, emap);
	emap->seam_path[emap->direction] = seam_dp(img, param, emap);

	img->channels = 3;

}

void DeleteSeam(IMG *img, PARAMETER *param, ENERGY *emap) {

	std::vector<std::vector <std::vector <int> > > r_image_target, seam_img;
	common::resize(r_image_target, img->channels, img->size, img->size);
	common::resize(seam_img, img->channels, img->size, img->size);

	seam_img = img->r_image;
	r_image_target = img->r_image;

	if (param->SEAMCARVING) {
		for (int y = 0; y < img->r_height; y++) {
			for (int x = emap->seam_pos_log[param->band_num][y + 1]; x < img->r_width - param->band_cut; x++) {

				if (x == emap->seam_pos_log[param->band_num][y + 1]) {
					seam_img[0][y][x] = seam_img[1][y][x] = 0;
					seam_img[2][y][x] = 255;
				}
				else {
					for (int i = 0; i < img->channels; i++) r_image_target[i][y][x] = img->r_image[i][y][x + 1];
				}

			}
		}
	}

	else {
		for (int y = 0; y < img->r_height; y++) {
			for (int x = 0; x < img->r_width - param->band_cut; x++) {

				if (x == emap->seam_pos_log[param->band_num][y + 1]) {

					for (int i = 0; i < img->channels; i++) {
						std::vector<int> IDCT_luminance(param->reduction_band_size, 0);
						IDCT_luminance = DCT_IDCT(img->r_image[i], param, emap, x, y);
						for (int n = 0; n < param->reduction_band_size; n++) r_image_target[i][y][x + n] = IDCT_luminance[n];
					}

					for (int i = 0; i < param->seam_band; i++) {
						seam_img[0][y][x + i] = seam_img[1][y][x + i] = 0;
						seam_img[2][y][x + i] = 255;
					}

				}

				else if (x >= emap->seam_pos_log[param->band_num][y + 1] + param->reduction_band_size) {
					for (int i = 0; i < img->channels; i++) r_image_target[i][y][x] = img->r_image[i][y][x + param->band_cut];
				}

			}
		}
	}

	/*sprintf(param->dir_res, "./resize_process/seam/%s/%s_%d.ppm", param->name, param->name, param->band_num);
	if (!emap->direction) {
		seam_img = common::transposition(seam_img, img->r_height, img->r_width);
		common::write_img(param->dir_res, seam_img, img->r_width, img->r_height);
	}
	else common::write_img(param->dir_res, seam_img, img->r_height, img->r_width);*/

	img->r_width -= param->band_cut;
	img->r_image = r_image_target;

}

void decide_direction(IMG *img, PARAMETER *param, ENERGY *emap) {

	if (emap->cost[0] >= emap->cost[1]) {
		emap->seam_pos_log[param->band_num][0] = 1;
		for (int i = 0; i < img->r_height; i++) emap->seam_pos_log[param->band_num][i + 1] = emap->seam_path[1][i];

		param->selected_y += param->band_cut;
		printf("Y selected\n");
		DeleteSeam(img, param, emap);
	}

	else {
		emap->seam_pos_log[param->band_num][0] = 0;
		for (int i = 0; i < img->r_width; i++) emap->seam_pos_log[param->band_num][i + 1] = emap->seam_path[0][i];

		param->selected_x += param->band_cut;

		TRANSPOSITION(img, param, emap);
		printf("X selected\n");
		DeleteSeam(img, param, emap);
		TRANSPOSITION(img, param, emap);
	}

}

void seamcarving(IMG *img, PARAMETER *param, ENERGY *emap) {

	for (param->band_num = 0; param->band_num < param->xy_max; param->band_num++) {

		printf("\n");
		printf("Seam SET Number %d \n", param->band_num + 1);
		printf("Seam X%d Y%d \n", param->selected_x, param->selected_y);

		if (param->selected_y < param->seam_y) {
			printf("Y search\n");
			emap->direction = 1;
			FindSeam(img, param, emap);
		}
		else {
			printf("Y not search\n");
			emap->cost[1] = Inf;
		}

		if (param->selected_x < param->seam_x) {
			printf("X search\n");
			emap->direction = 0;

			TRANSPOSITION(img, param, emap);
			FindSeam(img, param, emap);
			TRANSPOSITION(img, param, emap);
		}
		else {
			printf("X not search\n");
			emap->cost[0] = Inf;
		}

		printf("\n");
		printf("============= Seam Searched =========================================\n");
		printf("\n");

		decide_direction(img, param, emap);

		//リサイズ途中の画像を保存
		//sprintf(param->dir_res, "./resize_process/resize/%s/%s_%d.ppm", param->name, param->name, param->band_num);
		//common::write_img(param->dir_res, img->r_image, img->r_height, img->r_width);

		printf("\n");
		printf("=====================================================================\n");
		printf("\n");

	}

}

void reconst_belt_like_seam(IMG *img, PARAMETER *param, ENERGY *emap) {

	emap->direction = 1;
	img->band_image = img->r_image;

	for (int band_num = param->band_num - 1; band_num >= 0; band_num--) {

		if (emap->seam_pos_log[band_num][0] == 0) TRANSPOSITION(img, param, emap);

		std::vector<std::vector<std::vector<int> > > band_image_temp;
		common::resize(band_image_temp, img->channels, img->size, img->size);
		band_image_temp = img->band_image;

		for (int i = 0; i < img->channels; i++) {
			for (int y = 0; y < img->r_height; y++) {
				for (int x = emap->seam_pos_log[band_num][y + 1]; x < img->r_width - param->band_cut; x++) {
					if (x == emap->seam_pos_log[band_num][y + 1]) {
						for (int n = 0; n < param->seam_band; n++) {
							band_image_temp[0][y][x + n] = 0;
							band_image_temp[1][y][x + n] = 0;
							band_image_temp[2][y][x + n] = 255;
						}
					}
					else if (x >= emap->seam_pos_log[band_num][y + 1] + param->reduction_band_size) band_image_temp[i][y][x + param->band_cut] = img->band_image[i][y][x];
				}
			}
		}

		img->r_width += param->band_cut;
		img->band_image = band_image_temp;

		if (!emap->seam_pos_log[band_num][0]) TRANSPOSITION(img, param, emap);

	}

	sprintf(param->dir_res, "./seam_band_image/%s_%s_%s_%d_%d_DCT.ppm", param->name, param->Cost, param->Seam, param->seam_band, param->band_cut);
	common::write_img(param->dir_res, img->band_image, img->org_height, img->org_width);

}

void reconst_seamcarving(IMG *img, PARAMETER *param, ENERGY *emap) {

	emap->direction = 1;
	img->band_image = img->r_image;

	for (int band_num = param->band_num - 1; band_num >= 0; band_num--) {

		if (emap->seam_pos_log[band_num][0] == 0) TRANSPOSITION(img, param, emap);

		std::vector<std::vector<std::vector<int> > > band_image_temp;
		common::resize(band_image_temp, img->channels, img->size, img->size);
		band_image_temp = img->band_image;

		for (int i = 0; i < img->channels; i++) {
			for (int y = 0; y < img->r_height; y++) {
				for (int x = emap->seam_pos_log[band_num][y + 1]; x < img->r_width - 1; x++) {
					if (x == emap->seam_pos_log[band_num][y + 1]) {
						band_image_temp[0][y][x] = 0;
						band_image_temp[1][y][x] = 0;
						band_image_temp[2][y][x] = 255;
					}
					else band_image_temp[i][y][x + 1] = img->band_image[i][y][x];
				}
			}
		}

		img->r_width += 1;
		img->band_image = band_image_temp;

		if (!emap->seam_pos_log[band_num][0]) TRANSPOSITION(img, param, emap);

	}

	sprintf(param->dir_res, "./seam_band_image/%s_%s_%s_SEAMCARVING.ppm", param->name, param->Cost, param->Seam);
	common::write_img(param->dir_res, img->band_image, img->org_height, img->org_width);

}

int main(int argc, char **argv)
{
	std::chrono::system_clock::time_point start, end; //時間計測用

	PARAMETER *param;
	param = new PARAMETER();

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'x':
				param->seam_x = atoi(argv[++i]);
				break;
			case 'y':
				param->seam_y = atoi(argv[++i]);
				break;
			case 'f':
				param->fe_weight = atoi(argv[++i]);
				break;
			case 'r':
				param->re_weight = atoi(argv[++i]);
				break;
			case 'B':
				param->seam_band = atoi(argv[++i]);
				break;
			case 'C':
				param->band_cut = atoi(argv[++i]);
				param->reduction_band_size = param->seam_band - param->band_cut >= 0 ? param->seam_band - param->band_cut : 1;
				param->xy_max = (param->seam_x + param->seam_y) / param->band_cut;
				break;
			case 'S':
				param->SEAMCARVING = atoi(argv[++i]);
				break;
			default:
				fprintf(stderr, "Unknown option: %s!\n", argv[i]);
				exit(1);
			}
		}
		else {
			param->name = argv[i++];
			param->dir_img = argv[i++];
			param->Cost = argv[i++];
			param->Seam = argv[i++];

		}
	}

	IMG *img;
	img = new IMG(param);

	ENERGY *emap;
	emap = new ENERGY(param, img);

	param->write_param();

	//時間計測
	start = std::chrono::system_clock::now();

	seamcarving(img, param, emap);//resize

	if (param->SEAMCARVING) sprintf(param->dir_res, "./resize/%s_%s_%s_SEAMCARVING.ppm", param->name, param->Cost, param->Seam);
	else sprintf(param->dir_res, "./resize/%s_%s_%s_%d_%d_DCT.ppm", param->name, param->Cost, param->Seam, param->seam_band, param->band_cut);

	//resize画像の保存
	common::write_img(param->dir_res, img->r_image, img->r_height, img->r_width);

	//シーム侵入一覧表示
	if(param->SEAMCARVING) reconst_seamcarving(img, param, emap);
	else reconst_belt_like_seam(img, param, emap);

	end = std::chrono::system_clock::now();

	auto elapsed = std::chrono::duration_cast< std::chrono::milliseconds >(end - start).count();
	std::cout << elapsed << "ms" << std::endl;

	delete(param);
	delete(emap);
	delete(img);
	
	return 0;
}