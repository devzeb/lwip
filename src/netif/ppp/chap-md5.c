/*
 * chap-md5.c - New CHAP/MD5 implementation.
 *
 * Copyright (c) 2003 Paul Mackerras. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. The name(s) of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 3. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Paul Mackerras
 *     <paulus@samba.org>".
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lwip/opt.h"
#if PPP_SUPPORT && CHAP_SUPPORT  /* don't build if not configured for use in lwipopts.h */

#if 0 /* UNUSED */
#include <stdlib.h>
#include <string.h>
#endif /* UNUSED */

#include "netif/ppp/ppp_impl.h"

#include "netif/ppp/chap-new.h"
#include "netif/ppp/chap-md5.h"
#include "netif/ppp/magic.h"

#if LWIP_INCLUDED_POLARSSL_MD5
#include "netif/ppp/polarssl/md5.h"
#else
#include "polarssl/md5.h"
#endif

#define MD5_HASH_SIZE		16
#define MD5_MIN_CHALLENGE	16
#define MD5_MAX_CHALLENGE	24

#if PPP_SERVER
static void chap_md5_generate_challenge(unsigned char *cp) {
	int clen;

	clen = (int)(drand48() * (MD5_MAX_CHALLENGE - MD5_MIN_CHALLENGE))
		+ MD5_MIN_CHALLENGE;
	*cp++ = clen;
	random_bytes(cp, clen);
}

static int chap_md5_verify_response(int id, char *name,
			 unsigned char *secret, int secret_len,
			 unsigned char *challenge, unsigned char *response,
			 char *message, int message_space) {
	md5_context ctx;
	unsigned char idbyte = id;
	unsigned char hash[MD5_HASH_SIZE];
	int challenge_len, response_len;

	challenge_len = *challenge++;
	response_len = *response++;
	if (response_len == MD5_HASH_SIZE) {
		/* Generate hash of ID, secret, challenge */
		md5_starts(&ctx);
		md5_update(&ctx, &idbyte, 1);
		md5_update(&ctx, secret, secret_len);
		md5_update(&ctx, challenge, challenge_len);
		md5_finish(&ctx, hash);

		/* Test if our hash matches the peer's response */
		if (memcmp(hash, response, MD5_HASH_SIZE) == 0) {
			slprintf(message, message_space, "Access granted");
			return 1;
		}
	}
	slprintf(message, message_space, "Access denied");
	return 0;
}
#endif /* PPP_SERVER */

static void chap_md5_make_response(unsigned char *response, int id, char *our_name,
		       unsigned char *challenge, char *secret, int secret_len,
		       unsigned char *private) {
	md5_context ctx;
	unsigned char idbyte = id;
	int challenge_len = *challenge++;

	md5_starts(&ctx);
	md5_update(&ctx, &idbyte, 1);
	md5_update(&ctx, (u_char *)secret, secret_len);
	md5_update(&ctx, challenge, challenge_len);
	md5_finish(&ctx, &response[1]);
	response[0] = MD5_HASH_SIZE;
}

static struct chap_digest_type md5_digest = {
	CHAP_MD5,		/* code */
#if PPP_SERVER
	chap_md5_generate_challenge,
	chap_md5_verify_response,
#endif /* PPP_SERVER */
	chap_md5_make_response,
	NULL,			/* check_success */
	NULL,			/* handle_failure */
};

void chap_md5_init(void) {
	chap_register_digest(&md5_digest);
}

#endif /* PPP_SUPPORT && CHAP_SUPPORT */
