/***************************************************************************
                          dms.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dms.h"

#include <stdlib.h>

#include <QRegExp>

#include <kglobal.h>
#include <klocale.h>

void dms::setD(const int &d, const int &m, const int &s, const int &ms) {
    D = (double)abs(d) + ((double)m + ((double)s + (double)ms/1000.)/60.)/60.;
    if (d<0) {D = -1.0*D;}
}

void dms::setH( const double &x ) {
    setD( x*15.0 );
}

void dms::setH(const int &h, const int &m, const int &s, const int &ms) {
    D = 15.0*((double)abs(h) + ((double)m + ((double)s + (double)ms/1000.)/60.)/60.);
    if (h<0) {D = -1.0*D;}
}

void dms::setRadians( const double &Rad ) {
    setD( Rad/DegToRad );
}

bool dms::setFromString( const QString &str, bool isDeg ) {
    int d(0), m(0);
    double s(0.0);
    bool checkValue( false ), badEntry( false ), negative( false );
    QString entry = str.trimmed();
    entry.remove( QRegExp("[hdms'\"�]") );

    //Account for localized decimal-point settings
    //QString::toDouble() requires that the decimal symbol is "."
    entry.replace( KGlobal::locale()->decimalSymbol(), "." );

    //empty entry returns false
    if ( entry.isEmpty() ) {
        setD( NaN::d );
        return false;
    }

    //try parsing a simple integer
    d = entry.toInt( &checkValue );
    if ( checkValue ) {
        if (isDeg) setD( d, 0, 0 );
        else setH( d, 0, 0 );
        return true;
    }

    //try parsing a simple double
    double x = entry.toDouble( &checkValue );
    if ( checkValue ) {
        if ( isDeg ) setD( x );
        else setH( x );
        return true;
    }

    //try parsing multiple fields.
    QStringList fields;

    //check for colon-delimiters or space-delimiters
    if ( entry.contains(':') )
        fields = entry.split( ':', QString::SkipEmptyParts );
    else fields = entry.split( ' ', QString::SkipEmptyParts );

    //anything with one field is invalid!
    if ( fields.count() == 1 ) {
        setD( NaN::d );
        return false;
    }

    //If two fields we will add a third one, and then parse with
    //the 3-field code block. If field[1] is an int, add a third field equal to "0".
    //If field[1] is a double, convert it to integer arcmin, and convert
    //the remainder to integer arcsec
    //If field[1] is neither int nor double, return false.
    if ( fields.count() == 2 ) {
        m = fields[1].toInt( &checkValue );
        if ( checkValue ) fields.append( QString( "0" ) );
        else {
            double mx = fields[1].toDouble( &checkValue );
            if ( checkValue ) {
                fields[1] = QString::number( int(mx) );
                fields.append( QString::number( int( 60.0*(mx - int(mx)) ) ) );
            } else {
                setD( NaN::d );
                return false;
            }
        }
    }

    //Now have (at least) three fields ( h/d m s );
    //we can ignore anything after 3rd field
    if ( fields.count() >= 3 ) {
        //See if first two fields parse as integers, and third field as a double

        d = fields[0].toInt( &checkValue );
        if ( !checkValue ) badEntry = true;
        m = fields[1].toInt( &checkValue );
        if ( !checkValue ) badEntry = true;
        s = fields[2].toDouble( &checkValue );
        if ( !checkValue ) badEntry = true;

        //Special case: If first field is "-0", store the negative sign.
        //(otherwise it gets dropped)
        if ( fields[0].at(0) == '-' && d == 0 ) negative = true;
    }

    if ( !badEntry ) {
        double D = (double)abs(d) + (double)abs(m)/60.
                   + (double)fabs(s)/3600.;

        if ( negative || d<0 || m < 0 || s<0 ) { D = -1.0*D;}

        if (isDeg) {
            setD( D );
        } else {
            setH( D );
        }
    } else {
        setD( NaN::d );
        return false;
    }

    return true;
}

int dms::arcmin( void ) const {
    int am = int( float( 60.0*( fabs(D) - abs( degree() ) ) ) );
    if ( D<0.0 && D>-1.0 ) { //angle less than zero, but greater than -1.0
        am = -1*am; //make minute negative
    }
    return am; // Warning: Will return 0 if the value is NaN
}

int dms::arcsec( void ) const {
    int as = int( float( 60.0*( 60.0*( fabs(D) - abs( degree() ) ) - abs( arcmin() ) ) ) );
    //If the angle is slightly less than 0.0, give ArcSec a neg. sgn.
    if ( degree()==0 && arcmin()==0 && D<0.0 ) {
        as = -1*as;
    }
    return as; // Warning: Will return 0 if the value is NaN
}

int dms::marcsec( void ) const {
    int as = int( float( 1000.0*(60.0*( 60.0*( fabs(D) - abs( degree() ) ) - abs( arcmin() ) ) - abs( arcsec() ) ) ) );
    //If the angle is slightly less than 0.0, give ArcSec a neg. sgn.
    if ( degree()==0 && arcmin()==0 && arcsec()== 0 && D<0.0 ) {
        as = -1*as;
    }
    return as; // Warning: Will return 0 if the value is NaN
}

int dms::minute( void ) const {
    int hm = int( float( 60.0*( fabs( Hours() ) - abs( hour() ) ) ) );
    if ( Hours()<0.0 && Hours()>-1.0 ) { //angle less than zero, but greater than -1.0
        hm = -1*hm; //make minute negative
    }
    return hm; // Warning: Will return 0 if the value is NaN
}

int dms::second( void ) const {
    int hs = int( float( 60.0*( 60.0*( fabs( Hours() ) - abs( hour() ) ) - abs( minute() ) ) ) );
    if ( hour()==0 && minute()==0 && Hours()<0.0 ) {
        hs = -1*hs;
    }
    return hs; // Warning: Will return 0 if the value is NaN
}

int dms::msecond( void ) const {
    int hs = int( float( 1000.0*(60.0*( 60.0*( fabs( Hours() ) - abs( hour() ) ) - abs( minute() ) ) - abs( second() ) ) ) );
    if ( hour()==0 && minute()==0 && second()==0 && Hours()<0.0 ) {
        hs = -1*hs;
    }
    return hs; // Warning: Will return 0 if the value is NaN
}


const dms dms::reduce( void ) const {
    return dms( D - 360.0*floor(D/360.0) );
}

const QString dms::toDMSString(const bool forceSign) const {
    QString dummy;
    char pm(' ');
    int dd = abs(degree());
    int dm = abs(arcmin());
    int ds = abs(arcsec());

    if ( Degrees() < 0.0 ) pm = '-';
    else if (forceSign && Degrees() > 0.0 ) pm = '+';

    if (dd < 10)
        return dummy.sprintf("%c%1d%c %02d\' %02d\"", pm, dd, 176, dm, ds);
    if (dd < 100)
        return dummy.sprintf("%c%2d%c %02d\' %02d\"", pm, dd, 176, dm, ds);

    return dummy.sprintf("%c%3d%c %02d\' %02d\"", pm, dd, 176, dm, ds);
}

const QString dms::toHMSString() const {
    QString dummy;
    return dummy.sprintf("%02dh %02dm %02ds", hour(), minute(), second());
}

dms dms::fromString(const QString &st, bool deg) {
    dms result;
    bool ok( false );

    ok = result.setFromString( st, deg );

    //	if ( ok )
    return result;
    //	else {
    //		kDebug() << i18n( "Could Not Set Angle from string: " ) << st;
    //		return result;
    //	}
}

// M_PI is defined in math.h
const double dms::PI = M_PI;
const double dms::DegToRad = PI/180.0;
