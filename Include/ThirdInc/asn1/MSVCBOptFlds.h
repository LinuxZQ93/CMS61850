/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_MSVCBOptFlds_H_
#define	_MSVCBOptFlds_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MSVCBOptFlds {
	MSVCBOptFlds_refresh_time	= 0,
	MSVCBOptFlds_reserved	= 1,
	MSVCBOptFlds_sample_rate	= 2,
	MSVCBOptFlds_data_set_name	= 3,
	MSVCBOptFlds_security	= 4
} e_MSVCBOptFlds;

/* MSVCBOptFlds */
typedef BIT_STRING_t	 MSVCBOptFlds_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_MSVCBOptFlds_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_MSVCBOptFlds;
asn_struct_free_f MSVCBOptFlds_free;
asn_struct_print_f MSVCBOptFlds_print;
asn_constr_check_f MSVCBOptFlds_constraint;
per_type_decoder_f MSVCBOptFlds_decode_uper;
per_type_encoder_f MSVCBOptFlds_encode_uper;
per_type_decoder_f MSVCBOptFlds_decode_aper;
per_type_encoder_f MSVCBOptFlds_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _MSVCBOptFlds_H_ */
#include <asn_internal.h>
