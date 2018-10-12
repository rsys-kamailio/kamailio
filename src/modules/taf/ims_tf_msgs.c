/**
 * @file   ims_msgs.c
 * @brief  It contains the encoding and decoding of msgs received from Robot Framework
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdint.h>
#include<unistd.h>
#include<stdbool.h>

/* XML includes */
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "libxml/encoding.h"
#include "libxml/xmlwriter.h"
#include "../../dprint.h"
#include "ims_tf_msgs.h"
#include "ims_tf_msgs.x"

PUBLIC S16 handle_xml_msg(xmlNode * cur_node, uint32_t msgType);
PUBLIC S16 bldNPstImsMsg(void *tfMsg);
PUBLIC void hexDump (const char *desc, void *addr, int len);
PUBLIC int convert_hex_to_bytes(const char *hex_str, unsigned char *byte_array, int byte_array_max);
PUBLIC U32 bldImsBootUpNtfn(uint32_t msgType, char *status, char* rspMsg);
PUBLIC U32 bldImsSipRegNtfn(char *sipReg, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip100TryingNtfn(char *sipTry, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip401UnAuthNtfn(char *sipUnAuth, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip200OkNtfn(char *sip200Ok, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSipInvNtfn(char *sipInv, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip180RingNtfn(char *sip180Ring, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSipPrackNtfn(char *sipPrack, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSipAckNtfn(char *sipAck, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSipByeNtfn(char *sipBye, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSipCancelNtfn(char *sipCancel, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip603DeclineNtfn(char *sip603Decline, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip600BusyEvryWhr(char *sip600Busy, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip181CallFrwrd(char *sip181CallFrwrd, uint32_t msgType, char* rspMsg);
PUBLIC U32 bldImsSip486BusyHere(char *sip486BusyHere, uint32_t msgType, char* rspMsg);

xmlDoc *doc = NULL;
extern S32 connFd;
extern int taf_port;
extern char* taf_ip;

S16 handle_xml_msg(xmlNode *cur_node, uint32_t msgType)
{
   S16 ret = 1;

   /* Not required as of now */
   switch(msgType)
   {
      case 0:
      {
         break;
      }
      default:
      {
         LM_INFO("****** ERROR : Unknown msg received from TAF [%d]i *****\n", msgType);
         return ret;
      }
   }
   return ret;
}

PUBLIC S16 sndRspToTaf(char* msgInfo, uint32_t msgLen)
{
  int clientSocket;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  LM_INFO("****** Sending Response on TAF IP = [%s] TAF Port = [%u]******\n", taf_ip, (unsigned int)taf_port);
  /*Create UDP socket*/
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(taf_port);

  serverAddr.sin_addr.s_addr = inet_addr(taf_ip);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverAddr;

  /*Send message to server*/
  sendto(clientSocket, msgInfo, msgLen, 0, (struct sockaddr *)&serverAddr, addr_size);

  return 0;
}

PUBLIC S16 bldRspForTaf(uint32_t msgType, char* msgInfo)
{
   U32	    msgLen = 0;
   char     rspMsg[1024];

   switch(msgType)
   {
      case TF_IMS_BOOTUP_NTFN:
      {
         LM_INFO("****** Building IMS Bootup Notification for TFW ******\n");
         msgLen = bldImsBootUpNtfn(msgType, (char*)"UP", rspMsg);
	 LM_INFO("****** Sending IMS Bootup Notification from IMS ****** : [processId = [%u] ][msgLen = %d] [payload = %s]\n", (unsigned int)getpid(), (int)msgLen, (char*)&rspMsg[8]);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_REG_NTFN:
      {
         LM_INFO("****** Building IMS SIP Registation Notification for TFW ******\n");
         msgLen = bldImsSipRegNtfn((char*)msgInfo, msgType, rspMsg);
	 LM_INFO("****** Sending IMS SIP Registation Notification from IMS ****** : [processId = %u] [connFd = %d] [payload= %s]\n", (unsigned int)getpid(), (int)connFd, (char*)&rspMsg[8]);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_100_TRY_NTFN:
      {
         LM_INFO("****** Building IMS SIP 100 Trying Notification for TFW ******\n");
         msgLen = bldImsSip100TryingNtfn((char*)msgInfo, msgType, rspMsg);
	 hexDump ((const char*)"100 Trying", (void*)rspMsg, msgLen);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_401_UNAUTH_NTFN:
      {
         LM_INFO("****** Building IMS SIP 401 UnAuthorized Notification for TFW ******\n");
         msgLen = bldImsSip401UnAuthNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 401 UnAuthorized Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_200_OK_NTFN:
      {
         LM_INFO("****** Building IMS SIP 200 OK Notification for TFW ******\n");
         msgLen = bldImsSip200OkNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 200 OK Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_INV_NTFN:
      {
         LM_INFO("****** Building IMS SIP Invite Notification for TFW ******\n");
         msgLen = bldImsSipInvNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP Invite Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_180_RING_NTFN:
      {
         LM_INFO("****** Building IMS SIP 180 Ringing Notification for TFW ******\n");
         msgLen = bldImsSip180RingNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 180 Ring Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_PRACK_NTFN:
      {
         LM_INFO("****** Building IMS SIP PRACK Notification for TFW ******\n");
         msgLen = bldImsSipPrackNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP PRACK Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_ACK_NTFN:
      {
         LM_INFO("****** Building IMS SIP ACK Notification for TFW ******\n");
         msgLen = bldImsSipAckNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP ACK Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_BYE_NTFN:
      {
         LM_INFO("****** Building IMS SIP BYE Notification for TFW ******\n");
         msgLen = bldImsSipByeNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP BYE Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_CANCEL_NTFN:
      {
         LM_INFO("****** Building IMS SIP CANCEL Notification for TFW ******\n");
         msgLen = bldImsSipCancelNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP CANCEL Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_603_DECLINE_NTFN:
      {
         LM_INFO("****** Building IMS SIP 603 Decline Notification for TFW ******\n");
         msgLen = bldImsSip603DeclineNtfn((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 603 Decline Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_600_BUSY_EVRY_NTFN:
      {
         LM_INFO("****** Building IMS SIP 600 Busy Everywhere Notification for TFW ******\n");
         msgLen = bldImsSip600BusyEvryWhr((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 600 Busy Everywhere Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_181_CALL_FRWRD_NTFN:
      {
         LM_INFO("****** Building IMS SIP 181 Call Forwarding Notification for TFW ******\n");
         msgLen = bldImsSip181CallFrwrd((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 181 Call Forwarding Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      case TF_IMS_SIP_486_BUSY_HERE_NTFN:
      {
         LM_INFO("****** Building IMS SIP 486 Busy Here Notification for TFW ******\n");
         msgLen = bldImsSip486BusyHere((char*)msgInfo, msgType, rspMsg);
         LM_INFO("****** Sending IMS SIP 486 Busy Here Notification from IMS ****** : %s\n", (char*)rspMsg);
         sndRspToTaf(rspMsg, msgLen);
         break;
      }
      default:
      {
         LM_INFO("****** ERROR : Unknown msg received from ImsApp[%d] *****\n", msgType); 
         return RFAILED;
      }
   }
   return ROK;
}

PUBLIC U32 bldImsBootUpNtfn(uint32_t msgType, char *status, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "bootupStatus" as child of imsMsg */
   memset((char*)temp, 0, 100);
   sprintf((char*)temp, "%s", status);
   xmlTextWriterWriteElement(writer, BAD_CAST "bootupStatus", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipRegNtfn(char *sipReg, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   printf("Received String from IMS = %s\n", sipReg);
   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipReg[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip100TryingNtfn(char *sipTry, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipTry[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip401UnAuthNtfn(char *sipUnAuth, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipUnAuth[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Start an element named "authAlgo" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "authAlgo", BAD_CAST temp);

   /* Start an element named "authKey" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "authKey", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip200OkNtfn(char *sip200Ok, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip200Ok[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Start an element named "authAlgo" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "authAlgo", BAD_CAST temp);

   /* Start an element named "Path" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Path", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipInvNtfn(char *sipInv, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipInv[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "Route" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Route", BAD_CAST temp);

   /* Start an element named "rcrdRoute" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rcrdRoute", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip180RingNtfn(char *sip180Ring, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip180Ring[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Start an element named "rcrdRoute" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rcrdRoute", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipPrackNtfn(char *sipPrack, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipPrack[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "Rack" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Rack", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipAckNtfn(char *sipAck, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipAck[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "Route" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Route", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipByeNtfn(char *sipBye, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipBye[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "Route" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Route", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSipCancelNtfn(char *sipCancel, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Method" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sipCancel[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Method", BAD_CAST temp);

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "Reason" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Reason", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip603DeclineNtfn(char *sip603Decline, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip603Decline[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip600BusyEvryWhr(char *sip600Busy, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip600Busy[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip181CallFrwrd(char *sip181CallFrwrd, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip181CallFrwrd[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

PUBLIC U32 bldImsSip486BusyHere(char *sip486BusyHere, uint32_t msgType, char* rspMsg)
{
   xmlTextWriterPtr writer;
   xmlBufferPtr payload;
   char temp[100];
   U32 msgLen = 0;

   /* Create a new XML buffer, to which the XML document will be
    * written */
   payload = xmlBufferCreate();
   if (payload == NULL) {
      printf("ERROR : creating the xml buffer failed\n");
      return RFAILED;
   }

   /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
   writer = xmlNewTextWriterMemory(payload, 0);
   if (writer == NULL) {
      printf("ERROR: creating the xml writer failed\n");
      return RFAILED;
   }
   /* Start the document with the xml default for the version,
    * encoding ISO 8859-1 and the default for the standalone
    * declaration. */
   xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

   /* Start an element named "imsMsg". Since thist is the first
    * element, this will be the root element of the document. */
   xmlTextWriterStartElement(writer, BAD_CAST "imsMsg");

   /* Start an element named "Uri" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(&sip486BusyHere[1], ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "Uri", BAD_CAST temp);

   /* Start an element named "srcIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcIp", BAD_CAST temp);

   /* Start an element named "srcPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "srcPort", BAD_CAST temp);

   /* Start an element named "rxdIp" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdIp", BAD_CAST temp);

   /* Start an element named "rxdPort" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "rxdPort", BAD_CAST temp);

   /* Start an element named "callId" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "callId", BAD_CAST temp);

   /* Start an element named "cSeq" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "cSeq", BAD_CAST temp);

   /* Start an element named "statusCode" as child of imsMsg */
   memset((char*)temp, 0, 100);
   strcpy(temp, strtok(NULL, ","));
   xmlTextWriterWriteElement(writer, BAD_CAST "statusCode", BAD_CAST temp);

   /* Here we could close the element EXAMPLE using the
    * function xmlTextWriterEndElement, but since we do not want to
    * write any other elements, we simply call xmlTextWriterEndDocument,
    * which will do all the work. */
   xmlTextWriterEndDocument(writer);

   xmlFreeTextWriter(writer);

   xmlBufferDump(stderr, payload);

   /* Encode Msg Len */
   msgLen = (U32)(payload->use);
   rspMsg[0] = (msgLen>>24) & 0xFF;
   rspMsg[1] = (msgLen>>16) & 0xFF;
   rspMsg[2] = (msgLen>>8) & 0xFF;
   rspMsg[3] = msgLen & 0xFF;

   /* Encode Msg Type */
   rspMsg[4] = (msgType>>24) & 0xFF;
   rspMsg[5] = (msgType>>16) & 0xFF;
   rspMsg[6] = (msgType>>8) & 0xFF;
   rspMsg[7] = msgType & 0xFF;

   /* Encode Msg Contents */
   sprintf((char*)&rspMsg[8], "%s", payload->content);
   xmlBufferFree(payload);
   return (msgLen + MSG_ID_LEN + MSG_SIZE_LEN);
}

S16 bldNPstImsMsg(void *tfMsg)
{
   S16          ret = RFAILED;
   U32		msgType = 0;

   /* Not required as of now */
   switch(msgType)
   {
      case 0:
      {
         break;
      }
      default:
      {
         LM_INFO("****** ERROR : Unknown msg received from TAF [%d]i *****\n", msgType);
         return ret;
      }
   }

   if(ret != ROK)
   {
      LM_INFO("****** ERROR : Failed to post the msg from RFW to MME App\n");
   }

   return ret;
}

int convert_hex_to_bytes(const char *hex_str, unsigned char *byte_array, int byte_array_max)
{
    int hex_str_len = strlen(hex_str);
    int i = 0, j = 0;

    /* The output array size is half the hex_str length (rounded up) */
    int byte_array_size = (hex_str_len+1)/2;

    if (byte_array_size > byte_array_max)
    {
        /* Too big for the output array */
        return -1;
    }

    if (hex_str_len % 2 == 1)
    {
        /* hex_str is an odd length, so assume an implicit "0" prefix */
        if (sscanf(&(hex_str[0]), "%1hhx", &(byte_array[0])) != 1)
        {
            return -1;
        }

        i = j = 1;
    }

    for (; i < hex_str_len; i+=2, j++)
    {
        if (sscanf(&(hex_str[i]), "%2hhx", &(byte_array[j])) != 1)
        {
            return -1;
        }
    }

    return byte_array_size;
}

void hexDump (const char *desc, void *addr, int len)
{
   int i;
   unsigned char buff[17];
   unsigned char *pc = (unsigned char*)addr;

   if (desc != NULL)
      printf ("%s:\n", desc);

   if (len == 0 || len < 0)
   {
      printf("Invalif Length\n");
      return;
   }

   for (i = 0; i < len; i++)
   {
      /* Multiple of 16 means new line (with line offset) */
      if ((i % 16) == 0)
      {
         /* Just don't print ASCII for the zeroth line */
         if (i != 0)
         {
            printf ("  %s\n", buff);
         }

         /* Output the offset */
         printf ("  %04x ", i);
      }

      /* Now the hex code for the specific character */
      printf (" %02x", pc[i]);

      /* And store a printable ASCII character for later */
      if ((pc[i] < 0x20) || (pc[i] > 0x7e))
      {
         buff[i % 16] = '.';
      }
      else
      {
         buff[i % 16] = pc[i];
      }
      buff[(i % 16) + 1] = '\0';
   }

   /* Pad out last line if not exactly 16 characters */
   while ((i % 16) != 0)
   {
      printf ("   ");
      i++;
   }

   /* And print the final ASCII bit */
   printf ("  %s\n", buff);
}
