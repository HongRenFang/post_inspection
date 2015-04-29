//project header
#include "sc.h"
#include "EventLabel.h"
#include "connect.h"
#include "ui_sc.h"
//opencv header file
#include <opencv2\opencv.hpp>  
//#include <highgui.h>
//Qt header file
#include <QLabel>
#include <QMouseEvent>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QByteArray>
#include <qdebug.h>
#include <QString>
#include <QTableWidget>
#include <QHeaderView>
#include <time.h>
#include <omp.h>
using namespace cv;

sc::sc(QWidget *parent)
	: QMainWindow(parent)
 
{
	ui.setupUi(this);

	//Initialization
	cam = NULL;//default mode of camera
	timer = new QTimer(this);//new timer
	timer2 = new QTimer(this);
	timer3 = new QTimer(this);
	w=720;h=480;//width,height 
	imagearea = w*h;
	/// Infrared : 720*480 ,webcam:640*480
	drawline = cv::Mat::zeros(h,w,CV_8UC3);
	rec = cv::Mat::zeros(h,w,CV_8UC1 );
	//setting for ui table color
	ui.tableWidget->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: QColor(225,100,225) }");
	ui.tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: QColor(225,100,225) }");
	ui.tableWidget->setSelectionMode(QAbstractItemView::NoSelection);//table item can not be selected
	ui.tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);//can not edit
	//
	///default image frame setting
	label = new EventLabel(this);//new EventLabel object
    label->setGeometry(30,80,w,h);//label Geometry
	scrollArea = new QScrollArea(this);//scroll bar
	scrollArea->setGeometry(30,80,w,400);//scroll area Geometry
	scrollArea->setStyleSheet("border: 2px solid gray");//border color of scrollarea
	scrollArea->setPalette(Qt::gray);//color of scrollarea
	scrollArea->setWidget(label);//set label to scrollArea
	scrollArea->setWidgetResizable(true);//can Resize
	scrollArea->show();//show on the ui
	
	//
	///ui setting of image2
	label2 = new EventLabel(this);//new EventLabel object
	label2->setGeometry(30,510,w, h);//label Geometry
	scrollArea2 = new QScrollArea(this);//scroll bar
	scrollArea2->setGeometry(30,510,w,400);//scroll area Geometry
	scrollArea2->setStyleSheet("border: 2px solid gray");//border color of scrollarea
	scrollArea2->setPalette(Qt::gray);//color of scrollarea
	scrollArea2->setWidget(label2);//set label2 to scrollArea
	scrollArea2->setWidgetResizable(true);//can Resize
	scrollArea2->show();	//show on the ui
	//
	///ui setting of image3
	label3 = new EventLabel(this);//new EventLabel object
	label3->setGeometry(700,510,w, h);//label Geometry
	scrollArea3 = new QScrollArea(this);//scroll bar
	scrollArea3->setGeometry(700,510,w,400);//scroll area Geometry
	scrollArea3->setStyleSheet("border: 2px solid gray");//border color of scrollarea
	scrollArea3->setPalette(Qt::gray);//color of scrollarea
	scrollArea3->setWidget(label3);//set label3 to scrollArea
	scrollArea3->setWidgetResizable(true);//can Resize
	scrollArea3->show();//show on the ui
	//
	///load image
	connect(ui.actionOpen,SIGNAL(triggered()),this,SLOT(reset()));//zoom parameter resetting
	connect(ui.actionOpen,SIGNAL(triggered()),this,SLOT(closeCamera() ));//close camera
	connect(ui.actionOpen,SIGNAL(triggered()),this,SLOT(read_single_image()));
	connect(ui.actionBinary,SIGNAL(triggered()),this,SLOT(binary())); //binary
	connect(ui.actionDefects,SIGNAL(triggered()),this,SLOT(process())); //process
	//
	///Camera open
	connect(ui.actionOpen_2,SIGNAL(triggered()),this,SLOT(reset()));//zoom parameter resetting
	connect(ui.actionOpen_2,SIGNAL(triggered()),this,SLOT(opencamera()));//camera timer start
	connect(timer,SIGNAL(timeout()),this,SLOT(readframe()));//read frame from camera

	connect(ui.actionBinary,SIGNAL(triggered()),this,SLOT(openbin()));//start binary timer
	connect(timer2,SIGNAL(timeout()),this,SLOT(binary()));	//begin making image binarize
    connect(ui.actionDefects,SIGNAL(triggered()),this,SLOT(openprocess()));//start process timer
	connect(timer3,SIGNAL(timeout()),this,SLOT(process()));	//begin making defects be contoured
	
	//show pixel coordinate of image
	connect(label,SIGNAL(mouse(QPoint)),this,SLOT(showthePositionofMouse(QPoint)));//show x,y (1)
	connect(label2,SIGNAL(mouse(QPoint)),this,SLOT(showthePositionofMouse2(QPoint)));//show x,y (2)
	connect(label3,SIGNAL(mouse(QPoint)),this,SLOT(showthePositionofMouse3(QPoint)));//show x,y (3)
	//zoom in/out by actionbutton of menubar
	connect(ui.actionZoom_in,SIGNAL(triggered()),this,SLOT(zoomin()));//zoom in	
	connect(ui.actionZoom_out,SIGNAL(triggered()),this,SLOT(zoomout()));//zoom out

	ui.gridLayout->addWidget(scrollArea);
	ui.gridLayout_3->addWidget(scrollArea2);
	ui.gridLayout_4->addWidget(scrollArea3);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(defect_width()));
}
//destructor
sc::~sc()
{
}
//
///show pixel coordinate of image by moving mouse 
void sc::showthePositionofMouse(QPoint pos)
{	
	f=label->factor;//zoom factor
	//restrict region
	if(pos.x()/f<w && pos.y()/f<h)
	{
		///QString("%1, %2") for showing string ( pos.x()/f,pos.y()/f )
        ui.tableWidget->setItem(0, 0, new QTableWidgetItem(QString("%1, %2").arg(pos.x()/f).arg(pos.y()/f)));//show pixel coordinate
	    ui.tableWidget->item(0,0)->setTextAlignment(Qt::AlignCenter);//show text in the center
	}
}
void sc::showthePositionofMouse2(QPoint pos)
{
	f2=label2->factor;//zoom factor
	if(pos.x()/f2<w && pos.y()/f2<h)
	{
    	ui.tableWidget->setItem(0, 1, new QTableWidgetItem(QString("%1, %2").arg(pos.x()/f2).arg(pos.y()/f2)));//show pixel coordinate
		ui.tableWidget->item(0,1)->setTextAlignment(Qt::AlignCenter);//show text in the center
	}	
}
void sc::showthePositionofMouse3(QPoint pos)
{
	f3=label3->factor;//zoom factor
	if(pos.x()/f3<w && pos.y()/f3<h)
	{
	    ui.tableWidget->setItem(0, 2, new QTableWidgetItem(QString("%1, %2").arg(pos.x()/f3).arg(pos.y()/f3)));//show pixel coordinate
		ui.tableWidget->item(0,2)->setTextAlignment(Qt::AlignCenter);//show text in the center
	}	
}
//
///image zooming in and out
void sc::mousePressEvent(QMouseEvent *event) 
{
	if (label->hasFocus() ) 
	{
		//image border
		scrollArea->setStyleSheet("border: 4px solid blue");//choose frame
		scrollArea2->setStyleSheet("border: 2px solid gray");
		scrollArea3->setStyleSheet("border: 2px solid gray");
		//image selection
		ui.tableWidget->setItem(2, 0, new QTableWidgetItem(QString("v")));//choose frame
		ui.tableWidget->setItem(2, 1, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->setItem(2, 2, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->item(2,0)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,1)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,2)->setTextAlignment(Qt::AlignCenter);//show text align Center
    }	
	else if ( label2->hasFocus() ) 
	{
		//image border
		scrollArea->setStyleSheet("border: 2px solid gray");
		scrollArea2->setStyleSheet("border: 4px solid blue");//choose frame2		
		scrollArea3->setStyleSheet("border: 2px solid gray");  	
		//image selection
		ui.tableWidget->setItem(2, 0, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->setItem(2, 1, new QTableWidgetItem(QString("v")));
		ui.tableWidget->setItem(2, 2, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->item(2,0)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,1)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,2)->setTextAlignment(Qt::AlignCenter);//show text align Center
    }	
	else if ( label3->hasFocus() ) 
	{
		//image border
		scrollArea->setStyleSheet("border: 2px solid gray");
		scrollArea2->setStyleSheet("border: 2px solid gray");		
		scrollArea3->setStyleSheet("border: 4px solid blue"); //choose frame3 
		//image selection
		ui.tableWidget->setItem(2, 0, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->setItem(2, 1, new QTableWidgetItem(QString(" ")));
		ui.tableWidget->setItem(2, 2, new QTableWidgetItem(QString("v")));//choose frame3		
		ui.tableWidget->item(2,0)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,1)->setTextAlignment(Qt::AlignCenter);//show text align Center
		ui.tableWidget->item(2,2)->setTextAlignment(Qt::AlignCenter);//show text align Center
    }	
	else
    {
		//not choose any frame
		scrollArea->setStyleSheet("border: 2px solid gray");
		scrollArea2->setStyleSheet("border: 2px solid gray");
		scrollArea3->setStyleSheet("border: 2px solid gray");
    }
}
//
///load image 
void sc::read_single_image()
{
	label->imagepath = QFileDialog::getOpenFileName(this,tr("Open File"), "",tr("BMP(*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)" ));
	if (!label->imagepath.isEmpty()) 
    {  
	  label->byteArray=label->imagepath.toLocal8Bit();
	 
      label->c=label->byteArray.data();//path char for opencv to load image
	  //Qstring to char*
	  QString filename,filename2,filename3;
	  
	  for(int i=label->imagepath.length()-1;i>0;i--)
	  {
		  if(label->imagepath[i]!='/')
			 filename+=label->imagepath[i];
		  else
			  break;
	  }
      for(size_t i = 0; i < filename.length(); ++i) 
	  {
		 filename2[filename.length()-1-i] = filename[i];
      }


	  ui.label_4->setText(filename2);
	  label->src = imread(label->c);
	  
	  image1=label->src;  
	  label->imageq=new QImage((const uchar*)image1.data,image1.cols,image1.rows,QImage::Format_RGB888);
	  //label->imageq->load(label->imagepath); */
	  label->image = QPixmap::fromImage(*label->imageq);//QPixmap image;
	  label->image = label->image.scaled(QSize(w,h));//scaling		
	  label->setPixmap(label->image); //show
	}
	else
		QMessageBox::information(this, tr("Warning!!!"),
                                      tr("No image selected %1.").arg(imagepath));//show warning when not choosing any image

	///initialization
	label2->imageq=new QImage();//QImage 2
	label3->imageq=new QImage();//QImage 3
	label2->image = QPixmap::fromImage(*label2->imageq);//QPixmap image2;	
	label3->image = QPixmap::fromImage(*label3->imageq);//QPixmap image3;	
	label2->image = label2->image.scaled(QSize(w,h)*label2->factor);//scaling	
	label3->image = label3->image.scaled(QSize(w,h)*label3->factor);//scaling	
	label2->setPixmap(label2->image);
	label3->setPixmap(label3->image); //show Image on label

}
//
///binarization
void sc::binary()
{ 
	clock_t start, end;//calculate code processing time
	start = clock();
	//initialization
	count=0,total_time = 0;
    memset( grayhist, 0, 256*sizeof(int) );
	memset( gray_p, 0, 256*sizeof(float) );
	memset( cdf, 0, 256*sizeof(float) );

	//***************************************************************
    cvtColor(label->src, imgray, CV_BGR2GRAY);//src to gray scale image
	Scalar mean_scalar,stddev_scalar;//mean and standard deviation
    cv::meanStdDev ( imgray, mean_scalar, stddev_scalar );
    mean = mean_scalar.val[0];
    stddev = stddev_scalar.val[0];
	medianBlur(imgray,imgray,3);
	///histogram analysis

	for(int j = h-1 ;j >=0; j--)  
    {
	     for (int i = w-1; i >= 0; i-- ) 
	     {		
			grayhist[imgray.at<uchar>(j,i)]++;	
		 }
	}
	//#pragma omp parallel for
	for(int i = 0; i<256; i++)
	{
		gray_p[i]=(float)grayhist[i]/imagearea;
		if(i==0)
			cdf[i]=gray_p[i];	    
		else 
			cdf[i]=cdf[i-1]+gray_p[i];

	}
	// threshold need to higher than 90%
	
	for(int i =0;i<5;i++)
	{
		if(cdf[(int)(mean+i*stddev)]>0.95)
		{
			threshold_std = mean+i*stddev;

			qDebug()<<"threshold_std"<<threshold_std << i <<"cdf = "<<cdf[(int)(mean+i*stddev)];
			break;
		}		

	}
	qDebug()<<"mean"<<mean<<"std"<<stddev;
	//binarization
	
	//bilateralFilter(imgray,imgray2,3,6,1);
	threshold(imgray, imgbinary, threshold_std, 255, CV_THRESH_BINARY);
	//median filter
	medianBlur(imgbinary,imgmed,3);
	dilate(imgmed,imgmed,Mat());
	//dilate(imgmed,imgmed,Mat());
	erode(imgmed,imgmed,Mat());
	//class connected
	connected c1;
	c1.FindBlobs(imgmed,blobs);//find blobs by median_image to reduce noise
	qDebug()<<"size="<<blobs.size();
	
	if(blobs.size()>1)
	{
	    c1.analysisline(blobs,imgray,imgmed);
		//output :¡@imgmed	
	}
    dilate(imgmed,imgmed,Mat(),cv::Point(-1,-1),2);
	erode(imgmed,imgmed,Mat());
//*****************************************************************************
///============================================================================
/// 8.hole filling
///============================================================================
//***************************************************************************** 
	std::vector<std::vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
    cv::Mat contourOutput = imgmed.clone();
	Mat contourImage = Mat::zeros( imgmed.size(), imgmed.type() );;
    cv::findContours( contourOutput, contours,hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE );
	cv::drawContours(contourImage, contours, -1, 255,CV_FILLED,8,hierarchy,1);
	
	imgbinary = contourImage;
	///***************************
	//show image
	//Indexed8
	//imgbinary = imgray ;
	label2->imageq=new QImage((const uchar*)imgbinary.data,imgbinary.cols,imgbinary.rows,QImage::Format_Indexed8);//QImage
	label2->image = QPixmap::fromImage(*label2->imageq);//QPixmap image3;	
	label2->image = label2->image.scaled(QSize(w,h)*label2->factor);//scaling	
	label2->setPixmap(label2->image);
	//show the current size of image
    ui.tableWidget->setItem(1, 1, new QTableWidgetItem(QString("%1").arg(label2->factor)+"00%"));
	ui.tableWidget->item(1,1)->setTextAlignment(Qt::AlignCenter);//show in the center

	///******************************************
	end = clock();//end computing the execution time
    total_time = (float)(end-start)/CLOCKS_PER_SEC;//computing the execution time
    qDebug()<<"analysis time="<<total_time;//output
}
//
///searching defects
void sc::process()
{
    img = label->src.clone();// copy
	img2 = label->src.clone();
    ///draw countour around the defects
	int i2,j2;

	for(int j = 1; j < h-1; j++)  
    {
	      for (int i = 1; i < w-1; i++) 
	      {
				  i2=i-1;
		          j2=j-1;                 
				  //draw on the left ,right ,upper ,lower
		          if( imgbinary.at<uchar>(j,i)==(uchar)255 )
		          {
					  //left
					  if(imgbinary.at<uchar>(j,i-1)==(uchar)0)
                            img2.at<Vec3b>(j,i-1)=cv::Vec3b(0,0,255);
					  else if(img2.at<Vec3b>(j,i-1)==cv::Vec3b(0,0,255))
							img2.at<Vec3b>(j,i-1)=img.at<Vec3b>(j,i-1);
					  //right
		              if(imgbinary.at<uchar>(j,i+1)==(uchar)0)
			                img2.at<Vec3b>(j,i+1)=cv::Vec3b(0,0,255);
					  else if(img2.at<Vec3b>(j,i+1)==cv::Vec3b(0,0,255))
							img2.at<Vec3b>(j,i+1)=img.at<Vec3b>(j,i+1);
					  //up
			          if(imgbinary.at<uchar>(j-1,i)==(uchar)0)
			                img2.at<Vec3b>(j-1,i)=cv::Vec3b(0,0,255);
					  else if(img2.at<Vec3b>(j-1,i)==cv::Vec3b(0,0,255))
							img2.at<Vec3b>(j-1,i)=img.at<Vec3b>(j-1,i);
					  //low
			          if(imgbinary.at<uchar>(j+1,i)==(uchar)0)
			                img2.at<Vec3b>(j+1,i)=cv::Vec3b(0,0,255);
					  else if(img2.at<Vec3b>(j+1,i)==cv::Vec3b(0,0,255))
							img2.at<Vec3b>(j+1,i)=img.at<Vec3b>(j+1,i);

		                
		          }
				  if( i>2 && j>2 )
				  {

					if(img2.at<Vec3b>(j2,i2)==cv::Vec3b(0,0,255) )
			        {
						if(img2.at<Vec3b>(j2+1,i2-1)==cv::Vec3b(0,0,255) && imgbinary.at<uchar>(j2+1,i2)==255 )  //Upper left                    
		                {
							//if(img2.at<Vec3b>(j2,i2-2)!=cv::Vec3b(0,0,255))
							    img2.at<Vec3b>(j2,i2-1)=cv::Vec3b(0,0,255);
		                }
						if(img2.at<Vec3b>(j2+1,i2+1)==cv::Vec3b(0,0,255) && imgbinary.at<uchar>(j2,i2+1)==255 )  //lower left                    
		                {
							//if(img2.at<Vec3b>(j2+1,i2-1)!=cv::Vec3b(0,0,255))
							    img2.at<Vec3b>(j2+1,i2)=cv::Vec3b(0,0,255);
		                }
						if(img2.at<Vec3b>(j2-1,i2+1)==cv::Vec3b(0,0,255) && imgbinary.at<uchar>(j2-1,i2)==255 )  //lower right                    
		                {
							//if(img2.at<Vec3b>(j2,i2+2)!=cv::Vec3b(0,0,255))
							    img2.at<Vec3b>(j2,i2+1)=cv::Vec3b(0,0,255);
		                }
						if(img2.at<Vec3b>(j2-1,i2-1)==cv::Vec3b(0,0,255) && imgbinary.at<uchar>(j2,i2-1)==255 )  //Upper right                  
		                {
							//if(img2.at<Vec3b>(j2-1,i2+1)!=cv::Vec3b(0,0,255))
							    img2.at<Vec3b>(j2-1,i2)=cv::Vec3b(0,0,255);
		                }
				    }
			  }		          
		  }
	}
/*	for(int j = 1; j < h-1; j++)  
    {
	      for (int i = 1; i < w-1; i++) 
	      {
			  if(img2.at<Vec3b>(j,i)==cv::Vec3b(0,0,255) )
			  {

				  if(img2.at<Vec3b>(j-1,i)==cv::Vec3b(0,0,255) && img2.at<Vec3b>(j+1,i)==cv::Vec3b(0,0,255) )  //Upper ,lower                    
		          {
					  if(img2.at<Vec3b>(j,i+1)==cv::Vec3b(0,0,255))
					  {
						  img2.at<Vec3b>(j,i)=img.at<Vec3b>(j,i);//remove red
						  img2.at<Vec3b>(j-1,i+1)=cv::Vec3b(0,0,255);//add red on the top
						  if(img2.at<Vec3b>(j,i+2)==img.at<Vec3b>(j,i+2))
						      img2.at<Vec3b>(j,i+1)=img.at<Vec3b>(j,i+1);					  
					  }					
		          }
			  }
		  }
	}
	//horizontal
	for(int j = 1; j < h-1; j++)  
    {
	      for (int i = 1; i < w-1; i++) 
	      {
			  if(img2.at<Vec3b>(j,i)==cv::Vec3b(0,0,255) )
			  {

				  if(img2.at<Vec3b>(j-1,i)==cv::Vec3b(0,0,255) && img2.at<Vec3b>(j+1,i)==cv::Vec3b(0,0,255) )  //Upper ,lower                    
		          {
					  if(img2.at<Vec3b>(j,i+1)==cv::Vec3b(0,0,255))
					  {
						  img2.at<Vec3b>(j,i)=img.at<Vec3b>(j,i);//remove red
						  img2.at<Vec3b>(j,i+1)=img.at<Vec3b>(j,i+1);										  
					  }						
		          }
			  }
		  }
	}*/
	//incline
/*	for(int j = 1; j < h-1; j++)  
    {
	      for (int i = 1; i < w-1; i++) 
	      {
			  if(img2.at<Vec3b>(j,i)==cv::Vec3b(0,0,255) )
			  {
				  if(img2.at<Vec3b>(j+1,i+1)==cv::Vec3b(0,0,255) && img2.at<Vec3b>(j,i+1)==img.at<Vec3b>(j,i+1) && img2.at<Vec3b>(j+1,i)==img.at<Vec3b>(j+1,i) )  //Upper ,lower                    
		          {				
						  img2.at<Vec3b>(j,i)=img.at<Vec3b>(j,i);//remove red
						  img2.at<Vec3b>(j+1,i+1)=img.at<Vec3b>(j+1,i+1);										  										
		          }
			  }
		  }
	}*/


	

//=======================================================================================
  
	// if use RGB888 , need to transform from RGB to BGR 
    cv::cvtColor(img2,img2, CV_BGR2RGB); //chaner color model for Qt
	//img2=img;
	//img2=drawline;
	label3->imageq=new QImage((unsigned char*)img2.data,img2.cols, img2.rows,QImage::Format_RGB888);//QImage
	label3->image = QPixmap::fromImage(*label3->imageq);//QPixmap image3;	//QPixmap
	label3->image = label3->image.scaled(QSize(w,h)*label3->factor);//scaling	
	label3->setPixmap(label3->image); //show Image on label
	//show the current size of image
	ui.tableWidget->setItem(1, 2, new QTableWidgetItem(QString("%1").arg(label3->factor)+"00%"));
	ui.tableWidget->item(1,2)->setTextAlignment(Qt::AlignCenter);//show in the center
}

void sc::enable()
{
	ui.actionBinary->setEnabled(true);	//make binarization function enable
}
void sc::enable2()
{
	ui.actionDefects->setEnabled(true); //make search Contour function enable
}

//camera Timer bigin and setting
void sc::opencamera()
{  
    capture = cv::VideoCapture(0);
	capture.set(CV_CAP_PROP_FPS ,30);
    timer->start();  //no parameter to snappy cam 
 
}
//open camera
void sc::readframe()  
{	
	capture >>label->src; //to image
	
	video=label->src;//to opencv mat 
	cv::cvtColor(video,video2, CV_BGR2RGB);//color transformation  
	//video2=label->src;
	//show image
    QImage vqimage((const uchar*)video2.data, video2.cols,video2.rows, QImage::Format_RGB888);//QImage  
	label->image = QPixmap::fromImage(vqimage);//QPixmap image;
	label->image = label->image.scaled(QSize(w,h)*label->factor);//scaling	
	label->setPixmap(label->image); 
	//show the current size of image
	ui.tableWidget->setItem(1, 0, new QTableWidgetItem(QString("%1").arg(label->factor)+"00%"));
	ui.tableWidget->item(1,0)->setTextAlignment(Qt::AlignCenter);//show text in the center

	///initialization
	label2->imageq=new QImage();//QImage 2
	label3->imageq=new QImage();//QImage 3
	label2->image = QPixmap::fromImage(*label2->imageq);//QPixmap image2;	
	label3->image = QPixmap::fromImage(*label3->imageq);//QPixmap image3;	
	label2->image = label2->image.scaled(QSize(w,h)*label2->factor);//scaling	
	label3->image = label3->image.scaled(QSize(w,h)*label3->factor);//scaling	
	label2->setPixmap(label2->image);
	label3->setPixmap(label3->image); //show Image on label

}  
void sc::openbin()
{
	//timer2 only start for video mode
	if(timer->isActive())
        timer2->start();                
}
void sc::openprocess()
{
	if(timer->isActive())
       timer3->start();                
}
//close camera timer
void sc::closeCamera()  
{   
    timer->stop(); 
}
///=======================================================================================  
///                                    Zoom in/out
///=======================================================================================  
//zoom in setting of label
void sc::zoomin_label(EventLabel *label)
{
	 label->factor=label->factor*2;	
	 //zoom size limitation
     if(label->factor>8)
	 {
		label->factor=8;
	 }
	 label->image = label->image.scaled(QSize(w,h)*label->factor);	
	 label->setPixmap(label->image);
}
//zoom out setting of label
void sc::zoomout_label(EventLabel *label)
{
	 label->factor=label->factor/2;	
	  //zoom size limitation
     if(label->factor<=0)
	 {
		label->factor=1;
	 }
	 label->image = label->image.scaled(QSize(w,h)*label->factor);	
	 label->setPixmap(label->image);
}
//choose image to zoom in
void sc::zoomin()
{
	if (label->hasFocus() )
	{
		//choose frame 1
		zoomin_label(label);
		ui.tableWidget->setItem(1, 0, new QTableWidgetItem(QString("%1").arg(label->factor)+"00%"));//show zoom factor
		ui.tableWidget->item(1,0)->setTextAlignment(Qt::AlignCenter);
	}
	else if (label2->hasFocus() )
	{
		//choose frame 2
		zoomin_label(label2);
		ui.tableWidget->setItem(1, 1, new QTableWidgetItem(QString("%1").arg(label2->factor)+"00%"));//show zoom factor
	    ui.tableWidget->item(1,1)->setTextAlignment(Qt::AlignCenter);
	}
	else if (label3->hasFocus() )
	{
		//choose frame 3
		zoomin_label(label3);
		ui.tableWidget->setItem(1,2, new QTableWidgetItem(QString("%1").arg(label3->factor)+"00%"));//show zoom factor
	    ui.tableWidget->item(1,2)->setTextAlignment(Qt::AlignCenter);
	}
	
}
//choose image to zoom out
void sc::zoomout()
{
	if (label->hasFocus() )
	{
		zoomout_label(label);
		ui.tableWidget->setItem(1, 0, new QTableWidgetItem(QString("%1").arg(label->factor)+"00%"));//show zoom factor
		ui.tableWidget->item(1,0)->setTextAlignment(Qt::AlignCenter);
	}
	else if (label2->hasFocus() )
	{
		zoomout_label(label2);
		ui.tableWidget->setItem(1, 1, new QTableWidgetItem(QString("%1").arg(label2->factor)+"00%"));//show zoom factor
	    ui.tableWidget->item(1,1)->setTextAlignment(Qt::AlignCenter);
	}
	else if (label3->hasFocus() )
	{
		zoomout_label(label3);
		ui.tableWidget->setItem(1, 2, new QTableWidgetItem(QString("%1").arg(label3->factor)+"00%"));//show zoom factor
	    ui.tableWidget->item(1,2)->setTextAlignment(Qt::AlignCenter);
	}
	
}
//resetting the zoom factor when change between image and camera
void sc::reset()
{
	//set to original size
	label->factor=1;
    label2->factor=1;
	label3->factor=1;
}
///=======================================================================================  
///                                    Save File
///=======================================================================================  
void sc::save()
{
	if (label->hasFocus() )
	    imwrite("Sourceimage.bmp",label->src);//default image name
	else if (label2->hasFocus() )
	    imwrite("binary.bmp",imgbinary);//default image name
	else if (label3->hasFocus() )
	    imwrite("result.bmp",img);//default image name
	
}
//choose frame and then choosing the path saving file
void sc::save2()
{
	//save label1
	if (label->hasFocus() )
	    save_as(label);
	//save label2
	else if (label2->hasFocus() )
	    save_as(label2);
	//save label3
	else if (label3->hasFocus() )
	    save_as(label3);;
	
}
//function of save as   
void sc::save_as(EventLabel *qlabel)
{
	QByteArray byteArray2 = ( QVariant( qlabel->image ) ).toByteArray() ;
	QString save2file = QString(byteArray2.toBase64());
	save2file = QFileDialog::getSaveFileName(this, tr("Save File"),
                           "/home/jana/image.bmp",
                           tr("BMP(*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)" ));//defaul image type(can add another types)
	if(save2file.isEmpty())  //empty path conditioon  
            return;      
    else  
    {
       if( !( qlabel->image.save(save2file) ) ) //can not save file
        {  
            QMessageBox::information(this,  
            tr("Warning!"),  
            tr("Failed to save the image!"));  //fail save
            return;  
        }  
    }  

}
void sc::defect_width()
{
	///***************************
	/// calculate crack width
	///***************************
	connected c2;
	c2.FindBlobs(imgmed,blobs2);
    xcenter = new int [blobs2.size()];//dynamic array
	ycenter = new int [blobs2.size()];//dynamic array
	//Claculating mass center
	for(size_t i=0; i < blobs2.size(); i++) 
	{
		///mass center
		xi=0,yi=0;//blobs pixels coordinate
        for(size_t j=0; j < blobs2[i].size(); j++) 
		{
			xi=xi+blobs2[i][j].x;
			yi=yi+blobs2[i][j].y;          
        }
		//center of every blobs
		xcenter[i]=xi/blobs2[i].size();//x center
		ycenter[i]=yi/blobs2[i].size();//y center
		///qDebug()<<"("<<xcenter[i]<<","<<ycenter[i]<<")";
    }
	// Calculating width
	crack_width = new int [blobs2.size()];//dynamic array
	for(size_t i=0; i < blobs2.size(); i++) 
	{
		y_top_temp=ycenter[i];
		y_down_temp=ycenter[i];
		for(size_t j=0; j < blobs2[i].size(); j++) 
		{
			if(blobs2[i][j].x==xcenter[i])
			{
				if(blobs2[i][j].y > y_top_temp)
					y_top_temp=blobs2[i][j].y;
				if(blobs2[i][j].y < y_down_temp)
					y_down_temp=blobs2[i][j].y;
			}
			         
        }
		crack_width[i]=y_top_temp-y_down_temp+1;
		///qDebug()<<"("<<xcenter[i]<<","<<ycenter[i]<<"), width : "<<crack_width[i]<<" pixels";
    }
	///show result
	QString detailsText ;
	QMessageBox *a=new QMessageBox(this);
	//detailsText=QString("number\t center \t width\n");
	for(size_t i=0; i < blobs2.size(); i++) 
	{
			detailsText+=QString("%1 \t( %2,%3 )\t width=%4\t size=%5  \n").arg(i+1).arg(xcenter[i]).arg(ycenter[i]).arg(crack_width[i]).arg(blobs2[i].size());
	}
	detailsText+=QString("\nNumber of crack is %1").arg(blobs2.size());
    a->about(this,"crack width",QString(detailsText));
	// a->setDetailedText(detailsText);
	
}