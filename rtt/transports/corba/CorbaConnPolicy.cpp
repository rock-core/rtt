/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:07 CEST 2009  CorbaConnPolicy.cpp

                        CorbaConnPolicy.cpp -  description
                           -------------------
    begin                : Thu October 22 2009
    copyright            : (C) 2009 Peter Soetens
    email                : peter@thesourcworks.com

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


/*
 * CorbaConnPolicy.cpp
 *
 *  Created on: Oct 16, 2009
 *      Author: kaltan
 */

#include "CorbaConnPolicy.hpp"

RTT::corba::CConnPolicy toCORBA(RTT::ConnPolicy const& policy)
{
    RTT::corba::CConnPolicy corba_policy;
    corba_policy.type        = RTT::corba::CConnectionModel(policy.type);
    corba_policy.init        = policy.init;
    corba_policy.lock_policy = RTT::corba::CLockPolicy(policy.lock_policy);
    corba_policy.pull        = policy.pull;
    corba_policy.signalling  = policy.signalling;
    corba_policy.size        = policy.size;
    corba_policy.data_size   = policy.data_size;
    corba_policy.transport   = policy.transport;
    corba_policy.name_id     = CORBA::string_dup( policy.name_id.c_str() );
    return corba_policy;
}

RTT::ConnPolicy toRTT(RTT::corba::CConnPolicy const& corba_policy)
{
    RTT::ConnPolicy policy;
    policy.type        = corba_policy.type;
    policy.init        = corba_policy.init;
    policy.lock_policy = corba_policy.lock_policy;
    policy.pull        = corba_policy.pull;
    policy.signalling  = corba_policy.signalling;
    policy.size        = corba_policy.size;
    policy.data_size   = corba_policy.data_size;
    policy.transport   = corba_policy.transport;
    policy.name_id     = corba_policy.name_id;
    return policy;
}
