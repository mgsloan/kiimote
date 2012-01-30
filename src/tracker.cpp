#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <ntk/utils/opencv_utils.h>
#include <ntk/numeric/utils.h>
#include <ntk/geometry/pose_3d.h>
#include <ntk/gui/image_widget.h>

#include <math.h>

#include <iostream>

#include "tracker.h"

using namespace ntk;
using namespace cv;
using namespace std;

//utils
cv::Vec3b fromF(cv::Vec3f pnt) {
  return cv::Vec3b(ntk::math::rnd(pnt[0]),
                   ntk::math::rnd(pnt[1]),
                   ntk::math::rnd(pnt[2]));
}
cv::Vec3f fromB(cv::Vec3b vec) {
  return cv::Vec3f(vec[0], vec[1], vec[2]);
}

void ColorProfile::addColor(cv::Vec3b color) {
  colors.push_back(color);
}

void ColorProfile::calculateStatistics() {
  cv::Vec3f sum(0, 0, 0);
  for (int i = 0; i < colors.size(); i++)
    sum += colors[i];
  sum *= (1.0 / colors.size());
  mean = sum;
  cv::Vec3f dev(0, 0, 0);
  for (int i = 0; i < colors.size(); i++) {
    cv::Vec3f delt = (cv::Vec3f)colors[i] - mean;
    dev += delt.mul(delt);
  }
  dev *= (1.0 / (colors.size() - 1));
  deviation = dev;
}

void clipNear(RGBDImage& img) {
  Mat1f& depth = img.depthRef();
  Mat1b& mask = img.depthMaskRef();

  double minVal, maxVal;
  Point minLoc, maxLoc;
  minMaxLoc(depth, &minVal, &maxVal, &minLoc, &maxLoc, mask);

  Mat lt;
  compare(depth, minVal + 1.0, lt, CMP_LT);
  bitwise_and(mask, lt, mask);
}

void findLargest(Mat result, Point2f &top, double &max_radius) {
  vector<vector<Point> > contours;
  findContours(result, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

  for(int idx = 0; idx < contours.size(); idx++) {
    Point2f center; float rad;
    Mat contourMat(contours[idx]);
    minEnclosingCircle(contourMat, center, rad);
    double area = contourArea(contourMat);
    if (rad > max_radius && 0.3 * rad * rad < area) {
      max_radius = rad;
      top = center;
      /*
      top = Point2f(1000,1000);
      for (int i = 0; i < contours[idx].size(); i++) {
        if (contours[idx][i].y < top.y) {
          top.x = contours[idx][i].x;
          top.y = contours[idx][i].y;
        }
      }
      */
    }
  }
}

float nearMin(cv::Mat& m, int r, int c, int d) {
  int width = max(r + d, m.size().width);
  int height = max(c + d, m.size().height);
  float best = 100;
  for (int y = min(r - d, 0); y < height; y++) { 
    for (int x = min(c - d, 0); x < width; x++) { 
      float val = m.at<float>(y, x);
      if (val < best && val > 0.1)
        best = val;
    }
  }
  return best;
}

#define STEP_SIZE 1

void filterColor(const RGBDImage& img, vector<Mat>& hsvs,
         Mat1b& result, cv::Point3f upleft, cv::Point2f fudge,
         int hue_from, int hue_to, 
         int sat_from, int sat_to,
         int val_from, int val_to) {
  
  const Mat1f& depth = img.depth();
  const Mat1b& mask = img.depthMask();
  const Mat& rgb = img.rgb();
  const RGBDCalibration* calib = img.calibration();

  if (!calib || upleft.y > rgb.rows || upleft.x > rgb.cols) return;

  int wid = result.cols * STEP_SIZE, ht = result.rows * STEP_SIZE;

  cv::Range rslice(upleft.y - fudge.y, upleft.y + ht + fudge.y),
            cslice(upleft.x - fudge.x, upleft.x + wid + fudge.x);
  rslice.start = max(0, rslice.start);
  rslice.end = min(rgb.rows, rslice.end);
  cslice.start = max(0, cslice.start);
  cslice.end = min(rgb.cols, cslice.end);

  //printf("%i %i %i %i\n", rslice.start, rslice.end, cslice.start, cslice.end);
  
  if (hsvs.size() == 0) {
    Mat sliced_rgb(rgb, rslice, cslice);
    Mat hsv;
    cvtColor(sliced_rgb, hsv, CV_BGR2HSV);
    split(hsv, hsvs);
  }

  const Pose3D* rgb_pose = calib->rgb_pose;
  const Pose3D* depth_pose = calib->depth_pose;

  for (int r = 0; r < result.rows; r++)
  for (int c = 0; c < result.cols; c++) {
    int x = c * STEP_SIZE + upleft.x;
    int y = r * STEP_SIZE + upleft.y;
    if (mask(y, x) == 0) continue;
    // Calculate rgb location for this particular depth element.
    double dv = depth(y, x);
    Point3f pu = depth_pose->unprojectFromImage(Point2f(x, y), dv);
    Point3f prgb = rgb_pose->projectToImage(pu);
    int i_x = ntk::math::rnd(prgb.x) - cslice.start;
    int i_y = ntk::math::rnd(prgb.y) - rslice.start;
    if (is_yx_in_range(hsvs[0], i_y, i_x)) {
      uchar hue = hsvs[0].at<uchar>(i_y, i_x);
      uchar sat = hsvs[1].at<uchar>(i_y, i_x);
      uchar val = hsvs[2].at<uchar>(i_y, i_x);
      result.at<bool>(r, c) =
        (   ((hue_from > hue_to) && ((hue > hue_from) || (hue < hue_to)))
         || ((hue_from < hue_to) && ((hue > hue_from) && (hue < hue_to))))
        && sat > sat_from && sat < sat_to && val > val_from && val < val_to;
    }
  }
}

void mapPos(const Pose3D* depth_pose, const Mat& depth, const Point2f& top, Point3f& result) {
    int dx = ntk::math::rnd(top.x),
        dy = ntk::math::rnd(top.y);
    float d = depth.at<float>(dy, dx);
    cv::Point3f p = depth_pose->unprojectFromImage(top, d);
    result.x = p.x; result.y = p.y; result.z = p.z;
}

ImageWidget* img_widget = 0;
void displayBoolean(Mat1b& mat) {
  if (!img_widget) {
      img_widget = new ImageWidget(0);
      img_widget->show();
  }
  Mat1b dup = mat.clone();
  dup *= 255;

  img_widget->setImage(dup);
}

ImageWidget* img_widget2 = 0;
void displayRGB(Mat3b& mat) {
  if (!img_widget2) {
    img_widget2 = new ImageWidget(0);
    img_widget2->show();
  }
  img_widget2->setImage(mat);
  int x = 0, y = 0;
  img_widget2->getLastMousePos(x, y);
  cout << Point(x, y) << " --> "; 
  if (x != -1 || y != -1)
    printf("%u %u %u\n", mat.at<uchar>(x, y, 0),
      mat.at<uchar>(x, y, 1), mat.at<uchar>(x, y, 2));
}

ImageWidget* img_widget3 = 0;
void displayVal(Mat1b& mat) {
  if (!img_widget3) {
    img_widget3 = new ImageWidget(0);
    img_widget3->show();
  }
  img_widget3->setImage(mat);
  int x = 0, y = 0;
  img_widget3->getLastMousePos(x, y);
  cout << Point(x, y) << " --> "; 
  //if (x != -1 || y != -1)
    //printf("%u\n", mat.at<uchar>(y, x, 0));
}
void trackHands(ntk::RGBDImage& img, Point3f& rpos, Point3f& bpos) {
  Mat1f& depth = img.depthRef();
  Mat1b& mask = img.depthMaskRef();
  const RGBDCalibration* calib = img.calibration();

  if (calib) {
    const Pose3D* rgb_pose = calib->rgb_pose;
    const Pose3D* depth_pose = calib->depth_pose;

    Mat rgb = img.rgb().clone();
    //blur(rgb, rgb, Size(5, 5));

    int wid = 64, ht = 64;
    int min_rad = 4;
    cv::Point2f fudge(8, 8);
    
    Mat1b result(Size(wid, ht), 0);
    Point2f top; 
    double rrad = min_rad,
           brad = min_rad;
    vector<Mat> hsvs;
  
    Point3f upleft = depth_pose->projectToImage(rpos) - Point3f(wid / 2, ht / 2, 0);
    if (upleft.x == upleft.x) { //!isNaN(upleft)
      filterColor(img, hsvs, result, upleft, fudge, 160, 10, 160, 256, 80, 256);
      findLargest(result, top, rrad);
      top.x += upleft.x; top.y += upleft.y;
      top *= STEP_SIZE;
      mapPos(depth_pose, depth, top, rpos);
      //printf("partial r %f %f %f \n", top.x, top.y, rrad);
      hsvs.clear();
    }

    upleft = depth_pose->projectToImage(bpos) - Point3f(wid / 2, ht / 2, 0);
    if (upleft.x == upleft.x) { //!isNaN(upleft)
      result = Mat1b(Size(wid, ht), 0);
      filterColor(img, hsvs, result, upleft, fudge, 70, 100, 50, 256, 50, 256);
      findLargest(result, top, brad);
      //displayBoolean(result);
      top.x += upleft.x; top.y += upleft.y;
      top *= STEP_SIZE;
      mapPos(depth_pose, depth, top, bpos);
//      printf("partial b %f %f %f \n", top.x, top.y, brad);
      hsvs.clear();
    }
    
    upleft = Point3f(0, 0, 0);
    if (rrad <=  min_rad) {
      top = Point2f(0, 0);
      result = Mat1b(Size(depth.cols / STEP_SIZE, depth.rows / STEP_SIZE), 0);
      filterColor(img, hsvs, result, upleft, fudge, 160, 10, 160, 256, 80, 256);
      //Mat1b h(hsvs[0]);
      //displayVal(h);
      findLargest(result, top, rrad);
      top *= STEP_SIZE;
      mapPos(depth_pose, depth, top, rpos);
      //printf("full r %f %f %f \n", top.x, top.y, rrad);
    }
    if (brad <= min_rad) {
      top = Point2f(0, 0);
      result = Mat1b(Size(depth.cols / STEP_SIZE, depth.rows / STEP_SIZE), 0);
      filterColor(img, hsvs, result, upleft, fudge, 70, 100, 50, 256, 50, 256);
      findLargest(result, top, brad);
      top *= STEP_SIZE;
      mapPos(depth_pose, depth, top, bpos);
      //printf("full b %f %f %f \n", top.x, top.y, brad);
    }
  } else {
    bpos.x = bpos.y = bpos.z = 0;
    rpos.x = rpos.y = rpos.z = 0;
  }
}

void trackHead(ntk::RGBDImage& img, Point3f& hpos) {
  Mat1f& depth = img.depthRef();
  Mat1b& mask = img.depthMaskRef();
  const RGBDCalibration* calib = img.calibration();
  if (calib) {
    const Pose3D* rgb_pose = calib->rgb_pose;
    const Pose3D* depth_pose = calib->depth_pose;
    for (int y = 0; y < depth.rows; y+=10)
    for (int x = 0; x < depth.cols; x+=10) {
      if (mask.at<bool>(y, x)) {
        // probably a more opencv-ey way to do this, but eh.
        y = min(depth.rows, y + 20);
        int minx = x - 9, maxx = x + 40;
        for(int c = max(x - 9, 0); c < x + 40 && c < depth.cols; c++) {
          if(!mask.at<bool>(y, x)) {
            if (c < x) minx = c;
              if (c > x) { maxx = c; break; }
          }
        }
        x = (minx + maxx) / 2;
        float val = depth.at<float>(y, x);
        Point2f screenpos(x, y);
        cv::Point3f p = depth_pose->unprojectFromImage(screenpos, val);
        hpos.x = p.x; hpos.y = p.y; hpos.z = p.z - 0.05;
        return;
      }
    }
  }
}
