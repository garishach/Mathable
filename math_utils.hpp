bool collinear(cv::Point2f p1, cv::Point2f p2, cv::Point2f p3){
	float area = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
	if(area == 0)
		return true;
	else
		return false;
}

// to compute angle at b
float compute_angle(cv::Point2f a, cv::Point2f b, cv::Point2f c){
	float m1 = (a.y - b.y)/(a.x - b.x);
	float m2 = (c.y - b.y)/(c.x - b.x);
	float angle = std::atan((m2 - m1)/ (1+ m1*m2));
	return round(abs(angle*180/ CV_PI)*100)/100;
}