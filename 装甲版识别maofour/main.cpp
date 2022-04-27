// /home/ymj/Desktop/ood_red.mp4
// Created by ymj on 2022/4/5.
// /home/ymj/Desktop/ood_blue.mp4
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui_c.h>
#include<iostream>




using namespace std;
using namespace cv;

void callBack(int,void *);
int alpha = 9;
int beta = 0;
int color_1=105;
int color_2=60;
int color_3=55;
//Mat dst;


int main()
{
    int enemy_color= 1;//颜色  1：蓝   0：红
    VideoCapture video("/home/ymj/Desktop/ood_blue.mp4");
    if(!video.isOpened())
    {
        cout<<"失败"<<endl;
        return -1;
    }

    Mat image,png,img;

    for(;;)
    {
      video.read(image);
      image.copyTo(png);
        char input_win[] = "input_image";
        namedWindow(input_win, WINDOW_AUTOSIZE);
        imshow(input_win, image);
        Mat dst;
        int height = image.rows;
        int width = image.cols;
        //float beta = -100;

       // int alpha = 10;
       createTrackbar("对比度调节","output_image",&alpha,30,callBack);
        callBack(0,0);
        dst = Mat::zeros(image.size(), image.type());
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                if (image.channels() == 3) {
                    float b = image.at<Vec3b>(row, col)[0];
                    float g = image.at<Vec3b>(row, col)[1];
                    float r = image.at<Vec3b>(row, col)[2];

                    dst.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(b*alpha/10 + beta);
                    dst.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(g*alpha/10+ beta);
                    dst.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(r*alpha/10 + beta);

                }
                else if (image.channels() == 1) {
                    float v_gray = image.at<uchar>(row, col);
                    dst.at<uchar>(row, col) = saturate_cast<uchar>(v_gray + beta);
                }
            }
        }

        namedWindow(input_win, WINDOW_AUTOSIZE);
        imshow("output_image", dst);

        blur(dst, dst, Size(1,3));

        //cvtColor(image,image,COLOR_BGR2GRAY);
        vector<cv::Mat> channels;
        Mat color_channel;
        split(dst,channels);
        if(enemy_color)
        {
            color_channel=channels[0]-channels[2];
        }
        else {
            color_channel=channels.at(2)-channels.at(0);
        }
        threshold(color_channel,img,color_1,255,THRESH_BINARY);
        imshow("a",img);
        createTrackbar("colo1","a",&color_1,255);
        //threshold(image,image,30,255,THRESH_BINARY_INV);

        threshold(color_channel,img,color_2,255,THRESH_BINARY);
        imshow("b",img);
        createTrackbar("colo2","b",&color_2,255);
        Mat di= getStructuringElement(1,Size(3,3));
        dilate(img,img,di);
        //dilate(image,image,di);
        //threshold(img,img,40,255,THRESH_BINARY_INV);
        floodFill(img,Point(5,50),Scalar(255),0,FLOODFILL_FIXED_RANGE);
        //imshow("c",img);
        threshold(img,img,color_3,255,THRESH_BINARY_INV);
        imshow("c",img);
        createTrackbar("colo3","c",&color_3,255);
        vector<vector<Point>> contours;
        findContours(img, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
       //drawContours(png, contours, -1, Scalar(255,255,255), 4);
        vector <vector<Point>>  points;
        cout<<points.size()<<endl;

        if (contours.size() > 6)
           cout<<"debug: "<<__LINE__<<endl;
        for (size_t i = 0, j=0; i < contours.size(); i++)
        {

            double area = contourArea(contours[i]);
            cout<<"debug: "<<__LINE__<<endl;
           if (area < 40 || area >1e4 ) continue;//面积
         RotatedRect light= fitEllipse(contours[i]);
         double length= arcLength(contours[i],true);
         cout<<"debug: "<<__LINE__<<endl;
         if(length<15||length>4000)continue;//周长
           cout<<"debug: "<<__LINE__<<endl;
        // if(light.angle<80)continue;//角度
            cout<<"debug: "<<__LINE__<<endl;
          //size_t j;cout<<"debug: "<<__LINE__<<endl;
          points.push_back(contours[i]);cout<<"debug: "<<__LINE__<<endl;

            cout<<"debug: "<<__LINE__<<endl;
           cout<<points.size()<<endl;
           //drawContours(png, points, -1, Scalar(0), 4);
            //RotatedRect rrect = fitEllipse(points);
            //Point2f* vertices = new Point2f[4];
           //rrect.points(vertices);

       }
       //sort(points.begin(),points.end());
        cout<<"debug: "<<__LINE__<<endl;
      for(size_t i=0;i<points.size();i++)
        {
            for(size_t j=i+1;j<points.size();j++)
            {
                RotatedRect left  = fitEllipse(points[i]);
                RotatedRect right = fitEllipse(points[j]);
                cout<<"debug: "<<__LINE__<<endl;
                float angleDiff = abs(left.angle - right.angle);
                float LenDiff = abs(left.size.height- right.size.height) / max(left.size.height, right.size.height);
                cout<<"debug: "<<__LINE__<<endl;
                if(angleDiff>20||LenDiff>10)continue;//长度角度
                 float yDiff = abs(left.center.y - right.center.y);
                float xDiff = abs(left.center.x - right.center.x);
                float yDiff_ratio = yDiff /max(left.size.height, right.size.height);
                float lenght=(left.size.height+right.size.height)/4;
                float area_ratio=contourArea(points[i])/contourArea(points[i]);
                cout<<"debug: "<<__LINE__<<endl;
                if(xDiff/(2*lenght)>3||xDiff/(2*lenght)<1)continue;//长宽比
                if(yDiff_ratio>0.7)continue;//中心点
                if(area_ratio>1.3||area_ratio<0.7)continue;//面积比
                Point cen;
                cen.x=(right.center.x+left.center.x)/2;
                cen.y=(right.center.y+left.center.y)/2;
                Point lin[4];
                lin[0].x=left.center.x;
                lin[0].y=left.center.y-lenght;
                lin[1].x=left.center.x;
                lin[1].y=left.center.y+lenght;
                lin[2].x=right.center.x;
                lin[2].y=right.center.y+lenght;
                lin[3].x=right.center.x;
                lin[3].y=right.center.y-lenght;
                for(int i=0;i<4;i++){
                    if(i<3)
                        line(png,lin[i],lin[i+1],Scalar(255,255,255), 3);
                    else
                        line(png,lin[i],lin[0],Scalar (255,255,255), 3);
                }
                circle(png,cen, 13, Scalar (0,0,255), -1);

            }
        }




        imshow("q",img);
        imshow("new",png);

       if(waitKey(1) == 'q')
        break;

    }

}

void callBack(int,void *)
{
 cout<<alpha<<endl;
}
