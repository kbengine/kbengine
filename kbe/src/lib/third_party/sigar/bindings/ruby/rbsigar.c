/*
 * Copyright (c) 2007-2008 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ruby.h>
#include <errno.h>
#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"

#define RB_SIGAR_CROAK rb_raise(rb_eArgError, "%s", sigar_strerror(sigar, status))
#define NUM2PID NUM2UINT

static sigar_t *rb_sigar_get(VALUE obj)
{
    sigar_t *sigar;
    Data_Get_Struct(obj, sigar_t, sigar);
    return sigar;
}

static void rb_sigar_free(void *obj)
{
    free(obj);
}

static void rb_sigar_close(void *obj)
{
    sigar_close((sigar_t *)obj);
}

static VALUE rb_sigar_new(VALUE module)
{
    sigar_t *sigar;
    sigar_open(&sigar);
    return Data_Wrap_Struct(module, 0, rb_sigar_close, sigar);
}

static VALUE rb_sigar_format_size(VALUE rclass, VALUE size)
{
    char buffer[56];
    return rb_str_new2(sigar_format_size(NUM2LL(size), buffer));
}

static VALUE rb_sigar_net_interface_flags_to_s(VALUE rclass, VALUE flags)
{
    char buffer[1024];
    return rb_str_new2(sigar_net_interface_flags_to_string(NUM2LL(flags), buffer));
}

static VALUE rb_sigar_net_connection_type_to_s(VALUE rclass, VALUE type)
{
    return rb_str_new2(sigar_net_connection_type_get(NUM2INT(type)));
}

static VALUE rb_sigar_net_connection_state_to_s(VALUE rclass, VALUE state)
{
    return rb_str_new2(sigar_net_connection_state_get(NUM2INT(state)));
}

static VALUE rb_sigar_net_address_to_string(sigar_net_address_t *address)
{
    char addr_str[SIGAR_INET6_ADDRSTRLEN];
    sigar_net_address_to_string(NULL, address, addr_str);
    return rb_str_new2(addr_str);
}

#define rb_sigar_net_address_to_s(a) rb_sigar_net_address_to_string(&a) 

static VALUE rb_sigar_new_list(char *data, unsigned long number,
                               int size, VALUE rclass)
{
    unsigned long i;
    VALUE av = rb_ary_new2(number);

    for (i=0; i<number; i++, data += size) {
        void *ent = malloc(size);

        memcpy(ent, data, size);
        rb_ary_push(av, Data_Wrap_Struct(rclass, 0, free, ent));
    }

    return av;
}

static VALUE rb_sigar_new_strlist(char **data, unsigned long number)
{
    unsigned long i;
    VALUE av = rb_ary_new2(number);

    for (i=0; i<number; i++) {
        rb_ary_push(av, rb_str_new2(data[i]));
    }

    return av;
}

static VALUE rb_sigar_new_intlist(int *data, int number)
{
    int i;
    VALUE av = rb_ary_new2(number);

    for (i=0; i<number; i++) {
        rb_ary_push(av, rb_int2inum(data[i]));
    }

    return av;
}

static VALUE rb_sigar_net_interface_list(VALUE obj)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_net_interface_list_t iflist;
    VALUE RETVAL;

    status = sigar_net_interface_list_get(sigar, &iflist);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_strlist(iflist.data, iflist.number);

    sigar_net_interface_list_destroy(sigar, &iflist);

    return RETVAL;
}

static int rb_sigar_str2net_address(VALUE bytes, sigar_net_address_t *address)
{
    long len = RSTRING(bytes)->len;

    switch (len) {
      case 4:
        address->family = SIGAR_AF_INET;
        break;
      case 4*4:
        address->family = SIGAR_AF_INET6;
        break;
      default:
        return EINVAL;
    }

    memcpy(RSTRING(bytes)->ptr, &address->addr.in6, len);

    return SIGAR_OK;
}

static VALUE rb_cSigarNetStat;

static VALUE rb_sigar_net_stat_get(VALUE obj, VALUE flags, VALUE bytes, int port)
{
    int status;
    int has_port = (port != -1);
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_net_stat_t *RETVAL = malloc(sizeof(*RETVAL));
    sigar_net_address_t address;

    if (has_port) {
        status = rb_sigar_str2net_address(bytes, &address);
        if (status == SIGAR_OK) {
            status = sigar_net_stat_port_get(sigar, RETVAL, NUM2INT(flags),
                                             &address, port);
        }
    }
    else {
        status = sigar_net_stat_get(sigar, RETVAL, NUM2INT(flags));
    }

    if (status != SIGAR_OK) {
        free(RETVAL);
        RB_SIGAR_CROAK;
    }

    return Data_Wrap_Struct(rb_cSigarNetStat, 0, rb_sigar_free, RETVAL);
}

static VALUE rb_sigar_net_stat(VALUE obj, VALUE flags)
{
    return rb_sigar_net_stat_get(obj, flags, Qnil, -1);
}

static VALUE rb_sigar_net_stat_port(VALUE obj, VALUE flags, VALUE address, VALUE port)
{
    return rb_sigar_net_stat_get(obj, flags, address, NUM2INT(port));
}

static VALUE rb_sigar_NetStat_tcp_states(VALUE self)
{
    sigar_net_stat_t *net_stat;

    Data_Get_Struct(self, sigar_net_stat_t, net_stat);

    return rb_sigar_new_intlist(&net_stat->tcp_states[0], SIGAR_TCP_UNKNOWN);
}

static VALUE rb_cSigarNetConnection;

static VALUE rb_sigar_net_connection_list(VALUE obj, VALUE flags)
{
    int status;
    unsigned int i;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_net_connection_list_t connlist;
    VALUE RETVAL;

    status = sigar_net_connection_list_get(sigar, &connlist, NUM2UINT(flags));

    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_list((char *)&connlist.data[0],
                               connlist.number,
                               sizeof(*connlist.data),
                               rb_cSigarNetConnection);

    sigar_net_connection_list_destroy(sigar, &connlist);

    return RETVAL;
}

static VALUE rb_sigar_net_services_name(VALUE obj, VALUE protocol, VALUE port)
{
    sigar_t *sigar = rb_sigar_get(obj);
    char *name;

    if ((name = sigar_net_services_name_get(sigar, NUM2UINT(protocol), NUM2UINT(port)))) {
        return rb_str_new2(name);
    }
    else {
        return Qnil;
    }
}

static VALUE rb_cSigarCpuInfo;

static VALUE rb_sigar_cpu_info_list(VALUE obj)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_cpu_info_list_t cpu_infos;
    VALUE RETVAL;

    status = sigar_cpu_info_list_get(sigar, &cpu_infos);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_list((char *)&cpu_infos.data[0],
                               cpu_infos.number,
                               sizeof(*cpu_infos.data),
                               rb_cSigarCpuInfo);

    sigar_cpu_info_list_destroy(sigar, &cpu_infos);

    return RETVAL;
}

static VALUE rb_cSigarCpuPerc;

static VALUE rb_cSigarFileSystem;

static VALUE rb_sigar_file_system_list(VALUE obj)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_file_system_list_t fslist;
    VALUE RETVAL;

    status = sigar_file_system_list_get(sigar, &fslist);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_list((char *)&fslist.data[0],
                               fslist.number,
                               sizeof(*fslist.data),
                               rb_cSigarFileSystem);

    sigar_file_system_list_destroy(sigar, &fslist);

    return RETVAL;
}

static VALUE rb_cSigarWho;

static VALUE rb_sigar_who_list(VALUE obj)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_who_list_t list;
    VALUE RETVAL;

    status = sigar_who_list_get(sigar, &list);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_list((char *)&list.data[0],
                               list.number,
                               sizeof(*list.data),
                               rb_cSigarWho);

    sigar_who_list_destroy(sigar, &list);

    return RETVAL;
}

static VALUE rb_cSigarNetRoute;

static VALUE rb_sigar_net_route_list(VALUE obj)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_net_route_list_t list;
    VALUE RETVAL;

    status = sigar_net_route_list_get(sigar, &list);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_list((char *)&list.data[0],
                               list.number,
                               sizeof(*list.data),
                               rb_cSigarNetRoute);

    sigar_net_route_list_destroy(sigar, &list);

    return RETVAL;
}

static VALUE rb_sigar_proc_args(VALUE obj, VALUE pid)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_proc_args_t args;
    VALUE RETVAL;

    status = sigar_proc_args_get(sigar, NUM2PID(pid), &args);

    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    RETVAL = rb_sigar_new_strlist(args.data, args.number);

    sigar_proc_args_destroy(sigar, &args);

    return RETVAL;
}

static int rb_sigar_env_getall(void *data,
                               const char *key, int klen,
                               char *val, int vlen)
{
    rb_hash_aset(*((VALUE*)data),
                 rb_str_new(key, klen),
                 rb_str_new(val, vlen));
    return SIGAR_OK;
}

static VALUE rb_sigar_proc_env(VALUE obj, VALUE pid)
{
    int status;
    sigar_t *sigar = rb_sigar_get(obj);
    sigar_proc_env_t procenv;
    VALUE RETVAL = rb_hash_new();

    procenv.type = SIGAR_PROC_ENV_ALL;
    procenv.env_getter = rb_sigar_env_getall;
    procenv.data = &RETVAL;

    status = sigar_proc_env_get(sigar, NUM2PID(pid), &procenv);
    if (status != SIGAR_OK) {
        RB_SIGAR_CROAK;
    }

    return RETVAL;
}

#include "./rbsigar_generated.rx"

#define RB_SIGAR_CONST_INT(name) \
    rb_define_const(rclass, #name, INT2FIX(SIGAR_##name))

#define RB_SIGAR_CONST_STR(name) \
    rb_define_const(rclass, #name, rb_obj_freeze(rb_str_new2(SIGAR_##name)))

static void Init_rbsigar_constants(VALUE rclass)
{
    RB_SIGAR_CONST_INT(IFF_UP);
    RB_SIGAR_CONST_INT(IFF_BROADCAST);
    RB_SIGAR_CONST_INT(IFF_DEBUG);
    RB_SIGAR_CONST_INT(IFF_LOOPBACK);
    RB_SIGAR_CONST_INT(IFF_POINTOPOINT);
    RB_SIGAR_CONST_INT(IFF_NOTRAILERS);
    RB_SIGAR_CONST_INT(IFF_RUNNING);
    RB_SIGAR_CONST_INT(IFF_NOARP);
    RB_SIGAR_CONST_INT(IFF_PROMISC);
    RB_SIGAR_CONST_INT(IFF_ALLMULTI);
    RB_SIGAR_CONST_INT(IFF_MULTICAST);

    RB_SIGAR_CONST_INT(NETCONN_CLIENT);
    RB_SIGAR_CONST_INT(NETCONN_SERVER);
    RB_SIGAR_CONST_INT(NETCONN_TCP);
    RB_SIGAR_CONST_INT(NETCONN_UDP);
    RB_SIGAR_CONST_INT(NETCONN_RAW);
    RB_SIGAR_CONST_INT(NETCONN_UNIX);

    RB_SIGAR_CONST_INT(TCP_ESTABLISHED);
    RB_SIGAR_CONST_INT(TCP_SYN_SENT);
    RB_SIGAR_CONST_INT(TCP_SYN_RECV);
    RB_SIGAR_CONST_INT(TCP_FIN_WAIT1);
    RB_SIGAR_CONST_INT(TCP_FIN_WAIT2);
    RB_SIGAR_CONST_INT(TCP_TIME_WAIT);
    RB_SIGAR_CONST_INT(TCP_CLOSE);
    RB_SIGAR_CONST_INT(TCP_CLOSE_WAIT);
    RB_SIGAR_CONST_INT(TCP_LAST_ACK);
    RB_SIGAR_CONST_INT(TCP_LISTEN);
    RB_SIGAR_CONST_INT(TCP_CLOSING);
    RB_SIGAR_CONST_INT(TCP_IDLE);
    RB_SIGAR_CONST_INT(TCP_BOUND);
    RB_SIGAR_CONST_INT(TCP_UNKNOWN);

    RB_SIGAR_CONST_STR(NULL_HWADDR);
}

void Init_rbsigar(void)
{
    VALUE rclass = rb_define_class("Sigar", rb_cObject);

    rb_define_method(rclass, "cpu_info_list", rb_sigar_cpu_info_list, 0);
    rb_define_method(rclass, "file_system_list", rb_sigar_file_system_list, 0);
    rb_define_method(rclass, "net_connection_list", rb_sigar_net_connection_list, 1);
    rb_define_method(rclass, "net_interface_list", rb_sigar_net_interface_list, 0);
    rb_define_method(rclass, "net_services_name", rb_sigar_net_services_name, 2);
    rb_define_method(rclass, "net_stat", rb_sigar_net_stat, 1);
    rb_define_method(rclass, "net_stat_port", rb_sigar_net_stat_port, 3);
    rb_define_method(rclass, "net_route_list", rb_sigar_net_route_list, 0);
    rb_define_method(rclass, "who_list", rb_sigar_who_list, 0);
    rb_define_method(rclass, "proc_args", rb_sigar_proc_args, 1);
    rb_define_method(rclass, "proc_env", rb_sigar_proc_env, 1);

    rb_define_singleton_method(rclass, "new", rb_sigar_new, 0);
    rb_define_singleton_method(rclass, "format_size", rb_sigar_format_size, 1);
    rb_define_singleton_method(rclass, "net_interface_flags_to_s",
                               rb_sigar_net_interface_flags_to_s, 1);
    rb_define_singleton_method(rclass, "net_connection_type_to_s",
                               rb_sigar_net_connection_type_to_s, 1);
    rb_define_singleton_method(rclass, "net_connection_state_to_s",
                               rb_sigar_net_connection_state_to_s, 1);

    Init_rbsigar_constants(rclass);

    /* generated */
    rb_sigar_define_module_methods(rclass);
    rb_define_method(rb_cSigarNetStat, "tcp_states", rb_sigar_NetStat_tcp_states, 0);
}
