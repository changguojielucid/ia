/*
*             dP                   oo       dP          oo
*             88                            88
*    .d8888b. 88 dP    dP .d8888b. dP .d888b88 dP   .dP dP dP   .dP .d8888b.
*    88ooood8 88 88    88 88'  `"" 88 88'  `88 88   d8' 88 88   d8' 88'  `88
*    88.  ... 88 88.  .88 88.  ... 88 88.  .88 88 .88'  88 88 .88'  88.  .88
*    `88888P' dP `88888P' `88888P' dP `88888P8 8888P'   dP 8888P'   `88888P'
*    oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
*

*
*    Created:
*
*    Modified:
*
*
*
*
*
*                              Copyright (c) Elucid Bioimaging
*/





#include "EVFirst.h"
#include "EVWorkClient.h"
#include <QApplication>

#include "EVProServer.h"

int cap_main(int argc, char *argv[]);


int main(int argc, char *argv[])
{

    if (1)
    {

        // int cap_main(int argc, char *argv[]);

        /*
        EVWorkItem test;
        test.LoadImageFromSeriesPath("C:/Users/Public/Documents/CAP Exam Data/Images/ELUCID_008/2127272053/176929809");
    
        QString strJsnWorklistDesktopAppFileName = "C:/Users/Public/Documents/CAP Exam Data/Working Storage/wilist_tesh/wilist_tesh.json";
        int iWorkItemIdx = 0;  int iSeriesIdx = 0;

        // test.LoadImageFromWorkItemListFile(strJsnWorklistDesktopAppFileName, iWorkItemIdx, iSeriesIdx);

        EVTargetDefine* pEvTargetDefine = test.GetTargetDefine();
        pEvTargetDefine->EstablishNewTarget("LeftCarotid");
    
        //pEvTargetDefine->createTargetPre("LeftCarotid");

        // ---------------------------end testing
        // void targetDefine::acceptScreenControlFromAnalyze(QStackedWidget *seriesSelectionArea, imageSeries *series, targetDef *def)
        // on_continueWithAnalysisButton_clicked(), this function also needed in targetdefine, lots of work
        test.RenderImage();
        */
    
        if (1)
        {
            QApplication app(argc, argv);

            EVWorkClient client;
            client.CreateWorkItem("C:/Users/Public/Documents/CAP Exam Data/Images/ELUCID_008/2127272053/176929809");
            client.CreateWorkItem("C:/Users/Public/Documents/CAP Exam Data/Images/ELUCID_008/2127272053/3838099848");
            client.CreateWorkItem("C:/Users/Public/Documents/CAP Exam Data/Images/ELUCID_008/2127272053/2880937572");

            return app.exec();
        }
        else
        {
          QApplication app(argc, argv);
          EVProServer server;
          server.Run();

          return app.exec();
        }


    }
    else
    {
        return cap_main(argc, argv);
    }

  return 0;
}
