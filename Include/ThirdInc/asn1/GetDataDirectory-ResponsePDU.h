/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetDataDirectory_ResponsePDU_H_
#define	_GetDataDirectory_ResponsePDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BOOLEAN.h>
#include <asn_SEQUENCE_OF.h>
#include "SubReference.h"
#include "FunctionalConstraint.h"
#include <constr_SEQUENCE.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward definitions */
typedef struct GetDataDirectory_ResponsePDU__dataAttribute__Member {
	SubReference_t	 reference;
	FunctionalConstraint_t	*fc;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetDataDirectory_ResponsePDU__dataAttribute__Member;

/* GetDataDirectory-ResponsePDU */
typedef struct GetDataDirectory_ResponsePDU {
	struct GetDataDirectory_ResponsePDU__dataAttribute {
		A_SEQUENCE_OF(GetDataDirectory_ResponsePDU__dataAttribute__Member) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} dataAttribute;
	BOOLEAN_t	*moreFollows;	/* DEFAULT TRUE */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetDataDirectory_ResponsePDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetDataDirectory_ResponsePDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetDataDirectory_ResponsePDU_H_ */
#include <asn_internal.h>
