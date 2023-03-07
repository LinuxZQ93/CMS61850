/*
 *  Copyright 2014-2022 The GmSSL Project. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the License); you may
 *  not use this file except in compliance with the License.
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 */


#ifndef GMSSL_X509_H
#define GMSSL_X509_H


#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <gmssl/sm2.h>
#include <gmssl/oid.h>
#include <gmssl/asn1.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
X509 Public API

	x509_name_add_rdn
	x509_name_add_country_name
	x509_name_add_state_or_province_name
	x509_name_add_locality_name
	x509_name_add_organization_name
	x509_name_add_organizational_unit_name
	x509_name_add_common_name
	x509_name_add_domain_component
	x509_name_to_der
	x509_name_from_der
	x509_name_print
	x509_name_get_value_by_type
	x509_name_get_common_name

	x509_cert_sign
	x509_cert_verify
	x509_cert_verify_by_ca_cert
	x509_cert_get_issuer_and_serial_number
	x509_cert_get_issuer
	x509_cert_get_subject
	x509_cert_get_subject_public_key
	x509_cert_to_der
	x509_cert_from_der
	x509_cert_to_pem
	x509_cert_from_pem
	x509_cert_print
*/

enum X509_Version {
	X509_version_v1 = 0,
	X509_version_v2 = 1,
	X509_version_v3 = 2,
};

const char *x509_version_name(int version);
int x509_explicit_version_to_der(int index, int version, uint8_t **out, size_t *outlen);
int x509_explicit_version_from_der(int index, int *version, const uint8_t **in, size_t *inlen);

/*
Time ::= CHOICE {
	utcTime		UTCTime,
	generalTime	GeneralizedTime }
*/
int x509_time_to_der(time_t a, uint8_t **out, size_t *outlen);
int x509_time_from_der(time_t *a, const uint8_t **in, size_t *inlen);

/*
Validity ::= SEQUENCE {
	notBefore	Time,
	notAfter	Time }
*/
#define X509_VALIDITY_MIN_DAYS 1
#define X509_VALIDITY_MAX_DAYS (365 * 10) // ROOTCA, CA需要更长的时间！
int x509_validity_add_days(time_t *not_after, time_t not_before, int days);
int x509_validity_to_der(time_t not_before, time_t not_after, uint8_t **out, size_t *outlen);
int x509_validity_from_der(time_t *not_before, time_t *not_after, const uint8_t **in, size_t *inlen);
int x509_validity_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
AttributeTypeAndValue ::= SEQUENCE {
	type OBJECT IDENTIFIER,
	value ANY -- DEFINED BY AttributeType }

id-at
	name			DirectoryName		1..ub-name
	surname			DirectoryName		1..ub-name
	givenName		DirectoryName		1..ub-name
	initials		DirectoryName		1..ub-name
	generationQualifier	DirectoryName		1..ub-name
	commonName		DirectoryName		1..ub-common-name
	localityName		DirectoryName		1..ub-locality-name
	stateOrProvinceName	DirectoryName		1..ub-state-name
	organizationName	DirectoryName		1..ub-organization-name
	organizationalUnitName	DirectoryName		1..ub-organizational-unit-name
	title			DirectoryName		1..ub-title
	dnQualifier		PrintableString		N/A
	countryName		PrintableString		2..2
	serialNumber		PrintableString		1..ub-serial-number
	pseudonym		DirectoryName		1..ub-pseudonym
	domainComponent		IA5String		N/A
*/
#define X509_ub_name 32768
#define X509_ub_common_name 64
#define X509_ub_locality_name 128
#define X509_ub_state_name 128
#define X509_ub_organization_name 64
#define X509_ub_organizational_unit_name 64
#define X509_ub_title 64
#define X509_ub_serial_number 64
#define X509_ub_pseudonym 128

int x509_attr_type_and_value_check(int oid, int tag, const uint8_t *val, size_t vlen);
int x509_attr_type_and_value_to_der(int oid, int tag, const uint8_t *val, size_t vlen, uint8_t **out, size_t *outlen);
int x509_attr_type_and_value_from_der(int *oid, int *tag, const uint8_t **val, size_t *vlen, const uint8_t **in, size_t *inlen);
int x509_attr_type_and_value_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
*/
int x509_rdn_to_der(int oid, int tag, const uint8_t *val, size_t vlen, const uint8_t *more, size_t mlen, uint8_t **out, size_t *outlen);
int x509_rdn_from_der(int *oid, int *tag, const uint8_t **val, size_t *vlen, const uint8_t **more, size_t *mlen, const uint8_t **in, size_t *inlen);
int x509_rdn_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
Name ::= SEQUENCE OF RelativeDistinguishedName

Example:
    SEQUENCE LEN
      SET LEN
          SEQUENCE LEN OID=countryName, String=CN
      SET LEN
          SEQUENCE LEN OID=stateName, String=CN
          SEQUENCE LEN OID=unknown, String=ABC
      SET LEN
          SEQUENCE LEN OID=commonNmame, String=ABC
*/
int x509_name_add_rdn(uint8_t *d, size_t *dlen, size_t maxlen, int oid, int tag, const uint8_t *val, size_t vlen, const uint8_t *more, size_t mlen);
int x509_name_add_country_name(uint8_t *d, size_t *dlen, int maxlen, const char val[2] ); // val: PrintableString SIZE(2)
int x509_name_add_state_or_province_name(uint8_t *d, size_t *dlen, int maxlen, int tag, const uint8_t *val, size_t vlen);
int x509_name_add_locality_name(uint8_t *d, size_t *dlen, int maxlen, int tag, const uint8_t *val, size_t vlen);
int x509_name_add_organization_name(uint8_t *d, size_t *dlen, int maxlen, int tag, const uint8_t *val, size_t vlen);
int x509_name_add_organizational_unit_name(uint8_t *d, size_t *dlen, int maxlen, int tag, const uint8_t *val, size_t vlen);
int x509_name_add_common_name(uint8_t *d, size_t *dlen, int maxlen, int tag, const uint8_t *val, size_t vlen);
int x509_name_add_domain_component(uint8_t *d, size_t *dlen, int maxlen, const char *val, size_t vlen); // val: IA5String

int x509_name_set(uint8_t *d, size_t *dlen, size_t maxlen,
	const char *country, const char *state, const char *locality,
	const char *org, const char *org_unit, const char *common_name);

#define x509_name_to_der(d,dlen,out,outlen) asn1_sequence_to_der(d,dlen,out,outlen)
#define x509_name_from_der(d,dlen,in,inlen) asn1_sequence_from_der(d,dlen,in,inlen)
int x509_name_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);
int x509_name_get_value_by_type(const uint8_t *d, size_t dlen, int oid, int *tag, const uint8_t **val, size_t *vlen);
int x509_name_get_common_name(const uint8_t *d, size_t dlen, int *tag, const uint8_t **val, size_t *vlen);
int x509_name_equ(const uint8_t *a, size_t alen, const uint8_t *b, size_t blen);

int x509_names_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
SubjectPublicKeyInfo  ::=  SEQUENCE  {
	algorithm            AlgorithmIdentifier,
	subjectPublicKey     BIT STRING  }

algorithm.algorithm = OID_ec_public_key;
algorithm.parameters = OID_sm2;
subjectPublicKey = ECPoint
*/
#define x509_public_key_info_to_der(key,out,outlen) sm2_public_key_info_to_der(key,out,outlen)
#define x509_public_key_info_from_der(key,in,inlen) sm2_public_key_info_from_der(key,in,inlen)
int x509_public_key_info_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
Extension  ::=  SEQUENCE  {
	extnID OBJECT IDENTIFIER,
	critical BOOLEAN DEFAULT FALSE,
	extnValue OCTET STRING -- contains the DER encoding of an ASN.1 value
*/
int x509_ext_to_der(int oid, int critical, const uint8_t *val, size_t vlen, uint8_t **out, size_t *outlen);
int x509_ext_from_der(int *oid, uint32_t *nodes, size_t *nodes_cnt, int *critical, const uint8_t **val, size_t *vlen, const uint8_t **in, size_t *inlen);
int x509_ext_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
[3] EXPLICIT SEQUENCE OF Extension
 */
int x509_explicit_exts_to_der(int index, const uint8_t *d, size_t dlen, uint8_t **out, size_t *outlen);
int x509_explicit_exts_from_der(int index, const uint8_t **d, size_t *dlen, const uint8_t **in, size_t *inlen);
#define x509_exts_to_der(d,dlen,out,outlen) x509_explicit_exts_to_der(3,d,dlen,out,outlen)
#define x509_exts_from_der(d,dlen,in,inlen) x509_explicit_exts_from_der(3,d,dlen,in,inlen)

int x509_exts_get_count(const uint8_t *d, size_t dlen, size_t *cnt);
int x509_exts_get_ext_by_index(const uint8_t *d, size_t dlen, int index,
	int *oid, uint32_t *nodes, size_t *nodes_cnt, int *critical,
	const uint8_t **val, size_t *vlen);
int x509_exts_get_ext_by_oid(const uint8_t *d, size_t dlen, int oid,
	int *critical, const uint8_t **val, size_t *vlen);
int x509_exts_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
TBSCertificate ::= SEQUENCE {
	version			[0] EXPLICIT INTEGER DEFAULT v1,
	serialNumber		INTEGER,
	siganture		AlgorithmIdentifier,
	issuer			Name,
	validity		Validity,
	subject			Name,
	subjectPulbicKeyInfo	SubjectPublicKeyInfo,
	issuerUniqueID		[1] IMPLICIT BIT STRING OPTIONAL, -- If present, must be v2,v3
	subjectUniqueID		[2] IMPLICIT BIT STRING OPTIONAL, -- If present, must be v2,v3
	extensions		[3] EXPLICIT Extensions OPTIONAL  -- If present, must be v3 }
*/
#define X509_SERIAL_NUMBER_MIN_LEN	1
#define X509_SERIAL_NUMBER_MAX_LEN	20
#define X509_UNIQUE_ID_MIN_LEN 		32
#define X509_UNIQUE_ID_MAX_LEN 		32

int x509_tbs_cert_to_der(
	int version,
	const uint8_t *serial, size_t serial_len,
	int signature_algor,
	const uint8_t *issuer, size_t issuer_len,
	time_t not_before, time_t not_after,
	const uint8_t *subject, size_t subject_len,
	const SM2_KEY *subject_public_key,
	const uint8_t *issuer_unique_id, size_t issuer_unique_id_len,
	const uint8_t *subject_unique_id, size_t subject_unique_id_len,
	const uint8_t *exts, size_t exts_len,
	uint8_t **out, size_t *outlen);
int x509_tbs_cert_from_der(
	int *version,
	const uint8_t **serial, size_t *serial_len,
	int *signature_algor,
	const uint8_t **issuer, size_t *issuer_len,
	time_t *not_before, time_t *not_after,
	const uint8_t **subject, size_t *subject_len,
	SM2_KEY *subject_public_key,
	const uint8_t **issuer_unique_id, size_t *issuer_unique_id_len,
	const uint8_t **subject_unique_id, size_t *subject_unique_id_len,
	const uint8_t **exts, size_t *exts_len,
	const uint8_t **in, size_t *inlen);
int x509_tbs_cert_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

/*
Certificate  ::=  SEQUENCE  {
	tbsCertificate       TBSCertificate,
	signatureAlgorithm   AlgorithmIdentifier,
	signatureValue       BIT STRING }
*/
int x509_certificate_to_der(
	const uint8_t *tbs, size_t tbslen,
	int signature_algor,
	const uint8_t *sig, size_t siglen,
	uint8_t **out, size_t *outlen);
int x509_certificate_from_der(
	const uint8_t **tbs, size_t *tbslen,
	int *signature_algor,
	const uint8_t **sig, size_t *siglen,
	const uint8_t **in, size_t *inlen);
int x509_certificate_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);

// x509_cert functions
int x509_cert_sign(
	uint8_t *cert, size_t *certlen, size_t maxlen,
	int version,
	const uint8_t *serial, size_t serial_len,
	int signature_algor,
	const uint8_t *issuer, size_t issuer_len,
	time_t not_before, time_t not_after,
	const uint8_t *subject, size_t subject_len,
	const SM2_KEY *subject_public_key,
	const uint8_t *issuer_unique_id, size_t issuer_unique_id_len,
	const uint8_t *subject_unique_id, size_t subject_unique_id_len,
	const uint8_t *exts, size_t exts_len,
	const SM2_KEY *sign_key,
	const char *signer_id, size_t signer_id_len);
int x509_cert_verify(const uint8_t *a, size_t alen, const SM2_KEY *pub_key,
	const char *signer_id, size_t signer_id_len);
int x509_cert_verify_by_ca_cert(const uint8_t *a, size_t alen, const uint8_t *cacert, size_t cacertlen,
	const char *signer_id, size_t signer_id_len);

int x509_cert_to_der(const uint8_t *a, size_t alen, uint8_t **out, size_t *outlen);
int x509_cert_from_der(const uint8_t **a, size_t *alen, const uint8_t **in, size_t *inlen);
int x509_cert_to_pem(const uint8_t *a, size_t alen, FILE *fp);
int x509_cert_from_pem(uint8_t *a, size_t *alen, size_t maxlen, FILE *fp);
int x509_cert_from_pem_buf(uint8_t *d, size_t *dlen, size_t maxlen, uint8_t *buf, size_t buflen);
int x509_cert_from_pem_by_index(uint8_t *a, size_t *alen, size_t maxlen, int index, FILE *fp);
int x509_cert_from_pem_by_subject(uint8_t *a, size_t *alen, size_t maxlen, const uint8_t *name, size_t namelen, FILE *fp);
int x509_cert_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *a, size_t alen);

int x509_cert_get_details(const uint8_t *a, size_t alen,
	int *version,
	const uint8_t **serial_number, size_t *serial_number_len,
	int *inner_signature_algor,
	const uint8_t **issuer, size_t *issuer_len,
	time_t *not_before, time_t *not_after,
	const uint8_t **subject, size_t *subject_len,
	SM2_KEY *subject_public_key,
	const uint8_t **issuer_unique_id, size_t *issuer_unique_id_len,
	const uint8_t **subject_unique_id, size_t *subject_unique_id_len,
	const uint8_t **extensions, size_t *extensions_len,
	int *signature_algor,
	const uint8_t **signature, size_t *signature_len);

/*
IssuerAndSerialNumber ::= SEQUENCE {
	isser		Name,
	serialNumber	INTEGER }
*/
int x509_cert_get_issuer_and_serial_number(const uint8_t *a, size_t alen,
	const uint8_t **issuer, size_t *issuer_len,
	const uint8_t **serial_number, size_t *serial_number_len);
int x509_cert_get_issuer(const uint8_t *a, size_t alen, const uint8_t **name, size_t *namelen);
int x509_cert_get_subject(const uint8_t *a, size_t alen, const uint8_t **subj, size_t *subj_len);
int x509_cert_get_subject_public_key(const uint8_t *a, size_t alen, SM2_KEY *public_key);

int x509_certs_to_pem(const uint8_t *d, size_t dlen, FILE *fp);
int x509_certs_from_pem(uint8_t *d, size_t *dlen, size_t maxlen, FILE *fp);
int x509_certs_get_count(const uint8_t *d, size_t dlen, size_t *cnt);
int x509_certs_get_cert_by_index(const uint8_t *d, size_t dlen, int index, const uint8_t **cert, size_t *certlen);
int x509_certs_get_cert_by_subject(const uint8_t *d, size_t dlen, const uint8_t *subject, size_t subject_len, const uint8_t **cert, size_t *certlen);
int x509_certs_get_last(const uint8_t *d, size_t dlen, const uint8_t **cert, size_t *certlen);

int x509_certs_get_cert_by_issuer_and_serial_number(
	const uint8_t *certs, size_t certs_len,
	const uint8_t *issuer, size_t issuer_len,
	const uint8_t *serial, size_t serial_len,
	const uint8_t **cert, size_t *cert_len);


typedef enum {
	X509_verify_err_cert_revoked		= -2,
	X509_verify_err_cert_not_yet_valid	= -3,
	X509_verify_err_cert_has_expired	= -4,
	X509_verify_err_cert_chain_too_long	= -5,
} X509_VERIFY_ERR;

int x509_certs_verify(const uint8_t *certs, size_t certslen,
	const uint8_t *rootcerts, size_t rootcertslen, int depth, int *verify_result);
int x509_certs_verify_tlcp(const uint8_t *certs, size_t certslen,
	const uint8_t *rootcerts, size_t rootcertslen, int depth, int *verify_result);
int x509_certs_get_subjects(const uint8_t *certs, size_t certslen, uint8_t *names, size_t *nameslen);
int x509_certs_print(FILE *fp, int fmt, int ind, const char *label, const uint8_t *d, size_t dlen);


int x509_cert_new_from_file(uint8_t **out, size_t *outlen, const char *file);
int x509_certs_new_from_file(uint8_t **out, size_t *outlen, const char *file);




#ifdef __cplusplus
}
#endif
#endif
