/********************************************************************16**

            (c) Copyright 2012 by RadiSys Corporation. All rights reserved.

     This software is confidential and proprietary to RadiSys Corporation.
     No part of this software may be reproduced, stored, transmitted, 
     disclosed or used in any form or by any means other than as expressly
     provided by the written Software License Agreement between Radisys 
     and its licensee.

     Radisys warrants that for a period, as provided by the written
     Software License Agreement between Radisys and its licensee, this
     software will perform substantially to Radisys specifications as
     published at the time of shipment, exclusive of any updates or 
     upgrades, and the media used for delivery of this software will be 
     free from defects in materials and workmanship.  Radisys also warrants 
     that has the corporate authority to enter into and perform under the 
     Software License Agreement and it is the copyright owner of the software 
     as originally delivered to its licensee.

     RADISYS MAKES NO OTHER WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
     WITHOUT LIMITATION WARRANTIES OF MERCHANTABILITY OR FITNESS FOR
     A PARTICULAR PURPOSE WITH REGARD TO THIS SOFTWARE, SERVICE OR ANY RELATED
     MATERIALS.

     IN NO EVENT SHALL RADISYS BE LIABLE FOR ANY INDIRECT, SPECIAL,
     CONSEQUENTIAL DAMAGES, OR PUNITIVE DAMAGES IN CONNECTION WITH OR ARISING
     OUT OF THE USE OF, OR INABILITY TO USE, THIS SOFTWARE, WHETHER BASED
     ON BREACH OF CONTRACT, TORT (INCLUDING NEGLIGENCE), PRODUCT
     LIABILITY, OR OTHERWISE, AND WHETHER OR NOT IT HAS BEEN ADVISED
     OF THE POSSIBILITY OF SUCH DAMAGE.

                       Restricted Rights Legend:

     This software and all related materials licensed hereby are
     classified as "restricted computer software" as defined in clause
     52.227-19 of the Federal Acquisition Regulation ("FAR") and were
     developed entirely at private expense for nongovernmental purposes,
     are commercial in nature and have been regularly used for
     nongovernmental purposes, and, to the extent not published and
     copyrighted, are trade secrets and confidential and are provided
     with all rights reserved under the copyright laws of the United
     States.  The government's rights to the software and related
     materials are limited and restricted as provided in clause
     52.227-19 of the FAR.

                    IMPORTANT LIMITATION(S) ON USE

     The use of this software is limited to the use set
     forth in the written Software License Agreement between Radisys and
     its Licensee. Among other things, the use of this software
     may be limited to a particular type of Designated Equipment, as 
     defined in such Software License Agreement.
     Before any installation, use or transfer of this software, please
     consult the written Software License Agreement or contact Radisys at
     the location set forth below in order to confirm that you are
     engaging in a permissible use of the software.

                    RadiSys Corporation
                    Tel: +1 (858) 882 8800
                    Fax: +1 (858) 777 3388
                    Email: support@trillium.com
                    Web: http://www.radisys.com 
 
*********************************************************************17*/

/**
 * @file       ims_rcvthrd.c
 *
 * @brief      This is c source file.
               This file contains the socket creation and handling of 
               msgs from Robot Framework on the Tf interface
 *
 */

/* Header include files (.h) */
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

/* XML includes */
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "libxml/encoding.h"
#include "libxml/xmlwriter.h"
#include "../../dprint.h"
#include "ims_tf_msgs.h" /* RFW dependencies */
#include "ims_tf_msgs.x" /* RFW structure definitions */

EXTERN S16 handle_xml_msg(xmlNode * cur_node, uint32_t msgType);
EXTERN S16 bldRspForTaf(uint32_t msgType, char* msgInfo);

PUBLIC S32 connFd;

//PRIVATE void* tfAdapListner(void *param);

PUBLIC void initTfAdapRcvTask()
{
   LM_INFO("****** Entered initTfAdapRcvTask ******\n");
#if 0
   pthread_t taskId;
   pthread_attr_t attr;

   pthread_attr_init(&attr);

   pthread_create(&taskId, &attr, tfAdapListner, NULL);
   pthread_attr_destroy(&attr);
#endif
   bldRspForTaf(TF_IMS_BOOTUP_NTFN, (char*)NULL);
}

#if 0
PRIVATE void *tfAdapListner(void *param)
{
   U8 *rcvMsg = NULLP;
   S32  msgLen = 1;
   S32 sockfd = 0;
   U32 portno = 0;
   U32 numBytesRecd = 0;
   struct sockaddr_in serv_addr;
   U32 msgType = 0;
   U32 rcdMsgLen = 0;
   U32 tempU32 = 0;
   U32 *rx_ptr = NULL;
   Bool bMsgIdRecd = FALSE;
   Bool bMsgLenRecd = FALSE;

   //printf("SSAP Receiver task started\n");
   LM_INFO("****** SSAP Receiver task started ******\n");

   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      printf("ERROR opening socket\n");
      exit(1);
   }

   /* Initialize socket structure */
   memset((U8 *) &serv_addr, 0x00, sizeof(serv_addr));

   /* TODO - Remove Hard Coding */
   portno = 10043;

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons((short)portno);

   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("Bind");
      //printf("ERROR on binding IP in tfAdapListner\n");
      LM_INFO("****** ERROR on binding IP in tfAdapListner ******\n");
      exit(1);
   }

   if(listen(sockfd, 1) == -1)
   {
      //printf("Failed to set socket to listen mode\n");
      LM_INFO("****** Failed to set socket to listen mode ******\n");
      exit(1);
   }

   while(TRUE)
   {
      /* Read the message from the socket */
      //printf("Waiting on socket accept\n");
      LM_INFO("****** Waiting on socket accept ******\n");
      connFd = accept(sockfd, (struct sockaddr *)NULLP, NULLP);
      if(connFd < 0)
      {
         close(connFd);
      }
      //printf("Accepted new connection\n");
      LM_INFO("****** Accepted new connection ******\n");
      bldRspForTaf(TF_IMS_BOOTUP_NTFN, (char*)NULL);

      while(1) 
      {
         rcvMsg = (U8*)malloc(MAX_MSG_LEN);
         if (rcvMsg == NULLP)
         {
            printf("******Memory Allocation Failed******\n");

         }
         else
         {
            //printf("******MEMORY AVAILABLE******\n");
            LM_INFO("****** MEMORY AVAILABLE ******\n");
            memset((U8 *)rcvMsg, 0, MAX_MSG_LEN);
         }
         bMsgIdRecd = FALSE;
         bMsgLenRecd = FALSE;
         numBytesRecd = 0;
         msgLen = read(connFd, &(rcvMsg[numBytesRecd]), MAX_SIZE_PER_RCV);
         if(msgLen < 0)
         {
            /* If nothing received , break */
            //printf("Receive Error on SA socket, msgLen=%ld\n", (long int)msgLen);
            LM_INFO("****** Receive Error on SA socket, msgLen=%ld ******\n", (long int)msgLen);
            free(rcvMsg);
            rcvMsg = NULLP;
            close(connFd);
            break;
         }
         else if(msgLen == 0)
         {
            /* If nothing received , break */
            //printf("Receive Zero bytes on TF socket\n");
            LM_INFO("****** Received Zero bytes on TF socket ******\n");
            free(rcvMsg);
            rcvMsg = NULLP;
            close(connFd);
            break;
         }

         if ((msgLen >= (MSG_SIZE_LEN)) && bMsgLenRecd == FALSE)
         {
            rx_ptr = (U32*)rcvMsg;
            tempU32 = (U32)(*rx_ptr);
            rx_ptr++;
            rcdMsgLen = (((tempU32 & 0xFF000000) >> 24) |
                  ((tempU32 & 0x00FF0000) >>  8) |
                  ((tempU32 & 0x0000FF00) <<  8) |
                  ((tempU32 & 0x000000FF) << 24));
            //printf("rcdMsgLen=%lu\n", (long)rcdMsgLen);
            LM_INFO("****** rcdMsgLen=%lu ******\n",  (long)rcdMsgLen);
            bMsgLenRecd = TRUE;
         }

         if (bMsgIdRecd == FALSE && (msgLen >= (MSG_ID_LEN + MSG_SIZE_LEN)))
         {
            tempU32 = (U32)(*rx_ptr);
            rx_ptr++;
            msgType = (((tempU32 & 0xFF000000) >> 24) |
                  ((tempU32 & 0x00FF0000) >>  8) |
                  ((tempU32 & 0x0000FF00) <<  8) |
                  ((tempU32 & 0x000000FF) << 24));
            //printf("msgType =%lu\n", (long)msgType);
            LM_INFO("****** msgType =%lu ******\n",  (long)msgType);
            bMsgIdRecd = TRUE;
         }

         numBytesRecd += msgLen;

         printf("msgLen=[%ld] numBytesRecd=[%lu]\n", (long int)msgLen, (long)numBytesRecd);

         if (numBytesRecd >= (rcdMsgLen + MSG_ID_LEN + MSG_SIZE_LEN))
         {
            printf("Received the whole message\n");

            //printf("\n\n\n");
            //printf("*****************************************************************\n");
            //printf("      Received message from the RFW\n");
            //printf("*****************************************************************\n");
            LM_INFO("\n\n\n");
            LM_INFO("****************************************************************\n");
            LM_INFO("****** Received message from the RFW ******\n");
            LM_INFO("****************************************************************\n");

            S16        ret = RFAILED;
            xmlDoc*	   doc = NULL;
            xmlNode*	root_element = NULL;

            //printf("Received %lu bytes on socket\n", (long)numBytesRecd);
            LM_INFO("****** Received %lu bytes on socket ******\n", (long)numBytesRecd);

            LIBXML_TEST_VERSION
            doc = xmlParseDoc((const xmlChar*)rx_ptr);
            root_element = xmlDocGetRootElement(doc);
            ret = handle_xml_msg(root_element->children, msgType);
            //printf("********* Handling XML msg complete ***********\n");
            LM_INFO("****** Handling XML msg complete ******\n");
            xmlCleanupParser();
            if(ret != ROK)
            {
               printf("Could not parse TfImsMsg received from  RFW\n");
               return NULL;
            }
         }

         if(numBytesRecd >= MAX_MSG_LEN)
         {
            printf("MAX Msg Length exceeded\n");
            break;
         }

         free(rcvMsg);
         rcvMsg = NULLP;
      }
   }

   return (void *)rcvMsg;
}
#endif
