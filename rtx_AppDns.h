/****************************************************************************
*  Program/file: AppDns.h
*  
*  Copyright (C) by RTX A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX A/S, Denmark.
*
*  Programmer: LKA
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*   
*   
*  DESCRIPTION: Implementation of very simple DNS server to be used with SoftAp
*               mode. It return one ip (AP)
*   
*   
*   
*   
****************************************************************************/

/****************************************************************************
*                                  PVCS info                                 
*****************************************************************************

$Author:   Jordi Casas  $
$Date:   28 Apr 2015 07:19:38  $
$Revision:   1.0  $
$Modtime:   28 Apr 2015 07:19:38  $
$Archive:   ./Projects/Amelie/COLApps/Apps/SmartCitizenRTX4100/rtx_AppDns.h  $

*/

#ifndef RTXAPPDNS_H
#define RTXAPPDNS_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/
//Type definitions

//Framework/Kernel

//Interfaces

//Configurations

//Private 

/****************************************************************************
*                              Macro definitions                             
****************************************************************************/


/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/


/****************************************************************************
*                           Global variables/const                           
****************************************************************************/


/****************************************************************************
*                             Function prototypes                            
****************************************************************************/
PtEntryType* rtx_AppDnsServer(RsListEntryType *PtList);

#endif

