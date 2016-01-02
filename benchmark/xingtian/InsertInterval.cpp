#include <vector>
using std::vector;

struct Interval {
  int start;
  int end;
  Interval() : start(0), end(0) {}
  Interval(int s, int e) : start(s), end(e) {}
};


class Solution {
public:
    vector<Interval> insert(vector<Interval> &intervals, Interval newInterval) {
        vector<Interval> res;
    if (intervals.size()==0) {
      res.push_back(newInterval);
      return res;
    }
    bool done = false;
    int start, end;
    start = newInterval.start;
    end = newInterval.end;
    for (vector<Interval>::iterator it = intervals.begin();
    it!=intervals.end();it++) {
      // --: it
      // **: new

      // --- ***
      if (it->end < newInterval.start) {
        res.push_back(*it);
      }
      //    **********...
      // -------------...
      else if (it->start <= newInterval.start && it->end >= newInterval.start) {
        start = it->start;
        if (it->end > newInterval.end) {
          end = it->end;
        } else {
          end = newInterval.end;
        }
      }
      // ***********...
      //     -------...
      else if (it->start >= newInterval.start && it->start <= newInterval.end) {
        if (it->end < newInterval.end) ;
        else if (it->end > newInterval.end) {
          end = it->end;
        }
      }
      // **** ----
      else if (it->start > newInterval.start) {
        if (!done) {
          res.push_back(*(new Interval(start, end)));
          done = true;
        }
        res.push_back(*it);
      }
    }
    if (!done) {
          res.push_back(*(new Interval(start, end)));
    }
    return res;
    }
};

int main() {
  return 0;
}
