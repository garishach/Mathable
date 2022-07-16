//Ref for circle detection: https://github.com/sol-prog/OpenCV-red-circle-detection/blob/master/circle_detect.cpp
//Ref for video capture: https://github.com/opencv/opencv/blob/master/samples/cpp/videocapture_basic.cpp
//More Ref for detection in video: https://pycad.co/face-and-circle-detection-using-opencv-with-c/

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include <vector>

#define MIN_CIRCLE_SIZE 100
#define MAX_CIRCLE_SIZE 500
#define PER_DISTORTION_SIZE .20
#define PER_DISTORTIONS .05
#define RASPBERRY_PI_UI_RESOLUTION_X 1920
#define RASPBERRY_PI_UI_RESOLUTION_Y 1080
#define MARKER_DISPLACEMENT 237
#define TEXT_DISPLACEMENT 50
#define TEXT_SIZE 1
#define TEXT_THICKNESS 1
#define TEXT_COLOR_BGR cv::Scalar(255, 255, 255)
#define TEXT_FONT cv::FONT_HERSHEY_SIMPLEX
#define INFO_SIZE 0.5
#define INFO_COLOR_BGR cv::Scalar(0, 255, 0)
#define MENU_MARKER_COLOR_BGR cv::Scalar(0, 0, 255)
#define MARKER_DIAM 2.54 //cm
#define LEFT_MENU_X 120
#define TRIANGLE_MENU_SIZE 4
#define ASSET_SIZE 90

float radi;

using namespace cv;
using namespace std;

#include "image_utils.hpp"
#include "import_screen_assets.hpp"
#include "math_utils.hpp"

int main(int argc, char **argv) {

	cv::Mat bgr_image, hsv_image, canny_image, orig_image, current_image;
	vector<cv::Mat> screens;
	
	cv::Point2f ui_center(RASPBERRY_PI_UI_RESOLUTION_X/2, RASPBERRY_PI_UI_RESOLUTION_Y/2);

	vector<cv::Mat> assets;	
	vector<pair<unsigned, unsigned>> positions;	

	unsigned x, y, w=2;

	std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::RotatedRect minRect;
    int counter, countBads, num_distortions, max_distortion_size;

    cv::Mat lower_red_hue_range, upper_red_hue_range, red_hue_image;	

	cv::Point2f rect_points[4];
	float centerX, centerY, res;
	float side1, side2, minX, maxY, midX, midY, maxX;
	int text_coord_x, text_coord_y;
	vector<float> angles;
	float sum;

	std::vector<cv::Point2f> circles_centers;
	cv::Point2f menu_marker_center;

	bool flag_not_circle;
	unsigned flag_page = 1;
	unsigned option, option_height;
	
	cv::namedWindow("Display Window", cv::WINDOW_AUTOSIZE);	
	//cv::namedWindow("Original Window", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("Canny image", cv::WINDOW_AUTOSIZE);	
	
	VideoCapture vid; //integer (index) or (camera name as )string can be passed here

	vid.open("/dev/video2"); 		//camera name as string or 0 for default camera
	if(!vid.isOpened()){
		std::cout << "cannot open camera";
		return -1;
	}
	
	std::cout << "Get Mathable teach by bringing in a marker\nPress any key to terminate\n";

	// Setup a rectangle to define your region of interest
	cv::Rect my_ROI(352, 168, 1088, 624); //left corner pos x, left corner position y, width, height
	cv::Mat cropped;

	string text;
	unsigned textwidth;

	//screen 1
	import_assets(screens, assets, positions, flag_page);	
	
	text = "Welcome to";
	textwidth = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).width;
	cv::putText(screens[flag_page -1], text, cv::Point(ui_center.x- textwidth/2, positions[1].second - TEXT_DISPLACEMENT), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
	
	text = "Place one dot in the circle to begin";
	textwidth = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).width;
	cv::putText(screens[flag_page -1], text, cv::Point(ui_center.x- textwidth/2, positions[0].second + assets[0].rows + TEXT_DISPLACEMENT), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);

	current_image = screens[flag_page -1].clone();

	//cv::imshow("Display Window", current_image);

	assets.clear();
	positions.clear();

	//cv::waitKey(0);

	//exit(1);

	cv::Scalar color;
		
	while(1){

		// Load video frame wise
		vid >> bgr_image;	
		//bgr_image = cv::imread("2022-07-14-011054.jpg");

		//orig_image = bgr_image.clone();
		
		// Crop the full image to that image contained by the rectangle myROI
		// Note that this doesn't copy the data
		//cv::Mat cropped_ref(bgr_image, my_ROI);
		
		// Copy the data into new matrix
		//croppedRef.copyTo(cropped);
		//resize(cropped, bgr_image, Size(RASPBERRY_PI_UI_RESOLUTION_X, RASPBERRY_PI_UI_RESOLUTION_Y));	

		resize(bgr_image, bgr_image, Size(RASPBERRY_PI_UI_RESOLUTION_X, RASPBERRY_PI_UI_RESOLUTION_Y));	

		cv::medianBlur(bgr_image, bgr_image, 15);
		//cv::namedWindow("blur_image", cv::WINDOW_AUTOSIZE);
		//cv::imshow("blur_image", cropped_ref);

		// Convert input image to HSV
		
		cv::cvtColor(bgr_image, hsv_image, cv::COLOR_BGR2HSV);

		// Threshold the HSV image, keep only the red pixels
		cv::inRange(hsv_image, cv::Scalar(0, 100, 100), cv::Scalar(20, 255, 255), lower_red_hue_range);
		cv::inRange(hsv_image, cv::Scalar(160, 100, 100), cv::Scalar(180, 255, 255), upper_red_hue_range);

		//cv::namedWindow("Threshold lower image", cv::WINDOW_AUTOSIZE);
		//cv::imshow("Threshold lower image", lower_red_hue_range);
		//cv::namedWindow("Threshold upper image", cv::WINDOW_AUTOSIZE);
		//cv::imshow("Threshold upper image", upper_red_hue_range);

		// Combine the above two images		
		cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);
		//cv::namedWindow("Combined threshold images", cv::WINDOW_AUTOSIZE);
		//cv::imshow("Combined threshold images", red_hue_image);

		cv::GaussianBlur(red_hue_image, red_hue_image, cv::Size(9, 9), 2, 2);
		//cv::namedWindow("gaussian_image", cv::WINDOW_AUTOSIZE);
		//cv::imshow("gaussian_image", red_hue_image);

		// Use the Hough transform to detect circles in the combined threshold image

		cv::Canny(red_hue_image,canny_image,80,240,3,0);		
		cv::imshow("Canny image", canny_image);

		cv::findContours(canny_image,contours,hierarchy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

		//std::cout << contours.size() << "\n";    	

    	counter =0;    	
    	
    	for( size_t i = 0; i< contours.size(); i++ )
	    {
	        //Step 5
	        minRect = cv::minAreaRect( contours[i] );
	        minRect.points( rect_points );
	       // std::cout << "contour " << i << "\t";

	        //Step 6
	        if(contours[i].size()>MIN_CIRCLE_SIZE and contours[i].size()< MAX_CIRCLE_SIZE) ///size filteration
	        {	        	
	        	flag_not_circle = false;

	        	side1 = cv::norm(cv::Mat(rect_points[0]), cv::Mat(rect_points[1]));
	        	side2 = cv::norm(cv::Mat(rect_points[1]), cv::Mat(rect_points[2]));
	        	radi = (side1+side2)/4.0; //radius
	        	max_distortion_size = PER_DISTORTION_SIZE*radi;
	        	//std::cout << radi << "\t" << max_distortion_size << "\n";
	        	if(abs(side2 -side1) >= max_distortion_size*4){ //rectangle instead of square; skewed image will look ellipsoid
	        		//std::cout << "rejected cond 2 " << side2 << "\t" << side1 << "\n";

	        		//flag_not_circle = true;
	        		continue;// to next contour
	        	}	        	

	            //Step 7
	            centerX = (rect_points[0].x + rect_points[2].x)/2; //center X coordinate of detected circles
	            centerY = (rect_points[0].y + rect_points[2].y)/2;	//center Y coordinate of detected circles
	            cv::Point2f a(centerX,centerY);
	            

	            //Step 8 and Step 9	           
	            
	            countBads = 0; 
	            num_distortions = PER_DISTORTIONS*contours[i].size();
	            for(int j=0; j<(int)contours[i].size(); j++)
	            {
	                cv::Point2f b(contours[i][j].x,contours[i][j].y);
	                res = cv::norm(cv::Mat(a),cv::Mat(b));
	               // std::cout << res << "\t";
	                if(abs(res -radi) > max_distortion_size){
	                	countBads++;
	                	if(countBads >= num_distortions){
	                		flag_not_circle = true;
	                		//std::cout << "rejected cond 3\n";
	                		break;
	                	}
	                }
	            }
	            if(flag_not_circle)
	            	continue; // to next contour
	            else{
	            	//std::cout << "accepted " << contours[i].size() << "\n";
	            	if(flag_page > 3 and centerX < (LEFT_MENU_X + radi + TEXT_DISPLACEMENT + ASSET_SIZE)){
	            		menu_marker_center = a;
	            		color = MENU_MARKER_COLOR_BGR;
	            	}
	            	else{
	            		circles_centers.push_back(a);
	            		color = INFO_COLOR_BGR;
		                counter++;  
	            	}
	            	for ( int j = 0; j < 4; j++ )
	                {
	                    cv::putText(current_image,std::to_string(counter), a, TEXT_FONT, INFO_SIZE, color, 2);
	                    cv::rectangle(current_image, cv::Point(centerX-w/2, centerY-w/2), cv::Point(centerX+w/2, centerY+w/2), color, 2);
	                    cv::circle(current_image, a, radi, color, 2);
	                } 		                              
	            }
	        }
	        /*
	        else{
	        	//std::cout << "rejected cond 1 " << contours[i].size() <<"\n";
	        }
	        */
	    }
	    //std::cout << counter << "\t";
	    
	    if(flag_page == 1){ 
	    	if(counter == 1){
	    		if(abs(ui_center.x - circles_centers[0].x) < max_distortion_size*4 and abs(ui_center.y - circles_centers[0].y) < max_distortion_size*4){
		    		flag_page = 2; 
		    		import_assets(screens, assets, positions, flag_page);    
		    		text = "Move the dot to select what you want to study...";		
		    		cv::putText(screens[flag_page -1], text, cv::Point(LEFT_MENU_X + TEXT_DISPLACEMENT, LEFT_MENU_X + ASSET_SIZE), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
	    	
			    	assets.clear();
			    	positions.clear();           
		    	}
		    	else{
		    		text = "Great! Now slide dot inside the circle to begin";     		
		    	}
		    }
		    else if(counter>1){		    	
		    	text =  "Too many dots. Lets begin with just this 1"; 
		    }
		    if(flag_page == 1 and counter >= 1){
		    	textwidth = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).width;		    	
		    	cv::putText(current_image, text, cv::Point2f(circles_centers[0].x - radi, circles_centers[0].y + radi + TEXT_DISPLACEMENT), 
	    			TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
		    }
	    }

	    else if(flag_page == 2){//dot on screen, select topic	

	    	if(!counter){//no marker detected	
	    		text = "Sigh! No dot in sight. Lets bring back 1 to explore";
	    	}
	    	else if(counter == 1){	    		
	    		cv::rectangle(current_image, cv::Point(ui_center.x-w/2, ui_center.y- MARKER_DISPLACEMENT-w/2), cv::Point(ui_center.x+w/2, ui_center.y- MARKER_DISPLACEMENT+w/2), TEXT_COLOR_BGR, 2);
	    		
		    	//up
		    	if(abs(ui_center.x - circles_centers[0].x) < max_distortion_size*4 and abs(ui_center.y- MARKER_DISPLACEMENT - circles_centers[0].y) < max_distortion_size*4){
		    		flag_page = 3;     //selected topic as triangle  
		    		import_assets(screens, assets, positions, flag_page);  
				    text = "Place 3 dots to form a traingle...";
					//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
					cv::putText(screens[flag_page -1], text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT, LEFT_MENU_X + assets[0].rows), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
					
					assets.clear();
					positions.clear();             
       
		    	}
		    	//right
		    	else if(abs(ui_center.x +MARKER_DISPLACEMENT - circles_centers[0].x) < max_distortion_size*4 and abs(ui_center.y - circles_centers[0].y) < max_distortion_size*4){		    		
		    		text = "Mathable is upgrading. We are still to prepare learning Isosceles triangles";
		    	}
		    	//down
		    	else if(abs(ui_center.x - circles_centers[0].x) < max_distortion_size*4 and abs(ui_center.y +MARKER_DISPLACEMENT - circles_centers[0].y) < max_distortion_size*4){		    		
		    		text = "Mathable is upgrading. We are still to prepare learning Quadrilaterals";
		    	}
		    	//left
		    	else if(abs(ui_center.x - MARKER_DISPLACEMENT - circles_centers[0].x) < max_distortion_size*4 and abs(ui_center.y - circles_centers[0].y) < max_distortion_size*4){		    		
		    		text = "Mathable is upgrading. We are still to prepare learning Circles";
		    	}
		    	else{
		    		text = "Move dot to top circle to learn traingles";				
		    	}
		    }
		    else{
		    	text = "Too many dots. Lets continue for now with just this 1";				
		    }
		    if(flag_page == 2){
		    	cv::putText(current_image, text, cv::Point2f(circles_centers[0].x - radi, circles_centers[0].y + radi + TEXT_DISPLACEMENT), 
		    		TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
		    }    
	    }

	    else if(flag_page == 3){	 //selected topic as triangle	    	

	    	if(counter < 2){
	    		text = "Too few dots. Need 3 to learn traingles";   		
	    		cv::putText(current_image, text, cv::Point2f(circles_centers[0].x - radi, circles_centers[0].y + radi + TEXT_DISPLACEMENT), 
	    			TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);				
	    	}
	    	if(counter >= 2){ //at least 2 circles detected	
		    	minX = RASPBERRY_PI_UI_RESOLUTION_X; maxY = 0;
		    	for(int i = 0; i < circles_centers.size(); i++){
			    	line( current_image, circles_centers[i], circles_centers[(i+1)%circles_centers.size()], INFO_COLOR_BGR,2 );
			    	minX = min(minX, circles_centers[i].x);
			    	maxY = max(maxY, circles_centers[i].y);
			    }	     	    
			    
			    if(counter == 2){
			    	text = "Two points make a line. Lets add another to make a triangle";
			    }
			    else if(counter > 3){
			    	text = "We only need 3 points to make a traingle. Lets remove 1 to proceed";
			    }
			    else{//counter == 3
			    	if(collinear(circles_centers[0], circles_centers[1], circles_centers[2])){//collinear
			    		text = "All points lie on the same line. Lets move one to make a triangle";
			    	}
			    	else{
			    		flag_page = 4;
			    		import_assets(screens, assets, positions, flag_page); 
			    		line(screens[flag_page -1], cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT, positions[0].second), 
			    			cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT, positions[3].second + assets[3].rows), TEXT_COLOR_BGR, 2 );       
			    	}
			    }
			    if(flag_page == 3)
			    	cv::putText(current_image, text, cv::Point2f(minX - radi, maxY + radi + TEXT_DISPLACEMENT), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
			}
		}
		else if(flag_page == 4){	
			if(counter < 3){
				text = "Too few dots. Need 3 to continue learning triangles";	    		
	    		cv::putText(current_image, text, cv::Point2f(circles_centers[0].x - radi, circles_centers[0].y + radi + TEXT_DISPLACEMENT), 
	    			TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
				
	    	}
	    	else if(counter > 3){
	    		text = "Too many dots. Remove 1 to continue learning triangles";
	    		cv::putText(current_image, text, cv::Point2f(circles_centers[0].x - radi, circles_centers[0].y + radi + TEXT_DISPLACEMENT),
	    			TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
				
	    	}
	    	else{
	    		maxX = 0;
	    		for(int i = 0; i < circles_centers.size(); i++){
			    	line( current_image, circles_centers[i], circles_centers[(i+1)%circles_centers.size()], cv::Scalar(TEXT_COLOR_BGR),2 );	
			    	if(maxX < circles_centers[i].x){
			    		maxX = circles_centers[i].x;
			    		maxY = i;
			    	}
	    		}

				if(!menu_marker_center.x and !menu_marker_center.y){
					overlay_asset(assets[4], current_image, {LEFT_MENU_X, LEFT_MENU_X});
					text = "Great! Now select an option";
					//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
					cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT, LEFT_MENU_X + assets[0].rows/2), TEXT_FONT, INFO_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
					text = "from the menu to proceed...";
					//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
					cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT, LEFT_MENU_X + assets[0].rows), TEXT_FONT, INFO_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
							
				}
				else{//(counter == 3) at least 3 circles detected
					overlay_asset(assets[6], current_image, {LEFT_MENU_X, LEFT_MENU_X});
					option_height = (RASPBERRY_PI_UI_RESOLUTION_Y - LEFT_MENU_X*2)/(TRIANGLE_MENU_SIZE +1);
					option = menu_marker_center.y / option_height;
					switch(option){
						case 1:
							overlay_asset(assets[5], current_image, {LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2, LEFT_MENU_X});
							text = "Sum of any two sides is always greater than the third side.";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows/2), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS*2);
							text = "Move any dot to see this in action";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
							for(int i = 0; i < circles_centers.size(); i++){
								side1 = cv::norm(cv::Mat(circles_centers[i]), cv::Mat(circles_centers[(i+1)%circles_centers.size()]));
						    	side1 = side1 * MARKER_DIAM/(2*radi);
						    	side1 = round(side1 * 100)/100;
						    	midX = (circles_centers[i].x + circles_centers[(i+1)%circles_centers.size()].x)/2;
						    	midY = (circles_centers[i].y + circles_centers[(i+1)%circles_centers.size()].y)/2 ;
						    	cv::putText(current_image,to_string(side1) +" cm", cv::Point2f(maxY, midY), TEXT_FONT, INFO_SIZE ,TEXT_COLOR_BGR,TEXT_THICKNESS);
							}
							break;
						case 2:
							overlay_asset(assets[5], current_image, {LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2, LEFT_MENU_X});
							text = "Sum of all the angles of a traingle is always 180 degrees";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows/2), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS*2);
							text = "Move any dot to see this in action";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
							sum = 0;
							for(int i = 0; i < circles_centers.size(); i++){			    	
						    	angles.push_back(compute_angle(circles_centers[(i-1+circles_centers.size())%circles_centers.size()], circles_centers[i], circles_centers[(i+1)%circles_centers.size()]));
						    	cv::putText(current_image,to_string(angles[i]) + " degrees", cv::Point2f(circles_centers[i].x, circles_centers[i].y - radi),
						    		TEXT_FONT, INFO_SIZE, TEXT_COLOR_BGR,TEXT_THICKNESS);
						    	//cout << circles_centers[i].x << "\t" << circles_centers[i].y << "\t" << angles[i] << "\t";
						    	x = cv::getTextSize("degrees", TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).height;
						    	if(maxX + 2*TEXT_DISPLACEMENT > RASPBERRY_PI_UI_RESOLUTION_X and circles_centers[maxY].y > RASPBERRY_PI_UI_RESOLUTION_Y/2)
						    		y = RASPBERRY_PI_UI_RESOLUTION_Y/2 - LEFT_MENU_X;
						    	else
						    		y = RASPBERRY_PI_UI_RESOLUTION_Y/2 + LEFT_MENU_X;
						    	cv::putText(current_image,to_string(angles[i]) +" degrees", cv::Point2f(RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2,y + i*x+2),
						    	 TEXT_FONT, INFO_SIZE ,TEXT_COLOR_BGR,TEXT_THICKNESS);
						    	sum += angles[i];
						    }
						    angles.clear();
						    textwidth = cv::getTextSize(to_string(sum) +" degrees", TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
						    line( current_image, cv::Point2f(RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2, y + 2*x+2), 
						    	cv::Point2f(RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2 + textwidth, y + 2*x+2), INFO_COLOR_BGR,2 );

						    cv::putText(current_image,to_string(sum) +" degrees", cv::Point2f(RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2,y + 3*x+4),
						    	TEXT_FONT, INFO_SIZE ,TEXT_COLOR_BGR,TEXT_THICKNESS);

							break;
						case 0:
							flag_page = 2;
							break;
						default:
							text = "Mathable is preparing to learn more about about traingles.";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows/2), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
							text = "Lets go with the first 2 options for now.";
							//textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
							cv::putText(current_image, text, cv::Point(LEFT_MENU_X + assets[0].cols + TEXT_DISPLACEMENT*2 + assets[5].cols, LEFT_MENU_X + assets[0].rows), TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
					}			
				}
			}	
		}
		cv::imshow("Display Window", current_image);

		if(counter || flag_page > 1){
			current_image = screens[flag_page -1].clone();
			if(flag_page == 4)
				menu_marker_center.x = menu_marker_center.y = 0;
		}
	    
		// Show images
		//cv::imshow("Original Window", orig_image);

		circles_centers.clear();
		

		//cv::waitKey(0);

		if(cv::waitKey(5) >=0)
			break;

	}	
	return 0;
}