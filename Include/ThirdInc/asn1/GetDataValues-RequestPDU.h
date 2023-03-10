/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetDataValues_RequestPDU_H_
#define	_GetDataValues_RequestPDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include "ObjectReference.h"
#include "FunctionalConstraint.h"
#include <constr_SEQUENCE.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward definitions */
typedef struct GetDataValues_RequestPDU__data__Member {
	ObjectReference_t	 reference;
	FunctionalConstraint_t	*fc;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetDataValues_RequestPDU__data__Member;

/* GetDataValues-RequestPDU */
typedef struct GetDataValues_RequestPDU {
	struct GetDataValues_RequestPDU__data {
		A_SEQUENCE_OF(GetDataValues_RequestPDU__data__Member) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} data;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetDataValues_RequestPDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetDataValues_RequestPDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetDataValues_RequestPDU_H_ */
#include <asn_internal.h>
