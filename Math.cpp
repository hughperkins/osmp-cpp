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
//! \brief Contains quaternion and matrix maths

// see headerfile Math.h for documentation

#include <math.h>
#include <iostream>
using namespace std;

#include "Math.h"

#include "Diag.h"

void RotMultiply( Rot &Qr, const Rot &Q1, const Rot &Q2 )
{
    Qr.s = Q1.s * Q2.s - Q1.x * Q2.x - Q1.y * Q2.y - Q1.z * Q2.z;

    Qr.x = Q1.s * Q2.x + Q1.x * Q2.s     +    Q1.y * Q2.z - Q1.z * Q2.y;
    Qr.y = Q1.s * Q2.y + Q1.y * Q2.s     +    Q1.z * Q2.x - Q1.x * Q2.z;
    Qr.z = Q1.s * Q2.z + Q1.z * Q2.s     +    Q1.x * Q2.y - Q1.y * Q2.x;

    //DEBUG(  "RotMULITPLY in=" << Q1 << " " << Q2 << " out=" << Qr <<endl;
}

void VectorCross( Vector3 &vR, const Vector3 &v1, const Vector3 &v2 )
{
    vR.x = v1.y * v2.z - v1.z * v2.y;
    vR.y = v1.z * v2.x - v1.x * v2.z;
    vR.z = v1.x * v2.y - v1.y * v2.x;
}

float VectorDot( const Vector3 &v1, const Vector3 &v2 )
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void RotBetween( Rot &rResult, const Vector3 &Vector1, const Vector3 &Vector2 )
{
    Vector3 VectorNorm1 = Vector1;
    Vector3 VectorNorm2 = Vector2;

    VectorNormalize( VectorNorm1 );
    VectorNormalize( VectorNorm2 );

    Vector3 RotationAxis;
    VectorCross( RotationAxis, VectorNorm1, VectorNorm2 );
    //DEBUG(  "math: " << RotationAxis ); // DEBUG

    //DEBUG(  fabs( RotationAxis.x ) << " " << fabs( RotationAxis.y ) << " " << fabs( RotationAxis.z ) ); // DEBUG
    if( fabs( RotationAxis.x ) < 0.0005 && fabs( RotationAxis.y ) < 0.0005 && fabs( RotationAxis.z ) < 0.0005 )
    {
        Vector3 RandomVector = VectorNorm1;
        RandomVector.x += 0.5;
        VectorNormalize( RandomVector );
        VectorCross( RotationAxis, VectorNorm1, RandomVector );

        AxisAngle2Rot( rResult, RotationAxis, 3.1415926535 );
    }
    else
    {
        float DotProduct = VectorDot( VectorNorm1, VectorNorm2 );
        DEBUG( "DotProduct: " << DotProduct ); // DEBUG
        float Vangle = acos( DotProduct );
        DEBUG( "math: " << Vangle ); // DEBUG
        AxisAngle2Rot( rResult, RotationAxis, Vangle );
    }
}

float VectorMag( const Vector3 &V )
{
    return sqrt( V.x * V.x + V.y * V.y + V.z * V.z );
}

void VectorNormalize( Vector3 &V )
{
    //DEBUG(  "VectorNormalize in= " << V.x << " " << V.y << " " << V.z ); // DEBUG
    float fMag = sqrt( V.x * V.x + V.y * V.y + V.z * V.z );

    if( fMag > 0.0001 )
    {
        V.x = V.x / fMag;
        V.y = V.y / fMag;
        V.z = V.z / fMag;
    }
    else
    {
        V.x = 1.0;
        V.y = 0.0;
        V.z = 0.0;
    }
    //DEBUG(  "VectorNormalize out= " << V.x << " " << V.y << " " << V.z ); // DEBUG
}

void AxisAngle2Rot( Rot &Qr, const Vector3 &V, const float Theta )
{
    Vector3 NormVector = V;
    VectorNormalize( NormVector );

    float sin_a = sin( Theta / 2 );
    float cos_a = cos( Theta / 2 );

    Qr.x    = NormVector.x * sin_a;
    Qr.y    = NormVector.y * sin_a;
    Qr.z    = NormVector.z * sin_a;
    Qr.s    = cos_a;
    //DEBUG(  "AXISANGLE2Rot in= " << V.x << " " << V.y << " " << V.z << " theta " << Theta << "out=" << Qr ); // DEBUG
}

void AxisAngles2Rot(Rot &Qr, const Vector3 &V)
{
    Rot XRot, YRot, ZRot, result;

    AxisAngle2Rot(XRot, XAXIS, V.x);
    AxisAngle2Rot(YRot, YAXIS, V.y);
    AxisAngle2Rot(ZRot, ZAXIS, V.z);

    RotMultiply(result, ZRot, YRot);
    RotMultiply(Qr, result, XRot);
}

void InverseRot( Rot &RInverted, const Rot &R )
{
    RInverted.x = - R.x;
    RInverted.y = - R.y;
    RInverted.z = - R.z;
    RInverted.s = R.s;
}

void Rot2AxisAngle(Vector3 &Vr, float &Thetar, const Rot &R )
{
    //QuaternionNormalize( |X,Y,Z,W| );

    float cos_a = R.s;
    Thetar = acos( cos_a ) * 2;
    float sin_a = sqrt( 1.0 - cos_a * cos_a );

    if ( fabs( sin_a )> 0.0005 )
    {
        Vr.x = R.x / sin_a;
        Vr.y = R.y / sin_a;
        Vr.z = R.z / sin_a;
    }
    else
    {
        Vr.x = 1;
        Vr.y = 0;
        Vr.z = 0;
    }
}

/*
void MultiplyVectorByRot( Vector3 &vResult, const Rot &Rot, const Vector3 &vector  )
{
  Rot InverseInRot;
  InverseRot( InverseInRot, Rot );
  Rot VectorRot;
  VectorRot.x = vector.x;
  VectorRot.y = vector.y;
  VectorRot.z = vector.z;
  VectorRot.s = 0;
  Rot IntRot;
  RotMultiply( IntRot, VectorRot, Rot );
  Rot ResultRot;
  RotMultiply( ResultRot, InverseInRot, IntRot );
  vResult.x = ResultRot.x;
  vResult.y = ResultRot.y;
  vResult.z = ResultRot.z;
}
*/

float SquareVectorDistance( const Vector3 &p1, const Vector3 &p2 )
{
    float Distance = (p2.x - p1.x ) * (p2.x - p1.x ) + (p2.y - p1.y ) * (p2.y - p1.y ) + (p2.z - p1.z ) * (p2.z - p1.z );
    return Distance;
}

void SetZeroRot( Rot &rot )
{
    rot.x = 0;
    rot.y = 0;
    rot.z = 0;
    rot.s = 1;
}

ostream& operator<< (ostream& os, const Vector3& vector)
{
    os << "<vector x=\"" << vector.x << "\" y=\"" << vector.y << "\" z=\"" << vector.z << "\"/>" ;
    return os;
}

void Vector3::WriteToXMLElement( TiXmlElement *p_Element )
{
    DEBUG(  "Vector3::WriteToXMLElement x = " << x );
    p_Element->SetDoubleAttribute( "x", x );
    p_Element->SetDoubleAttribute( "y", y );
    p_Element->SetDoubleAttribute( "z", z );
    DEBUG(  "Vector3::WriteToXMLElement p_Element.Attribute(x) = " << p_Element->Attribute("x") );
}

Rot::Rot( const TiXmlElement *p_Element )
{
    x = atof( p_Element->Attribute( "x" ) );
    y = atof( p_Element->Attribute( "y" ) );
    z = atof( p_Element->Attribute( "z" ) );
    s = atof( p_Element->Attribute( "s" ) );
}
char *Rot::ToXMLAttributes()
{
    sprintf( XMLAttributes, "x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\"", x, y, z, s );
    return XMLAttributes;
}
void Rot::WriteToXMLElement( TiXmlElement *p_Element )
{
    p_Element->SetDoubleAttribute( "x", x );
    p_Element->SetDoubleAttribute( "y", y );
    p_Element->SetDoubleAttribute( "z", z );
    p_Element->SetDoubleAttribute( "s", s );
}

ostream& operator<< (ostream& os, const Rot& rot)
{
    //   DEBUG(  rot.x << " " << rot.y << " " << rot.z ); // DEBUG
    os << "<rot x=\"" << rot.x << "\" y=\"" << rot.y << "\" z=\"" << rot.z << "\" s=\"" << rot.s << "\"/>" ;
    return os;
}

