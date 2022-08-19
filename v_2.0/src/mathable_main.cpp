//Ref for circle detection: https://github.com/sol-prog/OpenCV-red-circle-detection/blob/master/circle_detect.cpp
//Ref for video capture: https://github.com/opencv/opencv/blob/master/samples/cpp/videocapture_basic.cpp
//More Ref for detection in video: https://pycad.co/face-and-circle-detection-using-opencv-with-c/
//circularity = (4*PI*A)/P^2

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#define MIN_CIRCLE_SIZE 60
#define MAX_CIRCLE_SIZE 110
#define MAX_RADIUS 16
#define PER_DISTORTION_SIZE .20
#define PER_DISTORTIONS .05
#define RASPBERRY_PI_UI_RESOLUTION_X 640
#define RASPBERRY_PI_UI_RESOLUTION_Y 480
#define TEXT_DISPLACEMENT 17
#define TEXT_SIZE 0.5
#define TEXT_THICKNESS 1
#define TEXT_COLOR_BGR cv::Scalar(255, 255, 255)
#define TEXT_FONT cv::FONT_HERSHEY_SIMPLEX
#define INFO_SIZE 0.5
#define INFO_COLOR_BGR cv::Scalar(0, 255, 0)
#define INFO_THICKNESS 1
#define MENU_MARKER_COLOR_BGR cv::Scalar(255, 0, 0)
#define MARKER_DIAM 3.25 //cm
#define LEFT_MENU_X 50
#define TRIANGLE_MENU_SIZE 2

float radi;

using namespace cv;
using namespace std;

#include "image_utils.hpp"
#include "import_screen_assets.hpp"
#include "math_utils.hpp"

int main(int argc, char **argv) {

	cv::Mat bgr_image, hsv_image, canny_image, orig_image, current_image;
	vector<cv::Mat> screens(3);
	vector<bool> flag_set = {0,0,0};

	std::vector<cv::Vec3f> circles;
	
	cv::Point2f ui_center(RASPBERRY_PI_UI_RESOLUTION_X/2, RASPBERRY_PI_UI_RESOLUTION_Y/2);

	vector<cv::Mat> assets;	
	vector<pair<unsigned, unsigned>> positions;	

	unsigned x, y, w=2;

	std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::RotatedRect minRect;

    int counter, countBads, num_distortions, max_distortion_size;

    //cv::Mat lower_red_hue_range, upper_red_hue_range, red_hue_image;	
    cv::Mat green_hue_image;

	cv::Point2f rect_points[4];
	float centerX, centerY, res;
	float side1, side2, minX, maxY, midX, midY, maxX;
	float eccentricity;
	int text_coord_x, text_coord_y;
	vector<int> angles;
	int sum;
	int side;

	std::vector<cv::Point2f> circles_centers;
	cv::Point2f menu_marker_center;

	bool flag_not_circle;
	unsigned flag_page = 1;
	unsigned option, option_height;

	vector<int> values;
	
	cv::namedWindow("Display Window", cv::WINDOW_AUTOSIZE);	
	//cv::namedWindow("Original Window", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("Canny image", cv::WINDOW_AUTOSIZE);	
	//cv::namedWindow("Threshold lower image", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("Threshold upper image", cv::WINDOW_AUTOSIZE);
	
	VideoCapture vid; //integer (index) or (camera name as )string can be passed here
	//sleep(60);
	
	vid.open(0); 		//camera name as string or 0 for default camera
	if(!vid.isOpened()){
		std::cout << "cannot open camera";
		return -1;
	}
	vid.set(CAP_PROP_FRAME_WIDTH, RASPBERRY_PI_UI_RESOLUTION_X);
	vid.set(CAP_PROP_FRAME_HEIGHT, RASPBERRY_PI_UI_RESOLUTION_Y);	
	
	std::cout << "Get Mathable teach by bringing in a marker\nPress any key to terminate\n";

	// Setup a rectangle to define your region of interest
	cv::Rect my_ROI(0.21*RASPBERRY_PI_UI_RESOLUTION_X , 0.27*RASPBERRY_PI_UI_RESOLUTION_Y, 0.58*RASPBERRY_PI_UI_RESOLUTION_X, 0.58*RASPBERRY_PI_UI_RESOLUTION_Y); //left corner pos x, left corner position y, width, height
	cv::Mat cropped;

	string text1, text2, text;
	unsigned textpos1, textpos2;
	unsigned textwidth, textheight;

	//screen 1
	import_assets(screens, assets, positions, flag_page);	
	flag_set[flag_page-1] = 1;	

	textheight = cv::getTextSize("H", TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).height;

	text = "Welcome to";
	textwidth = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).width;
	cv::putText(screens[flag_page -1], text, cv::Point(ui_center.x- textwidth/2, positions[0].second - TEXT_DISPLACEMENT),
	 TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
	cv::circle(screens[flag_page -1], cv::Point(ui_center.x, ui_center.y), 26, TEXT_COLOR_BGR, 2);

	text = "Place one dot in the circle to begin";
	textwidth = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).width;
	textheight = cv::getTextSize(text, TEXT_FONT, TEXT_SIZE, TEXT_THICKNESS,0).height;

	cv::putText(screens[flag_page -1], text, cv::Point(ui_center.x- textwidth/2, 5/8.0* RASPBERRY_PI_UI_RESOLUTION_Y + textheight), 
		TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);

	current_image = screens[flag_page -1].clone();	

	assets.clear();
	positions.clear();	

	cv::Scalar color;

	int morph_size = 2;
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, 
    	Size(2 * morph_size + 1, 2 * morph_size + 1), 
    	cv::Point(morph_size, morph_size));
    	
    int y_pixel_disp = -0.16 * RASPBERRY_PI_UI_RESOLUTION_Y;

    textpos1 = LEFT_MENU_X/2;
    textpos2 = LEFT_MENU_X;
		
	while(1){

		// Load video frame wise
		vid >> bgr_image;	
		//vid.grab();
		//vid.retrieve(bgr_image);
		//bgr_image = cv::imread("test_images/3.jpg");
		//cout << bgr_image.rows << "\t" << bgr_image.cols << "\n";

		orig_image = bgr_image.clone();
		//resize(orig_image, orig_image, Size(320, 240));
		//cv::imshow("Original Window", orig_image);		
		
		// Crop the full image to that image contained by the rectangle myROI
		// Note that this doesn't copy the data
		cv::Mat cropped_ref(bgr_image, my_ROI);
		
		// Copy the data into new matrix
		cropped_ref.copyTo(cropped);
		resize(cropped, bgr_image, Size(RASPBERRY_PI_UI_RESOLUTION_X, RASPBERRY_PI_UI_RESOLUTION_Y));		

		//resize(bgr_image, bgr_image, Size(RASPBERRY_PI_UI_RESOLUTION_X, RASPBERRY_PI_UI_RESOLUTION_Y));	

		cv::medianBlur(bgr_image, bgr_image, 15);
		//cv::namedWindow("blur_image", cv::WINDOW_AUTOSIZE);
		//cv::imshow("blur_image", cropped_ref);

		// Convert input image to HSV
		
		cv::cvtColor(bgr_image, hsv_image, cv::COLOR_BGR2HSV);

		// Threshold the HSV image, keep only the red pixels
		//cv::inRange(hsv_image, cv::Scalar(0, 100, 80), cv::Scalar(10, 255, 255), lower_red_hue_range);
		//cv::inRange(hsv_image, cv::Scalar(170, 100, 100), cv::Scalar(180, 255, 255), upper_red_hue_range);
		cv::inRange(hsv_image, cv::Scalar(42, 80, 40), cv::Scalar(72, 255, 255), green_hue_image);

		//cv::imshow("Threshold lower image", lower_red_hue_range);		
		//cv::imshow("Threshold upper image", upper_red_hue_range);

		// Combine the above two images		
		//cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);
		//cv::imshow("Combined threshold images", green_hue_image);

		cv::GaussianBlur(green_hue_image, green_hue_image, cv::Size(9, 9), 2, 2);
		//cv::imshow("gaussian_image", red_hue_image);

		// Use the Hough transform to detect circles in the combined threshold image
		
		cv::adaptiveThreshold(green_hue_image, canny_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 11, 5);
		//cv::Canny(red_hue_image,canny_image,80,240,3,0);		
		//cv::imshow("Canny image", canny_image);

		//cv::erode(canny_image, canny_image, element, Point(-1, -1), 1);
		//cv::imshow("Eroded image", canny_image);		
		//cv::imshow("Dilated", canny_image);
		cv::erode(canny_image, canny_image, element, Point(-1, -1), 1);
		cv::dilate(canny_image, canny_image, element, Point(-1, -1), 1);
		cv::erode(canny_image, canny_image, element, Point(-1, -1), 1);

		//cv::imshow("Eroded", canny_image);

		counter =0;  

	    cv::findContours(canny_image,contours,hierarchy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

		//std::cout << contours.size() << "\n";    	 	
    	
    	for( size_t i = 0; i< contours.size(); i++ )
	    {
	        //Step 6
	        if(contours[i].size()>MIN_CIRCLE_SIZE and contours[i].size()< MAX_CIRCLE_SIZE) ///size filteration    	
	    	{	
		        minRect = cv::fitEllipse( contours[i]);	            
	            side1 = minRect.size.width; //width <=height
	            side2 = minRect.size.height;
	            eccentricity = sqrt ( 1- round(pow((side1/side2),2) * 100)/100);
	            //cout << side1 << "\t" << side2 << "\t" <<eccentricity << endl;

	            if(eccentricity < 1){

	            	cv::Point2f a= minRect.center; // center
	            	a.y = a.y + y_pixel_disp;
		            radi = (side1+side2)/4.0 * 0.58;
		            
		            if(a.x > (RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2 - TEXT_DISPLACEMENT)){
	            		menu_marker_center = a;
	            		color = MENU_MARKER_COLOR_BGR;
	            		text = "M";
	            		if(flag_page < 3)
	            			continue;
	            	}
	            	else{
	            		circles_centers.push_back(a);
	            		color = INFO_COLOR_BGR;
	            		text = string() + char(65 + counter);
		                counter++;  
	            	}
                    cv::putText(current_image, text , a, TEXT_FONT, INFO_SIZE, color, INFO_THICKNESS);
                    cv::rectangle(current_image, cv::Point(a.x-w/2, a.y-w/2), cv::Point(a.x+w/2, a.y+w/2), color, INFO_THICKNESS);
                    cv::circle(current_image, a, radi, color, INFO_THICKNESS);
                }
		    }
	    } 
	   
	    if(counter >= 2){
	    	for(int i = 0; i < circles_centers.size(); i++){
			    line( current_image, circles_centers[i], circles_centers[(i+1)%circles_centers.size()], cv::Scalar(TEXT_COLOR_BGR),2 );	
			}
		}
	    

	    text1 = "";
	    text2 = "";

	    if(flag_page == 1 and counter == 1){ 
    		if(abs(ui_center.x - circles_centers[0].x) < TEXT_DISPLACEMENT and abs(ui_center.y - circles_centers[0].y) < TEXT_DISPLACEMENT){
	    		flag_page = 2; 	    		    
	    	}
	    }

	    else if(flag_page == 2 and (counter == 1 or counter == 2)){
	    	text1 = text2 = "";
	    	if(counter ==1){
	    		text1 = "Great! ";	
	    		text2 = "Now lets bring another to see some action!";
	    	}
	    	else if(counter == 2){	    		
	    		text1 = "Two dots make a line.";
	    		text2 = "Add another..";
	    	}	    	     
	    }

	    else if(flag_page == 3 and counter == 3){//selected topic as triangle	    	
	    			    
	    	if(collinear(circles_centers[0], circles_centers[1], circles_centers[2])){//collinear
	    		text1 = "All points lie on the same line.";
	    		text2 = "Lets move one to make a triangle";
	    	}
	    	else if(!menu_marker_center.x and !menu_marker_center.y){
	    		text1 = "Great! Now select an option";
	    		text2 = "from the menu on right to proceed...";
	    	}				
			else{//(counter == 3) at least 3 circles detected
				option_height = (RASPBERRY_PI_UI_RESOLUTION_Y - LEFT_MENU_X - TEXT_DISPLACEMENT*2)/(TRIANGLE_MENU_SIZE +1);
				option = (menu_marker_center.y - TEXT_DISPLACEMENT)/ option_height;
				x = cv::getTextSize("deg", TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).height;					    	
				text_coord_x = RASPBERRY_PI_UI_RESOLUTION_X - TEXT_DISPLACEMENT;
				switch(option){
					case 1:
						text1 = "Sum of any two sides is always greater than the third side.";
						text2 = "Move any dot to see this in action";
						values.clear();
						for(int i = 0; i < circles_centers.size(); i++){
							side = cv::norm(cv::Mat(circles_centers[i]), cv::Mat(circles_centers[(i+1)%circles_centers.size()]));
					    	side = side * MARKER_DIAM/(2*radi*0.58);
					    	side = (int)round((side * 100)/100);
					    	midX = (circles_centers[i].x + circles_centers[(i+1)%circles_centers.size()].x)/2 + 5;
					    	midY = (circles_centers[i].y + circles_centers[(i+1)%circles_centers.size()].y)/2 - 5 ;
					    	cv::putText(current_image,to_string(side), cv::Point2f(midX, midY), TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR, INFO_THICKNESS);
					    	values.push_back(side);
					    	text = std::string() + char(65 + i) +  char(65 + (i+1)%3) + " ( " + to_string(values[i]) + " )";
					    	textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
					    	cv::putText(current_image, text, cv::Point2f(text_coord_x  - textwidth, LEFT_MENU_X + i*x*3/2.0),
					    	 TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR,TEXT_THICKNESS);
					    	if(i == 0)
					    		text = "";
					    	else if(i == (circles_centers.size() -1))
					    		text = ">";
					    	else
					    		text = "+";
					    		cv::putText(current_image, text, cv::Point2f(text_coord_x - textwidth- textwidth/4, LEFT_MENU_X+ i*x*3/2.0),
					    	 TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR,TEXT_THICKNESS);
						}
						break;
					case 2:
						text1 = "Sum of all the angles of a traingle is always 180 degrees";
						text2 = "Move any dot to see this in action";
						sum = 0;
						for(int i = 0; i < circles_centers.size(); i++){			    	
					    	angles.push_back(round(compute_angle(circles_centers[(i-1+circles_centers.size())%circles_centers.size()], circles_centers[i], circles_centers[(i+1)%circles_centers.size()])));
					    	cv::putText(current_image,to_string(angles[i]) + " deg", cv::Point2f(circles_centers[i].x, circles_centers[i].y - radi - textheight),
					    		TEXT_FONT, INFO_SIZE, INFO_COLOR_BGR,TEXT_THICKNESS);
					    	//cout << circles_centers[i].x << "\t" << circles_centers[i].y << "\t" << angles[i] << "\t";
					    	text = to_string(angles[i]) + " deg";	
					    	textwidth = cv::getTextSize(text, TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
				    		
					    	cv::putText(current_image, text, cv::Point2f(text_coord_x  - textwidth, LEFT_MENU_X + i*x*3/2.0),
					    	 TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR,TEXT_THICKNESS);
					    	if(i == circles_centers.size() -1){
					    		cv::putText(current_image, "+", cv::Point2f(text_coord_x -textwidth -textwidth/4, LEFT_MENU_X+ i*x*3/2.0),
					    	 TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR,TEXT_THICKNESS);
					    	}
					    	sum += angles[i];
					    }
					    angles.clear();
					    textwidth = cv::getTextSize(to_string(sum) +" deg", TEXT_FONT, INFO_SIZE, TEXT_THICKNESS,0).width;
					    line( current_image, cv::Point2f( text_coord_x  - textwidth - textwidth/4, LEFT_MENU_X + x*4), 
					    	cv::Point2f(text_coord_x , LEFT_MENU_X + x*4), TEXT_COLOR_BGR,2 );

					    cv::putText(current_image,to_string(sum) +" deg", cv::Point2f(text_coord_x  - textwidth , LEFT_MENU_X + x*6),
					    	TEXT_FONT, INFO_SIZE ,INFO_COLOR_BGR,TEXT_THICKNESS);

						break;
				}
			}	
		}	 	
		if(!counter)
			flag_page = 1;
		else if(counter == 1){//no marker detected
			if(!(flag_page == 2))	
    			flag_page = 1;
    	}
    	else if(counter == 2){
    		flag_page = 2;		
    	}
    	else if(counter > 3){
    		text1 = "Too many dots.";
	    	text2 = "Just 3 in the drawing area for now please..";	
    	}
    	else if(counter == 3){
	    	flag_page = 3;     //selected topic as triangle 
	    }	

    	if(text1.size()){
	    	cv::putText(current_image, text1, cv::Point2f(LEFT_MENU_X + TEXT_DISPLACEMENT, LEFT_MENU_X/2), 
	    		TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
	 	}
	    if(text2.size()){
	    	cv::putText(current_image, text2, cv::Point2f(LEFT_MENU_X + TEXT_DISPLACEMENT, LEFT_MENU_X), 
	    		TEXT_FONT, TEXT_SIZE, TEXT_COLOR_BGR, TEXT_THICKNESS);
	    }    
		cv::imshow("Display Window", current_image);

		if(counter || flag_page > 1){
			if(!flag_set[flag_page -1]){
				import_assets(screens, assets, positions, flag_page); 
				if(flag_page > 1){
					cv::rectangle(screens[flag_page -1], cv::Point(TEXT_DISPLACEMENT, LEFT_MENU_X + TEXT_DISPLACEMENT), 
						cv::Point(RASPBERRY_PI_UI_RESOLUTION_X - LEFT_MENU_X*2 - TEXT_DISPLACEMENT, RASPBERRY_PI_UI_RESOLUTION_Y - TEXT_DISPLACEMENT), cv::Scalar(0, 255, 0), 2);

					//line(screens[flag_page -1], cv::Point( positions[1].first - TEXT_DISPLACEMENT, LEFT_MENU_X*2), 
					//	cv::Point(positions[1].first - TEXT_DISPLACEMENT, RASPBERRY_PI_UI_RESOLUTION_Y - LEFT_MENU_X), TEXT_COLOR_BGR, INFO_THICKNESS); 	    	
				}
		    	assets.clear();
		    	positions.clear();
		    	flag_set[flag_page-1] = 1;
		    }
			
			if(flag_page == 3)
				menu_marker_center.x = menu_marker_center.y = 0;

		}
		current_image = screens[flag_page -1].clone();
	    
		// Show images
		

		circles_centers.clear();
		

		//cv::waitKey(0);

		if(cv::waitKey(5) >=0)
			break;

	}	
	return 0;
}
