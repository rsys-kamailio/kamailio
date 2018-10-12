/*
 * Copyright (C) 2001-2003 FhG Fokus
 * Copyright (C) 2011 Carsten Bock, carsten@ng-voice.com
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*!
 * \file
 * \brief Route & Record-Route module
 * \ingroup taf 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <regex.h>

#include "../../sr_module.h"
#include "../../ut.h"
#include "../../error.h"
#include "../../pvar.h"
#include "../../mem/mem.h"
#include "../../mod_fix.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_to.h"
#include "../outbound/api.h"
#include "ims_tf_msgs.h"
#include "ims_tf_msgs.x"

#ifdef ENABLE_USER_CHECK
#include <string.h>
#include "../../str.h"
str i_user = {0,0};
#endif

int append_fromtag = 1;		/*!< append from tag by default */
int enable_double_taf = 1;	/*!< enable using of 2 RR by default */
int enable_full_lr = 0;		/*!< compatibilty mode disabled by default */
int add_username = 0;	 	/*!< do not add username by default */
int enable_socket_mismatch_warning = 1; /*!< enable socket mismatch warning */
static str custom_user_spec = {NULL, 0};
char* taf_ip = NULL;
int taf_port = 0;
static int fixup_taf_msg(void** param, int param_no);
pv_spec_t custom_user_avp;

ob_api_t taf_obb;

MODULE_VERSION

static int default_msgType = 500;
static str default_sipMsg = STR_STATIC_INIT("Internal Server Error");

extern void initTfAdapRcvTask(void);
extern S16 bldRspForTaf(uint32_t msgType, char* msgInfo);
static int  mod_init(void);static void mod_destroy(void);
/* fixup functions */
static int it_list_fixup(void** param, int param_no);
/* wrapper functions */
static int w_record_route_advertised_address(struct sip_msg *, char *, char *);
static int remove_record_route(sip_msg_t*, char*, char*);
static int w_bldRspForTaf(sip_msg_t*, char*, char*);

/*!
 * \brief Exported functions
 */
static cmd_export_t cmds[] = {
	{"record_route_advertised_address",  (cmd_function)w_record_route_advertised_address, 1, it_list_fixup, 0,
			REQUEST_ROUTE|BRANCH_ROUTE|FAILURE_ROUTE},
	{"remove_record_route",  remove_record_route, 0, 0, 0, REQUEST_ROUTE|FAILURE_ROUTE},
	{"send_to_taf",	(cmd_function)w_bldRspForTaf, 2, fixup_taf_msg, 0, ANY_ROUTE},
	{0, 0, 0, 0, 0, 0}
};


/*!
 * \brief Exported parameters
 */
static param_export_t params[] ={ 
	{"append_fromtag",	INT_PARAM, &append_fromtag},
	{"enable_double_taf",	INT_PARAM, &enable_double_taf},
	{"enable_full_lr",		INT_PARAM, &enable_full_lr},
#ifdef ENABLE_USER_CHECK
	{"ignore_user",		PARAM_STR, &i_user},
#endif
	{"add_username",		INT_PARAM, &add_username},
	{"enable_socket_mismatch_warning",INT_PARAM,&enable_socket_mismatch_warning},
	{"custom_user_avp",           PARAM_STR, &custom_user_spec},
	{"taf_ip",         	STR_PARAM, &taf_ip},
	{"taf_port",           	INT_PARAM, &taf_port},
	{0, 0, 0 }
};

/*!
 * \brief Exported Pseudo variables
 */
static pv_export_t mod_pvs[] = {
    {{0, 0}, 0, 0, 0, 0, 0, 0, 0}
};



struct module_exports exports = {
	"taf",
	DEFAULT_DLFLAGS,	/*!< dlopen flags */
	cmds,			/*!< Exported functions */
	params,			/*!< Exported parameters */
	0,				/*!< exported statistics */
	0,				/*!< exported MI functions */
	mod_pvs,			/*!< exported pseudo-variables */
	0,				/*!< extra processes */
	mod_init,			/*!< initialize module */
	0,				/*!< response function*/
	mod_destroy,		/*!< destroy function */
	0				/*!< per-child init function */
};


static int mod_init(void)
{
        /* Invoking TfAdap Task to handle events from Robot Framework */
	LM_INFO("****** Creating a new thread for talking to TAF ******\n");
        initTfAdapRcvTask();

	if (ob_load_api(&taf_obb) == 0)
		LM_DBG("Bound taf module to outbound module\n");
	else
	{
		LM_INFO("outbound module not available\n");
		memset(&taf_obb, 0, sizeof(ob_api_t));
	}

#ifdef ENABLE_USER_CHECK
	if(i_user.s && taf_obb.use_outbound)
	{
    LM_ERR("cannot use \"ignore_user\" with outbound\n");
    return -1;
	}
#endif

	if (add_username != 0 && taf_obb.use_outbound)
	{
		LM_ERR("cannot use \"add_username\" with outbound\n");
		return -1;
	}

	if (taf_port != 0 && taf_obb.use_outbound)
	{
		LM_ERR("cannot use \"taf_port\" with outbound\n");
		return -1;
	}

	if (custom_user_spec.s) {
		if (pv_parse_spec(&custom_user_spec, &custom_user_avp) == 0
				&& (custom_user_avp.type != PVT_AVP)) {
			LM_ERR("malformed or non AVP custom_user "
					"AVP definition in '%.*s'\n", custom_user_spec.len,custom_user_spec.s);
			return -1;
		}
	}
	if(taf_ip != NULL && taf_obb.use_outbound){
		LM_ERR("cannot use \"taf_ip\" with outbound\n");
		return -1;
	}

	return 0;
}

static void mod_destroy(void)
{
	LM_INFO("mod_destroy : Do nothing\n");
}

/**
 * @brief fixup for SL reply config file functions
 */
static int fixup_taf_msg(void** param, int param_no)
{
	if (param_no == 1) {
		return fixup_var_int_12(param, 1);
	} else if (param_no == 2) {
		return fixup_var_pve_str_12(param, 2);
	}
	return 0;
}

static int it_list_fixup(void** param, int param_no)
{
	pv_elem_t *model;
	str s;
	if(*param)
	{
		s.s = (char*)(*param); s.len = strlen(s.s);
		if(pv_parse_format(&s, &model)<0)
		{
			LM_ERR("wrong format[%s]\n",(char*)(*param));
			return E_UNSPEC;
		}
		*param = (void*)model;
	}
	return 0;
}

/**
 * wrapper for record_route(msg, params)
 */
static int w_record_route_advertised_address(struct sip_msg *msg, char *addr, char *bar)
{
	str s;

	if (msg->msg_flags & FL_RR_ADDED) {
		LM_ERR("Double attempt to record-route\n");
		return -1;
	}

	if (pv_printf_s(msg, (pv_elem_t*)addr, &s) < 0) {
		LM_ERR("failed to print the format\n");
		return -1;
	}
	msg->msg_flags |= FL_RR_ADDED;
	return 1;
}

static void free_taf_lump(struct lump **list)
{
	struct lump *prev_lump, *lump, *a, *foo, *next;
	int first_shmem;

	first_shmem=1;
	next=0;
	prev_lump=0;
	for(lump=*list;lump;lump=next) {
		next=lump->next;
		if (lump->type==HDR_RECORDROUTE_T) {
			/* may be called from railure_route */
			/* if (lump->flags & (LUMPFLAG_DUPED|LUMPFLAG_SHMEM)){
				LOG(L_CRIT, "BUG: free_taf_lmp: lump %p, flags %x\n",
						lump, lump->flags);
			*/	/* ty to continue */
			/*}*/
			a=lump->before;
			while(a) {
				foo=a; a=a->before;
				if (!(foo->flags&(LUMPFLAG_DUPED|LUMPFLAG_SHMEM)))
					free_lump(foo);
				if (!(foo->flags&LUMPFLAG_SHMEM))
					pkg_free(foo);
			}
			a=lump->after;
			while(a) {
				foo=a; a=a->after;
				if (!(foo->flags&(LUMPFLAG_DUPED|LUMPFLAG_SHMEM)))
					free_lump(foo);
				if (!(foo->flags&LUMPFLAG_SHMEM))
					pkg_free(foo);
			}
			
			if (first_shmem && (lump->flags&LUMPFLAG_SHMEM)) {
				/* This is the first element of the
				shmemzied lump list, we can not unlink it!
				It wound corrupt the list otherwise if we
				are in failure_route. -- No problem, only the
				anchor is left in the list */
				
				LM_DBG("lump %p is left in the list\n",
						lump);
				
				if (lump->len)
				    LM_CRIT("lump %p can not be removed, but len=%d\n",
						lump, lump->len);
						
				prev_lump=lump;
			} else {
				if (prev_lump) prev_lump->next = lump->next;
				else *list = lump->next;
				if (!(lump->flags&(LUMPFLAG_DUPED|LUMPFLAG_SHMEM)))
					free_lump(lump);
				if (!(lump->flags&LUMPFLAG_SHMEM)) {
					pkg_free(lump);
					lump = 0;
				}
			}
		} else {
			/* store previous position */
			prev_lump=lump;
		}
		if (first_shmem && lump && (lump->flags&LUMPFLAG_SHMEM))
			first_shmem=0;
	}
}

/*
 * Remove Record-Route header from message lumps
 */
static int remove_record_route(sip_msg_t* _m, char* _s1, char* _s2)
{
	free_taf_lump(&(_m->add_rm));
	return 1;
}

/*
 * Send Notificatio to TAF
 */
static int w_bldRspForTaf(sip_msg_t* _m, char* _s1, char* _s2)
{
	int msgType=0;
	str rxMsg;

        if (get_int_fparam(&msgType, _m, (fparam_t*)_s1) < 0) {
		msgType = default_msgType;
		LM_INFO("Could not convert msgType [%d]\n", msgType);
	}
       
        if (get_str_fparam(&rxMsg, _m, (fparam_t*)_s2) < 0) {
		rxMsg = default_sipMsg;
		LM_INFO("Could not convert sip msg\n");
	}

        LM_INFO("******************rxMsg type [%d] length [%d]************************\n", msgType, rxMsg.len);
        LM_INFO("****************** RECEIVED MSG FOR TAF [%s]************************\n", rxMsg.s);

	bldRspForTaf(msgType, rxMsg.s);
	return 1;
}
