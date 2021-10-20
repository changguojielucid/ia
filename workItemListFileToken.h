// Copyright (c) Elucid Bioimaging

/**
 *  \ingroup workItem
 */

#ifndef WORKITEMLISTFILETOKENS_H
#define WORKITEMLISTFILETOKENS_H

// general:
#define listProvenance_token "listProvenance" // section delimiter
#define performer_token "perfomer"
#define performDateTime_token "performDateTime" // ISO format, e.g. 2015-11-14T14:30:57-05:00
#define lastModifier_token "lastModifier"
#define lastModifiedDateTime_token "lastModifiedDateTime" // ISO format, e.g. 2015-11-14T14:30:57-05:00
#define workItemList_token "workItemList" // section delimiter
#define workItemID_token "workItemID"
#define workItemPriority_token "workItemPriority"
#define appliesDate_token "appliesDate" // ISO format, e.g. 2015-11-14T14:30:57-05:00
#define wilist_version_token "version"
#define timepoint_token "timepoint"
#define acqrep_token "acqrep"
#define workItemFolder_token "workItemFolder"

// contining clinicalJurisdiction and most general fields for subjects
#include "ebiWorkItemMetaDataTags.h"

// augmenting the above for subjects (of which they may be human, animal, or phantom, using only those tokens appropriate for the subjectType):
#define dob_token "dob" // ISO format, e.g. 2015-11-14
#define alsoKnownAs_token "alsoKnownAs"
#define individualID_token "individualID" // typically an MRN in clinical practice, animal identifier in preclinical research, or phantom serial number
#define subjectID_token "subjectID" // the subject ID if inidivudal is enrolled in a clinical study or trial
#define accession_token "accessionNo"

//      only used if subjectType == human:
#define patientName_token "patientName"

//      only used if subjectType == phantom:
#define phantomManufacturer_token "phantomManufacturer"
#define phantomModel_token "phantomModel"
#define phantomSerialNumber_token "phantomSerialNumber"

// specifically for image series:
#define imageSeriesSet_token        "imageSeriesSet" // section delimiter
#define seriesLocalFolderName_token "seriesLocalFolderName"
#define seriesThumbLocalFileName_token "seriesThumbLocalFileName"
#define seriesUID_token "seriesUID"
#include "ebiImageMetaDataTags.h"

// specifically for targets:
#define targetDefinitions_token "targetDefinitions" // section delimiter
#define targetLocalFolderName_token "targetLocalFolderName"
#define targetID_token "targetID"
#define targetInitializerFileName_token "targetInitializer"
#define targetPathFileName_token "targetPath"
#define bitplanes_token "bitplanes" // local file names for bitplanes an array underneath 
#define registrationTransforms_token "registrationTransforms" // seriesType/local file names for transform as key-value pairs underneath
#define regions_token "regions" // regionType/local file names for nrrd as key-value pairs underneath
#define probabilityMaps_token "probabilityMaps" // probabilityType/local file names for nrrd as key-value pairs underneath
#define valueMaps_token "valueMaps" // valueType/local file names for nrrd as key-value pairs underneath
#define keyImages_token "keyImages"
#define readingsLocalFileName_token "readingsLocalFileName"
#define lesionReadingsLocalFileName_token "lesionReadingsLocalFileName"
#define histologySections_token "histologySections"
#define referenceDataSet_token "referenceDataSet" // used when this target is being processad as part of a reference data set
#define processingParameters_token "processingParameters"
#define stageVersion_token "stageVersion"
#define stageSettings_token "stageSettings"
#include "ebiTargetMetaDataTags.h"

// specifically for updates (in addition to performer and performDate):
#define updates_token "updates" // section delimiter
#define step_token "step" // valid prefixes: {} "generation", "specification", "seriesSurvey", "targetDefine", "patientAnalyze", "patientReport" }
#define updateID_token "ID"

// the following are not used in the work item list, but are used to index into the processing steps in the user interface
// ORDER MUST BE SAME AS SEQUENCE OF CONSTRUCTION IN cap::selectWorkItemForAnalysis
#define WORKITEM 0
#define SERIESSURVEY 1
#define TARGETDEFINE 2
#define PATIENTANALYZE 3
#define PATIENTREPORT 4

#endif // WORKITEMLISTFILETOKENS_H
