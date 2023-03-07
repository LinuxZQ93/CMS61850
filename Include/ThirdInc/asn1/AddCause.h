/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "CMS61850Module"
 * 	found in "./61850CMS.asn"
 * 	`asn1c -D ./out -gen-APER -no-gen-BER -no-gen-OER -no-gen-XER -no-gen-JER -no-gen-example -fcompound-names`
 */

#ifndef	_AddCause_H_
#define	_AddCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AddCause {
	AddCause_unknown	= 0,
	AddCause_not_supported	= 1,
	AddCause_blocked_by_switching_hierarchy	= 2,
	AddCause_select_failed	= 3,
	AddCause_invalid_position	= 4,
	AddCause_position_reached	= 5,
	AddCause_parameter_change_in_execution	= 6,
	AddCause_step_limit	= 7,
	AddCause_blocked_by_Mode	= 8,
	AddCause_blocked_by_process	= 9,
	AddCause_blocked_by_interlocking	= 10,
	AddCause_blocked_by_synchrocheck	= 11,
	AddCause_command_already_in_execution	= 12,
	AddCause_blocked_by_health	= 13,
	AddCause_one_of_n_control	= 14,
	AddCause_abortion_by_cancel	= 15,
	AddCause_time_limit_over	= 16,
	AddCause_abortion_by_trip	= 17,
	AddCause_object_not_selected	= 18,
	AddCause_object_already_selected	= 19,
	AddCause_no_access_authority	= 20,
	AddCause_ended_with_overshoot	= 21,
	AddCause_abortion_due_to_deviation	= 22,
	AddCause_abortion_by_communication_loss	= 23,
	AddCause_blocked_by_command	= 24,
	AddCause_none	= 25,
	AddCause_locked_by_other_client	= 26,
	AddCause_inconsistent_parameters	= 27
} e_AddCause;

/* AddCause */
typedef long	 AddCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AddCause;
asn_struct_free_f AddCause_free;
asn_struct_print_f AddCause_print;
asn_constr_check_f AddCause_constraint;
per_type_decoder_f AddCause_decode_uper;
per_type_encoder_f AddCause_encode_uper;
per_type_decoder_f AddCause_decode_aper;
per_type_encoder_f AddCause_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _AddCause_H_ */
#include <asn_internal.h>