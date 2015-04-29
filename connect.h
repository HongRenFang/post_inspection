//connect.h
//header file of class connect

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class connected
{
	public:
		void FindBlobs(cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs);
        void drawlabel(cv::Mat &output, std::vector < std::vector<cv::Point2i> > &blobs);
		void drawlabel_bin(cv::Mat &output, std::vector < std::vector<cv::Point2i> > &blobs);
        void analysisline( std::vector < std::vector<cv::Point2i> > &blobs,Mat &img,Mat &source);//,double **dis);
        void sort(float *arr,int *index, int n);
		void blob_sort(std::vector < std::vector<cv::Point2i> > &blobs,int *arr);
        double distance(double x1,double y1,double x2, double y2);
		void neighborPoint(double slope,double x0,double y0,double sol[4]);
		int x;
		int y;
		int w,h;
		///analysisline***
	    int *sorting;
	    unsigned int blobsize;
	    unsigned int amount;//total number of blobs pixels
	    unsigned int amount5;//the bigger 5 components

	    int *xcenter,*ycenter; //mass center
	    float *p,*p2;//probability
	    float propotion_biggest2;
	    float avg_x,avg_y;//expected value
	    float exp_x,exp_y;//expected value
	    float mul_xy,mul_xx;//sum
	    float a,b;//Regression Line equation y=ax+b
	    float *dis;//distance to Line

	    unsigned int graylevel[256];float pro_gray[256];
	    float mean,var,sigma;//mean , variance of local region
	    float var2  ,sigma2;//variance and sigma of ycenter
	    unsigned int roi_area;
		float roi_value;
		///***analysisline
};


