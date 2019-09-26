#include "stdafx.h"

#ifdef TEST_CONFIGURATION
#include "CppUnitTest.h"

#include "../Utils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace test {
  TEST_CLASS(sanitizePathFile)
  {
  public:
    TEST_METHOD(transformsForwardSlash) {
      std::wstring str = L"D:/a/b/c/d";
      std::wstring exp = L"D:\\a\\b\\c\\d";
      std::wstring act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }

    TEST_METHOD(canonicalizes) {
      std::wstring str = L"D:\\a\\..\\c\\d";
      std::wstring exp = L"D:\\c\\d";
      std::wstring act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\d";
      exp = L"D:\\c\\d";
      act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\..\\d";
      exp = L"D:\\d";
      act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\..\\d\\.";
      exp = L"D:\\d";
      act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }

    TEST_METHOD(examples) {
      std::wstring str = L"D:\\a/..\\c\\d\\./e";
      std::wstring exp = L"D:\\c\\d\\e";
      std::wstring act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b/../..\\c\\../d\\.";
      exp = L"D:\\d";
      act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"a\\b/..\\c\\../d";
      exp = L"a\\d";
      act = Utils::fs::sanitizePathFile(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }
  };

  TEST_CLASS(sanitizePathDir)
  {
  public:
    TEST_METHOD(transformsForwardSlash) {
      std::wstring str = L"D:/a/b/c/d/";
      std::wstring exp = L"D:\\a\\b\\c\\d\\";
      std::wstring act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:/";
      exp = L"D:\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }

    TEST_METHOD(outputsTrailingSlash) {
      std::wstring str = L"D:\\a\\b\\c\\d";
      std::wstring exp = L"D:\\a\\b\\c\\d\\";
      std::wstring act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }

    TEST_METHOD(canonicalizes) {
      std::wstring str = L"D:\\a\\..\\c\\d\\";
      std::wstring exp = L"D:\\c\\d\\";
      std::wstring act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\d\\";
      exp = L"D:\\c\\d\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\..\\d\\";
      exp = L"D:\\d\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b\\..\\..\\c\\..\\d\\.\\";
      exp = L"D:\\d\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }

    TEST_METHOD(examples) {
      std::wstring str = L"D:\\a/..\\c\\d\\./e";
      std::wstring exp = L"D:\\c\\d\\e\\";
      std::wstring act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"D:\\a\\b/../..\\c\\../d\\.";
      exp = L"D:\\d\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());

      str = L"a\\b/..\\c\\../d\\.";
      exp = L"a\\d\\";
      act = Utils::fs::sanitizePathDir(str);
      Assert::AreEqual(exp.c_str(), act.c_str());
    }
  };
}

#endif