/***************************************************************************
                          ksalmanac.cpp  -  description

                             -------------------
    begin                : Friday May 8, 2009
    copyright            : (C) 2009 by Prakash Mohan
    email                : prakash.mohan@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ksalmanac.h"

#include <math.h>

#include "skyobjects/kssun.h"
#include "skyobjects/ksmoon.h"
#include "geolocation.h"
#include "kstars.h"
#include "kstarsdatetime.h"
#include "ksnumbers.h"
#include "dms.h"
#include "skyobjects/skyobject.h"

KSAlmanac* KSAlmanac::pinstance=NULL;

KSAlmanac* KSAlmanac::Instance() {
    if(!pinstance) pinstance = new KSAlmanac;
    return pinstance;
}

KSAlmanac::KSAlmanac() {
    ks = KStars::Instance();
    dt = KStarsDateTime::currentDateTime();
    geo = ks->geo();
    dt.setTime(QTime());
    dt = geo->LTtoUT(dt);
    m_Sun = new KSSun;
    m_Moon = new KSMoon;
    SunRise=SunSet=MoonRise=MoonSet=0;
    update();
}

void KSAlmanac::update() {
    RiseSetTime( m_Sun, &SunRise, &SunSet, &SunRiseT, &SunSetT );
    RiseSetTime( m_Moon, &MoonRise, &MoonSet, &MoonRiseT, &MoonSetT );
}

void KSAlmanac::RiseSetTime( SkyObject *o, double *riseTime, double *setTime, QTime *RiseTime, QTime *SetTime ) {
    //Compute Sun rise and set times
    const KStarsDateTime today = dt;
    const GeoLocation* _geo = geo;
    *RiseTime = o->riseSetTime( today.addDays(1), _geo, true );
    *SetTime = o->riseSetTime( today, _geo, false );
    *riseTime = -1.0 * RiseTime->secsTo(QTime()) / 86400.0; 
    *setTime = -1.0 * SetTime->secsTo(QTime()) / 86400.0;
  //check to see if Sun is circumpolar
    //requires temporary repositioning of Sun to target date
    KSNumbers *num = new KSNumbers( dt.djd() );
    KSNumbers *oldNum = new KSNumbers( ks->data()->ut().djd() );
    dms LST = geo->GSTtoLST( dt.gst() );
    o->updateCoords( num, true, geo->lat(), &LST );
    if ( o->checkCircumpolar( geo->lat() ) ) {
        if ( o->alt()->Degrees() > 0.0 ) {
            //Circumpolar, signal it this way:
            *riseTime = 0.0;
            *setTime = 1.0;
        } else {
            //never rises, signal it this way:
            *riseTime = 0.0;
            *setTime = -1.0;
        }
    }
    o->updateCoords( oldNum, true, ks->geo()->lat(), ks->LST() );
    o->EquatorialToHorizontal( ks->LST(), ks->geo()->lat() );
    delete num;
    delete oldNum;

}

void KSAlmanac::setDate( KStarsDateTime *newdt ) {
    dt = *newdt; 
    update();
}

void KSAlmanac::setLocation( GeoLocation *m_geo ) {
    geo = m_geo;
    update();
}

/*
double KSAlmanac::sunZenithAngleToTime( double z ) {
    double HA = acos( ( cos( z * dms::DegToRad ) - m_Sun->dec()->sin() * geo->lat()->sin() ) / (m_Sun->dec()->cos() * geo->lat()->cos()) );
    double HASunset = acos( ( -m_Sun->dec()->sin() * geo->lat()->sin() ) / (m_Sun->dec()->cos() * geo->lat()->cos()) );
    return SunSet*24*60 + ( HA - HASunset ) * 4.0 * 15.0;
}
*/
    