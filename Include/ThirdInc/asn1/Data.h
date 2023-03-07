/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_Data_H_
#define	_Data_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ServiceError.h"
#include <BOOLEAN.h>
#include "INT8.h"
#include "INT16.h"
#include "INT32.h"
#include "INT64.h"
#include "INT8U.h"
#include "INT16U.h"
#include "INT32U.h"
#include "INT64U.h"
#include "FLOAT32.h"
#include "FLOAT64.h"
#include <BIT_STRING.h>
#include <OCTET_STRING.h>
#include <VisibleString.h>
#include <UTF8String.h>
#include "UtcTime.h"
#include "BinaryTime.h"
#include "Quality.h"
#include "Dbpos.h"
#include "Tcmd.h"
#include "Check.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Data_PR {
	Data_PR_NOTHING,	/* No components present */
	Data_PR_error,
	Data_PR_array,
	Data_PR_structure,
	Data_PR_boolean,
	Data_PR_int8,
	Data_PR_int16,
	Data_PR_int32,
	Data_PR_int64,
	Data_PR_int8u,
	Data_PR_int16u,
	Data_PR_int32u,
	Data_PR_int64u,
	Data_PR_float32,
	Data_PR_float64,
	Data_PR_bit_string,
	Data_PR_octet_string,
	Data_PR_visible_string,
	Data_PR_unicode_string,
	Data_PR_utc_time,
	Data_PR_binary_time,
	Data_PR_quality,
	Data_PR_dbpos,
	Data_PR_tcmd,
	Data_PR_check
} Data_PR;

/* Forward declarations */
struct Data;

/* Data */
typedef struct Data {
	Data_PR present;
	union Data_u {
		ServiceError_t	 error;
		struct Data__array {
			A_SEQUENCE_OF(struct Data) list;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} array;
		struct Data__structure {
			A_SEQUENCE_OF(struct Data) list;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} structure;
		BOOLEAN_t	 boolean;
		INT8_t	 int8;
		INT16_t	 int16;
		INT32_t	 int32;
		INT64_t	 int64;
		INT8U_t	 int8u;
		INT16U_t	 int16u;
		INT32U_t	 int32u;
		INT64U_t	 int64u;
		FLOAT32_t	 float32;
		FLOAT64_t	 float64;
		BIT_STRING_t	 bit_string;
		OCTET_STRING_t	 octet_string;
		VisibleString_t	 visible_string;
		UTF8String_t	 unicode_string;
		UtcTime_t	 utc_time;
		BinaryTime_t	 binary_time;
		Quality_t	 quality;
		Dbpos_t	 dbpos;
		Tcmd_t	 tcmd;
		Check_t	 check;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Data_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Data;
extern asn_CHOICE_specifics_t asn_SPC_Data_specs_1;
extern asn_TYPE_member_t asn_MBR_Data_1[24];
extern asn_per_constraints_t asn_PER_type_Data_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Data.h"

#endif	/* _Data_H_ */
#include <asn_internal.h>