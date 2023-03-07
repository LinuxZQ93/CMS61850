/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_SetDataValues_ErrorPDU_H_
#define	_SetDataValues_ErrorPDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ServiceError.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SetDataValues-ErrorPDU */
typedef struct SetDataValues_ErrorPDU {
	struct SetDataValues_ErrorPDU__result {
		A_SEQUENCE_OF(ServiceError_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} result;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SetDataValues_ErrorPDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SetDataValues_ErrorPDU;

#ifdef __cplusplus
}
#endif

#endif	/* _SetDataValues_ErrorPDU_H_ */
#include <asn_internal.h>
