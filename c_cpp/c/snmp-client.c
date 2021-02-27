// http://www.net-snmp.org/tutorial/tutorial-5/toolkit/demoapp/snmpdemoapp.c
// http://www.net-snmp.org/tutorial/tutorial-5/toolkit/asyncapp/asyncapp.c


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <stdio.h>
#include <unistd.h>

#undef MAX_OID_LEN
#define MAX_OID_LEN 24

#define USER_DATA_SIZE_MAX 64

typedef union {
	int int32;
	float floatVal;
	double doubleVal;
	uint32_t uint32;
	uint64_t uint64;
	char string[USER_DATA_SIZE_MAX];
	uint8_t raw[USER_DATA_SIZE_MAX];
} UserData_t;

typedef enum {
	ort_nDef,
	ort_Get,
	ort_Walk
} OidReqType_t;

typedef struct {
	OidReqType_t type;
	const char *s; // ".1.3.6.1.2.1.2.2.1.16.10279"
	struct {
		UserData_t *self;
		UserData_t *storage;
		size_t size;
		size_t it;
	} uData;
	oid start[MAX_OID_LEN]; // for one string request
	oid end[MAX_OID_LEN]; // for one string request
	size_t len;
} OidReq_t;

typedef struct {
	const char *addr;
	struct snmp_session session, *ss;
	struct {
		OidReq_t *current;
		OidReq_t *all;
		int size;
	} reqs;
} Session_t;

typedef struct {
	struct {
		Session_t *self;
		int size;
	} sessions;
} drv_t;


static int oidreq_init(OidReq_t *req, OidReqType_t type, const char *s, UserData_t *udata, size_t dSize)
{
	int rc = 0;
	if (req && s) {
		req->type = type;
		req->s = s;
		req->len = MAX_OID_LEN;
		req->uData.self = udata;
		req->uData.size = dSize;
		req->uData.it = 0;
		// req->uData.storage = (type == ort_Walk)?
		// 		(UserData_t*)calloc(dSize, sizeof(UserData_t)) : 0;
		rc = read_objid(req->s, req->start, &req->len);
		if (rc == 1) {
			memcpy(req->end, req->start, MAX_OID_LEN);
			req->end[MAX_OID_LEN-1]++;
		}
	}
	return (rc == 1)? 0 : -1;
}

static inline void oidreq_get_init(OidReq_t *req, const char *s, UserData_t *udata)
{ return oidreq_init(req, ort_Get, s, udata, 1); }

static inline void oidreq_walk_init(OidReq_t *req, const char *s, UserData_t *udata, size_t dSize)
{ return oidreq_init(req, ort_Walk, s, udata, dSize); }


// static Server_t *create_server(int reqs)
// {
// 	if (reqs > 0) {
// 		Server_t *ret = (Server_t *)calloc(1, sizeof(Server_t));
// 		ret->reqs.size = reqs;
// 		ret->reqs.all = (OidReq_t *)calloc(reqs, sizeof(OidReq_t));
// 		ret->reqs.current = ret->reqs.all;
// 		return ret;
// 	}
// 	return 0;
// }

// static void free_server(Server_t *serv)
// {
// 	free(serv->reqs.all);
// 	free(serv);
// }


// static drv_t *create_drv()
// {
// 	drv_t *ret = (drv_t *)calloc(1, sizeof(drv_t));
// 	init_snmp("snmpclient");
// 	snmp_sess_init( &ret->session );
// 	return ret;
// }


// static void free_drv(drv_t *drv)
// {
// 	snmp_close(drv->ss);

// }


static int recv_callback(int op, netsnmp_session *ss, int reqid, netsnmp_pdu *response, void *priv)
{
	OidReq_t *req = (OidReq_t *)priv;
	struct variable_list *var;
	if (op == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
		printf("callback\n");

		// reqid

// var->type
// ASN_BOOLEAN
// ASN_INTEGER
// ASN_BIT_STR
// ASN_OCTET_STR
// ASN_NULL
// ASN_OBJECT_ID
// ASN_SEQUENCE
// ASN_SET

// var->val
// typedef union {
//    long           *integer;
//    u_char         *string; // withou \0
//    oid            *objid;
//    u_char         *bitstring;
//    struct counter64 *counter64;
//    float          *floatVal;
//    double         *doubleVal;
// } netsnmp_vardata;

		for (var = response->variables; var; var = var->next_variable) {
			print_variable(var->name, var->name_length, var);
		}
	} else {
		printf("callback to\n");
	}
	snmp_free_pdu(response);
	return 1;
}

int main()
{
	drv_t _drv;
	drv_t *drv = &_drv;

	init_snmp("snmpclient");
	snmp_sess_init( &drv->session );


	struct snmp_session session, *ss;
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;

	struct variable_list *vars;
	int status;

	OidReq_t oidReq[1];
	oidreq_get_init(&oidReq[0], ".1.3.6.1.2.1.2.2.1.16.10279", 0);

	init_snmp("snmpclient");

	snmp_sess_init( &session );
	session.version = SNMP_VERSION_2c;
	session.retries = 60;
	session.timeout = 1000*1000; // us
	session.peername = "10.24.1.56";
	session.localname = "10.24.1.1";
	session.local_port = 0;
    session.community = "secret";
    session.community_len = strlen(session.community);

	// session.callback_magic = &drv->sessions[n];

	ss = snmp_open(&session);

	int fds = 0, block = 1;
    fd_set fdset;
    struct timeval timeout;
    FD_ZERO(&fdset);
    snmp_select_info(&fds, &fdset, &timeout, &block);

	if (ss) {
		// get example
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		if (pdu) {
			printf("snmp_pdu_create ok\n");
			snmp_add_null_var(pdu, oidReq[0].start, oidReq[0].len);
			for (;;) {
				// snmp_async_send(ss, pdu, recv_callback, &(oidReq[0]));
				status = snmp_synch_response(ss, pdu, &response);
				if (!response) {
					printf("!response\n");
					snmp_sess_perror("snmpget", ss);
					continue;
				}
				if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
					for (vars = response->variables; vars; vars = vars->next_variable) {
						print_variable(vars->name, vars->name_length, vars);
					}
				} else {
					printf("Error in packet\nReason: %s\n", snmp_errstring(response->errstat));
				}
				snmp_free_pdu(response);
				usleep(1000*1000*1);
			}
		}
	}

	return 0;
}