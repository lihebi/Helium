#ifndef __TESTER_H__
#define __TESTER_H__
#include "common.h"
#include "segment.h"

enum ValgrindKind {
  VK_Success = 0,
  VK_CorrectFailure,
  VK_WrongFailure
};

typedef struct _TestResult {
  int return_code;
  enum ValgrindKind vk;
} TestResult;


class Tester {
public:
  Tester (const std::string &dir, Segment* seg);
  virtual ~Tester ();
  bool Success() {return m_success;}
  void Test();
  void WriteCSV();
private:
  std::string generateInput();

  std::string m_dir; // /tmp/helium-test-tmp.XXXXX
  
  std::string m_executable;
  std::vector<Variable> m_inv;
  std::vector<Variable> m_outv;
  // TestResult m_test_result;
  Segment *m_seg;
  bool m_success = false;

  int m_segment_begin_line=0;
  int m_segment_end_line = 0;

  // int m_val_correct_failure=0;
  // int m_val_wrong_failure=0;
  // int m_val_success=0;
  // int m_return_success = 0;
  // int m_return_failure = 0;
  std::vector<TestResult> m_results; // record result in a vector for all tests(totally test-number items)

};

#endif
