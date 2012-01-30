#ifndef SMOOTH_FILTER_H
#define SMOOTH_FILTER_H

#include <list>
#include <opencv2/core/core.hpp>
#include <vector>

using namespace std;

class PointMetric {
public:
  double operator()(cv::Point3f a, cv::Point3f b) {
    cv::Point3f delta = b - a;
    return delta.dot(delta);
  }
};

template<typename T>
class ScalarMetric {
public:
  double operator()(T a, T b) { return fabs(a - b); }
};

//TODO: make conditional on stability
//TODO: determine if the tracking has been lost.

template<typename T, typename F>
class SmoothFilter {
private:
  int sz;
  T value;
  F metric;
  bool uptodate;
  double max_dist, wacc;
  list<T> history;

public:
  SmoothFilter(int size, F f, double max, double acc) : 
    sz(size), value(), metric(f), uptodate(false), max_dist(max), wacc(acc)
   { }

  void push(const T& val) {
    history.push_back(val);
    while (history.size() > sz) history.erase(history.begin());
    uptodate = false;
  }

  list<T>& getHistory() { return history; }

  //TODO: this is written inefficiently for the sake of future development
  // flexibility. once a smoother is for sure chosen, one which is a function
  // of the previous mean and the new value shouldbe used.
  T computeValue() {
    if (uptodate || history.empty()) return value;
    vector<T> results;
    vector<double> weights;
//    vector<int> counts;
    double weight = 1;
    typename list<T>::iterator it;
    for (it = history.begin(); it != history.end(); ++it) {
      int closest = -1;
      double distance = max_dist;
      for (int j = 0; j < results.size(); j++) {
        cv::Point3f loc(results[j]);
        loc *= 1.0 / weights[j];
        double dist = metric(*it, loc);
        if (max_dist < 0 || dist < distance) {
            distance = dist;
            closest = j;
        }
      }
      if (closest == -1) {
        results.push_back((*it) * weight);
        weights.push_back(weight);
//        counts.push_back(1);
      } else {
        results[closest] += (*it) * weight;
        weights[closest] += weight;
//        counts[closest]++;
      }
      weight *= wacc;
    }
    int max_ix = 0;
    for (int i = 1; i < results.size(); i++)
      if (weights[i] > weights[max_ix]) max_ix = i;
    value = results[max_ix] * (1.0 / weights[max_ix]);
    uptodate = true;
    return value;
  }
};

#endif
