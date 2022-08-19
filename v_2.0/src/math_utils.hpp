bool collinear(cv::Point2f p1, cv::Point2f p2, cv::Point2f p3){
	float area = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
	//cout << abs(area) << "\t" << 16 * CV_PI * MAX_RADIUS << "\n";
	if(abs(area) -0 <= 16 * CV_PI * MAX_RADIUS)
		return true;
	else
		return false;
}

// to compute angle at b
float compute_angle(cv::Point2f a, cv::Point2f b, cv::Point2f c){

	cv::Point2f ab = { b.x - a.x, b.y - a.y };
    cv::Point2f cb = { b.x - c.x, b.y - c.y };

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    //return (int) floor(alpha * 180. / pi + 0.5);
    return abs(lrint(alpha * (float)(180.0 / CV_PI)));
}
