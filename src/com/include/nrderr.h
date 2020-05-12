/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Programmer(s) :  Volodymyr Bykov
 *
 *********************************************************************
 * Description :  Error codes. 
 *
 *********************************************************************
 *
 * Copyright 2006-2020, Volodymyr Bykov. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************
 * 
 * As of May 1 2020, a revocable permission to distribute binary copies 
 * of this software without source code and/or copyright notice has been
 * granted to Ozolio Inc (a Hawaii Corporation) by the author. 
 *
 ********************************************************************/

#ifndef  __NRD_ERROR_CODES_H__
#define  __NRD_ERROR_CODES_H__

#define NRDRC_BROKEN            11 /* Action was broken by user or system*/
#define NRDRC_EMPTY             10 /* Value is empty*/
#define NRDRC_AGAIN             9  /* Try again*/
#define NRDRC_PARTIAL           8  /* Partial result returned*/
#define NRDRC_WAITFAILED        7  /* Waiting was broken because of an error*/
#define NRDRC_DUPLICATE         6  /* Duplicate resource*/
#define NRDRC_NOTVALID          5  /* Invalid resource*/
#define NRDRC_NEEDRESP          4  /* Response is required*/
#define NRDRC_NOTFOUND          3  /* Resource was not found*/
#define NRDRC_EXISTS            2  /* Resource already exists*/
#define NRDRC_FALSE             1  /* Reply is negative*/
#define NRDRC_OK                0  /* No error*/
#define NRDRC_FAIL             -1  /* Critical error*/
#define NRDRC_NOTIMPL          -2  /* Method is not implemented*/
#define NRDRC_NOTAPPB          -3  /* Method is not applicable*/
#define NRDRC_NOTSUPP          -5  /* Method is not supported*/
#define NRDRC_ESIZE            -6  /* Invalid size*/
#define NRDRC_EACCESS          -7  /* Access denied*/
#define NRDRC_EARG             -8  /* Invalid or unsupported argument*/
#define NRDRC_EOPER            -9  /* Invalid or unsupported operation*/
#define NRDRC_EEVENT           -10 /* Invalid or unsupported event */
#define NRDRC_EPARAM           -11 /* Invalid or unsupported parameter*/
#define NRDRC_EACTION          -12 /* Invalid or unsupported action*/
#define NRDRC_EVAR             -13 /* Invalid variable*/
#define NRDRC_ECID             -14 /* Invalid Class ID */
#define NRDRC_EHANDLE          -15 /* Invalid handle */
#define NRDRC_EBUNDLE          -16 /* Bundle error */
#define NRDRC_EDESK            -17 /* Desktop error */
#define NRDRC_EAPP             -18 /* Application error */
#define NRDRC_EWND             -19 /* Window error*/
#define NRDRC_ECHANNEL         -20 /* Channel error */
#define NRDRC_ELIST            -21 /* List is not available*/
#define NRDRC_ESYS             -22 /* System error*/
#define NRDRC_EMENU            -23 /* Menu error*/
#define NRDRC_EDISPLAY         -24 /* Display error*/
#define NRDRC_ETITLE           -25 /* Invalid title*/
#define NRDRC_EBOUNDS          -26 /* Invalid bounds*/
#define NRDRC_EPID             -27 /* Invalid process PID*/
#define NRDRC_EVKEY            -28 /* Invalid virtual key code*/
#define NRDRC_EUSER            -29 /* Invalid username*/
#define NRDRC_EPASS            -30 /* Invalid password*/
#define NRDRC_EMSG             -31 /* Invalid message received*/
#define NRDRC_ESTATE           -32 /* Invalid state*/
#define NRDRC_EPATH            -33 /* Invalid path*/
#define NRDRC_ESOCK            -34 /* Invalid socket*/
#define NRDRC_ECONN            -35 /* Connection failed or broken*/
#define NRDRC_ESEND            -36 /* Data sending error*/
#define NRDRC_ERECV            -37 /* Data receiving error*/
#define NRDRC_EPROTO           -38 /* Unsupported protocol version*/
#define NRDRC_ECMD             -39 /* Invalid command*/
#define NRDRC_EAUTH            -40 /* Authentication failed*/
#define NRDRC_ETOUT            -41 /* Operation timeout*/
#define NRDRC_ERONLY           -42 /* Resource is read only*/
#define NRDRC_ECTX             -43 /* Invalid context*/
#define NRDRC_ETYPE            -44 /* Invalid resource type*/
#define NRDRC_EENCR            -45 /* Encription failed*/
#define NRDRC_ESOUND           -46 /* Common sound engine error*/
#define NRDRC_EPRINT           -47 /* Common printing engine error*/

#endif

