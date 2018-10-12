/**
 * @file   ims_tf_msgs.h
 * @brief  It contains the MACRO definitions for TF
 */

/** @define NULL Pointer */ 
#define NULLP 0

/** @define PUBLIC */ 
#ifdef PUBLIC
#undef PUBLIC
#define PUBLIC                    /* public is c default scope */
#else /* not PUBLIC */
#define PUBLIC
#endif /* PUBLIC */

/** @define PRIVATE */ 
#ifdef PRIVATE
#undef PRIVATE
#define PRIVATE       static      /* private is c static scope */
#else /* not PRIVATE */
#define PRIVATE       static      /* private is c static scope */
#endif /* PRIVATE */

/** @define EXTERN */ 
#ifdef EXTERN
#undef EXTERN
#define EXTERN  extern
#else /* not EXTERN */
#define EXTERN  extern
#endif /* EXTERN */

/** @define TRUE */ 
#define TRUE 	1

/** @define FALSE */ 
#define FALSE 	0

/** @define Return OK */ 
#define ROK	0

/** @define Return Fail */ 
#define RFAILED	1


/** @define Msg Base for all the msgs exchanged between Robot Framework and LTE Nodes */
#define MSG_BASE          		500

/** @define IMS Bootup Notification */
#define TF_IMS_BOOTUP_NTFN    		(MSG_BASE + 1)

/** @define IMS Bootup Configuration Request */
#define TF_IMS_BOOTUP_CFG_REQ 		(MSG_BASE + 2)

/** @define IMS Bootup Configuration Response */
#define TF_IMS_BOOTUP_CFG_RSP 		(MSG_BASE + 3)

/** @define SIP Registration Notification */
#define TF_IMS_SIP_REG_NTFN 		(MSG_BASE + 4)

/** @define SIP 100 Trying Notification */
#define TF_IMS_SIP_100_TRY_NTFN		(MSG_BASE + 5)

/** @define SIP 401 UnAuthorized Notification */
#define TF_IMS_SIP_401_UNAUTH_NTFN	(MSG_BASE + 6)

/** @define SIP 200 OK Notification */
#define TF_IMS_SIP_200_OK_NTFN		(MSG_BASE + 7)

/** @define SIP Invite Notification */
#define TF_IMS_SIP_INV_NTFN		(MSG_BASE + 8)

/** @define SIP 180 Ringing Notification */
#define TF_IMS_SIP_180_RING_NTFN	(MSG_BASE + 9)

/** @define SIP Prack Notification */
#define TF_IMS_SIP_PRACK_NTFN		(MSG_BASE + 10)

/** @define SIP Ack Notification */
#define TF_IMS_SIP_ACK_NTFN		(MSG_BASE + 11)

/** @define SIP Bye Notification */
#define TF_IMS_SIP_BYE_NTFN		(MSG_BASE + 12)

/** @define SIP Cancel Notification */
#define TF_IMS_SIP_CANCEL_NTFN		(MSG_BASE + 13)

/** @define SIP 603 Decline Notification */
#define TF_IMS_SIP_603_DECLINE_NTFN	(MSG_BASE + 14)

/** @define SIP 600 Busy Everywhere Notification */
#define TF_IMS_SIP_600_BUSY_EVRY_NTFN	(MSG_BASE + 15)

/** @define SIP 181 Call Forwarded Notification */
#define TF_IMS_SIP_181_CALL_FRWRD_NTFN	(MSG_BASE + 16)

/** @define SIP 486 Busy Here Notification */
#define TF_IMS_SIP_486_BUSY_HERE_NTFN	(MSG_BASE + 17)

/** @define Maximum Msg Length */ 
#define  MAX_MSG_LEN             500000

/** @define Maximum Size per receipt */ 
#define  MAX_SIZE_PER_RCV        1200

/** @define Message Identifier Length */ 
#define  MSG_ID_LEN              4

/** @define Message Size Length */ 
#define  MSG_SIZE_LEN            4
