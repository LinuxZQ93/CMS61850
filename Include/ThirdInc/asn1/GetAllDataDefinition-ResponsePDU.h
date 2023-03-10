/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetAllDataDefinition_ResponsePDU_H_
#define	_GetAllDataDefinition_ResponsePDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BOOLEAN.h>
#include <asn_SEQUENCE_OF.h>
#include "SubReference.h"
#include <VisibleString.h>
#include "DataDefinition.h"
#include <constr_SEQUENCE.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward definitions */
typedef struct GetAllDataDefinition_ResponsePDU__data__Member {
	SubReference_t	 reference;
	VisibleString_t	*cdcType;	/* OPTIONAL */
	DataDefinition_t	 definition;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetAllDataDefinition_ResponsePDU__data__Member;

/* GetAllDataDefinition-ResponsePDU */
typedef struct GetAllDataDefinition_ResponsePDU {
	struct GetAllDataDefinition_ResponsePDU__data {
		A_SEQUENCE_OF(GetAllDataDefinition_ResponsePDU__data__Member) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} data;
	BOOLEAN_t	*moreFollows;	/* DEFAULT TRUE */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetAllDataDefinition_ResponsePDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetAllDataDefinition_ResponsePDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetAllDataDefinition_ResponsePDU_H_ */
#include <asn_internal.h>
