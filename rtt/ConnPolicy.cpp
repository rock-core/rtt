/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:08 CEST 2009  ConnPolicy.cpp

                        ConnPolicy.cpp -  description
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
 * ConnPolicy.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: kaltan
 */

#include "ConnPolicy.hpp"
#include "Property.hpp"
#include "PropertyBag.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

namespace RTT
{
    ConnPolicy ConnPolicy::buffer(int size, int lock_policy /*= LOCK_FREE*/, bool init_connection /*= false*/, bool pull /*= false*/)
    {
        ConnPolicy result(BUFFER, lock_policy);
        result.init = init_connection;
        result.pull = pull;
        result.size = size;
        return result;
    }

    ConnPolicy ConnPolicy::circularBuffer(int size, int lock_policy /*= LOCK_FREE*/, bool init_connection /*= false*/, bool pull /*= false*/)
    {
        ConnPolicy result(CIRCULAR_BUFFER, lock_policy);
        result.init = init_connection;
        result.pull = pull;
        result.size = size;
        return result;
    }

    ConnPolicy ConnPolicy::data(int lock_policy /*= LOCK_FREE*/, bool init_connection /*= true*/, bool pull /*= false*/)
    {
        ConnPolicy result(DATA, lock_policy);
        result.init = init_connection;
        result.pull = pull;
        return result;
    }

    ConnPolicy::ConnPolicy(int type /* = DATA*/, int lock_policy /*= LOCK_FREE*/)
        : type(type), init(false), lock_policy(lock_policy), pull(false), size(0)
        , transport(0)
        , signalling(true)
        , data_size(0) {}

    /** @cond */
    /** This is dead code. We use the boost::serialization now.
     */
    bool composeProperty(const PropertyBag& bag, ConnPolicy& result)
    {
        Property<int> i;
        Property<bool> b;
        Property<string> s;
        if ( bag.getType() != "ConnPolicy")
            return false;
        log(Debug) <<"Composing ConnPolicy..." <<endlog();
        i = bag.getProperty("type");
        if ( i.ready() )
            result.type = i.get();
        else if ( bag.find("type") ){
            log(Error) <<"ConnPolicy: wrong property type of 'type'."<<endlog();
            return false;
        }
        i = bag.getProperty("lock_policy");
        if ( i.ready() )
            result.lock_policy = i.get();
        else if ( bag.find("lock_policy") ){
            log(Error) <<"ConnPolicy: wrong property type of 'lock_policy'."<<endlog();
            return false;
        }
        i = bag.getProperty("size");
        if ( i.ready() )
            result.size = i.get();
        else if ( bag.find("size") ){
            log(Error) <<"ConnPolicy: wrong property type of 'size'."<<endlog();
            return false;
        }
        i = bag.getProperty("data_size");
        if ( i.ready() )
            result.data_size = i.get();
        else if ( bag.find("data_size") ){
            log(Error) <<"ConnPolicy: wrong property type of 'data_size'."<<endlog();
            return false;
        }
        i = bag.getProperty("transport");
        if ( i.ready() )
            result.transport = i.get();
        else if ( bag.find("transport") ){
            log(Error) <<"ConnPolicy: wrong property type of 'transport'."<<endlog();
            return false;
        }

        b = bag.getProperty("init");
        if ( b.ready() )
            result.init = b.get();
        else if ( bag.find("init") ){
            log(Error) <<"ConnPolicy: wrong property type of 'init'."<<endlog();
            return false;
        }
        b = bag.getProperty("pull");
        if ( b.ready() )
            result.pull = b.get();
        else if ( bag.find("pull") ){
            log(Error) <<"ConnPolicy: wrong property type of 'pull'."<<endlog();
            return false;
        }

        s = bag.getProperty("name_id");
        if ( s.ready() )
            result.name_id = s.get();
        else if ( bag.find("name_id") ){
            log(Error) <<"ConnPolicy: wrong property type of 'name_id'."<<endlog();
            return false;
        }
        return true;
    }

    /** This is dead code. We use the boost::serialization now.
     * @internal
     */
    void decomposeProperty(const ConnPolicy& cp, PropertyBag& targetbag)
    {
        log(Debug) <<"Decomposing ConnPolicy..." <<endlog();
        assert( targetbag.empty() );
        targetbag.setType("ConnPolicy");
        targetbag.ownProperty( new Property<int>("type","Data type", cp.type));
        targetbag.ownProperty( new Property<bool>("init","Initialize flag", cp.init));
        targetbag.ownProperty( new Property<int>("lock_policy","Locking Policy", cp.lock_policy));
        targetbag.ownProperty( new Property<bool>("pull","Fetch data over network", cp.pull));
        targetbag.ownProperty( new Property<int>("size","The size of a buffered connection", cp.size));
        targetbag.ownProperty( new Property<int>("transport","The prefered transport. Set to zero if unsure.", cp.transport));
        targetbag.ownProperty( new Property<int>("data_size","A hint about the data size of a single data sample. Set to zero if unsure.", cp.transport));
        targetbag.ownProperty( new Property<string>("name_id","The name of the connection to be formed.",cp.name_id));
    }
    /** @endcond */

    std::ostream &operator<<(std::ostream &os, const ConnPolicy &cp)
    {
        std::string type;
        switch(cp.type) {
            case ConnPolicy::UNBUFFERED:      type = "UNBUFFERED"; break;
            case ConnPolicy::DATA:            type = "DATA"; break;
            case ConnPolicy::BUFFER:          type = "BUFFER"; break;
            case ConnPolicy::CIRCULAR_BUFFER: type = "CIRCULAR_BUFFER"; break;
            default:                          type = "(unknown type)"; break;
        }
        if (cp.size > 0) {
            type += "[" + boost::lexical_cast<std::string>(cp.size) + "]";
        }

        std::string lock_policy;
        switch(cp.lock_policy) {
            case ConnPolicy::UNSYNC:    lock_policy = "UNSYNC"; break;
            case ConnPolicy::LOCKED:    lock_policy = "LOCKED"; break;
            case ConnPolicy::LOCK_FREE: lock_policy = "LOCK_FREE"; break;
            default:                    lock_policy = "(unknown lock policy)"; break;
        }

        std::string pull;
        // note: cast to int to suppress clang "warning: switch condition has boolean value"
        switch(int(cp.pull)) {
            case int(ConnPolicy::PUSH): pull = "PUSH"; break;
            case int(ConnPolicy::PULL): pull = "PULL"; break;
        }

        os << pull << " ";
        os << lock_policy << " ";
        os << type;
        if (!cp.name_id.empty()) os << " (name_id=" << cp.name_id << ")";

        return os;
    }
}
