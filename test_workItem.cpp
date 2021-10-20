// Copyright (c) Elucid Bioimaging

#include <QApplication>
#include <QtTest/QTest.h>

#include <string.h>
#include <iostream>

#include "cap.h"

class tst_workItem : public QObject
{
  Q_OBJECT

private slots:
  void listLoading();
  void listCount();

  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
};

//void tst_workItem::firstTest() {
  //qDebug("would run the test here");
  //QFAIL("not yet implemented");
//}

//void tst_workItem::secondTest() {
  //bool success;
  //QString diagnosticOutput;

  //success = false; //< this is where the test goes
  //diagnosticOutput = "a string diagnositc from the program";

  //QVERIFY2(success, qPrintable(diagnosticOutput));
//}

void tst_workItem::listLoading() {
  bool success;
  QString diagnosticOutput;
  cap c;
  workItem w(&c);

  success = w.loadWorkItemListFromFile("testData/wilist_test.json", workItem::Json);
  diagnosticOutput = "couldn't load work item list";

  QVERIFY2(true, qPrintable(diagnosticOutput));
}

//void tst_workItem::thirdTest() {
  //int result, expected;

  //expected = 43; //< this is where the expected goes
  //result = 42; //< this is where the test goes

  //QCOMPARE(result, expected);
//}

void tst_workItem::listCount() {
  bool success;
  QString diagnosticOutput;
  cap c;
  workItem w(&c);
  int result, expected;

  success = w.loadWorkItemListFromFile("testData/wilist_test.json", workItem::Json);
  diagnosticOutput = "couldn't load work item list";

  QVERIFY2(success, qPrintable(diagnosticOutput));

  expected = 4; 
  result = w.workItemList().size(); 

  QCOMPARE(result, expected);
}

#include "tst_workItem.moc"

void tst_workItem::initTestCase() {
  return;
}

void tst_workItem::cleanupTestCase() {
  return;
}

void tst_workItem::init() {
  return;
}

void tst_workItem::cleanup() {
  return;
}

//===========================================================

typedef int(*AppRunFunc)();
int cap_main_impl(int, char **, AppRunFunc);

int run_workItem_test_functions()
{
  tst_workItem test;
  return QTest::qExec(&test, qApp->arguments());
}

int main(int argc, char *argv[])
{
  return cap_main_impl(argc, argv, run_workItem_test_functions);
}
