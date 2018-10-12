/*
 * SNMPStats Module 
 * Copyright (C) 2006 SOMA Networks, INC.
 * Written by: Jeffrey Magder (jmagder@somanetworks.com)
 *
 * Kamailio Server core objects addition
 * Copyright (C) 2013 Edvina AB, Sollentuna, Sweden
 * Written by Olle E. Johansson
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 *
 * Note: this file originally auto-generated by mib2c 
 *
 */
#ifndef KAMAILIOSERVER_H
#define KAMAILIOSERVER_H

/* function declarations */
void init_kamailioServer(void);
Netsnmp_Node_Handler handle_kamailioSrvMaxMemory;
Netsnmp_Node_Handler handle_kamailioSrvFreeMemory;
Netsnmp_Node_Handler handle_kamailioSrvMaxUsed;
Netsnmp_Node_Handler handle_kamailioSrvRealUsed;
Netsnmp_Node_Handler handle_kamailioSrvMemFragments;

Netsnmp_Node_Handler handle_kamailioSrvCnfFullVersion;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerName;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerVersion;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerArch;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerOs;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerId;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerCompTime;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerCompiler;
Netsnmp_Node_Handler handle_kamailioSrvCnfVerFlags;

#endif /* KAMAILIOSERVER_H */
