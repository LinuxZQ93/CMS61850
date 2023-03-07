/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_Originator_H_
#define	_Originator_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Originator__orCat {
	Originator__orCat_notSupported	= 0,
	Originator__orCat_bayControl	= 1,
	Originator__orCat_stationControl	= 2,
	Originator__orCat_remoteControl	= 3,
	Originator__orCat_automaticBay	= 4,
	Originator__orCat_automaticStation	= 5,
	Originator__orCat_automaticRemote	= 6,
	Originator__orCat_maintenance	= 7,
	Originator__orCat_process	= 8
} e_Originator__orCat;

/* Originator */
typedef struct Originator {
	long	 orCat;
	OCTET_STRING_t	 orIdent;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Originator_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Originator;

#ifdef __cplusplus
}
#endif

#endif	/* _Originator_H_ */
#include <asn_internal.h>