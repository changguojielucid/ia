// Copyright (c) Elucid Bioimaging

#include <QApplication>
#include <QtTest/QTest.h>

#include <string.h>
#include <iostream>

#include "cap.h"

class tst_seriesSurvey : public QObject
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

//void tst_seriesSurvey::firstTest() {
  //qDebug("would run the test here");
  //QFAIL("not yet implemented");
//}

//void tst_seriesSurvey::secondTest() {
  //bool success;
  //QString diagnosticOutput;

  //success = false; //< this is where the test goes
  //diagnosticOutput = "a string diagnositc from the program";

  //QVERIFY2(success, qPrintable(diagnosticOutput));
//}

void tst_seriesSurvey::listLoading() {
  bool success;
  QString diagnosticOutput;
  cap c;
  workItem w(&c);
  seriesSurvey s;

  success = w.loadWorkItemListFromFile("testData/wilist_test.json", workItem::Json);
  diagnosticOutput = "couldn't load testData/wilist_2015-12-01-1130.json";

  QVERIFY2(success, qPrintable(diagnosticOutput));
}

//void tst_seriesSurvey::thirdTest() {
  //int result, expected;

  //expected = 43; //< this is where the expected goes
  //result = 42; //< this is where the test goes

  //QCOMPARE(result, expected);
//}

void tst_seriesSurvey::listCount() {
  bool success;
  QString diagnosticOutput;
  cap c;
  workItem w(&c);
  int result, expected;

  success = w.loadWorkItemListFromFile("testData/wilist_test.json", workItem::Json);
  diagnosticOutput = "couldn't load testData/wilist_2015-12-01-1130.json";

  QVERIFY2(success, qPrintable(diagnosticOutput));

  expected = 4; 
  result = w.workItemList().size(); 

  QCOMPARE(result, expected);

  w.workItemSelectedForAnalysis(3);

  expected = 13; 
  result = w.workItemList().at(3).getSurvey()->imageSeriesSet()->size(); 

  QCOMPARE(result, expected);
}

#include "tst_seriesSurvey.moc"

void tst_seriesSurvey::initTestCase() {
  return;
}

void tst_seriesSurvey::cleanupTestCase() {
  return;
}

void tst_seriesSurvey::init() {
  return;
}

void tst_seriesSurvey::cleanup() {
  return;
}

//===========================================================

typedef int(*AppRunFunc)();
int cap_main_impl(int, char **, AppRunFunc);

int run_seriesSurvey_test_functions()
{
  tst_seriesSurvey test;
  return QTest::qExec(&test, qApp->arguments());
}

int main(int argc, char *argv[])
{
  return cap_main_impl(argc, argv, run_seriesSurvey_test_functions);
}
