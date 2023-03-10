/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetLogicalNodeDirectory_ResponsePDU_H_
#define	_GetLogicalNodeDirectory_ResponsePDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BOOLEAN.h>
#include "SubReference.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GetLogicalNodeDirectory-ResponsePDU */
typedef struct GetLogicalNodeDirectory_ResponsePDU {
	struct GetLogicalNodeDirectory_ResponsePDU__reference {
		A_SEQUENCE_OF(SubReference_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} reference;
	BOOLEAN_t	*moreFollows;	/* DEFAULT TRUE */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetLogicalNodeDirectory_ResponsePDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetLogicalNodeDirectory_ResponsePDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetLogicalNodeDirectory_ResponsePDU_H_ */
#include <asn_internal.h>
