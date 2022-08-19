void overlay_asset(cv::Mat asset, cv::Mat & image, pair<unsigned, unsigned> & position){
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
	
	switch(screen){
		case 1:
			path_assets.push_back("../Mathable_UI_Assets/Screen_1/Logo.png"); //0
			break;
		case 2:
			path_assets.push_back("../Mathable_UI_Assets/Screen_6/BulbIcon.png"); //0	
			break;
		case 3:
			path_assets.push_back("../Mathable_UI_Assets/Screen_6/BulbIcon.png"); //0	
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon1.png"); //1
			path_assets.push_back("../Mathable_UI_Assets/Screen_5/SideIcon2.png"); //2
			break;
	}

	//reading all assets for corresponding screen
	for(unsigned i = 0; i < path_assets.size(); i++){
		float rescale;
		if(screen ==3 or i == 0)
			rescale = 2.25;
		else
			rescale = 3;
		assets.push_back(cv::imread(path_assets[i], cv::IMREAD_UNCHANGED));		
		resize(assets[i], assets[i], Size(assets[i].cols/rescale, assets[i].rows/rescale));
		check_if_image_exist(assets[i], path_assets[i]);		
	}

	//defining positions for each of those assets	
	switch(screen){
		case 1:
			positions.push_back({center_x-assets[0].cols/2, 3/8.0* RASPBERRY_PI_UI_RESOLUTION_Y - assets[0].rows}); //logo
			break;
		case 2:
			positions.push_back({TEXT_DISPLACEMENT, LEFT_MENU_X/3}); //bulb icon
			break;
		case 3:
			positions.push_back({TEXT_DISPLACEMENT, LEFT_MENU_X/3}); //bulb icon
			unsigned x = (RASPBERRY_PI_UI_RESOLUTION_Y - LEFT_MENU_X*2)/(TRIANGLE_MENU_SIZE +1 );
			for(unsigned i = 0; i < TRIANGLE_MENU_SIZE; i++){
				positions.push_back({RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X - assets[1].rows, x*(i+1) + assets[1].cols}); //1-4
			}
			break;
	}


	//overlaying the assets at defined positions on given image
	for(unsigned i = 0; i < positions.size(); i++){
		overlay_asset(assets[i], blank_image, positions[i]);		
	}
	screens[screen -1] =blank_image;
	
}
