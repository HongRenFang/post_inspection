//member function definition

#include<iostream>
#include <qdebug.h>
#include "connect.h"
#include <omp.h>
using namespace std; 
using namespace cv;
///find connected component
void connected::FindBlobs(cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs)
{
    blobs.clear();
    // Fill the label_image with the blobs
    // 0  - background
    // 1  - unlabelled foreground (before label)
    // 2+ - labelled foreground

    cv::Mat label_image;
    binary.convertTo(label_image, CV_32SC1);//32bits image

    int label_count = 2; // pixel value , starting at 2 because 0(black),1(white) are already used
	//New value of the repainted domain pixels
    for(int y=0; y < label_image.rows; y++) {
        int *row = (int*)label_image.ptr(y);///
        for(int x=0; x < label_image.cols; x++) {
            if(row[x] != 255) {
                continue;
            }

            cv::Rect rect;//rectangle
			//connected component
            cv::floodFill(label_image, cv::Point(x,y), label_count, &rect, 0, 0, 4);

            std::vector <cv::Point2i> blob;//new blobs

			//for same label value
            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                int *row2 = (int*)label_image.ptr(i);///
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != label_count) {
                        continue;
                    }

                    blob.push_back(cv::Point2i(j,i));//same label value ,push to same vector
                }
            }

            blobs.push_back(blob);//number of blobs ++
            label_count++;//next label value
        }
    }
}
///components labeling
void connected::drawlabel(cv::Mat &output, std::vector < std::vector<cv::Point2i> > &blobs)
{
	// Random coloring the blobs with rgb color
	
    for(size_t i=0; i < blobs.size(); i++) 
	{
        unsigned char r = 255 * (rand()/(1.0 + RAND_MAX));
        unsigned char g = 255 * (rand()/(1.0 + RAND_MAX));
        unsigned char b = 255 * (rand()/(1.0 + RAND_MAX));

		//if(blobs[i].size()>5)
        for(size_t j=0; j < blobs[i].size(); j++) 
		{
            int x = blobs[i][j].x;//position
            int y = blobs[i][j].y;

            output.at<cv::Vec3b>(y,x)[0] = 255;//b;//assign color to image
            output.at<cv::Vec3b>(y,x)[1] = 255;//g;
            output.at<cv::Vec3b>(y,x)[2] = 255;//r;
			
        }
    }
}
void connected::drawlabel_bin(cv::Mat &output, std::vector < std::vector<cv::Point2i> > &blobs)
{
    for(size_t i=0; i < blobs.size(); i++) 
	{
		if(blobs[i].size()<20)
        for(size_t j=0; j < blobs[i].size(); j++) 
		{
            int x = blobs[i][j].x;//position
            int y = blobs[i][j].y;
            output.at<uchar>(y,x)= 0;//b;//assign color to image			
        }
    }
}
///
/// Main analysis function
///
void connected::analysisline(std::vector <std::vector<cv::Point2i>> &blobs,Mat &imgray,Mat &source)//,double **dis)
{
	//int *sorting;
	blobsize=5;
	///amount=0;//total number of blobs pixels
	amount5=0;//the bigger 5 components
	//int *xcenter,*ycenter; //mass center
	//double *p,*p2;//probability
	propotion_biggest2=0;
	//avg_x=0.0,avg_y=0.0;//expected value
	exp_x=0.0,exp_y=0.0;//expected value
	mul_xy=0.0,mul_xx=0.0;//sum
	a=0.0,b=0.0;//Regression Line equation y=ax+b
	//double *dis;//distance to Line
	mean=0.0 ,var = 0.0 ,sigma=0.0;//mean , variance of local region
	var2 = 0.0 ,sigma2=0.0;//variance and sigma of ycenter
	roi_area=0,roi_value=0;
	w=720;h=480;
	memset( graylevel, 0, 256*sizeof(unsigned int) );
	memset( pro_gray, 0, 256*sizeof(float) );


	if(blobs.size()<blobsize)
		blobsize=blobs.size();
//*****************************************************************************
///============================================================================
///1. sorting
///============================================================================	
//*****************************************************************************
	sorting = new int [blobs.size()];
	for(int i=0;i<blobs.size();i++)
	{
		sorting[i]=i;
	}	
	blob_sort(blobs,sorting);///sort from big to small
//*****************************************************************************
///============================================================================
///2. mass center
///============================================================================	
//*****************************************************************************
	///initial dynamic array of mass center	
    xcenter = new int [blobs.size()];//dynamic array
	ycenter = new int [blobs.size()];//dynamic array
	p= new float [blobs.size()];//dynamic array
	//mass center
	for(size_t i=0; i < blobsize; i++) 
	{
		///mass center
		int xi=0,yi=0;//blobs pixels coordinate
        for(size_t j=0; j < blobsize; j++) 
		{
			xi=xi+blobs[sorting[i]][j].x;
			yi=yi+blobs[sorting[i]][j].y;    
        }
		///center of every blobs
		xcenter[sorting[i]]=xi/blobsize;//x center
		ycenter[sorting[i]]=yi/blobsize;//y center	 

		amount5=amount5+blobs[sorting[i]].size();
    }
	///by prob.
	for(size_t i=0; i < blobsize; i++) 
	{
		p[i]=(float)blobs[sorting[i]].size()/amount5;
		qDebug()<<"p["<<i<<"]"<<p[i];
	}

	///propotion of biggest two component

//*****************************************************************************
///============================================================================
///3. separate to 2 or 5 component	
///============================================================================	
//*****************************************************************************
	propotion_biggest2 = p[0]+p[1];
	qDebug()<<"propotion"<<propotion_biggest2;
	///if the biggest 2 is too large or too many components
	if(propotion_biggest2>0.5 || blobs.size()>10)
	{
		blobsize=2;
		p2 = new float [blobsize];//dynamic array
		exp_x=0;
		exp_y=0;

		p2[0]=p[0]/(p[0]+p[1]);
		p2[1] = 1-p2[0];
		qDebug()<<p2[0]<<p2[1];
		for(size_t i=0; i < 2 ; i++) 
	    {		 
		   exp_x+=xcenter[sorting[i]]*p2[i];//Expectation of x
		   exp_y+=ycenter[sorting[i]]*p2[i];//Expectation of y			
	    }
		qDebug()<<"exp_x"<<exp_x<<"exp_x"<<exp_y;
	}
	///for 5 components
	else
	{
		for(size_t i=0; i < blobsize; i++) 
	    {
		    exp_x+=xcenter[sorting[i]]*p[i];
		    exp_y+=ycenter[sorting[i]]*p[i];
	    }
	}
//*****************************************************************************
///============================================================================
///4.Regression line equation y=ax+b	
///============================================================================
//*****************************************************************************
	for(size_t i=0; i < blobsize; i++) 
	{
		mul_xy+=((float)xcenter[sorting[i]]-exp_x)*((float)ycenter[sorting[i]]-exp_y);
		mul_xx+=((float)xcenter[sorting[i]]-exp_x)*((float)xcenter[sorting[i]]-exp_x);	
	}
	a=mul_xy/mul_xx;//slope
	b=exp_y-(a)*exp_x;
	qDebug()<<"a="<<a<<"b="<<b; 	
//*****************************************************************************
///============================================================================
/// 5.calculate value of roi
///============================================================================
//*****************************************************************************
	float dist;
	dist=0;
    for(size_t i=0; i < 2; i++) 
	{		
		for(size_t j=0; j < blobs[sorting[i]].size(); j++) 
	    {
			if(abs(a*blobs[sorting[i]][j].x-blobs[sorting[i]][j].y+b)/sqrt((a)*(a)+1) > dist)
				 dist = abs(a*blobs[sorting[i]][j].x-blobs[sorting[i]][j].y+b)/sqrt((a)*(a)+1);
		}
	}

	roi_value=dist;	
	qDebug()<<"roi_value"<<roi_value;

	
//*****************************************************************************
///============================================================================
/// 6.Local threshold
///============================================================================
//*****************************************************************************   
	for(int j2 = h-1 ; j2 >= 0; j2--)  
    {
	     for (int i2 = w-1; i2 >= 0; i2--) 
	     {
			if( abs(a*i2-j2+b)/sqrt((a)*(a)+1)< roi_value)
		    {
				graylevel[imgray.at<uchar>(j2,i2)]++;
				roi_area++;
				source.at<uchar>(j2,i2)=(uchar)255;
			}
			else
				source.at<uchar>(j2,i2)=0;//noise reduction*/
			// }
			/* else if (a>=0.2)
			 {
				 if(j2<avg_y+roi_value && j2>avg_y-roi_value )
			     {
				     graylevel[imgray.at<uchar>(j2,i2)]++;
				     roi_area++;
				     source.at<uchar>(j2,i2)=(uchar)255;
			     }
				 else
				     source.at<uchar>(j2,i2)=0;//noise reduction
			 }*/           		 		 
		 }
	}
	float cdf2[256]={0};
	float threshold_std2=0;
	///statistical analysis on local region
	//probability calculation
	for(int i=0;i<256;i++)
    {
        pro_gray[i]=(float)graylevel[i]/roi_area;//probability of graylevel[i]
		if(i==0)
			cdf2[i]=pro_gray[i];	    
		else 
			cdf2[i]=cdf2[i-1]+pro_gray[i];
	}
	
	/// mean
	for(int i=0;i<256;i++)
    {
		mean = mean + i*pro_gray[i];//mean		
	}	
	///variance
	for(int i=0;i<256;i++)
    {
		  var = var + pow((i-mean),2)*pro_gray[i];
	}
	sigma = pow((double)var,0.5);//standard deviation
	qDebug()<<"mean"<<mean<<"std="<<sigma;
	for(int i =0;i<4;i++)
	{
		if(cdf2[(int)(mean+i*sigma)]>0.9)
		{
			threshold_std2 = mean+i*sigma;
			qDebug()<<"threshold_std"<<threshold_std2<<"i="<<i <<"cdf = "<<cdf2[(int)(mean+i*sigma)];
			break;
		}		

	}
	//qDebug()<<"mean="<<mean<<"sigma"<<sigma<<"mean+2*sigma="<< mean+2*sigma;
	///qDebug()<<mean+sigma;
	for(int j2 = 0; j2 < h; j2++)  
    {
	     for (int i2 = 0; i2 < w; i2++) 
	     {  
		     if( source.at<uchar>(j2,i2)==255)
			 {
				 if(imgray.at<uchar>(j2,i2)<=threshold_std2)//not defined yet
					 source.at<uchar>(j2,i2)=0;
				 
			 }		      
		 }
	}
//*****************************************************************************
///============================================================================
/// 7.Remove small object
///============================================================================
//*****************************************************************************  	
	float *p3;
	int *xcenter3,*ycenter3;
	unsigned int amount3=0;
	float exp_x3=0.0,exp_y3=0.0,var3=0.0,sigma3=0.0;
	float th_removal=0;

	blobs.clear();
	FindBlobs(source,blobs);
	p3 = new float[blobs.size()];
	xcenter3 = new int [blobs.size()];
	ycenter3 = new int [blobs.size()];
	for(size_t i=0; i < blobs.size(); i++) 
	{	  	
		amount3=amount3+blobs[i].size();	
    }
	for(size_t i=0; i < blobs.size(); i++) 
	{
		p3[i]=(float)blobs[i].size()/amount3;	
    }
	
	th_removal = (float)0.5/blobs.size();

	qDebug()<<"th_removal="<<th_removal;
	for(size_t i=0; i < blobs.size(); i++) 
	{
		if( p3[i]<th_removal )
        for(size_t j=0; j < blobs[i].size(); j++) 
		{
				 int x = blobs[i][j].x;//position
                 int y = blobs[i][j].y;			 
                 source.at<uchar>(y,x)= 0;//b;//assign color to image		
        }			
    }
	
	//morphologyEx(source,source,MORPH_OPEN,Mat());
	

}


void connected::blob_sort(std::vector < std::vector<cv::Point2i> > &blobs,int *index)
{
	float *size_sort;
	size_sort=new float [blobs.size()];
	
	for(int i=0;i<blobs.size();i++)
	{
		size_sort[i]=blobs[i].size();
	}
	//sort from big to small
	sort(size_sort,index,blobs.size());

	for(int i=0;i<5;i++)
	{
		//qDebug()<<blobs[index[i]].size()<<" "<<size_sort[i];
	}

	   
}
///bubble sort
void connected::sort(float *arr,int *index, int n)
{
	//index = new int[n];
    for (int i=0;i<n;i++)
    {
        for (int j=0; j<n; j++)
        {
            if(arr[j]<arr[j+1])
            {
                float temp =  arr[j];//change
                arr[j]=arr[j+1];
                arr[j+1]=temp;
				int temp2 = index[j];//change
                index[j]=index[j+1];
                index[j+1]=temp2;

            }
			
        }

     }

}
//distance between 2 points

double connected::distance(double x1,double y1,double x2, double y2)
{
	return abs( sqrt ( pow((x1-x2),2)+pow((y1-y2),2) ));

}
void connected::neighborPoint(double slope,double x0,double y0,double sol[4])
{
	double b2=0.0;
	double poly_a=0.0,poly_b=0.0,poly_c=0.0,error=10;
	//b=y-ax
	b2=y0-slope*x0;
	//solve polynomial equation
	// three coefficient
	poly_a=slope*slope+1;
	poly_b=2*slope*b2-2*slope*y0-2*x0;
	poly_c=pow(b2,2)-2*b2*y0+pow(y0,2)+pow(x0,2)-pow(error,2);
	// solution
	//+
	sol[0]=(-poly_b+sqrt(pow(poly_b,2)-4*poly_a*poly_c));
	sol[0]=sol[0]/(2*poly_a);
	sol[1]=slope*sol[0]+b2;
	//-
	sol[2]=(-poly_b-sqrt(pow(poly_b,2)-4*poly_a*poly_c));
	sol[2]=sol[2]/(2*poly_a);
	sol[3]=slope*sol[2]+b2;
	
}
