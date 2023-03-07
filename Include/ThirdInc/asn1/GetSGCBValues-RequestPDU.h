/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetSGCBValues_RequestPDU_H_
#define	_GetSGCBValues_RequestPDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ObjectReference.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GetSGCBValues-RequestPDU */
typedef struct GetSGCBValues_RequestPDU {
	struct GetSGCBValues_RequestPDU__sgcbReference {
		A_SEQUENCE_OF(ObjectReference_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} sgcbReference;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetSGCBValues_RequestPDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetSGCBValues_RequestPDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetSGCBValues_RequestPDU_H_ */
#include <asn_internal.h>