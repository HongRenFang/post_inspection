#ifndef SC_H
#define SC_H

#include "ui_sc.h"
#include "EventLabel.h"
#include <vector>
#include <opencv2\opencv.hpp>  
#include <QtWidgets/QMainWindow>
#include <qlabel.h>
#include <QTimer> 
#include <QScrollArea>


using namespace cv;
//class EventLabel;

class sc : public QMainWindow
{
	Q_OBJECT

public:
	sc(QWidget *parent = 0);
	~sc();
	QLabel *q1;

public slots:
	void showthePositionofMouse(QPoint pos);
	void showthePositionofMouse2(QPoint pos);
	void showthePositionofMouse3(QPoint pos);
	
	//save file
	void save();
	void save_as(EventLabel *label);
	void save2();
	//menu enable 
	void enable();
	void enable2();
	//camera open/close
	void opencamera();
	void openbin();
	void openprocess();	
	void closeCamera(); 
	// three frame
	void readframe();//read camera
	void read_single_image();//read_single_image
	void binary();//binary image
	void process();//result
	//zoom
	void zoomin();
	void zoomout();
	void zoomin_label(EventLabel *label);
	void zoomout_label(EventLabel *label);
	
	//resetup when change mode(load image &¡@camera)
	void reset();
	void defect_width();
signals:
	void focus(bool a);
	//void factor(int f);
protected:
	void mousePressEvent(QMouseEvent *event);
private:
	EventLabel *label;
	EventLabel *label2;
	EventLabel *label3;
	Ui::scClass ui;
    QScrollArea *scrollArea;
	QScrollArea *scrollArea2;
	QScrollArea *scrollArea3;
		
	QTimer *timer;
	QTimer *timer2;
	QTimer *timer3;
	QImage *vimage,*sinimage;
	CvCapture *cam;

	VideoCapture capture;
	Mat video,video2;
	Mat imgray,imgray2,imgbinary,imgmed,imgerode,img, img2 ,image1;
	Mat drawline,rec;
	int f,f2,f3;
	int w,h;
	double mean,stddev,threshold_std ;
	QString imagepath;
	std::vector < std::vector<cv::Point2i > > blobs,blobs2;
	
	int imagearea;
	///defect_width
	int *xcenter,*ycenter,*crack_width,y_top_temp,y_down_temp; //mass center
	int xi,yi;

	//*****************binary********************
	int grayhist[256];
	float gray_p[256];float cdf[256];
	float total_time;
	float count;
	//********************************************
};

#endif // SC_H
