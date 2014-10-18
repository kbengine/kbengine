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

#ifndef SIGAR_FORMAT_H
#define SIGAR_FORMAT_H

#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)

typedef struct {
    double user;
    double sys;
    double nice;
    double idle;
    double wait;
    double irq;
    double soft_irq;
    double stolen;
    double combined;
} sigar_cpu_perc_t;

SIGAR_DECLARE(int) sigar_cpu_perc_calculate(sigar_cpu_t *prev,
                                            sigar_cpu_t *curr,
                                            sigar_cpu_perc_t *perc);

SIGAR_DECLARE(int) sigar_uptime_string(sigar_t *sigar, 
                                       sigar_uptime_t *uptime,
                                       char *buffer,
                                       int buflen);

SIGAR_DECLARE(char *) sigar_format_size(sigar_uint64_t size, char *buf);

SIGAR_DECLARE(int) sigar_net_address_equals(sigar_net_address_t *addr1,
                                            sigar_net_address_t *addr2);

SIGAR_DECLARE(int) sigar_net_address_to_string(sigar_t *sigar,
                                               sigar_net_address_t *address,
                                               char *addr_str);

SIGAR_DECLARE(sigar_uint32_t) sigar_net_address_hash(sigar_net_address_t *address);


SIGAR_DECLARE(const char *)sigar_net_connection_type_get(int type);

SIGAR_DECLARE(const char *)sigar_net_connection_state_get(int state);

SIGAR_DECLARE(char *) sigar_net_interface_flags_to_string(sigar_uint64_t flags, char *buf);

SIGAR_DECLARE(char *)sigar_net_services_name_get(sigar_t *sigar,
                                                 int protocol, unsigned long port);

#endif

