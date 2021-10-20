#pragma once



#define  EV_MSG_FIRST                                                     100
#define  EV_MSG_PIPELN_STATE_OUT_OF_SYNC                                ( EV_MSG_FIRST + 0 )   // "Pipeline state out of sync, skipping.";
#define  EV_MSG_ERROR_COMPUTING_RESET_TARGET                            ( EV_MSG_FIRST + 1 )   // ("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
#define  EV_MSG_ERROR_OPENING_RESET_TARGET                              ( EV_MSG_FIRST + 2 )   // ("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
#define  EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST          ( EV_MSG_FIRST + 3 )   // "List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
#define  EV_MSG_PRELOAD_DEFINE_PRE_FINISHED                             ( EV_MSG_FIRST + 4 )              
#define  EV_MSG_PROCESSING_PARAMETERS_SETTINGS_CHANGED                  ( EV_MSG_FIRST + 5 ) 
#define  EV_MSG_RESET_BACKING_SERIES_FINISHED                           ( EV_MSG_FIRST + 6 ) 
#define  EV_MSG_ON_LABEL_DISTAL_VESSEL                                  ( EV_MSG_FIRST + 7 )
#define  EV_MSG_ERROR_NEW_TARGET_MAY_NOT_BE_INITIALIZED_SKIPPING        ( EV_MSG_FIRST + 8 )  //"Error: New target may not be initialized, skipping."

