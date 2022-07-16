void check_program_arguments(int argc) {
	if(argc != 2) {
		std::cout << "Error! Program usage:" << std::endl;
		std::cout << "./circle_detect image_circles_path" << std::endl;
		std::exit(-1);
	}	
}
void check_if_image_exist(const cv::Mat &img, const std::string &path) {
	if(img.empty()) {
		std::cout << "Error! Unable to load image: " << path << std::endl;
		std::exit(-1);
	}	
}