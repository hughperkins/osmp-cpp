// Copyright Hugh Perkins 2004
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURVector3E. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

//! \file
//! \brief mvbasictypes contains the definition of Rot, Vector3, Vector3 and Color

// see headerfile BasicTypes.h for documentation

#include <sstream>

#include "tinyxml.h"

#include "Diag.h"
#include "BasicTypes.h"

Color::Color( const TiXmlElement *p_Element )
{
    r = atof( p_Element->Attribute( "r" ) );
    g = atof( p_Element->Attribute( "g" ) );
    b = atof( p_Element->Attribute( "b" ) );
}
char *Color::ToXMLAttributes()
{
    sprintf( XMLAttributes, "r=\"%f\" g=\"%f\" b=\"%f\"", r, g, b );
    return XMLAttributes;
}
void Color::WriteToXMLElement( TiXmlElement *p_Element )
{
    p_Element->SetDoubleAttribute( "r", r );
    p_Element->SetDoubleAttribute( "g", g );
    p_Element->SetDoubleAttribute( "b", b );
}

ostream& operator<< ( ostream& os, const Color &color )
{
    os << "<color r=\"" << color.r << "\" g=\"" << color.g << "\" b=\"" << color.b << "\"/>" ;
    return os;
}
