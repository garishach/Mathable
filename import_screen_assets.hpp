void overlay_asset(cv::Mat asset, cv::Mat & image, pair<unsigned, unsigned> position){
	cv::Mat layers[4];

	cv::split(asset, layers); //4 layers- r,g,b, and transperancy
	cv::merge(layers, 3, asset); //converted to non-transperant image with only 3 layers

	//use tranperancy layer as a mask when copying the 3 layered asset
	asset.copyTo(image(cv::Rect(position.first, position.second, asset.cols, asset.rows)), layers[3]);
}

void import_assets(vector<cv::Mat> & screens, vector<cv::Mat> & assets, vector<pair<unsigned, unsigned>> & positions, unsigned screen){
	
	vector<string> path_assets;	
	cv::Mat blank_image = cv::Mat(Size(RASPBERRY_PI_UI_RESOLUTION_X, RASPBERRY_PI_UI_RESOLUTION_Y), CV_8UC3, Scalar(0,0,0));

	unsigned center_x = RASPBERRY_PI_UI_RESOLUTION_X/2;
	unsigned center_y = RASPBERRY_PI_UI_RESOLUTION_Y/2;

	//listing paths of assests for the specific screens
	switch(screen){
		case 1:
			path_assets.push_back("../Mathable_UI_Assets/Screen_1/Asset1.png"); //0
			path_assets.push_back("../Mathable_UI_Assets/Screen_1/Logo.png"); //1
			break;
		case 2:
			path_assets.push_back("../Mathable_UI_Assets/Screen_2/Combined_Asset.png"); //0
			break;
		case 3:
			path_assets.push_back("../Mathable_UI_Assets/Screen_4/Top_Left_icon.png"); //0
			break;
		case 4:			
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon1.png"); //0
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon2.png"); //1
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon3.png"); //2
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon4.png"); //3
			path_assets.push_back("../Mathable_UI_Assets/Screen_4/Top_Left_icon.png"); //4
			path_assets.push_back("../Mathable_UI_Assets/Screen_6/BulbIcon.png"); //5
			path_assets.push_back("../Mathable_UI_Assets/Screen_6/BackButton.png"); //6
			break;
	}

	//reading all assets for corresponding screen
	for(unsigned i = 0; i < path_assets.size(); i++){
		assets.push_back(cv::imread(path_assets[i], cv::IMREAD_UNCHANGED));
		check_if_image_exist(assets[i], path_assets[i]);		
	}

	//defining positions for each of those assets
	switch(screen){
		case 1:
			positions.push_back({center_x-assets[0].cols/2, center_y-assets[0].rows/2}); //0 at x-center, y-center
			positions.push_back({center_x-assets[1].cols/2, positions[0].second- assets[1].rows - TEXT_DISPLACEMENT}); //1 at x-center, y-above asset[0]
			break;
		case 2:
			positions.push_back({center_x-assets[0].cols/2, center_y-assets[0].rows/2}); //0 at x-center, y-center
			break;
		case 3:
			positions.push_back({LEFT_MENU_X, LEFT_MENU_X}); //0 at left top corner
			break;
		case 4:
			unsigned x = (RASPBERRY_PI_UI_RESOLUTION_Y - LEFT_MENU_X*2)/(TRIANGLE_MENU_SIZE +1);
			positions.push_back({LEFT_MENU_X, LEFT_MENU_X + x}); //0 at left top corner
			for(unsigned i = 0; i < TRIANGLE_MENU_SIZE-1; i++){
				positions.push_back({LEFT_MENU_X, positions[i].second + x}); //1-4
			}
			//positions.push_back({LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2, LEFT_MENU_X}); //5			
			break;
	}

	//overlaying the assets at defined positions on given image
	for(unsigned i = 0; i < positions.size(); i++){
		overlay_asset(assets[i], blank_image, positions[i]);		
	}
	screens.push_back(blank_image);
	
}