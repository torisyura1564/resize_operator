#include "new_smc.h"
#include <chrono>

void TRANSPOSITION(IMG *img, PARAMETER *param, ENERGY *emap) {

	img->r_image = common::transposition(img->r_image, img->r_height, img->r_width);
	for (auto i = 0; i < param->color_num; i++) emap->obj_map[i] = common::transposition(emap->obj_map[i], img->r_height, img->r_width);

	int height = img->r_height;
	int width = img->r_width;

	img->r_height = width;
	img->r_width = height;

}

void DCT_energy(IMG *img, PARAMETER *param, ENERGY *emap, const int x, const int y) { // DCT

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

void make_dct_emap(IMG *img, PARAMETER *param, ENERGY *emap) {

	img->convert(param);

	std::vector<std::vector <int> > mask;
	common::resize(mask, img->r_height, img->r_width);

	for (int y = 0; y < img->r_height; y++) {
		for (int x = 0; x < img->r_width; x++) {
			for (int i = 0; i < param->color_num; i++) {
				if (emap->obj_map[i][y][x] > 0 && param->mask_weight[i] == Inf) {
					mask[y][x] = Inf;
					break;
				}
			}
		}
	}

	for (int y = 0; y < img->r_height; y++) {
		for (int x = 1; x < img->r_width - param->seam_band; x++) {
			DCT_energy(img, param, emap, x, y);
			for (int i = 0; i < param->seam_band; i++) {
				if (mask[y][x + i] == Inf) {
					emap->maskweight[y][x] = Inf;
					break;
				}
				if(!i) emap->maskweight[y][x] = 1;
			}
		}
	}

	for (int y = 1; y < img->r_height; y++) {
		for (int x = 1; x < img->r_width - param->seam_band - 1; x++) {

			int xback = x - 1; int xfor = x + 1; int xfor_n = x + param->seam_band; int yback = y - 1;

			double fe1 = img->process_image[y][xback] - emap->idct_luminance[y][x][0];
			double fe2 = img->process_image[y][xfor_n] - emap->idct_luminance[y][x][param->reduction_band_size - 1];

			//左上から
			double fe3 = emap->idct_luminance[yback][x][0] - img->process_image[y][xback];
			double fe4 = img->process_image[yback][xfor_n - 1] - emap->idct_luminance[y][x][param->reduction_band_size - 1];

			//右上から
			double fe7 = img->process_image[yback][x] - emap->idct_luminance[y][x][0];
			double fe8 = img->process_image[y][xfor_n] - emap->idct_luminance[yback][xfor][param->reduction_band_size - 1];

			emap->fe[y][x][0] = (pow(fe1, 2) + pow(fe2, 2)) / 2;
			emap->fe[y][x][1] = (pow(fe3, 2) + pow(fe4, 2)) / 2;
			emap->fe[y][x][2] = 0;
			emap->fe[y][x][3] = (pow(fe7, 2) + pow(fe8, 2)) / 2;

		}
	}

}

void costmap_dp(IMG *img, PARAMETER *param, ENERGY *emap) {

	for (int y = 0; y < img->r_height; y++) {
		for (int x = 0; x < img->r_width - param->seam_band; x++) {
			if (x == 0 || x == img->r_width - param->seam_band - 1) {
				emap->dpmap[y][x] = Inf;
				continue;
			}
			emap->dpmap[y][x] = emap->maskweight[y][x] == Inf ? Inf : emap->remap[y][x] * param->re_weight;
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
			emap->dpmap[y][x] = emap->dpmap[y][x] + cost_pixel >= Inf ? Inf : emap->dpmap[y][x] + cost_pixel;
			emap->seam_path_map[y][x] = idx - 1;

			emap->femap[y][x] = (emap->fe[y][x][0] + emap->fe[y][x][idx + 1]) * param->fe_weight;

		}
	}

}

std::vector<int> seam_dp(IMG *img, PARAMETER *param, ENERGY *emap) {

	emap->cost[emap->direction] = Inf;
	int x = 1;
	std::vector<int> seam_path_temp(img->r_height, 0);

	for (int i = 1; i < img->r_width - param->seam_band; i++) {
		if (emap->dpmap[img->r_height - 1][i] < emap->cost[emap->direction]) {
			emap->cost[emap->direction] = emap->dpmap[img->r_height - 1][i];
			x = i;
		}
	}

	if (emap->cost[emap->direction] == Inf) {
		std::cout << "Cannot Select Path" << std::endl;
		return seam_path_temp;
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

	make_dct_emap(img, param, emap);
	costmap_dp(img, param, emap);
	emap->seam_path[emap->direction] = seam_dp(img, param, emap);

	if (emap->cost[emap->direction] == Inf) {
		std::cout << "Selected End" << std::endl;
		if (emap->direction) param->selected_y = param->seam_y;
		else param->selected_x = param->seam_x;
		img->channels = 3;
		return;
	}

	img->channels = 3;

}
		
void DeleteSeam(IMG *img, PARAMETER *param, ENERGY *emap) {

	std::vector<std::vector <std::vector <int> > > r_image_target, obj_map_target;

	common::resize(r_image_target, img->channels, img->size, img->size);
	common::resize(obj_map_target, param->color_num, img->size, img->size);


	for (int y = 0; y < img->r_height; y++) {
		for (int x = 0; x < img->r_width - param->band_cut; x++) {

			if (x < emap->seam_pos_log[param->band_num][y + 1]) {
				for (int i = 0; i < img->channels; i++) r_image_target[i][y][x] = img->r_image[i][y][x];
				for (int i = 0; i < param->color_num; i++)	obj_map_target[i][y][x] = emap->obj_map[i][y][x];
			}

			else if (x == emap->seam_pos_log[param->band_num][y + 1]) {

				for (int i = 0; i < img->channels; i++) {
					std::vector<int> IDCT_luminance(param->reduction_band_size, 0);
					IDCT_luminance = DCT_IDCT(img->r_image[i], param, emap, x, y);

					for (int n = 0; n < param->reduction_band_size; n++) r_image_target[i][y][x + n] = IDCT_luminance[n];
				}

				for (int i = 0; i < param->color_num; i++) {
					std::vector<int> IDCT_luminance(param->reduction_band_size, 0);
					IDCT_luminance = DCT_IDCT(emap->obj_map[i], param, emap, x, y);

					for (int n = 0; n < param->reduction_band_size; n++) obj_map_target[i][y][x + n] = IDCT_luminance[n];
				}

			}

			else if (x >= emap->seam_pos_log[param->band_num][y + 1] + param->reduction_band_size) {
				for (int i = 0; i < img->channels; i++) r_image_target[i][y][x] = img->r_image[i][y][x + param->band_cut];
				for (int i = 0; i < param->color_num; i++) obj_map_target[i][y][x] = emap->obj_map[i][y][x + param->band_cut];
			}

		}
	}

	img->r_width -= param->band_cut;
	img->r_image = r_image_target;
	emap->obj_map = obj_map_target;

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

		if (emap->cost[0] == Inf && emap->cost[1] == Inf) {
			std::cout << "seamcarving finished" << std::endl;
			return;
		}

		decide_direction(img, param, emap);

		for (int i = 0; i < param->color_num; i++) {
			if (emap->obj_map_flag[i]) {
				auto temp = 0.0;
				for (int y = 0; y < img->r_height; y++) temp += accumulate(emap->obj_map[i][y].begin(), emap->obj_map[i][y].end(), 0);
				std::cout << "thr[" << i << "] : " << emap->obj_map_thr[i] << " , temp : " << temp << std::endl;
				if (emap->obj_map_thr[i] >= temp) {
					param->mask_weight[i] = Inf;
					emap->obj_map_flag[i] = false;
				}
			}
		}

		printf("\n");
		printf("=====================================================================\n");
		printf("\n");

	}

	std::cout << "get Aspect ratio" << std::endl;

	while(true){

		for (int i = 0; i < param->ratio_x; i++) {
			printf("Y search\n");
			emap->direction = 1;

			FindSeam(img, param, emap);

			if (emap->cost[emap->direction] == Inf) 
				return;

			emap->seam_pos_log[++param->band_num][0] = 1;
			for (int i = 0; i < img->r_height; i++) emap->seam_pos_log[param->band_num][i + 1] = emap->seam_path[1][i];

			DeleteSeam(img, param, emap);
			
			//オブジェクト判定．
			for (int i = 0; i < param->color_num; i++) {
				if (emap->obj_map_flag[i]) {
					auto temp = 0.0;
					for (int y = 0; y < img->r_height; y++) temp += accumulate(emap->obj_map[i][y].begin(), emap->obj_map[i][y].end(), 0);
					if (emap->obj_map_thr[i] >= temp) {
						param->mask_weight[i] = Inf;
						emap->obj_map_flag[i] = false;
					}
				}
			}
			param->band_num++;
		}

		for (int i = 0; i < param->ratio_y; i++) {
			printf("X search\n");
			emap->direction = 0;

			TRANSPOSITION(img, param, emap);
			FindSeam(img, param, emap);
			TRANSPOSITION(img, param, emap);

			if (emap->cost[emap->direction] == Inf) 
				return;

			emap->seam_pos_log[++param->band_num][0] = 0;
			for (int i = 0; i < img->r_width; i++) emap->seam_pos_log[param->band_num][i + 1] = emap->seam_path[0][i];

			TRANSPOSITION(img, param, emap);
			DeleteSeam(img, param, emap);
			TRANSPOSITION(img, param, emap);

			for (int i = 0; i < param->color_num; i++) {
				if (emap->obj_map_flag[i]) {
					auto temp = 0.0;
					for (int y = 0; y < img->r_height; y++) temp += accumulate(emap->obj_map[i][y].begin(), emap->obj_map[i][y].end(), 0);
					if (emap->obj_map_thr[i] >= temp) {
						param->mask_weight[i] = Inf;
						emap->obj_map_flag[i] = false;
					}
				}
			}
			param->band_num++;
		}
	}
}

int main(int argc, char **argv)
{
	PARAMETER *param;
	param = new PARAMETER();

	std::vector<double> surv_rate;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'x':
				param->ratio_x = atoi(argv[++i]);
				break;
			case 'y':
				param->ratio_y = atoi(argv[++i]);
				break;
			case 'f':
				param->fe_weight = atoi(argv[++i]);
				break;
			case 'r':
				param->re_weight = atoi(argv[++i]);
				break;
			case 'b':
				param->seam_band = atoi(argv[++i]);
				break;
			case 'c':
				param->band_cut = atoi(argv[++i]);
				param->reduction_band_size = param->seam_band - param->band_cut;
				break;
			case 'k':
				param->kernel = atoi(argv[++i]);
				break;
			case 'C':
				param->sigma_color = atof(argv[++i]);
				break;
			case 'S':
				param->sigma_space = atof(argv[++i]);
				if (param->kernel && param->sigma_color && param->sigma_space) sprintf(param->mode, "_%d_%.1f_%.1f", param->kernel, param->sigma_color, param->sigma_space);
				else param->filter_flag = false;
				break;
			default:
				fprintf(stderr, "Unknown option: %s!\n", argv[i]);
				exit(1);
			}
		}
		else {

			for (int j = 0; j < param->obj_num; j++) {
				surv_rate.push_back(atof(argv[i++]));
			}
			param->name = argv[i++];
			param->dir_img = argv[i++];
			param->dir_mask = argv[i++];
			param->Cost = argv[i++];
			param->Seam = argv[i++];

		}
	}

	IMG *img;
	img = new IMG(param);

	cv::Mat mask = cv::imread(param->dir_mask, cv::IMREAD_UNCHANGED);
	if (mask.data == NULL) {
		std::cerr << "Can't open !" << std::endl;
		exit(1);
	}

	std::vector<int> color_num(param->obj_num, 0);

	for (int y = 0; y < img->org_height; y++) {
		for (int x = 0; x < img->org_width; x++) {
			int b = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[0]);
			int g = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[1]);
			int r = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[2]);

			if (param->field[std::make_tuple(r, g, b)]) {
				if (color_num[param->field[std::make_tuple(r, g, b)] - 1] == 0) color_num[param->field[std::make_tuple(r, g, b)] - 1]++;
			}
		}
	}

	param->color_num = accumulate(color_num.begin(), color_num.end(), 0);
	int j = 1;
	for (int i = 0; i < color_num.size(); i++) color_num[i] = color_num[i] > 0 ? j++ : 0;
	for (auto i : color_num) std::cout << i << std::endl;
	std::cout << "color_num : " << param->color_num << std::endl;

	ENERGY *emap;
	emap = new ENERGY(param, img);

	param->mask_weight.resize(param->color_num, 0);
	param->surv_rate.resize(param->color_num);

	for (int y = 0; y < img->org_height; y++) {
		for (int x = 0; x < img->org_width; x++) {
			int b = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[0]);
			int g = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[1]);
			int r = cv::saturate_cast<int>(mask.at<cv::Vec3b>(y, x)[2]);

			if (param->field[std::make_tuple(r, g, b)]) {
				emap->obj_map[color_num[param->field[std::make_tuple(r, g, b)] - 1] - 1][y][x] = 255;
				param->surv_rate[color_num[param->field[std::make_tuple(r, g, b)] - 1] - 1] = surv_rate[param->field[std::make_tuple(r, g, b)] - 1];
			}

		}
	}

	for (int i = 0; i < param->color_num; i++) {
		for (int y = 0; y < img->org_height; y++) {
			emap->obj_map_thr[i] += accumulate(emap->obj_map[i][y].begin(), emap->obj_map[i][y].end(), 0);
		}
		emap->obj_map_thr[i] *= param->surv_rate[i];
	}

	for (int i = 0; i < param->color_num; i++) std::cout << "thr[" << i << "] : " << emap->obj_map_thr[i] << std::endl;

	param->write_param();

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	seamcarving(img, param, emap);
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	std::cout << "SeamCarving OK!!" << std::endl;

	sprintf(param->dir_res, "./resize/uniform_ratio/%s_%s_%s%s.ppm", param->name, param->Cost, param->Seam, param->mode);
	std::cout << param->dir_res << std::endl;
	common::write_img(param->dir_res, img->r_image, img->r_height, img->r_width);

	auto elapsed = std::chrono::duration_cast< std::chrono::milliseconds >(end - start).count();
	std::cout << elapsed << "ms" << std::endl;

	delete(param);
	delete(emap);
	delete(img);

	return 0;
}