/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:56 CEST 2006  RealTimeToolkit.cxx 

                        RealTimeToolkit.cxx -  description
                           -------------------
    begin                : Mon June 26 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@fmtc.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 

#include <pkgconf/system.h>
#include <rtt/RealTimeToolkit.hpp>
#include <rtt/Types.hpp>
#include <rtt/Operators.hpp>
#include <rtt/OperatorTypes.hpp>
#include <rtt/TemplateTypeInfo.hpp>
#include <rtt/MultiVector.hpp>
#include <rtt/mystd.hpp>

#include <rtt/TypeStream.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/VectorComposition.hpp>
#include <ostream>

namespace RTT
{
    using namespace std;
    

    RealTimeToolkitPlugin RealTimeToolkit;

    std::string RealTimeToolkitPlugin::getName()
    {
        return "RealTime";
    }

#ifndef ORO_EMBEDDED
    /**
     * This class tells Orocos how to handle std::vector<double>.
     */
    struct StdVectorTypeInfo
        : public TemplateContainerTypeInfo<std::vector<double>, int, double, ArrayIndexChecker<std::vector<double> >, SizeAssignChecker<std::vector<double> >, true >
    {
        StdVectorTypeInfo( std::string name )
            : TemplateContainerTypeInfo<std::vector<double>, int, double, ArrayIndexChecker<std::vector<double> >, SizeAssignChecker<std::vector<double> >, true >(name)
        {}

        bool decomposeTypeImpl(const std::vector<double>& vec, PropertyBag& targetbag) const
        {
            decomposeProperty( vec, targetbag );
            return true;
        }

        bool composeTypeImpl(const PropertyBag& bag, std::vector<double>& result) const
        {
            return composeProperty( bag, result );
        }
    };

#endif
    /**
     * Write boolean as 'true' or 'false'.
     */
    struct BoolTypeInfo
        : public TemplateTypeInfo<bool>
    {
        BoolTypeInfo()
            : TemplateTypeInfo<bool>("bool")
        {}
        
        virtual std::ostream& write( std::ostream& os, DataSourceBase::shared_ptr in ) const {
            DataSource<bool>* d = AdaptDataSource<bool>()( in );
            if (d)
                return os << boolalpha << d->value();
            return os;
        }
    };

    bool RealTimeToolkitPlugin::loadTypes() 
    {
        TypeInfoRepository::shared_ptr ti = TypeInfoRepository::Instance();

        ti->addType( new TemplateTypeInfo<int, true>("int") );
        ti->addType( new TemplateTypeInfo<unsigned int, true>("uint") );
        ti->addType( new TemplateTypeInfo<double, true>("double") );
        ti->addType( new BoolTypeInfo() );

#ifdef OROPKG_CORBA
        ti->addType( new TemplateTypeInfo<CORBA::Any, false>("CORBA::Any") );
#endif
#ifndef ORO_EMBEDDED
        ti->addType( new TemplateTypeInfo<PropertyBag, false>("PropertyBag") );
        ti->addType( new TemplateTypeInfo<float, true>("float") );
        ti->addType( new TemplateTypeInfo<char, true>("char") );
        ti->addType( new StdVectorTypeInfo("array") );
#endif

        // string is a special case for assignment, we need to assign from the c_str() instead of from the string(),
        // the latter causes capacity changes, probably due to the copy-on-write implementation of string(). Assignment
        // from a c-style string obviously disables a copy-on-write connection.
        ti->addType( new TemplateContainerTypeInfo<std::string, int, char, ArrayIndexChecker<std::string>,AlwaysAssignChecker<std::string>, true >("string") );
        
        return true;
    }

    namespace {
#ifndef ORO_EMBEDDED
        // CONSTRUCTORS
        struct array_ctor
            : public std::unary_function<int, const std::vector<double>&>
        {
            typedef const std::vector<double>& (Signature)( int );
            mutable boost::shared_ptr< std::vector<double> > ptr;
            array_ctor()
                : ptr( new std::vector<double>() ) {}
            const std::vector<double>& operator()( int size ) const
            {
                ptr->resize( size );
                return *(ptr);
            }
        };
#endif

        struct string_ctor
            : public std::unary_function<int, const std::string&>
        {
            mutable boost::shared_ptr< std::string > ptr;
            typedef const std::string& (Signature)( int );
            string_ctor()
                : ptr( new std::string() ) {}
            const std::string& operator()( int size ) const
            {
                ptr->resize( size );
                return *(ptr);
            }
        };

        struct string_index
            : public std::binary_function<const std::string&, int, char>
        {
            char operator()(const std::string& s, int index) const
            {
                if ( index >= (int)(s.length()) || index < 0)
                    return 0;
                return s[index];
            }
        };

        template<class T>
        struct get_capacity
            : public std::unary_function<T, int>
        {
            int operator()(T cont ) const
            {
                return cont.capacity();
            }
        };

        template<class T>
        struct get_size
            : public std::unary_function<T, int>
        {
            int operator()(T cont ) const
            {
                return cont.size();
            }
        };

                    
#ifndef ORO_EMBEDDED
        struct array_index
            : public std::binary_function<const std::vector<double>&, int, double>
        {
            double operator()(const std::vector<double>& v, int index) const
            {
                if ( index >= (int)(v.size()) || index < 0)
                    return 0;
                return v[index];
            }
        };
#endif
    }

    using namespace detail;

    bool RealTimeToolkitPlugin::loadConstructors()
    {
        TypeInfoRepository::shared_ptr ti = TypeInfoRepository::Instance();

#ifndef ORO_EMBEDDED
        ti->type("array")->addConstructor( newConstructor( array_ctor() ) );
#endif
        ti->type("string")->addConstructor( newConstructor( string_ctor() ) );
        return true;
    }

    bool RealTimeToolkitPlugin::loadOperators()
    {
        OperatorRepository::shared_ptr oreg = OperatorRepository::Instance();

        // boolean stuff:
        oreg->add( newUnaryOperator( "!", std::logical_not<bool>() ) );
        oreg->add( newBinaryOperator( "&&", std::logical_and<bool>() ) );
        oreg->add( newBinaryOperator( "||", std::logical_or<bool>() ) );
        oreg->add( newBinaryOperator( "==", std::equal_to<bool>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to<bool>() ) );

        // int stuff
        oreg->add( newUnaryOperator( "-", std::negate<int>() ) );
        oreg->add( newUnaryOperator( "+", identity<int>() ) );
        oreg->add( newBinaryOperator( "*", std::multiplies<int>() ) );
        oreg->add( newBinaryOperator( "/", divides3<int,int,int>() ) ); // use our own divides<> which detects div by zero
        oreg->add( newBinaryOperator( "%", std::modulus<int>() ) );
        oreg->add( newBinaryOperator( "+", std::plus<int>() ) );
        oreg->add( newBinaryOperator( "-", std::minus<int>() ) );
        oreg->add( newBinaryOperator( "<", std::less<int>() ) );
        oreg->add( newBinaryOperator( "<=", std::less_equal<int>() ) );
        oreg->add( newBinaryOperator( ">", std::greater<int>() ) );
        oreg->add( newBinaryOperator( ">=", std::greater_equal<int>() ) );
        oreg->add( newBinaryOperator( "==", std::equal_to<int>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to<int>() ) );


        // double stuff..
        oreg->add( newUnaryOperator( "-", std::negate<double>() ) );
        oreg->add( newUnaryOperator( "+", identity<double>() ) );
        oreg->add( newBinaryOperator( "*", std::multiplies<double>() ) );
        oreg->add( newBinaryOperator( "/", std::divides<double>() ) );
        oreg->add( newBinaryOperator( "+", std::plus<double>() ) );
        oreg->add( newBinaryOperator( "-", std::minus<double>() ) );
        oreg->add( newBinaryOperator( "<", std::less<double>() ) );
        oreg->add( newBinaryOperator( "<=", std::less_equal<double>() ) );
        oreg->add( newBinaryOperator( ">", std::greater<double>() ) );
        oreg->add( newBinaryOperator( ">=", std::greater_equal<double>() ) );
        oreg->add( newBinaryOperator( "==", std::equal_to<double>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to<double>() ) );
        // with integers :
        oreg->add( newBinaryOperator( "*", multiplies3<double,int, double>() ) );
        oreg->add( newBinaryOperator( "*", multiplies3<double,double, int>() ) );
        oreg->add( newBinaryOperator( "/", divides3<double,int, double>() ) );
        oreg->add( newBinaryOperator( "/", divides3<double,double, int>() ) );
        oreg->add( newBinaryOperator( "+", adds3<double,int, double>() ) );
        oreg->add( newBinaryOperator( "+", adds3<double,double, int>() ) );
        oreg->add( newBinaryOperator( "-", subs3<double,int, double>() ) );
        oreg->add( newBinaryOperator( "-", subs3<double,double, int>() ) );

        // strings
        //  oreg->add( newBinaryOperator( "+", std::plus<std::string>() ) );
        oreg->add( newBinaryOperator( "==", std::equal_to<const std::string&>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to< const std::string&>() ) );
        oreg->add( newBinaryOperator( "<", std::less<const std::string&>() ) );
        oreg->add( newBinaryOperator( ">", std::greater<const std::string&>() ) );
        oreg->add( newBinaryOperator( "<=", std::less_equal<std::string>() ) );
        oreg->add( newBinaryOperator( ">=", std::greater_equal<std::string>() ) );
        oreg->add( newBinaryOperator( "[]", string_index() ) );
        oreg->add( newDotOperator( "size", get_size<const std::string&>() ) );
        oreg->add( newDotOperator( "length", get_size<const std::string&>() ) );
        oreg->add( newDotOperator( "capacity", get_capacity<const std::string&>() ) );

#ifndef ORO_EMBEDDED
        // chars
        oreg->add( newBinaryOperator( "==", std::equal_to<char>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to<char>() ) );
        oreg->add( newBinaryOperator( "<", std::less<char>() ) );
        oreg->add( newBinaryOperator( ">", std::greater<char>() ) );
        oreg->add( newBinaryOperator( "<=", std::less_equal<char>() ) );
        oreg->add( newBinaryOperator( ">=", std::greater_equal<char>() ) );

        // array :
        oreg->add( newBinaryOperator( "[]", array_index() ) );
        oreg->add( newBinaryOperator( "==", std::equal_to<const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "!=", std::not_equal_to<const std::vector<double>&>() ) );

#if 0
        // causes memory allocation....
        oreg->add( newUnaryOperator( "-", std::negate<const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "*", std::multiplies<const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "+", std::plus<const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "-", std::minus<const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "*", multiplies3<const std::vector<double>&, double, const std::vector<double>&>() ) );
        oreg->add( newBinaryOperator( "*", multiplies3<const std::vector<double>&, const std::vector<double>&, double>() ) );
        oreg->add( newBinaryOperator( "/", divides3<const std::vector<double>&, const std::vector<double>&, double>() ) );
#endif
        oreg->add( newDotOperator( "size", get_size<const std::vector<double>&>() ) );
        oreg->add( newDotOperator( "capacity", get_capacity<const std::vector<double>&>() ) );
#endif
        return true;
    }
}
