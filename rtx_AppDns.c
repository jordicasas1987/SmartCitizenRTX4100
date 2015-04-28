/****************************************************************************
*  Program/file: AppDhcpd.c
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


/****************************************************************************
*                               Include files                                 
****************************************************************************/
//Type definitions
#include <Core/RtxCore.h> // Mandatory, Must be the first include

//Framework/Kernel
#include <ROS/RosCfg.h>

//Interfaces
#include <Api/Api.h>
#include <Cola/Cola.h>
#include <SwClock/SwClock.h>
#include <Protothreads/Protothreads.h>
#include <NetUtils/NetUtils.h>
#include <PtApps/AppSocket.h>
#include <PtApps/AppCommon.h>

//Configurations

//Private 
#include <RtxEai/RtxEai.h>
#include <../Projects/Amelie/COLApps/Apps/SmartCitizenRTX4100/rtx_AppDns.h>
#include <PtApps/AppWiFi.h>

/****************************************************************************
*                              Macro definitions                              
****************************************************************************/
#define DNS_SERVER_PORT 53
#define printf(...) RtxEaiPrintf(ColaIf->ColaTaskId,__VA_ARGS__)

/****************************************************************************
*                     Enumerations/Type definitions/Structs                   
****************************************************************************/
typedef struct 
{
  rsuint16  id;             /* identifier */
  rsuint16  flags;             /* flags*/
  rsuint16  qdcount;           /* question count */
  rsuint16  ancount;           /* answer record count */
  rsuint16  nscount;           /* authority record count */
  rsuint16  arcount;           /* additional record count */
  rsuint8   bodysection[200];  /* contain question or question & answer */
} DnsType;

/****************************************************************************
*                            Global variables/const                           
****************************************************************************/

/****************************************************************************
*                            Local variables/const                            
****************************************************************************/
//Header DNS
static const rschar dns_h_flags[] = {0x81,0x80};            //Flags activate: Response & Recursive
static const rschar dns_h_ancount[] = {0x00,0x01};          //Force to 1 answer
static const rschar dns_h_nscount[] = {0x00,0x00};          //Force to 0 authority record
static const rschar dns_h_arcount[] = {0x00,0x00};          //Force to 0 additional record


//Answer DNS
static const rschar dns_a_name[] = {0xc0,0x0c};             //Name
static const rschar dns_a_type[] = {0x00,0x01};             //Type = A
static const rschar dns_a_class[] = {0x00,0x01};            //Class = IN
static const rschar dns_a_ttl[] = {0x00,0x00,0x03,0x84};    //TTL (900s=15min)
static const rschar dns_a_sizeaddr[] = {0x00,0x04};         //SizeAddr = 4 (ipv4 -> 32b -> 4B)

/****************************************************************************
*                          Local Function prototypes                          
****************************************************************************/
/**
 * @brief Helper function to reverse data order
 * @param data : pointer to data
 **/
static void reverse_rsuint32(rsuint32* data) {
 rschar tmp_c[4];
 rsuint32 tmp = 0;

 memcpy(&tmp_c, data, 4);

 rsuint8 i;
 for (i=0; i<4; i++) {
  tmp = tmp << 8;
  tmp += tmp_c[i];
 }
*data = tmp;
}

/****************************************************************************
*                                Implementation                               
****************************************************************************/
static PT_THREAD(PtOnDNSreq(struct pt *Pt, const RosMailType* Mail))
{
    static DnsType *pRx;
    static DnsType *pTx;
    AppSocketDataType *pInst = (AppSocketDataType*)PtInstDataPtr;
    static ApiSocketAddrType client_addr;
  
    PT_BEGIN(Pt);

        while (AppWifiIsSoftAp()){
            
            //wait request
            PT_YIELD_UNTIL(Pt, IS_RECEIVED(API_SOCKET_RECEIVE_FROM_IND) 
                    && ((ApiSocketReceiveFromIndType*)Mail)->Handle == pInst->SocketHandle);
            
            pRx = (DnsType*)((ApiSocketReceiveFromIndType*)Mail)->BufferPtr;
            client_addr = ((ApiSocketReceiveFromIndType*)Mail)->Addr;
            
                
            //request a buffer for the response
            SendApiSocketGetTxBufferReq(COLA_TASK, pInst->SocketHandle, sizeof(DnsType));
            PT_YIELD_UNTIL(Pt, IS_RECEIVED(API_SOCKET_GET_TX_BUFFER_CFM) && 
                    ((ApiSocketGetTxBufferCfmType*)Mail)->Handle == pInst->SocketHandle);
            pTx = (DnsType*)((ApiSocketGetTxBufferCfmType*)Mail)->BufferPtr;
    
    
            //generate response
            //headers
            pTx->id = pRx->id;
            pTx->qdcount = pRx->qdcount;
            memcpy(&pTx->flags, dns_h_flags, 2);
            memcpy(&pTx->ancount, dns_h_ancount, 2);
            memcpy(&pTx->nscount, dns_h_nscount, 2);
            memcpy(&pTx->arcount, dns_h_arcount, 2);
            rsuint8 sizeTxbuffer = 12; //size headers
            
            //bodysection 
            rsuint8 body_size = 0;
            
            //question (bodysection)
            body_size = strlen((const char*)pRx->bodysection); //length domain string
            body_size += (1 + 4);	//1bytes for null bit + 2bytes fix
            memcpy(pTx->bodysection, pRx->bodysection, body_size);

            //Answer (bodysection)
            memcpy(pTx->bodysection+body_size, dns_a_name, 2); body_size +=2;
            memcpy(pTx->bodysection+body_size, dns_a_type, 2); body_size +=2;
            memcpy(pTx->bodysection+body_size, dns_a_class, 2); body_size +=2;
            memcpy(pTx->bodysection+body_size, dns_a_ttl, 4); body_size +=4;
            memcpy(pTx->bodysection+body_size, dns_a_sizeaddr, 2); body_size +=2;
    
            rsuint32 local_ip = (rsuint32)AppWifiIpv4GetAddress();
            reverse_rsuint32(&local_ip);
            memcpy(pTx->bodysection+body_size, &local_ip, 4); body_size +=4;    
            
            //Sending data... 
            SendApiSocketSendToReq(COLA_TASK, pInst->SocketHandle, (rsuint8*)pTx, 
                    sizeTxbuffer + body_size, 0, client_addr);
            PT_YIELD_UNTIL(Pt, IS_RECEIVED(API_SOCKET_SEND_CFM));
            
            //Free buffers
            SendApiSocketFreeBufferReq(COLA_TASK, pInst->SocketHandle, (rsuint8*)pRx);
            SendApiSocketFreeBufferReq(COLA_TASK, pInst->SocketHandle, (rsuint8*)pTx);
        }
    
    PT_END(Pt);
}

PtEntryType* rtx_AppDnsServer(RsListEntryType *PtList)
{
    ApiSocketAddrType addr;

    addr.Domain = ASD_AF_INET;
    addr.Ip.V4.Addr = 0;
    addr.Port = DNS_SERVER_PORT;
    
    printf ("DNS->Starting DNS server");
    
    return AppSocketStartUdpServer(PtList, addr, PtOnDNSreq);   
}
