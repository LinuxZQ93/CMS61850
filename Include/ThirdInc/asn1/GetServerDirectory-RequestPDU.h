/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_GetServerDirectory_RequestPDU_H_
#define	_GetServerDirectory_RequestPDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "ObjectReference.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GetServerDirectory_RequestPDU__objectClass {
	GetServerDirectory_RequestPDU__objectClass_reserved	= 0,
	GetServerDirectory_RequestPDU__objectClass_logical_device	= 1,
	GetServerDirectory_RequestPDU__objectClass_file_system	= 2
} e_GetServerDirectory_RequestPDU__objectClass;

/* GetServerDirectory-RequestPDU */
typedef struct GetServerDirectory_RequestPDU {
	long	 objectClass;
	ObjectReference_t	*referenceAfter;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetServerDirectory_RequestPDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetServerDirectory_RequestPDU;

#ifdef __cplusplus
}
#endif

#endif	/* _GetServerDirectory_RequestPDU_H_ */
#include <asn_internal.h>
