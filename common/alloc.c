/* alloc.c

   Memory allocation... */

/*
 * Copyright (c) 1996-2000 Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about the Internet Software Consortium, see
 * ``http://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#ifndef lint
static char copyright[] =
"$Id: alloc.c,v 1.53.2.2 2001/06/05 17:48:21 mellon Exp $ Copyright (c) 1996-2000 The Internet Software Consortium.  All rights reserved.\n";
#endif /* not lint */

#include "dhcpd.h"
#include <omapip/omapip_p.h>

struct dhcp_packet *dhcp_free_list;
struct packet *packet_free_list;

OMAPI_OBJECT_ALLOC (subnet, struct subnet, dhcp_type_subnet)
OMAPI_OBJECT_ALLOC (shared_network, struct shared_network,
		    dhcp_type_shared_network)
OMAPI_OBJECT_ALLOC (group_object, struct group_object, dhcp_type_group)
OMAPI_OBJECT_ALLOC (dhcp_control, dhcp_control_object_t, dhcp_type_control)

int option_chain_head_allocate (ptr, file, line)
	struct option_chain_head **ptr;
	const char *file;
	int line;
{
	int size;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct option_chain_head *)0;
#endif
	}

	*ptr = dmalloc (sizeof **ptr, file, line);
	if (*ptr) {
		memset (*ptr, 0, sizeof **ptr);
		(*ptr) -> refcnt = 1;
		return 1;
	}
	return 0;
}

int option_chain_head_reference (ptr, bp, file, line)
	struct option_chain_head **ptr;
	struct option_chain_head *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct option_chain_head *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int option_chain_head_dereference (ptr, file, line)
	struct option_chain_head **ptr;
	const char *file;
	int line;
{
	int i;
	struct option_chain_head *option_chain_head;
	pair car, cdr;

	if (!ptr || !*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	option_chain_head = *ptr;
	*ptr = (struct option_chain_head *)0;
	--option_chain_head -> refcnt;
	rc_register (file, line, ptr,
		     option_chain_head, option_chain_head -> refcnt);
	if (option_chain_head -> refcnt > 0)
		return 1;

	if (option_chain_head -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	/* If there are any options on this head, free them. */
	for (car = option_chain_head -> first; car; car = cdr) {
		cdr = car -> cdr;
		if (car -> car)
			option_cache_dereference ((struct option_cache **)
						  (&car -> car), MDL);
		dfree (car, MDL);
		car = cdr;
	}

	dfree (option_chain_head, file, line);
	return 1;
}

int group_allocate (ptr, file, line)
	struct group **ptr;
	const char *file;
	int line;
{
	int size;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct group *)0;
#endif
	}

	*ptr = dmalloc (sizeof **ptr, file, line);
	if (*ptr) {
		memset (*ptr, 0, sizeof **ptr);
		(*ptr) -> refcnt = 1;
		return 1;
	}
	return 0;
}

int group_reference (ptr, bp, file, line)
	struct group **ptr;
	struct group *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct group *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int group_dereference (ptr, file, line)
	struct group **ptr;
	const char *file;
	int line;
{
	int i;
	struct group *group;

	if (!ptr || !*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	group = *ptr;
	*ptr = (struct group *)0;
	--group -> refcnt;
	rc_register (file, line, ptr, group, group -> refcnt);
	if (group -> refcnt > 0)
		return 1;

	if (group -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	if (group -> object)
		group_object_dereference (&group -> object, MDL);
	if (group -> subnet)	
		subnet_dereference (&group -> subnet, MDL);
	if (group -> shared_network)
		shared_network_dereference (&group -> shared_network, MDL);
	if (group -> statements)
		executable_statement_dereference (&group -> statements, MDL);
	dfree (group, file, line);
	return 1;
}

struct dhcp_packet *new_dhcp_packet (file, line)
	const char *file;
	int line;
{
	struct dhcp_packet *rval;
	rval = (struct dhcp_packet *)dmalloc (sizeof (struct dhcp_packet),
					      file, line);
	return rval;
}

struct protocol *new_protocol (file, line)
	const char *file;
	int line;
{
	struct protocol *rval = dmalloc (sizeof (struct protocol), file, line);
	return rval;
}

struct domain_search_list *new_domain_search_list (file, line)
	const char *file;
	int line;
{
	struct domain_search_list *rval =
		dmalloc (sizeof (struct domain_search_list), file, line);
	return rval;
}

struct name_server *new_name_server (file, line)
	const char *file;
	int line;
{
	struct name_server *rval =
		dmalloc (sizeof (struct name_server), file, line);
	return rval;
}

void free_name_server (ptr, file, line)
	struct name_server *ptr;
	const char *file;
	int line;
{
	dfree ((VOIDPTR)ptr, file, line);
}

struct option *new_option (file, line)
	const char *file;
	int line;
{
	struct option *rval =
		dmalloc (sizeof (struct option), file, line);
	if (rval)
		memset (rval, 0, sizeof *rval);
	return rval;
}

void free_option (ptr, file, line)
	struct option *ptr;
	const char *file;
	int line;
{
/* XXX have to put all options on heap before this is possible. */
#if 0
	if (ptr -> name)
		dfree ((VOIDPTR)option -> name, file, line);
	dfree ((VOIDPTR)ptr, file, line);
#endif
}

struct universe *new_universe (file, line)
	const char *file;
	int line;
{
	struct universe *rval =
		dmalloc (sizeof (struct universe), file, line);
	return rval;
}

void free_universe (ptr, file, line)
	struct universe *ptr;
	const char *file;
	int line;
{
	dfree ((VOIDPTR)ptr, file, line);
}

void free_domain_search_list (ptr, file, line)
	struct domain_search_list *ptr;
	const char *file;
	int line;
{
	dfree ((VOIDPTR)ptr, file, line);
}

void free_protocol (ptr, file, line)
	struct protocol *ptr;
	const char *file;
	int line;
{
	dfree ((VOIDPTR)ptr, file, line);
}

void free_dhcp_packet (ptr, file, line)
	struct dhcp_packet *ptr;
	const char *file;
	int line;
{
	dfree ((VOIDPTR)ptr, file, line);
}

struct client_lease *new_client_lease (file, line)
	const char *file;
	int line;
{
	return (struct client_lease *)dmalloc (sizeof (struct client_lease),
					       file, line);
}

void free_client_lease (lease, file, line)
	struct client_lease *lease;
	const char *file;
	int line;
{
	dfree (lease, file, line);
}

pair free_pairs;

pair new_pair (file, line)
	const char *file;
	int line;
{
	pair foo;

	if (free_pairs) {
		foo = free_pairs;
		free_pairs = foo -> cdr;
		memset (foo, 0, sizeof *foo);
		dmalloc_reuse (foo, file, line, 0);
		return foo;
	}

	foo = dmalloc (sizeof *foo, file, line);
	if (!foo)
		return foo;
	memset (foo, 0, sizeof *foo);
	return foo;
}

void free_pair (foo, file, line)
	pair foo;
	const char *file;
	int line;
{
	foo -> cdr = free_pairs;
	free_pairs = foo;
	dmalloc_reuse (free_pairs, (char *)0, 0, 0);
}

#if defined (DEBUG_MEMORY_LEAKAGE)
void relinquish_free_pairs ()
{
	pair pf, pc;

	for (pf = free_pairs; pf; pf = pc) {
		pc = pf -> cdr;
		dfree (pf, MDL);
	}
	free_pairs = (pair)0;
}
#endif

struct expression *free_expressions;

int expression_allocate (cptr, file, line)
	struct expression **cptr;
	const char *file;
	int line;
{
	struct expression *rval;

	if (free_expressions) {
		rval = free_expressions;
		free_expressions = rval -> data.not;
	} else {
		rval = dmalloc (sizeof (struct expression), file, line);
		if (!rval)
			return 0;
	}
	memset (rval, 0, sizeof *rval);
	return expression_reference (cptr, rval, file, line);
}

int expression_reference (ptr, src, file, line)
	struct expression **ptr;
	struct expression *src;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct expression *)0;
#endif
	}
	*ptr = src;
	src -> refcnt++;
	rc_register (file, line, ptr, src, src -> refcnt);
	dmalloc_reuse (src, file, line, 1);
	return 1;
}

void free_expression (expr, file, line)
	struct expression *expr;
	const char *file;
	int line;
{
	expr -> data.not = free_expressions;
	free_expressions = expr;
	dmalloc_reuse (free_expressions, (char *)0, 0, 0);
}

#if defined (DEBUG_MEMORY_LEAKAGE)
void relinquish_free_expressions ()
{
	struct expression *e, *n;

	for (e = free_expressions; e; e = n) {
		n = e -> data.not;
		dfree (e, MDL);
	}
	free_expressions = (struct expression *)0;
}
#endif

struct binding_value *free_binding_values;
				
int binding_value_allocate (cptr, file, line)
	struct binding_value **cptr;
	const char *file;
	int line;
{
	struct binding_value *rval;

	if (free_binding_values) {
		rval = free_binding_values;
		free_binding_values = rval -> value.bv;
	} else {
		rval = dmalloc (sizeof (struct binding_value), file, line);
		if (!rval)
			return 0;
	}
	memset (rval, 0, sizeof *rval);
	return binding_value_reference (cptr, rval, file, line);
}

int binding_value_reference (ptr, src, file, line)
	struct binding_value **ptr;
	struct binding_value *src;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct binding_value *)0;
#endif
	}
	*ptr = src;
	src -> refcnt++;
	rc_register (file, line, ptr, src, src -> refcnt);
	dmalloc_reuse (src, file, line, 1);
	return 1;
}

void free_binding_value (bv, file, line)
	struct binding_value *bv;
	const char *file;
	int line;
{
	bv -> value.bv = free_binding_values;
	free_binding_values = bv;
	dmalloc_reuse (free_binding_values, (char *)0, 0, 0);
}

#if defined (DEBUG_MEMORY_LEAKAGE)
void relinquish_free_binding_values ()
{
	struct binding_value *b, *n;

	for (b = free_binding_values; b; b = n) {
		n = b -> value.bv;
		dfree (b, MDL);
	}
	free_binding_values = (struct binding_value *)0;
}
#endif

int fundef_allocate (cptr, file, line)
	struct fundef **cptr;
	const char *file;
	int line;
{
	struct fundef *rval;

	rval = dmalloc (sizeof (struct fundef), file, line);
	if (!rval)
		return 0;
	memset (rval, 0, sizeof *rval);
	return fundef_reference (cptr, rval, file, line);
}

int fundef_reference (ptr, src, file, line)
	struct fundef **ptr;
	struct fundef *src;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct fundef *)0;
#endif
	}
	*ptr = src;
	src -> refcnt++;
	rc_register (file, line, ptr, src, src -> refcnt);
	dmalloc_reuse (src, file, line, 1);
	return 1;
}

struct option_cache *free_option_caches;

#if defined (DEBUG_MEMORY_LEAKAGE)
void relinquish_free_option_caches ()
{
	struct option_cache *o, *n;

	for (o = free_option_caches; o; o = n) {
		n = (struct option_cache *)(o -> expression);
		dfree (o, MDL);
	}
	free_option_caches = (struct option_cache *)0;
}
#endif

int option_cache_allocate (cptr, file, line)
	struct option_cache **cptr;
	const char *file;
	int line;
{
	struct option_cache *rval;

	if (free_option_caches) {
		rval = free_option_caches;
		free_option_caches =
			(struct option_cache *)(rval -> expression);
		dmalloc_reuse (rval, file, line, 0);
	} else {
		rval = dmalloc (sizeof (struct option_cache), file, line);
		if (!rval)
			return 0;
	}
	memset (rval, 0, sizeof *rval);
	return option_cache_reference (cptr, rval, file, line);
}

int option_cache_reference (ptr, src, file, line)
	struct option_cache **ptr;
	struct option_cache *src;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct option_cache *)0;
#endif
	}
	*ptr = src;
	src -> refcnt++;
	rc_register (file, line, ptr, src, src -> refcnt);
	dmalloc_reuse (src, file, line, 1);
	return 1;
}

int buffer_allocate (ptr, len, file, line)
	struct buffer **ptr;
	unsigned len;
	const char *file;
	int line;
{
	struct buffer *bp;

	bp = dmalloc (len + sizeof *bp, file, line);
	if (!bp)
		return 0;
	memset (bp, 0, sizeof *bp);
	bp -> refcnt = 0;
	return buffer_reference (ptr, bp, file, line);
}

int buffer_reference (ptr, bp, file, line)
	struct buffer **ptr;
	struct buffer *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct buffer *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int buffer_dereference (ptr, file, line)
	struct buffer **ptr;
	const char *file;
	int line;
{
	struct buffer *bp;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	if (!*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	(*ptr) -> refcnt--;
	rc_register (file, line, ptr, *ptr, (*ptr) -> refcnt);
	if (!(*ptr) -> refcnt) {
		dfree ((*ptr), file, line);
	} else if ((*ptr) -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	*ptr = (struct buffer *)0;
	return 1;
}

int dns_host_entry_allocate (ptr, hostname, file, line)
	struct dns_host_entry **ptr;
	const char *hostname;
	const char *file;
	int line;
{
	struct dns_host_entry *bp;

	bp = dmalloc (strlen (hostname) + sizeof *bp, file, line);
	if (!bp)
		return 0;
	memset (bp, 0, sizeof *bp);
	bp -> refcnt = 0;
	strcpy (bp -> hostname, hostname);
	return dns_host_entry_reference (ptr, bp, file, line);
}

int dns_host_entry_reference (ptr, bp, file, line)
	struct dns_host_entry **ptr;
	struct dns_host_entry *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct dns_host_entry *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int dns_host_entry_dereference (ptr, file, line)
	struct dns_host_entry **ptr;
	const char *file;
	int line;
{
	struct dns_host_entry *bp;

	if (!ptr || !*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	(*ptr) -> refcnt--;
	rc_register (file, line, ptr, *ptr, (*ptr) -> refcnt);
	if (!(*ptr) -> refcnt)
		dfree ((*ptr), file, line);
	if ((*ptr) -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	*ptr = (struct dns_host_entry *)0;
	return 1;
}

int option_state_allocate (ptr, file, line)
	struct option_state **ptr;
	const char *file;
	int line;
{
	unsigned size;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct option_state *)0;
#endif
	}

	size = sizeof **ptr + (universe_count - 1) * sizeof (VOIDPTR);
	*ptr = dmalloc (size, file, line);
	if (*ptr) {
		memset (*ptr, 0, size);
		(*ptr) -> universe_count = universe_count;
		(*ptr) -> refcnt = 1;
		rc_register (file, line, ptr, *ptr, (*ptr) -> refcnt);
		return 1;
	}
	return 0;
}

int option_state_reference (ptr, bp, file, line)
	struct option_state **ptr;
	struct option_state *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct option_state *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int option_state_dereference (ptr, file, line)
	struct option_state **ptr;
	const char *file;
	int line;
{
	int i;
	struct option_state *options;

	if (!ptr || !*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	options = *ptr;
	*ptr = (struct option_state *)0;
	--options -> refcnt;
	rc_register (file, line, ptr, options, options -> refcnt);
	if (options -> refcnt > 0)
		return 1;

	if (options -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	/* Loop through the per-universe state. */
	for (i = 0; i < options -> universe_count; i++)
		if (options -> universes [i] &&
		    universes [i] -> option_state_dereference)
			((*(universes [i] -> option_state_dereference))
			 (universes [i], options, file, line));
	dfree (options, file, line);
	return 1;
}

int executable_statement_allocate (ptr, file, line)
	struct executable_statement **ptr;
	const char *file;
	int line;
{
	struct executable_statement *bp;

	bp = dmalloc (sizeof *bp, file, line);
	if (!bp)
		return 0;
	memset (bp, 0, sizeof *bp);
	return executable_statement_reference (ptr, bp, file, line);
}

int executable_statement_reference (ptr, bp, file, line)
	struct executable_statement **ptr;
	struct executable_statement *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct executable_statement *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

static struct packet *free_packets;

#if defined (DEBUG_MEMORY_LEAKAGE)
void relinquish_free_packets ()
{
	struct packet *p, *n;
	for (p = free_packets; p; p = n) {
		n = (struct packet *)(p -> raw);
		dfree (p, MDL);
	}
	free_packets = (struct packet *)0;
}
#endif

int packet_allocate (ptr, file, line)
	struct packet **ptr;
	const char *file;
	int line;
{
	int size;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct packet *)0;
#endif
	}

	if (free_packets) {
		*ptr = free_packets;
		free_packets = (struct packet *)((*ptr) -> raw);
	} else {
		*ptr = dmalloc (sizeof **ptr, file, line);
	}
	if (*ptr) {
		memset (*ptr, 0, sizeof **ptr);
		(*ptr) -> refcnt = 1;
		return 1;
	}
	return 0;
}

int packet_reference (ptr, bp, file, line)
	struct packet **ptr;
	struct packet *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct packet *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int packet_dereference (ptr, file, line)
	struct packet **ptr;
	const char *file;
	int line;
{
	int i;
	struct packet *packet;

	if (!ptr || !*ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	packet = *ptr;
	*ptr = (struct packet *)0;
	--packet -> refcnt;
	rc_register (file, line, ptr, packet, packet -> refcnt);
	if (packet -> refcnt > 0)
		return 1;

	if (packet -> refcnt < 0) {
		log_error ("%s(%d): negative refcnt!", file, line);
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	if (packet -> options)
		option_state_dereference (&packet -> options, file, line);
	if (packet -> interface)
		interface_dereference (&packet -> interface, MDL);
	packet -> raw = (struct dhcp_packet *)free_packets;
	free_packets = packet;
	dmalloc_reuse (free_packets, (char *)0, 0, 0);
	return 1;
}

int dns_zone_allocate (ptr, file, line)
	struct dns_zone **ptr;
	const char *file;
	int line;
{
	int size;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct dns_zone *)0;
#endif
	}

	*ptr = dmalloc (sizeof **ptr, file, line);
	if (*ptr) {
		memset (*ptr, 0, sizeof **ptr);
		(*ptr) -> refcnt = 1;
		return 1;
	}
	return 0;
}

int dns_zone_reference (ptr, bp, file, line)
	struct dns_zone **ptr;
	struct dns_zone *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct dns_zone *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

int binding_scope_allocate (ptr, file, line)
	struct binding_scope **ptr;
	const char *file;
	int line;
{
	struct binding_scope *bp;

	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}

	bp = dmalloc (sizeof *bp, file, line);
	if (!bp)
		return 0;
	memset (bp, 0, sizeof *bp);
	binding_scope_reference (ptr, bp, file, line);
	return 1;
}

int binding_scope_reference (ptr, bp, file, line)
	struct binding_scope **ptr;
	struct binding_scope *bp;
	const char *file;
	int line;
{
	if (!ptr) {
		log_error ("%s(%d): null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		return 0;
#endif
	}
	if (*ptr) {
		log_error ("%s(%d): non-null pointer", file, line);
#if defined (POINTER_DEBUG)
		abort ();
#else
		*ptr = (struct binding_scope *)0;
#endif
	}
	*ptr = bp;
	bp -> refcnt++;
	rc_register (file, line, ptr, bp, bp -> refcnt);
	dmalloc_reuse (bp, file, line, 1);
	return 1;
}

/* Make a copy of the data in data_string, upping the buffer reference
   count if there's a buffer. */

void data_string_copy (dest, src, file, line)
	struct data_string *dest;
	struct data_string *src;
	const char *file;
	int line;
{
	if (src -> buffer)
		buffer_reference (&dest -> buffer, src -> buffer, file, line);
	dest -> data = src -> data;
	dest -> terminated = src -> terminated;
	dest -> len = src -> len;
}

/* Release the reference count to a data string's buffer (if any) and
   zero out the other information, yielding the null data string. */

void data_string_forget (data, file, line)
	struct data_string *data;
	const char *file;
	int line;
{
	if (data -> buffer)
		buffer_dereference (&data -> buffer, file, line);
	memset (data, 0, sizeof *data);
}

/* Make a copy of the data in data_string, upping the buffer reference
   count if there's a buffer. */

void data_string_truncate (dp, len)
	struct data_string *dp;
	int len;
{
	if (len < dp -> len) {
		dp -> terminated = 0;
		dp -> len = len;
	}
}
