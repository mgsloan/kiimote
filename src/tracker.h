#ifndef TRACKER_H
#define TRACKER_H

#include <opencv2/core/core.hpp>
#include <ntk/camera/rgbd_image.h>
#include <vector>

class ColorProfile {
 public:
  vector<cv::Vec3b> colors;
  cv::Vec3f mean;
  cv::Vec3f deviation;
  void addColor(cv::Vec3b color);
  void calculateStatistics();
};

void clipNear(ntk::RGBDImage& img);

void findLargest(cv::Mat result, cv::Point2f &middle, double &max_radius);

void trackHands(ntk::RGBDImage& img, cv::Point3f& rpos, cv::Point3f& bpos);

void trackHead(ntk::RGBDImage& img, cv::Point3f& hpos);

#endif // TRACKER_H