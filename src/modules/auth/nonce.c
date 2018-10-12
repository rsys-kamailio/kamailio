/*
 * Digest Authentication Module
 * Nonce related functions
 * 
 * Copyright (C) 2001-2003 FhG Fokus
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
 *
 */

#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../../compiler_opt.h"
#include "../../md5.h"
#include "../../dprint.h"
#include "../../ut.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_from.h"
#include "../../ip_addr.h"
#include "nonce.h"
#include "../../globals.h"
#include <assert.h>
#ifdef USE_NC
#include "nc.h"
#endif
#ifdef USE_OT_NONCE
#include "ot_nonce.h"
#endif


int auth_checks_reg = 0;
int auth_checks_ood = 0;
int auth_checks_ind = 0;

/* maximum time drift accepted for the nonce creation time
 * (e.g. nonce generated by another proxy in the same cluster with the
 * clock slightly in the future)
 */
unsigned int nonce_auth_max_drift = 3; /* in s */

/** Select extra check configuration based on request type.
 * This function determines which configuration variable for
 * extra authentication checks is to be used based on the
 * type of the request. It returns the value of auth_checks_reg
 * for REGISTER requests, the value auth_checks_ind for requests
 * that contain valid To tag and the value of auth_checks_ood
 * otherwise.
 */
int get_auth_checks(struct sip_msg* msg)
{
	str tag;

	if (msg == NULL) return 0;

	if (msg->REQ_METHOD == METHOD_REGISTER) {
		return auth_checks_reg;
	}
		
	if (!msg->to && parse_headers(msg, HDR_TO_F, 0) == -1) {
		DBG("auth: Error while parsing To header field\n");
		return auth_checks_ood;
	}
	if (msg->to) {
		tag = get_to(msg)->tag_value;
		if (tag.s && tag.len > 0) return auth_checks_ind;
	}
	return auth_checks_ood;
}



/* takes a pre-filled bin_nonce union (see BIN_NONCE_PREPARE), fills the
 * MD5s and returns the length of the binary nonce (cannot return error).
 * See calc_nonce below for more details.*/
inline static int calc_bin_nonce_md5(union bin_nonce* b_nonce, int cfg,
										str* secret1, str* secret2,
										struct sip_msg* msg)
{
	MD5_CTX ctx;
	
	str* s;
	int len;

	MD5Init(&ctx);
	
	U_MD5Update(&ctx, &b_nonce->raw[0], 4 + 4);
	if (cfg && msg){
		/* auth extra checks => 2 md5s */
		len = 4 + 4 + 16 + 16;
#if defined USE_NC  || defined USE_OT_NONCE
		if (b_nonce->n.nid_pf & (NF_VALID_NC_ID | NF_VALID_OT_ID)){
			/* if extra auth checks enabled, nid & pf are after the 2nd md5 */
			U_MD5Update(&ctx, (unsigned char*)&b_nonce->n.nid_i,
                                                        nonce_nid_extra_size);
			len+=nonce_nid_extra_size;
		}
#endif /* USE_NC || USE_OT_NONCE */
		MD5Update(&ctx, secret1->s, secret1->len);
		MD5Final(&b_nonce->n.md5_1[0], &ctx);
		/* second MD5(auth_extra_checks) */
		MD5Init(&ctx);
		if (cfg & AUTH_CHECK_FULL_URI) {
			s = GET_RURI(msg);
			MD5Update(&ctx, s->s, s->len);
		}
		if ((cfg & AUTH_CHECK_CALLID) && 
			!(parse_headers(msg, HDR_CALLID_F, 0) < 0 || msg->callid == 0)) {
			MD5Update(&ctx, msg->callid->body.s, msg->callid->body.len);
		}
		if ((cfg & AUTH_CHECK_FROMTAG) &&
			!(parse_from_header(msg) < 0 )) {
			MD5Update(&ctx, get_from(msg)->tag_value.s, 
					  get_from(msg)->tag_value.len);
		}
		if (cfg & AUTH_CHECK_SRC_IP) {
			U_MD5Update(&ctx, msg->rcv.src_ip.u.addr, msg->rcv.src_ip.len);
		}
		MD5Update(&ctx, secret2->s, secret2->len);
		MD5Final(&b_nonce->n.md5_2[0], &ctx);
	}else{
		/* no extra checks => only one md5 */
		len = 4 + 4 + 16;
#if defined USE_NC || USE_OT_NONCE
		if (b_nonce->n_small.nid_pf & (NF_VALID_NC_ID | NF_VALID_OT_ID)){
			/* if extra auth checks are not enabled, nid & pf are after the
			 *  1st md5 */
			U_MD5Update(&ctx, (unsigned char*)&b_nonce->n_small.nid_i,
							nonce_nid_extra_size);
			len+=nonce_nid_extra_size;
		}
#endif /* USE_NC  || USE_OT_NONCE*/
		MD5Update(&ctx, secret1->s, secret1->len);
		MD5Final(&b_nonce->n.md5_1[0], &ctx);
	}
	
	return len;
}



/** Calculates the nonce string for RFC2617 digest authentication.
 * This function creates the nonce string as it will be sent to the
 * user agent in digest challenge. The format of the nonce string
 * depends on the value of three module parameters, auth_checks_register,
 * auth_checks_no_dlg, and auth_checks_in_dlg. These module parameters
 * control the amount of information from the SIP requst that will be
 * stored in the nonce string for verification purposes.
 *
 * If all three parameters contain zero then the nonce string consists
 * of time in seconds since 1.1. 1970 and a secret phrase:
 * <expire_time> <valid_since> MD5(<expire_time>, <valid_since>, secret)
 * If any of the parameters is not zero (some optional checks are enabled
 * then the nonce string will also contain MD5 hash of selected parts
 * of the SIP request:
 * <expire_time> <valid_since> MD5(<expire_time>, <valid_since>, secret1) MD5(<extra_checks>, secret2)
 * @param nonce  Pointer to a buffer of *nonce_len. It must have enough
 *               space to hold the nonce. MAX_NONCE_LEN should be always 
 *               safe.
 * @param nonce_len A value/result parameter. Initially it contains the
 *                  nonce buffer length. If the length is too small, it 
 *                  will be set to the needed length and the function will 
 *                  return error immediately. After a succesfull call it will 
 *                  contain the size of nonce written into the buffer, 
 *                  without the terminating 0.
 * @param cfg This is the value of one of the tree module parameters that
 *            control which optional checks are enabled/disabled and which
 *            parts of the message will be included in the nonce string.
 * @param since Time when nonce was created, i.e. nonce is valid since <valid_since> up to <expires>
 * @param expires Time in seconds after which the nonce will be considered 
 *                stale.
 * @param n_id    Nounce count and/or one-time nonce index value
 *                (32 bit counter)
 * @param pf      First 2 bits are flags, the rest is the index pool number
 *                 used if nonce counts or one-time nonces are enabled.
 *                The possible flags values are: NF_VALID_NC_ID which means
 *                the nonce-count support is enabled and NF_VALID_OT_ID 
 *                which means the one-time nonces support is enabled.
 *                The pool number can be obtained by and-ing with
 *                NF_POOL_NO_MASK
 * @param secret1 A secret used for the nonce expires integrity check:
 *                MD5(<expire_time>, <valid_since>, secret1).
 * @param secret2 A secret used for integrity check of the message parts 
 *                selected by auth_extra_checks (if any):
 *                MD5(<msg_parts(auth_extra_checks)>, secret2).
 * @param msg     The message for which the nonce is computed. If 
 *                auth_extra_checks is set, the MD5 of some fields of the
 *                message will be included in the  generated nonce.
 * @return 0 on success and -1 on error
 */
int calc_nonce(char* nonce, int *nonce_len, int cfg, int since, int expires,
#if defined USE_NC || defined USE_OT_NONCE
					unsigned int n_id, unsigned char pf,
#endif /* USE_NC || USE_OT_NONCE */
					str* secret1, str* secret2,
					struct sip_msg* msg)
{
	union bin_nonce b_nonce;
	int len;
	if (unlikely(*nonce_len < MAX_NONCE_LEN)) {
		len=get_nonce_len(cfg, pf & NF_VALID_NC_ID);
		if (unlikely(*nonce_len<len)){
			*nonce_len=len;
			return -1;
		}
	}

	BIN_NONCE_PREPARE(&b_nonce, expires, since, n_id, pf, cfg, msg);
	len=calc_bin_nonce_md5(&b_nonce, cfg, secret1, secret2, msg);
	*nonce_len=base64_enc(&b_nonce.raw[0], len, 
							(unsigned char*)nonce, *nonce_len);
	assert(*nonce_len>=0); /*FIXME*/
	return 0;
}



/** Returns the expire time of the nonce string.
 * This function returns the absolute expire time
 * extracted from the nonce string in the parameter.
 * @param bn is a valid pointer to a union bin_nonce (decoded nonce)
 * @return Absolute time when the nonce string expires.
 */

#define get_bin_nonce_expire(bn) ((time_t)ntohl((bn)->n.expire))

/** Returns the valid_since time of the nonce string.
 * This function returns the absolute time
 * extracted from the nonce string in the parameter.
 * @param bn is a valid pointer to a union bin_nonce (decoded nonce)
 * @return Absolute time when the nonce string was created.
 */
#define get_bin_nonce_since(bn) ((time_t)ntohl((bn)->n.since))



/** Checks if nonce is stale.
 * This function checks if a nonce given to it in the parameter is stale. 
 * A nonce is stale if the expire time stored in the nonce is in the past.
 * @param b_nonce a pointer to a union bin_nonce to be checked.
 * @return 1 the nonce is stale, 0 the nonce is not stale.
 */
#define is_bin_nonce_stale(b_nonce, t) (get_bin_nonce_expire(b_nonce) < (t))



/** Utility to convert 8 hex digit string to int */
static inline int l8hex2int(char* _s, unsigned int *_r)
{
    unsigned int i, res = 0;

    for(i = 0; i < 8; i++) {
	res *= 16;
	if ((_s[i] >= '0') && (_s[i] <= '9')) {
	    res += _s[i] - '0';
	} else if ((_s[i] >= 'a') && (_s[i] <= 'f')) {
	    res += _s[i] - 'a' + 10;
	} else if ((_s[i] >= 'A') && (_s[i] <= 'F')) {
	    res += _s[i] - 'A' + 10;
	} else return -1;
    }
    
    *_r = res;
    return 0;
}



/** Check whether the nonce returned by UA is valid.
 * This function checks whether the nonce string returned by UA
 * in digest response is valid. The function checks if the nonce
 * string hasn't expired, it verifies the secret stored in the nonce
 * string with the secret configured on the server. If any of the
 * optional extra integrity checks are enabled then it also verifies
 * whether the corresponding parts in the new SIP requests are same.
 * @param nonce A nonce string to be verified.
 * @param secret1 A secret used for the nonce expires integrity check:
 *                MD5(<expire_time>,, secret1).
 * @param secret2 A secret used for integrity check of the message parts 
 *                selected by auth_extra_checks (if any):
 *                MD5(<msg_parts(auth_extra_checks)>, secret2).
 * @param msg The message which contains the nonce being verified. 
 * @return 0 - success (the nonce was not tampered with and if 
 *             auth_extra_checks are enabled - the selected message fields
 *             have not changes from the time the nonce was generated)
 *         -1 - invalid nonce
 *          1 - nonce length too small
 *          2 - no match
 *          3 - nonce expires ok, but the auth_extra checks failed
 *          4 - stale
 *          5 - invalid nc value (not an unsigned int)
 *          6 - nonce reused
 */
int check_nonce(auth_body_t* auth, str* secret1, str* secret2,
					struct sip_msg* msg)
{
	str * nonce;
	int since, b_nonce2_len, b_nonce_len, cfg;
	union bin_nonce b_nonce;
	union bin_nonce b_nonce2;
	time_t t;
#if defined USE_NC || defined USE_OT_NONCE
	unsigned int n_id;
	unsigned char pf;
#endif /* USE_NC || USE_OT_NONCE */
#ifdef USE_NC
	unsigned int nc;
#endif

	cfg = get_auth_checks(msg);
	nonce=&auth->digest.nonce;

	if (unlikely(nonce->s == 0)) {
		return -1;  /* Invalid nonce */
	}
	
	if (unlikely(nonce->len<MIN_NONCE_LEN)){ 
		return 1; /* length musth be >= then minimum length */
	}
	
#if defined USE_NC || defined USE_OT_NONCE
	/* clear all possible nonce flags positions prior to decoding,
	 * to make sure they can be used even if the nonce is shorter */
	b_nonce.n.nid_pf=0;
	b_nonce.n_small.nid_pf=0;
#endif /* USE_NC || USE_OT_NONCE */
	
	/* decode nonce */
	b_nonce_len=base64_dec((unsigned char*)nonce->s, nonce->len,
							&b_nonce.raw[0], sizeof(b_nonce));
	if (unlikely(b_nonce_len < MIN_BIN_NONCE_LEN)){
		DBG("auth: check_nonce: base64_dec failed\n");
		return -1; /* error decoding the nonce (invalid nonce since we checked
		              the len of the base64 enc. nonce above)*/
	}
	
	since = get_bin_nonce_since(&b_nonce);
	if (unlikely(since < up_since)) {
		/* if valid_since time is time pointing before ser was started 
		 * then we consider nonce as stalled. 
		   It may be the nonce generated by previous ser instance having
		   different length (for example because of different auth.
		   checks)..  Therefore we force credentials to be rebuilt by UAC
		   without prompting for password */
		/* if current time is less than start time, reset the start time
		 * (e.g., after start, the system clock was set in the past) */
		t=time(0);
		if (t < up_since)
			up_since = t;
		if (since < t)
			return 4; 
	}
	t=time(0);
	if (unlikely((since > t) && ((since-t) > nonce_auth_max_drift) )){
		/* the nonce comes from the future, either because of an external
		 * time adjustment, or because it was generated by another host
		 * which has the time slightly unsynchronized */
		return 4; /* consider it stale */
	}
	b_nonce2=b_nonce; /*pre-fill it with the values from the received nonce*/
	b_nonce2.n.expire=b_nonce.n.expire;
	b_nonce2.n.since=b_nonce.n.since;
#if defined USE_NC || defined USE_OT_NONCE
	if (cfg){
		b_nonce2.n.nid_i=b_nonce.n.nid_i;
		b_nonce2.n.nid_pf=b_nonce.n.nid_pf;
		pf=b_nonce.n.nid_pf;
		n_id=ntohl(b_nonce.n.nid_i);
	}else{
		b_nonce2.n_small.nid_i=b_nonce.n_small.nid_i;
		b_nonce2.n_small.nid_pf=b_nonce.n_small.nid_pf;
		pf=b_nonce.n_small.nid_pf;
		n_id=ntohl(b_nonce.n_small.nid_i);
	}
#ifdef UE_NC
	if (unlikely(nc_enabled && !(pf & NF_VALID_NC_ID)) )
		/* nounce count enabled, but nonce is not marked as nonce count ready
		 * or is too short => either an old nonce (should
		 * be caught by the ser start time  check) or truncated nonce  */
		return 4; /* return stale for now */
	}
#endif /* USE_NC */
#ifdef USE_OT_NONCE
	if (unlikely(otn_enabled && !(pf & NF_VALID_OT_ID))){
		/* same as above for one-time-nonce */
		return 4; /* return stale for now */
	}
#endif  /* USE_OT_NONCE */
	/* don't check if we got the expected length, if the length is smaller 
	 * then expected then  the md5 check below will fail (since the nid 
	 * members of the bin_nonce struct will be 0); if the length is bigger
	 * and it was not caught by the base64_dec above, and the md5 matches,
	 * we ignore the extra stuff */
#endif /* USE_NC || USE_OT_NONCE */
	b_nonce2_len=calc_bin_nonce_md5(&b_nonce2, cfg, secret1, secret2, msg);
	if (!memcmp(&b_nonce.n.md5_1[0], &b_nonce2.n.md5_1[0], 16)) {
#ifdef USE_NC
		/* if nounce-count checks enabled & auth. headers has nc */
		if (nc_enabled && (pf & NF_VALID_NC_ID) && auth->digest.nc.s &&
				auth->digest.nc.len){
		        if ((auth->digest.nc.len != 8) ||
			    l8hex2int(auth->digest.nc.s, &nc) != 0) {
			    ERR("check_nonce: bad nc value %.*s\n",
			        auth->digest.nc.len, auth->digest.nc.s);
			    return 5; /* invalid nc */
			}
			switch(nc_check_val(n_id, pf & NF_POOL_NO_MASK, nc)){
				case NC_OK:
					/* don't perform extra checks or one-time nonce checks
					 * anymore, if we have nc */
					goto check_stale;
				case NC_ID_OVERFLOW: /* id too old => stale */
				case NC_TOO_BIG:  /* nc overlfow => force re-auth => stale */
				case NC_REPLAY:    /* nc seen before => re-auth => stale */
				case NC_INV_POOL: /* pool-no too big, maybe ser restart?*/
					return 4; /* stale */
			}
		}
#endif /* USE_NC */
#ifdef USE_OT_NONCE
		if (otn_enabled && (pf & NF_VALID_OT_ID)){
			switch(otn_check_id(n_id, pf & NF_POOL_NO_MASK)){
				case OTN_OK:
					/* continue in case auth extra checks are enabled */
					break;
				case OTN_ID_OVERFLOW:
				case OTN_INV_POOL:
				case OTN_REPLAY:
					return 6; /* reused */
			}
		}
#endif
		if (cfg) {
			if (unlikely(b_nonce_len != b_nonce2_len))
				return 2; /* someone truncated our nonce? */
			if (memcmp(&b_nonce.n.md5_2[0], &b_nonce2.n.md5_2[0], 16))
				return 3; /* auth_extra_checks failed */
		}
#ifdef USE_NC
check_stale:
#endif /* USE_NC */
		if (unlikely(is_bin_nonce_stale(&b_nonce, t)))
			return 4;
		return 0;
	}
	
	return 2;
}

